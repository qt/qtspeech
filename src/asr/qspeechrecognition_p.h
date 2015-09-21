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

#ifndef QSPEECHRECOGNITION_P_H
#define QSPEECHRECOGNITION_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qspeechrecognition.h"
#include "qspeechrecognitionengine_p.h"

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QJsonObject>
#include <QtCore/QThread>
#include <QtCore/QTimer>
#include <QtCore/QLoggingCategory>
#include <QtCore/private/qobject_p.h>

QT_BEGIN_NAMESPACE

Q_DECLARE_LOGGING_CATEGORY(lcSpeechAsr)

class QSpeechRecognitionManager;
class QSpeechRecognitionGrammarImpl;
class QSpeechRecognitionManagerInterface;

class QSpeechRecognitionPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QSpeechRecognition)

public:
    QSpeechRecognitionPrivate();
    ~QSpeechRecognitionPrivate();

    // Methods called by QSpeechRecognitionManagerInterface
    void onEngineCreated(const QString &engineName);
    void onGrammarCreated(const QString &grammarName);
    void onListeningStarted(int session);
    void onListeningMuted(int session);
    void onProcessingStarted(int session);
    void onNotListening(int session);
    void onResult(int session, const QString &grammarName, const QVariantMap &resultData);
    void onError(int session, QSpeechRecognition::Error errorCode, const QVariantMap &parameters);
    void onAttributeUpdated(const QString &key);
    void onUnmuteTimeout();
    void onEngineParameterUpdated(const QString &engineName, const QString &key, const QVariant &value);

private:
    void setState(QSpeechRecognition::State state);
    QSpeechRecognitionManagerInterface *m_managerInterface;
    int m_session;
    bool m_muted;
    QSpeechRecognition::State m_state;
    QThread *m_managerThread;
    QSpeechRecognitionManager *m_manager;
    QSpeechRecognitionEngineImpl *m_engine;
    QSpeechRecognitionGrammarImpl *m_grammar;
    QMap<QString, QSpeechRecognitionEngineImpl *> m_engines;
    QMap<QString, QSpeechRecognitionGrammarImpl *> m_grammars;
    QTimer m_unmuteTimer;
    qint64 m_unmuteTime;
};

class QSpeechRecognitionManagerInterface: public QObject
{
    Q_OBJECT

public:
    QSpeechRecognitionManagerInterface(QSpeechRecognitionPrivate *speech):
        m_speech(speech)
    {
    }

public slots:
    void onEngineCreated(const QString &engineName)
    {
        m_speech->onEngineCreated(engineName);
    }
    void onGrammarCreated(const QString &grammarName)
    {
        m_speech->onGrammarCreated(grammarName);
    }
    void onListeningStarted(int session)
    {
        m_speech->onListeningStarted(session);
    }
    void onListeningMuted(int session)
    {
        m_speech->onListeningMuted(session);
    }
    void onProcessingStarted(int session)
    {
        m_speech->onProcessingStarted(session);
    }
    void onNotListening(int session)
    {
        m_speech->onNotListening(session);
    }
    void onResult(int session, const QString &grammarName, const QVariantMap &resultData)
    {
        m_speech->onResult(session, grammarName, resultData);
    }
    void onError(int session, QSpeechRecognition::Error errorCode, const QVariantMap &parameters)
    {
        m_speech->onError(session, errorCode, parameters);
    }
    void onAttributeUpdated(const QString &key)
    {
        m_speech->onAttributeUpdated(key);
    }
    void onEngineParameterUpdated(const QString &engineName, const QString &key, const QVariant &value)
    {
        m_speech->onEngineParameterUpdated(engineName, key, value);
    }
    void onUnmuteTimeout()
    {
        m_speech->onUnmuteTimeout();
    }
    void onSetEngineParameter(const QString &key, const QVariant &value)
    {
        QSpeechRecognitionEngineImpl *engine = qobject_cast<QSpeechRecognitionEngineImpl *>(QObject::sender());
        if (engine)
            emit setEngineParameter(engine->name(), key, value);
    }

signals:
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

private:
    QSpeechRecognitionPrivate *m_speech;
};

QT_END_NAMESPACE

#endif
