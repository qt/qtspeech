// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "qtexttospeech_winrt_plugin.h"
#include "qtexttospeech_winrt.h"

QTextToSpeechEngine *QTextToSpeechWinRTPlugin::createTextToSpeechEngine(const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    Q_UNUSED(errorString);
    return new QTextToSpeechEngineWinRT(parameters, parent);
}
