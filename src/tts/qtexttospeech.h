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



#ifndef QTEXTTOSPEECH_H
#define QTEXTTOSPEECH_H

#include <QtTextToSpeech/qtexttospeech_global.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>
#include <QtCore/QSharedDataPointer>
#include <QtCore/qlocale.h>

#include <QtTextToSpeech/qvoice.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechPrivate;
class QTEXTTOSPEECH_EXPORT QTextToSpeech : public QObject
{
    Q_OBJECT
    Q_ENUMS(QTextToSpeech::State)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY volumeChanged)
    Q_PROPERTY(double rate READ rate WRITE setRate NOTIFY rateChanged)
    Q_PROPERTY(double pitch READ pitch WRITE setPitch NOTIFY pitchChanged)
    Q_PROPERTY(QLocale locale READ locale WRITE setLocale NOTIFY localeChanged)
    Q_PROPERTY(QVoice voice READ voice WRITE setVoice NOTIFY voiceChanged)
    Q_DECLARE_PRIVATE(QTextToSpeech)
public:
    enum State {
        Ready,
        Speaking,
        Paused,
        BackendError
    };

    explicit QTextToSpeech(QObject *parent = 0);
    State state() const;

    QVector<QLocale> availableLocales() const;
    QLocale locale() const;

    QVoice voice() const;
    QVector<QVoice> availableVoices() const;

    double rate() const;
    double pitch() const;
    int volume() const;

public Q_SLOTS:
    void say(const QString &text);
    void stop();
    void pause();
    void resume();

    void setLocale(const QLocale &locale);

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(int volume);
    void setVoice(const QVoice &voice);

Q_SIGNALS:
    void stateChanged(QTextToSpeech::State state);
    void localeChanged(const QLocale &locale);
    void rateChanged(double rate);
    void pitchChanged(double pitch);
    void volumeChanged(int volume);
    void voiceChanged(const QVoice &voice);

private:
    Q_DISABLE_COPY(QTextToSpeech)
};

Q_DECLARE_TYPEINFO(QTextToSpeech::State, Q_PRIMITIVE_TYPE);
Q_DECLARE_METATYPE(QTextToSpeech::State)

QT_END_NAMESPACE

#endif
