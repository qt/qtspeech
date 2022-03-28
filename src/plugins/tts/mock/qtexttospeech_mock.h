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

#ifndef QTEXTTOSPEECH_MOCK_H
#define QTEXTTOSPEECH_MOCK_H

#include "qtexttospeechengine.h"
#include <QtCore/QBasicTimer>

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineMock : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    explicit QTextToSpeechEngineMock(const QVariantMap &parameters, QObject *parent = nullptr);
    ~QTextToSpeechEngineMock();

    QList<QLocale> availableLocales() const override;
    QList<QVoice> availableVoices() const override;

    void say(const QString &text) override;
    void stop() override;
    void pause() override;
    void resume() override;

    double rate() const override;
    bool setRate(double rate) override;
    double pitch() const override;
    bool setPitch(double pitch) override;
    QLocale locale() const override;
    bool setLocale(const QLocale &locale) override;
    double volume() const override;
    bool setVolume(double volume) override;
    QVoice voice() const override;
    bool setVoice(const QVoice &voice) override;
    QTextToSpeech::State state() const override;

protected:
    void timerEvent(QTimerEvent *e) override;

private:
    // mock engine uses 100ms per word, +/- 50ms depending on rate
    constexpr int wordTime() { return 100 - int(50.0 * m_rate); }

    const QVariantMap m_parameters;
    QStringList m_words;
    QLocale m_locale;
    QVoice m_voice;
    QBasicTimer m_timer;
    double m_rate = 0.0;
    double m_pitch = 0.0;
    double m_volume = 0.5;
    QTextToSpeech::State m_state = QTextToSpeech::Ready;
    bool m_pauseRequested = false;
};

QT_END_NAMESPACE

#endif
