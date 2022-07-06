# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0-only WITH Qt-GPL-exception-1.0

qt_get_android_sdk_jar_for_api("android-21" android_sdk)

set(java_sources
    src/org/qtproject/qt/android/speech/QtTextToSpeech.java
)

qt_internal_add_jar(QtAndroidTextToSpeech
    INCLUDE_JARS ${android_sdk}
    SOURCES ${java_sources}
    OUTPUT_DIR "${QT_BUILD_DIR}/jar"
)

install_jar(QtAndroidTextToSpeech
    DESTINATION jar
    COMPONENT Devel
)
