// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "qtexttospeech_darwin_plugin.h"
#include "qtexttospeech_darwin.h"

QTextToSpeechEngine *QTextToSpeechDarwinPlugin::createTextToSpeechEngine(const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    Q_UNUSED(errorString);
    return new QTextToSpeechEngineDarwin(parameters, parent);
}
