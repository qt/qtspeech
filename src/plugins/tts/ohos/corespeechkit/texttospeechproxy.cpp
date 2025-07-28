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

private:
    struct JsScopeData
    {
        std::shared_ptr<QNapi::Reference<QNapi::Object>> textToSpeechEngineRef;
    };

    explicit TextToSpeechProxyImpl(const std::string &language, std::shared_ptr<JsScopeData> jsScopeData);

    static q23::expected<std::shared_ptr<JsScopeData>, std::string> tryMakeJsScopeData(
        const std::string &language, int personTimbre);

    std::shared_ptr<JsScopeData> m_jsScopeData;
    const std::string m_language;
};

std::string createUuid()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
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

q23::expected<std::function<std::shared_ptr<TextToSpeechProxy>()>, std::string> tryMakeTextToSpeechProxyFactory(
    const std::string &language, int personTimbre)
{
    auto proxyImplInstanceOrError = TextToSpeechProxyImpl::tryMakeInstance(language, personTimbre);
    if (!proxyImplInstanceOrError)
        return q23::unexpected(std::move(proxyImplInstanceOrError.error()));

    return std::function<std::shared_ptr<TextToSpeechProxy>()>(
        [proxyImplInstance = std::move(proxyImplInstanceOrError.value())]() {
            return proxyImplInstance;
        });
}

}

QT_END_NAMESPACE
