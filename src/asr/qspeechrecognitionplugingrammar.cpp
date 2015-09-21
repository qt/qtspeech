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

#include "qspeechrecognitionplugingrammar.h"
#include "qspeechrecognitionplugingrammar_p.h"

QT_BEGIN_NAMESPACE

/*!
  \class QSpeechRecognitionPluginGrammar
  \inmodule QtSpeechRecognition
  \brief The QSpeechRecognitionPluginGrammar class is the base for all speech recognition grammar integrations.

  A grammar implementation should derive from QSpeechRecognitionPluginGrammar. The grammar instance is
  created by QSpeechRecognitionPluginEngine::createGrammar().
*/

/*!
  Constructs the speech recognition grammar base class.

  The grammar \a name should be passed as it was given to the factory
  QSpeechRecognitionPluginEngine::createGrammar().
*/
QSpeechRecognitionPluginGrammar::QSpeechRecognitionPluginGrammar(const QString &name, QObject *parent) :
    QObject(*new QSpeechRecognitionPluginGrammarPrivate(), parent)
{
    Q_D(QSpeechRecognitionPluginGrammar);
    d->m_name = name;
}

/*!
  Gets the grammar name.
*/
const QString &QSpeechRecognitionPluginGrammar::name() const
{
    Q_D(const QSpeechRecognitionPluginGrammar);
    return d->m_name;
}

QT_END_NAMESPACE
