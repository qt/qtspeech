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

#include "qspeechrecognitionmanager_p.h"
#include "qspeechrecognitionplugin.h"
#include "qspeechrecognitionpluginloader_p.h"
#include "qspeechrecognitionpluginengine.h"
#include "qspeechrecognitionplugingrammar.h"

#include <QLoggingCategory>
#include <QFile>
#include <QTimer>

QT_BEGIN_NAMESPACE

QSpeechRecognitionManager::QSpeechRecognitionManager():
    m_session(0),
    m_listening(false),
    m_muted(false),
    m_exiting(false),
    m_grammar()
{
}

QSpeechRecognitionManager::~QSpeechRecognitionManager()
{
    foreach (QSpeechRecognitionPluginEngine* engine, m_engines)
        engine->reset();
    // Clear the containers and set the exiting flag to prevent access to the objects
    // while they are being destroyed. This could happen if any of the children allow
    // the events to be processed during destruction, which in turn may cause calls to
    // QSpeechRecognitionManager API (e.g. createGrammar()).
    m_engineLoaders.clear();
    m_engines.clear();
    m_grammars.clear();
    m_grammar = GrammarInfo();
    m_listening = false;
    m_exiting = true;
    qDeleteAll(findChildren<QSpeechRecognitionPluginLoader*>());
}

QSpeechRecognitionManager::AttributeData QSpeechRecognitionManager::getAttribute(const QString &key)
{
    QMutexLocker lock(&m_attributeMutex);
    m_updatedAttributes.remove(key);
    return m_attributes.value(key, AttributeData());
}

void QSpeechRecognitionManager::init()
{
}

void QSpeechRecognitionManager::setSession(int session)
{
    QMutexLocker lock(&m_attributeMutex);
    m_session = session;
    // Remove old session attributes. Not strictly necessary for other than keeping m_updatedAttributes up-to-date.
    QMap<QString, AttributeData>::iterator i = m_attributes.begin();
    while (i != m_attributes.end()) {
        const AttributeData &data = i.value();
        if (data.session != NO_SESSION) {
            m_updatedAttributes.remove(i.key());
            i = m_attributes.erase(i);
        } else {
            ++i;
        }
    }
}

void QSpeechRecognitionManager::createEngine(const QString &engineName, const QString &provider, const QVariantMap &parameters)
{
    QVariantMap errorParams;
    errorParams.insert(QSpeechRecognition::Engine, engineName);
    if (m_exiting)
        return;
    if (!m_engines.contains(engineName)) {
        QSpeechRecognitionPluginEngine *engine = 0;
        QSpeechRecognitionPluginLoader *engineLoader = m_engineLoaders.value(provider, 0);
        if (!engineLoader) {
            engineLoader = new QSpeechRecognitionPluginLoader(provider, this, true);
        }
        QString errorString;
        if (engineLoader && (engine = engineLoader->createEngine(engineName, parameters, &errorString)) != 0) {
            connect(engine, &QSpeechRecognitionPluginEngine::requestProcess, this, &QSpeechRecognitionManager::onRequestProcess);
            connect(engine, &QSpeechRecognitionPluginEngine::result, this, &QSpeechRecognitionManager::onResult);
            connect(engine, &QSpeechRecognitionPluginEngine::attributeUpdated, this, &QSpeechRecognitionManager::onAttributeUpdated);
            // Force handling of the following signals to be asynchronous
            connect(engine, &QSpeechRecognitionPluginEngine::requestStop, this, &QSpeechRecognitionManager::onRequestStop, Qt::QueuedConnection);
            connect(engine, &QSpeechRecognitionPluginEngine::error, this, &QSpeechRecognitionManager::onError, Qt::QueuedConnection);
            m_engines.insert(engineName, engine);
            m_engineLoaders.insert(provider, engineLoader);
            const QVariantMap &engineParams = engine->parameters();
            // Set initial values of all the engine parameters
            for (QVariantMap::const_iterator param = engineParams.begin(); param != engineParams.end(); ++param)
                emit engineParameterUpdated(engineName, param.key(), param.value());
            emit engineCreated(engineName);
        } else {
            delete engineLoader;
            if (!errorString.isEmpty())
                errorParams.insert(QSpeechRecognition::Reason, errorString);
            emit error(NO_SESSION, QSpeechRecognition::EngineInitError, errorParams);
        }
    } else {
        errorParams.insert(QSpeechRecognition::Reason, QLatin1String("Engine with the given name already exists"));
        emit error(NO_SESSION, QSpeechRecognition::EngineInitError, errorParams);
    }
}

void QSpeechRecognitionManager::createGrammar(const QString &engineName, const QString &grammarName, const QUrl &location)
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::createGrammar()";
    QVariantMap errorParams;
    errorParams.insert(QSpeechRecognition::Engine, engineName);
    errorParams.insert(QSpeechRecognition::Grammar, grammarName);
    if (!m_grammars.contains(grammarName)) {
        QSpeechRecognitionPluginEngine* engine = m_engines.value(engineName, 0);
        if (engine) {
            QString errorString;
            GrammarInfo grammar;
            grammar.engine = engine;
            grammar.grammar = grammar.engine->createGrammar(grammarName, location, &errorString);
            if (grammar.grammar) {
                m_grammars.insert(grammarName, grammar);
                emit grammarCreated(grammarName);
            } else {
                if (!errorString.isEmpty())
                    errorParams.insert(QSpeechRecognition::Reason, errorString);
                emit error(NO_SESSION, QSpeechRecognition::GrammarInitError, errorParams);
            }
        } else {
            errorParams.insert(QSpeechRecognition::Reason, QLatin1String("The given engine was not properly initialized"));
            emit error(NO_SESSION, QSpeechRecognition::GrammarInitError, errorParams);
        }
    } else {
        errorParams.insert(QSpeechRecognition::Reason, QLatin1String("Grammar with the given name already exists"));
        emit error(NO_SESSION, QSpeechRecognition::GrammarInitError, errorParams);
    }
}

void QSpeechRecognitionManager::deleteGrammar(const QString &grammarName)
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::deleteGrammar()";
    GrammarInfo grammar = m_grammars.value(grammarName, GrammarInfo());
    if (grammar.grammar) {
        Q_ASSERT(grammar.grammar != m_grammar.grammar); // This case is handled on the upper level
        m_grammars.remove(grammarName);
        delete grammar.grammar;
    }
}

void QSpeechRecognitionManager::setGrammar(const QString &grammarName)
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::setGrammar()";
    QVariantMap errorParams;
    errorParams.insert(QSpeechRecognition::Grammar, grammarName);
    GrammarInfo grammar = m_grammars.value(grammarName, GrammarInfo());
    if (m_grammar.engine && grammar.engine && grammar.engine != m_grammar.engine)
        m_grammar.engine->reset(); // Make sure any audio resources are released
    else if (m_listening)
        m_grammar.engine->abortListening();
    if (m_listening) {
        m_grammar.grammar = 0;
        m_listening = false;
    }
    emit notListening(m_session);
    if (grammar.grammar) {
        if (m_grammar.grammar != grammar.grammar) {
            QString errorString;
            QSpeechRecognition::Error errorCode = grammar.engine->setGrammar(grammar.grammar, &errorString);
            if (errorCode == QSpeechRecognition::NoError) {
                m_grammar = grammar;
            } else {
                if (!errorString.isEmpty())
                    errorParams.insert(QSpeechRecognition::Reason, errorString);
                emit error(m_session, errorCode, errorParams);
            }
        }
    } else if (m_grammar.grammar) {
        m_grammar.grammar = 0;
    }
}

void QSpeechRecognitionManager::startListening()
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::startListening()";
    if (!m_grammar.grammar) {
        emit notListening(m_session);
        emit error(m_session, QSpeechRecognition::NoGrammarError, QVariantMap());
    } else if (!m_listening) {
        QString errorString;
        QSpeechRecognition::Error errorCode = m_grammar.engine->startListening(m_session, m_muted, &errorString);
        if (errorCode == QSpeechRecognition::NoError) {
            m_listening = true;
            if (m_muted)
                emit listeningMuted(m_session);
            else
                emit listeningStarted(m_session);
        } else {
            emit notListening(m_session);
            QVariantMap errorParams;
            errorParams.insert(QSpeechRecognition::Engine, m_grammar.engine->name());
            if (!errorString.isEmpty())
                errorParams.insert(QSpeechRecognition::Reason, errorString);
            emit error(m_session, errorCode, errorParams);
        }
    }
}

void QSpeechRecognitionManager::stopListening(qint64 timestamp)
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::stopListening()";
    if (m_grammar.grammar && m_listening) {
        m_listening = false;
        if (!m_muted) {
            // Sending processing-notification first allows engine->stopListening() to send the final result synchronously.
            emit processingStarted(m_session);
            m_grammar.engine->stopListening(timestamp);
        } else {
            m_grammar.engine->abortListening();
            emit notListening(m_session);
        }
    } else {
        emit notListening(m_session);
    }
}

void QSpeechRecognitionManager::abortListening()
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::abortListening()";
    // Call engine->abortListening() even if m_listening is false, as the previous call
    // may have been stopListening() and the engine may still be doing some post-processing.
    if (m_grammar.grammar) {
        m_grammar.engine->abortListening();
    }
    m_listening = false;
    emit notListening(m_session);
}

void QSpeechRecognitionManager::mute()
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::mute()";
    if (m_grammar.grammar) {
        m_grammar.engine->abortListening();
    }
    m_listening = false;
    m_muted = true;
    emit notListening(m_session);
}

void QSpeechRecognitionManager::unmute(qint64 timestamp)
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::unmute()";
    if (m_grammar.grammar && m_listening && m_muted) {
        m_grammar.engine->unmute(timestamp);
        emit listeningStarted(m_session);
    }
    m_muted = false;
}

void QSpeechRecognitionManager::reset()
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::reset()";
    if (m_grammar.engine) {
        m_grammar.engine->reset();
        m_grammar = GrammarInfo();
    }
    m_listening = false;
    emit notListening(m_session);
}

void QSpeechRecognitionManager::dispatchMessage(const QString &engineName, const QString &messageText, const QVariantMap &parameters)
{
    if (engineName.isEmpty()) {
        emit message(messageText, parameters);
    }
}

void QSpeechRecognitionManager::setEngineParameter(const QString &engineName, const QString &key, const QVariant &value)
{
    QSpeechRecognitionPluginEngine* engine = m_engines.value(engineName, 0);
    if (engine) {
        QString errorString;
        QSpeechRecognition::Error errorCode = engine->setParameter(key, value, &errorString);
        if (errorCode == QSpeechRecognition::NoError) {
            emit engineParameterUpdated(engineName, key, value);
        } else {
            QVariantMap errorParams;
            errorParams.insert(QSpeechRecognition::Engine, engineName);
            if (!errorString.isEmpty())
                errorParams.insert(QSpeechRecognition::Reason, errorString);
            emit error(m_session, errorCode, errorParams);
        }
    }
}

void QSpeechRecognitionManager::onProcess()
{
    //qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::onProcess()";
    QSet<QSpeechRecognitionPluginEngine*> engines = m_enginesToProcess;
    m_enginesToProcess.clear();
    foreach (QSpeechRecognitionPluginEngine* engine, engines) {
        if (engine->process()) {
            scheduleProcess(engine);
        }
    }
}

void QSpeechRecognitionManager::scheduleProcess(QSpeechRecognitionPluginEngine *engine)
{
    if (m_enginesToProcess.isEmpty()) {
        QTimer::singleShot(0, this, SLOT(onProcess()));
    }
    m_enginesToProcess.insert(engine);
}

void QSpeechRecognitionManager::onResult(int session, const QSpeechRecognitionPluginGrammar *grammar, const QVariantMap &resultData)
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::onResult()";
    if (session == m_session)
        emit result(session, grammar->name(), resultData);
}

void QSpeechRecognitionManager::onError(int session, QSpeechRecognition::Error errorCode, const QVariantMap &parameters)
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::onError()";
    if (session == m_session || session == NO_SESSION) {
        if (session == m_session && m_listening) {
            m_grammar.engine->abortListening();
            emit notListening(m_session);
        }
        emit error(session, errorCode, parameters);
    }
}

void QSpeechRecognitionManager::onRequestProcess()
{
    QSpeechRecognitionPluginEngine *engine = qobject_cast<QSpeechRecognitionPluginEngine *>(QObject::sender());
    if (engine)
        scheduleProcess(engine);
}

void QSpeechRecognitionManager::onRequestStop(int session, qint64 timestamp)
{
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionManager::onRequestStop()";
    if (m_listening && (session == m_session || session == NO_SESSION)) {
        QSpeechRecognitionPluginEngine *engine = qobject_cast<QSpeechRecognitionPluginEngine *>(QObject::sender());
        if (engine && m_grammar.engine == engine)
            stopListening(timestamp);
    }
}

void QSpeechRecognitionManager::onAttributeUpdated(int session, const QString &key, const QVariant &value)
{
    if (!value.isValid())
        return;
    if (session == m_session || session == NO_SESSION) {
        bool updated = false;
        m_attributeMutex.lock();
        AttributeData data = m_attributes.value(key, AttributeData());
        if (data.session != session || data.value != value) {
            data.session = session;
            data.value = value;
            m_attributes.insert(key, data);
            if (!m_updatedAttributes.contains(key)) {
                m_updatedAttributes.insert(key);
                updated = true;
            }
        }
        m_attributeMutex.unlock();
        if (updated) {
            emit attributeUpdated(key);
        }
    }
}

QT_END_NAMESPACE
