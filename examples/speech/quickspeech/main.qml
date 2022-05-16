/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtSpeech.TextToSpeech

ApplicationWindow {
    id: root
    visible: true
    title: qsTr("Text to Speech")
    minimumWidth: inputForm.implicitWidth
    minimumHeight: inputForm.implicitHeight

    TextToSpeech {
        id: tts
        volume: volumeSlider.value
        pitch: pitchSlider.value
        rate: rateSlider.value

        onStateChanged: (state) => {
            switch (state) {
                case TextToSpeech.Ready:
                    statusLabel.text = qsTr("Ready")
                    break
                case TextToSpeech.Speaking:
                    statusLabel.text = qsTr("Speaking")
                    break
                case TextToSpeech.Paused:
                    statusLabel.text = qsTr("Paused...")
                    break
                case TextToSpeech.Error
                    statusLabel.text = qsTr("Error!")
                    break
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        id: inputForm

        TextArea {
            id: input
            text: qsTr("Hello, world!")
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
        RowLayout {
            Button {
                text: qsTr("Speak")
                enabled: [TextToSpeech.Paused, TextToSpeech.Ready].includes(tts.state)
                onClicked: {
                    let voices = tts.availableVoices()
                    tts.voice = voices[voicesComboBox.currentIndex]
                    tts.say(input.text)
                }
            }
            Button {
                text: qsTr("Pause")
                enabled: tts.state == TextToSpeech.Speaking
                onClicked: tts.pause()
            }
            Button {
                text: qsTr("Resume")
                enabled: tts.state == TextToSpeech.Paused
                onClicked: tts.resume()
            }
            Button {
                text: qsTr("Stop")
                enabled: [TextToSpeech.Speaking, TextToSpeech.Paused].includes(tts.state)
                onClicked: tts.stop()
            }
        }

        GridLayout {
            columns: 2

            Text {
                text: qsTr("Engine:")
            }
            ComboBox {
                id: enginesComboBox
                Layout.fillWidth: true
                model: tts.availableEngines()
                onActivated: {
                    tts.engine = textAt(currentIndex)
                    updateLocales()
                    updateVoices()
                }
            }
            Text {
                text: qsTr("Locale:")
            }
            ComboBox {
                id: localesComboBox
                Layout.fillWidth: true
                onActivated: {
                    let locales = tts.availableLocales()
                    tts.locale = locales[currentIndex]
                    updateVoices()
                }
            }
            Text {
                text: qsTr("Voice:")
            }
            ComboBox {
                id: voicesComboBox
                Layout.fillWidth: true
            }
            Text {
                text: qsTr("Volume:")
            }
            Slider {
                id: volumeSlider
                from: 0
                to: 1.0
                stepSize: 0.2
                value: 0.8
                Layout.fillWidth: true
            }
            Text {
                text: qsTr("Pitch:")
            }
            Slider {
                id: pitchSlider
                from: -1.0
                to: 1.0
                stepSize: 0.5
                value: 0
                Layout.fillWidth: true
            }
            Text {
                text: qsTr("Rate:")
            }
            Slider {
                id: rateSlider
                from: -1.0
                to: 1.0
                stepSize: 0.5
                value: 0
                Layout.fillWidth: true
            }
        }
    }
    footer: Label {
        id: statusLabel
    }

    Component.onCompleted: {
        root.width = inputForm.implicitWidth
        root.height = inputForm.implicitHeight

        enginesComboBox.currentIndex = tts.availableEngines().indexOf(tts.engine)
        updateLocales()
        updateVoices()

        tts.onStateChanged(tts.state)
    }

    function updateLocales() {
        let allLocales = tts.availableLocales().map((locale) => locale.nativeLanguageName)
        let currentLocaleIndex = allLocales.indexOf(tts.locale.nativeLanguageName)
        localesComboBox.model = allLocales
        localesComboBox.currentIndex = currentLocaleIndex
    }

    function updateVoices() {
        voicesComboBox.model = tts.availableVoices().map((voice) => voice.name)
        let indexOfVoice = tts.availableVoices().indexOf(tts.voice)
        voicesComboBox.currentIndex = indexOfVoice
    }
}
