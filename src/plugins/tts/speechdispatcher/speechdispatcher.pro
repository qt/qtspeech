TARGET = qtexttospeech_speechd
PLUGIN_TYPE = texttospeech
PLUGIN_CLASS_NAME = QTextToSpeechPluginSpeechd

load(qt_plugin)

QT += core texttospeech

PKGCONFIG = speech-dispatcher
LIBS += -lspeechd

HEADERS += \
    qtexttospeech_speechd.h \
    qtexttospeech_speechd_plugin.h \

SOURCES += \
    qtexttospeech_speechd.cpp \
    qtexttospeech_speechd_plugin.cpp \

OTHER_FILES += \
    speechd_plugin.json
