# Generated from flite.pro.

#####################################################################
## QTextToSpeechEngineFlite Plugin:
#####################################################################

qt_internal_add_plugin(QTextToSpeechEngineFlite
    OUTPUT_NAME qttexttospeech_flite
    PLUGIN_TYPE texttospeech
    SOURCES
        ../common/qtexttospeechprocessor.cpp ../common/qtexttospeechprocessor_p.h
        qtexttospeech_flite.cpp qtexttospeech_flite.h
        qtexttospeech_flite_plugin.cpp qtexttospeech_flite_plugin.h
        qtexttospeech_flite_processor.cpp qtexttospeech_flite_processor.h
    LIBRARIES
        Flite::Flite
        Qt::Core
        Qt::Multimedia
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:flite.pro:<TRUE>:
# OTHER_FILES = "flite_plugin.json"
# QT_FOR_CONFIG = "texttospeech-private"

## Scopes:
#####################################################################

qt_internal_extend_target(QTextToSpeechEngineFlite CONDITION QT_FEATURE_flite_alsa
    LIBRARIES
        ALSA::ALSA
)
