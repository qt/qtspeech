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

#include "qspeechrecognitiongrammar_p.h"

QT_BEGIN_NAMESPACE

/*!
  \class QSpeechRecognitionGrammar
  \inmodule QtSpeechRecognition
  \brief The QSpeechRecognitionGrammar class is a handle to a speech recognition grammar.

  A speech recognition grammar can be created with \l QSpeechRecognition::createGrammar()
  and released with \l QSpeechRecognition::deleteGrammar().
*/

/*!
  \enum QSpeechRecognitionGrammar::State

  Current state of the speech recognition grammar:

  \value InvalidState           The grammar has not been initialized.
  \value InitializingState      The grammar initialization is ongoing.
  \value ReadyState             The grammar has been successfully initialized.
  \value ErrorState             An critical error has occurred and the grammar is unusable.
  \value DeletedState           The grammar has been deleted (only the handle is still alive).
*/

/*!
  \property QSpeechRecognitionGrammar::engine
  The engine to which the grammar belongs.
*/

/*!
  \property QSpeechRecognitionGrammar::state
  The current state of the grammar.
*/

/*!
  \property QSpeechRecognitionGrammar::name
  The name of the grammar.
*/

QSpeechRecognitionGrammar::QSpeechRecognitionGrammar(QObject *parent):
    QObject(parent)
{
}

QSpeechRecognitionGrammar::~QSpeechRecognitionGrammar()
{
}

QSpeechRecognitionGrammarImpl::QSpeechRecognitionGrammarImpl(const QString &name, QSpeechRecognitionEngine *engine, QObject *parent):
    QSpeechRecognitionGrammar(parent),
    m_name(name),
    m_engine(engine),
    m_state(QSpeechRecognitionGrammar::InvalidState)
{
}

QSpeechRecognitionEngine *QSpeechRecognitionGrammarImpl::engine() const
{
    return m_engine;
}

QSpeechRecognitionGrammar::State QSpeechRecognitionGrammarImpl::state() const
{
    return m_state;
}

QString QSpeechRecognitionGrammarImpl::name() const
{
    return m_name;
}

void QSpeechRecognitionGrammarImpl::setState(QSpeechRecognitionGrammar::State state)
{
    m_state = state;
}

QT_END_NAMESPACE
