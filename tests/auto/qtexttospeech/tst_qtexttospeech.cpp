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
#include <qttexttospeech-config.h>

#if QT_CONFIG(speechd)
    #include <libspeechd.h>
    #if LIBSPEECHD_MAJOR_VERSION == 0 && LIBSPEECHD_MINOR_VERSION < 9
        #define HAVE_SPEECHD_BEFORE_090
    #endif
#endif

enum : int { SpeechDuration = 20000 };

class tst_QTextToSpeech : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase_data();
    void init();

    void availableVoices();
    void availableLocales();

    void rate();
    void pitch();
    void volume();

    void sayHello();
    void pauseResume();
    void sayWithVoices();
    void sayWithRates();
};

void tst_QTextToSpeech::initTestCase_data()
{
    qInfo("Available text-to-speech engines:");
    QTest::addColumn<QString>("engine");
    const auto engines = QTextToSpeech::availableEngines();
    if (!engines.count())
        QSKIP("No speech engines available, skipping test case");
    for (const auto &engine : engines) {
        QTest::addRow("%s", engine.toUtf8().constData()) << engine;

        QString engineName = engine;
#if QT_CONFIG(speechd) && defined(LIBSPEECHD_MAJOR_VERSION) && defined(LIBSPEECHD_MINOR_VERSION)
        if (engineName == "speechd") {
            engineName += QString(" (using libspeechd %1.%2)").arg(LIBSPEECHD_MAJOR_VERSION)
                                                              .arg(LIBSPEECHD_MINOR_VERSION);
        }
#endif
        qInfo().noquote() << "- " << engineName;
    }
}

void tst_QTextToSpeech::init()
{
    QFETCH_GLOBAL(QString, engine);
    if (engine == "speechd") {
        QTextToSpeech tts(engine);
        if (tts.state() == QTextToSpeech::BackendError) {
            QSKIP("speechd engine reported a backend error, "
                  "make sure the speech-dispatcher service is running!");
        }
    }
}

void tst_QTextToSpeech::availableVoices()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
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
    const auto availableLocales = tts.availableLocales();
    QVERIFY(availableLocales.count() > 0);
    qInfo("Available locales:");
    for (const auto &locale : availableLocales)
        qInfo().noquote() << "-" << locale;
}

void tst_QTextToSpeech::rate()
{
    QFETCH_GLOBAL(QString, engine);
    const QString text = QStringLiteral("example text at different rates");
    QTextToSpeech tts(engine);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);

    tts.setRate(0.5);
#ifndef HAVE_SPEECHD_BEFORE_090
    QCOMPARE(tts.rate(), 0.5);
#endif
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
    tts.setPitch(0.0);
    QCOMPARE(tts.pitch(), 0.0);

    QSignalSpy spy(&tts, &QTextToSpeech::pitchChanged);
    int signalCount = 0;
    for (int i = -10; i <= 10; ++i) {
        tts.setPitch(i / 10.0);
#ifndef HAVE_SPEECHD_BEFORE_090
        QCOMPARE(tts.pitch(), i / 10.0);
#endif
        QCOMPARE(spy.count(), ++signalCount);
        tts.setPitch(tts.pitch());
        QCOMPARE(spy.count(), signalCount);
    }
}

void tst_QTextToSpeech::volume()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    tts.setVolume(0.0);

    QSignalSpy spy(&tts, &QTextToSpeech::volumeChanged);
    tts.setVolume(0.7);
    QTRY_COMPARE(spy.count(), 1);
    QVERIFY(spy.value(0).first().toDouble() > 0.6);

#ifndef HAVE_SPEECHD_BEFORE_090  // older speechd doesn't signal any volume changes
    // engines use different systems (integers etc), even fuzzy compare is off
    QVERIFY2(tts.volume() > 0.65, QByteArray::number(tts.volume()));
    QVERIFY2(tts.volume() < 0.75, QByteArray::number(tts.volume()));
#endif

    tts.setVolume(tts.volume());
    QCOMPARE(spy.count(), 1);
}


void tst_QTextToSpeech::sayHello()
{
    QFETCH_GLOBAL(QString, engine);
    const QString text = QStringLiteral("saying hello with %1");
    QTextToSpeech tts(engine);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);

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
    const QString text = QStringLiteral("Hello. World.");
    QTextToSpeech tts(engine);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);

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
    const QString text = QStringLiteral("engine %1 with voice of %2");
    QTextToSpeech tts(engine);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);

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
    const QString text = QStringLiteral("test at different rates");
    QTextToSpeech tts(engine);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);

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
        QVERIFY(time > lastTime);
        lastTime = time;
    }
}

QTEST_MAIN(tst_QTextToSpeech)
#include "tst_qtexttospeech.moc"
