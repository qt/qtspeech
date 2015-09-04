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

LIBS += $$PWD/../../../3rdparty/pocketsphinx/src/libpocketsphinx/.libs/libpocketsphinx.a
LIBS += $$PWD/../../../3rdparty/sphinxbase/src/libsphinxbase/.libs/libsphinxbase.a

INCLUDEPATH += $$PWD/../../../3rdparty/pocketsphinx/include
INCLUDEPATH += $$PWD/../../../3rdparty/sphinxbase/include
DEPENDPATH += $$PWD/../../../3rdparty/pocketsphinx/include
DEPENDPATH += $$PWD/../../../3rdparty/sphinxbase/include

PRE_TARGETDEPS += $$PWD/../../../3rdparty/pocketsphinx/src/libpocketsphinx/.libs/libpocketsphinx.a
PRE_TARGETDEPS += $$PWD/../../../3rdparty/sphinxbase/src/libsphinxbase/.libs/libsphinxbase.a
