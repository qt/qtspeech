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

#include "qspeechrecognitionaudiobuffer.h"
#include "qspeechrecognitionaudiobuffer_p.h"

#include <QDateTime>
#include <QLoggingCategory>

#include <math.h>

QT_BEGIN_NAMESPACE

/*!
  \class QSpeechRecognitionAudioBuffer
  \inmodule QtSpeechRecognition
  \brief The QSpeechRecognitionAudioBuffer class can be used to implement audio buffering
  for speech recognition engines.

  \threadsafe

  The class attempts to re-use allocated audio buffers as much as possible, and implements
  some useful features related to audio stream handling, like audio level calculation
  and recording timeline estimation.
*/

/*! \fn void QSpeechRecognitionAudioBuffer::dataAvailable()

  Emitted when data has been added to the buffer.
*/

/*!
  Constructs the QSpeechRecognitionAudioBuffer instance.
*/
QSpeechRecognitionAudioBuffer::QSpeechRecognitionAudioBuffer(QObject *parent) :
    QIODevice(*new QSpeechRecognitionAudioBufferPrivate(), parent)
{
}

QSpeechRecognitionAudioBuffer::~QSpeechRecognitionAudioBuffer()
{
    clear(true);
}

qint64 QSpeechRecognitionAudioBuffer::readData(char *data, qint64 maxlen)
{
    Q_UNUSED(data)
    Q_UNUSED(maxlen)

    return 0;
}

qint64 QSpeechRecognitionAudioBuffer::writeData(const char *data, qint64 len)
{
    Q_D(QSpeechRecognitionAudioBuffer);
    d->m_mutex.lock();
    qint64 writtenLen = d->appendToBuffer(data, len);
    d->updateAudioLevel(data, len);
    d->m_mutex.unlock();
    if (writtenLen > 0)
        emit dataAvailable();
    return writtenLen;
}

/*!
  Sets the sample rate in the buffered audio to \a sampleRate.

  This value is only used for calculating features like audio level
  and recording time.

  \sa audioLevel()
  \sa recordingTime()

  Default: 16000.
*/
void QSpeechRecognitionAudioBuffer::setSampleRate(int sampleRate)
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    d->m_sampleRate = sampleRate;
}

/*!
  Sets the sample size in the buffered audio to \a sampleSize.
  The value is expressed in bits.

  This value is only used for calculating features like audio level
  and recording time.

  \sa audioLevel()
  \sa recordingTime()

  Default: 16.
*/
void QSpeechRecognitionAudioBuffer::setSampleSize(int sampleSize)
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    d->m_sampleSize = sampleSize;
}

/*!
  Sets the number of channels in the buffered audio to \a channelCount.

  This value is only used for calculating features like audio level
  and recording time.

  \sa audioLevel()
  \sa recordingTime()

  Default: 1.
*/
void QSpeechRecognitionAudioBuffer::setChannelCount(int channelCount)
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    d->m_channelCount = channelCount;
}

/*!
  Gets the current limit for the FIFO buffer size.
  A negative value means that no limit is set.

  \sa setFifoLimit()
*/
int QSpeechRecognitionAudioBuffer::fifoLimit() const
{
    Q_D(const QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    return d->m_fifoLimit;
}

/*!
  Sets a limit for the FIFO buffer size.

  When a new chunk of audio is about to be added in the buffer, the oldest chunks are dropped until
  the buffered audio size is at most the given number of \a bytes. The new chunk is not included in
  the calculation.

  If parameter \a bytes is less than 0, the FIFO size is only limited by the amount of memory
  that is available.

  Default: no limit.
*/
void QSpeechRecognitionAudioBuffer::setFifoLimit(int bytes)
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    d->m_fifoLimit = bytes;
}

/*!
  Gets the current limit for the free buffers in the pool.
  A negative value means that no limit is set.

  \sa setFreeLimit()
*/
int QSpeechRecognitionAudioBuffer::freeLimit() const
{
    Q_D(const QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    return d->m_poolLimit;
}

/*!
  Sets a limit for the total size of the free buffers in the pool.

  When a chunk of audio has been processed and is released with \l releaseChunk(),
  the chunk is usually added to the pool of free buffers. If the total size of the
  existing buffers in the pool exceed the limit given in parameter \a bytes, one of the
  buffers is deleted before adding the new buffer in the pool.

  The deleted buffer is selected by attempting to remove buffers that are less
  likely to be reused. As only one buffer is deleted at a time, the total size of the
  free buffers may not immediately get reduced to the given target.

  If parameter \a bytes is less than 0, the size of the free buffers is unlimited,
  and all the released buffers are returned to the pool.

  Default: no limit.
*/
void QSpeechRecognitionAudioBuffer::setFreeLimit(int bytes)
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    d->m_poolLimit = bytes;
}

/*!
  Gets the next chunk of audio in the FIFO buffer.

  After the chunk has been processed, it should be released with \l releaseChunk().
*/
QSpeechRecognitionAudioBuffer::AudioChunk *QSpeechRecognitionAudioBuffer::nextChunk()
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    QSpeechRecognitionAudioBufferPrivate::AudioChunkPrivate *privateChunk = d->nextChunk();
    if (privateChunk)
        return &privateChunk->publicData;
    return 0;
}

/*!
  Releases a chunk of audio.
  The data pointed by parameter \a chunk should not be accessed after calling this method.

  \sa setFreeLimit()
*/
void QSpeechRecognitionAudioBuffer::releaseChunk(AudioChunk *chunk)
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    QSpeechRecognitionAudioBufferPrivate::AudioChunkPrivate *privateChunk
        = reinterpret_cast<QSpeechRecognitionAudioBufferPrivate::AudioChunkPrivate *>(chunk);
    d->releaseChunk(privateChunk);
}

/*!
  Gets the estimated recording time for a given stream offset.

  The time is calculated from the parameter \a streamOffset (expressed
  in bytes). The method assumes zero audio input latency, in such way that
  no audio sample has been observed ahead of its expected time. Therefore,
  the values returned by this method change over time, even with the same
  input parameter.

  The values returned by this method are only valid if the stream of audio
  written to QSpeechRecognitionAudioBuffer has been continuous after the
  device was opened (with \l open()).

  The method returns a timestamp in milliseconds, based on the value
  returned by QDateTime::currentMSecsSinceEpoch().
*/
qint64 QSpeechRecognitionAudioBuffer::recordingTime(qint64 streamOffset) const
{
    Q_D(const QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    return d->recordingTime(streamOffset);
}

/*!
  Get current audio level.

  Returns the audio level between 0.0 and 1.0, calculated from the most recently added audio data.
 */
qreal QSpeechRecognitionAudioBuffer::audioLevel() const
{
    Q_D(const QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    return d->m_audioLevel;
}

/*!
  Returns \c true if the buffer is currently empty.
*/
bool QSpeechRecognitionAudioBuffer::isEmpty() const
{
    Q_D(const QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    return d->m_buffer.isEmpty();
}

/*!
  Discards all the audio in the buffer.

  If parameter \a reset is true, deletes all the allocated buffers.
  Otherwise the released buffers are added to the pool of free buffers.
*/
void QSpeechRecognitionAudioBuffer::clear(bool reset)
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    d->clear(reset);
}

/*!
  Closes the buffer device.
*/
void QSpeechRecognitionAudioBuffer::close()
{
    Q_D(QSpeechRecognitionAudioBuffer);
    QMutexLocker lock(&d->m_mutex);
    QIODevice::close();
    d->m_streamOffset = 0;
    d->m_streamStartTime = 0;
}

QSpeechRecognitionAudioBufferPrivate::QSpeechRecognitionAudioBufferPrivate() :
    m_sampleRate(16000),
    m_sampleSize(16),
    m_channelCount(1),
    m_poolSize(0),
    m_fifoSize(0),
    m_poolLimit(-1),
    m_fifoLimit(-1),
    m_audioLevel(0.0),
    m_previousAudioLevel(0.0),
    m_previousAudioSamples(0),
    m_streamOffset(0),
    m_streamStartTime(0)
{
}

void QSpeechRecognitionAudioBufferPrivate::clear(bool reset)
{
    QListIterator<AudioChunkPrivate *> i(m_buffer);
    if (reset) {
        while (i.hasNext())
            delete[] i.next()->publicData.data;
        m_buffer.clear();
        m_fifoSize = 0;
        QMapIterator<int, AudioChunkPrivate *> j(m_pool);
        while (j.hasNext())
            delete[] j.next().value()->publicData.data;
        m_pool.clear();
        m_poolSize = 0;
    } else {
        while (i.hasNext()) {
            AudioChunkPrivate *chunk = i.next();
            m_pool.insert(chunk->bufferSize, chunk);
            m_poolSize += chunk->bufferSize;
        }
        m_buffer.clear();
        m_fifoSize = 0;
    }
}

QSpeechRecognitionAudioBufferPrivate::AudioChunkPrivate *QSpeechRecognitionAudioBufferPrivate::nextChunk()
{
    if (!m_buffer.isEmpty())
        return m_buffer.dequeue();
    return 0;
}

void QSpeechRecognitionAudioBufferPrivate::releaseChunk(AudioChunkPrivate *chunk)
{
    // The chunk is removed from the FIFO size count when it's released by the user
    m_fifoSize -= chunk->publicData.audioSize;
    if (m_poolLimit >= 0 && m_poolSize >= (unsigned int)m_poolLimit) {
        QMap<int, AudioChunkPrivate *>::iterator firstChunk = m_pool.begin();
        QMap<int, AudioChunkPrivate *>::iterator lastChunk = m_pool.end();
        lastChunk--;
        quint32 lastSize = lastChunk.value()->bufferSize;
        if (lastSize > chunk->bufferSize) {
            AudioChunkPrivate *c = lastChunk.value();
            delete[] c->publicData.data;
            m_poolSize -= c->bufferSize;
            qCDebug(lcSpeechAsr) << "QSpeechRecognitionAudioBuffer: delete"
                                 << c->bufferSize
                                 << "bytes (largest), total buffers"
                                 << m_poolSize + m_fifoSize;
            m_pool.erase(lastChunk);
        } else {
            AudioChunkPrivate *c = firstChunk.value();
            delete[] c->publicData.data;
            m_poolSize -= c->bufferSize;
            qCDebug(lcSpeechAsr) << "QSpeechRecognitionAudioBuffer: delete"
                                 << c->bufferSize
                                 << "bytes (smallest), total buffers"
                                 << m_poolSize + m_fifoSize;
            m_pool.erase(firstChunk);
        }
    }
    m_pool.insert(chunk->bufferSize, chunk);
    m_poolSize += chunk->bufferSize;
}

qint64 QSpeechRecognitionAudioBufferPrivate::appendToBuffer(const char *data, qint64 len)
{
    AudioChunkPrivate *chunk = 0;
    if (m_fifoLimit >= 0) {
        while (!m_buffer.isEmpty() && m_fifoSize > (unsigned int)m_fifoLimit) {
            chunk = m_buffer.dequeue();
            qCDebug(lcSpeechAsr) << "QSpeechRecognitionAudioBuffer: discard"
                                 << chunk->publicData.audioSize
                                 << "bytes, fifo size" << m_fifoSize
                                 << "total buffers" << m_poolSize + m_fifoSize;
            releaseChunk(chunk);
        }
    }
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    qint64 streamOffset = m_streamOffset;
    m_streamOffset += len;
    if (streamOffset == 0) {
        m_streamStartTime = timestamp;
        m_streamStartTime = recordingTime(-len);
    } else {
        qint64 recTime = recordingTime(m_streamOffset);
        // Adjust the stream start time so that no audio sample has arrived "in the future".
        // This will set the time of each chunk according to the lowest-latency chunk that was
        // received, assuming that no audio frames were skipped before adding them here.
        if (recTime > timestamp) {
            m_streamStartTime -= (recTime - timestamp);
            qCDebug(lcSpeechAsr) << "QSpeechRecognitionAudioBuffer: adjust stream start time by"
                                 << (recTime - timestamp) << "ms";
        }
    }
    QMap<int, AudioChunkPrivate *>::iterator freeChunk = m_pool.lowerBound(len);
    if (freeChunk != m_pool.end()) {
        chunk = freeChunk.value();
        Q_ASSERT(chunk->bufferSize >= len);
        m_poolSize -= chunk->bufferSize;
        m_pool.erase(freeChunk);
        chunk->publicData.audioSize = (quint32)len;
        chunk->publicData.streamOffset = streamOffset;
        memcpy(chunk->publicData.data, data, (size_t)len);
        m_buffer.append(chunk);
        m_fifoSize += (unsigned int)len;
        return len;
    } else {
        chunk = new AudioChunkPrivate;
        chunk->publicData.data = new char[len];
        if (chunk->publicData.data) {
            chunk->bufferSize = (quint32)len;
            chunk->publicData.audioSize = (quint32)len;
            chunk->publicData.streamOffset = streamOffset;
            memcpy(chunk->publicData.data, data, (size_t)len);
            m_buffer.append(chunk);
            m_fifoSize += (unsigned int)len;
            qCDebug(lcSpeechAsr) << "QSpeechRecognitionAudioBuffer: allocate"
                                 << len << "bytes, fifo size" << m_fifoSize
                                 << "total buffers" << m_poolSize + m_fifoSize;
            return len;
        } else {
            delete chunk;
        }
    }
    return 0;
}

static qreal pcmToReal(qint16 pcm)
{
    static const quint16 PCMS16MaxAmplitude = 32768; // because minimum is -32768
    return qreal(pcm) / PCMS16MaxAmplitude;
}

void QSpeechRecognitionAudioBufferPrivate::updateAudioLevel(const char *data, qint64 len)
{
    static const unsigned int AUDIO_LEVEL_WINDOW = 256;
    if (m_sampleSize != 16
    || m_channelCount != 1) {
        return; // Not supported
    }
    qreal sum = 0.0;
    const char *const end = data + len;
    while (data < end) {
        const qint16 value = *reinterpret_cast<const qint16*>(data);
        const qreal fracValue = pcmToReal(value);
        sum += fracValue * fracValue;
        data += 2;
    }
    unsigned int numSamples = (unsigned int)len / 2;
    qreal rmsLevel = sqrt(sum / numSamples);
    // If the audio chunk is small, include the previous chunk in audio level calculation:
    if (numSamples < AUDIO_LEVEL_WINDOW) {
        unsigned int previousChunkWeight = AUDIO_LEVEL_WINDOW - numSamples;
        if (previousChunkWeight > m_previousAudioSamples)
            previousChunkWeight = m_previousAudioSamples;
        m_audioLevel = ((previousChunkWeight * m_previousAudioLevel) + (rmsLevel * numSamples))
                / (previousChunkWeight + numSamples);
    } else {
        m_audioLevel = rmsLevel;
    }
    m_audioLevel = qMin(qreal(1.0), m_audioLevel);
    m_previousAudioLevel = rmsLevel;
    m_previousAudioSamples = numSamples;
}

qint64 QSpeechRecognitionAudioBufferPrivate::recordingTime(qint64 streamOffset) const
{
    return m_streamStartTime + ((streamOffset * 1000)
                                / (m_sampleSize / 8 * m_channelCount * m_sampleRate));
}

QT_END_NAMESPACE
