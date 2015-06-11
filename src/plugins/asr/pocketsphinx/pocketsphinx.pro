TARGET = qtspeechrecognition_pocketsphinx
QT = core multimedia speechrecognition

PLUGIN_TYPE = speechrecognition
PLUGIN_CLASS_NAME = QSpeechRecognitionPluginPocketSphinx
load(qt_plugin)

HEADERS += \
    qspeechrecognitionengine_pocketsphinx.h \
    qspeechrecognitiongrammar_pocketsphinx.h \
    qspeechrecognitionplugin_pocketsphinx.h

SOURCES += \
    qspeechrecognitionengine_pocketsphinx.cpp \
    qspeechrecognitiongrammar_pocketsphinx.cpp \
    qspeechrecognitionplugin_pocketsphinx.cpp

OTHER_FILES += \
    pocketsphinx_plugin.json

CONFIG += link_pkgconfig
PKGCONFIG += pocketsphinx
