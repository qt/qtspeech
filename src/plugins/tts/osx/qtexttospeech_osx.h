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

#ifndef QTEXTTOSPEECHENGINE_OSX_H
#define QTEXTTOSPEECHENGINE_OSX_H

#include <QtCore/qobject.h>
#include <QtCore/qvector.h>
#include <QtCore/qstring.h>
#include <QtCore/qlocale.h>
#include <QtTextToSpeech/qtexttospeechengine.h>
#include <QtTextToSpeech/qvoice.h>

Q_FORWARD_DECLARE_OBJC_CLASS(QT_MANGLE_NAMESPACE(StateDelegate));
Q_FORWARD_DECLARE_OBJC_CLASS(NSSpeechSynthesizer);
Q_FORWARD_DECLARE_OBJC_CLASS(NSString);

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineOsx : public QTextToSpeechEngine
{
    Q_OBJECT

public:
    QTextToSpeechEngineOsx(const QVariantMap &parameters, QObject *parent);
    ~QTextToSpeechEngineOsx();

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

    void speechStopped(bool);

private:
    void updateVoices();
    bool isSpeaking() const;
    bool isPaused() const;

    QTextToSpeech::State m_state;
//    QVoice m_currentVoice;

    QVoice voiceForNSVoice(NSString *voiceString) const;
    NSSpeechSynthesizer *speechSynthesizer;
    QT_MANGLE_NAMESPACE(StateDelegate) *stateDelegate;
    QVector<QLocale> m_locales;
    QMultiMap<QString, QVoice> m_voices;
};

QT_END_NAMESPACE

#endif
