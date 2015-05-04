/****************************************************************************
**
** Copyright (C) 2014 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the test suite of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia. For licensing terms and
** conditions see http://qt.digia.com/licensing. For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights. These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QTest>
#include <QTextToSpeech>
#include <QSignalSpy>

class tst_QTextToSpeech : public QObject
{
    Q_OBJECT

private slots:
    void say_hello();
    void speech_rate();
    void pitch();
    void set_voice();
};


void tst_QTextToSpeech::say_hello()
{
    QString text = QStringLiteral("this is an example text");
    QTextToSpeech tts;
    QCOMPARE(tts.state(), QTextToSpeech::Ready);

    QElapsedTimer timer;
    timer.start();
    tts.say(text);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
    QSignalSpy spy(&tts, SIGNAL(stateChanged(QTextToSpeech::State)));
    spy.wait(10000);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);
    QVERIFY(timer.elapsed() > 100);
}

void tst_QTextToSpeech::speech_rate()
{
    QString text = QStringLiteral("this is an example text");
    QTextToSpeech tts;
    tts.setRate(0.5);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);
    QCOMPARE(tts.rate(), 0.5);

    qint64 lastTime = 0;
    // check that speaking at slower rate takes more time (for 0.5, 0.0, -0.5)
    for (int i = 1; i >= -1; --i) {
        tts.setRate(i * 0.5);
        QElapsedTimer timer;
        timer.start();
        tts.say(text);
        QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
        QSignalSpy spy(&tts, SIGNAL(stateChanged(QTextToSpeech::State)));
        spy.wait(10000);
        QCOMPARE(tts.state(), QTextToSpeech::Ready);
        qint64 time = timer.elapsed();
        QVERIFY(time > lastTime);
        lastTime = time;
    }
}

void tst_QTextToSpeech::pitch()
{
    QTextToSpeech tts;
    for (int i = -10; i <= 10; ++i) {
        tts.setPitch(i / 10.0);
        QCOMPARE(tts.pitch(), i / 10.0);
    }
}

void tst_QTextToSpeech::set_voice()
{
    QString text = QStringLiteral("this is an example text");
    QTextToSpeech tts;
    QCOMPARE(tts.state(), QTextToSpeech::Ready);

    // Choose a voice
    QVector<QVoice> voices = tts.availableVoices();
    int vId = 0;
    if (voices.length() > 1) {
        vId = 1;
    }
    tts.setVoice(voices[vId]);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);

    QElapsedTimer timer;
    timer.start();
    tts.say(text);
    QTRY_COMPARE(tts.state(), QTextToSpeech::Speaking);
    QSignalSpy spy(&tts, SIGNAL(stateChanged(QTextToSpeech::State)));
    spy.wait(10000);
    QCOMPARE(tts.state(), QTextToSpeech::Ready);
    QVERIFY(timer.elapsed() > 100);
}

QTEST_MAIN(tst_QTextToSpeech)
#include "tst_qtexttospeech.moc"
