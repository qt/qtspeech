/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Speech module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qtexttospeech_flite_plugin.h"
#include "qtexttospeech_flite.h"

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSpeechTtsFlite, "qt.speech.tts.flite")

QTextToSpeechEngine *QTextToSpeechPluginFlite::createTextToSpeechEngine(
        const QVariantMap &parameters, QObject *parent, QString *errorString) const
{
    QTextToSpeechEngineFlite *flite = new QTextToSpeechEngineFlite(parameters, parent);
    if (flite && flite->init(errorString)) {
        return flite;
    }
    delete flite;
    return 0;
}

QT_END_NAMESPACE
