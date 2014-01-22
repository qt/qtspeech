/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
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
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
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


#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    connect(ui.speakButton, &QPushButton::clicked, this, &MainWindow::speak);
    connect(ui.stopButton, &QPushButton::clicked, &m_speech, &QTextToSpeech::stop);
    connect(ui.pauseButton, &QPushButton::clicked, &m_speech, &QTextToSpeech::pause);
    connect(ui.resumeButton, &QPushButton::clicked, &m_speech, &QTextToSpeech::resume);

    connect(ui.pitch, SIGNAL(valueChanged(double)), &m_speech, SLOT(setPitch(double)));
    connect(ui.rate, SIGNAL(valueChanged(double)), &m_speech, SLOT(setRate(double)));
    connect(ui.volume, SIGNAL(valueChanged(double)), &m_speech, SLOT(setVolume(double)));

    connect(&m_speech, &QTextToSpeech::stateChanged, this, &MainWindow::stateChanged);
    connect(ui.language, SIGNAL(currentIndexChanged(int)), this, SLOT(languageSelected(int)));
//    connect(ui.voiceType, SIGNAL(currentIndexChanged(QString)), &m_speech, SLOT(setVoiceType(QString)));

//    QStringList voices;
//    foreach (const QTextToSpeechVoice &voice, m_speech.availableVoices()) {

//        voices.append(voice.locale().name() + " (" + voice.name() + ")");
//    }

//    qDebug() << voices;
//    ui.language->addItems(voices);
//    ui.voiceType->addItems(m_speech.availableVoiceTypes().toList());

//    m_speech.voiceTypes();
}

void MainWindow::speak()
{
    m_speech.say(ui.plainTextEdit->toPlainText());
}
void MainWindow::stop()
{
    m_speech.stop();
}

void MainWindow::stateChanged(QTextToSpeech::State state)
{
    if (state == QTextToSpeech::Speaking) {
        ui.statusbar->showMessage("Speech started...");
    } else if (state == QTextToSpeech::Ready)
        ui.statusbar->showMessage("Speech stopped...", 2000);
    else if (state == QTextToSpeech::Paused)
        ui.statusbar->showMessage("Speech paused...");
    else
        ui.statusbar->showMessage("Speech error!");

    ui.pauseButton->setEnabled(state == QTextToSpeech::Speaking);
    ui.resumeButton->setEnabled(state == QTextToSpeech::Paused);
    ui.stopButton->setEnabled(state == QTextToSpeech::Speaking || QTextToSpeech::Paused);
}

void MainWindow::languageSelected(int language)
{
//    QTextToSpeechVoice voice = m_speech.availableVoices()[language];
//    m_speech.setVoice(voice);
}
