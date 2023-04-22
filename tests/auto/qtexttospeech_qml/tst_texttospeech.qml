// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

import QtQuick

import QtTest
import QtTextToSpeech

TestCase {
    id: testCase
    name: "TextToSpeech"

    TextToSpeech {
        id: tts
        engine: "mock"
    }

    // verifies that the mock engine is synchronous
    function initTestCase() {
        compare(tts.state, TextToSpeech.Ready)
    }

    function test_availableLocales() {
        compare(tts.availableLocales().length, 5)
    }

    function test_availableVoices() {
        compare(tts.availableVoices().length, 2)
    }

    function test_findVoices() {
        let bob = tts.findVoices({"name":"Bob"})
        compare(bob.length, 1)
        let women = tts.findVoices({"gender":Voice.Female})
        compare(women.length, 5)
        let children = tts.findVoices({"age":Voice.Child})
        compare(children.length, 1)
        // includes all english speakers, no matter where they're from
        let english = tts.findVoices({"language":Qt.locale("en")})
        compare(english.length, 4)
        let bokmalers = tts.findVoices({"locale":Qt.locale("NO")})
        compare(bokmalers.length, 2)
        let nynorskers = tts.findVoices({"locale":Qt.locale("nn-NO")})
        compare(nynorskers.length, 2)

        let englishWomen = tts.findVoices({
            "language": Qt.locale("en"),
            "gender": Voice.Female,
            "age": Voice.Adult
        });
        compare(englishWomen.length, 1)
    }

    Component {
        id: name_selector
        TextToSpeech {
            engine: "mock"
            VoiceSelector.name: "Ingvild"
        }
    }

    Component {
        id: genderLanguage_selector
        TextToSpeech {
            engine: "mock"
            VoiceSelector.gender: Voice.Female
            VoiceSelector.language: Qt.locale("en")
        }
    }

    function test_voiceSelector() {
        var selector = createTemporaryObject(name_selector, testCase)
        compare(selector.voice.name, "Ingvild")

        // there is no way to get to QLocale::English from QML
        let EnglishID = 75

        selector = createTemporaryObject(genderLanguage_selector, testCase)
        verify(["Anne", "Mary"].includes(selector.voice.name))
        let oldName = selector.voice.name
        compare(selector.voice.gender, Voice.Female)
        compare(selector.voice.language, EnglishID)

        // overwrite after initialization
        selector.VoiceSelector.gender = Voice.Male
        // no change until select is called
        compare(selector.voice.name, oldName)
        selector.VoiceSelector.select()

        compare(selector.voice.name, "Bob")
        compare(selector.voice.gender, Voice.Male)
        compare(selector.voice.language, EnglishID)
    }

    function test_delayedSelection() {
        var selector = createTemporaryObject(name_selector, testCase)

        selector.VoiceSelector.gender = Voice.Female
        selector.VoiceSelector.name = "Kjersti"
        selector.VoiceSelector.select()

        compare(selector.voice.name, "Kjersti")
    }

    function test_regularExpressionName() {
        var selector = createTemporaryObject(name_selector, testCase)

        selector.VoiceSelector.name = /K.*/
        selector.VoiceSelector.select()

        verify(["Kjersti", "Kari"].includes(selector.voice.name))
    }
}
