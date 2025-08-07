// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_ohos.h"
#include "qtexttospeech_ohos_plugin.h"

#include "corespeechkit/texttospeechproxy.h"

#include <QtTextToSpeech/qvoice.h>

#include <QtCore/qlist.h>
#include <QtCore/qlocale.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qstring.h>

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>

QT_BEGIN_NAMESPACE

namespace {

constexpr const char *chineseLanguage = "zh-CN";
constexpr int defaultPersonTimbre = 0;

class QTextToSpeechEngineOhos : public QTextToSpeechEngine
{
public:
    QTextToSpeechEngineOhos(
        const QVariantMap &parameters, QObject *parent,
        std::function<std::shared_ptr<CoreSpeechKit::TextToSpeechProxy>(
            std::shared_ptr<CoreSpeechKit::TextToSpeechProxy::EngineEventsListener>)> ttsProxyFactory);

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

    std::shared_ptr<CoreSpeechKit::TextToSpeechProxy> m_ttsProxy;
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
    std::function<std::shared_ptr<CoreSpeechKit::TextToSpeechProxy>(std::shared_ptr<CoreSpeechKit::TextToSpeechProxy::EngineEventsListener>)> ttsProxyFactory)
    : QTextToSpeechEngine(parent)
{
    m_ttsProxy = ttsProxyFactory(std::make_shared<TextToSpeechEngineEventsListener>(*this));
}

void QTextToSpeechEngineOhos::handleOnStart(const QString &)
{
}

void QTextToSpeechEngineOhos::handleOnComplete(
    const QString &, std::optional<CoreSpeechKit::TtsCompletionType>)
{
}

void QTextToSpeechEngineOhos::handleOnStop(const QString &)
{
}

void QTextToSpeechEngineOhos::handleOnError(
    const QString &, std::uint32_t, const QString &)
{
}

QList<QLocale> QTextToSpeechEngineOhos::availableLocales() const
{
    return {};
}

QList<QVoice> QTextToSpeechEngineOhos::availableVoices() const
{
    return {};
}

void QTextToSpeechEngineOhos::say(const QString &)
{
}

void QTextToSpeechEngineOhos::synthesize(const QString &)
{
    qCWarning(lcSpeechTtsOhos) << Q_FUNC_INFO << ": synthesize feature is not supported";
}

void QTextToSpeechEngineOhos::stop(QTextToSpeech::BoundaryHint)
{
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
    return 0.0;
}

bool QTextToSpeechEngineOhos::setRate(double)
{
    return false;
}

double QTextToSpeechEngineOhos::pitch() const
{
    return 0.0;
}

bool QTextToSpeechEngineOhos::setPitch(double)
{
    return false;
}

QLocale QTextToSpeechEngineOhos::locale() const
{
    return {};
}

bool QTextToSpeechEngineOhos::setLocale(const QLocale &)
{
    return false;
}

double QTextToSpeechEngineOhos::volume() const
{
    return 1.0;
}

bool QTextToSpeechEngineOhos::setVolume(double)
{
    return false;
}

QVoice QTextToSpeechEngineOhos::voice() const
{
    return {};
}

bool QTextToSpeechEngineOhos::setVoice(const QVoice &)
{
    return false;
}

QTextToSpeech::State QTextToSpeechEngineOhos::state() const
{
    return QTextToSpeech::State::Error;
}

QTextToSpeech::ErrorReason QTextToSpeechEngineOhos::errorReason() const
{
    qCWarning(lcSpeechTtsOhos) << Q_FUNC_INFO << ": errorReason feature is not supported";
    return QTextToSpeech::ErrorReason::NoError;
}

QString QTextToSpeechEngineOhos::errorString() const
{
    qCWarning(lcSpeechTtsOhos) << Q_FUNC_INFO << ": errorString feature is not supported";
    return QString();
}

}

QTextToSpeechEngine *createQTextToSpeechEngineOhos(
    const QVariantMap &parameters, QObject *parent, QString *errorString)
{
    auto ttsProxyFactoryOrError = CoreSpeechKit::tryMakeTextToSpeechProxyFactory(
        chineseLanguage, defaultPersonTimbre);

    if (!ttsProxyFactoryOrError) {
        if (errorString != nullptr)
            *errorString = QString::fromStdString(ttsProxyFactoryOrError.error());
        return nullptr;
    }
    return new QTextToSpeechEngineOhos(
        parameters, parent, std::move(ttsProxyFactoryOrError.value()));
}

QT_END_NAMESPACE
