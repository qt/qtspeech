// Copyright (C) 2026 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR LGPL-3.0-only OR GPL-2.0-only OR GPL-3.0-only
// Qt-Security score:significant reason:default

#ifndef QTEXTTOSPEECHPLUGIN_OHOS_H
#define QTEXTTOSPEECHPLUGIN_OHOS_H

#include <QtTextToSpeech/qtexttospeechengine.h>
#include <QtTextToSpeech/qtexttospeechplugin.h>

#include <QtCore/qloggingcategory.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcSpeechTtsOhos)

class QTextToSpeechOhosPlugin : public QObject, public QTextToSpeechPlugin
{
    Q_OBJECT
    Q_INTERFACES(QTextToSpeechPlugin)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.speech.tts.plugin/6.0"
        FILE "ohos_plugin.json")

public:
    QTextToSpeechEngine *createTextToSpeechEngine(
        const QVariantMap &parameters, QObject *parent, QString *errorString) const override;
};

QT_END_NAMESPACE

#endif
