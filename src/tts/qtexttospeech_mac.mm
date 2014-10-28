/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtWidgets module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/



#include "qtexttospeech.h"
#include "qtexttospeech_p.h"

#import <Cocoa/Cocoa.h>
#include <qdebug.h>


QT_BEGIN_NAMESPACE

class QTextToSpeechPrivateMac;

@interface StateDelegate : NSObject <NSSpeechSynthesizerDelegate>
{
    QTextToSpeechPrivateMac *speechPrivate;
}
@end

class QTextToSpeechPrivateMac : public QTextToSpeechPrivate
{
public:
    QTextToSpeechPrivateMac(QTextToSpeech *speech);
    ~QTextToSpeechPrivateMac();

    QVector<QLocale> availableLocales() const Q_DECL_OVERRIDE;
    void say(const QString &text) Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    void resume() Q_DECL_OVERRIDE;

    void setRate(double rate) Q_DECL_OVERRIDE;
    void setPitch(double pitch) Q_DECL_OVERRIDE;
    void setVolume(int volume) Q_DECL_OVERRIDE;
    void setLocale(const QLocale &locale) Q_DECL_OVERRIDE;
    QLocale currentLocale() const Q_DECL_OVERRIDE;
    QTextToSpeech::State state() const Q_DECL_OVERRIDE;

    bool isPaused() const { return false; }
    bool isSpeaking() const;

    void speechStopped(bool success);

private:
    NSSpeechSynthesizer *speechSynthesizer;
    StateDelegate *stateDelegate;
};

@implementation StateDelegate
- (id)initWithSpeechPrivate:(QTextToSpeechPrivateMac *) priv {
    self = [super init];
    speechPrivate = priv;
    return self;
}
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)success {
    Q_UNUSED(sender);
    speechPrivate->speechStopped(success);
}
@end

QTextToSpeech::QTextToSpeech(QObject *parent)
    : QObject(*new QTextToSpeechPrivateMac(this), parent)
{
    qRegisterMetaType<QTextToSpeech::State>();
}

QTextToSpeechPrivateMac::QTextToSpeechPrivateMac(QTextToSpeech *speech)
    : QTextToSpeechPrivate(speech)
{
    stateDelegate = [[StateDelegate alloc] initWithSpeechPrivate:this];

    speechSynthesizer = [[NSSpeechSynthesizer alloc] init];
    [speechSynthesizer setDelegate: stateDelegate];
}

QTextToSpeechPrivateMac::~QTextToSpeechPrivateMac()
{
    [speechSynthesizer release];
    [stateDelegate release];
}


QTextToSpeech::State QTextToSpeechPrivate::state() const
{
    return m_state;
}

bool QTextToSpeechPrivateMac::isSpeaking() const
{
    return [speechSynthesizer isSpeaking];
}

void QTextToSpeechPrivateMac::speechStopped(bool success)
{
    Q_UNUSED(success);
    if (m_state != QTextToSpeech::Ready) {
        m_state = QTextToSpeech::Ready;
        emitStateChanged(m_state);
    }
}

void QTextToSpeechPrivateMac::say(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_state != QTextToSpeech::Ready)
        stop();

    if([speechSynthesizer isSpeaking]) {
        [speechSynthesizer stopSpeaking];
    }

    NSString *ntext = text.toNSString();
    [speechSynthesizer startSpeakingString:ntext];

    if ([speechSynthesizer isSpeaking] && m_state != QTextToSpeech::Speaking) {
        m_state = QTextToSpeech::Speaking;
        emitStateChanged(m_state);
    }
}

void QTextToSpeechPrivateMac::stop()
{
    if([speechSynthesizer isSpeaking])
        [speechSynthesizer stopSpeaking];
}

void QTextToSpeechPrivateMac::pause()
{
    if ([speechSynthesizer isSpeaking]) {
        [speechSynthesizer pauseSpeakingAtBoundary: NSSpeechWordBoundary];
        m_state = QTextToSpeech::Paused;
        emitStateChanged(m_state);
    }
}

void QTextToSpeechPrivateMac::resume()
{
    m_state = QTextToSpeech::Speaking;
    emitStateChanged(m_state);
    [speechSynthesizer continueSpeaking];
}

void QTextToSpeechPrivateMac::setPitch(double pitch)
{
}

void QTextToSpeechPrivateMac::setRate(double rate)
{
    // NSSpeechSynthesizer supports words per minute,
    // human speech is 180 to 220 - use 0 to 400 as range here
    [speechSynthesizer setRate: 200 + (rate * 200)];
}

void QTextToSpeechPrivateMac::setVolume(int volume)
{
    [speechSynthesizer setVolume: volume / 100.0];
}

QVector<QLocale> QTextToSpeechPrivateMac::availableLocales() const
{
    return QVector<QLocale>();
}

void QTextToSpeechPrivateMac::setLocale(const QLocale &locale)
{
}

QLocale QTextToSpeechPrivateMac::currentLocale() const
{
    return QLocale();
}

QTextToSpeech::State QTextToSpeechPrivateMac::state() const
{
    return m_state;
}


QT_END_NAMESPACE
