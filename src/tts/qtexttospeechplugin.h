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

#ifndef QTEXTTOSPEECHPLUGIN_H
#define QTEXTTOSPEECHPLUGIN_H

#include <QtTextToSpeech/qtexttospeechengine.h>

#include <QtCore/QtPlugin>
#include <QtCore/QString>
#include <QtCore/QVariantMap>

QT_BEGIN_NAMESPACE

class Q_TEXTTOSPEECH_EXPORT QTextToSpeechPlugin
{
public:
    virtual ~QTextToSpeechPlugin() {}

    virtual QTextToSpeechEngine *createTextToSpeechEngine(
            const QVariantMap &parameters,
            QObject *parent,
            QString *errorString) const;
};

Q_DECLARE_INTERFACE(QTextToSpeechPlugin,
                    "org.qt-project.qt.speech.tts.plugin/5.0")

QT_END_NAMESPACE

#endif
