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

#include "qspeechrecognitionplugin.h"

QT_BEGIN_NAMESPACE

/*!
  \class QSpeechRecognitionPlugin
  \inmodule QtSpeechRecognition
  \brief The QSpeechRecognitionPlugin class is the base for all speech recognition plug-ins.

  A plug-in implementation should derive from QSpeechRecognitionPlugin and re-implement
  \l createSpeechRecognitionEngine().

  All plug-ins belonging to the same QSpeechRecognition instance are used from
  the single thread that runs the speech recognizer task queue. However, the thread
  where the QSpeechRecognitionPlugin instance is created may be different.
*/

/*!
  Factory method that is triggered by a call to \l QSpeechRecognition::createEngine().
  The \a name and \a parameters are given by the application and should be used when
  constructing the QSpeechRecognitionPluginEngine instance.

  If an error occurs, the method should return 0 and (optionally) give a description
  of the error in \a errorString. In this case, the application will receive
  QSpeechRecognition::error() with code QSpeechRecognition::EngineInitError.

  If \a parent is 0, the caller takes the ownership of the returned engine instance.
*/
QSpeechRecognitionPluginEngine *QSpeechRecognitionPlugin::createSpeechRecognitionEngine(const QString &name,
        const QVariantMap &parameters,
        QObject *parent,
        QString *errorString) const
{
    Q_UNUSED(name)
    Q_UNUSED(parameters)
    Q_UNUSED(parent)
    Q_UNUSED(errorString)

    return 0;
}

QT_END_NAMESPACE
