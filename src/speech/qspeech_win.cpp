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


#include "qspeech_p.h"

#include <windows.h>
#include <sapi.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE


class QSpeechPrivateWindows : public QSpeechPrivate, public ISpNotifyCallback
{
public:
    QSpeechPrivateWindows(QSpeech *speech);
    ~QSpeechPrivateWindows();

    void say(const QString &text);
    void stop();
    void pause();
    void resume();

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(double volume);
    QSpeech::State state() const;

    bool isPaused() const { return m_pauseCount; }
    bool isSpeaking() const;

    HRESULT NotifyCallback(WPARAM /*wParam*/, LPARAM /*lParam*/);

private:
    ISpVoice *voice;
    double m_pitch;
    int m_pauseCount;
};


QSpeech::QSpeech(QObject *parent)
    : QObject(*new QSpeechPrivateWindows(this), parent)
{
    qRegisterMetaType<QSpeech::State>();
}

QSpeechPrivateWindows::QSpeechPrivateWindows(QSpeech *speech)
    : QSpeechPrivate(speech), m_pitch(0.0), m_pauseCount(0) //, m_voices(0)
{
    if (FAILED(::CoInitialize(NULL)))
        qWarning() << "Init of COM failed";

    HRESULT hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&voice);
    if (!SUCCEEDED(hr))
        qWarning() << "Could not init voice";

    voice->SetInterest(SPFEI_ALL_TTS_EVENTS, SPFEI_ALL_TTS_EVENTS);
    voice->SetNotifyCallbackInterface(this, 0, 0);
}

QSpeechPrivateWindows::~QSpeechPrivateWindows()
{
}

QSpeech::State QSpeechPrivate::state() const
{
    return m_state;
}

bool QSpeechPrivateWindows::isSpeaking() const
{
    SPVOICESTATUS eventStatus;
    voice->GetStatus(&eventStatus, NULL);
    return eventStatus.dwRunningState == SPRS_IS_SPEAKING;
}

void QSpeechPrivateWindows::say(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_state != QSpeech::Ready)
        stop();
    qDebug() << "say: " << text;

    if (m_pitch != 0.0) {
         // prepend <pitch middle = '-10'/>
        ;
    }

    std::wstring wtext = text.toStdWString();
    voice->Speak(wtext.data(), SPF_ASYNC, NULL);
}

void QSpeechPrivateWindows::stop()
{
    if (isPaused())
        resume();
    voice->Speak(NULL, SPF_PURGEBEFORESPEAK, 0);
}

void QSpeechPrivateWindows::pause()
{
    if (!isSpeaking())
        return;

    if (m_pauseCount == 0) {
        ++m_pauseCount;
        voice->Pause();
        m_state = QSpeech::Paused;
        emitStateChanged(m_state);
    }
}

void QSpeechPrivateWindows::resume()
{
    if (m_pauseCount > 0) {
        --m_pauseCount;
        voice->Resume();
        if (isSpeaking()) {
            m_state = QSpeech::Speaking;
        } else {
            m_state = QSpeech::Ready;
        }
        emitStateChanged(m_state);
    }
}

void QSpeechPrivateWindows::setPitch(double pitch)
{
    m_pitch = pitch;
}

void QSpeechPrivateWindows::setRate(double rate)
{
    // -10 to 10
    voice->SetRate(long(rate*10));
}

void QSpeechPrivateWindows::setVolume(double volume)
{
    // 0 to 100
    voice->SetVolume(USHORT((volume + 1) * 50));
}

QSpeech::State QSpeechPrivateWindows::state() const
{
    return m_state;
}

HRESULT QSpeechPrivateWindows::NotifyCallback(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    QSpeech::State newState = QSpeech::Ready;
    if (isPaused()) {
        newState = QSpeech::Paused;
    } else if (isSpeaking()) {
        newState = QSpeech::Speaking;
    } else {
        newState = QSpeech::Ready;
    }

    if (m_state != newState) {
        m_state = newState;
        emitStateChanged(newState);
    }

    return S_OK;
}

QT_END_NAMESPACE
