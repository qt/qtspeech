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



#ifndef QSPEECH_H
#define QSPEECH_H

#include <QtSpeech/qspeech_global.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>
#include <QtCore/QSharedDataPointer>
#include <QtCore/qlocale.h>

QT_BEGIN_NAMESPACE


//class QSpeechVoicePrivate;
//class QSPEECH_EXPORT QSpeechVoice
//{
//public:
//    ~QSpeechVoice();
//    QString name() const;
//    QLocale locale() const;

//    QSpeechVoice();
//    QSpeechVoice(const QSpeechVoice &other);
//private:
//    QSharedDataPointer<QSpeechVoicePrivate> d;
//    friend class QSpeechVoicePrivate;
//    friend class QSpeechPrivate;
//};
//Q_DECLARE_TYPEINFO(QSpeechVoice, Q_MOVABLE_TYPE);

class QSpeechPrivate;
class QSPEECH_EXPORT QSpeech : public QObject
{
    Q_OBJECT
    Q_ENUMS(QSpeech::State)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_DECLARE_PRIVATE(QSpeech)
public:
    enum State {
        Ready,
        Speaking,
        Paused,
        BackendError
    };

    QSpeech(QObject *parent = 0);
    State state() const;

public Q_SLOTS:
    void say(const QString &text);
    void stop();
    void pause();
    void resume();

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(double volume);

//    QSpeechVoice currentVoice() const;
//    void setVoice(const QSpeechVoice &locale);
//    QVector<QSpeechVoice> availableVoices() const;

    // FIXME is qstring really good enough here?
    // also it uses localized strings... uhm???
//    QVector<QString> availableVoiceTypes() const;
//    void setVoiceType(const QString &type);
//    QString currentVoiceType() const;

Q_SIGNALS:
    void stateChanged(QSpeech::State state);

private:
    Q_DISABLE_COPY(QSpeech)
};

Q_DECLARE_TYPEINFO(QSpeech::State, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(QSpeech::State)

QT_END_NAMESPACE

#endif
