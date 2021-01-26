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

#ifndef QTEXTTOSPEECHENGINE_OSX_H
#define QTEXTTOSPEECHENGINE_OSX_H

#include <QtCore/qobject.h>
#include <QtCore/qvector.h>
#include <QtCore/qstring.h>
#include <QtCore/qlocale.h>
#include <QtTextToSpeech/qtexttospeechengine.h>
#include <QtTextToSpeech/qvoice.h>

Q_FORWARD_DECLARE_OBJC_CLASS(QT_MANGLE_NAMESPACE(StateDelegate));
Q_FORWARD_DECLARE_OBJC_CLASS(NSSpeechSynthesizer);
Q_FORWARD_DECLARE_OBJC_CLASS(NSString);

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineOsx : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineOsx(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineOsx();

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

    void speechStopped(bool);

private:
    void updateVoices();
    bool isSpeaking() const;
    bool isPaused() const;

    QTextToSpeech::State m_state;
//    QVoice m_currentVoice;

    QVoice voiceForNSVoice(NSString *voiceString) const;
    NSSpeechSynthesizer *speechSynthesizer;
    QT_MANGLE_NAMESPACE(StateDelegate) *stateDelegate;
    QVector<QLocale> m_locales;
    QMultiMap<QString, QVoice> m_voices;
};

QT_END_NAMESPACE

#endif
