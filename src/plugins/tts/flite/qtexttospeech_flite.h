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

#ifndef QTEXTTOSPEECHENGINE_FLITE_H
#define QTEXTTOSPEECHENGINE_FLITE_H

#include "qtexttospeech_flite_processor.h"
#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QVector>
#include <QtCore/QSharedPointer>

#include <flite/flite.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineFlite : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineFlite(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineFlite() override;

    // Plug-in API:
    QVector<QLocale> availableLocales() const override;
    QVector<QVoice> availableVoices() const override;
    void say(const QString &text) override;
    void stop() override;
    void pause() override;
    void resume() override;
    double rate() const override;
    bool setRate(double rate) override;
    double pitch() const override;
    bool setPitch(double pitch) override;
    QLocale locale() const override;
    bool setLocale(const QLocale &locale) override;
    double volume() const override;
    bool setVolume(double volume) override;
    QVoice voice() const override;
    bool setVoice(const QVoice &voice) override;
    QTextToSpeech::State state() const override;

    // Internal API:
    bool init(QString *errorString);

public slots:
    void onNotSpeaking(int statusCode);

private:
    QTextToSpeech::State m_state;
    QSharedPointer<QTextToSpeechProcessorFlite> m_processor;
    QLocale m_currentLocale;
    QVector<QLocale> m_locales;
    QVoice m_currentVoice;
    // Voices mapped by their locale name.
    QMultiMap<QString, QVoice> m_voices;
};

QT_END_NAMESPACE

#endif
