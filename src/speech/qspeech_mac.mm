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



#include "qspeech.h"
#include "qspeech_p.h"

#import <Cocoa/Cocoa.h>
#include <qdebug.h>


QT_BEGIN_NAMESPACE

class QSpeechPrivateMac;

@interface StateDelegate : NSObject <NSSpeechSynthesizerDelegate>
{
    QSpeechPrivateMac *speechPrivate;
}
@end

class QSpeechPrivateMac : public QSpeechPrivate
{
public:
    QSpeechPrivateMac(QSpeech *speech);
    ~QSpeechPrivateMac();

    void say(const QString &text);
    void stop();
    void pause();
    void resume();

    void setRate(double rate);
    void setPitch(double pitch);
    void setVolume(double volume);
    QSpeech::State state() const;

    bool isPaused() const { return false; }
    bool isSpeaking() const;

    void speechStopped(bool success);

private:
    NSSpeechSynthesizer *speechSynthesizer;
    StateDelegate *stateDelegate;
};

@implementation StateDelegate
- (id)initWithSpeechPrivate:(QSpeechPrivateMac *) priv {
    self = [super init];
    speechPrivate = priv;
    return self;
}
- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)success {
    Q_UNUSED(sender);
    speechPrivate->speechStopped(success);
}
@end

QSpeech::QSpeech(QObject *parent)
    : QObject(*new QSpeechPrivateMac(this), parent)
{
    qRegisterMetaType<QSpeech::State>();
}

QSpeechPrivateMac::QSpeechPrivateMac(QSpeech *speech)
    : QSpeechPrivate(speech)
{
    stateDelegate = [[StateDelegate alloc] initWithSpeechPrivate:this];

    speechSynthesizer = [[NSSpeechSynthesizer alloc] init];
    [speechSynthesizer setDelegate: stateDelegate];
}

QSpeechPrivateMac::~QSpeechPrivateMac()
{
    [speechSynthesizer release];
    [stateDelegate release];
}


QSpeech::State QSpeechPrivate::state() const
{
    return m_state;
}

bool QSpeechPrivateMac::isSpeaking() const
{
    return [speechSynthesizer isSpeaking];
}

void QSpeechPrivateMac::speechStopped(bool success)
{
    Q_UNUSED(success);
    if (m_state != QSpeech::Ready) {
        m_state = QSpeech::Ready;
        emitStateChanged(m_state);
    }
}

void QSpeechPrivateMac::say(const QString &text)
{
    if (text.isEmpty())
        return;

    if (m_state != QSpeech::Ready)
        stop();

    if([speechSynthesizer isSpeaking]) {
        [speechSynthesizer stopSpeaking];
    }

    NSString *ntext = text.toNSString();
    [speechSynthesizer startSpeakingString:ntext];

    if ([speechSynthesizer isSpeaking] && m_state != QSpeech::Speaking) {
        m_state = QSpeech::Speaking;
        emitStateChanged(m_state);
    }
}

void QSpeechPrivateMac::stop()
{
    if([speechSynthesizer isSpeaking])
        [speechSynthesizer stopSpeaking];
}

void QSpeechPrivateMac::pause()
{
}

void QSpeechPrivateMac::resume()
{
}

void QSpeechPrivateMac::setPitch(double pitch)
{
}

void QSpeechPrivateMac::setRate(double rate)
{
}

void QSpeechPrivateMac::setVolume(double volume)
{
}

QSpeech::State QSpeechPrivateMac::state() const
{
    return m_state;
}


QT_END_NAMESPACE
