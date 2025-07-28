# Copyright (C) 2026 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

qt_find_package(NodeAddonApi MODULE PROVIDED_TARGETS NodeAddonApi::NodeAddonApi)

qt_internal_add_plugin(QTextToSpeechOhosPlugin
    OUTPUT_NAME qttexttospeech_ohos
    PLUGIN_TYPE texttospeech
    EXCEPTIONS
    SOURCES
        corespeechkit/texttospeechproxy.cpp corespeechkit/texttospeechproxy.h
        qtexttospeech_ohos.cpp qtexttospeech_ohos.h
        qtexttospeech_ohos_plugin.cpp qtexttospeech_ohos_plugin.h
    DEFINES
        NAPI_CPP_EXCEPTIONS
    LIBRARIES
        NodeAddonApi::NodeAddonApi
        Qt::Core
        Qt::CorePrivate
        Qt::TextToSpeech
        Qt::Multimedia
        ace_napi.z
)
