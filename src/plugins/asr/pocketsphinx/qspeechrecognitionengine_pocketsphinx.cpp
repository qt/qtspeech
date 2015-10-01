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

#include "qspeechrecognitionengine_pocketsphinx.h"
#include "qspeechrecognitiongrammar_pocketsphinx.h"
#include "qspeechrecognitionaudiobuffer.h"

#include <QLocale>
#include <QDebug>
#include <QUrl>
#include <QDateTime>
#include <QDataStream>
#include <QVector>
#include <QFile>

#include <pocketsphinx.h>
#include <cstdio>
#include <sphinxbase/err.h>

QT_BEGIN_NAMESPACE

void PocketShpinxErrorCb(void *user_data, err_lvl_t lvl, const char *fmt, ...)
{
    char buf[500];
    va_list ap;
    Q_UNUSED(user_data);
    Q_UNUSED(lvl);

    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    qCDebug(lcSpeechAsrPocketSphinx) << "PocketSphinx:" << buf;
    va_end(ap);
}

QSpeechRecognitionEnginePocketSphinx::QSpeechRecognitionEnginePocketSphinx(const QString &name,
    const QVariantMap &parameters, QObject *parent)
        : QSpeechRecognitionPluginEngine(name, createEngineParameters(parameters),
            QStringList() << QSpeechRecognitionEngine::Locale
                          << QSpeechRecognitionEngine::Dictionary
                          << QSpeechRecognitionEngine::ResourceDirectory
                          << QSpeechRecognitionEngine::DataDirectory
                          << QSpeechRecognitionEngine::DebugAudioDirectory
                          << QSpeechRecognitionEngine::AudioSampleRate
                          << QSpeechRecognitionEngine::AudioInputFile
                          << QSpeechRecognitionEngine::AudioInputDevices
                          << QSpeechRecognitionEngine::AudioInputDevice,
            parent),
        m_session(0),
        m_decoder(0),
        m_device(QAudioDeviceInfo::defaultInputDevice()),
        m_audioInput(0),
        m_grammar(0),
        m_audioBuffer(0),
        m_audioBufferLimit(-1),
        m_debugAudioFile(0),
        m_sessionStarted(false),
        m_cmnVec(0),
        m_cmnSize(0)
{
    const QVariantMap &engineParams = QSpeechRecognitionPluginEngine::parameters();
    QString inputDeviceName = engineParams[QSpeechRecognitionEngine::AudioInputDevice].toString();
    QList<QAudioDeviceInfo> audioDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo device, audioDevices) {
        if (!inputDeviceName.isEmpty() && device.deviceName() == inputDeviceName) {
            m_device = device;
            break;
        }
    }
}

QSpeechRecognitionEnginePocketSphinx::~QSpeechRecognitionEnginePocketSphinx()
{
    // Make sure all the grammars are deleted before uninitializing the engine
    qDeleteAll(findChildren<QSpeechRecognitionGrammarPocketSphinx*>());
    disconnect(&m_inputFileDecoder);
    ps_free(m_decoder);
    delete[] m_cmnVec;
}

bool QSpeechRecognitionEnginePocketSphinx::init(QString *errorString)
{
    static const int AUDIO_BUFFER_LIMIT_SEC = 8; // TODO: Add an init parameter for this

    m_format.setSampleRate(audioSampleRate());
    m_format.setChannelCount(1);
    m_format.setSampleSize(16);
    m_format.setSampleType(QAudioFormat::SignedInt);
    m_format.setByteOrder(QAudioFormat::LittleEndian);
    m_format.setCodec("audio/pcm");
    m_audioBuffer = new QSpeechRecognitionAudioBuffer(this);
    m_audioBuffer->setFreeLimit(5000);
    m_audioBuffer->setSampleRate(m_format.sampleRate());
    m_audioBuffer->setChannelCount(m_format.channelCount());
    m_audioBuffer->setSampleSize(m_format.sampleSize());
    m_audioBufferLimit = m_format.bytesPerFrame() * m_format.sampleRate() * AUDIO_BUFFER_LIMIT_SEC;
    connect(m_audioBuffer, &QSpeechRecognitionAudioBuffer::dataAvailable, this, &QSpeechRecognitionEnginePocketSphinx::onAudioDataAvailable);
    QString acmodelName = "acmodel";
    acmodelName += "_" + QString::number(m_format.sampleRate());
    QString acmodelPath = localizedFilePath(acmodelName);
    // Fallback: no sample rate specification
    if (acmodelPath.isEmpty())
        acmodelPath = localizedFilePath("acmodel");
    if (!acmodelPath.isEmpty()) {
        cmd_ln_t *config = cmd_ln_init(NULL, ps_args(), TRUE,
                                       "-hmm", acmodelPath.toUtf8().constData(),
                                       "-bestpath", "no",
                                       NULL);
        m_decoder = ps_init(config);
        if (!m_decoder) {
            *errorString = QString("PocketSphinx initialization failed.");
            return false;
        }
        err_set_callback(PocketShpinxErrorCb, this);
    } else {
        *errorString = QString("Acoustic model not found.");
        return false;
    }
    QString dictionaryPath;
    QUrl dictionaryLoc = dictionaryLocation();
    if (!dictionaryLoc.isEmpty()) {
        if (dictionaryLoc.isLocalFile()) {
            dictionaryPath = localizedFilePath(dictionaryLoc.toLocalFile());
        } else {
            *errorString = QString("Dictionary must be a local file.");
            return false;
        }
    } else {
        dictionaryPath = localizedFilePath("lexicon.dict");
    }
    if (dictionaryPath.isEmpty()
    || ps_load_dict(m_decoder, dictionaryPath.toUtf8().constData(), NULL, NULL) != 0) {
        *errorString = QString("Dictionary could not be loaded.");
        return false;
    }
    feat_t *feat = ps_get_feat(m_decoder);
    m_cmnSize = feat_cepsize(feat);
    m_cmnVec = new mfcc_t[m_cmnSize];
    m_cmnFilePath = dataDirectory().absoluteFilePath(QLatin1String("pocketsphinx_")
                                                     + name() + QLatin1String("_cmn"));
    // Attempt to load adapted cepstrum means from the data file. The default values are not
    // optimal for any specific audio path, often causing bad results from the first few utterances.
    loadCmn();
    m_inputFilePath = audioInputFile();
    if (m_inputFilePath.isEmpty())
        createAudioInput();
    m_inputFileDecoder.setAudioFormat(m_format);
    connect(&m_inputFileDecoder, &QAudioDecoder::bufferReady, this, &QSpeechRecognitionEnginePocketSphinx::onAudioDecoderBufferReady);
    connect(&m_inputFileDecoder, &QAudioDecoder::finished, this, &QSpeechRecognitionEnginePocketSphinx::onAudioDecoderFinished);
    connect(&m_inputFileDecoder, static_cast<void (QAudioDecoder::*)(QAudioDecoder::Error)>(&QAudioDecoder::error),
            this,  &QSpeechRecognitionEnginePocketSphinx::onAudioDecoderError);
    *errorString = QString();
    return true;
}

void QSpeechRecognitionEnginePocketSphinx::createAudioInput()
{
    delete m_audioInput;
    m_audioInput = new QAudioInput(m_device, m_format, this);
    connect(m_audioInput, &QAudioInput::stateChanged, this, &QSpeechRecognitionEnginePocketSphinx::onAudioStateChanged);
}

QSpeechRecognition::Error QSpeechRecognitionEnginePocketSphinx::updateParameter(const QString &key, const QVariant &value, QString *errorString)
{
    if (key == QSpeechRecognitionEngine::AudioInputDevice) {
        if (m_sessionStarted) {
            *errorString = "Cannot set audio input device while the engine is busy";
            return QSpeechRecognition::EngineParameterError;
        }
        if (value.type() != QVariant::String) {
            *errorString = QString("Parameter \"") + key + "\" has invalid type";
            return QSpeechRecognition::EngineParameterError;
        }
        QList<QAudioDeviceInfo> audioDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
        foreach (QAudioDeviceInfo device, audioDevices) {
            if (device.deviceName() == value.toString()) {
                m_device = device;
                if (m_inputFilePath.isEmpty())
                    createAudioInput();
                return QSpeechRecognition::NoError;
            }
        }
        *errorString = QString("Audio input device with name \"") + value.toString() + "\" does not exist";
    } else if (key == QSpeechRecognitionEngine::AudioInputFile) {
        if (value.type() != QVariant::String) {
            *errorString = QString("Parameter \"") + key + "\" has invalid type";
            return QSpeechRecognition::EngineParameterError;
        }
        m_inputFilePath = value.toString();
        if (m_inputFilePath.isEmpty() && !m_audioInput)
            createAudioInput();
        return QSpeechRecognition::NoError;
    } else {
        *errorString = QString("Parameter \"") + key + "\" cannot be updated";
    }
    return QSpeechRecognition::EngineParameterError;
}

QSpeechRecognitionPluginGrammar *QSpeechRecognitionEnginePocketSphinx::createGrammar(const QString &name, const QUrl &location, QString *errorString)
{
    QSpeechRecognitionGrammarPocketSphinx *grammar = new QSpeechRecognitionGrammarPocketSphinx(name, location, this);
    if (grammar && !grammar->exists()) {
        *errorString = QString("Grammar file not found");
        delete grammar;
        grammar = 0;
    }
    return grammar;
}

QSpeechRecognition::Error QSpeechRecognitionEnginePocketSphinx::setGrammar(QSpeechRecognitionPluginGrammar *grammar, QString *errorString)
{
    QSpeechRecognitionGrammarPocketSphinx *grammarPriv = qobject_cast<QSpeechRecognitionGrammarPocketSphinx*>(grammar);
    if (grammarPriv) {
        if (!grammarPriv->load()) {
            *errorString = "Loading the grammar failed";
            return QSpeechRecognition::GrammarInitError;
        } else if (ps_set_search(m_decoder, grammarPriv->name().toUtf8().constData()) != 0) {
            *errorString = "Setting the grammar failed";
            return QSpeechRecognition::GrammarInitError;
        } else {
            m_grammar = grammarPriv;
        }
    } else {
        m_grammar = 0;
    }
    return QSpeechRecognition::NoError;
}

QSpeechRecognition::Error QSpeechRecognitionEnginePocketSphinx::startListening(int session, bool mute, QString *errorString)
{
    Q_UNUSED(errorString);
    if (!m_audioBuffer)
        return QSpeechRecognition::EngineInitError;
    if (!m_device.isFormatSupported(m_format)) {
        *errorString = QString("Required audio format not supported by the selected audio device");
        return QSpeechRecognition::AudioError;
    }
    m_session = session;
    m_audioBuffer->clear();
    m_inputFileDecoder.stop();
    m_inputFileDecoder.setSourceFilename(m_inputFilePath);
    if (!m_inputFilePath.isEmpty()) {
        // Recognition from a file: ensure that not recording
        if (m_audioInput)
            m_audioInput->stop();
        m_audioBuffer->close();
        m_inputFileDecoder.start();
        qCDebug(lcSpeechAsrPocketSphinx) << "Starting listening from file" << m_inputFilePath;
    } else if (m_audioInput->state() == QAudio::StoppedState) {
        m_audioBuffer->open(QIODevice::WriteOnly);
        m_audioInput->start(m_audioBuffer);
        qCDebug(lcSpeechAsrPocketSphinx) << "Starting listening from device" << m_device.deviceName();
    }

    if (m_sessionStarted && !m_muted) {
        // Session already active, abort recognition
        ps_end_utt(m_decoder);
    }

    if (!mute) {
        ps_start_utt(m_decoder);
        m_audioBuffer->setFifoLimit(m_audioBufferLimit);
    } else {
        // Discard the audio when on mute
        m_audioBuffer->setFifoLimit(0);
    }
    if (m_debugAudioFile)
        delete m_debugAudioFile;
    m_debugAudioFile = openDebugWavFile("pocketsphinx_audio_" + QString::number(session) + ".wav",
                                        m_format.sampleRate(), m_format.sampleSize(),
                                        m_format.channelCount());
    m_sessionStarted = true;
    m_muted = mute;
    return QSpeechRecognition::NoError;
}

void QSpeechRecognitionEnginePocketSphinx::stopListening(qint64 timestamp)
{
    Q_UNUSED(timestamp); // Not much harm in letting some extra audio get to the recognizer
    if (m_sessionStarted) {
        m_inputFileDecoder.stop();
        if (m_audioInput)
            m_audioInput->stop();
        if (!m_muted) {
            while (processNextAudio()) { }
            ps_end_utt(m_decoder);
            int32 score;
            const char* hyp = ps_get_hyp(m_decoder, &score);
            if (hyp) {
                QString transcription(hyp);
                qCDebug(lcSpeechAsrPocketSphinx) << "Result: " + transcription;
                QVariantMap params;
                params.insert(QSpeechRecognition::Transcription, QVariant(transcription));
                emit result(m_session, m_grammar, params);
                // Store the adapted CMN values:
                storeCmn();
            } else {
                emit error(m_session, QSpeechRecognition::NoResultError, QVariantMap());
            }
        } else {
            emit error(m_session, QSpeechRecognition::NoResultError, QVariantMap());
        }
        m_audioBuffer->close();
        m_audioBuffer->clear(true);
        delete m_debugAudioFile;
        m_debugAudioFile = 0;
        m_sessionStarted = false;
        emit attributeUpdated(NO_SESSION, QSpeechRecognition::AudioLevel, QVariant((qreal)0.0));
    }
}

void QSpeechRecognitionEnginePocketSphinx::abortListening()
{
    if (m_sessionStarted) {
        m_inputFileDecoder.stop();
        if (m_audioInput)
            m_audioInput->stop();
        m_audioBuffer->close();
        m_audioBuffer->clear(true);
        if (!m_muted)
            ps_end_utt(m_decoder);
        m_sessionStarted = false;
        emit attributeUpdated(NO_SESSION, QSpeechRecognition::AudioLevel, QVariant((qreal)0.0));
    }
}

void QSpeechRecognitionEnginePocketSphinx::unmute(qint64 timestamp)
{
    Q_UNUSED(timestamp);
    if (m_sessionStarted && m_muted) {
        m_audioBuffer->clear(); // TODO: Discard only audio that is older than the timestamp
        m_audioBuffer->setFifoLimit(m_audioBufferLimit);
        ps_start_utt(m_decoder);
        m_muted = false;
        // Immediately check if audio is available.
        // For file input this is mandatory, as onAudioDecoderBufferReady() has
        // probably been already called (and ignored).
        emit requestProcess();
    }
}

void QSpeechRecognitionEnginePocketSphinx::reset()
{
    abortListening();
    m_grammar = 0;
}

bool QSpeechRecognitionEnginePocketSphinx::process()
{
    if (m_sessionStarted && !m_muted)
        return processNextAudio();
    return false;
}

void QSpeechRecognitionEnginePocketSphinx::onAudioDataAvailable()
{
    if (m_sessionStarted && !m_muted) {
        emit requestProcess();
        emit attributeUpdated(m_session, QSpeechRecognition::AudioLevel, QVariant(m_audioBuffer->audioLevel()));
    }
}

void QSpeechRecognitionEnginePocketSphinx::onAudioStateChanged(QAudio::State state)
{
    QVariantMap errorParams;
    errorParams.insert(QSpeechRecognition::Engine, name());
    switch (state) {
        case QAudio::ActiveState:
            break;
        case QAudio::StoppedState:
            if (m_audioInput->error() != QAudio::NoError) {
                errorParams.insert(QSpeechRecognition::Reason, QString("Error (") + QString::number(m_audioInput->error()) + ") in QAudioInput");
                emit error(m_session, QSpeechRecognition::AudioError, errorParams);
            }
            break;
        default:
            break;
    }
}

void QSpeechRecognitionEnginePocketSphinx::onAudioDecoderError(QAudioDecoder::Error errorCode)
{
    QVariantMap errorParams;
    errorParams.insert(QSpeechRecognition::Engine, name());
    switch (errorCode) {
        case QAudioDecoder::NoError:
            break;
        default:
            errorParams.insert(QSpeechRecognition::Reason, QString("QAudioDecoder error (") + QString::number(errorCode) + "): " + m_inputFileDecoder.errorString());
            emit error(m_session, QSpeechRecognition::AudioError, errorParams);
            break;
    }
}

void QSpeechRecognitionEnginePocketSphinx::onAudioDecoderBufferReady()
{
    if (m_sessionStarted && !m_muted)
        emit requestProcess();
}

void QSpeechRecognitionEnginePocketSphinx::onAudioDecoderFinished()
{
    emit requestStop(m_session, QDateTime::currentMSecsSinceEpoch());
}

bool QSpeechRecognitionEnginePocketSphinx::createGrammarFromFile(const QString &name, const QString &filePath)
{
    if (m_decoder
    && ps_set_jsgf_file(m_decoder, name.toUtf8().constData(), filePath.toUtf8().constData()) == 0) {
        return true;
    }
    return false;
}

bool QSpeechRecognitionEnginePocketSphinx::createGrammarFromString(const QString &name, const QString &grammarData)
{
    if (m_decoder
    && ps_set_jsgf_string(m_decoder, name.toUtf8().constData(), grammarData.toUtf8().constData()) == 0) {
        return true;
    }
    return false;
}

void QSpeechRecognitionEnginePocketSphinx::deleteGrammar(const QString &name)
{
    ps_unset_search(m_decoder, name.toUtf8().constData());
    if (m_grammar && m_grammar->name() == name)
        m_grammar = 0;
}

QString QSpeechRecognitionEnginePocketSphinx::localizedFilePath(const QString &filePath) const
{
    return QSpeechRecognitionPluginEngine::localizedFilePath(filePath);
}

QVariantMap QSpeechRecognitionEnginePocketSphinx::createEngineParameters(const QVariantMap &inputParameters)
{
    QVariantMap newParameters = inputParameters;
    QStringList audioDeviceNames;
    QString inputDeviceName;
    QAudioDeviceInfo inputDevice = QAudioDeviceInfo::defaultInputDevice();
    if (inputParameters.contains(QSpeechRecognitionEngine::AudioInputDevice))
        inputDeviceName = inputParameters[QSpeechRecognitionEngine::AudioInputDevice].toString();
    QList<QAudioDeviceInfo> audioDevices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach (QAudioDeviceInfo device, audioDevices) {
        audioDeviceNames.append(device.deviceName());
        if (!inputDeviceName.isEmpty() && device.deviceName() == inputDeviceName)
            inputDevice = device;
    }
    newParameters.insert(QSpeechRecognitionEngine::AudioInputDevices, audioDeviceNames);
    newParameters.insert(QSpeechRecognitionEngine::AudioInputDevice, inputDevice.deviceName());
    return newParameters;
}

bool QSpeechRecognitionEnginePocketSphinx::processNextAudio()
{
    if (!m_inputFileDecoder.sourceFilename().isEmpty()) { // File input was selected for this session
        if (m_inputFileDecoder.bufferAvailable()) {
            QAudioBuffer buf = m_inputFileDecoder.read();
            processAudio(buf.constData(), buf.byteCount());
            return m_inputFileDecoder.bufferAvailable();
        }
    } else {
        QSpeechRecognitionAudioBuffer::AudioChunk *chunk = m_audioBuffer->nextChunk();
        if (chunk) {
            processAudio(chunk->data, chunk->audioSize);
            m_audioBuffer->releaseChunk(chunk);
            return !m_audioBuffer->isEmpty();
        }
    }
    return false;
}

void QSpeechRecognitionEnginePocketSphinx::processAudio(const void *data, size_t dataSize)
{
    ps_process_raw(m_decoder, (int16*)data, dataSize/2, 0, 0);
    if (m_debugAudioFile)
        m_debugAudioFile->write((const char*)data, (qint64)dataSize); // Assume little-endian data order in the audio buffer
}

// Store cepstrum mean values to file
void QSpeechRecognitionEnginePocketSphinx::storeCmn()
{
    QFile dataFile(m_cmnFilePath);
    feat_t *feat = ps_get_feat(m_decoder);
    if (dataFile.open(QIODevice::WriteOnly)) {
        QVector<double> dataVec;
        QDataStream dataStream(&dataFile);
        cmn_prior_get(feat->cmn_struct, m_cmnVec);
        for (int i = 0; i < m_cmnSize; i++)
            dataVec.append(m_cmnVec[i]);
        dataStream << dataVec;
        dataFile.close();
    }
}

// Load cepstrum mean values from file
void QSpeechRecognitionEnginePocketSphinx::loadCmn()
{
    QVector<double> dataVec;
    QFile dataFile(m_cmnFilePath);
    if (dataFile.exists() && dataFile.open(QIODevice::ReadOnly)) {
        QDataStream dataStream(&dataFile);
        dataStream >> dataVec;
    }
    feat_t *feat = ps_get_feat(m_decoder);
    if (dataVec.size() == m_cmnSize) {
        for (int i = 0; i < m_cmnSize; i++)
            m_cmnVec[i] = (mfcc_t)dataVec[i];
        cmn_prior_set(feat->cmn_struct, m_cmnVec);
    } else {
        // Just to leave m_cmnVec with valid values:
        cmn_prior_get(feat->cmn_struct, m_cmnVec);
    }
    dataFile.close();
}

QT_END_NAMESPACE
