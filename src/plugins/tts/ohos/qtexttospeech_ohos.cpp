// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_ohos.h"
#include "qtexttospeech_ohos_plugin.h"

#include "corespeechkit/texttospeechproxy.h"

#include <QtTextToSpeech/qvoice.h>

#include <QtCore/qcoreapplication.h>
#include <QtCore/qlist.h>
#include <QtCore/qlocale.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qstring.h>

#include <cstdint>
#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

namespace {

constexpr const char *chineseLanguage = "zh-CN";
constexpr const char *chineseLanguageContext = "zh-CN";
constexpr int defaultPersonTimbre = 0;

QVoice::Gender mapVoiceGender(const std::string &gender)
{
    return gender == "male"   ? QVoice::Gender::Male   :
           gender == "female" ? QVoice::Gender::Female :
                                QVoice::Gender::Unknown;
}

QVariantMap mapVoiceExtraData(const CoreSpeechKit::VoiceInfo &info)
{
    return QVariantMap{
        {QStringLiteral("style"), QString::fromStdString(info.style)},
        {QStringLiteral("person"), info.personTimbre},
    };
}

struct EngineError
{
    QTextToSpeech::ErrorReason reason;
    QString message;
};

// Source of the error codes can be found under section 'Error Codes':
// https://developer.huawei.com/consumer/en/doc/harmonyos-references/errorcode-corespeech
EngineError mapTtsError(std::uint32_t errorCode)
{
    static constexpr std::uint32_t ohosTextOutOfRangeOrEmptyValue = 1002300001;
    static constexpr std::uint32_t ohosLanguageNotSupportedValue = 1002300002;
    static constexpr std::uint32_t ohosPersonNotSupportedValue = 1002300003;
    static constexpr std::uint32_t ohosCreateEngineFailedValue = 1002300005;
    static constexpr std::uint32_t ohosParameterErrorValue = 1002300009;

    switch (errorCode) {
    case ohosTextOutOfRangeOrEmptyValue:
        return {
            QTextToSpeech::ErrorReason::Input,
            QCoreApplication::translate("QTextToSpeech", "Speech synthesizing failure.")
        };
    case ohosLanguageNotSupportedValue:
    case ohosPersonNotSupportedValue:
    case ohosParameterErrorValue:
        return {
            QTextToSpeech::ErrorReason::Configuration,
            QCoreApplication::translate("QTextToSpeech", "Could not apply text-to-speech parameters.")
        };
    case ohosCreateEngineFailedValue:
        return {
            QTextToSpeech::ErrorReason::Initialization,
            QCoreApplication::translate("QTextToSpeech", "Failed to initialize text-to-speech engine.")
        };
    default:
        return {
            QTextToSpeech::ErrorReason::Playback,
            QCoreApplication::translate("QTextToSpeech", "Unknown error: %1.").arg(errorCode)
        };
    }
}

double mapVolumeToOhosVolume(double volume)
{
    constexpr double maxTtsVolume = 2.0;
    return volume * maxTtsVolume;
}

double mapRateToOhosSpeed(double rate)
{
    constexpr double minTtsRate = 0.5;
    constexpr double normalTtsRate = 1.0;

    return rate < 0
        ? minTtsRate * rate + normalTtsRate
        : rate + normalTtsRate;
}

double mapPitchToOhosPitch(double pitch)
{
    constexpr double minTtsPitch = 0.5;
    constexpr double normalTtsPitch = 1.0;

    return pitch < 0
        ? minTtsPitch * pitch + normalTtsPitch
        : pitch + normalTtsPitch;
}

class QTextToSpeechEngineOhos : public QTextToSpeechEngine
{
public:
    QTextToSpeechEngineOhos(
        const QVariantMap &parameters, QObject *parent,
        std::shared_ptr<CoreSpeechKit::TextToSpeechProxy> ttsProxy);

    QList<QLocale> availableLocales() const override;
    QList<QVoice> availableVoices() const override;
    void say(const QString &text) override;
    void synthesize(const QString &text) override;
    void stop(QTextToSpeech::BoundaryHint boundaryHint) override;
    void pause(QTextToSpeech::BoundaryHint boundaryHint) override;
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
    QTextToSpeech::ErrorReason errorReason() const override;
    QString errorString() const override;

private:
    class TextToSpeechEngineEventsListener
        : public CoreSpeechKit::TextToSpeechProxy::EngineEventsListener
    {
    public:
        explicit TextToSpeechEngineEventsListener(QTextToSpeechEngineOhos &owner);

        void onStart(std::string utteranceId) override;
        void onComplete(std::string utteranceId, std::optional<CoreSpeechKit::TtsCompletionType> optCompletionType) override;
        void onStop(std::string utteranceId) override;
        void onError(std::string utteranceId, std::uint32_t errorCode, std::string errorMsg) override;

        QTextToSpeechEngineOhos& m_owner;
    };

    void handleOnStart(const QString &utteranceId);
    void handleOnComplete(const QString &utteranceId, std::optional<CoreSpeechKit::TtsCompletionType> optCompletionType);
    void handleOnStop(const QString &utteranceId);
    void handleOnError(
        const QString &utteranceId, std::uint32_t errorCode, const QString &errorMessage);
    void setState(QTextToSpeech::State state);
    void setError(const EngineError &error);

    std::shared_ptr<CoreSpeechKit::TextToSpeechProxy> m_ttsProxy;

    QTextToSpeech::State m_state;
    QTextToSpeech::ErrorReason m_errorReason;
    QString m_errorString;
    QList<QVoice> m_voices;
    QList<QLocale> m_locales;
    QVoice m_currentVoice;
    double m_volume;
    double m_rate;
    double m_pitch;
};

QTextToSpeechEngineOhos::TextToSpeechEngineEventsListener::TextToSpeechEngineEventsListener(
    QTextToSpeechEngineOhos &owner)
    : m_owner(owner)
{
}

void QTextToSpeechEngineOhos::TextToSpeechEngineEventsListener::onStart(std::string utteranceId)
{
    m_owner.handleOnStart(QString::fromStdString(utteranceId));
}

void QTextToSpeechEngineOhos::TextToSpeechEngineEventsListener::onComplete(
    std::string utteranceId, std::optional<CoreSpeechKit::TtsCompletionType> optCompletionType)
{
    m_owner.handleOnComplete(QString::fromStdString(utteranceId), optCompletionType);
}

void QTextToSpeechEngineOhos::TextToSpeechEngineEventsListener::onStop(std::string utteranceId)
{
    m_owner.handleOnStop(QString::fromStdString(utteranceId));
}

void QTextToSpeechEngineOhos::TextToSpeechEngineEventsListener::onError(
    std::string utteranceId, std::uint32_t errorCode, std::string errorMessage)
{
    m_owner.handleOnError(
         QString::fromStdString(utteranceId), errorCode, QString::fromStdString(errorMessage));
}

QTextToSpeechEngineOhos::QTextToSpeechEngineOhos(
    const QVariantMap &, QObject *parent,
    std::shared_ptr<CoreSpeechKit::TextToSpeechProxy> ttsProxy)
    : QTextToSpeechEngine(parent)
    , m_ttsProxy(std::move(ttsProxy))
    , m_state(QTextToSpeech::Ready)
    , m_errorReason(QTextToSpeech::ErrorReason::NoError)
    , m_volume(1.0)
    , m_rate(0.0)
    , m_pitch(0.0)
{
    m_ttsProxy->setEngineEventsListener(std::make_shared<TextToSpeechEngineEventsListener>(*this));

    auto optVoices = m_ttsProxy->listVoices();
    if (!optVoices) {
        setError({
            QTextToSpeech::ErrorReason::Configuration,
            QCoreApplication::translate("QTextToSpeech", "Failed to initialize default locale and voice.")
        });
        return;
    }

    for (const auto &voiceInfo : *optVoices) {
        QLocale locale(QString::fromStdString(voiceInfo.language));
        m_voices.append(
            createVoice(
                QString::fromStdString(voiceInfo.description), locale,
                mapVoiceGender(voiceInfo.gender), QVoice::Other,
                mapVoiceExtraData(voiceInfo)));

        if (!m_locales.contains(locale))
            m_locales.append(locale);
    }

    if (!m_voices.isEmpty())
        m_currentVoice = m_voices.first();
}

void QTextToSpeechEngineOhos::handleOnStart(const QString &)
{
    setState(QTextToSpeech::Speaking);
}

void QTextToSpeechEngineOhos::handleOnComplete(
    const QString &, std::optional<CoreSpeechKit::TtsCompletionType> optCompletionType)
{
    if (!optCompletionType) {
        setError({
            QTextToSpeech::ErrorReason::Playback,
            QCoreApplication::translate("QTextToSpeech", "Speech synthesizing failure.")
        });
        return;
    }

    if (*optCompletionType == CoreSpeechKit::TtsCompletionType::SpeechComplete)
        setState(QTextToSpeech::Ready);
}

void QTextToSpeechEngineOhos::handleOnStop(const QString &)
{
    setState(QTextToSpeech::Ready);
}

void QTextToSpeechEngineOhos::handleOnError(
    const QString &utteranceId, std::uint32_t errorCode, const QString &errorMessage)
{
    qCWarning(lcSpeechTtsOhos)
        << Q_FUNC_INFO << ": text to speech error: code" << errorCode
        << errorMessage << ", utteranceId:" << utteranceId;

    setError(mapTtsError(errorCode));
}

void QTextToSpeechEngineOhos::setState(QTextToSpeech::State state)
{
    if (m_state == state)
        return;

    m_state = state;
    Q_EMIT stateChanged(m_state);

    if (m_state == QTextToSpeech::Error) {
        Q_EMIT errorOccurred(m_errorReason, m_errorString);
    } else {
        m_errorReason = QTextToSpeech::ErrorReason::NoError;
        m_errorString.clear();
    }
}

void QTextToSpeechEngineOhos::setError(const EngineError &error)
{
    m_errorReason = error.reason;
    m_errorString = error.message;

    if (error.reason == QTextToSpeech::ErrorReason::NoError) {
        m_errorString.clear();
        return;
    }

    if (m_state != QTextToSpeech::Error) {
        m_state = QTextToSpeech::Error;
        Q_EMIT stateChanged(m_state);
    }
    Q_EMIT errorOccurred(m_errorReason, m_errorString);
}

QList<QLocale> QTextToSpeechEngineOhos::availableLocales() const
{
    return m_locales;
}

QList<QVoice> QTextToSpeechEngineOhos::availableVoices() const
{
    QList<QVoice> voices;
    for (const auto &voice : m_voices) {
        if (voice.locale() == m_currentVoice.locale())
            voices << voice;
    }

    return voices;
}

void QTextToSpeechEngineOhos::say(const QString &text)
{
    if (text.isEmpty())
        return;

    m_ttsProxy->speak(
        {
            .text = text.toStdString(),
            .speed = mapRateToOhosSpeed(m_rate),
            .volume = mapVolumeToOhosVolume(m_volume),
            .pitch = mapPitchToOhosPitch(m_pitch),
            .languageContext = chineseLanguageContext,
        });
}

void QTextToSpeechEngineOhos::synthesize(const QString &)
{
    qCWarning(lcSpeechTtsOhos) << Q_FUNC_INFO << ": synthesize feature is not supported";
}

void QTextToSpeechEngineOhos::stop(QTextToSpeech::BoundaryHint)
{
    m_ttsProxy->stop();
}

void QTextToSpeechEngineOhos::pause(QTextToSpeech::BoundaryHint)
{
    qCWarning(lcSpeechTtsOhos) << Q_FUNC_INFO << ": pause feature is not supported";
}

void QTextToSpeechEngineOhos::resume()
{
    qCWarning(lcSpeechTtsOhos) << Q_FUNC_INFO << ": resume feature is not supported";
}

double QTextToSpeechEngineOhos::rate() const
{
    return m_rate;
}

bool QTextToSpeechEngineOhos::setRate(double rate)
{
    m_rate = rate;
    return true;
}

double QTextToSpeechEngineOhos::pitch() const
{
    return m_pitch;
}

bool QTextToSpeechEngineOhos::setPitch(double pitch)
{
    m_pitch = pitch;
    return true;
}

QLocale QTextToSpeechEngineOhos::locale() const
{
    return m_currentVoice.locale();
}

bool QTextToSpeechEngineOhos::setLocale(const QLocale &locale)
{
    if (!m_locales.contains(locale))
        return false;

    if (m_currentVoice.locale() == locale)
        return true;

    for (const auto &voice : m_voices) {
        if (voice.locale() == locale) {
            m_currentVoice = voice;
            return true;
        }
    }

    return false;
}

double QTextToSpeechEngineOhos::volume() const
{
    return m_volume;
}

bool QTextToSpeechEngineOhos::setVolume(double volume)
{
    m_volume = volume;
    return true;
}

QVoice QTextToSpeechEngineOhos::voice() const
{
    return m_currentVoice;
}

bool QTextToSpeechEngineOhos::setVoice(const QVoice &voice)
{
    if (!m_voices.contains(voice)) {
        qCWarning(lcSpeechTtsOhos) << "voice" << voice.name() << "is not available in this engine";
        return false;
    }

    m_currentVoice = voice;
    return true;
}

QTextToSpeech::State QTextToSpeechEngineOhos::state() const
{
    return m_state;
}

QTextToSpeech::ErrorReason QTextToSpeechEngineOhos::errorReason() const
{
    return m_errorReason;
}

QString QTextToSpeechEngineOhos::errorString() const
{
    return m_errorString;
}

}

QTextToSpeechEngine *createQTextToSpeechEngineOhos(
    const QVariantMap &parameters, QObject *parent, QString *errorString)
{
    auto ttsProxyOrError = CoreSpeechKit::tryMakeTextToSpeechProxy(chineseLanguage, defaultPersonTimbre);

    if (!ttsProxyOrError) {
        if (errorString != nullptr)
            *errorString = QString::fromStdString(ttsProxyOrError.error());
        return nullptr;
    }
    return new QTextToSpeechEngineOhos(parameters, parent, std::move(ttsProxyOrError.value()));
}

QT_END_NAMESPACE
