# Generated from winrt.pro.

#####################################################################
## QTextToSpeechPluginWinRT Plugin:
#####################################################################

qt_internal_add_plugin(QTextToSpeechWinRTPlugin
    OUTPUT_NAME qtexttospeech_winrt
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_winrt.cpp qtexttospeech_winrt.h
        qtexttospeech_winrt_plugin.cpp qtexttospeech_winrt_plugin.h
    LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::Gui
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:winrt.pro:<TRUE>:
# OTHER_FILES = "winrt_plugin.json"
