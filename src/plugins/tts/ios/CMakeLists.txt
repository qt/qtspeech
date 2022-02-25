# Generated from ios.pro.

#####################################################################
## QTextToSpeechPluginIos Plugin:
#####################################################################

qt_internal_add_plugin(QTextToSpeechPluginIos
    OUTPUT_NAME qtexttospeech_speechios
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_ios.h qtexttospeech_ios.mm
        qtexttospeech_ios_plugin.cpp qtexttospeech_ios_plugin.h
    LIBRARIES
        ${FWAVFoundation}
        ${FWFoundation}
        Qt::Core
        Qt::Gui
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:ios.pro:<TRUE>:
# OTHER_FILES = "ios_plugin.json"
