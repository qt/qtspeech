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

#ifndef QTEXTTOSPEECHENGINE_FLITE_H
#define QTEXTTOSPEECHENGINE_FLITE_H

#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/QString>
#include <QtCore/QLocale>
#include <QtCore/QVector>
#include <QtCore/QThread>
#include <QtCore/QMutex>
#include <QtCore/QSemaphore>
#include <QtCore/QIODevice>
#include <QtMultimedia/QAudioOutput>

#include <flite/flite.h>

QT_BEGIN_NAMESPACE

class FliteProcessor : public QThread {
    Q_OBJECT

public:
    FliteProcessor();
    ~FliteProcessor();
    void say(cst_voice *voice, const QString &text);
    void stop();
    bool setRate(float rate);
    bool setPitch(float pitch);
    bool setVolume(int volume);
    void exit();
    bool isIdle();
    float rate();
    float pitch();
    int volume();

signals:
    void notSpeaking();

private:
    QMutex m_lock;
    bool m_stop;
    bool m_idle;
    float m_rate;
    float m_pitch;
    int m_volume;
    QSemaphore m_speakSem;
    QString m_nextText;
    cst_voice *m_nextVoice;
    QAudioOutput *m_audio;
    QIODevice *m_audioBuffer;
    void run();
    void setRateForVoice(cst_voice *voice, float rate);
    void setPitchForVoice(cst_voice *voice, float pitch);
    int audioOutput(const cst_wave *w, int start, int size,
                    int last, cst_audio_streaming_info *asi);
    static int fliteAudioCb(const cst_wave *w, int start, int size,
                            int last, cst_audio_streaming_info *asi);
};

class QTextToSpeechEngineFlite : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineFlite(const QVariantMap &parameters, QObject *parent);
    virtual ~QTextToSpeechEngineFlite();

    // Plug-in API:
    QVector<QLocale> availableLocales() const Q_DECL_OVERRIDE;
    QVector<QVoice> availableVoices() const Q_DECL_OVERRIDE;
    void say(const QString &text) Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    void resume() Q_DECL_OVERRIDE;
    double rate() const Q_DECL_OVERRIDE;
    bool setRate(double rate) Q_DECL_OVERRIDE;
    double pitch() const Q_DECL_OVERRIDE;
    bool setPitch(double pitch) Q_DECL_OVERRIDE;
    QLocale locale() const Q_DECL_OVERRIDE;
    bool setLocale(const QLocale &locale) Q_DECL_OVERRIDE;
    int volume() const Q_DECL_OVERRIDE;
    bool setVolume(int volume) Q_DECL_OVERRIDE;
    QVoice voice() const Q_DECL_OVERRIDE;
    bool setVoice(const QVoice &voice) Q_DECL_OVERRIDE;
    QTextToSpeech::State state() const Q_DECL_OVERRIDE;

    // Internal API:
    bool init(QString *errorString);

public slots:
    void onNotSpeaking();

private:
    QTextToSpeech::State m_state;
    QLocale m_currentLocale;
    QVector<QLocale> m_locales;
    QVoice m_currentVoice;
    // Voices mapped by their locale name.
    QMultiMap<QString, QVoice> m_voices;
};

QT_END_NAMESPACE

#endif
