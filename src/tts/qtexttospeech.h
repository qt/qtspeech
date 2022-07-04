// Copyright (C) 2015 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only




#ifndef QTEXTTOSPEECH_H
#define QTEXTTOSPEECH_H

#include <QtTextToSpeech/qtexttospeech_global.h>
#include <QtCore/qobject.h>
#include <QtCore/qshareddata.h>
#include <QtCore/QSharedDataPointer>
#include <QtCore/qlocale.h>
#include <QtTextToSpeech/qvoice.h>

#include <QtQmlIntegration/qqmlintegration.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechPrivate;
class Q_TEXTTOSPEECH_EXPORT QTextToSpeech : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString engine READ engine WRITE setEngine NOTIFY engineChanged)
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
        Error
    };
    Q_ENUM(State)

    enum class ErrorReason {
        NoError,
        Initialization,
        Configuration,
        Input,
        Playback
    };
    Q_ENUM(ErrorReason)

    enum class BoundaryHint {
        Default,
        Immediate,
        Word,
        Sentence
    };
    Q_ENUM(BoundaryHint)

    explicit QTextToSpeech(QObject *parent = nullptr);
    explicit QTextToSpeech(const QString &engine, QObject *parent = nullptr);
    explicit QTextToSpeech(const QString &engine, const QVariantMap &params,
                           QObject *parent = nullptr);
    ~QTextToSpeech() override;

    Q_INVOKABLE bool setEngine(const QString &engine, const QVariantMap &params = QVariantMap());
    QString engine() const;

    QTextToSpeech::State state() const;
    Q_INVOKABLE QTextToSpeech::ErrorReason errorReason() const;
    Q_INVOKABLE QString errorString() const;

    Q_INVOKABLE QList<QLocale> availableLocales() const;
    QLocale locale() const;

    QVoice voice() const;
    Q_INVOKABLE QList<QVoice> availableVoices() const;

    double rate() const;
    double pitch() const;
    double volume() const;

    Q_INVOKABLE static QStringList availableEngines();

public Q_SLOTS:
    void say(const QString &text);
    void stop(QTextToSpeech::BoundaryHint boundaryHint = QTextToSpeech::BoundaryHint::Default);
    void pause(QTextToSpeech::BoundaryHint boundaryHint = QTextToSpeech::BoundaryHint::Default);
    void resume();

    void setLocale(const QLocale &locale);

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(double volume);
    void setVoice(const QVoice &voice);

Q_SIGNALS:
    void engineChanged(const QString &engine);
    void stateChanged(QTextToSpeech::State state);
    void errorOccurred(QTextToSpeech::ErrorReason error, const QString &errorString);
    void localeChanged(const QLocale &locale);
    void rateChanged(double rate);
    void pitchChanged(double pitch);
    void volumeChanged(double volume);
    void voiceChanged(const QVoice &voice);

private:
    Q_DISABLE_COPY(QTextToSpeech)
};

QT_END_NAMESPACE

#endif
