TARGET = qtexttospeech_speechd
PLUGIN_TYPE = texttospeech
PLUGIN_CLASS_NAME = QTextToSpeechPluginSapi

load(qt_plugin)

QT += core texttospeech

HEADERS += \
    qtexttospeech_sapi.h \
    qtexttospeech_sapi_plugin.h \

SOURCES += \
    qtexttospeech_sapi.cpp \
    qtexttospeech_sapi_plugin.cpp \

OTHER_FILES += \
    sapi_plugin.json \
    sapi_plugin.json
