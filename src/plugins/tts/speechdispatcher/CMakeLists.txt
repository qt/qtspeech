# Generated from speechdispatcher.pro.

#####################################################################
## qtexttospeech_speechd Plugin:
#####################################################################

add_qt_plugin(qtexttospeech_speechd
    TYPE texttospeech
    CLASS_NAME QTextToSpeechPluginSpeechd
    SOURCES
        qtexttospeech_speechd.cpp qtexttospeech_speechd.h
        qtexttospeech_speechd_plugin.cpp qtexttospeech_speechd_plugin.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:speechdispatcher.pro:<TRUE>:
# OTHER_FILES = "speechd_plugin.json"
# PLUGIN_CLASS_NAME = "QTextToSpeechPluginSpeechd"
# PLUGIN_TYPE = "texttospeech"
# QT_FOR_CONFIG = "texttospeech-private"
# _LOADED = "qt_plugin"

## Scopes:
#####################################################################

extend_target(qtexttospeech_speechd CONDITION QT_FEATURE_speechd
    LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
