// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#include "qtexttospeech_mock_plugin.h"
#include "qtexttospeech_mock.h"

QTextToSpeechEngine *QTextToSpeechMockPlugin::createTextToSpeechEngine(const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    Q_UNUSED(errorString);
    return new QTextToSpeechEngineMock(parameters, parent);
}
