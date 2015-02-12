/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSpeech module of the Qt Toolkit.
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

#include <windows.h>
#include <sapi.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE


class QTextToSpeechPrivateWindows : public QTextToSpeechPrivate, public ISpNotifyCallback
{
public:
    QTextToSpeechPrivateWindows(QTextToSpeech *speech);
    ~QTextToSpeechPrivateWindows();

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
    void setVoice(const QVoice &voiceName) Q_DECL_OVERRIDE;
    QVoice voice() const Q_DECL_OVERRIDE;
    QTextToSpeech::State state() const Q_DECL_OVERRIDE;

    bool isPaused() const { return m_pauseCount; }
    bool isSpeaking() const;

    HRESULT STDMETHODCALLTYPE NotifyCallback(WPARAM /*wParam*/, LPARAM /*lParam*/);

private:
    ISpVoice *m_voice;
    double m_pitch;
    int m_pauseCount;
};


QTextToSpeech::QTextToSpeech(QObject *parent)
    : QObject(*new QTextToSpeechPrivateWindows(this), parent)
{
    qRegisterMetaType<QTextToSpeech::State>();
}

QTextToSpeechPrivateWindows::QTextToSpeechPrivateWindows(QTextToSpeech *speech)
    : QTextToSpeechPrivate(speech), m_pitch(0.0), m_pauseCount(0) //, m_voices(0)
{
    if (FAILED(::CoInitialize(NULL)))
        qWarning() << "Init of COM failed";

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&m_voice);
    if (!SUCCEEDED(hr))
        qWarning() << "Could not init voice";

    m_voice->SetInterest(SPFEI_ALL_TTS_EVENTS, SPFEI_ALL_TTS_EVENTS);
    m_voice->SetNotifyCallbackInterface(this, 0, 0);
}

QTextToSpeechPrivateWindows::~QTextToSpeechPrivateWindows()
{
}

QTextToSpeech::State QTextToSpeechPrivate::state() const
{
    return m_state;
}

bool QTextToSpeechPrivateWindows::isSpeaking() const
{
    SPVOICESTATUS eventStatus;
    m_voice->GetStatus(&eventStatus, NULL);
    return eventStatus.dwRunningState == SPRS_IS_SPEAKING;
}

void QTextToSpeechPrivateWindows::say(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_state != QTextToSpeech::Ready)
        stop();
    qDebug() << "say: " << text;

    if (m_pitch != 0.0) {
         // prepend <pitch middle = '-10'/>
        ;
    }

    std::wstring wtext = text.toStdWString();
    m_voice->Speak(wtext.data(), SPF_ASYNC, NULL);
}

void QTextToSpeechPrivateWindows::stop()
{
    if (isPaused())
        resume();
    m_voice->Speak(NULL, SPF_PURGEBEFORESPEAK, 0);
}

void QTextToSpeechPrivateWindows::pause()
{
    if (!isSpeaking())
        return;

    if (m_pauseCount == 0) {
        ++m_pauseCount;
        m_voice->Pause();
        m_state = QTextToSpeech::Paused;
        emitStateChanged(m_state);
    }
}

void QTextToSpeechPrivateWindows::resume()
{
    if (m_pauseCount > 0) {
        --m_pauseCount;
        m_voice->Resume();
        if (isSpeaking()) {
            m_state = QTextToSpeech::Speaking;
        } else {
            m_state = QTextToSpeech::Ready;
        }
        emitStateChanged(m_state);
    }
}

void QTextToSpeechPrivateWindows::setPitch(double pitch)
{
    m_pitch = pitch;
}

double QTextToSpeechPrivateWindows::pitch() const
{
    return m_pitch;
}

void QTextToSpeechPrivateWindows::setRate(double rate)
{
    // -10 to 10
    m_voice->SetRate(long(rate*10));
}

double QTextToSpeechPrivateWindows::rate() const
{
    long rateValue;
    if (m_voice->GetRate(&rateValue) == S_OK)
        return rateValue / 10.0;
    return -1;
}

void QTextToSpeechPrivateWindows::setVolume(int volume)
{
    // 0 to 100
    m_voice->SetVolume(volume);
}

int QTextToSpeechPrivateWindows::volume() const
{
    USHORT baseVolume;
    if (m_voice->GetVolume(&baseVolume) == S_OK)
    {
        return baseVolume;
    }
    return -1;
}

QVector<QLocale> QTextToSpeechPrivateWindows::availableLocales() const
{
    // FIXME: Implement this method.
    return QVector<QLocale>();
}

void QTextToSpeechPrivateWindows::setLocale(const QLocale &locale)
{
    Q_UNUSED(locale)
    // FIXME: Implement this method.
}

QLocale QTextToSpeechPrivateWindows::locale() const
{
    // FIXME: Implement this method.
    return QLocale::system();
}

QVector<QVoice> QTextToSpeechPrivateWindows::availableVoices() const
{
    // FIXME: Implement this method.
    return QVector<QVoice>();
}

void QTextToSpeechPrivateWindows::setVoice(const QVoice &voice)
{
    Q_UNUSED(voice)
    // FIXME: Implement this method.
}

QVoice QTextToSpeechPrivateWindows::voice() const
{
    // FIXME: Implement this method.
    return QVoice();
}

QTextToSpeech::State QTextToSpeechPrivateWindows::state() const
{
    return m_state;
}

HRESULT QTextToSpeechPrivateWindows::NotifyCallback(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    QTextToSpeech::State newState = QTextToSpeech::Ready;
    if (isPaused()) {
        newState = QTextToSpeech::Paused;
    } else if (isSpeaking()) {
        newState = QTextToSpeech::Speaking;
    } else {
        newState = QTextToSpeech::Ready;
    }

    if (m_state != newState) {
        m_state = newState;
        emitStateChanged(newState);
    }

    return S_OK;
}

QT_END_NAMESPACE
