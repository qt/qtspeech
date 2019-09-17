# Generated from tts.pro.

#####################################################################
## TextToSpeech Module:
#####################################################################

add_qt_module(TextToSpeech
    PLUGIN_TYPES texttospeech
    SOURCES
        qtexttospeech.cpp qtexttospeech.h qtexttospeech_p.h
        qtexttospeech_global.h
        qtexttospeechengine.cpp qtexttospeechengine.h
        qtexttospeechplugin.cpp qtexttospeechplugin.h
        qvoice.cpp qvoice.h qvoice_p.h
    DEFINES
        QTEXTTOSPEECH_LIBRARY
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
)

if(ANDROID)
    set_property(TARGET TextToSpeech APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/QtTextToSpeech.jar
    )
    set_property(TARGET TextToSpeech APPEND PROPERTY QT_ANDROID_LIB_DEPENDENCIES
        plugins/texttospeech/libqttexttospeech_android.so
    )
endif()

#### Keys ignored in scope 1:.:.:tts.pro:<TRUE>:
# MODULE = "texttospeech"
# MODULE_PLUGIN_TYPES = "texttospeech"
# _LOADED = "qt_module"
