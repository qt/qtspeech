/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSpeech module of the Qt Toolkit.
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
    QVector<QVoice> availableVoices() const Q_DECL_OVERRIDE;
    void say(const QString &text) Q_DECL_OVERRIDE;
    void stop() Q_DECL_OVERRIDE;
    void pause() Q_DECL_OVERRIDE;
    void resume() Q_DECL_OVERRIDE;

    double rate() const Q_DECL_OVERRIDE;
    void setRate(double rate) Q_DECL_OVERRIDE;
    double pitch() const Q_DECL_OVERRIDE;
    void setPitch(double pitch) Q_DECL_OVERRIDE;
    int volume() const Q_DECL_OVERRIDE;
    void setVolume(int volume) Q_DECL_OVERRIDE;
    void setLocale(const QLocale &locale) Q_DECL_OVERRIDE;
    QLocale locale() const Q_DECL_OVERRIDE;
    void setVoice(const QVoice &voice) Q_DECL_OVERRIDE;
    QVoice voice() const Q_DECL_OVERRIDE;
    QTextToSpeech::State state() const Q_DECL_OVERRIDE;

    bool isPaused() const;
    bool isSpeaking() const;

    void speechStopped(bool success);

private:
    QVoice voiceForNSVoice(NSString *voiceString) const;
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

bool QTextToSpeechPrivateMac::isPaused() const
{
    return m_state == QTextToSpeech::Paused;
}

void QTextToSpeechPrivateMac::resume()
{
    m_state = QTextToSpeech::Speaking;
    emitStateChanged(m_state);
    [speechSynthesizer continueSpeaking];
}

double QTextToSpeechPrivateMac::rate() const
{
    return [speechSynthesizer rate] / 200 - 200;
}

void QTextToSpeechPrivateMac::setPitch(double pitch)
{
    // 30 to 65
    double p = 30.0 + ((pitch + 1.0) / 2.0) * 35.0;
    [speechSynthesizer setObject:[NSNumber numberWithFloat:p] forProperty:NSSpeechPitchBaseProperty error:nil];
}

double QTextToSpeechPrivateMac::pitch() const
{
    double pitch = [[speechSynthesizer objectForProperty:NSSpeechPitchBaseProperty error:nil] floatValue];
    return (pitch - 30.0) / 35.0 * 2.0 - 1.0;
}

int QTextToSpeechPrivateMac::volume() const
{
    return [speechSynthesizer volume] * 100;
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

QLocale localeForVoice(NSString *voice)
{
    NSDictionary *attrs = [NSSpeechSynthesizer attributesForVoice:voice];
    return QString::fromNSString(attrs[NSVoiceLocaleIdentifier]);
}

QVoice QTextToSpeechPrivateMac::voiceForNSVoice(NSString *voiceString) const
{
    NSDictionary *attrs = [NSSpeechSynthesizer attributesForVoice:voiceString];
    QVoice voice;
    voice.setName(QString::fromNSString(attrs[NSVoiceName]));
    NSString *gender = attrs[NSVoiceGender];
    voice.setGender(gender == NSVoiceGenderMale ? QVoice::Male :
                    gender == NSVoiceGenderFemale ? QVoice::Female :
                    QVoice::Unknown);
    NSString *age = attrs[NSVoiceAge];
    int voiceAge = QString::fromNSString(age).toInt();
    voice.setAge(voiceAge < 13 ? QVoice::Child :
                 voiceAge < 20 ? QVoice::Teenager :
                 voiceAge < 45 ? QVoice::Adult :
                 voiceAge < 90 ? QVoice::Senior : QVoice::Other);
    voice.setData(QVariant(QString::fromNSString(attrs[NSVoiceIdentifier])));
    return voice;
}

QVector<QLocale> QTextToSpeechPrivateMac::availableLocales() const
{
    QVector<QLocale> locales;
    NSArray *voices = [NSSpeechSynthesizer availableVoices];
    for (NSString *voice in voices) {
        QLocale locale = localeForVoice(voice);
        if (!locales.contains(locale))
            locales.append(locale);
    }
    return locales;
}

void QTextToSpeechPrivateMac::setLocale(const QLocale &locale)
{
    NSArray *voices = [NSSpeechSynthesizer availableVoices];
    NSString *voice = [NSSpeechSynthesizer defaultVoice];
    // always prefer default
    if (locale == localeForVoice(voice)) {
        [speechSynthesizer setVoice:voice];
        emitLocaleChanged(locale);
        emitVoiceChanged(voiceForNSVoice(voice));
        return;
    }

    for (voice in voices) {
        QLocale voiceLocale = localeForVoice(voice);
        if (locale == voiceLocale) {
            [speechSynthesizer setVoice:voice];
            emitLocaleChanged(locale);
            emitVoiceChanged(voiceForNSVoice(voice));
            return;
        }
    }
}

QLocale QTextToSpeechPrivateMac::locale() const
{
    NSString *voice = [speechSynthesizer voice];
    return localeForVoice(voice);
}

QTextToSpeech::State QTextToSpeechPrivateMac::state() const
{
    return m_state;
}

QVector<QVoice> QTextToSpeechPrivateMac::availableVoices() const
{
    QVector<QVoice> voiceList;
    NSArray *voices = [NSSpeechSynthesizer availableVoices];
    for (NSString *voice in voices) {
        QVoice data = voiceForNSVoice(voice);
        voiceList.append(data);
    }
    return voiceList;
}

void QTextToSpeechPrivateMac::setVoice(const QVoice &voice)
{
    NSString *identifier = voice.data().toString().toNSString();
    [speechSynthesizer setVoice:identifier];
    QLocale newLocale = localeForVoice(identifier);
    emitLocaleChanged(newLocale);
    emitVoiceChanged(voice);
}

QVoice QTextToSpeechPrivateMac::voice() const
{
    NSString *voice = [speechSynthesizer voice];
    return voiceForNSVoice(voice);
}

QT_END_NAMESPACE
