# Generated from osx.pro.

#####################################################################
## QTextToSpeechPluginOsx Plugin:
#####################################################################

qt_internal_add_plugin(QTextToSpeechPluginOsx
    OUTPUT_NAME qtexttospeech_speechosx
    PLUGIN_TYPE texttospeech
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
