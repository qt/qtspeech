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

#include "qspeechrecognitionengine_p.h"

#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

/*!
  \class QSpeechRecognitionEngine
  \inmodule QtSpeech
  \brief The QSpeechRecognitionEngine class is a handle to a speech recognition engine.

  A speech recognition engine can be created with \l QSpeechRecognition::createEngine().
*/

/*!
  \property QSpeechRecognitionEngine::name
  The name of the engine.
*/

/*!
  \property QSpeechRecognitionEngine::created
  Whether the speech recognition engine has been successfully initialized.

  The property is \c false if the engine is still initializing (or failed to initialize)
  and will be set \c true after the engine has been constructed and initialized.
*/

/*!
  \fn bool QSpeechRecognitionEngine::setParameter(const QString &key, const QVariant &value)

  Set engine run-time parameter (name given in \a key) to the given \a value.

  The method should only be called after the engine has been \l created. The method immediately
  fails if the parameter is unknown or the value type is inconsistent with the current value.

  The value is set asynchronously. Therefore, the value returned by \l parameter() will reflect
  the updated value only after the engine has accepted it. In case the engine does not accept
  the value, signal QSpeechRecognition::error() will be emitted.

  Returns \c true if the parameter request was successfully sent to the engine.

  \sa supportedParameters()
*/

/*!
  \fn QList<QString> QSpeechRecognitionEngine::supportedParameters() const

  Get names of the supported engine parameters.

  The method should only be called after the engine has been \l created; an empty list will
  be returned before that.

  Some parameters may only be supported during engine initialization (when calling
  QSpeechRecognition::createEngine()), or may contain read-only information. The following table
  lists some of the commonly supported engine parameters:

  \table
  \header
    \li Key
    \li Value type
    \li Description
  \row
    \li locale
    \li QLocale
    \li The locale for speech recognition
  \row
    \li dictionary
    \li QUrl
    \li Location of the speech recognition dictionary (default lexicon).
        If the URL contains a relative file path, the dictionary is loaded
        from the locale-specific sub-directory under the engine resource directory.
        Some engines may also support reading the dictionary from built-in Qt resources.
  \row
    \li resourceDirectory
    \li QString
    \li Path to the directory where engine-specific resource files are located.
        If not given, the program's working directory is used.
  \row
    \li dataDirectory
    \li QString
    \li Path to a persistent directory where any engine-specific data
        can be stored between application restarts. If not given, the program's
        working directory is used.
  \row
    \li debugAudioDirectory
    \li QString
    \li Path to a directory where the engine should write all the audio clips that go
        to the recognizer. If not given (or empty), no audio clips will be produced.
        This feature is meant to be used only for debugging purposes.
  \row
    \li audioSampleRate
    \li int
    \li Samples per second in the input audio. Default: 16000.
  \row
    \li audioInputFile
    \li QString
    \li Path to an audio file that should be read instead of an audio input device.
        The given file will be read once for each recognition session.
        If the engine is muted when the session starts, no data is read from the file
        until mute is released. The recognition session will be automatically stopped
        when the entire file has been read.
  \row
    \li audioInputDevices
    \li QStringList
    \li Names of the supported audio input devices (read-only).
  \row
    \li audioInputDevice
    \li QString
    \li Name of the currently selected audio input device.
        If not set, the system default will be used.
  \endtable

  Returns the names of the supported engine parameters.
*/

/*!
  \fn QVariant QSpeechRecognitionEngine::parameter(const QString &key)

  Get engine parameter with the given \a key.

  The method should only be called after the engine has been \l created; an empty value will
  be returned before that.

  Returns the latest known value of the parameter with the given \a key. If the value has not
  been set, or if the parameter is unknown, an empty variant is returned.

  \sa supportedParameters()
*/

QSpeechRecognitionEngine::QSpeechRecognitionEngine(QObject *parent):
    QObject(parent)
{
}

QSpeechRecognitionEngine::~QSpeechRecognitionEngine()
{
}

QSpeechRecognitionEngineImpl::QSpeechRecognitionEngineImpl(const QString &name, QObject *parent):
    QSpeechRecognitionEngine(parent),
    m_name(name),
    m_created(false)
{
}

QString QSpeechRecognitionEngineImpl::name() const
{
    return m_name;
}

bool QSpeechRecognitionEngineImpl::setParameter(const QString &key, const QVariant &value)
{
    if (m_parameters.contains(key)
    && (!m_parameters.value(key).isValid() || m_parameters.value(key).type() == value.type())) {
        emit requestSetParameter(key, value);
        return true;
    }
    return false;
}

QList<QString> QSpeechRecognitionEngineImpl::supportedParameters() const
{
    return m_parameters.keys();
}

QVariant QSpeechRecognitionEngineImpl::parameter(const QString &key) const
{
    return m_parameters.value(key);
}

bool QSpeechRecognitionEngineImpl::isCreated()
{
    return m_created;
}

void QSpeechRecognitionEngineImpl::setCreated(bool created)
{
    if (!m_created)
        QMetaObject::invokeMethod(this, "created", Qt::QueuedConnection);
    m_created = created;
}

void QSpeechRecognitionEngineImpl::saveParameter(const QString &key, const QVariant &value)
{
    qCDebug(lcSpeechAsr) << QLatin1String("QSpeechRecognitionEngine (") + m_name + QLatin1String("): Parameter") << key << "<--" << value;
    m_parameters.insert(key, value);
}

QT_END_NAMESPACE
