# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS Multimedia)

qt_internal_add_plugin(QTextToSpeechFlitePlugin
    OUTPUT_NAME qtexttospeech_flite
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_flite.cpp qtexttospeech_flite.h
        qtexttospeech_flite_plugin.cpp qtexttospeech_flite_plugin.h
        qtexttospeech_flite_processor.cpp qtexttospeech_flite_processor.h
    LIBRARIES
        Flite::Flite
        Qt::Core
        Qt::Multimedia
        Qt::TextToSpeech
)

qt_internal_extend_target(QTextToSpeechFlitePlugin CONDITION QT_FEATURE_flite_alsa
    LIBRARIES
        ALSA::ALSA
)
