/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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

#ifndef QTEXTTOSPEECHPROCESSOR_FLITE_H
#define QTEXTTOSPEECHPROCESSOR_FLITE_H

#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QtCore/QSharedPointer>
#include <QtCore/QLibrary>
#include <QtCore/QString>
#include <QtCore/QBasicTimer>
#include <QtCore/QTimerEvent>
#include <QtCore/QAbstractEventDispatcher>
#include <QtCore/QProcessEnvironment>
#include <QtCore/QDateTime>
#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>

#include <flite/flite.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechProcessorFlite : public QObject
{
    Q_OBJECT

public:
    QTextToSpeechProcessorFlite();
    ~QTextToSpeechProcessorFlite();

    struct VoiceInfo
    {
        int id;
        cst_voice *vox;
        void (*unregister_func)(cst_voice *vox);
        QString name;
        QString locale;
        QVoice::Gender gender;
        QVoice::Age age;
    };

    Q_INVOKABLE void say(const QString &text, int voiceId, double pitch, double rate, double volume);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void resume();
    Q_INVOKABLE void stop();

    const QList<QTextToSpeechProcessorFlite::VoiceInfo> &voices() const;
    static constexpr QTextToSpeech::State audioStateToTts(QAudio::State audioState);

private:
    // Process a single text
    void processText(const QString &text, int voiceId, double pitch, double rate);

    // Flite callbacks
    static int fliteOutputCb(const cst_wave *w, int start, int size,
                            int last, cst_audio_streaming_info *asi);
    int fliteOutput(const cst_wave *w, int start, int size,
                    int last, cst_audio_streaming_info *asi);

    bool audioOutput(const char *data, qint64 dataSize, QString &errorString);
    void setRateForVoice(cst_voice *voice, float rate);
    void setPitchForVoice(cst_voice *voice, float pitch);

    bool init();
    bool initAudio(double rate, int channelCount);
    void deinitAudio();
    bool checkFormat(const QAudioFormat &format, const QAudioDevice &device);
    bool checkVoice(int voiceId);
    void deleteSink();
    void createSink();
    QAudio::State audioSinkState() const;

    inline int remainingTime() const;

    void startTimer(int msecs);
    enum TimeoutReason {TimeOut, Stop};
    int stopTimer(TimeoutReason reason = Stop);
    void timerEvent(QTimerEvent *event) override;
    void setError(QAudio::Error err, const QString &errorString);
    QAudio::Error error() const;
    void clearError() { setError(QAudio::NoError, QString()); };

    // Read available flite voices
    QStringList fliteAvailableVoices(const QString &langCode) const;

private slots:
    void changeState(QAudio::State newState);

Q_SIGNALS:
    void errorChanged(QAudio::Error error, const QString &errorString);
    void stateChanged(QTextToSpeech::State);

private:
    QAudioSink *m_audioSink = nullptr;
    QAudio::State m_state = QAudio::IdleState;
    QIODevice *m_audioBuffer = nullptr;
    QBasicTimer m_sinkTimer;
    int m_sinkTimerPausedAt = 0;
    QAudio::Error m_error = QAudio::NoError;

    QAudioFormat m_format;
    double m_volume = 1;

    QList<VoiceInfo> m_voices;

    // Statistics for debugging
    qint64 numberChunks = 0;
    qint64 totalBytes = 0;
};

QT_END_NAMESPACE

#endif
