TARGET = QtSpeech
QT = core-private
DEFINES += QSPEECH_LIBRARY
MODULE = speech

load(qt_module)


HEADERS = \
    qspeech.h \
    qspeech_p.h \
    qspeech_global.h \

SOURCES = \
    qspeech.cpp \

win32 {
    SOURCES += qspeech_win.cpp
    LIBS *= -lole32 -lsapi
} else:mac {
    OBJECTIVE_SOURCES += qspeech_mac.mm
    LIBS *= -framework Cocoa
} else:android {
    SUBDIRS += android
    SOURCES += qspeech_android.cpp
} else:unix {
    SOURCES += qspeech_unix.cpp
    LIBS += -lspeechd
}


ANDROID_BUNDLED_JAR_DEPENDENCIES = \
    jar/QtSpeech-bundled.jar:org.qtproject.qt5.android.speech.QtSpeech
ANDROID_JAR_DEPENDENCIES = \
    jar/QtSpeech.jar:org.qtproject.qt5.android.speech.QtSpeech

OTHER_FILES +=
