// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:critical reason:data-parser

#include "qtexttospeech_winrt_audiosource.h"

#include <QtCore/qdebug.h>
#include <QtCore/qspan.h>
#include <QtCore/qtimer.h>

#include <QtCore/private/qfunctions_winrt_p.h>
#include <QtCore/private/qsystemerror_p.h>

#include <robuffer.h>
#include <winrt/base.h>
#include <QtCore/private/qfactorycacheregistration_p.h>
#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.media.core.h>
#include <windows.media.speechsynthesis.h>
#include <windows.storage.streams.h>

#include <wrl.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Media::Core;
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
    close();
}

/*
    Cancel any incomplete asynchronous operations, and stop the
    sink before closing the QIODevice.
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
    return m_bufferSpan.size_bytes() + QIODevice::bytesAvailable();
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

    if (m_bufferSpan.isEmpty() && atEnd())
        return -1;

    auto take = [](auto span, qsizetype n) {
        return span.first(std::min(span.size(), n));
    };

    auto drop = [](auto span, qsizetype n) {
        return span.subspan(std::min(span.size(), n));
    };

    QSpan source = take(m_bufferSpan, maxlen);

    switch (m_pause) {
    case NoPause:
        break;
    case PauseRequested: {
        Q_ASSERT(audioFormat.sampleFormat() == QAudioFormat::Int16);

        if (m_pauseRequestedAt) {
            if (m_pauseRequestedAt <= m_bytesRead) {
                // we missed the window, pause immediately
                source = {};
            } else if (m_pauseRequestedAt <= m_bytesRead + quint64(source.size())) {
                source = take(source, qsizetype(m_pauseRequestedAt - m_bytesRead));
            } else {
                break; // pause point is in a later chunk
            }
            m_pause = Paused;
            m_pauseRequestedAt = 0;
            break;
        }
        // If no byte to pause at is specified, look for silence in the current
        // chunk. We are dealing with artificially created sound, so we don't have
        // to find a large enough window with overall low energy; we can just
        // look for a series (e.g. 1/50th of a second) of samples with low
        // absolute values.
        const int silenceDuration = audioFormat.sampleRate() / 50;
        const auto samples = QSpan(reinterpret_cast<const short *>(source.data()),
                                   source.size() / sizeof(short));

        if (!m_pauseDetectionSilenceCount.has_value())
            m_pauseDetectionSilenceCount = 0;

        for (qsizetype i = 0; i < samples.size(); ++i) {
            if (qAbs(samples[i]) < 10)
                *m_pauseDetectionSilenceCount += 1;
            else {
                *m_pauseDetectionSilenceCount = 0;
                continue;
            }
            if (*m_pauseDetectionSilenceCount > silenceDuration) {
                // long enough silence found, only provide data until we are in the silence.
                // we will still try to play at least half of the silent part
                const qsizetype silentSamplesToPlay = silenceDuration / 2;
                source = take(source,
                              (i > silentSamplesToPlay ? i - silentSamplesToPlay : i)
                                      * sizeof(short));
                m_pause = Paused;
                m_pauseDetectionSilenceCount = std::nullopt;
                break;
            } else {
                *m_pauseDetectionSilenceCount = 0;
            }
        }
        // no silence found - stop after this chunk
        if (m_pause != Paused) {
            m_pauseDetectionSilenceCount = std::nullopt;
            m_pause = Paused;
        }
        break;
    }
    case Paused:
        // starve the sink so that it goes idle
        return 0;
    }

    if (source.isEmpty())
        return 0;

    memcpy(data, source.data(), source.size());
    m_bufferSpan = drop(m_bufferSpan, source.size());

    if (m_bufferSpan.isEmpty())
        fetchMore();

    m_bytesRead += source.size();
    return source.size();
}

bool AudioSource::atEnd() const
{
    // not done as long as QIODevice's buffer is not empty
    if (!QIODevice::atEnd() && QIODevice::bytesAvailable())
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
    if (randomAccessStream) {
        randomAccessStream->get_Size(&ioSize);
        randomAccessStream->get_Position(&ioPos);
    }
    return ioPos >= ioSize;
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
    inputStream.As(&randomAccessStream);

    ComPtr<IBufferFactory> bufferFactory;
    hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(),
                                IID_PPV_ARGS(&bufferFactory));
    RETURN_HR_IF_FAILED("Could not create buffer factory.");
    // use the same buffer size as default read chunk size
    bufferFactory->Create(16384, m_buffer.GetAddressOf());

    hr = m_buffer->QueryInterface(IID_PPV_ARGS(&bufferByteAccess));
    RETURN_HR_IF_FAILED("Could not access buffer.");

    populateBoundaries();

    // release our reference to the speech stream operation
    if (asyncInfo)
        asyncInfo->Close();
    synthOperation.Reset();

    // we are buffered, but we don't want QIODevice to buffer as well
    open(QIODevice::ReadOnly|QIODevice::Unbuffered);
    emit streamReady(audioFormat);
    fetchMore();
    return S_OK;
}

/*
*/
void AudioSource::populateBoundaries()
{
    ComPtr<ITimedMetadataTrackProvider> metaData;
    if (!SUCCEEDED(inputStream.As(&metaData)))
        return;

    ComPtr<IVectorView<TimedMetadataTrack*>> metaDataTracks;
    metaData->get_TimedMetadataTracks(&metaDataTracks);
    quint32 trackCount = 0;
    metaDataTracks->get_Size(&trackCount);

    for (quint32 i = 0; i < trackCount; ++i) {
        ComPtr<ITimedMetadataTrack> metaDataTrack;
        HRESULT hr = metaDataTracks->GetAt(i, &metaDataTrack);
        Q_ASSERT_SUCCEEDED(hr);

        const auto boundaryType = [metaDataTrack]{
            ComPtr<IMediaTrack> mediaTrack;
            HRESULT hr = metaDataTrack.As(&mediaTrack);
            Q_ASSERT_SUCCEEDED(hr);

            if (HString hstr; SUCCEEDED(mediaTrack->get_Id(hstr.GetAddressOf()))) {
                const QString trackName = QString::fromWCharArray(hstr.GetRawBuffer(0));
                if (trackName == QStringLiteral("SpeechWord"))
                    return Boundary::Word;
                if (trackName == QStringLiteral("SpeechSentence"))
                    return Boundary::Sentence;
            }
            return Boundary::Unknown;
        }();
        if (boundaryType == Boundary::Unknown)
            continue;

        ComPtr<IVectorView<IMediaCue*>> cues;
        if (!SUCCEEDED(metaDataTrack->get_Cues(&cues)))
            continue;

        quint32 cueCount = 0;
        cues->get_Size(&cueCount);
        boundaries.reserve(boundaries.size() + cueCount);

        for (quint32 j = 0; j < cueCount; ++j) {
            ComPtr<IMediaCue> cue;
            hr = cues->GetAt(j, &cue);
            Q_ASSERT_SUCCEEDED(hr);

            ComPtr<ISpeechCue> speechCue;
            if (!SUCCEEDED(cue.As(&speechCue))) {
                qWarning("Invalid cue");
                break;
            }

            QString text;
            if (HString hstr; SUCCEEDED(speechCue->get_Text(hstr.GetAddressOf())))
                text = QString::fromWCharArray(hstr.GetRawBuffer(0));
            int startIndex = -1;
            if (IReference<int> *refInt; SUCCEEDED(speechCue->get_StartPositionInInput(&refInt)))
                refInt->get_Value(&startIndex);
            int endIndex = -1;
            if (IReference<int> *refInt; SUCCEEDED(speechCue->get_EndPositionInInput(&refInt)))
                refInt->get_Value(&endIndex);

            // A time period expressed in 100-nanosecond units. the Duration property is always 0 for speech.
            TimeSpan startTime = {};
            cue->get_StartTime(&startTime);
            const qint64 usec = startTime.Duration / 10; // QAudioSink APIs operate on microseconds
            boundaries.append(Boundary{boundaryType, text, startIndex, endIndex, usec});
        }
    }
    std::sort(boundaries.begin(), boundaries.end());
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

    m_bufferSpan = [&] {
        byte *ptr = nullptr;
        bufferByteAccess->Buffer(&ptr);
        UINT32 length = 0;
        m_buffer->get_Length(&length);
        return QSpan(reinterpret_cast<const char *>(ptr), qsizetype(length));
    }();

    // Skip the RIFF header in the first buffer to prevent an audible click.
    constexpr qsizetype WaveHeaderLength = 44;
    if (!m_headerConsumed) {
        Q_ASSERT(m_bytesRead == 0);
        Q_ASSERT(m_bufferSpan.size() >= WaveHeaderLength);
        Q_ASSERT(QByteArrayView(m_bufferSpan).startsWith("RIFF"));
        m_bufferSpan = m_bufferSpan.subspan(WaveHeaderLength);
        m_headerConsumed = true;
    }

    ComPtr<IAsyncInfo> asyncInfo;
    if (HRESULT hr = readOperation.As(&asyncInfo); SUCCEEDED(hr))
        asyncInfo->Close();
    readOperation.Reset();

    // inform the sink that more data has arrived
    if (m_pause == NoPause && !m_bufferSpan.isEmpty())
        emit readyRead();

    return S_OK;
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
