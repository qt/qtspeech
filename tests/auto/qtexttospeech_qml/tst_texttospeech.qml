// Copyright (C) 2023 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

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
}
