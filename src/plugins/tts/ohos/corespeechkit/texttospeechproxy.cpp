// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "texttospeechproxy.h"

#include <QtCore/quuid.h>
#include <QtCore/private/qcore_ohos_p.h>
#include <QtCore/private/qnapi_p.h>
#include <QtCore/private/qohoslogger_p.h>

QT_BEGIN_NAMESPACE

namespace CoreSpeechKit {

namespace {

std::optional<TtsCompletionType> tryParseTtsCompletionType(const QNapi::Object &response)
{
    static constexpr int ohosSynthesisCompleteTypeValue = 0;
    static constexpr int ohosSpeechCompleteTypeValue = 1;

    auto optTypeValue = QNapi::getOptionalPropOrEmpty<QNapi::Number>(response, "type");
    if (optTypeValue.IsEmpty())
        return {};

    switch (optTypeValue.Int32Value()) {
    case ohosSynthesisCompleteTypeValue:
        return TtsCompletionType::SynthesisComplete;
    case ohosSpeechCompleteTypeValue:
        return TtsCompletionType::SpeechComplete;
    default:
        return {};
    }
}

// Source of values used for EngineMode can be found under section 'online':
// https://developer.huawei.com/consumer/en/doc/harmonyos-references/hms-ai-texttospeech#section1638144844811
enum class EngineMode
{
    Online = 0,
    Offline = 1,
};

// Source of values used for SpeechPlayType can be found under section 'extraParams.playType':
// https://developer.huawei.com/consumer/en/doc/harmonyos-references/hms-ai-texttospeech#section7843122735210
enum class SpeechPlayType
{
    SynthesisOnly = 0,
    SynthesisAndPlayback = 1,
};

// Source of values used for SpeechQueueMode can be found under section 'extraParams.queueMode':
// https://developer.huawei.com/consumer/en/doc/harmonyos-references/hms-ai-texttospeech#section7843122735210
enum class SpeechQueueMode
{
    Queue = 0,
    Preemption = 1,
};

constexpr auto engineMode = EngineMode::Offline;
constexpr auto speechPlayType = SpeechPlayType::SynthesisAndPlayback;
constexpr auto speechQueueMode = SpeechQueueMode::Preemption;

class TextToSpeechProxyImpl : public TextToSpeechProxy
{
public:
    static q23::expected<std::shared_ptr<TextToSpeechProxyImpl>, std::string> tryMakeInstance(
        const std::string &language, int personTimbre);

    void speak(const SpeakParams &params) override;
    void stop() override;
    std::vector<VoiceInfo> listVoices() override;

    void setEngineEventsListener(
        std::shared_ptr<TextToSpeechProxy::EngineEventsListener> engineEventsListener);

private:
    struct JsScopeData
    {
        std::shared_ptr<QNapi::Reference<QNapi::Object>> textToSpeechEngineRef;
    };

    explicit TextToSpeechProxyImpl(const std::string &language, std::shared_ptr<JsScopeData> jsScopeData);

    static q23::expected<std::shared_ptr<JsScopeData>, std::string> tryMakeJsScopeData(
        const std::string &language, int personTimbre);

    std::shared_ptr<JsScopeData> m_jsScopeData;
    std::shared_ptr<TextToSpeechProxy::EngineEventsListener> m_engineEventsListener;
    const std::string m_language;
};

class QtThreadBasedEngineEventsListener : public TextToSpeechProxy::EngineEventsListener
{
public:
    QtThreadBasedEngineEventsListener(
        std::shared_ptr<TextToSpeechProxy::EngineEventsListener> baseEngineEventsListener);

    virtual void onStart(std::string utteranceId) override;
    virtual void onComplete(std::string utteranceId, std::optional<TtsCompletionType> optCompletionType) override;
    virtual void onStop(std::string utteranceId) override;
    virtual void onError(
        std::string utteranceId, std::uint32_t errorCode, std::string errorMessage) override;

private:
     std::shared_ptr<TextToSpeechProxy::EngineEventsListener> m_baseEngineEventsListener;
};

QtThreadBasedEngineEventsListener::QtThreadBasedEngineEventsListener(
    std::shared_ptr<TextToSpeechProxy::EngineEventsListener> baseEngineEventsListener)
    : m_baseEngineEventsListener(baseEngineEventsListener)
{
}

void QtThreadBasedEngineEventsListener::onStart(std::string utteranceId)
{
    QtOhos::invokeInQtThread(
        [weakBaseEngineEventsListener = QtOhos::makeWeakPtr(m_baseEngineEventsListener), utteranceId]() {
            auto baseEngineEventsListener = weakBaseEngineEventsListener.lock();
            if (baseEngineEventsListener)
                baseEngineEventsListener->onStart(utteranceId);
        });
}

void QtThreadBasedEngineEventsListener::onComplete(
    std::string utteranceId, std::optional<TtsCompletionType> optCompletionType)
{
    QtOhos::invokeInQtThread(
        [weakBaseEngineEventsListener = QtOhos::makeWeakPtr(m_baseEngineEventsListener), utteranceId, optCompletionType]() {
            auto baseEngineEventsListener = weakBaseEngineEventsListener.lock();
            if (baseEngineEventsListener)
                baseEngineEventsListener->onComplete(utteranceId, optCompletionType);
        });
}

void QtThreadBasedEngineEventsListener::onStop(std::string utteranceId)
{
    QtOhos::invokeInQtThread(
        [weakBaseEngineEventsListener = QtOhos::makeWeakPtr(m_baseEngineEventsListener), utteranceId]() {
            auto baseEngineEventsListener = weakBaseEngineEventsListener.lock();
            if (baseEngineEventsListener)
                baseEngineEventsListener->onStop(utteranceId);
        });
}

void QtThreadBasedEngineEventsListener::onError(
            std::string utteranceId, std::uint32_t errorCode, std::string errorMessage)
{
    QtOhos::invokeInQtThread(
        [weakBaseEngineEventsListener = QtOhos::makeWeakPtr(m_baseEngineEventsListener), utteranceId, errorCode, errorMessage]() {
            auto baseEngineEventsListener = weakBaseEngineEventsListener.lock();
            if (baseEngineEventsListener)
                baseEngineEventsListener->onError(utteranceId, errorCode, errorMessage);
        });
}

std::shared_ptr<TextToSpeechProxy::EngineEventsListener> makeEngineEventsListenerQtDecorator(
    std::shared_ptr<TextToSpeechProxy::EngineEventsListener> engineEventsListener)
{
    return std::make_shared<QtThreadBasedEngineEventsListener>(engineEventsListener);
}

std::string createUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
}

std::function<void(const QOhosCallbackInfo &)> makeOnStateCallbackDelegate(
    std::shared_ptr<TextToSpeechProxy::EngineEventsListener> engineEventsListener,
    void (TextToSpeechProxy::EngineEventsListener::*memberPtr)(std::string))
{
    return [weakEngineEventsListener = QtOhos::makeWeakPtr(engineEventsListener), memberPtr](const QNapi::CallbackInfo &cbInfo) {
        auto utteranceId = cbInfo.getFirstArg<QNapi::String>(Q_FUNC_INFO);
        auto engineEventsListener = weakEngineEventsListener.lock();
        if (engineEventsListener)
            (engineEventsListener.get()->*memberPtr)(utteranceId);
    };
}

std::function<void(const QOhosCallbackInfo &)> makeOnCompleteCallbackDelegate(
    std::shared_ptr<TextToSpeechProxy::EngineEventsListener> engineEventsListener)
{
    return [weakEngineEventsListener = QtOhos::makeWeakPtr(engineEventsListener)](const QNapi::CallbackInfo &cbInfo) {
        QNapi::String utteranceId;
        QNapi::Object response;
        cbInfo.getLeadingArgs(Q_FUNC_INFO, utteranceId, response);
        auto optCompletionType = tryParseTtsCompletionType(response);
        if (!optCompletionType) {
            qOhosPrintfWarning(
                "%s: unexpected completion type in response: %s, utteranceId: %s",
                Q_FUNC_INFO, QNapi::toJsonString(response).c_str(), utteranceId.Utf8Value().c_str());
        }
        auto engineEventsListener = weakEngineEventsListener.lock();
        if (engineEventsListener)
            engineEventsListener->onComplete(utteranceId, optCompletionType);
    };
}

void setEngineListenerInJsThread(
    QOhosJsState &jsState, QNapi::Object engine,
    std::shared_ptr<TextToSpeechProxy::EngineEventsListener> engineEventsListener)
{
    engine.eval(
        "setListener(*)",
        {
            QNapi::makeObject(
                jsState.env(),
                {
                    {
                        "onStart",
                        makeOnStateCallbackDelegate(
                            engineEventsListener,
                            &TextToSpeechProxy::EngineEventsListener::onStart)
                    },
                    {
                        "onComplete",
                        makeOnCompleteCallbackDelegate(engineEventsListener)
                    },
                    {
                        "onStop",
                        makeOnStateCallbackDelegate(
                            engineEventsListener,
                            &TextToSpeechProxy::EngineEventsListener::onStop)
                    },
                    {
                        "onError",
                        [weakEngineEventsListener = QtOhos::makeWeakPtr(engineEventsListener)](const QNapi::CallbackInfo &cbInfo) {
                            QNapi::String utteranceId;
                            QNapi::Number errorCode;
                            QNapi::String errorMessage;
                            cbInfo.getLeadingArgs(Q_FUNC_INFO, utteranceId, errorCode, errorMessage);
                            auto engineEventsListener = weakEngineEventsListener.lock();
                            if (engineEventsListener) {
                                engineEventsListener->onError(
                                    utteranceId, errorCode, errorMessage);
                            }
                        }
                    },
                })
        });
}

void tryCreateEngineImpl(
    QOhosJsState &jsState, const std::string &language, int personTimbre,
    QOhosTaskPromise<q23::expected<std::shared_ptr<QNapi::Reference<QNapi::Object>>, std::string>> taskPromise)
{
    auto thenCatchPromises = std::move(taskPromise).makeThenCatchBranches(Q_FUNC_INFO);
    jsState.evalToPromiseOrRejectOnThrow(
        "@kit.CoreSpeechKit.textToSpeech.createEngine(*)",
        {
            QNapi::makeObject(
                jsState.env(),
                {
                    {"language", language},
                    {"person", personTimbre},
                    {"online", static_cast<int>(engineMode)},
                }),
        })
    .onThen(
        [thenPromise = std::move(thenCatchPromises.first)](const QOhosCallbackInfo &cbInfo) {
            auto engine = cbInfo.getFirstArg<QNapi::Object>(Q_FUNC_INFO);
            auto engineRef = QtOhos::moveToSharedPtr(QNapi::Reference<>::makePersistentFrom(engine));
            thenPromise(
                QtOhos::makeSharedPtrWithAttachedExtraData(
                    engineRef,
                    QtOhos::makeDestroyNotifier(
                        [engineRef]() {
                            try {
                                engineRef->eval("shutdown()");
                            } catch (const Napi::Error &error) {
                                qOhosPrintfError(
                                    "%s: shutdown() failed with error: %s", Q_FUNC_INFO, error.what());
                            }
                        })
                ));
        })
    .onCatch(
        [catchPromise = std::move(thenCatchPromises.second)](const QOhosCallbackInfo &cbInfo) {
            auto errorObj = cbInfo.getFirstArg<QNapi::Object>(Q_FUNC_INFO);
            catchPromise(q23::unexpected(errorObj.get<QNapi::String>("message")));
        });
}

q23::expected<std::shared_ptr<TextToSpeechProxyImpl>, std::string> TextToSpeechProxyImpl::tryMakeInstance(
    const std::string &language, int personTimbre)
{
    auto jsScopeDataOrError = tryMakeJsScopeData(language, personTimbre);
    if (!jsScopeDataOrError)
        return q23::unexpected(std::move(jsScopeDataOrError.error()));

    return std::shared_ptr<TextToSpeechProxyImpl>(
        new TextToSpeechProxyImpl(language, std::move(jsScopeDataOrError.value())));
}

void TextToSpeechProxyImpl::setEngineEventsListener(
    std::shared_ptr<TextToSpeechProxy::EngineEventsListener> engineEventsListener)
{
    m_engineEventsListener = makeEngineEventsListenerQtDecorator(engineEventsListener);
    QOhosJsThreadGateway::runAndWait(
        [&](QOhosJsState &jsState) {
            setEngineListenerInJsThread(
                jsState, m_jsScopeData->textToSpeechEngineRef->Value(),
                m_engineEventsListener);
        },
        Q_FUNC_INFO);
}

q23::expected<std::shared_ptr<TextToSpeechProxyImpl::JsScopeData>, std::string> TextToSpeechProxyImpl::tryMakeJsScopeData(
    const std::string &language, int personTimbre)
{
    return QOhosJsThreadGateway::evalWithPromise<q23::expected<std::shared_ptr<TextToSpeechProxyImpl::JsScopeData>, std::string>>(
        [&](QOhosJsState &jsState, auto evalPromise) {
            auto sharedEvalPromise = QtOhos::moveToSharedPtr(
                std::move(evalPromise).makeChained(Q_FUNC_INFO));
            tryCreateEngineImpl(
                jsState, language, personTimbre,
                QOhosTaskPromise<q23::expected<std::shared_ptr<QNapi::Reference<QNapi::Object>>, std::string>>(
                    [sharedEvalPromise](q23::expected<std::shared_ptr<QNapi::Reference<QNapi::Object>>, std::string> engineRefOrError) {
                        if (!engineRefOrError) {
                            (*sharedEvalPromise)(q23::unexpected(std::move(engineRefOrError.error())));
                            return;
                        }
                        (*sharedEvalPromise)(
                            QtOhos::makeProxyWithJsThreadDeleter(
                                QtOhos::moveToSharedPtr(
                                    TextToSpeechProxyImpl::JsScopeData {
                                        .textToSpeechEngineRef = engineRefOrError.value(),
                                    })));
                    },
                    {},
                    Q_FUNC_INFO));
        },
        Q_FUNC_INFO);
}

TextToSpeechProxyImpl::TextToSpeechProxyImpl(
    const std::string &language, std::shared_ptr<JsScopeData> jsScopeData)
    : m_jsScopeData(std::move(jsScopeData))
    , m_language(language)
{
}

void TextToSpeechProxyImpl::speak(const SpeakParams &params)
{
    QOhosJsThreadGateway::runAndWait(
        [&](QOhosJsState &jsState) {
            m_jsScopeData->textToSpeechEngineRef->eval(
                "speak(*)",
                {
                    params.text,
                    QNapi::makeObject(
                        jsState.env(),
                        {
                            {"requestId", createUuid()},
                            {
                                "extraParams",
                                QNapi::makeObject(
                                    jsState.env(),
                                    {
                                        {"queueMode", static_cast<int>(speechQueueMode)},
                                        {"speed", params.speed},
                                        {"volume", params.volume},
                                        {"pitch", params.pitch},
                                        {"languageContext", params.languageContext},
                                        {"playType", static_cast<int>(speechPlayType)},
                                    })
                            },
                        }),
                });
        },
        Q_FUNC_INFO);
}

void TextToSpeechProxyImpl::stop()
{
    QOhosJsThreadGateway::runAndWait(
        [&](QOhosJsState &) {
            m_jsScopeData->textToSpeechEngineRef->eval("stop()");
        },
        Q_FUNC_INFO);
}

std::vector<VoiceInfo> TextToSpeechProxyImpl::listVoices()
{
    return QOhosJsThreadGateway::evalWithPromise<std::vector<VoiceInfo>>(
        [&](QOhosJsState &jsState, QOhosTaskPromise<std::vector<VoiceInfo>> evalPromise) {
            auto thenCatchPromises = std::move(evalPromise).makeThenCatchBranches(Q_FUNC_INFO);
            m_jsScopeData->textToSpeechEngineRef->evalToPromiseOrRejectOnThrow(
                "listVoices(*)",
                {
                    QNapi::makeObject(
                        jsState.env(),
                        {
                            {"requestId", createUuid()},
                            {"online", static_cast<int>(engineMode)},
                        })
                })
            .onThen(
                [thenPromise = std::move(thenCatchPromises.first)](const QOhosCallbackInfo &cbInfo) {
                    auto voicesArray = cbInfo.getFirstArg<QNapi::Array>(Q_FUNC_INFO);

                    thenPromise(
                        QNapi::getArrayElements<std::vector<VoiceInfo>, QNapi::Object>(
                            voicesArray,
                            [](QNapi::Object jsVoice) {
                                return VoiceInfo{
                                    .language = jsVoice.get<QNapi::String>("language"),
                                    .style = jsVoice.get<QNapi::String>("style"),
                                    .gender = jsVoice.get<QNapi::String>("gender"),
                                    .description = jsVoice.get<QNapi::String>("description"),
                                    .personTimbre = jsVoice.get<QNapi::Number>("person"),
                                };
                            }));
                })
            .onCatch(
                [catchPromise = std::move(thenCatchPromises.second)](const QOhosCallbackInfo &cbInfo) {
                    QtOhos::logJsCallbackError(cbInfo, "listVoices() failed");
                    catchPromise({});
                });
        },
        Q_FUNC_INFO);
}

}

TextToSpeechProxy::TextToSpeechProxy() = default;

TextToSpeechProxy::~TextToSpeechProxy() = default;

TextToSpeechProxy::EngineEventsListener::EngineEventsListener() = default;

TextToSpeechProxy::EngineEventsListener::~EngineEventsListener() = default;

q23::expected<std::function<std::shared_ptr<TextToSpeechProxy>(std::shared_ptr<TextToSpeechProxy::EngineEventsListener>)>, std::string> tryMakeTextToSpeechProxyFactory(
    const std::string &language, int personTimbre)
{
    auto proxyImplInstanceOrError = TextToSpeechProxyImpl::tryMakeInstance(language, personTimbre);
    if (!proxyImplInstanceOrError)
        return q23::unexpected(std::move(proxyImplInstanceOrError.error()));

    return std::function<std::shared_ptr<TextToSpeechProxy>(std::shared_ptr<TextToSpeechProxy::EngineEventsListener>)>(
        [proxyImplInstance = std::move(proxyImplInstanceOrError.value())](std::shared_ptr<TextToSpeechProxy::EngineEventsListener> engineEventsListener) {
            proxyImplInstance->setEngineEventsListener(engineEventsListener);
            return proxyImplInstance;
        });
}

}

QT_END_NAMESPACE
