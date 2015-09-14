/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QSPEECHRECOGNITIONAUDIOBUFFER_P_H
#define QSPEECHRECOGNITIONAUDIOBUFFER_P_H

#include "qspeechrecognitionaudiobuffer.h"
#include <QtCore/QQueue>
#include <QtCore/QMultiMap>
#include <QtCore/QMutex>
#include <QtCore/QLoggingCategory>
#include <QtCore/private/qiodevice_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcSpeechAsr)

class QSpeechRecognitionAudioBufferPrivate : public QIODevicePrivate
{
public:
    struct AudioChunkPrivate
    {
        QSpeechRecognitionAudioBuffer::AudioChunk publicData;
        quint32 bufferSize;
    };
    QSpeechRecognitionAudioBufferPrivate();
    void clear(bool reset);
    AudioChunkPrivate *nextChunk();
    void releaseChunk(AudioChunkPrivate *chunk);
    qint64 appendToBuffer(const char *data, qint64 len);
    void updateAudioLevel(const char *data, qint64 len);
    qint64 recordingTime(qint64 streamOffset) const;

    mutable QMutex m_mutex;
    QQueue<AudioChunkPrivate *> m_buffer;
    QMultiMap<int, AudioChunkPrivate *> m_pool;
    int m_sampleRate;
    int m_sampleSize;
    int m_channelCount;
    unsigned int m_poolSize;
    unsigned int m_fifoSize;
    int m_poolLimit;
    int m_fifoLimit;
    qreal m_audioLevel;
    qreal m_previousAudioLevel;
    unsigned int m_previousAudioSamples;
    qint64 m_streamOffset;
    qint64 m_streamStartTime;
};

QT_END_NAMESPACE

#endif // QSPEECHRECOGNITIONAUDIOBUFFER_P_H
