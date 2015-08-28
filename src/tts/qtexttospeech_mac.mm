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
    void updateVoices();
    QVoice voiceForNSVoice(NSString *voiceString) const;
    NSSpeechSynthesizer *speechSynthesizer;
    StateDelegate *stateDelegate;
    QVector<QLocale> m_locales;
    QMultiMap<QString, QVoice> m_voices;
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
    updateVoices();
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

    if (m_state != QTextToSpeech::Speaking) {
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
    QString voiceName = QString::fromNSString(attrs[NSVoiceName]);
    voice.setName(voiceName);
    NSString *gender = attrs[NSVoiceGender];
    voice.setGender([gender isEqualToString:NSVoiceGenderMale] ? QVoice::Male :
                    [gender isEqualToString:NSVoiceGenderFemale] ? QVoice::Female :
                    QVoice::Unknown);
    NSNumber *age = attrs[NSVoiceAge];
    int ageInt = age.intValue;
    voice.setAge(ageInt < 13 ? QVoice::Child :
                 ageInt < 20 ? QVoice::Teenager :
                 ageInt < 45 ? QVoice::Adult :
                 ageInt < 90 ? QVoice::Senior : QVoice::Other);
    voice.setData(QVariant(QString::fromNSString(attrs[NSVoiceIdentifier])));
    return voice;
}

QVector<QLocale> QTextToSpeechPrivateMac::availableLocales() const
{
    return m_locales;
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

void QTextToSpeechPrivateMac::updateVoices()
{
    NSArray *voices = [NSSpeechSynthesizer availableVoices];
    for (NSString *voice in voices) {
        QLocale locale = localeForVoice(voice);
        QVoice data = voiceForNSVoice(voice);
        if (!m_locales.contains(locale))
            m_locales.append(locale);
        m_voices.insert(locale.name(), data);
    }
}

QVector<QVoice> QTextToSpeechPrivateMac::availableVoices() const
{
    return m_voices.values(locale().name()).toVector();
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
