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

#include "qtexttospeech_flite.h"

QT_BEGIN_NAMESPACE

QTextToSpeechEngineFlite::QTextToSpeechEngineFlite(QString *errorString, const QVariantMap &parameters, QObject *parent)
    : QTextToSpeechEngine(parent)
{
    Q_UNUSED(parameters);

    *errorString = QString();

    // Connect processor to engine for state changes and error
    connect(&m_processor, &QTextToSpeechProcessorFlite::stateChanged, this,
            &QTextToSpeechEngineFlite::changeState);

    // Read voices from processor before moving it to a separate thread
    const QList<QTextToSpeechProcessorFlite::VoiceInfo> voices = m_processor.voices();

    int i = 0;
    for (const QTextToSpeechProcessorFlite::VoiceInfo &voiceInfo : voices) {
        const QLocale locale(voiceInfo.locale);
        const QVoice voice = QTextToSpeechEngine::createVoice(voiceInfo.name, locale,
                                                              voiceInfo.gender, voiceInfo.age,
                                                              QVariant(voiceInfo.id));
        m_voices.insert(locale, voice);
        // Use the first available locale/voice as a fallback
        if (i == 0)
            m_voice = voice;
        i++;
    }

    if (i)
        m_state = QTextToSpeech::Ready;

    m_processor.moveToThread(&m_thread);
    m_thread.start();
}

QTextToSpeechEngineFlite::~QTextToSpeechEngineFlite()
{
    m_thread.exit();
    m_thread.wait();
}

QList<QLocale> QTextToSpeechEngineFlite::availableLocales() const
{
    return m_voices.uniqueKeys();
}

QList<QVoice> QTextToSpeechEngineFlite::availableVoices() const
{
    return m_voices.values(m_voice.locale());
}

void QTextToSpeechEngineFlite::say(const QString &text)
{
    QMetaObject::invokeMethod(&m_processor, "say", Qt::QueuedConnection, Q_ARG(QString, text),
                              Q_ARG(int, voiceData(voice()).toInt()), Q_ARG(double, pitch()),
                              Q_ARG(double, rate()), Q_ARG(double, volume()));
}

void QTextToSpeechEngineFlite::stop()
{
    QMetaObject::invokeMethod(&m_processor, &QTextToSpeechProcessorFlite::stop, Qt::QueuedConnection);
}

void QTextToSpeechEngineFlite::pause()
{
    QMetaObject::invokeMethod(&m_processor, &QTextToSpeechProcessorFlite::pause, Qt::QueuedConnection);
}

void QTextToSpeechEngineFlite::resume()
{
    QMetaObject::invokeMethod(&m_processor, &QTextToSpeechProcessorFlite::resume, Qt::QueuedConnection);
}

double QTextToSpeechEngineFlite::rate() const
{
    return m_rate;
}

bool QTextToSpeechEngineFlite::setRate(double rate)
{
    if (m_rate == rate)
        return false;

    m_rate = rate;
    return true;
}

double QTextToSpeechEngineFlite::pitch() const
{
    return m_pitch;
}

bool QTextToSpeechEngineFlite::setPitch(double pitch)
{
    if (m_pitch == pitch)
        return false;

    m_pitch = pitch;
    return true;
}

QLocale QTextToSpeechEngineFlite::locale() const
{
    return m_voice.locale();
}

bool QTextToSpeechEngineFlite::setLocale(const QLocale &locale)
{
    const auto &voices = m_voices.values(locale);
    if (voices.isEmpty())
        return false;
    // The list returned by QMultiHash::values is reversed
    setVoice(voices.last());
    return true;
}

double QTextToSpeechEngineFlite::volume() const
{
    return m_volume;
}

bool QTextToSpeechEngineFlite::setVolume(double volume)
{
    if (m_volume == volume)
        return false;

    m_volume = volume;
    return true;
}

QVoice QTextToSpeechEngineFlite::voice() const
{
    return m_voice;
}

bool QTextToSpeechEngineFlite::setVoice(const QVoice &voice)
{
    QLocale locale = m_voices.key(voice); // returns default locale if not found, so
    if (!m_voices.contains(locale, voice)) {
        qWarning() << "Voice" << voice << "is not supported by this engine";
        return false;
    }

    m_voice = voice;
    return true;
}

void QTextToSpeechEngineFlite::changeState(QTextToSpeech::State newState)
{
    if (newState != m_state) {
        m_state = newState;
        emit stateChanged(newState);
    }
}

QTextToSpeech::State QTextToSpeechEngineFlite::state() const
{
    return m_state;
}

QT_END_NAMESPACE
