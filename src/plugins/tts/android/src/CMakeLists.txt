# Generated from src.pro.

#####################################################################
## QTextToSpeechEngineAndroid Plugin:
#####################################################################

qt_internal_add_plugin(QTextToSpeechEngineAndroid
    OUTPUT_NAME qttexttospeech_android
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_android.cpp qtexttospeech_android.h
        qtexttospeech_android_plugin.cpp qtexttospeech_android_plugin.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::CorePrivate
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:src.pro:<TRUE>:
# OTHER_FILES = "android_plugin.json"
