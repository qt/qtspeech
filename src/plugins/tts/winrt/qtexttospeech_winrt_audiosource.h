/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Speech module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QTEXTTOSPEECHENGINE_WINRT_AUDIOSOURCE_H
#define QTEXTTOSPEECHENGINE_WINRT_AUDIOSOURCE_H

#include <QtCore/QIODevice>
#include <QtMultimedia/QAudioFormat>

#include <robuffer.h>
#include <winrt/base.h>
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
