// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR BSD-3-Clause

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtTextToSpeech

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
                case TextToSpeech.Error:
                    statusLabel.text = qsTr("Error!")
                    break
            }
        }
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 8
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
