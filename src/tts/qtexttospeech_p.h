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


#ifndef QTEXTTOSPEECH_P_H
#define QTEXTTOSPEECH_P_H

#include <qtexttospeech.h>
#include <QtCore/private/qobject_p.h>

#if defined(Q_OS_ANDROID)
#elif defined(Q_OS_OSX)
#elif defined(Q_OS_UNIX)
#include <libspeechd.h>
#endif

class QTextToSpeechBackend;

//class QTextToSpeechVoicePrivate : public QSharedData
//{
//public:
//    QTextToSpeechVoicePrivate()
//    {}
//    virtual ~QTextToSpeechVoicePrivate() {}
//    virtual QString name() const = 0;
//    virtual QLocale locale() const = 0;
//};

class QTextToSpeech;
class QTextToSpeechPrivate : public QObjectPrivate
{
public:
    QTextToSpeechPrivate(QTextToSpeech *speech);

    // system specific initialization, sets up a backend

//    QTextToSpeechVoice currentVoice() const;
//    void setVoice(const QTextToSpeechVoice &voice);
//    QVector<QTextToSpeechVoice> availableVoices() const;

//    QVector<QString> availableVoiceTypes() const;
//    void setVoiceType(const QString& type);
//    QString currentVoiceType() const;

    virtual void say(const QString &text) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual void setRate(double rate) = 0;
    virtual void setPitch(double pitch) = 0;
    virtual void setVolume(double volume) = 0;
    virtual QTextToSpeech::State state() const = 0;

protected:
    void emitStateChanged(QTextToSpeech::State s)
    {
        emit m_speech->stateChanged(s);
    }
    QTextToSpeech *m_speech;
    QTextToSpeech::State m_state;
};

#endif
