// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only

#ifndef QTEXTTOSPEECHENGINE_WINRT_AUDIOSOURCE_H
#define QTEXTTOSPEECHENGINE_WINRT_AUDIOSOURCE_H

#include <QtCore/QIODevice>
#include <QtMultimedia/QAudioFormat>

#include <robuffer.h>
#include <winrt/base.h>
#include <QtCore/private/qfactorycacheregistration_p.h>
#include <windows.foundation.h>
#include <windows.media.speechsynthesis.h>
#include <windows.storage.streams.h>

#include <wrl.h>

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Media::SpeechSynthesis;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;

QT_BEGIN_NAMESPACE

using StreamReadyHandler = IAsyncOperationCompletedHandler<SpeechSynthesisStream*>;
using BytesReadyHandler = IAsyncOperationWithProgressCompletedHandler<IBuffer*, UINT32>;

class AudioSource : public QIODevice,
                    public StreamReadyHandler,
                    public BytesReadyHandler
{
    Q_OBJECT
public:
    AudioSource(ComPtr<IAsyncOperation<SpeechSynthesisStream*>> synthOperation);

    bool isSequential() const override { return true; }

    void close() override;
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override { return 0; }

    bool atEnd() const override;
    qint64 bytesAvailable() const override;

    enum PauseState {
        NoPause,
        PauseRequested,
        Paused
    } m_pause = NoPause;

    void pause()
    {
        m_pause = PauseRequested;
    }

    void resume()
    {
        m_pause = NoPause;
        if (bytesAvailable())
            emit readyRead();
    }

    // IUnknown
    ULONG AddRef() { return ++ref; }
    ULONG Release() {
        if (!--ref) {
            delete this;
            return 0;
        }
        return ref;
    }
    HRESULT QueryInterface(REFIID riid, VOID **ppvInterface);

    // completion handler for synthesising the stream
    HRESULT Invoke(IAsyncOperation<SpeechSynthesisStream*> *operation,
                   AsyncStatus status) override;
    // completion handler for reading from the stream
    HRESULT Invoke(IAsyncOperationWithProgress<IBuffer*, unsigned int> *read,
                   AsyncStatus status) override;

Q_SIGNALS:
    void streamReady(const QAudioFormat &format);
    void errorInStream();

private:
    // lifetime is controlled via IUnknown reference counting, make sure
    // we don't destroy by accident, polymorphically, or via a QObject parent
    ~AudioSource() override;

    qint64 bytesInBuffer() const;
    bool fetchMore();

    QAudioFormat audioFormat;

    // The input stream that gives access to the synthesis stream. We keep the
    // async operation so that we can cancel it if we get destroyed prematurely.
    ComPtr<IAsyncOperation<SpeechSynthesisStream*>> synthOperation;
    ComPtr<IInputStream> inputStream;
    // the current ReadAsync operation that yields an IBuffer
    ComPtr<IAsyncOperationWithProgress<IBuffer*, UINT32>> readOperation;
    ComPtr<IBuffer> m_buffer;
    // access to the raw pcm bytes in the IBuffer; this took much reading of Windows header files...
    ComPtr<::Windows::Storage::Streams::IBufferByteAccess> bufferByteAccess;
    // The data in the IBuffer might be paritally consumed
    UINT32 m_bufferOffset = 0;

    ULONG ref = 1;
};

QT_END_NAMESPACE

#endif
