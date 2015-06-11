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

#ifndef QSPEECHRECOGNITIONPLUGINLOADER_P_H
#define QSPEECHRECOGNITIONPLUGINLOADER_P_H

#include "qspeechrecognition.h"
#include <QtCore/QObject>
#include <QtCore/QJsonObject>
#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

class QFactoryLoader;
class QSpeechRecognitionPlugin;
class QSpeechRecognitionPluginGrammar;
class QSpeechRecognitionPluginEngine;

class QSpeechRecognitionPluginLoader : public QObject
{
    Q_OBJECT
public:
    QSpeechRecognitionPluginLoader(const QString &provider, QObject *parent = 0, bool allowExperimental = false);
    ~QSpeechRecognitionPluginLoader();
    QSpeechRecognitionPluginEngine *createEngine(const QString &name, const QVariantMap &parameters, QString *errorString);

private:
    static QMutex m_mutex;
    static QFactoryLoader m_loader;
    QString m_provider;
    QJsonObject m_metaData;
    QSpeechRecognitionPlugin *m_plugin;
    bool m_allowExperimental;

    void loadMeta(const QString &providerName);
    static QHash<QString, QJsonObject> plugins(bool reload = false);
    static void loadPluginMetadata(QHash<QString, QJsonObject> &list);
};

QT_END_NAMESPACE

#endif
