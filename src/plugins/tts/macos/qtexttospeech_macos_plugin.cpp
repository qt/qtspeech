// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "qtexttospeech_macos_plugin.h"
#include "qtexttospeech_macos.h"

QTextToSpeechEngine *QTextToSpeechMacOSPlugin::createTextToSpeechEngine(const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    Q_UNUSED(errorString);
    return new QTextToSpeechEngineMacOS(parameters, parent);
}
