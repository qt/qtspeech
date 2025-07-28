// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef TEXTTOSPEECHPROXY_H
#define TEXTTOSPEECHPROXY_H

#include <QtCore/private/qexpected_p.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

QT_BEGIN_NAMESPACE

namespace CoreSpeechKit {

struct SpeakParams
{
    std::string text;
    double speed;
    double volume;
    double pitch;
    std::string languageContext;
};

struct VoiceInfo
{
    std::string language;
    std::string style;
    std::string gender;
    std::string description;
    int personTimbre;
};

class TextToSpeechProxy
{
    Q_DISABLE_COPY_MOVE(TextToSpeechProxy)
public:
    virtual ~TextToSpeechProxy();

    virtual void speak(const SpeakParams &params) = 0;
    virtual void stop() = 0;
    virtual std::vector<VoiceInfo> listVoices() = 0;

protected:
    TextToSpeechProxy();
};

q23::expected<std::function<std::shared_ptr<TextToSpeechProxy>()>, std::string> tryMakeTextToSpeechProxyFactory(
    const std::string &language, int personTimbre);

}

QT_END_NAMESPACE

#endif
