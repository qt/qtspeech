TARGET = qttexttospeech_flite
QT = core multimedia texttospeech

PLUGIN_TYPE = texttospeech
PLUGIN_CLASS_NAME = QTextToSpeechEngineFlite
load(qt_plugin)

HEADERS += \
    qtexttospeech_flite.h \
    qtexttospeech_flite_plugin.h

SOURCES += \
    qtexttospeech_flite.cpp \
    qtexttospeech_flite_plugin.cpp

OTHER_FILES += \
    flite_plugin.json

LIBS += -lflite_cmu_us_kal16 -lflite_usenglish -lflite_cmulex -lflite
