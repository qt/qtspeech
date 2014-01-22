/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include "qtexttospeech_p.h"

#include <qdebug.h>
#include <libspeechd.h>

QT_BEGIN_NAMESPACE

//class QTextToSpeechVoicePrivateUnix : public QTextToSpeechVoicePrivate
//{
//public:
//    QTextToSpeechVoicePrivateUnix()
//        : voice(0)
//    {}
//    QString name() const;

//    QLocale locale() const;

//    SPDVoice *voice;
//};

//QString QTextToSpeechVoicePrivateUnix::name() const
//{
//    if (voice)
//        return QString::fromUtf8(voice->name);
//    return QStringLiteral("default");
//}

//QTextToSpeechVoice::QTextToSpeechVoice()
//    : d(new QTextToSpeechVoicePrivateUnix)
//{}


QString dummyModule = QStringLiteral("dummy");

struct VoiceTypeMapping {
    const char *name;
    SPDVoiceType type;
};

static VoiceTypeMapping map[] = {
    { QT_TRANSLATE_NOOP("QTextToSpeech", "Male Voice 1"), SPD_MALE1 },
    { QT_TRANSLATE_NOOP("QTextToSpeech", "Male Voice 2"), SPD_MALE2 },
    { QT_TRANSLATE_NOOP("QTextToSpeech", "Male Voice 3"), SPD_MALE3 },
    { QT_TRANSLATE_NOOP("QTextToSpeech", "Female Voice 1"), SPD_FEMALE1 },
    { QT_TRANSLATE_NOOP("QTextToSpeech", "Female Voice 2"), SPD_FEMALE2},
    { QT_TRANSLATE_NOOP("QTextToSpeech", "Female Voice 3"), SPD_FEMALE3 },
    { QT_TRANSLATE_NOOP("QTextToSpeech", "Male Child Voice"), SPD_CHILD_MALE },
    { QT_TRANSLATE_NOOP("QTextToSpeech", "Female Child Voice"), SPD_CHILD_FEMALE },
};

class QTextToSpeechPrivateSpeechDispatcher : public QTextToSpeechPrivate
{
public:
    QTextToSpeechPrivateSpeechDispatcher(QTextToSpeech *speech);
    ~QTextToSpeechPrivateSpeechDispatcher();

    void say(const QString &text);
    void stop();
    void pause();
    void resume();

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(double volume);
    QTextToSpeech::State state() const;

    void spdStateChanged(SPDNotificationType state);
private:
    SPDConnection *speechDispatcher;
    // speech dispatcher doesn't allow us to sensibly delete the voices
    // so at least cache them
    mutable SPDVoice **m_voices;
//    QTextToSpeechVoice m_currentVoice;
//    QString m_voiceType;
};


QTextToSpeech::QTextToSpeech(QObject *parent)
    : QObject(*new QTextToSpeechPrivateSpeechDispatcher(this), parent)
{
    qRegisterMetaType<QTextToSpeech::State>();
}

//QLocale QTextToSpeechVoicePrivateUnix::locale() const
//{
//    QString lang_var = QString::fromLatin1(voice->language);
//    if (qstrcmp(voice->variant, "none") != 0) {
//        QString var = QString::fromLatin1(voice->variant);
//        lang_var += QLatin1Char('_') + var;
//    }
//    return QLocale(lang_var);
//}


class QTextToSpeechPrivate;
typedef QList<QTextToSpeechPrivateSpeechDispatcher*> QTextToSpeechSpeechDispatcherBackendList;
Q_GLOBAL_STATIC(QTextToSpeechSpeechDispatcherBackendList, backends)

void speech_finished_callback(size_t msg_id, size_t client_id, SPDNotificationType state);


QTextToSpeechPrivateSpeechDispatcher::QTextToSpeechPrivateSpeechDispatcher(QTextToSpeech *speech)
    : QTextToSpeechPrivate(speech), m_voices(0)
{
    backends->append(this);
    speechDispatcher = spd_open("QTextToSpeech", "main", 0, SPD_MODE_THREADED);
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
        qWarning() << "Found no modules in speech-dispatcher. No text to speech possible.";
    } else if (availableModules.length() == 1 && availableModules.at(0) == dummyModule) {
        qWarning() << "Found only the dummy module in speech-dispatcher. No text to speech possible. Install a tts module (e.g. espeak).";
    } else {
        m_state = QTextToSpeech::Ready;
    }
}

QTextToSpeechPrivateSpeechDispatcher::~QTextToSpeechPrivateSpeechDispatcher()
{
    if ((m_state != QTextToSpeech::BackendError) && (m_state != QTextToSpeech::Ready))
        spd_cancel_all(speechDispatcher);
    spd_close(speechDispatcher);
}

//QTextToSpeechVoice QTextToSpeechPrivate::currentVoice() const
//{
//    return m_currentVoice;
//}

//void QTextToSpeechPrivate::setVoice(const QTextToSpeechVoice &voice)
//{
//    m_currentVoice = voice;
//    const QTextToSpeechVoicePrivateUnix *voicePrivate = static_cast<const QTextToSpeechVoicePrivateUnix *>(voice.d.data());
//    int ret = spd_set_synthesis_voice(speechDispatcher, voicePrivate->voice->name);
//    qDebug() << "Set voice: " << ret;
//}

//QVector<QTextToSpeechVoice> QTextToSpeechPrivate::availableVoices() const
//{
//    QVector<QTextToSpeechVoice> voiceList;
//    if (!m_voices)
//        m_voices = spd_list_synthesis_voices(speechDispatcher);
//    int k = 0;
//    while (m_voices && m_voices[k]) {
//        QTextToSpeechVoice voice;
////        QTextToSpeechVoicePrivateUnix *voicePrivate = voice.d;
//        QTextToSpeechVoicePrivateUnix *voicePrivate = static_cast<QTextToSpeechVoicePrivateUnix *>(voice.d.data());
//        voicePrivate->voice = m_voices[k];
//        voiceList.append(voice);
//        ++k;
//    }
//    return voiceList;
//}

//QVector<QString> QTextToSpeechPrivate::availableVoiceTypes() const
//{
//    QVector<QString> voiceTypes;
//    for (uint i = 0; i < sizeof(map) / sizeof(VoiceTypeMapping); ++i)
//        voiceTypes.append(QTextToSpeech::tr(map[i].name));
//    return voiceTypes;
//}

//void QTextToSpeechPrivate::setVoiceType(const QString& type)
//{
//    for (uint i = 0; i < sizeof(map) / sizeof(VoiceTypeMapping); ++i) {
//        if (QTextToSpeech::tr(map[i].name) == type) {
//            spd_set_voice_type_all(speechDispatcher, map[i].type);
//            break;
//        }
//    }
//}

//QString QTextToSpeechPrivate::currentVoiceType() const
//{
//    return m_voiceType;
//}

// hack to get state notifications
void QTextToSpeechPrivateSpeechDispatcher::spdStateChanged(SPDNotificationType state)
{
    qDebug() << "SPD state changed: " << state;

    QTextToSpeech::State s = QTextToSpeech::BackendError;
    if (state == SPD_EVENT_PAUSE)
        s = QTextToSpeech::Paused;
    else if ((state == SPD_EVENT_BEGIN) || (state == SPD_EVENT_RESUME))
        s = QTextToSpeech::Speaking;
    else if ((state == SPD_EVENT_CANCEL) || (state == SPD_EVENT_END))
        s = QTextToSpeech::Ready;

    if (m_state != s) {
        m_state = s;
        emitStateChanged(m_state);
    }
}

QTextToSpeech::State QTextToSpeechPrivate::state() const
{
    return m_state;
}

void QTextToSpeechPrivateSpeechDispatcher::say(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_state != QTextToSpeech::Ready)
        stop();
    int ret = spd_say(speechDispatcher, SPD_MESSAGE, text.toUtf8().constData());
    qDebug() << "say: " << ret;
}

void QTextToSpeechPrivateSpeechDispatcher::stop()
{
    int r1 = -77;
    if (m_state == QTextToSpeech::Paused)
        r1 = spd_resume_all(speechDispatcher);
    int ret = spd_cancel_all(speechDispatcher);
    qDebug() << "stop: " << r1 << ", " << ret;
}

void QTextToSpeechPrivateSpeechDispatcher::pause()
{
    if (m_state == QTextToSpeech::Speaking) {
        int ret = spd_pause_all(speechDispatcher);
        qDebug() << "pause: " << ret;
    }
}

void QTextToSpeechPrivateSpeechDispatcher::resume()
{
    if (m_state == QTextToSpeech::Paused) {
        int ret = spd_resume_all(speechDispatcher);
        qDebug() << "resume: " << ret;
    }
}

void QTextToSpeechPrivateSpeechDispatcher::setPitch(double pitch)
{
    spd_set_voice_pitch(speechDispatcher, static_cast<int>(pitch * 100));
}

void QTextToSpeechPrivateSpeechDispatcher::setRate(double rate)
{
    spd_set_voice_rate(speechDispatcher, static_cast<int>(rate * 100));
}

void QTextToSpeechPrivateSpeechDispatcher::setVolume(double volume)
{
    spd_set_volume(speechDispatcher, static_cast<int>(volume * 100));
}

QTextToSpeech::State QTextToSpeechPrivateSpeechDispatcher::state() const
{
    return m_state;
}

// We have no way of knowing our own client_id since speech-dispatcher seems to be incomplete
// (history functions are just stubs)
void speech_finished_callback(size_t /*msg_id*/, size_t /*client_id*/, SPDNotificationType state)
{
    Q_FOREACH (QTextToSpeechPrivateSpeechDispatcher *backend, *backends)
        backend->spdStateChanged(state);
}

QT_END_NAMESPACE
