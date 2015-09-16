/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the Qt Speech module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL3$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPLv3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or later as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file. Please review the following information to
** ensure the GNU General Public License version 2.0 requirements will be
** met: http://www.gnu.org/licenses/gpl-2.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSPEECHRECOGNITIONENGINE_H
#define QSPEECHRECOGNITIONENGINE_H

#include "qspeechrecognition_global.h"

#include <QtCore/QObject>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QVariant>

QT_BEGIN_NAMESPACE

class QSPEECHRECOGNITION_EXPORT QSpeechRecognitionEngine : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool created READ isCreated NOTIFY created)

protected:
    explicit QSpeechRecognitionEngine(QObject *parent = 0);
    ~QSpeechRecognitionEngine();

public:
    virtual QString name() const = 0;
    Q_INVOKABLE virtual bool setParameter(const QString &key, const QVariant &value) = 0;
    Q_INVOKABLE virtual QVariant parameter(const QString &key) const = 0;
    Q_INVOKABLE virtual QList<QString> supportedParameters() const = 0;
    virtual bool isCreated() = 0;

    // Common engine parameter keys:
    static const QString Locale;
    static const QString Dictionary;
    static const QString ResourceDirectory;
    static const QString DataDirectory;
    static const QString DebugAudioDirectory;
    static const QString AudioSampleRate;
    static const QString AudioInputFile;
    static const QString AudioInputDevice;
    static const QString AudioInputDevices;

Q_SIGNALS:
    void created();
};

QT_END_NAMESPACE

#endif // QSPEECHRECOGNITIONENGINE_H
