TARGET = QtTextToSpeech
QT = core-private
DEFINES += QTEXTTOSPEECH_LIBRARY
MODULE = texttospeech

load(qt_module)


HEADERS = \
    qtexttospeech.h \
    qtexttospeech_p.h \
    qtexttospeech_global.h \

SOURCES = \
    qtexttospeech.cpp \

win32 {
    SOURCES += qtexttospeech_win.cpp
    LIBS *= -lole32 -lsapi
} else:mac {
    OBJECTIVE_SOURCES += qtexttospeech_mac.mm
    LIBS *= -framework Cocoa
} else:android {
    SUBDIRS += android
    SOURCES += qtexttospeech_android.cpp
} else:unix {
    SOURCES += qtexttospeech_unix.cpp
    LIBS += -lspeechd
}


ANDROID_BUNDLED_JAR_DEPENDENCIES = \
    jar/QtTextToSpeech-bundled.jar:org.qtproject.qt5.android.speech.QtTextToSpeech
ANDROID_JAR_DEPENDENCIES = \
    jar/QtTextToSpeech.jar:org.qtproject.qt5.android.speech.QtTextToSpeech

SUBDIRS += \
    android/android.pro

OTHER_FILES += \
    android/jar/src/org/qtproject/qt5/android/speech/QtTextToSpeech.java
