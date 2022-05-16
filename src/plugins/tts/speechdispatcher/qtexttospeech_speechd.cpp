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


#include "qtexttospeech_speechd.h"

#include <qdebug.h>
#include <libspeechd.h>

#if LIBSPEECHD_MAJOR_VERSION > 0 || LIBSPEECHD_MINOR_VERSION >= 9
  #define HAVE_SPD_090
#endif

QT_BEGIN_NAMESPACE

typedef QList<QTextToSpeechEngineSpeechd*> QTextToSpeechSpeechDispatcherBackendList;
Q_GLOBAL_STATIC(QTextToSpeechSpeechDispatcherBackendList, backends)

void speech_finished_callback(size_t msg_id, size_t client_id, SPDNotificationType state);

QLocale QTextToSpeechEngineSpeechd::localeForVoice(SPDVoice *voice) const
{
    QString lang_var = QString::fromLatin1(voice->language);
    if (qstrcmp(voice->variant, "none") != 0) {
        QString var = QString::fromLatin1(voice->variant);
        lang_var += QLatin1Char('_') + var;
    }
    return QLocale(lang_var);
}

QTextToSpeechEngineSpeechd::QTextToSpeechEngineSpeechd(const QVariantMap &, QObject *)
    : speechDispatcher(nullptr)
{
    backends->append(this);
    connectToSpeechDispatcher();
}

QTextToSpeechEngineSpeechd::~QTextToSpeechEngineSpeechd()
{
    if (speechDispatcher) {
        if ((m_state != QTextToSpeech::Error) && (m_state != QTextToSpeech::Ready))
            spd_cancel_all(speechDispatcher);
        spd_close(speechDispatcher);
    }
    backends->removeAll(this);
}

bool QTextToSpeechEngineSpeechd::connectToSpeechDispatcher()
{
    if (speechDispatcher)
        return true;

    speechDispatcher = spd_open("QTextToSpeech", "main", nullptr, SPD_MODE_THREADED);
    if (speechDispatcher) {
        speechDispatcher->callback_begin = speech_finished_callback;
        spd_set_notification_on(speechDispatcher, SPD_BEGIN);
        speechDispatcher->callback_end = speech_finished_callback;
        spd_set_notification_on(speechDispatcher, SPD_END);
        speechDispatcher->callback_cancel = speech_finished_callback;
        spd_set_notification_on(speechDispatcher, SPD_CANCEL);
        speechDispatcher->callback_resume = speech_finished_callback;
        spd_set_notification_on(speechDispatcher, SPD_RESUME);
        speechDispatcher->callback_pause = speech_finished_callback;
        spd_set_notification_on(speechDispatcher, SPD_PAUSE);

        QStringList availableModules;
        char **modules = spd_list_modules(speechDispatcher);
        int i = 0;
        while (modules && modules[i]) {
            availableModules.append(QString::fromUtf8(modules[i]));
            ++i;
        }

        if (availableModules.length() == 0) {
            m_errorReason = QTextToSpeech::ErrorReason::Configuration;
            m_errorString = tr("Found no modules in speech-dispatcher.");
        } else {
            m_state = QTextToSpeech::Ready;
            m_errorReason = QTextToSpeech::ErrorReason::NoError;
        }

        updateVoices();
        // Set the default locale (which is usually the system locale), and fall back
        // to a locale that has the same language if that fails. That might then still fail,
        // in which case there won't be a valid voice.
        if (!setLocale(QLocale()) && !setLocale(QLocale().language())) {
            m_errorReason = QTextToSpeech::ErrorReason::Configuration;
            m_errorString = tr("Failed to set default locale and voice");
        }
        return true;
    }

    m_state = QTextToSpeech::Error;
    m_errorReason = QTextToSpeech::ErrorReason::Initialization;
    m_errorString = tr("Connection to speech-dispatcher failed");

    return false;
}

// hack to get state notifications
void QTextToSpeechEngineSpeechd::spdStateChanged(SPDNotificationType state)
{
    QTextToSpeech::State s = QTextToSpeech::Error;
    if (state == SPD_EVENT_PAUSE)
        s = QTextToSpeech::Paused;
    else if ((state == SPD_EVENT_BEGIN) || (state == SPD_EVENT_RESUME))
        s = QTextToSpeech::Speaking;
    else if ((state == SPD_EVENT_CANCEL) || (state == SPD_EVENT_END))
        s = QTextToSpeech::Ready;

    if (m_state != s) {
        m_state = s;
        emit stateChanged(m_state);
    }
}

void QTextToSpeechEngineSpeechd::say(const QString &text)
{
    if (text.isEmpty() || !connectToSpeechDispatcher())
        return;

    if (m_state != QTextToSpeech::Ready)
        stop();
    spd_say(speechDispatcher, SPD_MESSAGE, text.toUtf8().constData());
}

void QTextToSpeechEngineSpeechd::stop()
{
    if (!connectToSpeechDispatcher())
        return;

    if (m_state == QTextToSpeech::Paused)
        spd_resume_all(speechDispatcher);
    spd_cancel_all(speechDispatcher);
}

void QTextToSpeechEngineSpeechd::pause()
{
    if (!connectToSpeechDispatcher())
        return;

    if (m_state == QTextToSpeech::Speaking) {
        spd_pause_all(speechDispatcher);
    }
}

void QTextToSpeechEngineSpeechd::resume()
{
    if (!connectToSpeechDispatcher())
        return;

    if (m_state == QTextToSpeech::Paused) {
        spd_resume_all(speechDispatcher);
    }
}

bool QTextToSpeechEngineSpeechd::setPitch(double pitch)
{
    if (!connectToSpeechDispatcher())
        return false;

    int result = spd_set_voice_pitch(speechDispatcher, static_cast<int>(pitch * 100));
    if (result == 0)
        return true;
    return false;
}

double QTextToSpeechEngineSpeechd::pitch() const
{
    double pitch = 0.0;
#ifdef HAVE_SPD_090
    if (speechDispatcher != 0) {
        int result = spd_get_voice_pitch(speechDispatcher);
        pitch = result / 100.0;
    }
#endif
    return pitch;
}

bool QTextToSpeechEngineSpeechd::setRate(double rate)
{
    if (!connectToSpeechDispatcher())
        return false;

    int result = spd_set_voice_rate(speechDispatcher, static_cast<int>(rate * 100));
    return result == 0;
}

double QTextToSpeechEngineSpeechd::rate() const
{
    double rate = 0.0;
#ifdef HAVE_SPD_090
    if (speechDispatcher != 0) {
        int result = spd_get_voice_rate(speechDispatcher);
        rate = result / 100.0;
    }
#endif
    return rate;
}

bool QTextToSpeechEngineSpeechd::setVolume(double volume)
{
    if (!connectToSpeechDispatcher())
        return false;

    // convert from 0.0..1.0 to -100..100
    int result = spd_set_volume(speechDispatcher, (volume - 0.5) * 200);
    return result == 0;
}

double QTextToSpeechEngineSpeechd::volume() const
{
    double volume = 0.0;
#ifdef HAVE_SPD_090
    if (speechDispatcher != 0) {
        int result = spd_get_volume(speechDispatcher);
        // -100..100 to 0.0..1.0
        volume = (result + 100) / 200.0;
    }
#endif
    return volume;
}

bool QTextToSpeechEngineSpeechd::setLocale(const QLocale &locale)
{
    if (!connectToSpeechDispatcher())
        return false;

    int result = spd_set_language(speechDispatcher, locale.uiLanguages().at(0).toUtf8().data());
    if (result == 0) {
        const QVoice previousVoice = m_currentVoice;

        const QList<QVoice> voices = m_voices.values(locale);
        // QMultiHash returns the values in the reverse order
        if (voices.size() > 0 && setVoice(voices.last()))
            return true;

        // try to go back to the previous locale/voice
        setVoice(previousVoice);
    }
    return false;
}

QLocale QTextToSpeechEngineSpeechd::locale() const
{
    return m_currentVoice.locale();
}

bool QTextToSpeechEngineSpeechd::setVoice(const QVoice &voice)
{
    if (!connectToSpeechDispatcher())
        return false;

    const QByteArray moduleName = voiceData(voice).value<QByteArray>();
    const int result = spd_set_output_module(speechDispatcher, moduleName);
    if (result != 0)
        return false;
    const int result2 = spd_set_synthesis_voice(speechDispatcher, voice.name().toUtf8().data());
    if (result2 == 0) {
        m_currentVoice = voice;
        return true;
    }
    return false;
}

QVoice QTextToSpeechEngineSpeechd::voice() const
{
    return m_currentVoice;
}

QTextToSpeech::State QTextToSpeechEngineSpeechd::state() const
{
    return m_state;
}

QTextToSpeech::ErrorReason QTextToSpeechEngineSpeechd::errorReason() const
{
    return m_errorReason;
}
QString QTextToSpeechEngineSpeechd::errorString() const
{
    return m_errorString;
}

void QTextToSpeechEngineSpeechd::updateVoices()
{
    char **modules = spd_list_modules(speechDispatcher);
#ifdef HAVE_SPD_090
    char *original_module = spd_get_output_module(speechDispatcher);
#else
    char *original_module = modules[0];
#endif
    char **module = modules;
    while (module != nullptr && module[0] != nullptr) {
        spd_set_output_module(speechDispatcher, module[0]);

        SPDVoice **voices = spd_list_synthesis_voices(speechDispatcher);
        int i = 0;
        while (voices != nullptr && voices[i] != nullptr) {
            const QLocale locale = localeForVoice(voices[i]);
            const QVariant data = QVariant::fromValue<QByteArray>(module[0]);
            // speechd declares enums and APIs for gender and age, but the SPDVoice struct
            // carries no relevant information.
            const QVoice voice = createVoice(QString::fromUtf8(voices[i]->name), locale,
                                             QVoice::Unknown, QVoice::Other, data);
            m_voices.insert(locale, voice);
            ++i;
        }
        // free voices.
#ifdef HAVE_SPD_090
        free_spd_voices(voices);
#endif
        ++module;
    }

#ifdef HAVE_SPD_090
    // Also free modules.
    free_spd_modules(modules);
#endif
    // Set the output module back to what it was.
    spd_set_output_module(speechDispatcher, original_module);
#ifdef HAVE_SPD_090
    free(original_module);
#endif
}

QList<QLocale> QTextToSpeechEngineSpeechd::availableLocales() const
{
    return m_voices.uniqueKeys();
}

QList<QVoice> QTextToSpeechEngineSpeechd::availableVoices() const
{
    QList<QVoice> resultList = m_voices.values(m_currentVoice.locale());
    std::reverse(resultList.begin(), resultList.end());
    return resultList;
}

// We have no way of knowing our own client_id since speech-dispatcher seems to be incomplete
// (history functions are just stubs)
void speech_finished_callback(size_t /*msg_id*/, size_t /*client_id*/, SPDNotificationType state)
{
    for (QTextToSpeechEngineSpeechd *backend : qAsConst(*backends))
        backend->spdStateChanged(state);
}

QT_END_NAMESPACE
