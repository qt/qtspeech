// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only

#include "qtexttospeech_winrt_audiosource.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include <QtCore/private/qfunctions_winrt_p.h>
#include <QtCore/private/qsystemerror_p.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Media::SpeechSynthesis;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

QT_BEGIN_NAMESPACE

/*
    AudioSource implements a sequential QIODevice for a stream of synthesized speech.
    It also implements the handler interfaces for responding to the asynchronous
    operations of the synthesized speech stream being avialable, and reading data from
    the stream via a COM buffer.

    Whenever the QIODevice has read all the data from the COM buffer, more data is
    requested. When the data is available (the BytesReadyHandler's handler's Invoke
    implementation is called), readyRead is emitted.

    The AudioSource directly controls a QAudioSink. As soon as the data stream is
    available, the source calls QAudioSink::start. Pause/resume are delegated to the
    sink; closing the source stops the sink.
*/
AudioSource::AudioSource(ComPtr<IAsyncOperation<SpeechSynthesisStream*>> synthOperation)
    : synthOperation(synthOperation)
{
    synthOperation->put_Completed(this);

    audioFormat.setSampleFormat(QAudioFormat::Int16);
    audioFormat.setSampleRate(16000);
    audioFormat.setChannelConfig(QAudioFormat::ChannelConfigMono);
}

/*
    Calls close, which is virtual and called from ~QIODevice,
    but that won't call our override.
*/
AudioSource::~AudioSource()
{
    Q_ASSERT(ref == 0);
    close();
}

/*
    Cancel any incomplete asynchronous operations, and stop the
    sink before closing the QIODevice.\
*/
void AudioSource::close()
{
    ComPtr<IAsyncInfo> asyncInfo;
    AsyncStatus status = AsyncStatus::Completed;
    if (synthOperation) {
        if (HRESULT hr = synthOperation.As(&asyncInfo); SUCCEEDED(hr)) {
            asyncInfo->get_Status(&status);
            if (status != AsyncStatus::Completed)
                asyncInfo->Cancel();
        }
    }
    if (readOperation) {
        if (HRESULT hr = readOperation.As(&asyncInfo); SUCCEEDED(hr)) {
            asyncInfo->get_Status(&status);
            if (status != AsyncStatus::Completed)
                asyncInfo->Cancel();
        }
    }
    QIODevice::close();
}

qint64 AudioSource::bytesAvailable() const
{
    return bytesInBuffer() + QIODevice::bytesAvailable();
}

/*
    Fills data with as many bytes from the COM buffer as possible. If this
    empties the COM buffer, calls fetchMore to start a new asynchronous
    read operation.
*/
qint64 AudioSource::readData(char *data, qint64 maxlen)
{
    // this may happen as per the documentation
    if (!maxlen)
        return 0;

    Q_ASSERT(bufferByteAccess);

    const qint64 available = bytesInBuffer();
    maxlen = qMin(available, maxlen);

    if (!maxlen && atEnd())
        return -1;

    byte *pbyte = nullptr;
    bufferByteAccess->Buffer(&pbyte);
    pbyte += m_bufferOffset;

    switch (m_pause) {
    case NoPause:
        break;
    case PauseRequested: {
        Q_ASSERT(audioFormat.sampleFormat() == QAudioFormat::Int16);
        // we are dealing with artificially created sound, so we don't have
        // to find a large enough window with overall low energy; we can just
        // look for a series (e.g. 1/20th of a second) of samples with value 0.
        const int silenceDuration = audioFormat.sampleRate() / 20;
        const short *sample = reinterpret_cast<short*>(pbyte);
        const qsizetype sampleCount = maxlen / sizeof(short);
        qint64 silenceCount = 0;
        for (qint64 index = 0; index < sampleCount; ++index) {
            if (!sample[index]) {
                ++silenceCount;
            } else if (silenceCount > silenceDuration) {
                // long enough silence found, only provide the data until we are in the
                // silence. If the silence is at the beginning of our buffer, start from
                // there, otherwise play a bit of silence now.
                if (index != silenceCount)
                    silenceCount /= 2;

                maxlen = (index - silenceCount) * 2;
                // The next attempt to pull data will return nothing, and the audio sink
                // will move to idle state.
                m_pause = Paused;
                break;
            } else {
                silenceCount = 0;
            }
        }
        break;
    }
    case Paused:
        // starve the sink so that it goes idle
        maxlen = 0;
        break;
    }

    if (!maxlen)
        return 0;

    memcpy(data, pbyte, maxlen);

    // We emptied the buffer, so schedule fetching more
    if (available <= maxlen)
        QTimer::singleShot(0, this, &AudioSource::fetchMore);
    else
        m_bufferOffset += maxlen;

    return maxlen;
}

bool AudioSource::atEnd() const
{
    // not done as long as QIODevice's buffer is not empty
    if (!QIODevice::atEnd())
        return false;

    // If we get here, bytesAvailable() has returned 0, so our buffers are
    // exhaused. Try to see if we are waiting for readOperation to finish.
    AsyncStatus status = AsyncStatus::Completed;
    if (readOperation) {
        ComPtr<IAsyncInfo> asyncInfo;
        if (HRESULT hr = readOperation.As(&asyncInfo); SUCCEEDED(hr))
            asyncInfo->get_Status(&status);
    }
    if (status == AsyncStatus::Started)
        return false;

    // ... or if there is more in the stream
    UINT64 ioPos = 0;
    UINT64 ioSize = 0;
    if (inputStream) {
        ComPtr<IRandomAccessStream> ioStream;
        if (HRESULT hr = inputStream.As(&ioStream); SUCCEEDED(hr)) {
            ioStream->get_Size(&ioSize);
            ioStream->get_Position(&ioPos);
        }
    }
    return ioPos >= ioSize;
}

HRESULT AudioSource::QueryInterface(REFIID riid, VOID **ppvInterface)
{
    if (!ppvInterface)
        return E_POINTER;

    if (riid == __uuidof(IUnknown)) {
        *ppvInterface = static_cast<IUnknown*>(static_cast<StreamReadyHandler *>(this));
    } else if (riid == __uuidof(StreamReadyHandler)) {
        *ppvInterface = static_cast<StreamReadyHandler *>(this);
    } else if (riid == __uuidof(BytesReadyHandler)) {
        *ppvInterface = static_cast<BytesReadyHandler *>(this);
    } else {
        *ppvInterface = nullptr;
        return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
}

/*
    Completion handler for synthesising the stream.

    Is called as soon as synthesized pcm data is available, at which point we can
    open the QIODevice, fetch data, and start the audio sink.
*/
HRESULT AudioSource::Invoke(IAsyncOperation<SpeechSynthesisStream*> *operation, AsyncStatus status)
{
    Q_ASSERT(operation == synthOperation.Get());
    ComPtr<IAsyncInfo> asyncInfo;
    synthOperation.As(&asyncInfo);

    if (status == AsyncStatus::Error) {
        QString errorString;
        if (asyncInfo) {
            HRESULT errorCode;
            asyncInfo->get_ErrorCode(&errorCode);
            if (errorCode == 0x80131537) // Windows gives us only an Unknown error
                errorString = QStringLiteral("Error when synthesizing: Input format error");
            else
                errorString = QSystemError(errorCode, QSystemError::NativeError).toString();
        } else {
            errorString = QStringLiteral("Error when synthesizing: no information available");
        }
        setErrorString(errorString);
        emit errorInStream();
    }
    if (status != AsyncStatus::Completed) {
        if (asyncInfo)
            asyncInfo->Close();
        synthOperation.Reset();

        return E_FAIL;
    }


    ComPtr<ISpeechSynthesisStream> speechStream;
    HRESULT hr = operation->GetResults(&speechStream);
    RETURN_HR_IF_FAILED("Could not access stream.");

    hr = speechStream.As(&inputStream);
    RETURN_HR_IF_FAILED("Could not cast to inputStream.");

    ComPtr<IBufferFactory> bufferFactory;
    hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(),
                                IID_PPV_ARGS(&bufferFactory));
    RETURN_HR_IF_FAILED("Could not create buffer factory.");
    // use the same buffer size as default read chunk size
    bufferFactory->Create(16384, m_buffer.GetAddressOf());

    hr = m_buffer->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));
    RETURN_HR_IF_FAILED("Could not access buffer.");

    // release our reference to the speech stream operation
    if (asyncInfo)
        asyncInfo->Close();
    synthOperation.Reset();

    // we are buffered, but we don't want QIODevice to buffer as well
    open(QIODevice::ReadOnly|QIODevice::Unbuffered);
    fetchMore();
    emit streamReady(audioFormat);
    return S_OK;
}

/*
    Completion handler for reading from the stream.

    Resets the COM buffer so that it points at the correct position in the
    stream, and emits readyRead so that the sink pulls more data.
*/
HRESULT AudioSource::Invoke(IAsyncOperationWithProgress<IBuffer*, unsigned int> *read,
                            AsyncStatus status)
{
    if (status != AsyncStatus::Completed)
        return E_FAIL;

    // there should never be multiple read operations
    Q_ASSERT(readOperation.Get() == read);

    HRESULT hr = read->GetResults(&m_buffer);
    RETURN_HR_IF_FAILED("Could not access buffer.");
    m_bufferOffset = 0;

    ComPtr<IAsyncInfo> asyncInfo;
    if (HRESULT hr = readOperation.As(&asyncInfo); SUCCEEDED(hr))
        asyncInfo->Close();
    readOperation.Reset();

    // inform the sink that more data has arrived
    if (m_pause == NoPause && bytesInBuffer())
        emit readyRead();

    return S_OK;
}

qint64 AudioSource::bytesInBuffer() const
{
    if (!m_buffer)
        return 0;

    UINT32 bytes;
    m_buffer->get_Length(&bytes);
    return bytes - m_bufferOffset;
}

/*
    Starts an asynchronous read operation. There can only be one such
    operation pending at any given time, so fetchMore must only be called
    if the buffer provided by a previous operation is exhaused.
*/
bool AudioSource::fetchMore()
{
    Q_ASSERT(m_buffer);

    if (readOperation) {
        qWarning () << "Fetching more while a read operation is already pending";
        return false;
    }

    UINT32 capacity;
    m_buffer->get_Capacity(&capacity);
    InputStreamOptions streamOptions = {};
    HRESULT hr = inputStream->ReadAsync(m_buffer.Get(), capacity, streamOptions, readOperation.GetAddressOf());
    if (!SUCCEEDED(hr))
        return false;

    readOperation->put_Completed(this);
    return true;
}

QT_END_NAMESPACE
