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

#include "qtexttospeech_p.h"
#include <jni.h>
#include <QtCore/private/qjni_p.h>
#include <QtCore/private/qjnihelpers_p.h>

QT_BEGIN_NAMESPACE

static jclass g_qtSpeechClass = 0;
class QTextToSpeechPrivateAndroid : public QTextToSpeechPrivate
{
public:
    QTextToSpeechPrivateAndroid(QTextToSpeech *speech);
    ~QTextToSpeechPrivateAndroid();

    QVector<QLocale> availableLocales() const Q_DECL_OVERRIDE;
    QVector<QVoice> availableVoices() const Q_DECL_OVERRIDE;

    void say(const QString &text) Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    void resume() Q_DECL_OVERRIDE;

    double rate() const Q_DECL_OVERRIDE;
    void setRate(double rate) Q_DECL_OVERRIDE;
    double pitch() const Q_DECL_OVERRIDE;
    void setPitch(double pitch) Q_DECL_OVERRIDE;
    int volume() const Q_DECL_OVERRIDE;
    void setVolume(int volume) Q_DECL_OVERRIDE;
    void setLocale(const QLocale &locale) Q_DECL_OVERRIDE;
    QLocale locale() const Q_DECL_OVERRIDE;
    void setVoice(const QVoice &voice) Q_DECL_OVERRIDE;
    QVoice voice() const Q_DECL_OVERRIDE;
    QTextToSpeech::State state() const Q_DECL_OVERRIDE;
    void setState(QTextToSpeech::State state);

private:
    QString m_text;
    QJNIObjectPrivate m_speech;
};

typedef QMap<jlong, QTextToSpeechPrivateAndroid *> TextToSpeechMap;
Q_GLOBAL_STATIC(TextToSpeechMap, textToSpeechMap)

static void notifyError(JNIEnv *env, jobject thiz, jlong id)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechPrivateAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    tts->setState(QTextToSpeech::BackendError);
}

static void notifyReady(JNIEnv *env, jobject thiz, jlong id)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechPrivateAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts || tts->state() == QTextToSpeech::Paused)
        return;

    tts->setState(QTextToSpeech::Ready);
}

static void notifySpeaking(JNIEnv *env, jobject thiz, jlong id)
{
    Q_UNUSED(env);
    Q_UNUSED(thiz);

    QTextToSpeechPrivateAndroid *const tts = (*textToSpeechMap)[id];
    if (!tts)
        return;

    tts->setState(QTextToSpeech::Speaking);
}

Q_DECL_EXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void */*reserved*/)
{
    typedef union {
        JNIEnv *nativeEnvironment;
        void *venv;
    } UnionJNIEnvToVoid;

    UnionJNIEnvToVoid uenv;
    uenv.venv = NULL;

    if (vm->GetEnv(&uenv.venv, JNI_VERSION_1_4) != JNI_OK)
        return JNI_ERR;

    JNIEnv *jniEnv = uenv.nativeEnvironment;
    jclass clazz = jniEnv->FindClass("org/qtproject/qt5/android/speech/QtTextToSpeech");

    static const JNINativeMethod methods[] = {
        {"notifyError", "(J)V", reinterpret_cast<void *>(notifyError)},
        {"notifyReady", "(J)V", reinterpret_cast<void *>(notifyReady)},
        {"notifySpeaking", "(J)V", reinterpret_cast<void *>(notifySpeaking)}
    };

    if (clazz) {
        g_qtSpeechClass = static_cast<jclass>(jniEnv->NewGlobalRef(clazz));
        if (jniEnv->RegisterNatives(g_qtSpeechClass,
                                    methods,
                                    sizeof(methods) / sizeof(methods[0])) != JNI_OK) {
            return JNI_ERR;
        }
    }

    return JNI_VERSION_1_4;
}

QTextToSpeechPrivateAndroid::QTextToSpeechPrivateAndroid(QTextToSpeech *speech)
    : QTextToSpeechPrivate(speech)
{
    Q_ASSERT(g_qtSpeechClass);

    const jlong id = reinterpret_cast<jlong>(this);
    m_speech = QJNIObjectPrivate::callStaticObjectMethod(g_qtSpeechClass,
                                                         "open",
                                                         "(Landroid/content/Context;J)Lorg/qtproject/qt5/android/speech/QtTextToSpeech;",
                                                         QtAndroidPrivate::activity(),
                                                         id);
    (*textToSpeechMap)[id] = this;
}

QTextToSpeechPrivateAndroid::~QTextToSpeechPrivateAndroid()
{
    textToSpeechMap->remove(reinterpret_cast<jlong>(this));
    m_speech.callMethod<void>("shutdown");
}

QTextToSpeech::QTextToSpeech(QObject *parent)
    : QObject(*new QTextToSpeechPrivateAndroid(this), parent)
{
    qRegisterMetaType<QTextToSpeech::State>();
}

void QTextToSpeechPrivateAndroid::say(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_state == QTextToSpeech::Speaking)
        stop();

    m_text = text;
    QJNIEnvironmentPrivate env;
    jstring jstr = env->NewString(reinterpret_cast<const jchar*>(text.constData()),
                                  text.length());
    m_speech.callMethod<void>("say", "(Ljava/lang/String;)V", jstr);
}

QTextToSpeech::State QTextToSpeechPrivateAndroid::state() const
{
    return m_state;
}

void QTextToSpeechPrivateAndroid::setState(QTextToSpeech::State state)
{
    if (m_state == state)
        return;

    m_state = state;
    emitStateChanged(m_state);
}

void QTextToSpeechPrivateAndroid::stop()
{
    if (m_state == QTextToSpeech::Ready)
        return;

    m_speech.callMethod<void>("stop", "()V");
}

void QTextToSpeechPrivateAndroid::pause()
{
    if (m_state == QTextToSpeech::Paused)
        return;

    stop();
    setState(QTextToSpeech::Paused);
}

void QTextToSpeechPrivateAndroid::resume()
{
    if (m_state != QTextToSpeech::Paused)
        return;

    say(m_text);
}

void QTextToSpeechPrivateAndroid::setPitch(double /*pitch*/)
{
}

void QTextToSpeechPrivateAndroid::setRate(double /*rate*/)
{
}

void QTextToSpeechPrivateAndroid::setVolume(int /*volume*/)
{
}

double QTextToSpeechPrivateAndroid::pitch() const
{
    return 0.0; // FIXME
}

double QTextToSpeechPrivateAndroid::rate() const
{
    return 0.0; // FIXME
}

int QTextToSpeechPrivateAndroid::volume() const
{
    return 100; // FIXME
}

QVector<QLocale> QTextToSpeechPrivateAndroid::availableLocales() const
{
    return QVector<QLocale>();
}

void QTextToSpeechPrivateAndroid::setLocale(const QLocale & /* locale */)
{
}

QLocale QTextToSpeechPrivateAndroid::locale() const
{
    return QLocale();
}

QVector<QVoice> QTextToSpeechPrivateAndroid::availableVoices() const
{
    return QVector<QVoice>();
}

void QTextToSpeechPrivateAndroid::setVoice(const QVoice & /* voice */)
{
}

QVoice QTextToSpeechPrivateAndroid::voice() const
{
    return QVoice();
}

QT_END_NAMESPACE
