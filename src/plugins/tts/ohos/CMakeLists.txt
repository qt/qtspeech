# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_internal_add_plugin(QTextToSpeechOhosPlugin
    OUTPUT_NAME qttexttospeech_ohos
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_ohos.cpp qtexttospeech_ohos.h
        qtexttospeech_ohos_plugin.cpp qtexttospeech_ohos_plugin.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::TextToSpeech
        Qt::Multimedia
)
