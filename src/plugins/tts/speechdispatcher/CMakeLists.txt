# Generated from speechdispatcher.pro.

#####################################################################
## QTextToSpeechPluginSpeechd Plugin:
#####################################################################

qt_internal_add_plugin(QTextToSpeechPluginSpeechd
    OUTPUT_NAME qtexttospeech_speechd
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_speechd.cpp qtexttospeech_speechd.h
        qtexttospeech_speechd_plugin.cpp qtexttospeech_speechd_plugin.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:speechdispatcher.pro:<TRUE>:
# OTHER_FILES = "speechd_plugin.json"
# QT_FOR_CONFIG = "texttospeech-private"

## Scopes:
#####################################################################

qt_internal_extend_target(QTextToSpeechPluginSpeechd CONDITION QT_FEATURE_speechd
    LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
