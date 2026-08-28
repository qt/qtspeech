# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

if(NOT QT_BUILD_STANDALONE_TESTS AND NOT QT_BUILDING_QT)
    cmake_minimum_required(VERSION 3.16)
    project(tst_qtexttospeech LANGUAGES CXX)
    find_package(Qt6BuildInternals REQUIRED COMPONENTS STANDALONE_TEST)
endif()

qt_internal_add_test(tst_qtexttospeech
    SOURCES
        tst_qtexttospeech.cpp
    LIBRARIES
        Qt::TextToSpeechPrivate
        Qt::Multimedia
)

# on macOS we need to run a Cocoa event dispatcher
qt_internal_extend_target(tst_qtexttospeech CONDITION MACOS
    LIBRARIES
        Qt::Gui
)

# for a standalone test build we have to find_package SpeechDispatcher again
find_package(SpeechDispatcher)
qt_internal_extend_target(tst_qtexttospeech CONDITION QT_FEATURE_speechd
    LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
