TARGET = qttexttospeech_flite
QT = core multimedia texttospeech

PLUGIN_TYPE = texttospeech
PLUGIN_CLASS_NAME = QTextToSpeechEngineFlite
load(qt_plugin)

HEADERS += \
    qtexttospeechengine_flite.h \
    qtexttospeechplugin_flite.h

SOURCES += \
    qtexttospeechengine_flite.cpp \
    qtexttospeechplugin_flite.cpp

OTHER_FILES += \
    flite_plugin.json

LIBS += -lflite_cmu_us_kal16 -lflite_usenglish -lflite_cmulex -lflite
