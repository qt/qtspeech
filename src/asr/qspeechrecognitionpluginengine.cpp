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

#include "qspeechrecognitionpluginengine.h"
#include "qspeechrecognitionpluginengine_p.h"

#include <QFileInfo>
#include <QDataStream>
#include <QLoggingCategory>

QT_BEGIN_NAMESPACE

/*!
  \class QSpeechRecognitionPluginEngine
  \inmodule QtSpeechRecognition
  \brief The QSpeechRecognitionPluginEngine class is the base for all speech recognition engine integrations.

  An engine implementation should derive from QSpeechRecognitionPluginEngine and implement all
  the pure virtual methods. The base class also includes some methods that can be used for
  performing common operations, like parsing the common initialization parameters.

  All the methods are called from the thread that runs the recognizer task queue, allowing the
  engine to perform time-consuming operations without separate threading. However, most of the
  real-time audio processing should be performed in the low-priority \l process() method,
  allowing better response for the application commands.

  The engine communicates with the application (and the speech recognizer library) by
  emitting signals.
*/

/*! \fn QSpeechRecognition::Error QSpeechRecognitionPluginEngine::updateParameter(const QString &key, const QVariant &value, QString *errorString)

  Attempts to update engine run-time parameter.
  Parameters \a key and \a value are passed directly from QSpeechRecognitionEngine::setParameter().

  Returns QSpeechRecognition::NoError on success, or an error code (usually
  QSpeechRecognition::EngineParameterError) if an error occurred. The method may
  also give a description of the error in \a errorString.

  If QSpeechRecognition::NoError is returned, the map returned by \l parameters() is
  updated with the new value. The value returned by QSPeechRecognitionEngine::parameter()
  for the given key is updated as well.
*/

/*! \fn QSpeechRecognitionPluginGrammar *QSpeechRecognitionPluginEngine::createGrammar(const QString &name, const QUrl &location, QString *errorString)

  Creates a new grammar for the engine.
  Parameters \a name and \a location are passed directly from QSpeechRecognition::createGrammar().

  The engine retains the ownership of the returned grammar. However, the caller is allowed to
  delete the grammar as long as the engine instance is alive.

  If an error occurs, the method should return 0 and (optionally) give a description
  of the error in \a errorString. In this case, the application will receive
  QSpeechRecognition::error() with code QSpeechRecognition::GrammarInitError.
*/

/*! \fn QSpeechRecognition::Error QSpeechRecognitionPluginEngine::setGrammar(QSpeechRecognitionPluginGrammar *grammar, QString *errorString)

  Sets the active \a grammar for the engine.

  The speech recognizer library ensures that this method is never called while listening
  for commands. If necessary, the call to this method is preceded by a call to \l abortListening().

  Returns QSpeechRecognition::NoError on success, or an error code (usually
  QSpeechRecognition::GrammarError) if an error occurred. The method may
  also give a description of the error in \a errorString.

  If QSpeechRecognition::GrammarInitError is returned, the grammar will be moved permanently
  to an error state, making it unusable from the application.
*/

/*! \fn QSpeechRecognition::Error QSpeechRecognitionPluginEngine::startListening(int session, bool mute, QString *errorString)

  Starts a new recognition session. If speech recognition is already ongoing,
  any buffered audio should be discarded before starting a new recognition session.

  Value of the parameter \a session should be used when sending results and session-related errors
  to the application.

  If parameter \a mute is true, the session is started in muted state. In this case,
  the engine should prepare to start recognition immediately when the mute is released
  (method \l unmute()).

  Returns QSpeechRecognition::NoError on success. If an error occurred, the method should
  return and error code and (optionally) give a description of the error in \a errorString.
*/

/*! \fn void QSpeechRecognitionPluginEngine::stopListening(qint64 timestamp)

  Stops listening for commands and starts processing for the final result.
  The engine should emit signal \l result() either directly from this method
  or later. If a result cannot be produced, the engine should emit signal \l error()
  instead, with error code QSpeechRecognition::NoResultError.

  Parameter \a timestamp marks the time returned by QDateTime::currentMSecsSinceEpoch()
  at the moment the application initiated this command. The engine may choose to utilize
  this information to compensate the delay of the command.
*/

/*! \fn void QSpeechRecognitionPluginEngine::abortListening()

  Aborts the recognition session.

  Any results and session-related errors emitted after the abort was initiated will be
  silently discarded. Therefore, the engine may also abort any post-processing
  if \l stopListening() was called before this method.
*/

/*! \fn void QSpeechRecognitionPluginEngine::unmute(qint64 timestamp)

  Releases the mute and starts listening for commands in the selected grammar.

  Parameter \a timestamp marks the time returned by QDateTime::currentMSecsSinceEpoch()
  at the moment the application initiated this command. The engine may choose to utilize
  this information to compensate the delay of the command. If the application used
  QSpeechRecognition::unmuteAfter() to initiate the command, \a timestamp will have a
  value that was calculated at the moment QSpeechRecognition::unmuteAfter() was called.
*/

/*! \fn void QSpeechRecognitionPluginEngine::reset()

  Aborts the recognition session and makes sure that any resources related to audio input
  are released.

  Any results and session-related errors emitted after this method is called will be
  silently discarded. Therefore, the engine may also abort any post-processing
  if \l stopListening() was called before this method.
*/

/*! \fn bool QSpeechRecognitionPluginEngine::process()

  Performs any run-time tasks the engine needs to do, like audio processing.

  The method is only called after the engine has requested so by emitting signal
  \l requestProcess() or by returning true from the previous call to this method.

  The priority of this method is lower than the priority of the other tasks (i.e.
  application commands). Therefore, this method is not called until all the
  other tasks in the queue have been processed. This allows fast response to
  commands even under high CPU load. Also, as \l result() is emitted synchronously
  with the application, the engine can make sure that any commands the application
  may give when \l result() is delivered will be handled before process() is called
  the next time.

  Returns true if another call to process() should be immediately scheduled.
  This can be used to split a larger task into smaller pieces without blocking
  the task queue too long.
*/

/*! \fn void QSpeechRecognitionPluginEngine::requestProcess()

  Schedules the \l process() to be called as soon as there are no other tasks
  in the queue. The method \l process() will be called only once even if this
  signal is emitted multiple times before that.
*/

/*! \fn void QSpeechRecognitionPluginEngine::requestStop(int session, qint64 timestamp)

  If value of \a session (as given in \l startListening()) is still valid and
  the engine is still listening for commands, triggers a call to \l stopListening().
  Parameter \a timestamp is directly passed to \l stopListening().

  This signal can be used to end the recognition session without an error,
  for example when the engine has been configured to react to end-of-speech condition,
  or if recognition is performed from a file and all the data has been processed.

  The signal is handled asynchronously.
*/

/*! \fn void QSpeechRecognitionPluginEngine::result(int session, const QSpeechRecognitionPluginGrammar *grammar, const QVariantMap &resultData)

  Sends a recognition result to the application.

  If value of \a session (as given in \l startListening()) is still valid, the application
  will receive \a resultData via signal QSpeechRecognition::result(). Parameter \a grammar
  should point to the grammar instance that produced the result.

  The signal is delivered synchronously from the recognizer task thread to the application.
  Therefore, if the signal is emitted from the engine context, the engine won't receive any
  further commands until QSpeechRecognition::result() has been delivered. This can be used
  for synchronizing the result handling with audio stream processing.
*/

/*! \fn void QSpeechRecognitionPluginEngine::error(int session, QSpeechRecognition::Error error, const QVariantMap &parameters)

  Sends an \a error to the application, with optional \a parameters.

  The signal is delivered asynchronously.

  If the value of \a session (as given in \l startListening()) is still valid, or if \a session
  is set to QSpeechRecognitionPluginEngine::NO_SESSION, the current recognition session
  (if any) is aborted and the error is forwarded to the application. The application will
  receive the error via signal QSpeechRecognition::error().
*/

/*! \fn void QSpeechRecognitionPluginEngine::attributeUpdated(int session, const QString &key, const QVariant &value)

  Updates a run-time attribute (name specified by \a key) with given \a value.

  If value of \a session (as given in \l startListening()) is still valid, or if \a session
  is set to QSpeechRecognitionPluginEngine::NO_SESSION, the application will receive the
  key-value pair via signal QSpeechRecognition::attributeUpdated(). As an additional
  restriction, session-related attributes are not delivered if QSpeechRecognition has
  already moved to idle state (not listening or processing for results).

  The application will only receive QSpeechRecognition::attributeUpdated() if the \a value is
  different from the previous stored value. If the same attribute is updated multiple times
  before the application gets the notification, QSpeechRecognition::attributeUpdated()
  gets emitted only once, with the latest value.
*/

/*!
  Constructs the speech recognition engine base class.

  Parameters \a name and \a parent should be passed as they were given to the factory
  QSpeechRecognitionPlugin::createSpeechRecognitionEngine(). The map \a parameters should
  minimally contain the same keys that were given to the factory, but may additionally contain the
  initial values of any engine-specific parameters. The name and the parameters can be later
  retrieved by calling \l name() and \l parameters().

  The names of supported engine parameters (init-time or run-time) should be listed in
  \a supportedParameters. Only these keys are included in the map returned by \l parameters()
  and can be updated by the application (the engine receives a call to \l updateParameter()).
  Any parameters that are listed in \a supportedParameters but are not included in map
  \a parameters are initialized to the default (or empty) value.
*/
QSpeechRecognitionPluginEngine::QSpeechRecognitionPluginEngine(const QString &name,
                                                               const QVariantMap &parameters,
                                                               const QList<QString> &supportedParameters,
                                                               QObject *parent) :
    QObject(*new QSpeechRecognitionPluginEnginePrivate(), parent)
{
    QVariantMap initialParameters = parameters;
    QVariantMap knownParameters;
    Q_D(QSpeechRecognitionPluginEngine);
    // Initialize built-in parameters to their default values if the value is not set:
    if (!initialParameters.contains(QSpeechRecognitionEngine::Locale))
        initialParameters.insert(QSpeechRecognitionEngine::Locale, locale());
    if (!initialParameters.contains(QSpeechRecognitionEngine::AudioSampleRate))
        initialParameters.insert(QSpeechRecognitionEngine::AudioSampleRate, audioSampleRate());
    if (!initialParameters.contains(QSpeechRecognitionEngine::ResourceDirectory))
        initialParameters.insert(QSpeechRecognitionEngine::ResourceDirectory, resourceDirectory().path());
    if (!initialParameters.contains(QSpeechRecognitionEngine::DataDirectory))
        initialParameters.insert(QSpeechRecognitionEngine::DataDirectory, dataDirectory().path());
    if (!initialParameters.contains(QSpeechRecognitionEngine::Dictionary))
        initialParameters.insert(QSpeechRecognitionEngine::Dictionary, dictionaryLocation());
    // Filter out unknown parameters:
    for (QVariantMap::const_iterator param = initialParameters.begin(); param != initialParameters.end(); ++param) {
        if (supportedParameters.contains(param.key()))
            knownParameters.insert(param.key(), param.value());
    }
    // Add empty values for known parameters that are undefined:
    foreach (const QString &param, supportedParameters) {
        if (!knownParameters.contains(param))
            knownParameters.insert(param, QVariant());
    }
    d->m_name = name;
    d->m_parameters = knownParameters;
}

/*!
  Called by the speech recognizer library when the application attempts to set an engine
  parameter by calling QSpeechRecognitionEngine::setParameter().

  Initiates a call to \l updateParameter() and updates the parameters returned by \l parameters()
  if the call was successful. Method parameters \a key, \a value and \a errorString are passed
  directly to \l updateParameter().

  Returns QSpeechRecognition::NoError on success.
*/
QSpeechRecognition::Error QSpeechRecognitionPluginEngine::setParameter(const QString &key, const QVariant &value, QString *errorString)
{
    Q_D(QSpeechRecognitionPluginEngine);
    QSpeechRecognition::Error errorCode = updateParameter(key, value, errorString);
    if (errorCode == QSpeechRecognition::NoError)
        d->m_parameters.insert(key, value);
    return errorCode;
}

/*!
  Gets the engine name.
*/
const QString &QSpeechRecognitionPluginEngine::name() const
{
    Q_D(const QSpeechRecognitionPluginEngine);
    return d->m_name;
}

/*!
  Retrieves the engine parameters.
*/
const QVariantMap &QSpeechRecognitionPluginEngine::parameters() const
{
    Q_D(const QSpeechRecognitionPluginEngine);
    return d->m_parameters;
}

/*!
  Extracts the locale from engine parameters.
  If not set, returns the default value.

  Key QSpeechRecognitionEngine::Locale should be listed in the supported engine parameters.
*/
QLocale QSpeechRecognitionPluginEngine::locale() const
{
    Q_D(const QSpeechRecognitionPluginEngine);
    if (d->m_parameters.contains(QSpeechRecognitionEngine::Locale)) {
        const QVariant &locale = d->m_parameters[QSpeechRecognitionEngine::Locale];
        if (locale.userType() == QMetaType::QLocale)
            return locale.toLocale();
    }
    return QLocale();
}

/*!
  Extracts the engine resource directory from engine parameters.
  If not set, returns the default value.

  Key QSpeechRecognitionEngine::ResourceDirectory should be listed in the supported
  engine parameters.
*/
QDir QSpeechRecognitionPluginEngine::resourceDirectory() const
{
    Q_D(const QSpeechRecognitionPluginEngine);
    if (d->m_parameters.contains(QSpeechRecognitionEngine::ResourceDirectory)) {
        const QVariant &resourceDirectory = d->m_parameters[QSpeechRecognitionEngine::ResourceDirectory];
        if (resourceDirectory.userType() == QMetaType::QString)
            return QDir(resourceDirectory.toString());
    }
    return QDir();
}

/*!
  Extracts the writable data directory from engine parameters.
  If not set, returns the default value.

  Key QSpeechRecognitionEngine::DataDirectory should be listed in the supported engine parameters.
*/
QDir QSpeechRecognitionPluginEngine::dataDirectory() const
{
    Q_D(const QSpeechRecognitionPluginEngine);
    if (d->m_parameters.contains(QSpeechRecognitionEngine::DataDirectory)) {
        const QVariant &dataDirectory = d->m_parameters[QSpeechRecognitionEngine::DataDirectory];
        if (dataDirectory.userType() == QMetaType::QString) {
            return QDir(dataDirectory.toString());
        }
    }
    return QDir();
}

/*!
  Extracts the dictionary URL from the engine parameters.
  If not set, returns the default value.

  Key QSpeechRecognitionEngine::Dictionary should be listed in the supported engine parameters.
*/
QUrl QSpeechRecognitionPluginEngine::dictionaryLocation() const
{
    Q_D(const QSpeechRecognitionPluginEngine);
    if (d->m_parameters.contains(QSpeechRecognitionEngine::Dictionary)) {
        const QVariant &dictionaryLocation = d->m_parameters[QSpeechRecognitionEngine::Dictionary];
        if (dictionaryLocation.userType() == QMetaType::QUrl) {
            return dictionaryLocation.toUrl();
        }
    }
    return QUrl();
}

/*!
  Extracts the audio sample rate from the engine parameters.
  If not set, returns the default value.

  Key QSpeechRecognitionEngine::AudioSampleRate should be listed in the supported
  engine parameters.
*/
int QSpeechRecognitionPluginEngine::audioSampleRate() const
{
    Q_D(const QSpeechRecognitionPluginEngine);
    if (d->m_parameters.contains(QSpeechRecognitionEngine::AudioSampleRate)) {
        const QVariant &audioSampleRate = d->m_parameters[QSpeechRecognitionEngine::AudioSampleRate];
        if (audioSampleRate.userType() == QMetaType::Int)
            return audioSampleRate.toInt();
    }
    return 16000;
}

/*!
  Extracts the audio input file path from the engine parameters.
  If not set, returns an empty string.

  Key QSpeechRecognitionEngine::AudioInputFile should be listed in the supported engine parameters.
*/
QString QSpeechRecognitionPluginEngine::audioInputFile() const
{
    Q_D(const QSpeechRecognitionPluginEngine);
    if (d->m_parameters.contains(QSpeechRecognitionEngine::AudioInputFile)) {
        const QVariant &audioInputFile = d->m_parameters[QSpeechRecognitionEngine::AudioInputFile];
        if (audioInputFile.userType() == QMetaType::QString) {
            return audioInputFile.toString();
        }
    }
    return QString();
}

/*!
  Gets a localized file path and ensures that the file exists.

  If \a filePath is an absolute path and the file exists, returns back
  the unaltered value with any wildcards resolved.

  If a relative file path is given, attempts to find the file in
  a locale-specific sub-directory under the engine resource directory.

  Wildcards are supported in the file name, but not in any directory names
  in the middle of the path. If a wildcard matches to several files,
  the first of them is selected.

  BCP 47 locale name is used for the locale-specific sub-directory.
  If the file cannot be found, the last part of the locale name is
  removed and existence of the file is checked again.

  Returns an absolute path to the file. If the file does not exist,
  returns an empty string.
*/
QString QSpeechRecognitionPluginEngine::localizedFilePath(const QString &filePath) const
{
    if (QDir::isAbsolutePath(filePath))
        return QSpeechRecognitionPluginEnginePrivate::findFilesWithWildcards(filePath).value(0);
    QString localeStr = locale().bcp47Name();
    int dashPos;
    QString relativeResourcePath = localeStr + QLatin1String("/") + filePath;
    QDir resourceDir = resourceDirectory();
    QString absoluteResourcePath = resourceDir.absoluteFilePath(relativeResourcePath);
    QStringList foundFiles = QSpeechRecognitionPluginEnginePrivate::findFilesWithWildcards(absoluteResourcePath);
    while (foundFiles.isEmpty()
    && (dashPos = localeStr.lastIndexOf(QLatin1String("-"))) > 0) {
        localeStr = localeStr.left(dashPos);
        relativeResourcePath = localeStr + QLatin1String("/") + filePath;
        absoluteResourcePath = resourceDir.absoluteFilePath(relativeResourcePath);
        foundFiles = QSpeechRecognitionPluginEnginePrivate::findFilesWithWildcards(absoluteResourcePath);
    }
    return foundFiles.value(0);
}

/*!
  Creates a WAV-file for writing debug audio.

  If \a filePath is an absolute path, always attempts to create the file. If a relative
  file path is given, only creates the file if engine parameter
  QSpeechRecognitionEngine::DebugAudioDirectory has been set (see
  QSpeechRecognition::createEngine()).

  Parameters \a sampleRate, \a sampleSize and \a channelCount specify the type of audio data
  that will be written to the file. Sample size should be expressed in bits.

  If the file was successfully created, returns a pointer to an opened file object where
  raw PCM-formatted data can be written. Otherwise returns 0. The caller should delete
  the object after all data has been written.

  The audio data should be written in little-endian byte order.
*/
QFile *QSpeechRecognitionPluginEngine::openDebugWavFile(const QString &filePath, int sampleRate, int sampleSize, int channelCount)
{
    Q_D(const QSpeechRecognitionPluginEngine);
    QString finalPath;
    if (QDir::isAbsolutePath(filePath)) {
        finalPath = filePath;
    } else if (d->m_parameters.contains(QSpeechRecognitionEngine::DebugAudioDirectory)) {
        QString audioDirPath = d->m_parameters.value(QSpeechRecognitionEngine::DebugAudioDirectory).toString();
        if (!audioDirPath.isEmpty()) {
            QDir audioDir(audioDirPath);
            if (audioDir.exists())
                finalPath = audioDir.absoluteFilePath(filePath);
        }
    }
    if (!finalPath.isEmpty()) {
        qCDebug(lcSpeechAsr) << "QSpeechRecognitionPluginEngine: Writing debug audio to file" << finalPath;
        QSpeechRecognitionDebugAudioFile *file = new QSpeechRecognitionDebugAudioFile(finalPath, sampleRate, sampleSize, channelCount);
        if (file->open())
            return file;
        delete file;
    }
    return 0;
}

QSpeechRecognitionPluginEngine::~QSpeechRecognitionPluginEngine()
{
}

QStringList QSpeechRecognitionPluginEnginePrivate::findFilesWithWildcards(const QString &filePath)
{
    QFileInfo info(filePath);
    QDir dir = info.dir();
    QString fileName = info.fileName();
    QStringList result;
    foreach (QString file, dir.entryList(QStringList() << fileName))
        result << dir.absoluteFilePath(file);
    return result;
}

QSpeechRecognitionDebugAudioFile::QSpeechRecognitionDebugAudioFile(const QString &filePath, int sampleRate, int sampleSize, int channelCount):
    QFile(filePath),
    m_sampleRate(sampleRate),
    m_sampleSize(sampleSize),
    m_channelCount(channelCount),
    m_totalSizeOffset(0),
    m_dataSizeOffset(0),
    m_dataOffset(0)
{
    connect(this, &QFile::aboutToClose, this, &QSpeechRecognitionDebugAudioFile::onAboutToClose);
}

QSpeechRecognitionDebugAudioFile::~QSpeechRecognitionDebugAudioFile()
{
    disconnect(this); // Prevent a signal to onAboutToClose() from QFile destructor
    if (isOpen())
        onAboutToClose();
}

bool QSpeechRecognitionDebugAudioFile::open(OpenMode flags)
{
    if (flags != QIODevice::WriteOnly)
        return false;
    if (!QFile::open(flags))
        return false;
    if (!writeWavHeader()) {
        close();
        return false;
    }
    return true;
}

void QSpeechRecognitionDebugAudioFile::onAboutToClose()
{
    // Fill in the size fields in WAV header, replacing the placeholder values:
    qint64 fileSize = size();
    QDataStream dataStream(this);
    dataStream.setByteOrder(QDataStream::LittleEndian);
    seek(m_totalSizeOffset);
    dataStream << (quint32)(fileSize - 8);
    seek(m_dataSizeOffset);
    dataStream << (quint32)(fileSize - m_dataOffset);
}

bool QSpeechRecognitionDebugAudioFile::writeWavHeader()
{
    struct {
        quint16 formatTag;
        quint16 channels;
        quint32 samplesPerSec;
        quint32 bytesPerSec;
        quint16 blockAlign;
        quint16 bitsPerSample;
    } wavFormatData;

    const char ZEROS[4] = { 0 };
    const quint32 FORMAT_SIZE = sizeof(wavFormatData);

    QDataStream dataStream(this);
    dataStream.setByteOrder(QDataStream::LittleEndian);

    wavFormatData.formatTag = 1;    // PCM
    wavFormatData.channels = m_channelCount;
    wavFormatData.samplesPerSec = m_sampleRate;
    wavFormatData.bytesPerSec = (m_sampleRate
                               * m_sampleSize
                               * m_channelCount) / 8;
    wavFormatData.blockAlign = (m_sampleSize * m_channelCount) / 8;
    wavFormatData.bitsPerSample = m_sampleSize;

    dataStream.writeRawData("RIFF", 4);
    m_totalSizeOffset = pos();
    dataStream.writeRawData(ZEROS, 4); // Placeholder: RIFF size
    dataStream.writeRawData("WAVEfmt ", 8);
    dataStream << FORMAT_SIZE;
    dataStream << wavFormatData.formatTag;
    dataStream << wavFormatData.channels;
    dataStream << wavFormatData.samplesPerSec;
    dataStream << wavFormatData.bytesPerSec;
    dataStream << wavFormatData.blockAlign;
    dataStream << wavFormatData.bitsPerSample;
    dataStream.writeRawData("data", 4);
    m_dataSizeOffset = pos();
    dataStream.writeRawData(ZEROS, 4); // Placeholder: data size
    m_dataOffset = pos();
    return true;
}

QT_END_NAMESPACE
