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

import QtQuick 2.4
import QtQuick.Controls 1.3
import QtQuick.Window 2.2
import QtQuick.Dialogs 1.2
import QtSpeechRecognition 0.1

ApplicationWindow {
    title: qsTr("Speech recognition demo")
    width: 640
    height: 480
    visible: true

    SpeechRecognition {
        id: speech
        property string resourceDir: homeDir + "/speechres"
        property url dictionaryFile: "file:lexicon.dict"
        property url mainGrammarFile: "qrc:grammar/main"
        property url yesNoGrammarFile: "qrc:grammar/yesno"
        property var engine: createEngine("local", "pocketsphinx",
                                        { "Locale" : Qt.locale("en_US"),
                                          "ResourceDirectory" : resourceDir,
                                          //"DebugAudioDirectory" : "/tmp",
                                          "Dictionary" : dictionaryFile })
        property var mainGrammar: createGrammar(engine, "main", mainGrammarFile)
        property var yesNoGrammar: createGrammar(engine, "yesno", yesNoGrammarFile)

        Component.onCompleted: {
        }

        onResult: {
            mainForm.resultText = grammarName + ": " + resultData["Transcription"]
            mainForm.statusText = "Ready"
        }
        onError: {
            var errorText = "Error " + errorCode
            if (parameters["Reason"] !== undefined)  {
                errorText += ": " + parameters["Reason"]
            }
            mainForm.resultText = ""
            mainForm.statusText = errorText
            console.log(errorText)
        }
        onListeningStarted: {
            mainForm.statusText = "Listening..."
        }
        onListeningStopped: {
            if (expectResult)
                mainForm.statusText = "Hold on..."
            else
                mainForm.statusText = "Ready"
        }
        onAttributeUpdated: {
            if (key == "AudioLevel")
                mainForm.audioLevel = value
        }
        onStateChanged: {
            console.log("State: " + speech.state)
        }
        onMessage: {
            console.log("Message: " + message)
        }
    }

    Connections {
        target: speech.engine
        onCreated: {
            console.log("Engine " + speech.engine.name + " created")
            var supportedParameters = speech.engine.supportedParameters()
            console.log("Supported engine parameters: " + supportedParameters)
            // Switch to "pulse" audio device if available
            if (supportedParameters.indexOf("AudioInputDevices") !== -1) {
                var inputDevices = speech.engine.parameter("AudioInputDevices")
                if (inputDevices.indexOf("pulse") !== -1)
                    speech.engine.setParameter("AudioInputDevice", "pulse")
            }
            // Example: recognize audio clip instead of live audio:
            //if (supportedParameters.indexOf("AudioInputFile") !== -1) {
            //    speech.engine.setParameter("AudioInputFile", homeDir + "/asr_input_1.wav")
            //}
        }
    }

    MainForm {
        id: mainForm
        anchors.fill: parent
        onSelectedGrammarChanged: {
            switch (selectedGrammar) {
                case mainGrammar:
                    speech.activeGrammar = speech.mainGrammar
                    break;
                case yesNoGrammar:
                    speech.activeGrammar = speech.yesNoGrammar
                    break;
                default:
                    speech.activeGrammar = null
                    break;
            }
        }
        startStopButton.onClicked: {
            speech.dispatchMessage("Start/stop");
            switch (speech.state) {
                case SpeechRecognition.IdleState:
                case SpeechRecognition.ProcessingState:
                    //speech.muted = true
                    speech.startListening()
                    //speech.unmuteAfter(500);
                    break;
                default:
                    speech.stopListening()
                    break;
            }
            mainForm.resultText = ""
        }
    }
}
