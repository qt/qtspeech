// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#include "qtexttospeech_ohos_plugin.h"

#include "qtexttospeech_ohos.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSpeechTtsOhos, "qt.speech.tts.ohos")

QTextToSpeechEngine *QTextToSpeechOhosPlugin::createTextToSpeechEngine(
    const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    return createQTextToSpeechEngineOhos(parameters, parent, errorString);
}

QT_END_NAMESPACE
