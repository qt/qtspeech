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

#include "qtexttospeech_flite_processor.h"
#include "qtexttospeech_flite_plugin.h"

#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QMap>

#include <flite/flite.h>

QT_BEGIN_NAMESPACE

QTextToSpeechProcessorFlite::QTextToSpeechProcessorFlite()
    : m_state(QAudio::IdleState), m_error(QAudio::NoError)
{
    init();
}

QTextToSpeechProcessorFlite::~QTextToSpeechProcessorFlite()
{
    for (const VoiceInfo &voice : qExchange(m_voices, {}))
        voice.unregister_func(voice.vox);
}

const QList<QTextToSpeechProcessorFlite::VoiceInfo> &QTextToSpeechProcessorFlite::voices() const
{
    return m_voices;
}

int QTextToSpeechProcessorFlite::fliteOutputCb(const cst_wave *w, int start, int size,
    int last, cst_audio_streaming_info *asi)
{
    QTextToSpeechProcessorFlite *processor = static_cast<QTextToSpeechProcessorFlite *>(asi->userdata);
    if (processor)
        return processor->fliteOutput(w, start, size, last, asi);
    return CST_AUDIO_STREAM_STOP;
}

int QTextToSpeechProcessorFlite::fliteOutput(const cst_wave *w, int start, int size,
                int last, cst_audio_streaming_info *asi)
{
    Q_UNUSED(asi);
    if (start == 0 && !initAudio(w->sample_rate, w->num_channels))
        return CST_AUDIO_STREAM_STOP;

    int bytesToWrite = size * sizeof(short);
    QString errorString;
    if (!audioOutput((const char *)(&w->samples[start]), bytesToWrite, errorString)) {
        if (!errorString.isEmpty()) {
            stopTimer();
            setError(QAudio::IOError, errorString);
        }
        stop();
        return CST_AUDIO_STREAM_STOP;
    }

    if (last == 1)
        qCDebug(lcSpeechTtsFlite) << "last data chunk written";

    return CST_AUDIO_STREAM_CONT;
}

bool QTextToSpeechProcessorFlite::audioOutput(const char *data, qint64 dataSize, QString &errorString)
{
    // Send data
    if (!m_audioBuffer->write(data, dataSize)) {
        errorString = "audio streaming error";
        return false;
    }

    errorString.clear();

    // Stats for debugging
    ++numberChunks;
    totalBytes += dataSize;

    return true;
}

void QTextToSpeechProcessorFlite::processText(const QString &text, int voiceId, double pitch, double rate)
{
    qCDebug(lcSpeechTtsFlite) << "processText() begin";
    if (!checkVoice(voiceId))
        return;

    float secsToSpeak = -1;
    const VoiceInfo &voiceInfo = m_voices.at(voiceId);
    cst_voice *voice = voiceInfo.vox;
    cst_audio_streaming_info *asi = new_audio_streaming_info();
    asi->asc = QTextToSpeechProcessorFlite::fliteOutputCb;
    asi->userdata = (void *)this;
    feat_set(voice->features, "streaming_info", audio_streaming_info_val(asi));
    setRateForVoice(voice, rate);
    setPitchForVoice(voice, pitch);
    secsToSpeak = flite_text_to_speech(text.toUtf8().constData(), voice, "none");

    if (error() == QAudio::NoError) {
        startTimer(secsToSpeak * 1000);
        qCDebug(lcSpeechTtsFlite) << "processText() end" << secsToSpeak << "Seconds";
    }
}

void QTextToSpeechProcessorFlite::setRateForVoice(cst_voice *voice, float rate)
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

void QTextToSpeechProcessorFlite::setPitchForVoice(cst_voice *voice, float pitch)
{
    float f0;
    Q_ASSERT(pitch >= -1.0 && pitch <= 1.0);
    // Conversion taken from Speech Dispatcher
    f0 = (pitch * 80) + 100;
    feat_set_float(voice->features, "int_f0_target_mean", f0);
}

typedef cst_voice*(*registerFnType)();
typedef void(*unregisterFnType)(cst_voice *);

bool QTextToSpeechProcessorFlite::init()
{
    flite_init();

    const QLocale locale(QLocale::English, QLocale::UnitedStates);
    // ### FIXME: hardcode for now, the only voice files we know about are for en_US
    // We could source the language and perhaps the list of voices we want to load
    // (hardcoded below) from an environment variable.
    const QLatin1String langCode("us");
    const QLatin1String libPrefix("flite_cmu_%1_%2");
    const QLatin1String registerPrefix("register_cmu_%1_%2");
    const QLatin1String unregisterPrefix("unregister_cmu_%1_%2");

    for (const auto &voice : fliteAvailableVoices(langCode)) {
        QLibrary library(libPrefix.arg(langCode, voice));
        Q_ASSERT_X(library.load(), "library not found: ", qPrintable(library.fileName()));
        auto registerFn = reinterpret_cast<registerFnType>(library.resolve(
            registerPrefix.arg(langCode, voice).toLatin1().constData()));
        auto unregisterFn = reinterpret_cast<unregisterFnType>(library.resolve(
            unregisterPrefix.arg(langCode, voice).toLatin1().constData()));
        if (registerFn && unregisterFn) {
            const int id = m_voices.count();
            m_voices.append(VoiceInfo{
                id,
                registerFn(),
                unregisterFn,
                voice,
                locale.name(),
                QVoice::Male,
                QVoice::Adult
            });
        } else {
            library.unload();
        }
    }

    return !m_voices.isEmpty();
}

QStringList QTextToSpeechProcessorFlite::fliteAvailableVoices(const QString &langCode) const
{
    // Read statically linked voices
    QStringList voices;
    for (const cst_val *v = flite_voice_list; v; v = val_cdr(v)) {
        cst_voice *voice = val_voice(val_car(v));
        voices.append(voice->name);
    }

    // Read available libraries
    // TODO: make default library paths OS dependent
    QProcessEnvironment pe;
    QStringList ldPaths = pe.value("LD_LIBRARY_PATH", "/usr/lib64:/usr/local/lib64:/lib64").split(":", Qt::SkipEmptyParts);
    ldPaths.removeDuplicates();

    for (const auto &path : ldPaths) {
        QDir dir(path);
        if (!dir.isReadable() || dir.isEmpty())
            continue;
        dir.setNameFilters({QString("libflite_cmu_%1*.so").arg(langCode)});
        dir.setFilter(QDir::Files);
        QFileInfoList fileList = dir.entryInfoList();
        for (const auto &file : fileList) {
            const QString vox = file.fileName().mid(16, file.fileName().lastIndexOf(".") - 16);
            voices.append(vox);
        }
    }

    voices.removeDuplicates();
    return voices;
}

bool QTextToSpeechProcessorFlite::initAudio(double rate, int channelCount)
{
    m_format.setSampleRate(rate);
    m_format.setChannelCount(channelCount);
    if (!checkFormat(m_format, QMediaDevices::defaultAudioOutput()))
       return false;

    createSink();

    if (!m_audioBuffer) {
        setError(QAudio::OpenError, QLatin1String("Open error: No I/O device assigned."));
        return false;
    }

    m_audioSink->setVolume(m_volume);

    return true;
}

void QTextToSpeechProcessorFlite::deleteSink()
{
    if (m_audioSink) {
        m_audioSink->disconnect();
        delete m_audioSink;
        m_audioSink = nullptr;
        m_audioBuffer = nullptr;
    }
}

void QTextToSpeechProcessorFlite::createSink()
{
    // Create new sink if none exists or the format has changed
    if (!m_audioSink || (m_audioSink && m_audioSink->format() != m_format)) {
        // No signals while we create new sink with QIODevice
        const bool sigs = signalsBlocked();
        auto resetSignals = qScopeGuard([this, sigs](){ blockSignals(sigs); });
        blockSignals(true);
        deleteSink();
        m_audioSink = new QAudioSink(QMediaDevices::defaultAudioOutput(), m_format, this);
        connect(m_audioSink, &QAudioSink::stateChanged, this, &QTextToSpeechProcessorFlite::changeState);
        connect(QThread::currentThread(), &QThread::finished, m_audioSink, &QObject::deleteLater);
    }
    m_audioBuffer = m_audioSink->start();
}

// Wrapper for QAudioSink::stateChanged, bypassing early idle bug
void QTextToSpeechProcessorFlite::changeState(QAudio::State newState)
{
    if (m_state == newState)
        return;

    if (m_state == QAudio::ActiveState && newState == QAudio::IdleState && m_sinkTimer.isActive()) {
        // FIXME: Wait because GStreamer backend transitions to idle while playing back
        qCDebug(lcSpeechTtsFlite) << "early transition to idle despite of " << remainingTime() << "ms to talk!";
        return;
    }

    qCDebug(lcSpeechTtsFlite) << "State transition" << m_state << newState;
    m_state = newState;

    emit stateChanged(audioStateToTts(newState));
}

void QTextToSpeechProcessorFlite::setError(QAudio::Error err, const QString &errorString)
{
    if (m_error == err)
        return;

     m_error = err;
     if (err == QAudio::NoError)
         changeState(QAudio::IdleState);

     qCDebug(lcSpeechTtsFlite) << err << errorString;
     emit errorChanged(err, errorString);
     emit stateChanged(QTextToSpeech::BackendError);
}

QAudio::Error QTextToSpeechProcessorFlite::error() const
{
    return m_error;
}

constexpr QTextToSpeech::State QTextToSpeechProcessorFlite::audioStateToTts(QAudio::State AudioState)
{
    switch (AudioState) {
    case QAudio::ActiveState:
        return QTextToSpeech::Speaking;
    case QAudio::IdleState:
        return QTextToSpeech::Ready;
    case QAudio::SuspendedState:
        return QTextToSpeech::Paused;
    case QAudio::StoppedState:
        return QTextToSpeech::Ready;
    }
    Q_UNREACHABLE();
}

void QTextToSpeechProcessorFlite::deinitAudio()
{
    stopTimer();
    deleteSink();
}

// Check format/device and set corresponding error messages
bool QTextToSpeechProcessorFlite::checkFormat(const QAudioFormat &format, const QAudioDevice &device)
{
    QString formatString;
    QDebug(&formatString) << format;
    bool formatOK = true;

    // Format must be valid
    if (!format.isValid()) {
        formatOK = false;
        setError(QAudio::FatalError, QLatin1String("Invalid audio format: ")
                        + formatString);
    }

    // Device must exist
    if (device.isNull()) {
        formatOK = false;
        setError (QAudio::FatalError, QLatin1String("No audio device specified"));
    }

    // Device must support requested format
    if (!device.isFormatSupported(format)) {
        formatOK = false;
        setError(QAudio::FatalError, QLatin1String("Audio device does not support format: ")
                        + formatString);
    }

    if (!formatOK)
        emit stateChanged(QTextToSpeech::BackendError);

    return formatOK;
}

// Check voice validity
bool QTextToSpeechProcessorFlite::checkVoice(int voiceId)
{
    if (voiceId >= 0 && voiceId < m_voices.size())
        return true;

    qCDebug(lcSpeechTtsFlite) << "Invalid voiceId" << voiceId;
    setError(QAudio::FatalError, QString("Illegal voiceId %1").arg(voiceId));
    return false;;
}

// Wrap QAudioSink::state and compensate early idle bug
QAudio::State QTextToSpeechProcessorFlite::audioSinkState() const
{
    return (m_audioSink) ? m_state : QAudio::StoppedState;
}

// Stop current and cancel subsequent utterances
void QTextToSpeechProcessorFlite::stop()
{
    if (audioSinkState() != QAudio::ActiveState && audioSinkState() != QAudio::SuspendedState)
        return;

    // Block signals while we abort
    const bool sigs = signalsBlocked();
    blockSignals(true);
    deinitAudio();
    blockSignals(sigs);

    // Call manual state change as audio sink has been deleted
    changeState(QAudio::StoppedState);
}

void QTextToSpeechProcessorFlite::pause()
{
    if (!m_audioSink)
        return;

    if (audioSinkState() != QAudio::ActiveState)
        return;

    // FIXME
    // save remaining time
    if (m_sinkTimer.isActive())
        m_sinkTimerPausedAt = stopTimer();

    m_audioSink->suspend();
}

void QTextToSpeechProcessorFlite::resume()
{
    if (!m_audioSink)
        return;

    if (audioSinkState() != QAudio::SuspendedState)
        return;

    if (m_sinkTimerPausedAt > 0)
        startTimer(m_sinkTimerPausedAt);

    // FIXME
    // AudioSink emits stopped state after resume
    // ==> block signals and call manual state change
    const bool sigs = signalsBlocked();
    blockSignals(true);
    m_audioSink->blockSignals(true);
    m_audioSink->resume();
    m_audioSink->blockSignals(false);
    blockSignals(sigs);
    changeState(QAudio::ActiveState);
}

void QTextToSpeechProcessorFlite::startTimer(int msecs)
{
    // Stop first
    stopTimer();

    qCDebug(lcSpeechTtsFlite) << "Started timer with ms:" << msecs;

    // FIXME: adding 10% margin, otherwise the end will be cut off
    // 10% works well for different lengths, but I don't know why!
    m_sinkTimer.start(float(msecs * 1.1), this);
}

void QTextToSpeechProcessorFlite::timerEvent(QTimerEvent *event)
{
    if (event->type() != QEvent::Timer or signalsBlocked())
        return;

    stopTimer(TimeoutReason::TimeOut);
    changeState(QAudio::IdleState);
}

int QTextToSpeechProcessorFlite::stopTimer(TimeoutReason reason)
{
    if (!m_sinkTimer.isActive())
        return -1;

    // QBasicTimer remains active until timerEvent returns
    // => assume 0 remaining time if entered from timerEvent
    const int rt = (reason == Stop) ? remainingTime() : 0;

    // Block signals while we stop
    const bool sigs = signalsBlocked();
    blockSignals(true);
    m_sinkTimer.stop();
    blockSignals(sigs);
    m_sinkTimerPausedAt = 0;

    if (reason == Stop)
        qCDebug(lcSpeechTtsFlite) << "Stopped timer at" << rt;
    else
        qCDebug(lcSpeechTtsFlite) << "Timer elapsed";

    return rt;
}

inline int QTextToSpeechProcessorFlite::remainingTime() const
{
    if (!m_sinkTimer.isActive())
        return -1;

    // FIXME: remove 10% margin
    return float(QAbstractEventDispatcher::instance()->remainingTime(m_sinkTimer.timerId()) / 1.1);
}

void QTextToSpeechProcessorFlite::say(const QString &text, int voiceId, double pitch, double rate, double volume)
{
    clearError();

    if (text.isEmpty())
        return;

    if (!checkVoice(voiceId))
        return;

    // Set audio format
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);
    m_format.setChannelConfig(QAudioFormat::ChannelConfigMono);
    m_volume = volume;

    processText(text, voiceId, pitch, rate);
}

QT_END_NAMESPACE
