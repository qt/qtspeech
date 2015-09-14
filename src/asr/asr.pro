TARGET = QtSpeechRecognition
QT = core-private
DEFINES += QSPEECHRECOGNITION_LIBRARY
MODULE = speechrecognition
MODULE_PLUGIN_TYPES = speechrecognition

load(qt_module)

PUBLIC_HEADERS += \
    $$PWD/qspeechrecognition.h \
    $$PWD/qspeechrecognitiongrammar.h \
    $$PWD/qspeechrecognitionengine.h \
    $$PWD/qspeechrecognitionplugin.h \
    $$PWD/qspeechrecognitionpluginengine.h \
    $$PWD/qspeechrecognitionpluginengine.h \
    $$PWD/qspeechrecognitionplugingrammar.h \
    $$PWD/qspeechrecognitionaudiobuffer.h

PRIVATE_HEADERS += \
    $$PWD/qspeechrecognition_p.h \
    $$PWD/qspeechrecognitiongrammar_p.h \
    $$PWD/qspeechrecognitionengine_p.h \
    $$PWD/qspeechrecognitionmanager_p.h \
    $$PWD/qspeechrecognitionpluginloader_p.h \
    $$PWD/qspeechrecognitionpluginengine_p.h \
    $$PWD/qspeechrecognitionplugingrammar_p.h \
    $$PWD/qspeechrecognitionaudiobuffer_p.h \
    $$PWD/qspeechrecognition_global.h

SOURCES += \
    $$PWD/qspeechrecognition.cpp \
    $$PWD/qspeechrecognitionmanager.cpp \
    $$PWD/qspeechrecognitionaudiobuffer.cpp \
    $$PWD/qspeechrecognitionengine.cpp \
    $$PWD/qspeechrecognitiongrammar.cpp \
    $$PWD/qspeechrecognitionpluginloader.cpp \
    $$PWD/qspeechrecognitionpluginengine.cpp \
    $$PWD/qspeechrecognitionplugin.cpp \
    $$PWD/qspeechrecognitionplugingrammar.cpp

HEADERS += $$PUBLIC_HEADERS $$PRIVATE_HEADERS
