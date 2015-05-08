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



#ifndef QTEXTTOSPEECH_P_H
#define QTEXTTOSPEECH_P_H

#include <qtexttospeech.h>
#include <QtCore/private/qobject_p.h>

#if defined(Q_OS_ANDROID)
#elif defined(Q_OS_OSX)
#elif defined(Q_OS_UNIX)
#include <speech-dispatcher/libspeechd.h>
#endif

class QTextToSpeechBackend;

class QTextToSpeech;
class QTextToSpeechPrivate : public QObjectPrivate
{
public:
    QTextToSpeechPrivate(QTextToSpeech *speech);

    virtual QVector<QLocale> availableLocales() const = 0;
    virtual QVector<QVoice> availableVoices() const = 0;

    virtual void say(const QString &text) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual double rate() const = 0;
    virtual void setRate(double rate) = 0;
    virtual double pitch() const = 0;
    virtual void setPitch(double pitch) = 0;
    virtual void setLocale(const QLocale &locale) = 0;
    virtual QLocale locale() const = 0;
    virtual int volume() const = 0;
    virtual void setVolume(int volume) = 0;
    virtual void setVoice(const QVoice &voiceName) = 0;
    virtual QVoice voice() const = 0;
    virtual QTextToSpeech::State state() const = 0;

protected:
    void emitStateChanged(QTextToSpeech::State s)
    {
        emit m_speech->stateChanged(s);
    }

    void emitRateChanged(double rate)
    {
        emit m_speech->rateChanged(rate);
    }

    void emitPitchChanged(double pitch)
    {
        emit m_speech->pitchChanged(pitch);
    }

    void emitVolumeChanged(int volume)
    {
        emit m_speech->volumeChanged(volume);
    }

    void emitLocaleChanged(const QLocale &locale)
    {
        emit m_speech->localeChanged(locale);
    }

    void emitVoiceChanged(const QVoice &voice)
    {
        emit m_speech->voiceChanged(voice);
    }

    QTextToSpeech *m_speech;
    QTextToSpeech::State m_state;
};

#endif
