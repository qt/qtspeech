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
        compare(tts.availableLocales().length, 3)
    }

    function test_availableVoices() {
        compare(tts.availableVoices().length, 2)
    }
}
