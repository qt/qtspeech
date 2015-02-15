TARGET = QtTextToSpeech
QT = core-private
DEFINES += QTEXTTOSPEECH_LIBRARY
MODULE = texttospeech

load(qt_module)


HEADERS = \
    qtexttospeech.h \
    qtexttospeech_p.h \
    qtexttospeech_global.h \
    qvoice.h \
    qvoice_p.h \

SOURCES = \
    qtexttospeech.cpp \
    qvoice.cpp \

win32 {
    QMAKE_CFLAGS_RELEASE -= -Zc:strictStrings
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -Zc:strictStrings
    SOURCES += qtexttospeech_win.cpp
    LIBS *= -lole32 -lsapi
} else:mac {
    OBJECTIVE_SOURCES += qtexttospeech_mac.mm
    LIBS *= -framework Cocoa
} else:android {
    SUBDIRS += android
    SOURCES += qtexttospeech_android.cpp
} else:unix {
    CONFIG += link_pkgconfig
    SOURCES += qtexttospeech_unix.cpp
    LIBS += -lspeechd
    PKGCONFIG = speech-dispatcher
}


ANDROID_BUNDLED_JAR_DEPENDENCIES = \
    jar/QtTextToSpeech-bundled.jar:org.qtproject.qt5.android.speech.QtTextToSpeech
ANDROID_JAR_DEPENDENCIES = \
    jar/QtTextToSpeech.jar:org.qtproject.qt5.android.speech.QtTextToSpeech

SUBDIRS += \
    android/android.pro

OTHER_FILES += \
    android/jar/src/org/qtproject/qt5/android/speech/QtTextToSpeech.java
