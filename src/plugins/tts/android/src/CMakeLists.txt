# Generated from src.pro.

#####################################################################
## qttexttospeech_android Plugin:
#####################################################################

add_qt_plugin(qttexttospeech_android
    TYPE texttospeech
    CLASS_NAME QTextToSpeechEngineAndroid
    SOURCES
        qtexttospeech_android.cpp qtexttospeech_android.h
        qtexttospeech_android_plugin.cpp qtexttospeech_android_plugin.h
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:src.pro:<TRUE>:
# OTHER_FILES = "android_plugin.json"
# PLUGIN_CLASS_NAME = "QTextToSpeechEngineAndroid"
# PLUGIN_TYPE = "texttospeech"
# _LOADED = "qt_plugin"
