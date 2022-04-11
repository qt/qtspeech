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

class tst_QVoice : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase_data();
    void init();

    void basic();
    void sameEngine();
    void datastream();
};

void tst_QVoice::initTestCase_data()
{
    qInfo("Available text-to-speech engines:");
    QTest::addColumn<QString>("engine");
    const auto engines = QTextToSpeech::availableEngines();
    if (!engines.count())
        QSKIP("No speech engines available, skipping test case");
    for (const auto &engine : engines) {
        QTest::addRow("%s", engine.toUtf8().constData()) << engine;
        qInfo().noquote() << "- " << engine;
    }
}

void tst_QVoice::init()
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

/*
    Test basic value type semantics.
*/
void tst_QVoice::basic()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts(engine);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
    const QList<QVoice> voices = tts.availableVoices();
    QVERIFY(voices.count());

    QVoice emptyVoice;
    for (const auto &voice : voices) {
        QCOMPARE(voice, voice);
        QCOMPARE(voice.locale(), tts.locale());
        QVERIFY(voice != emptyVoice);

        QVoice voiceCopy = voice;
        QCOMPARE(voiceCopy, voice);
    }

    QVoice movedFrom = voices.first();
    QVoice movedTo = std::move(movedFrom);
    QCOMPARE(movedTo, voices.first());
    QCOMPARE(movedFrom, emptyVoice);
}

/*
    A QVoice from one engine should match the same voice from the same engine.
*/
void tst_QVoice::sameEngine()
{
    QFETCH_GLOBAL(QString, engine);
    QTextToSpeech tts1(engine);
    QTextToSpeech tts2(engine);
    QTRY_COMPARE(tts1.state(), QTextToSpeech::Ready);
    QTRY_COMPARE(tts2.state(), QTextToSpeech::Ready);

    const QList<QVoice> voices = tts1.availableVoices();
    QVERIFY(voices.count());
    QCOMPARE(tts1.availableVoices(), tts2.availableVoices());

    for (const auto &voice : voices)
        QVERIFY(tts2.availableVoices().indexOf(voice) != -1);
}

void tst_QVoice::datastream()
{
    QFETCH_GLOBAL(QString, engine);
    QVoice savedVoice;

    QByteArray storage;
    {
        QTextToSpeech tts(engine);
        QTRY_COMPARE(tts.state(), QTextToSpeech::Ready);
        const QList<QVoice> voices = tts.availableVoices();
        QVERIFY(voices.count());
        savedVoice = voices.first();

        QDataStream writeStream(&storage, QIODevice::WriteOnly);
        writeStream << savedVoice;
        QVERIFY(storage.size() > 0);
    }

    QVoice loadedVoice;
    QDataStream readStream(storage);
    readStream >> loadedVoice;

    QCOMPARE(loadedVoice, savedVoice);
}

QTEST_MAIN(tst_QVoice)
#include "tst_qvoice.moc"
