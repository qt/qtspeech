/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the documentation of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QSpeechRecognition>
#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

class MyApp : public QCoreApplication
{
    Q_OBJECT

public:
    explicit MyApp(int &argc, char **argv);

public slots:
    void onSpeechButtonClicked();
    void onResult(const QString &grammarName, const QVariantMap &resultData);
    void onError(QSpeechRecognition::Error errorCode, const QVariantMap &parameters);
    void onListeningStarted(const QString &grammarName);
    void onListeningStopped(bool expectResult);
    void onMessage(const QString &message, const QVariantMap &parameters);

private:
    QSpeechRecognition *asr;
};

MyApp::MyApp(int &argc, char **argv):
    QCoreApplication(argc, argv),
    asr(0)
{
//! [Init]
    QVariantMap params;
    QSpeechRecognition *asr = new QSpeechRecognition(this);
    params.insert("locale", QVariant(QLocale("en_US")));
    QSpeechRecognitionEngine *pocketAsr = asr->createEngine("local", "pocketsphinx", params);
    asr->createGrammar(pocketAsr, "main", QUrl(QStringLiteral("file:grammar.jsgf")));
    connect(asr, &QSpeechRecognition::result, this, &MyApp::onResult);
    connect(asr, &QSpeechRecognition::error, this, &MyApp::onError);
    connect(asr, &QSpeechRecognition::listeningStarted, this, &MyApp::onListeningStarted);
    connect(asr, &QSpeechRecognition::listeningStopped, this, &MyApp::onListeningStopped);
//! [Init]
    connect(asr, &QSpeechRecognition::message, this, &MyApp::onMessage);
    asr->dispatchMessage("init_done");
    MyApp::asr = asr;
}

//! [Start and stop]
void MyApp::onSpeechButtonClicked()
{
    switch (asr->state()) {
        case QSpeechRecognition::IdleState:
        case QSpeechRecognition::ProcessingState:
            asr->setActiveGrammar(asr->grammar("main"));
            asr->startListening();
            break;
        default:
            asr->stopListening();
            break;
    }
}
//! [Start and stop]

void MyApp::onResult(const QString &grammarName, const QVariantMap &resultData)
{
    qDebug() << grammarName << ": " << resultData;
    quit();
}

void MyApp::onError(QSpeechRecognition::Error errorCode, const QVariantMap &parameters)
{
    qDebug() << errorCode << parameters;
    quit();
}

void MyApp::onMessage(const QString &message, const QVariantMap &parameters)
{
    onSpeechButtonClicked();
    QTimer::singleShot(2000, this, &MyApp::onSpeechButtonClicked);
}

void MyApp::onListeningStarted(const QString &grammarName)
{
    qDebug() << "Start";
}

void MyApp::onListeningStopped(bool expectResult)
{
    qDebug() << "Stop";
}

int main(int argc, char *argv[])
{
    MyApp app(argc, argv);
    app.exec();
}

#include "speechrecognition.moc"
