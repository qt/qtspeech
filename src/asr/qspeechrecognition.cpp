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

#include "qspeechrecognition.h"
#include "qspeechrecognition_p.h"
#include "qspeechrecognitionengine_p.h"
#include "qspeechrecognitiongrammar_p.h"
#include "qspeechrecognitionmanager_p.h"

#include <QDateTime>

QT_BEGIN_NAMESPACE

Q_LOGGING_CATEGORY(lcSpeechAsr, "qt.speech.asr")

/*
    When these conditions are satisfied, QStringLiteral is implemented by
    gcc's statement-expression extension.  However, in this file it will
    not work, because "statement-expressions are not allowed outside functions
    nor in template-argument lists".
    MSVC 2012 produces an internal compiler error on encountering
    QStringLiteral in this context.

    Fall back to the less-performant QLatin1String in this case.
*/
#if defined(Q_CC_GNU) && defined(Q_COMPILER_LAMBDA)
#    define Q_DEFINE_ASR_ATTRIBUTE(key) const QString QSpeechRecognition::key(QStringLiteral(#key))
#    define Q_DEFINE_ASR_ERROR_PARAMETER(key) const QString QSpeechRecognition::key(QStringLiteral(#key))
#    define Q_DEFINE_ASR_RESULT_PARAMETER(key) const QString QSpeechRecognition::key(QStringLiteral(#key))
#else
#    define Q_DEFINE_ASR_ATTRIBUTE(key) const QString QSpeechRecognition::key(QLatin1String(#key))
#    define Q_DEFINE_ASR_ERROR_PARAMETER(key) const QString QSpeechRecognition::key(QLatin1String(#key))
#    define Q_DEFINE_ASR_RESULT_PARAMETER(key) const QString QSpeechRecognition::key(QLatin1String(#key))
#endif

// Run-time attributes:

/*! \variable QSpeechRecognition::AudioLevel

    This constant is used as the key for a speech recognition run-time attribute.
    See attributeUpdated().
*/
Q_DEFINE_ASR_ATTRIBUTE(AudioLevel);

// Error parameters:

/*! \variable QSpeechRecognition::Reason

    This constant is used as the key for a speech recognition error parameter.
    See error().
*/
Q_DEFINE_ASR_ERROR_PARAMETER(Reason);

/*! \variable QSpeechRecognition::Engine

    This constant is used as the key for a speech recognition error parameter.
    See error().
*/
Q_DEFINE_ASR_ERROR_PARAMETER(Engine);

/*! \variable QSpeechRecognition::Grammar

    This constant is used as the key for a speech recognition error parameter.
    See error().
*/
Q_DEFINE_ASR_ERROR_PARAMETER(Grammar);

// Result parameters:

/*! \variable QSpeechRecognition::Transcription

    This constant is used as the key for a speech recognition result parameter.
    See result().
*/
Q_DEFINE_ASR_RESULT_PARAMETER(Transcription);

/*!
  \class QSpeechRecognition
  \inmodule QtSpeechRecognition
  \brief The QSpeechRecognition class provides access to speech recognition engines

  \reentrant

  Use \l createEngine() to load and initialize a speech recognition engine from a plug-in.
  One or more grammars should to be created with \l createGrammar() and the
  active grammar can then be selected using \l setActiveGrammar().
  Use \l startListening() to begin listening for a command and \l stopListening() to mark the
  end-of-command.

  Unless otherwise mentioned, all the methods in this API return immediately and are handled
  asynchronously (in the call order).

  \section1 Initialization

  The following example creates a QSpeechRecognition instance with PocketSphinx engine
  and creates one grammar. The acoustic model and dictionary for the given locale are
  assumed to be found in the default resource directory for the engine.

  \snippet asr/speechrecognition.cpp Init

  \section1 Basic Control

  The following example method starts and stops speech recognition for the grammar that
  was created above:

  \snippet asr/speechrecognition.cpp Start and stop

  After calling \l {QSpeechRecognition::}{startListening()}, the application receives a
  \l {QSpeechRecognition::}{listeningStarted()} signal as soon as audio recording has been
  started.

  After calling \l {QSpeechRecognition::}{stopListening()}, the application first receives a
  \l {QSpeechRecognition::}{listeningStopped()} signal. Later, if a recognition result is
  available, a \l {QSpeechRecognition::}{result()} signal is received.
*/

/*!
  \enum QSpeechRecognition::Error

  Error code, delivered with signal \l error():

  \value EngineInitError        Recognizer engine initialization failed.
                                Further attempt to use the engine will result to an error.
  \value EngineParameterError   Setting engine run-time parameter failed.
  \value EngineError            An engine-related operation failed.
  \value GrammarInitError       Grammar initialization failed.
                                Further attempt to use the grammar will result to an error.
  \value GrammarError           A grammar-related operation failed.
  \value AudioError             Error using the audio input.
  \value ConnectionError        Network connection is needed but cannot be established.
  \value NoGrammarError         Attempting to start recognition without a valid grammar.
  \value NoResultError          Recognition did not produce a result.

  \omitvalue NoError
*/

/*!
  \enum QSpeechRecognition::State

  Current state of the speech recognizer:

  \value IdleState              Recognizer is not currently doing anything.
  \value ListeningStartingState Recognizer has been commanded to start listening using the
                                selected grammar.
  \value ListeningState         Recognizer is listening for a command in the selected grammar.
  \value ListeningStoppingState Recognizer has been commanded to stop listening.
  \value ProcessingState        Recognizer has stopped listening and is processing for the
                                final result.
  \value MutedState             Recognizer engine is running but the audio input is muted.
                                Listening will start immediately after the mute is released.
*/

/*!
  \property QSpeechRecognition::state
  The current state of the speech recognizer.

  The notification signal stateChanged() is always emitted asynchronously (through
  queued connection).
*/

/*!
  \property QSpeechRecognition::muted
  Whether the speech recognizer audio input is muted.

  Enabling mute aborts listening, similarly to calling \l abortListening().
*/

/*!
  \property QSpeechRecognition::activeGrammar
  The currently active grammar, or 0 if no grammar is active.
*/

/*!
  \fn void QSpeechRecognition::error(QSpeechRecognition::Error errorCode, const QVariantMap &parameters)

  Emitted when an error occurred. The type of the error is given in \a errorCode.
  The signal may include additional \a parameters that describe the error in more detail.

  The following table describes some common error parameters:

  \table
  \header
    \li Key
    \li Value type
    \li Error codes that use the parameter
    \li Description
  \row
    \li \l Reason
    \li QString
    \li any
    \li Human-readable description of the error
  \endtable

*/

/*!
  \fn void QSpeechRecognition::result(const QString &grammarName, const QVariantMap &resultData)

  Emitted when a speech recognition result is available. Parameter \a grammarName
  specifies the grammar that produced the result. Parameter \a resultData minimally includes
  the transcription of what was recognized, but often also contains other useful information.

  The following table describes some common fields in the result data:

  \table
  \header
    \li Key
    \li Value type
    \li Description
  \row
    \li \l Transcription
    \li QString
    \li Transcription of what was recognized
  \endtable

  The signal is never emitted if the recognition session has been aborted, e.g. by
  calling \l abortListening() or \l reset(), by enabling mute, or by switching the grammar.
*/

/*!
  \fn void QSpeechRecognition::listeningStarted(const QString &grammarName)

  Emitted when the recognizer has started listening for commands in the grammar specified
  by \a grammarName.

  The signal is never emitted if the recognition session has already been aborted, e.g. by
  calling \l abortListening() or \l reset(), by enabling mute, or by switching the grammar.
*/

/*!
  \fn void QSpeechRecognition::listeningStopped(bool expectResult)

  Emitted when the recognizer has stopped listening for commands. If parameter \a expectResult
  is true, the application should expect either a \l result(), or \l error() with code
  \l QSpeechRecognition::NoResultError. However, if the session is aborted by the application
  before that, neither a result nor the error will be delivered.

  The signal is only emitted if \l listeningStarted() was emitted for the same recognition session.
  The signal is never emitted if the session was aborted and a new recognition session has
  already been started. In this case, the application will receive another \l listeningStarted()
  signal without an intervening \l listeningStopped().
*/

/*!
  \fn void QSpeechRecognition::attributeUpdated(const QString &key, const QVariant &value)

  Emitted after the recognizer has updated a run-time attribute (specified by parameter
  \a key), including the latest \a value of the attribute. If the engine has done multiple
  updates to the same attribute after this signal was previously emitted, only one notification
  with the latest value is sent.

  The following table describes some commonly used run-time attributes:

  \table
  \header
    \li Key
    \li Value type
    \li Description
  \row
    \li \l AudioLevel
    \li qreal
    \li Audio level between 0.0 and 1.0. Only updated when listening for commands.
  \endtable
*/

/*!
  \fn void QSpeechRecognition::message(const QString &message, const QVariantMap &parameters)

  Delivers the \a message that was queued using \l dispatchMessage(), together with
  the given \a parameters.

  The signal is emitted from the thread that runs the recognizer task queue.
*/

QSpeechRecognition::QSpeechRecognition(QObject *parent):
    QObject(*new QSpeechRecognitionPrivate(), parent)
{
    Q_D(QSpeechRecognition);
    if (d->m_manager) {
        // Messages are emitted directly from the manager thread to the application, which allows blocking when handling them
        connect(d->m_manager, &QSpeechRecognitionManager::message, this, &QSpeechRecognition::message, Qt::DirectConnection);
    }
}

QSpeechRecognition::~QSpeechRecognition()
{
}

QSpeechRecognition::State QSpeechRecognition::state() const
{
    Q_D(const QSpeechRecognition);
    return d->m_state;
}

/*!
  Creates new recognizer engine using the plug-in that matches \a providerName.
  The engine must be given a user-defined \a name.

  The engine may require \a parameters, some of which may be engine-specific. Unknown parameters are silently
  ignored. See QSpeechRecognitionEngine::supportedParameters() for the list of some commonly used
  parameters.

  Returns a pointer to the engine handle. QSpeechRecognition retains
  ownership of the returned pointer. The same pointer can be later retrieved with \l engine().
*/
QSpeechRecognitionEngine *QSpeechRecognition::createEngine(const QString &name, const QString &providerName, const QVariantMap &parameters)
{
    Q_D(QSpeechRecognition);
    if (d->m_engines.contains(name))
        return 0; // Already exists
    QSpeechRecognitionEngineImpl *engine = new QSpeechRecognitionEngineImpl(name, this);
    connect(engine, &QSpeechRecognitionEngineImpl::requestSetParameter, d->m_managerInterface, &QSpeechRecognitionManagerInterface::onSetEngineParameter);
    d->m_engines.insert(name, engine);
    emit d->m_managerInterface->createEngine(name, providerName, parameters);
    return engine;
}

/*!
  Gets a pointer to the engine handle with the given \a name, previously created with
  \l createEngine(). Returns 0 if no engine with the given \a name exists.
*/
QSpeechRecognitionEngine *QSpeechRecognition::engine(const QString& name) const
{
    Q_D(const QSpeechRecognition);
    return d->m_engines.value(name, 0);
}

/*!
  Creates a new grammar for the given \a engine, from the file pointed by \a location.
  The grammar must be given a user-defined \a name.

  If a grammar with the given name already exists, the grammar is deleted and recreated,
  and the existing grammar handle will point to the new grammar instance.

  If the \a location URL contains a relative file path, the grammar is loaded from
  the locale-specific sub-directory under the engine resource directory.
  The grammar can also be loaded from built-in Qt resources.
  The format of the grammar file is engine-specific and is automatically detected by the engine.

  Returns a pointer to the grammar handle. QSpeechRecognition retains ownership
  of the returned pointer. The same pointer can be later retrieved with \l grammar().
*/
QSpeechRecognitionGrammar *QSpeechRecognition::createGrammar(QSpeechRecognitionEngine *engine, const QString &name, const QUrl &location)
{
    Q_D(QSpeechRecognition);
    QSpeechRecognitionEngineImpl *enginePriv = qobject_cast<QSpeechRecognitionEngineImpl *>(engine);
    if (!engine || !enginePriv || name.isEmpty())
        return 0; // Invalid param
    if (engine->parent() != this) {
        qCCritical(lcSpeechAsr) << "QSpeechRecognition::createGrammar(): The given engine does not belong to this speech recognizer instance.";
        return 0;
    }
    QSpeechRecognitionGrammarImpl *grammar = d->m_grammars.value(name, 0);
    if (!grammar)
        grammar = new QSpeechRecognitionGrammarImpl(name, engine, this);
    else // Already exists -> recreate
        deleteGrammar(grammar);
    if (grammar) {
        grammar->setState(QSpeechRecognitionGrammar::InitializingState);
        d->m_grammars.insert(name, grammar);
        emit d->m_managerInterface->createGrammar(enginePriv->m_name, name, location);
        return grammar;
    }
    return 0;
}

/*!
  Gets a pointer to the grammar handle with the given \a name, previously created with
  \l createGrammar(). Returns 0 if the grammar does not exist.
*/
QSpeechRecognitionGrammar *QSpeechRecognition::grammar(const QString &name) const
{
    Q_D(const QSpeechRecognition);
    QSpeechRecognitionGrammarImpl *grammar = d->m_grammars.value(name, 0);
    if (grammar && grammar->state() == QSpeechRecognitionGrammar::DeletedState)
        return 0;
    return grammar;
}

/*!
  Releases all the resources for the given \a grammar. The grammar handle will be
  set to DeletedState and any further calls to \l grammar() with the associated
  grammar name will return 0.

  If the grammar is currently active, this call stops listening for commands and
  discards any results.
*/
void QSpeechRecognition::deleteGrammar(QSpeechRecognitionGrammar *grammar)
{
    Q_D(QSpeechRecognition);
    if (grammar && grammar->parent() != this) {
        qCCritical(lcSpeechAsr) << "QSpeechRecognition::deleteGrammar(): The given grammar does not belong to this speech recognizer instance.";
        return;
    }
    QSpeechRecognitionGrammarImpl *grammarPriv = qobject_cast<QSpeechRecognitionGrammarImpl *>(grammar);
    if (grammarPriv && grammarPriv->state() != QSpeechRecognitionGrammar::DeletedState) {
        QString grammarName = grammarPriv->name();
        if (d->m_grammar == grammarPriv) {
            reset();
        }
        emit d->m_managerInterface->deleteGrammar(grammarName);
        grammarPriv->setState(QSpeechRecognitionGrammar::DeletedState);
    }
}

/*!
  Sets the active \a grammar. If the parameter is 0, removes any active grammars.
  The call automatically stops listening for commands and discards any results.
*/
void QSpeechRecognition::setActiveGrammar(QSpeechRecognitionGrammar *grammar)
{
    Q_D(QSpeechRecognition);
    if (grammar && grammar->parent() != this) {
        qCCritical(lcSpeechAsr) << "QSpeechRecognition::setActiveGrammar(): The given grammar does not belong to this speech recognizer instance.";
        return;
    }
    QString grammarName;
    QSpeechRecognitionGrammarImpl *grammarPriv = qobject_cast<QSpeechRecognitionGrammarImpl *>(grammar);
    if (grammarPriv) {
        QSpeechRecognitionGrammar::State state = grammarPriv->state();
        // Handle invalid grammar as empty
        if (state != QSpeechRecognitionGrammar::DeletedState
        && state != QSpeechRecognitionGrammar::InvalidState
        && state != QSpeechRecognitionGrammar::ErrorState) {
            grammarName = grammarPriv->name();
        }
    }
    d->m_grammar = grammarPriv;
    d->m_session++;
    emit d->m_managerInterface->setSession(d->m_session);
    emit d->m_managerInterface->setGrammar(grammarName);
}

/*!
  Gets a pointer to the handle of the currently active grammar.
  Returns 0 if no grammar is active.
*/
QSpeechRecognitionGrammar *QSpeechRecognition::activeGrammar() const
{
    Q_D(const QSpeechRecognition);
    return d->m_grammar;
}

/*!
  Starts listening for commands in the selected grammar.
  If the recognizer is muted, listening starts when the mute is released.
  If listening is already ongoing or about to start, this call does nothing.
  If \l stopListening() has been called, discards any results and restarts listening.
*/
void QSpeechRecognition::startListening()
{
    Q_D(QSpeechRecognition);
    if (d->m_state == QSpeechRecognition::IdleState
    || d->m_state == QSpeechRecognition::ListeningStoppingState
    || d->m_state == QSpeechRecognition::ProcessingState) {
        d->m_session++;
        emit d->m_managerInterface->setSession(d->m_session);
        emit d->m_managerInterface->startListening();
        d->setState(QSpeechRecognition::ListeningStartingState);
    }
}

/*!
  Stops listening for commands and starts processing for the final result, if available.
  If listening has already been commanded to stop, this call does nothing.
*/
void QSpeechRecognition::stopListening()
{
    Q_D(QSpeechRecognition);
    if (d->m_state == QSpeechRecognition::ListeningStartingState
    || d->m_state == QSpeechRecognition::ListeningState
    || d->m_state == QSpeechRecognition::MutedState) {
        qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
        emit d->m_managerInterface->stopListening(timestamp);
        d->setState(QSpeechRecognition::ListeningStoppingState);
    }
}

/*!
  Stops listening for commands and discards any results.
  If \l stopListening() has been called, discards any results.
*/
void QSpeechRecognition::abortListening()
{
    Q_D(QSpeechRecognition);
    d->m_session++;
    emit d->m_managerInterface->setSession(d->m_session);
    emit d->m_managerInterface->abortListening();
    if (d->m_state == QSpeechRecognition::ListeningState
    || d->m_state == QSpeechRecognition::MutedState)
        d->setState(QSpeechRecognition::ListeningStoppingState);
    else if (d->m_state != QSpeechRecognition::ListeningStoppingState)
        d->setState(QSpeechRecognition::IdleState);
}

void QSpeechRecognition::setMuted(bool muted)
{
    Q_D(QSpeechRecognition);
    d->m_unmuteTimer.stop();
    d->m_unmuteTime = 0;
    if (d->m_muted != muted) {
        if (muted) {
            d->m_session++;
            emit d->m_managerInterface->setSession(d->m_session);
            emit d->m_managerInterface->mute();
        } else {
            qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
            emit d->m_managerInterface->unmute(timestamp);
        }
        d->m_muted = muted;
        emit muteChanged();
    }
}

bool QSpeechRecognition::isMuted()
{
    Q_D(QSpeechRecognition);
    return d->m_muted;
}

/*!
  Unmutes audio input after given number of milliseconds (parameter \a millisec).
  If \l setMuted() or \l unmuteAfter() is called before the given time has passed,
  the previous call to \l unmuteAfter() has no effect.
*/
void QSpeechRecognition::unmuteAfter(int millisec)
{
    Q_D(QSpeechRecognition);
    qint64 timestamp = QDateTime::currentMSecsSinceEpoch();
    d->m_unmuteTime = timestamp + millisec;
    d->m_unmuteTimer.start(millisec);
}

/*!
  \fn void QSpeechRecognition::reset()

  Stops listening for commands and discards any results.
  Any active grammar is removed and all the resources related to audio input are released.

  This method does not affect the mute state (returned by \l isMuted()).
*/
void QSpeechRecognition::reset()
{
    Q_D(QSpeechRecognition);
    d->m_session++;
    emit d->m_managerInterface->setSession(d->m_session);
    emit d->m_managerInterface->reset();
    d->m_grammar = 0;
    if (d->m_state == QSpeechRecognition::ListeningState
    || d->m_state == QSpeechRecognition::MutedState)
        d->setState(QSpeechRecognition::ListeningStoppingState);
    else if (d->m_state != QSpeechRecognition::ListeningStoppingState)
        d->setState(QSpeechRecognition::IdleState);
}

/*!
  \fn void QSpeechRecognition::dispatchMessage(const QString &message, const QVariantMap &parameters)

  Adds a custom \a message to the recognizer task queue. The message with the given
  \a parameters is delivered back to the application via signal \l message(), after the engine
  has handled all the previous tasks and messages.

  Messages allow the application to track the completion of slow initialization task(s).
  The application can also use a message as a checkpoint to see if everything went OK during
  initialization, instead of immediately reacting to a series of \l error() signals.

  As \l message() is emitted from the recognizer task thread, a direct connection to the signal
  can be used to force the recognizer to wait before handling the next task. However, please
  note that QSpeechRecognition is not thread-safe; the application is responsible for implementing
  mutual exclusion in this case.
*/
void QSpeechRecognition::dispatchMessage(const QString &message, const QVariantMap &parameters)
{
    Q_D(QSpeechRecognition);
    emit d->m_managerInterface->dispatchMessage(QString(), message, parameters);
}

/*******************************************************************************
*******************************************************************************/

QSpeechRecognitionPrivate::QSpeechRecognitionPrivate():
    m_managerInterface(new QSpeechRecognitionManagerInterface(this)),
    m_session(0),
    m_muted(false),
    m_state(QSpeechRecognition::IdleState),
    m_managerThread(new QThread()),
    m_manager(new QSpeechRecognitionManager()),
    m_engine(0),
    m_grammar(0),
    m_unmuteTime(0)
{
    m_unmuteTimer.setSingleShot(true);
    QObject::connect(m_managerThread, &QThread::started, m_manager, &QSpeechRecognitionManager::init);
    QObject::connect(m_managerThread, &QThread::finished, m_manager, &QSpeechRecognitionManager::deleteLater);
    QObject::connect(m_managerThread, &QThread::finished, m_managerThread, &QThread::deleteLater);
    QObject::connect(&m_unmuteTimer, &QTimer::timeout, m_managerInterface, &QSpeechRecognitionManagerInterface::onUnmuteTimeout);
    m_manager->moveToThread(m_managerThread);

    /*** Results and notifications from the manager: ***/
    qRegisterMetaType<QSpeechRecognition::Error>();
    QObject::connect(m_manager, &QSpeechRecognitionManager::notListening, m_managerInterface, &QSpeechRecognitionManagerInterface::onNotListening);
    QObject::connect(m_manager, &QSpeechRecognitionManager::error, m_managerInterface, &QSpeechRecognitionManagerInterface::onError);
    QObject::connect(m_manager, &QSpeechRecognitionManager::attributeUpdated, m_managerInterface, &QSpeechRecognitionManagerInterface::onAttributeUpdated);
    QObject::connect(m_manager, &QSpeechRecognitionManager::engineParameterUpdated, m_managerInterface, &QSpeechRecognitionManagerInterface::onEngineParameterUpdated);
    // Results and some other notifications from the manager are delivered synchronously.
    // This allows better synchronization of audio stream with the events, as
    // audio processing can be paused until results are handled.
    QObject::connect(m_manager, &QSpeechRecognitionManager::engineCreated, m_managerInterface, &QSpeechRecognitionManagerInterface::onEngineCreated, Qt::BlockingQueuedConnection);
    QObject::connect(m_manager, &QSpeechRecognitionManager::grammarCreated, m_managerInterface, &QSpeechRecognitionManagerInterface::onGrammarCreated, Qt::BlockingQueuedConnection);
    QObject::connect(m_manager, &QSpeechRecognitionManager::listeningStarted, m_managerInterface, &QSpeechRecognitionManagerInterface::onListeningStarted, Qt::BlockingQueuedConnection);
    QObject::connect(m_manager, &QSpeechRecognitionManager::listeningMuted, m_managerInterface, &QSpeechRecognitionManagerInterface::onListeningMuted, Qt::BlockingQueuedConnection);
    QObject::connect(m_manager, &QSpeechRecognitionManager::processingStarted, m_managerInterface, &QSpeechRecognitionManagerInterface::onProcessingStarted, Qt::BlockingQueuedConnection);
    QObject::connect(m_manager, &QSpeechRecognitionManager::result, m_managerInterface, &QSpeechRecognitionManagerInterface::onResult, Qt::BlockingQueuedConnection);

    /*** Commands to the manager (asynchronous): ***/
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::setSession, m_manager, &QSpeechRecognitionManager::setSession, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::createEngine, m_manager, &QSpeechRecognitionManager::createEngine, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::createGrammar, m_manager, &QSpeechRecognitionManager::createGrammar, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::deleteGrammar, m_manager, &QSpeechRecognitionManager::deleteGrammar, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::setGrammar, m_manager, &QSpeechRecognitionManager::setGrammar, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::startListening, m_manager, &QSpeechRecognitionManager::startListening, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::stopListening, m_manager, &QSpeechRecognitionManager::stopListening, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::abortListening, m_manager, &QSpeechRecognitionManager::abortListening, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::mute, m_manager, &QSpeechRecognitionManager::mute, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::unmute, m_manager, &QSpeechRecognitionManager::unmute, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::reset, m_manager, &QSpeechRecognitionManager::reset, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::dispatchMessage, m_manager, &QSpeechRecognitionManager::dispatchMessage, Qt::QueuedConnection);
    QObject::connect(m_managerInterface, &QSpeechRecognitionManagerInterface::setEngineParameter, m_manager, &QSpeechRecognitionManager::setEngineParameter, Qt::QueuedConnection);

    m_managerThread->start();
}

QSpeechRecognitionPrivate::~QSpeechRecognitionPrivate()
{
    m_managerThread->exit();
    delete m_managerInterface;
}

void QSpeechRecognitionPrivate::onEngineCreated(const QString &engineName)
{
    QSpeechRecognitionEngineImpl *engine = m_engines.value(engineName, 0);
    if (engine)
        engine->setCreated(true);
}

void QSpeechRecognitionPrivate::onGrammarCreated(const QString &grammarName)
{
    QSpeechRecognitionGrammarImpl *grammar = m_grammars.value(grammarName, 0);
    if (grammar && grammar->state() == QSpeechRecognitionGrammar::InitializingState)
        grammar->setState(QSpeechRecognitionGrammar::ReadyState);
}

void QSpeechRecognitionPrivate::onListeningStarted(int session)
{
    Q_Q(QSpeechRecognition);
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionPrivate::onListeningStarted()";
    if (session == m_session) {
        setState(QSpeechRecognition::ListeningState);
        emit q->listeningStarted(m_grammar->name());
    }
}

void QSpeechRecognitionPrivate::onListeningMuted(int session)
{
    Q_Q(QSpeechRecognition);
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionPrivate::onListeningMuted()";
    if (session == m_session) {
        QSpeechRecognition::State state = m_state;
        setState(QSpeechRecognition::MutedState);
        if (state == QSpeechRecognition::ListeningState)
            emit q->listeningStopped(false);
    }
}

void QSpeechRecognitionPrivate::onProcessingStarted(int session)
{
    Q_Q(QSpeechRecognition);
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionPrivate::onProcessingStarted()";
    if (session == m_session) {
        setState(QSpeechRecognition::ProcessingState);
        emit q->listeningStopped(true);
    }
}

void QSpeechRecognitionPrivate::onNotListening(int session)
{
    Q_Q(QSpeechRecognition);
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionPrivate::onNotListening()";
    if (session == m_session) {
        QSpeechRecognition::State state = m_state;
        setState(QSpeechRecognition::IdleState);
        if (state == QSpeechRecognition::ListeningState
        || state == QSpeechRecognition::ListeningStoppingState) {
            emit q->listeningStopped(false);
        }
    }
}

void QSpeechRecognitionPrivate::onResult(int session, const QString &grammarName, const QVariantMap &resultData)
{
    Q_Q(QSpeechRecognition);
    qCDebug(lcSpeechAsr) << "QSpeechRecognitionPrivate::onResult()";
    if (session == m_session
    && m_state == QSpeechRecognition::ProcessingState) {
        setState(QSpeechRecognition::IdleState);
        emit q->result(grammarName, resultData);
    }
}

void QSpeechRecognitionPrivate::onError(int session, QSpeechRecognition::Error errorCode, const QVariantMap &parameters)
{
    Q_Q(QSpeechRecognition);
    qCDebug(lcSpeechAsr) << errorCode;
    if (session == QSpeechRecognitionManager::NO_SESSION || session == m_session) {
        QSpeechRecognitionGrammarImpl *grammar = 0;
        switch (errorCode) {
            // Ignore NoResultError if not waiting for result
            case QSpeechRecognition::NoResultError:
                if (m_state == QSpeechRecognition::ProcessingState) {
                    setState(QSpeechRecognition::IdleState);
                    emit q->error(errorCode, parameters);
                }
                break;
            case QSpeechRecognition::GrammarInitError:
                grammar = m_grammars.value(parameters.value(QSpeechRecognition::Grammar).toString(), 0);
                if (grammar) {
                    grammar->setState(QSpeechRecognitionGrammar::ErrorState);
                    emit q->error(errorCode, parameters);
                }
                break;
            default:
                emit q->error(errorCode, parameters);
                break;
        }
    }
}

void QSpeechRecognitionPrivate::onAttributeUpdated(const QString &key)
{
    Q_Q(QSpeechRecognition);
    QSpeechRecognitionManager::AttributeData data = m_manager->getAttribute(key);
    if (data.value.isValid()) {
        // Ignore session attributes if already moved to idle state
        if ((data.session == m_session && m_state != QSpeechRecognition::IdleState)
        || data.session == QSpeechRecognitionManager::NO_SESSION) {
            emit q->attributeUpdated(key, data.value);
        }
    }
}

void QSpeechRecognitionPrivate::onEngineParameterUpdated(const QString &engineName, const QString &key, const QVariant &value)
{
    QSpeechRecognitionEngineImpl *engine = m_engines.value(engineName, 0);
    if (engine)
        engine->saveParameter(key, value);
}

void QSpeechRecognitionPrivate::onUnmuteTimeout()
{
    Q_Q(QSpeechRecognition);
    if (m_unmuteTime > 0) {
        if (m_muted) {
            emit m_managerInterface->unmute(m_unmuteTime);
            m_unmuteTime = 0;
            m_muted = false;
            emit q->muteChanged();
        } else {
            m_unmuteTime = 0;
        }
    }
}

void QSpeechRecognitionPrivate::setState(QSpeechRecognition::State state)
{
    Q_Q(QSpeechRecognition);
    if (state != m_state) {
        m_state = state;
        qCDebug(lcSpeechAsr) << m_state;
        QMetaObject::invokeMethod(q, "stateChanged", Qt::QueuedConnection);
    }
}

QT_END_NAMESPACE
