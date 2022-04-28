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


#include "qtexttospeech_mock.h"
#include <QtCore/QTimerEvent>

QT_BEGIN_NAMESPACE

QTextToSpeechEngineMock::QTextToSpeechEngineMock(const QVariantMap &parameters, QObject *parent)
    : QTextToSpeechEngine(parent), m_parameters(parameters)
{
    m_locale = availableLocales().first();
    m_voice = availableVoices().first();
}

QTextToSpeechEngineMock::~QTextToSpeechEngineMock()
{
}

QList<QLocale> QTextToSpeechEngineMock::availableLocales() const
{
    QList<QLocale> locales;

    locales << QLocale(QLocale::English, QLocale::UnitedKingdom)
            << QLocale(QLocale::NorwegianBokmal, QLocale::Norway)
            << QLocale(QLocale::Finnish, QLocale::Finland);

    return locales;
}

QList<QVoice> QTextToSpeechEngineMock::availableVoices() const
{
    QList<QVoice> voices;

    const QString voiceData = m_locale.bcp47Name();
    switch (m_locale.language()) {
    case QLocale::English: {
        voices << createVoice("Bob", m_locale, QVoice::Male, QVoice::Adult, voiceData + QLatin1String("-1"))
               << createVoice("Anne", m_locale, QVoice::Female, QVoice::Adult, voiceData + QLatin1String("-2"));
        break;
    }
    case QLocale::NorwegianBokmal:
        voices << createVoice("Eivind", m_locale, QVoice::Male, QVoice::Adult, voiceData + QLatin1String("-1"))
               << createVoice("Kjersti", m_locale, QVoice::Female, QVoice::Adult, voiceData + QLatin1String("-2"));
        break;
    case QLocale::Finnish:
        voices << createVoice("Kari", m_locale, QVoice::Male, QVoice::Adult, voiceData + QLatin1String("-1"))
               << createVoice("Anneli", m_locale, QVoice::Female, QVoice::Adult, voiceData + QLatin1String("-2"));
        break;
    default:
        Q_ASSERT_X(false, "availableVoices", "Unsupported locale!");
        break;
    }
    return voices;
}

void QTextToSpeechEngineMock::say(const QString &text)
{
    m_words = text.split(" ");
    m_timer.start(wordTime(), Qt::PreciseTimer, this);
    m_state = QTextToSpeech::Speaking;
    stateChanged(m_state);
}

void QTextToSpeechEngineMock::stop()
{
    if (m_state == QTextToSpeech::Ready || m_state == QTextToSpeech::BackendError)
        return;

    Q_ASSERT(m_timer.isActive());
    // finish immediately
    m_words.clear();
    m_timer.stop();

    m_state = QTextToSpeech::Ready;
    stateChanged(m_state);
}

void QTextToSpeechEngineMock::pause()
{
    if (m_state != QTextToSpeech::Speaking)
        return;

    // implement "pause after word end"
    m_pauseRequested = true;
}

void QTextToSpeechEngineMock::resume()
{
    if (m_state != QTextToSpeech::Paused)
        return;

    m_timer.start(wordTime(), Qt::PreciseTimer, this);
    m_state = QTextToSpeech::Speaking;
    stateChanged(m_state);
}

void QTextToSpeechEngineMock::timerEvent(QTimerEvent *e)
{
    if (e->timerId() != m_timer.timerId()) {
        QTextToSpeechEngine::timerEvent(e);
        return;
    }

    Q_ASSERT(m_state == QTextToSpeech::Speaking);
    Q_ASSERT(m_words.count());

    m_words.takeFirst(); // next word has been spoken

    if (m_words.isEmpty()) {
        // done speaking all words
        m_timer.stop();
        m_state = QTextToSpeech::Ready;
        stateChanged(m_state);
    } else if (m_pauseRequested) {
        m_timer.stop();
        m_state = QTextToSpeech::Paused;
        stateChanged(m_state);
    }
    m_pauseRequested = false;
}

double QTextToSpeechEngineMock::rate() const
{
    return m_rate;
}

bool QTextToSpeechEngineMock::setRate(double rate)
{
    m_rate = rate;
    if (m_timer.isActive()) {
        m_timer.stop();
        m_timer.start(wordTime(), Qt::PreciseTimer, this);
    }
    return true;
}

double QTextToSpeechEngineMock::pitch() const
{
    return m_pitch;
}

bool QTextToSpeechEngineMock::setPitch(double pitch)
{
    m_pitch = pitch;
    return true;
}

QLocale QTextToSpeechEngineMock::locale() const
{
    return m_locale;
}

bool QTextToSpeechEngineMock::setLocale(const QLocale &locale)
{
    if (!availableLocales().contains(locale))
        return false;
    m_locale = locale;
    const auto voices = availableVoices();
    if (!voices.contains(m_voice))
        m_voice = voices.isEmpty() ? QVoice() : voices.first();
    return true;
}

double QTextToSpeechEngineMock::volume() const
{
    return m_volume;
}

bool QTextToSpeechEngineMock::setVolume(double volume)
{
    if (volume < 0.0 || volume > 1.0)
        return false;

    m_volume = volume;
    return true;
}

QVoice QTextToSpeechEngineMock::voice() const
{
    return m_voice;
}

bool QTextToSpeechEngineMock::setVoice(const QVoice &voice)
{
    const QString voiceId = voiceData(voice).toString();
    const QLocale voiceLocale = QLocale(voiceId.left(voiceId.lastIndexOf("-")));
    if (!availableLocales().contains(voiceLocale)) {
        qWarning("Engine does not support voice's locale %s",
                 qPrintable(voiceLocale.bcp47Name()));
        return false;
    }
    m_locale = voiceLocale;
    if (!availableVoices().contains(voice)) {
        qWarning("Engine does not support voice %s in the locale %s",
                 qPrintable(voice.name()), qPrintable(voiceLocale.bcp47Name()));
        return false;
    }
    m_voice = voice;
    return true;
}

QTextToSpeech::State QTextToSpeechEngineMock::state() const
{
    return m_state;
}


QT_END_NAMESPACE
