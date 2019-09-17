# Generated from ios.pro.

#####################################################################
## qtexttospeech_speechios Plugin:
#####################################################################

add_qt_plugin(qtexttospeech_speechios
    TYPE texttospeech
    CLASS_NAME QTextToSpeechPluginIos
    SOURCES
        qtexttospeech_ios.mm
        qtexttospeech_ios_plugin.cpp qtexttospeech_ios_plugin.h
    PUBLIC_LIBRARIES
        ${FWAVFoundation}
        ${FWFoundation}
        Qt::Core
        Qt::Gui
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:ios.pro:<TRUE>:
# OBJECTIVE_HEADERS = "qtexttospeech_ios.h"
# OTHER_FILES = "ios_plugin.json"
# PLUGIN_CLASS_NAME = "QTextToSpeechPluginIos"
# PLUGIN_TYPE = "texttospeech"
# _LOADED = "qt_plugin"
