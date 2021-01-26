/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Speech module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:COMM$
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** $QT_END_LICENSE$
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
**
****************************************************************************/

#include "qtexttospeechplugin.h"

QT_BEGIN_NAMESPACE

/*!
  \class QTextToSpeechPlugin
  \inmodule QtSpeech
  \brief The QTextToSpeechPlugin class is the base for all text-to-speech plug-ins.

  A plug-in implementation should derive from QTextToSpeechPlugin and re-implement
  \l createTextToSpeechEngine().
*/

/*!
  Factory method that is triggered by a call to \l QTextToSpeech::QTextToSpeech()
  when a provider name is given in the constructor and a text-to-speech plug-in
  matching the provider name was successfully loaded.

  Value of \a parameters is currently always empty.

  If an error occurs, the method should return 0 and (optionally) give a description
  of the error in \a errorString. In this case, QTextToSpeech::state() will return
  QTextToSpeech::BackendError.

  If \a parent is 0, the caller takes the ownership of the returned engine instance.
*/
QTextToSpeechEngine *QTextToSpeechPlugin::createTextToSpeechEngine(
        const QVariantMap &parameters,
        QObject *parent,
        QString *errorString) const
{
    Q_UNUSED(parameters)
    Q_UNUSED(parent)
    Q_UNUSED(errorString)

    return 0;
}

QT_END_NAMESPACE
