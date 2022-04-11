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

#include <Cocoa/Cocoa.h>
#include "qtexttospeech_macos.h"
#include <qdebug.h>

@interface QT_MANGLE_NAMESPACE(StateDelegate) : NSObject <NSSpeechSynthesizerDelegate>
@end

@implementation QT_MANGLE_NAMESPACE(StateDelegate)
{
    QT_PREPEND_NAMESPACE(QTextToSpeechEngineMacOS) *speechPrivate;
}

- (instancetype)initWithSpeechPrivate:(QTextToSpeechEngineMacOS *) priv
{
    if ((self = [super init]))
        speechPrivate = priv;

    return self;
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender willSpeakWord:(NSRange)characterRange ofString:(NSString *)string
{
    Q_UNUSED(sender);
    speechPrivate->speaking();
}

- (void)speechSynthesizer:(NSSpeechSynthesizer *)sender didFinishSpeaking:(BOOL)finishedSpeaking
{
    Q_UNUSED(sender);
    speechPrivate->speechStopped(finishedSpeaking == YES);
}
@end

QT_BEGIN_NAMESPACE

QTextToSpeechEngineMacOS::QTextToSpeechEngineMacOS(const QVariantMap &/*parameters*/, QObject *parent)
    : QTextToSpeechEngine(parent), m_state(QTextToSpeech::Ready)
{
    stateDelegate = [[QT_MANGLE_NAMESPACE(StateDelegate) alloc] initWithSpeechPrivate:this];
    speechSynthesizer = [[NSSpeechSynthesizer alloc] init];
    speechSynthesizer.delegate = stateDelegate;
    updateVoices();
}

QTextToSpeechEngineMacOS::~QTextToSpeechEngineMacOS()
{
    [speechSynthesizer setDelegate: nil];
    if ([speechSynthesizer isSpeaking])
        [speechSynthesizer stopSpeakingAtBoundary:NSSpeechImmediateBoundary];
    [speechSynthesizer release];
    [stateDelegate release];
}


QTextToSpeech::State QTextToSpeechEngineMacOS::state() const
{
    return m_state;
}


void QTextToSpeechEngineMacOS::speaking()
{
    if (m_state != QTextToSpeech::Speaking) {
        m_state = QTextToSpeech::Speaking;
        emit stateChanged(m_state);
    }
}

void QTextToSpeechEngineMacOS::speechStopped(bool success)
{
    Q_UNUSED(success);
    if (m_state != QTextToSpeech::Ready) {
        if (pauseRequested)
            m_state = QTextToSpeech::Paused;
        else
            m_state = QTextToSpeech::Ready;
        emit stateChanged(m_state);
    }
    pauseRequested = false;
}

void QTextToSpeechEngineMacOS::say(const QString &text)
{
    if (text.isEmpty())
        return;

    pauseRequested = false;
    if (m_state != QTextToSpeech::Ready)
        stop();

    NSString *ntext = text.toNSString();
    [speechSynthesizer startSpeakingString:ntext];
    speaking();
}

void QTextToSpeechEngineMacOS::stop()
{
    if (speechSynthesizer.isSpeaking || m_state == QTextToSpeech::Paused)
        [speechSynthesizer stopSpeakingAtBoundary:NSSpeechImmediateBoundary];
}

void QTextToSpeechEngineMacOS::pause()
{
    if (speechSynthesizer.isSpeaking) {
        pauseRequested = true;
        [speechSynthesizer pauseSpeakingAtBoundary: NSSpeechWordBoundary];
    }
}

void QTextToSpeechEngineMacOS::resume()
{
    pauseRequested = false;
    [speechSynthesizer continueSpeaking];
}

double QTextToSpeechEngineMacOS::rate() const
{
    return (speechSynthesizer.rate - 200) / 200.0;
}

bool QTextToSpeechEngineMacOS::setPitch(double pitch)
{
    // 30 to 65
    double p = 30.0 + ((pitch + 1.0) / 2.0) * 35.0;
    [speechSynthesizer setObject:[NSNumber numberWithFloat:p] forProperty:NSSpeechPitchBaseProperty error:nil];
    return true;
}

double QTextToSpeechEngineMacOS::pitch() const
{
    double pitch = [[speechSynthesizer objectForProperty:NSSpeechPitchBaseProperty error:nil] floatValue];
    return (pitch - 30.0) / 35.0 * 2.0 - 1.0;
}

double QTextToSpeechEngineMacOS::volume() const
{
    return speechSynthesizer.volume;
}

bool QTextToSpeechEngineMacOS::setRate(double rate)
{
    // NSSpeechSynthesizer supports words per minute,
    // human speech is 180 to 220 - use 0 to 400 as range here
    speechSynthesizer.rate = 200 + (rate * 200);
    return true;
}

bool QTextToSpeechEngineMacOS::setVolume(double volume)
{
    speechSynthesizer.volume = volume;
    return true;
}

static QLocale localeForVoice(NSString *voice)
{
    NSDictionary *attrs = [NSSpeechSynthesizer attributesForVoice:voice];
    return QLocale(QString::fromNSString(attrs[NSVoiceLocaleIdentifier]));
}

QVoice QTextToSpeechEngineMacOS::voiceForNSVoice(NSString *voiceString) const
{
    NSDictionary *attrs = [NSSpeechSynthesizer attributesForVoice:voiceString];
    QString voiceName = QString::fromNSString(attrs[NSVoiceName]);

    NSString *genderString = attrs[NSVoiceGender];
    QVoice::Gender gender = [genderString isEqualToString:NSVoiceGenderMale] ? QVoice::Male
                          : [genderString isEqualToString:NSVoiceGenderFemale] ? QVoice::Female
                          : QVoice::Unknown;

    NSNumber *ageNSNumber = attrs[NSVoiceAge];
    int ageInt = ageNSNumber.intValue;
    QVoice::Age age = (ageInt < 13 ? QVoice::Child :
                       ageInt < 20 ? QVoice::Teenager :
                       ageInt < 45 ? QVoice::Adult :
                       ageInt < 90 ? QVoice::Senior : QVoice::Other);
    QVariant data = QString::fromNSString(attrs[NSVoiceIdentifier]);
    return createVoice(voiceName, localeForVoice(voiceString), gender, age, data);
}

QList<QLocale> QTextToSpeechEngineMacOS::availableLocales() const
{
    return m_voices.uniqueKeys();
}

bool QTextToSpeechEngineMacOS::setLocale(const QLocale &locale)
{
    NSArray *voices = NSSpeechSynthesizer.availableVoices;
    NSString *voice = NSSpeechSynthesizer.defaultVoice;
    // always prefer default
    if (locale == localeForVoice(voice)) {
        speechSynthesizer.voice = voice;
        return true;
    }

    for (voice in voices) {
        if (locale == localeForVoice(voice)) {
            speechSynthesizer.voice = voice;
            return true;
        }
    }
    return false;
}

QLocale QTextToSpeechEngineMacOS::locale() const
{
    NSString *voice = speechSynthesizer.voice;
    return localeForVoice(voice);
}

void QTextToSpeechEngineMacOS::updateVoices()
{
    NSArray *voices = NSSpeechSynthesizer.availableVoices;
    for (NSString *voice in voices) {
        const QVoice data = voiceForNSVoice(voice);
        m_voices.insert(data.locale(), data);
    }
}

QList<QVoice> QTextToSpeechEngineMacOS::availableVoices() const
{
    return m_voices.values(locale());
}

bool QTextToSpeechEngineMacOS::setVoice(const QVoice &voice)
{
    NSString *identifier = voiceData(voice).toString().toNSString();
    speechSynthesizer.voice = identifier;
    return true;
}

QVoice QTextToSpeechEngineMacOS::voice() const
{
    return voiceForNSVoice(speechSynthesizer.voice);
}

QT_END_NAMESPACE
