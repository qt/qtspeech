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

#ifndef QTEXTTOSPEECHENGINE_H
#define QTEXTTOSPEECHENGINE_H

#include <QtTextToSpeech/qtexttospeech.h>

#include <QtCore/QObject>
#include <QtCore/QLocale>
#include <QtCore/QDir>

QT_BEGIN_NAMESPACE

class Q_TEXTTOSPEECH_EXPORT QTextToSpeechEngine : public QObject
{
    Q_OBJECT

public:
    explicit QTextToSpeechEngine(QObject *parent = nullptr);
    ~QTextToSpeechEngine();

    virtual QVector<QLocale> availableLocales() const = 0;
    virtual QVector<QVoice> availableVoices() const = 0;

    virtual void say(const QString &text) = 0;
    virtual void stop() = 0;
    virtual void pause() = 0;
    virtual void resume() = 0;

    virtual double rate() const = 0;
    virtual bool setRate(double rate) = 0;
    virtual double pitch() const = 0;
    virtual bool setPitch(double pitch) = 0;
    virtual QLocale locale() const = 0;
    virtual bool setLocale(const QLocale &locale) = 0;
    virtual double volume() const = 0;
    virtual bool setVolume(double volume) = 0;
    virtual QVoice voice() const = 0;
    virtual bool setVoice(const QVoice &voice) = 0;
    virtual QTextToSpeech::State state() const = 0;

protected:
    static QVoice createVoice(const QString &name, QVoice::Gender gender, QVoice::Age age, const QVariant &data);
    static QVariant voiceData(const QVoice &voice);

Q_SIGNALS:
    void stateChanged(QTextToSpeech::State state);
};

QT_END_NAMESPACE

#endif
