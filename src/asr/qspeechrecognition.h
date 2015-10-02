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

#ifndef QSPEECHRECOGNITION_H
#define QSPEECHRECOGNITION_H

#include "qspeechrecognition_global.h"
#include "qspeechrecognitiongrammar.h"
#include "qspeechrecognitionengine.h"
#include <QtCore/QObject>
#include <QtCore/QUrl>
#include <QtCore/QVariantMap>

QT_BEGIN_NAMESPACE

class QSpeechRecognitionPrivate;

class QSPEECHRECOGNITION_EXPORT QSpeechRecognition : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSpeechRecognition)
    Q_PROPERTY(State state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY muteChanged)
    Q_PROPERTY(QSpeechRecognitionGrammar *activeGrammar READ activeGrammar WRITE setActiveGrammar)

public:
    enum Error {
        NoError,
        EngineInitError,
        EngineParameterError,
        EngineError,
        GrammarInitError,
        GrammarError,
        AudioError,
        ConnectionError,
        NoGrammarError,
        NoResultError
    };
    Q_ENUM(Error)

    enum State {
        IdleState,
        ListeningStartingState,
        ListeningState,
        ListeningStoppingState,
        ProcessingState,
        MutedState,
    };
    Q_ENUM(State)

    QSpeechRecognition(QObject *parent = 0);
    ~QSpeechRecognition();
    State state() const;
    Q_INVOKABLE QSpeechRecognitionEngine *createEngine(const QString &name, const QString &providerName, const QVariantMap &parameters = QVariantMap());
    Q_INVOKABLE QSpeechRecognitionEngine *engine(const QString &name) const;
    Q_INVOKABLE QSpeechRecognitionGrammar *createGrammar(QSpeechRecognitionEngine *engine, const QString &name, const QUrl &location);
    Q_INVOKABLE QSpeechRecognitionGrammar *grammar(const QString &name) const;
    Q_INVOKABLE void deleteGrammar(QSpeechRecognitionGrammar *grammar);
    void setActiveGrammar(QSpeechRecognitionGrammar *grammar);
    QSpeechRecognitionGrammar *activeGrammar() const;
    Q_INVOKABLE void startListening();
    Q_INVOKABLE void stopListening();
    Q_INVOKABLE void abortListening();
    void setMuted(bool muted);
    bool isMuted();
    Q_INVOKABLE void unmuteAfter(int millisec);
    Q_INVOKABLE void reset();
    Q_INVOKABLE void dispatchMessage(const QString &message, const QVariantMap &parameters = QVariantMap());

    // Common attribute keys:
    static const QString AudioLevel;

    // Common error parameter keys:
    static const QString Reason;
    static const QString Engine;
    static const QString Grammar;

    // Common result parameter keys:
    static const QString Transcription;

Q_SIGNALS:
    void stateChanged();
    void muteChanged();
    void error(QSpeechRecognition::Error errorCode, const QVariantMap &parameters);
    void result(const QString &grammarName, const QVariantMap &resultData);
    void listeningStarted(const QString &grammarName);
    void listeningStopped(bool expectResult);
    void attributeUpdated(const QString &key, const QVariant &value);
    void message(const QString &message, const QVariantMap &parameters);
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QSpeechRecognition::Error)

#endif
