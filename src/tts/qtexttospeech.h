/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Speech module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
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
class Q_TEXTTOSPEECH_EXPORT QTextToSpeech : public QObject
{
    Q_OBJECT
    Q_ENUMS(QTextToSpeech::State)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(double volume READ volume WRITE setVolume NOTIFY volumeChanged)
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

    explicit QTextToSpeech(QObject *parent = nullptr);
    explicit QTextToSpeech(const QString &engine, QObject *parent = nullptr);
    State state() const;

    QVector<QLocale> availableLocales() const;
    QLocale locale() const;

    QVoice voice() const;
    QVector<QVoice> availableVoices() const;

    double rate() const;
    double pitch() const;
    double volume() const;

    static QStringList availableEngines();

public Q_SLOTS:
    void say(const QString &text);
    void stop();
    void pause();
    void resume();

    void setLocale(const QLocale &locale);

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(double volume);
    void setVoice(const QVoice &voice);

Q_SIGNALS:
    void stateChanged(QTextToSpeech::State state);
    void localeChanged(const QLocale &locale);
    void rateChanged(double rate);
    void pitchChanged(double pitch);
    void volumeChanged(int volume);  // ### Qt 6: remove this bad overload
    void volumeChanged(double volume);
    void voiceChanged(const QVoice &voice);

private:
    Q_DISABLE_COPY(QTextToSpeech)
};

Q_DECLARE_TYPEINFO(QTextToSpeech::State, Q_PRIMITIVE_TYPE);

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QTextToSpeech::State)

#endif
