# Generated from osx.pro.

#####################################################################
## qtexttospeech_speechosx Plugin:
#####################################################################

add_qt_plugin(qtexttospeech_speechosx
    TYPE texttospeech
    CLASS_NAME QTextToSpeechPluginOsx
    SOURCES
        qtexttospeech_osx.h qtexttospeech_osx.mm
        qtexttospeech_osx_plugin.cpp qtexttospeech_osx_plugin.h
    PUBLIC_LIBRARIES
        ${FWCocoa}
        Qt::Core
        Qt::Gui
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:osx.pro:<TRUE>:
# OTHER_FILES = "osx_plugin.json"
# PLUGIN_CLASS_NAME = "QTextToSpeechPluginOsx"
# PLUGIN_TYPE = "texttospeech"
# _LOADED = "qt_plugin"
