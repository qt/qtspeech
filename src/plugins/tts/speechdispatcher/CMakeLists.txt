# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_internal_add_plugin(QTextToSpeechSpeechdPlugin
    OUTPUT_NAME qtexttospeech_speechd
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_speechd.cpp qtexttospeech_speechd.h
        qtexttospeech_speechd_plugin.cpp qtexttospeech_speechd_plugin.h
    LIBRARIES
        Qt::Core
        Qt::TextToSpeech
)

qt_internal_extend_target(QTextToSpeechSpeechdPlugin CONDITION QT_FEATURE_speechd
    LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
