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

#ifndef QTEXTTOSPEECHENGINE_SPEECHD_H
#define QTEXTTOSPEECHENGINE_SPEECHD_H

#include "qtexttospeechengine.h"
#include "qvoice.h"

#include <QtCore/qobject.h>
#include <QtCore/qvector.h>
#include <QtCore/qstring.h>
#include <QtCore/qlocale.h>
#include <speech-dispatcher/libspeechd.h>

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineSpeechd : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineSpeechd(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineSpeechd();

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
    double volume() const Q_DECL_OVERRIDE;
    bool setVolume(double volume) Q_DECL_OVERRIDE;
    QVoice voice() const Q_DECL_OVERRIDE;
    bool setVoice(const QVoice &voice) Q_DECL_OVERRIDE;
    QTextToSpeech::State state() const Q_DECL_OVERRIDE;

    void spdStateChanged(SPDNotificationType state);

private:

    QLocale localeForVoice(SPDVoice *voice) const;
    bool connectToSpeechDispatcher();
    void updateVoices();

    QTextToSpeech::State m_state;
    SPDConnection *speechDispatcher;
    QLocale m_currentLocale;
    QVector<QLocale> m_locales;
    QVoice m_currentVoice;
    // Voices mapped by their locale name.
    QMultiMap<QString, QVoice> m_voices;
};
QT_END_NAMESPACE

#endif
