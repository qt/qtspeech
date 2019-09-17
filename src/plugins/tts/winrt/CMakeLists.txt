# Generated from winrt.pro.

#####################################################################
## qtexttospeech_winrt Plugin:
#####################################################################

add_qt_plugin(qtexttospeech_winrt
    TYPE texttospeech
    CLASS_NAME QTextToSpeechPluginWinRT
    SOURCES
        qtexttospeech_winrt.cpp qtexttospeech_winrt.h
        qtexttospeech_winrt_plugin.cpp qtexttospeech_winrt_plugin.h
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:winrt.pro:<TRUE>:
# OTHER_FILES = "winrt_plugin.json"
# PLUGIN_CLASS_NAME = "QTextToSpeechPluginWinRT"
# PLUGIN_TYPE = "texttospeech"
# _LOADED = "qt_plugin"
