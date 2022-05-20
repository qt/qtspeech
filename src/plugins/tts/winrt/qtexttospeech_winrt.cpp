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

#include "qtexttospeech_winrt.h"
#include "qtexttospeech_winrt_audiosource.h"

#include <QtMultimedia/QAudioSink>
#include <QtMultimedia/QMediaDevices>
#include <QtMultimedia/QAudioDevice>

#include <QtCore/private/qfunctions_winrt_p.h>

#include <winrt/base.h>
#include <windows.foundation.h>
#include <windows.foundation.collections.h>
#include <windows.media.speechsynthesis.h>
#include <windows.storage.streams.h>

#include <wrl.h>

using namespace Qt::StringLiterals;

using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Media::SpeechSynthesis;
using namespace ABI::Windows::Storage::Streams;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

QT_BEGIN_NAMESPACE

class QTextToSpeechEngineWinRTPrivate
{
    Q_DECLARE_PUBLIC(QTextToSpeechEngineWinRT);
public:
    QTextToSpeechEngineWinRTPrivate(QTextToSpeechEngineWinRT *q);
    ~QTextToSpeechEngineWinRTPrivate();

    QTextToSpeech::State state = QTextToSpeech::Error;
    QTextToSpeech::ErrorReason m_errorReason = QTextToSpeech::ErrorReason::Initialization;
    QString m_errorString;

    // interfaces used to access the speech synthesizer
    ComPtr<ISpeechSynthesizer> synth;
    ComPtr<ISpeechSynthesizerOptions2> options;

    // data streaming - AudioSource implements COM interfaces as well as
    // QIODevice, so we store it in a ComPtr instead of a std::unique_ptr
    ComPtr<AudioSource> audioSource;
    // the sink is connected to the source
    std::unique_ptr<QAudioSink> audioSink;

    template <typename Fn> void forEachVoice(Fn &&lambda) const;
    void updateVoices();
    QVoice createVoiceForInformation(const ComPtr<IVoiceInformation> &info) const;
    void initializeAudioSink(const QAudioFormat &format);
    void sinkStateChanged(QAudio::State sinkState);

private:
    QAudioDevice audioDevice;
    QTextToSpeechEngineWinRT *q_ptr;
};


QTextToSpeechEngineWinRTPrivate::QTextToSpeechEngineWinRTPrivate(QTextToSpeechEngineWinRT *q)
    : q_ptr(q)
{
}

QTextToSpeechEngineWinRTPrivate::~QTextToSpeechEngineWinRTPrivate()
{
    // Close and free the source explicitly and in the right order so that the buffer's
    // aboutToClose signal gets emitted before this private object is destroyed.
    if (audioSource) {
        audioSource->close();
        audioSource.Reset();
    }
}

QTextToSpeechEngineWinRT::QTextToSpeechEngineWinRT(const QVariantMap &params, QObject *parent)
    : QTextToSpeechEngine(parent)
    , d_ptr(new QTextToSpeechEngineWinRTPrivate(this))
{
    Q_D(QTextToSpeechEngineWinRT);

    if (const auto it = params.find("audioDevice"_L1); it != params.end())
        d->audioDevice = (*it).value<QAudioDevice>();
    else
        d->audioDevice = QMediaDevices::defaultAudioOutput();

    if (d->audioDevice.isNull()) {
        d->m_errorString = tr("No audio device available");
        d->m_errorReason = QTextToSpeech::ErrorReason::Playback;
    }

    HRESULT hr = CoInitialize(nullptr);
    Q_ASSERT_SUCCEEDED(hr);

    hr = RoActivateInstance(HString::MakeReference(RuntimeClass_Windows_Media_SpeechSynthesis_SpeechSynthesizer).Get(),
                            &d->synth);
    if (!SUCCEEDED(hr)) {
        d->m_errorReason = QTextToSpeech::ErrorReason::Initialization;
        d->m_errorString = tr("Could not instantiate speech synthesizer.");
    } else if (voice() == QVoice()) {
        d->m_errorReason = QTextToSpeech::ErrorReason::Configuration;
        d->m_errorString = tr("Could not set default voice.");
    } else {
        d->state = QTextToSpeech::Ready;
        d->m_errorReason = QTextToSpeech::ErrorReason::NoError;
    }

    // the rest is optional, we might not support these features

    ComPtr<ISpeechSynthesizer2> synth2;
    hr = d->synth->QueryInterface(__uuidof(ISpeechSynthesizer2), &synth2);
    RETURN_VOID_IF_FAILED("ISpeechSynthesizer2 not implemented.");

    ComPtr<ISpeechSynthesizerOptions> options1;
    hr = synth2->get_Options(&options1);
    Q_ASSERT_SUCCEEDED(hr);

    hr = options1->QueryInterface(__uuidof(ISpeechSynthesizerOptions2), &d->options);
    RETURN_VOID_IF_FAILED("ISpeechSynthesizerOptions2 not implemented.");

    d->options->put_AudioPitch(1.0);
    d->options->put_AudioVolume(1.0);
    d->options->put_SpeakingRate(1.0);
}

QTextToSpeechEngineWinRT::~QTextToSpeechEngineWinRT()
{
    d_ptr.reset();
    CoUninitialize();
}

/* Voice and language/locale management */

QVoice QTextToSpeechEngineWinRTPrivate::createVoiceForInformation(const ComPtr<IVoiceInformation> &info) const
{
    Q_Q(const QTextToSpeechEngineWinRT);
    HRESULT hr;

    HString nativeName;
    hr = info->get_DisplayName(nativeName.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);

    VoiceGender gender;
    hr = info->get_Gender(&gender);
    Q_ASSERT_SUCCEEDED(hr);

    HString voiceLanguage;
    hr = info->get_Language(voiceLanguage.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);

    HString voiceId;
    hr = info->get_Id(voiceId.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);

    return q->createVoice(QString::fromWCharArray(nativeName.GetRawBuffer(0)),
                          QLocale(QString::fromWCharArray(voiceLanguage.GetRawBuffer(0))),
                          gender == VoiceGender_Male ? QVoice::Male : QVoice::Female,
                          QVoice::Other,
                          QString::fromWCharArray(voiceId.GetRawBuffer(0)));
}

/*
    Iterates over all available voices and calls \a lambda with the
    IVoiceInformation for each. If the lambda returns true, the iteration
    ends.

    This helper is used in all voice and locale related engine implementations.
    While not particular fast, none of these functions are performance critical,
    and always operating on the official list of all voices avoids that we need
    to maintain our own mappings, or keep all voice information instances in
    memory.
*/
template <typename Fn>
void QTextToSpeechEngineWinRTPrivate::forEachVoice(Fn &&lambda) const
{
    HRESULT hr;

    ComPtr<IInstalledVoicesStatic> stat;
    hr = RoGetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Media_SpeechSynthesis_SpeechSynthesizer).Get(),
                                IID_PPV_ARGS(&stat));
    Q_ASSERT_SUCCEEDED(hr);

    ComPtr<IVectorView<VoiceInformation*>> voiceInformations;
    hr = stat->get_AllVoices(&voiceInformations);
    RETURN_VOID_IF_FAILED("Could not get voice information.");

    quint32 voiceSize;
    hr = voiceInformations->get_Size(&voiceSize);
    RETURN_VOID_IF_FAILED("Could not access size of voice information.");

    for (quint32 i = 0; i < voiceSize; ++i) {
        ComPtr<IVoiceInformation> voiceInfo;
        hr = voiceInformations->GetAt(i, &voiceInfo);
        Q_ASSERT_SUCCEEDED(hr);

        if (lambda(voiceInfo))
            break;
    }
}

QList<QLocale> QTextToSpeechEngineWinRT::availableLocales() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    QSet<QLocale> uniqueLocales;
    d->forEachVoice([&uniqueLocales](const ComPtr<IVoiceInformation> &voiceInfo) {
        HString voiceLanguage;
        HRESULT hr = voiceInfo->get_Language(voiceLanguage.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);

        uniqueLocales.insert(QLocale(QString::fromWCharArray(voiceLanguage.GetRawBuffer(0))));
        return false;
    });
    return uniqueLocales.values();
}

QList<QVoice> QTextToSpeechEngineWinRT::availableVoices() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    QList<QVoice> voices;
    const QLocale currentLocale = locale();
    d->forEachVoice([&](const ComPtr<IVoiceInformation> &voiceInfo) {
        const QVoice voice = d->createVoiceForInformation(voiceInfo);
        if (currentLocale == voice.locale())
            voices.append(voice);
        return false;
    });
    return voices;
}

QLocale QTextToSpeechEngineWinRT::locale() const
{
    Q_D(const QTextToSpeechEngineWinRT);

    ComPtr<IVoiceInformation> voiceInfo;
    HRESULT hr = d->synth->get_Voice(&voiceInfo);

    HString language;
    hr = voiceInfo->get_Language(language.GetAddressOf());

    return QLocale(QString::fromWCharArray(language.GetRawBuffer(0)));
}

bool QTextToSpeechEngineWinRT::setLocale(const QLocale &locale)
{
    Q_D(QTextToSpeechEngineWinRT);

    ComPtr<IVoiceInformation> foundVoice;
    d->forEachVoice([&locale, &foundVoice](const ComPtr<IVoiceInformation> &voiceInfo) {
        HString voiceLanguage;
        HRESULT hr = voiceInfo->get_Language(voiceLanguage.GetAddressOf());
        Q_ASSERT_SUCCEEDED(hr);

        if (locale == QLocale(QString::fromWCharArray(voiceLanguage.GetRawBuffer(0)))) {
            foundVoice = voiceInfo;
            return true;
        }
        return false;
    });

    if (!foundVoice) {
        qWarning() << "No voice found for locale" << locale;
        return false;
    }

    return SUCCEEDED(d->synth->put_Voice(foundVoice.Get()));
}

QVoice QTextToSpeechEngineWinRT::voice() const
{
    Q_D(const QTextToSpeechEngineWinRT);

    ComPtr<IVoiceInformation> voiceInfo;
    d->synth->get_Voice(&voiceInfo);

    return d->createVoiceForInformation(voiceInfo);
}

bool QTextToSpeechEngineWinRT::setVoice(const QVoice &voice)
{
    Q_D(QTextToSpeechEngineWinRT);

    const QString data = QTextToSpeechEngine::voiceData(voice).toString();
    if (data.isEmpty())
        return false;

    ComPtr<IVoiceInformation> foundVoice;
    d->forEachVoice([&data, &foundVoice](const ComPtr<IVoiceInformation> &voiceInfo) {
        HString voiceId;
        HRESULT hr = voiceInfo->get_Id(voiceId.GetAddressOf());
        if (data == QString::fromWCharArray(voiceId.GetRawBuffer(0))) {
            foundVoice = voiceInfo;
            return true;
        }
        return false;
    });

    if (!foundVoice) {
        qWarning() << "No voice found for " << voice;
        return false;
    }

    return SUCCEEDED(d->synth->put_Voice(foundVoice.Get()));
}

/* State and speech control */

QTextToSpeech::State QTextToSpeechEngineWinRT::state() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    return d->state;
}

QTextToSpeech::ErrorReason QTextToSpeechEngineWinRT::errorReason() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    return d->m_errorReason;
}

QString QTextToSpeechEngineWinRT::errorString() const
{
    Q_D(const QTextToSpeechEngineWinRT);
    return d->m_errorString;
}

void QTextToSpeechEngineWinRTPrivate::initializeAudioSink(const QAudioFormat &format)
{
    Q_Q(const QTextToSpeechEngineWinRT);

    // cancelled or another call to say() while waiting for the synthesizer
    if (!audioSource)
        return;

    audioSink.reset(new QAudioSink(audioDevice, format));
    QObject::connect(audioSink.get(), &QAudioSink::stateChanged,
                     q, [this](QAudio::State sinkState) {
        sinkStateChanged(sinkState);
    });
    // ### this shouldn't be needed, but readyRead doesn't wake up
    // an idle sink.
    QObject::connect(audioSource.Get(), &QIODevice::readyRead,
                     audioSink.get(), &QAudioSink::resume);
    audioSink->start(audioSource.Get());
}

void QTextToSpeechEngineWinRTPrivate::sinkStateChanged(QAudio::State sinkState)
{
    Q_Q(QTextToSpeechEngineWinRT);

    const auto oldState = state;
    switch (sinkState) {
    case QAudio::IdleState:
        if (audioSource) {
            if (audioSource->atEnd()) {
                state = QTextToSpeech::Ready;
            } else {
                // ### an unsuspended sink doesn't wake up from readyRead,
                // and it only resumes (see above) if we suspend it first.
                audioSink->suspend();
            }
        }
        break;
    case QAudio::StoppedState:
        state = QTextToSpeech::Ready;
        break;
    case QAudio::ActiveState:
        state = QTextToSpeech::Speaking;
        break;
    case QAudio::SuspendedState:
        state = QTextToSpeech::Paused;
        break;
    }
    if (state != oldState)
        emit q->stateChanged(state);
}

void QTextToSpeechEngineWinRT::say(const QString &text)
{
    Q_D(QTextToSpeechEngineWinRT);

    // stop ongoing speech
    stop(QTextToSpeech::BoundaryHint::Default);

    HRESULT hr = S_OK;

    HStringReference nativeText(reinterpret_cast<LPCWSTR>(text.utf16()), text.length());

    ComPtr<IAsyncOperation<SpeechSynthesisStream*>> synthOperation;
    hr = d->synth->SynthesizeTextToStreamAsync(nativeText.Get(), &synthOperation);
    RETURN_VOID_IF_FAILED("Could not synthesize text.");

    if (!SUCCEEDED(hr)) {
        d->state = QTextToSpeech::Error;
        return;
    }

    // The source will wait for the the data resulting out of the synthOperation, and emits
    // streamReady when data is available. This starts a QAudioSink, which pulls the data.
    d->audioSource.Attach(new AudioSource(synthOperation));

    connect(d->audioSource.Get(), &AudioSource::streamReady, this, [d](const QAudioFormat &format){
        d->initializeAudioSink(format);
    });
    connect(d->audioSource.Get(), &AudioSource::errorInStream, this, [this]{
        Q_D(QTextToSpeechEngineWinRT);
        QTextToSpeech::State oldState = d->state;
        d->state = QTextToSpeech::Error;
        if (oldState != d->state)
            emit stateChanged(d->state);
    });
    connect(d->audioSource.Get(), &QIODevice::aboutToClose, this, [d]{
        d->audioSink.reset();
    });
}

void QTextToSpeechEngineWinRT::stop(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    Q_D(QTextToSpeechEngineWinRT);

    if (d->audioSource) {
        d->audioSource->close();
        d->audioSource.Reset();
    }
}

void QTextToSpeechEngineWinRT::pause(QTextToSpeech::BoundaryHint boundaryHint)
{
    Q_UNUSED(boundaryHint);
    Q_D(QTextToSpeechEngineWinRT);

    if (d->audioSource)
        d->audioSource->pause();
}

void QTextToSpeechEngineWinRT::resume()
{
    Q_D(QTextToSpeechEngineWinRT);

    if (d->audioSource)
        d->audioSource->resume();
}

/* Properties */

/*
    The native value can range from 0.5 (half the default rate) to 6.0 (6x the default rate), inclusive.
    The default value is 1.0 (the "normal" speaking rate for the current voice).

    QTextToSpeech rate is from -1.0 to 1.0.
*/
double QTextToSpeechEngineWinRT::rate() const
{
    Q_D(const QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return 0.0;
    }

    double value;
    d->options->get_SpeakingRate(&value);
    if (value < 1.0) // 0.5 to 1.0 maps to -1.0 to 0.0
        value = (value - 1.0) * 2.0;
    else // 1.0 to 6.0 maps to 0.0 to 1.0
        value = (value - 1.0) / 5.0;
    return value;
}

bool QTextToSpeechEngineWinRT::setRate(double rate)
{
    Q_D(QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return false;
    }

    if (rate < 0.0) // -1.0 to 0.0 maps to 0.5 to 1.0
        rate = 0.5 * rate + 1.0;
    else // 0.0 to 1.0 maps to 1.0 to 6.0
        rate = rate * 5.0 + 1.0;
    HRESULT hr = d->options->put_SpeakingRate(rate);
    return SUCCEEDED(hr);
}

/*
    The native value can range from 0.0 (lowest pitch) to 2.0 (highest pitch), inclusive.
    The default value is 1.0.

    The QTextToSpeech value is from -1.0 to 1.0.
*/
double QTextToSpeechEngineWinRT::pitch() const
{
    Q_D(const QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return 0.0;
    }

    double value;
    d->options->get_AudioPitch(&value);
    return value - 1.0;
}

bool QTextToSpeechEngineWinRT::setPitch(double pitch)
{
    Q_D(QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return false;
    }

    HRESULT hr = d->options->put_AudioPitch(pitch + 1.0);
    return SUCCEEDED(hr);
}

/*
    The native value can range from 0.0 (lowest volume) to 1.0 (highest volume), inclusive.
    The default value is 1.0.

    This maps directly to the QTextToSpeech volume range.
*/
double QTextToSpeechEngineWinRT::volume() const
{
    Q_D(const QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return 0.0;
    }

    double value;
    d->options->get_AudioVolume(&value);
    return value;
}

bool QTextToSpeechEngineWinRT::setVolume(double volume)
{
    Q_D(QTextToSpeechEngineWinRT);

    if (!d->options) {
        Q_UNIMPLEMENTED();
        return false;
    }

    HRESULT hr = d->options->put_AudioVolume(volume);
    return SUCCEEDED(hr);
}

QT_END_NAMESPACE
