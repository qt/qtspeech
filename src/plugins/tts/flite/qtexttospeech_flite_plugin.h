// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial

#ifndef QTEXTTOSPEECHPLUGIN_FLITE_H
#define QTEXTTOSPEECHPLUGIN_FLITE_H

#include "qtexttospeechplugin.h"
#include "qtexttospeechengine.h"

#include <QtCore/QObject>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcSpeechTtsFlite)

class QTextToSpeechFlitePlugin : public QObject, public QTextToSpeechPlugin
{
    Q_OBJECT
    Q_INTERFACES(QTextToSpeechPlugin)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.speech.tts.plugin/6.0"
                      FILE "flite_plugin.json")

public:
    QTextToSpeechEngine *createTextToSpeechEngine(
                                const QVariantMap &parameters,
                                QObject *parent,
                                QString *errorString) const override;
};

QT_END_NAMESPACE

#endif
