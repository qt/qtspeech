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

#ifndef QTEXTTOSPEECHENGINE_WINRT_H
#define QTEXTTOSPEECHENGINE_WINRT_H

#include <QtTextToSpeech/qtexttospeechengine.h>
#include <QtTextToSpeech/qvoice.h>

#include <QtCore/QObject>
#include <QtCore/QVector>
#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QScopedPointer>
#include <QtCore/qt_windows.h>
#include <wrl.h>

namespace ABI {
    namespace Windows {
        namespace Media {
            namespace SpeechSynthesis {
                struct IVoiceInformation;
            }
        }
    }
}

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineWinRTPrivate;

class QTextToSpeechEngineWinRT : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineWinRT(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineWinRT();

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

public slots:
    void checkElementState();
private:
    void init();
    QVoice createVoiceForInformation(Microsoft::WRL::ComPtr<ABI::Windows::Media::SpeechSynthesis::IVoiceInformation> info) const;

    QScopedPointer<QTextToSpeechEngineWinRTPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QTextToSpeechEngineWinRT)
};

QT_END_NAMESPACE

#endif
