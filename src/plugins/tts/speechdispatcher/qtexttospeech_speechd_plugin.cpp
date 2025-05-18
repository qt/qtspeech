// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "qtexttospeech_speechd_plugin.h"
#include "qtexttospeech_speechd.h"

QTextToSpeechEngine *QTextToSpeechSpeechdPlugin::createTextToSpeechEngine(const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    Q_UNUSED(errorString);
    return new QTextToSpeechEngineSpeechd(parameters, parent);
}
