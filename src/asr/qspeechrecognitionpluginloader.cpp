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

#include "qspeechrecognitionpluginloader_p.h"
#include "qspeechrecognitionplugin.h"

#include <QtCore/private/qfactoryloader_p.h>
#include <QFile>
#include <QDir>
#include <QStringList>
#include <QJsonObject>

QT_BEGIN_NAMESPACE

QMutex QSpeechRecognitionPluginLoader::m_mutex;
QFactoryLoader QSpeechRecognitionPluginLoader::m_loader("org.qt-project.qt.speech.asr.plugin/5.0", QLatin1String("/speechrecognition"));

QSpeechRecognitionPluginLoader::QSpeechRecognitionPluginLoader(const QString &provider, QObject *parent, bool allowExperimental)
    : QObject(parent),
      m_provider(provider),
      m_plugin(0),
      m_allowExperimental(allowExperimental)
{
    loadMeta(m_provider);
}

QSpeechRecognitionPluginEngine *QSpeechRecognitionPluginLoader::createEngine(const QString &name, const QVariantMap &parameters, QString *errorString)
{
    QMutexLocker lock(&m_mutex);
    if (int(m_metaData.value(QLatin1String("index")).toDouble()) < 0) {
        if (errorString)
            *errorString = QString(QLatin1String("Speech recognition plugin \"%1\" is not supported.")).arg(m_provider);
        return 0;
    }
    int idx = int(m_metaData.value(QLatin1String("index")).toDouble());
    QObject *inst = m_loader.instance(idx);
    m_plugin = qobject_cast<QSpeechRecognitionPlugin *>(inst);
    if (m_plugin)
        return m_plugin->createSpeechRecognitionEngine(name, parameters, this, errorString);
    else if (errorString)
        *errorString = QLatin1String("Speech recognition plugin \"") + m_provider + QLatin1String("\" could not be loaded.");
    return 0;
}

QSpeechRecognitionPluginLoader::~QSpeechRecognitionPluginLoader()
{
}

void QSpeechRecognitionPluginLoader::loadMeta(const QString &providerName)
{
    m_plugin = 0;
    m_metaData = QJsonObject();
    m_metaData.insert(QLatin1String("index"), -1);

    QList<QJsonObject> candidates = QSpeechRecognitionPluginLoader::plugins().values(providerName);

    int versionFound = -1;
    int idx = -1;

    // figure out which version of the plugin we want
    // (always latest unless experimental)
    for (int i = 0; i < candidates.size(); ++i) {
        QJsonObject meta = candidates[i];
        if (meta.contains(QLatin1String("Version"))
        && meta.value(QLatin1String("Version")).isDouble()
        && meta.contains(QLatin1String("Experimental"))
        && meta.value(QLatin1String("Experimental")).isBool()) {
            int ver = int(meta.value(QLatin1String("Version")).toDouble());
            if (ver > versionFound && (m_allowExperimental || !meta.value(QLatin1String("Experimental")).toBool())) {
                versionFound = ver;
                idx = i;
            }
        }
    }

    if (idx != -1) {
        m_metaData = candidates[idx];
    }
}

QHash<QString, QJsonObject> QSpeechRecognitionPluginLoader::plugins(bool reload)
{
    static QHash<QString, QJsonObject> plugins;
    static bool alreadyDiscovered = false;
    QMutexLocker lock(&m_mutex);

    if (reload == true)
        alreadyDiscovered = false;

    if (!alreadyDiscovered) {
        loadPluginMetadata(plugins);
        alreadyDiscovered = true;
    }
    return plugins;
}

void QSpeechRecognitionPluginLoader::loadPluginMetadata(QHash<QString, QJsonObject> &list)
{
    QList<QJsonObject> meta = m_loader.metaData();
    for (int i = 0; i < meta.size(); ++i) {
        QJsonObject obj = meta.at(i).value(QLatin1String("MetaData")).toObject();
        obj.insert(QLatin1String("index"), i);
        list.insertMulti(obj.value(QLatin1String("Provider")).toString(), obj);
    }
}

QT_END_NAMESPACE
