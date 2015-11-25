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

#include "qtexttospeech_flite.h"

#include <QGlobalStatic>
#include <QIODevice>
#include <QAudioOutput>
#include <QMutex>
#include <QDebug>
#include <QSemaphore>
#include <QThread>

#include <flite/flite.h>

// en_US voice:
extern "C" cst_voice *register_cmu_us_kal16();
extern "C" void unregister_cmu_us_kal16(cst_voice *vox);

QT_BEGIN_NAMESPACE

// Class that handles global Flite initialization and
// creates the processor thread.
class FliteLoader
{
public:
    struct FliteVoiceInfo {
        cst_voice *vox;
        void (*unregister_func)(cst_voice *vox);
        QString name;
        QString localeName;
        QString gender;
        QString age;
    };
    FliteLoader()
    {
        flite_init();
        FliteVoiceInfo voice_enus = { register_cmu_us_kal16(), unregister_cmu_us_kal16, "kal16", "en_US", "male", "adult" };
        if (voice_enus.vox)
            m_voices.append(voice_enus);
        m_processor = new FliteProcessor();
        QObject::connect(m_processor, &QThread::finished, &QThread::deleteLater);
        m_processor->start();
    }
    ~FliteLoader()
    {
        foreach (const FliteLoader::FliteVoiceInfo &voice, m_voices)
            voice.unregister_func(voice.vox);
        m_processor->exit();
    }
    const QVector<FliteVoiceInfo> &voices() const
    {
        return m_voices;
    }
    FliteProcessor *processor() const
    {
        return m_processor;
    }
private:
    QVector<FliteVoiceInfo> m_voices;
    FliteProcessor *m_processor;
};

Q_GLOBAL_STATIC(FliteLoader, fliteLoader)

FliteProcessor::FliteProcessor():
    m_stop(true),
    m_idle(true),
    m_rate(0),
    m_pitch(0),
    m_volume(100)
{
}

FliteProcessor::~FliteProcessor()
{
}

void FliteProcessor::say(cst_voice *voice, const QString &text)
{
    if (isInterruptionRequested())
        return;
    QMutexLocker lock(&m_lock);
    m_stop = true; // Cancel any previous utterance
    m_idle = false;
    m_nextText = text;
    m_nextVoice = voice;
    setRateForVoice(m_nextVoice, m_rate);
    setPitchForVoice(m_nextVoice, m_pitch);
    m_speakSem.release();
}

void FliteProcessor::stop()
{
    QMutexLocker lock(&m_lock);
    m_stop = true;
    m_nextText.clear();
    m_nextVoice = 0;
    m_speakSem.release();
}

bool FliteProcessor::setRate(float rate)
{
    QMutexLocker lock(&m_lock);
    if (rate >= -1.0 && rate <= 1.0) {
        m_rate = rate;
        return true;
    }
    return false;
}

bool FliteProcessor::setPitch(float pitch)
{
    QMutexLocker lock(&m_lock);
    if (pitch >= -1.0 && pitch <= 1.0) {
        m_pitch = pitch;
        return true;
    }
    return false;
}

bool FliteProcessor::setVolume(int volume)
{
    QMutexLocker lock(&m_lock);
    if (volume >= 0 && volume <= 100) {
        m_volume = volume;
        if (m_audio)
            m_audio->setVolume(((qreal)m_volume) / 100.0);
        return true;
    }
    return false;
}

void FliteProcessor::exit()
{
    QThread::exit();
    requestInterruption();
    stop();
}

bool FliteProcessor::isIdle()
{
    QMutexLocker lock(&m_lock);
    return m_idle;
}

float FliteProcessor::rate()
{
    QMutexLocker lock(&m_lock);
    return m_rate;
}

float FliteProcessor::pitch()
{
    QMutexLocker lock(&m_lock);
    return m_pitch;
}

int FliteProcessor::volume()
{
    QMutexLocker lock(&m_lock);
    return m_volume;
}

void FliteProcessor::run()
{
    forever {
        m_lock.lock();
        if (!m_speakSem.tryAcquire()) {
            m_idle = true;
            m_lock.unlock();
            emit notSpeaking(); // Going idle
            m_speakSem.acquire();
            m_lock.lock();
        }
        if (isInterruptionRequested()) {
            m_lock.unlock();
            return;
        }
        m_stop = false;
        if (!m_nextText.isEmpty() && m_nextVoice) {
            cst_audio_streaming_info *asi;
            QString text = m_nextText;
            cst_voice *voice = m_nextVoice;
            m_nextText.clear();
            m_nextVoice = 0;
            m_lock.unlock();
            asi = new_audio_streaming_info();
            asi->asc = FliteProcessor::fliteAudioCb;
            asi->userdata = (void *)this;
            feat_set(voice->features, "streaming_info", audio_streaming_info_val(asi));
            flite_text_to_speech(text.toUtf8().constData(), voice, "none");
        } else {
            m_lock.unlock();
        }
    }
}

void FliteProcessor::setRateForVoice(cst_voice *voice, float rate)
{
    float stretch = 1.0;
    Q_ASSERT(rate >= -1.0 && rate <= 1.0);
    // Stretch multipliers taken from Speech Dispatcher
    if (rate < 0)
        stretch -= rate * 2;
    if (rate > 0)
        stretch -= rate * (100.0 / 175.0);
    feat_set_float(voice->features, "duration_stretch", stretch);
}

void FliteProcessor::setPitchForVoice(cst_voice *voice, float pitch)
{
    float f0;
    Q_ASSERT(pitch >= -1.0 && pitch <= 1.0);
    // Conversion taken from Speech Dispatcher
    f0 = (pitch * 80) + 100;
    feat_set_float(voice->features, "int_f0_target_mean", f0);
}

int FliteProcessor::audioOutput(const cst_wave *w, int start, int size,
                int last, cst_audio_streaming_info *asi)
{
    Q_UNUSED(asi);
    int ret = CST_AUDIO_STREAM_CONT;
    if (start == 0) {
        m_lock.lock();
        QAudioFormat format;
        format.setSampleRate(w->sample_rate);
        format.setChannelCount(w->num_channels);
        format.setSampleSize(16);
        format.setSampleType(QAudioFormat::SignedInt);
        format.setCodec("audio/pcm");
        m_audio = new QAudioOutput(format);
        m_audio->setVolume(((qreal)m_volume) / 100.0);
        m_audioBuffer = m_audio->start();
        m_lock.unlock();
    }
    int bytesToWrite = size * sizeof(short);
    int bytesWritten = 0;
    forever {
        m_lock.lock();
        if (m_stop || !m_audioBuffer
        || m_audio->state() == QAudio::StoppedState || isInterruptionRequested()) {
            m_lock.unlock();
            ret = CST_AUDIO_STREAM_STOP;
            break;
        }
        bytesWritten += m_audioBuffer->write((const char*)(&w->samples[start + bytesWritten/sizeof(short)]), bytesToWrite - bytesWritten);
        m_lock.unlock();
        if (bytesWritten >= bytesToWrite)
            break;
        QThread::msleep(200);
    }
    m_lock.lock();
    if (m_stop || last == 1) {
        if (m_stop) {
            m_audio->reset(); // Discard buffered audio
        } else {
            // TODO: Find a way to reliably check if all the audio has been written out before stopping
            m_audioBuffer->write(QByteArray(1024, 0));
            QThread::msleep(200);
            m_audio->stop();
        }
        delete m_audio;
        m_audio = 0;
        m_audioBuffer = 0;
    }
    m_lock.unlock();
    return ret;
}

int FliteProcessor::fliteAudioCb(const cst_wave *w, int start, int size,
    int last, cst_audio_streaming_info *asi)
{
    FliteProcessor *processor = static_cast<FliteProcessor *>(asi->userdata);
    if (processor)
        return processor->audioOutput(w, start, size, last, asi);
    return CST_AUDIO_STREAM_STOP;
}

QTextToSpeechEngineFlite::QTextToSpeechEngineFlite(
    const QVariantMap &parameters, QObject *parent) :
        QTextToSpeechEngine(parent),
        m_state(QTextToSpeech::Ready)
{
    Q_UNUSED(parameters);
}

QTextToSpeechEngineFlite::~QTextToSpeechEngineFlite()
{

}

QVector<QLocale> QTextToSpeechEngineFlite::availableLocales() const
{
    return m_locales;
}

QVector<QVoice> QTextToSpeechEngineFlite::availableVoices() const
{
    return m_voices.values(m_currentLocale.name()).toVector();
}

void QTextToSpeechEngineFlite::say(const QString &text)
{
    int id = QTextToSpeechEngine::voiceData(m_currentVoice).toInt();
    cst_voice *voiceData = fliteLoader()->voices()[id].vox;
    m_state = QTextToSpeech::Speaking;
    emit stateChanged(m_state);
    fliteLoader()->processor()->say(voiceData, text);
}

void QTextToSpeechEngineFlite::stop()
{
    fliteLoader()->processor()->stop();
    m_state = QTextToSpeech::Ready;
    emit stateChanged(m_state);
}

void QTextToSpeechEngineFlite::pause()
{
    // Not supported, just stop:
    stop();
}

void QTextToSpeechEngineFlite::resume()
{

}

double QTextToSpeechEngineFlite::rate() const
{
    return fliteLoader()->processor()->rate();
}

bool QTextToSpeechEngineFlite::setRate(double rate)
{
    return fliteLoader()->processor()->setRate(rate);
}

double QTextToSpeechEngineFlite::pitch() const
{
    return fliteLoader()->processor()->pitch();
}

bool QTextToSpeechEngineFlite::setPitch(double pitch)
{
    return fliteLoader()->processor()->setPitch(pitch);
}

QLocale QTextToSpeechEngineFlite::locale() const
{
    return m_currentLocale;
}

bool QTextToSpeechEngineFlite::setLocale(const QLocale &locale)
{
    bool localeFound = false;
    foreach (const QLocale &l, m_locales) {
        if (l.name() == locale.name()) {
            localeFound = true;
            break;
        }
    }
    if (!localeFound)
        return false;
    if (m_currentLocale.name() != locale.name()) {
        m_currentLocale = locale;
        m_currentVoice = availableVoices().at(0);
    }
    return true;
}

int QTextToSpeechEngineFlite::volume() const
{
    return fliteLoader()->processor()->volume();
}

bool QTextToSpeechEngineFlite::setVolume(int volume)
{
    return fliteLoader()->processor()->setVolume(volume);
}

QVoice QTextToSpeechEngineFlite::voice() const
{
    return m_currentVoice;
}

bool QTextToSpeechEngineFlite::setVoice(const QVoice &voice)
{
    foreach (const QVoice &availableVoice, availableVoices()) {
        if (QTextToSpeechEngine::voiceData(availableVoice) == QTextToSpeechEngine::voiceData(voice)) {
            m_currentVoice = voice;
            return true;
        }
    }
    return false;
}

QTextToSpeech::State QTextToSpeechEngineFlite::state() const
{
    return m_state;
}

bool QTextToSpeechEngineFlite::init(QString *errorString)
{
    int i = 0;
    QVector<FliteLoader::FliteVoiceInfo> voices = fliteLoader()->voices();
    foreach (const FliteLoader::FliteVoiceInfo &fliteVoice, voices) {
        QVoice::Age age = QVoice::Other;
        QVoice::Gender gender = QVoice::Unknown;
        QString name = fliteVoice.name;
        QLocale locale(fliteVoice.localeName);
        QVoice voice = QTextToSpeechEngine::createVoice(name, gender, age, QVariant(i));
        m_voices.insert(fliteVoice.localeName, voice);
        if (!m_locales.contains(locale))
            m_locales.append(locale);
        // Use the first available locale/voice as a fallback
        if (i == 0) {
            m_currentVoice = voice;
            m_currentLocale = locale;
        }
        i++;
    }
    // Attempt to switch to the system locale
    setLocale(QLocale::system());
    connect(fliteLoader()->processor(), &FliteProcessor::notSpeaking,
            this, &QTextToSpeechEngineFlite::onNotSpeaking);
    if (errorString)
        *errorString = QStringLiteral("");
    return true;
}

void QTextToSpeechEngineFlite::onNotSpeaking()
{
    if (m_state != QTextToSpeech::Ready && fliteLoader()->processor()->isIdle()) {
        m_state = QTextToSpeech::Ready;
        emit stateChanged(m_state);
    }
}

QT_END_NAMESPACE
