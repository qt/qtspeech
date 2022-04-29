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


#include <QTest>
#include <QTextToSpeech>
#include <QSignalSpy>
#include <QMediaDevices>
#include <QAudioDevice>
#include <QOperatingSystemVersion>
#include <qttexttospeech-config.h>

#if QT_CONFIG(speechd)
    #include <libspeechd.h>
    #if LIBSPEECHD_MAJOR_VERSION == 0 && LIBSPEECHD_MINOR_VERSION < 9
        #define HAVE_SPEECHD_BEFORE_090
    #endif
#endif

using namespace Qt::StringLiterals;

enum : int { SpeechDuration = 20000 };

class tst_QTextToSpeech : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase_data();
    void init();

    void availableVoices();
    void availableLocales();

    void locale();
    void voice();

    void rate();
    void pitch();
    void volume();

    void sayHello();
    void pauseResume();
    void sayWithVoices();
    void sayWithRates();

private:
    static bool hasDefaultAudioOutput()
    {
        return !QMediaDevices::defaultAudioOutput().isNull();
    }
};

void tst_QTextToSpeech::initTestCase_data()
{
    qInfo("Available text-to-speech engines:");
    QTest::addColumn<QString>("engine");
    const auto engines = QTextToSpeech::availableEngines();
    if (!engines.count())
        QSKIP("No speech engines available, skipping test case");
    for (const auto &engine : engines) {

        QString engineName = engine;
#if QT_CONFIG(speechd) && defined(LIBSPEECHD_MAJOR_VERSION) && defined(LIBSPEECHD_MINOR_VERSION)
        if (engine == "speechd") {
            engineName += QString(" (using libspeechd %1.%2)").arg(LIBSPEECHD_MAJOR_VERSION)
                                                              .arg(LIBSPEECHD_MINOR_VERSION);
        }
#ifdef HAVE_SPEECHD_BEFORE_090
        engineName += " - libspeechd too old, skipping";
#endif
#endif
        qInfo().noquote() << "- " << engineName;

#ifdef HAVE_SPEECHD_BEFORE_090
        if (engine == "speechd")
            continue;
#endif
        QTest::addRow("%s", engine.toUtf8().constData()) << engine;
    }
}

void tst_QTextToSpeech::init()
{
    QFETCH_GLOBAL(QString, engine);
    if (engine == "speechd") {
        QTextToSpeech tts(engine);
        QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
        if (tts.state() == QTextToSpeech::BackendError) {
            QSKIP("speechd engine reported a backend error, "
                  "make sure the speech-dispatcher service is running!");
        }
    } else if (engine == "ios"
        && QOperatingSystemVersion::current() <= QOperatingSystemVersion::MacOSMojave) {
        QTextToSpeech tts(engine);
        if (!tts.availableLocales().count())
            QSKIP("iOS engine is not functional on macOS <= 10.14");
    }
}

void tst_QTextToSpeech::availableVoices()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
    qInfo("Available voices:");
    const auto availableVoices = tts.availableVoices();
    QVERIFY(availableVoices.count() > 0);
    for (const auto &voice : availableVoices)
        qInfo().noquote() << "-" << voice;
}

void tst_QTextToSpeech::availableLocales()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
    const auto availableLocales = tts.availableLocales();
    QVERIFY(availableLocales.count() > 0);
    qInfo("Available locales:");
    for (const auto &locale : availableLocales)
        qInfo().noquote() << "-" << locale;
}

/*
    Testing the locale property, and its dependency on the voice
    property.
*/
void tst_QTextToSpeech::locale()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_VERIFY(tts.state() == QTextToSpeech::Ready);

    const auto availableLocales = tts.availableLocales();
    // every engine must have a working default locale if it's Ready
    QVERIFY(availableLocales.contains(tts.locale()));
    if (availableLocales.count() < 2)
        QSKIP("Engine doesn't support more than one locale");

    tts.setLocale(availableLocales[0]);
    // changing the locale results in a voice that is supported by that locale
    const auto voices0 = tts.availableVoices();
    const QVoice voice0 = tts.voice();
    QVERIFY(voices0.contains(voice0));

    QSignalSpy localeSpy(&tts, &QTextToSpeech::localeChanged);
    QSignalSpy voiceSpy(&tts, &QTextToSpeech::voiceChanged);

    // repeat, watch signal emissions
    tts.setLocale(availableLocales[1]);
    QCOMPARE(localeSpy.count(), 1);

    // a locale is only available if it has voices
    const auto voices1 = tts.availableVoices();
    QVERIFY(voices1.count());
    // If the voice is supported in the new locale, then it shouldn't change,
    // otherwise the voice should change as well.
    if (voices1.contains(voice0))
        QCOMPARE(voiceSpy.count(), 0);
    else
        QCOMPARE(voiceSpy.count(), 1);
}

/*
    Testing the voice property, and its dependency on the locale
    property.

    We cannot test all things on engines that have only a single voice, or no
    locale that supports multiple voices.
*/
void tst_QTextToSpeech::voice()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_VERIFY(tts.state() == QTextToSpeech::Ready);

    const QVoice defaultVoice = tts.voice();
    const QLocale defaultLocale = tts.locale();
    // every engine must have a working default voice if it's Ready
    QVERIFY(defaultVoice != QVoice());

    // find a locale with more than one voice, and a voice from another locale
    QList<QVoice> availableVoices;
    QLocale voicesLocale;
    QVoice otherVoice;
    QLocale otherLocale;
    for (const auto &locale : tts.availableLocales()) {
        tts.setLocale(locale);
        const auto voices = tts.availableVoices();
        if (voices.count() > 1 && availableVoices.isEmpty()) {
            availableVoices = voices;
            voicesLocale = locale;
        }
        if (voices != availableVoices && voices.first() != defaultVoice) {
            otherVoice = voices.first();
            otherLocale = locale;
        }
        // we found everything
        if (availableVoices.count() > 1 && otherVoice != QVoice())
            break;
    }
    // if we found neither, then we cannot test
    if (availableVoices.count() < 2 && otherVoice == QVoice())
        QSKIP("Engine %s supports only a single voice", qPrintable(engine));

    tts.setVoice(defaultVoice);
    QCOMPARE(tts.locale(), defaultLocale);

    int expectedVoiceChanged = 0;
    int expectedLocaleChanged = 0;
    QSignalSpy voiceSpy(&tts, &QTextToSpeech::voiceChanged);
    QSignalSpy localeSpy(&tts, &QTextToSpeech::localeChanged);

    if (otherVoice != QVoice() && otherVoice != defaultVoice) {
        QVERIFY(otherLocale != voicesLocale);
        // at this point, setting to otherVoice only changes things when it's not the default
        ++expectedVoiceChanged;
        ++expectedLocaleChanged;
        tts.setVoice(otherVoice);
        QCOMPARE(voiceSpy.count(), expectedVoiceChanged);
        QCOMPARE(tts.locale(), otherLocale);
        QCOMPARE(localeSpy.count(), expectedLocaleChanged);
    } else {
        otherLocale = defaultLocale;
    }

    // two voices from the same locale
    if (availableVoices.count() > 1) {
        if (tts.voice() != availableVoices[0])
            ++expectedVoiceChanged;
        tts.setVoice(availableVoices[0]);
        QCOMPARE(voiceSpy.count(), expectedVoiceChanged);
        if (voicesLocale != otherLocale)
            ++expectedLocaleChanged;
        QCOMPARE(localeSpy.count(), expectedLocaleChanged);
        tts.setVoice(availableVoices[1]);
        QCOMPARE(voiceSpy.count(), ++expectedVoiceChanged);
        QCOMPARE(localeSpy.count(), expectedLocaleChanged);
    }
}

void tst_QTextToSpeech::rate()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);

    tts.setRate(0.5);
    QCOMPARE(tts.rate(), 0.5);
    QSignalSpy spy(&tts, &QTextToSpeech::rateChanged);
    tts.setRate(0.0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.value(0).first().toDouble(), 0.0);

    tts.setRate(tts.rate());
    QCOMPARE(spy.count(), 1);
}

void tst_QTextToSpeech::pitch()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
    tts.setPitch(0.0);
    QCOMPARE(tts.pitch(), 0.0);

    QSignalSpy spy(&tts, &QTextToSpeech::pitchChanged);
    int signalCount = 0;
    for (int i = -10; i <= 10; ++i) {
        tts.setPitch(i / 10.0);
        QString actual = QString("%1").arg(tts.pitch(), 0, 'g', 2);
        QString expected = QString("%1").arg(i / 10.0, 0, 'g', 2);
        QCOMPARE(actual, expected);
        QCOMPARE(spy.count(), ++signalCount);
        tts.setPitch(tts.pitch());
        QCOMPARE(spy.count(), signalCount);
    }
}

void tst_QTextToSpeech::volume()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
    tts.setVolume(0.0);

    QSignalSpy spy(&tts, &QTextToSpeech::volumeChanged);
    tts.setVolume(0.7);
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.value(0).first().toDouble() > 0.6);

    QVERIFY2(tts.volume() > 0.65, QByteArray::number(tts.volume()));
    QVERIFY2(tts.volume() < 0.75, QByteArray::number(tts.volume()));

    tts.setVolume(tts.volume());
    QCOMPARE(spy.count(), 1);
}


void tst_QTextToSpeech::sayHello()
{
    QFETCH_GLOBAL(QString, engine);
    if (engine != "mock" && !hasDefaultAudioOutput())
        QSKIP("No audio device present");

    const QString text = QStringLiteral("saying hello with %1");
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);

    QElapsedTimer timer;
    timer.start();
    tts.say(text.arg(engine));
    QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
    QSignalSpy spy(&tts, &QTextToSpeech::stateChanged);
    QVERIFY(spy.wait(SpeechDuration));
    QCOMPARE(tts.state(), QTextToSpeech::Ready);
    QVERIFY(timer.elapsed() > 100);
}

void tst_QTextToSpeech::pauseResume()
{
    QFETCH_GLOBAL(QString, engine);
    if (engine != "mock" && !hasDefaultAudioOutput())
        QSKIP("No audio device present");

    const QString text = QStringLiteral("Hello. World.");
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);

    tts.say(text);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
    // tts engines will either pause immediately, or in a suitable break,
    // which would be after "Hello."
    tts.pause();
    QTRY_COMPARE(tts.state(), QTextToSpeech::Paused);
    tts.resume();
    QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
}

void tst_QTextToSpeech::sayWithVoices()
{
    QFETCH_GLOBAL(QString, engine);
    if (engine != "mock" && !hasDefaultAudioOutput())
        QSKIP("No audio device present");

    const QString text = QStringLiteral("engine %1 with voice of %2");
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);

    const QList<QVoice> voices = tts.availableVoices();
    for (const auto &voice : voices) {
        tts.setVoice(voice);
        auto logger = qScopeGuard([&voice]{
            qWarning() << "Failure with voice" << voice;
        });
        QCOMPARE(tts.state(), QTextToSpeech::Ready);

        QElapsedTimer timer;
        timer.start();
        tts.say(text.arg(engine, voice.name()));
        QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
        QSignalSpy spy(&tts, &QTextToSpeech::stateChanged);
        QVERIFY(spy.wait(SpeechDuration));
        QCOMPARE(tts.state(), QTextToSpeech::Ready);
        QVERIFY(timer.elapsed() > 100);
        logger.dismiss();
    }
}

void tst_QTextToSpeech::sayWithRates()
{
    QFETCH_GLOBAL(QString, engine);
    if (engine != "mock" && !hasDefaultAudioOutput())
        QSKIP("No audio device present");

    const QString text = QStringLiteral("test at different rates");
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);

    // warmup at normal rate so that the result doesn't get skewed on engines
    // that initialize lazily on first utterance (like Android).
    tts.say(text);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);

    qint64 lastTime = 0;
    // check that speaking at slower rate takes more time (for 0.5, 0.0, -0.5)
    for (int i = 1; i >= -1; --i) {
        tts.setRate(i * 0.5);
        QElapsedTimer timer;
        timer.start();
        tts.say(text);
        QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
        QSignalSpy spy(&tts, &QTextToSpeech::stateChanged);
        QVERIFY(spy.wait(SpeechDuration));
        QCOMPARE(tts.state(), QTextToSpeech::Ready);
        qint64 time = timer.elapsed();
        QVERIFY2(time > lastTime, qPrintable(QString("%1 took %2, last was %3"_L1)
                                             .arg(i).arg(time).arg(lastTime)));
        lastTime = time;
    }
}

QTEST_MAIN(tst_QTextToSpeech)
#include "tst_qtexttospeech.moc"
