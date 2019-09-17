# Generated from flite.pro.

#####################################################################
## qttexttospeech_flite Plugin:
#####################################################################

add_qt_plugin(qttexttospeech_flite
    TYPE texttospeech
    CLASS_NAME QTextToSpeechEngineFlite
    SOURCES
        ../common/qtexttospeechprocessor.cpp ../common/qtexttospeechprocessor_p.h
        qtexttospeech_flite.cpp qtexttospeech_flite.h
        qtexttospeech_flite_plugin.cpp qtexttospeech_flite_plugin.h
        qtexttospeech_flite_processor.cpp qtexttospeech_flite_processor.h
    LIBRARIES
        Flite::Flite
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Multimedia
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:flite.pro:<TRUE>:
# OTHER_FILES = "flite_plugin.json"
# PLUGIN_CLASS_NAME = "QTextToSpeechEngineFlite"
# PLUGIN_TYPE = "texttospeech"
# QT_FOR_CONFIG = "texttospeech-private"
# _LOADED = "qt_plugin"

## Scopes:
#####################################################################

extend_target(qttexttospeech_flite CONDITION QT_FEATURE_flite_alsa
    LIBRARIES
        ALSA::ALSA
)
