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

#ifndef QSPEECHRECOGNITIONENGINE_POCKETSPHINX_H
#define QSPEECHRECOGNITIONENGINE_POCKETSPHINX_H

#include "qspeechrecognitionplugin_pocketsphinx.h"
#include "qspeechrecognitionaudiobuffer.h"

#include <QtCore/QFile>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioDecoder>

#include <pocketsphinx.h>

QT_BEGIN_NAMESPACE

class QSpeechRecognitionAudioBuffer;
class QSpeechRecognitionGrammarPocketSphinx;

class QSpeechRecognitionEnginePocketSphinx : public QSpeechRecognitionPluginEngine
{
    Q_OBJECT
public:
    QSpeechRecognitionEnginePocketSphinx(const QString &name, const QVariantMap &parameters, QObject *parent);
    virtual ~QSpeechRecognitionEnginePocketSphinx();

    // Plug-in API:
    QSpeechRecognition::Error updateParameter(const QString &key, const QVariant &value, QString *errorString);
    QSpeechRecognitionPluginGrammar *createGrammar(const QString &name, const QUrl &location, QString *errorString);
    QSpeechRecognition::Error setGrammar(QSpeechRecognitionPluginGrammar *grammar, QString *errorString);
    QSpeechRecognition::Error startListening(int session, bool mute, QString *errorString);
    void stopListening(qint64 timestamp);
    void abortListening();
    void unmute(qint64 timestamp);
    void reset();
    bool process();

    // Internal API:
    bool init(QString *errorString);
    void createAudioInput();
    bool createGrammarFromFile(const QString &name, const QString &filePath);
    bool createGrammarFromString(const QString &name, const QString &grammarData);
    void deleteGrammar(const QString &name);
    QString localizedFilePath(const QString &filePath) const;

public slots:
    void onAudioDataAvailable();
    void onAudioStateChanged(QAudio::State state);
    void onAudioDecoderError(QAudioDecoder::Error errorCode);
    void onAudioDecoderBufferReady();
    void onAudioDecoderFinished();
private:
    static QVariantMap createEngineParameters(const QVariantMap &inputParameters);
    bool processNextAudio();
    void processAudio(const void *data, size_t dataSize);
    void storeCmn();
    void loadCmn();
    int m_session;
    bool m_muted;
    ps_decoder_t *m_decoder;
    QAudioFormat m_format;
    QAudioDeviceInfo m_device;
    QAudioInput *m_audioInput;
    QAudioDecoder m_inputFileDecoder;
    QString m_inputFilePath;
    QSpeechRecognitionGrammarPocketSphinx *m_grammar;
    QSpeechRecognitionAudioBuffer *m_audioBuffer;
    int m_audioBufferLimit;
    QFile *m_debugAudioFile;
    bool m_sessionStarted;
    QString m_cmnFilePath;
    mfcc_t *m_cmnVec;
    int m_cmnSize;
};

QT_END_NAMESPACE

#endif
