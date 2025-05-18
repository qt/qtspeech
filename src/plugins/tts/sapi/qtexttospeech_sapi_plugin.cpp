// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "qtexttospeech_sapi_plugin.h"
#include "qtexttospeech_sapi.h"

QTextToSpeechEngine *QTextToSpeechSapiPlugin::createTextToSpeechEngine(const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    Q_UNUSED(errorString);
    return new QTextToSpeechEngineSapi(parameters, parent);
}
