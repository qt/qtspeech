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

#ifndef QSPEECHRECOGNITIONMANAGER_P_H
#define QSPEECHRECOGNITIONMANAGER_P_H

#include "qspeechrecognition.h"
#include "qspeechrecognitionpluginengine.h"

#include <QtCore/QObject>
#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QVariantMap>
#include <QtCore/QSet>
#include <QtCore/QMutex>
#include <QtCore/QLoggingCategory>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcSpeechAsr)

class QSpeechRecognitionPluginLoader;
class QSpeechRecognitionPluginEngine;
class QSpeechRecognitionPluginGrammar;

class QSpeechRecognitionManager : public QObject
{
    Q_OBJECT
public:
    struct AttributeData {
        int session;
        QVariant value;
    };
    enum {
        NO_SESSION = QSpeechRecognitionPluginEngine::NO_SESSION
    };
    explicit QSpeechRecognitionManager();
    ~QSpeechRecognitionManager();
    AttributeData getAttribute(const QString &key);

signals:
    void engineCreated(const QString &engineName);
    void grammarCreated(const QString &grammarName);
    void listeningStarted(int session);
    void listeningMuted(int session);
    void processingStarted(int session);
    void notListening(int session);
    void message(const QString &messageText, const QVariantMap &parameters);
    void result(int session, const QString &grammarName, const QVariantMap &resultData);
    void error(int session, QSpeechRecognition::Error errorCode, const QVariantMap &parameters);
    void attributeUpdated(const QString &key);
    void engineParameterUpdated(const QString &engineName, const QString &key, const QVariant &value);

public slots:
    void init();
    void setSession(int session);
    void createEngine(const QString &engineName, const QString &provider, const QVariantMap &parameters);
    void createGrammar(const QString &engineName, const QString &grammarName, const QUrl &location);
    void deleteGrammar(const QString &grammarName);
    void setGrammar(const QString &grammarName);
    void startListening();
    void stopListening(qint64 timestamp);
    void abortListening();
    void mute();
    void unmute(qint64 timestamp);
    void reset();
    void dispatchMessage(const QString &engineName, const QString &message, const QVariantMap &parameters);
    void setEngineParameter(const QString &engineName, const QString &key, const QVariant &value);

private slots:
    void onProcess();
    void onRequestProcess();
    void onRequestStop(int session, qint64 timestamp);
    void onResult(int session, const QSpeechRecognitionPluginGrammar *grammar, const QVariantMap &resultData);
    void onError(int session, QSpeechRecognition::Error errorCode, const QVariantMap &parameters);
    void onAttributeUpdated(int session, const QString &key, const QVariant &value);

private:
    void scheduleProcess(QSpeechRecognitionPluginEngine *engine);

private:
    struct GrammarInfo {
        QSpeechRecognitionPluginEngine *engine;
        QSpeechRecognitionPluginGrammar *grammar;
    };
    int m_session;
    bool m_listening;
    bool m_muted;
    bool m_exiting;
    GrammarInfo m_grammar;
    QMap<QString, AttributeData> m_attributes;
    QSet<QString> m_updatedAttributes;
    mutable QMutex m_attributeMutex;
    QMap<QString, QSpeechRecognitionPluginLoader*> m_engineLoaders;
    QMap<QString, QSpeechRecognitionPluginEngine*> m_engines;
    QMap<QString, GrammarInfo> m_grammars;
    QSet<QSpeechRecognitionPluginEngine*> m_enginesToProcess;
};

QT_END_NAMESPACE

#endif // QSPEECHRECOGNITIONMANAGER_P_H
