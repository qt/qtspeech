qt_internal_add_module(TextToSpeech
    PLUGIN_TYPES texttospeech
    SOURCES
        qtexttospeech.cpp qtexttospeech.h qtexttospeech_p.h
        qtexttospeech_global.h
        qtexttospeech_qmltypes_p.h
        qtexttospeechengine.cpp qtexttospeechengine.h
        qtexttospeechplugin.cpp qtexttospeechplugin.h
        qvoice.cpp qvoice.h qvoice_p.h
    DEFINES
        QTEXTTOSPEECH_LIBRARY
    LIBRARIES
        Qt::CorePrivate
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
)

if(TARGET Qt::Qml)
    qt_internal_add_qml_module(TextToSpeech
        URI "QtSpeech.TextToSpeech"
        VERSION "${PROJECT_VERSION}"
        SOURCES
            qtexttospeech_qmltypes_p.h
        PUBLIC_LIBRARIES
            Qt::TextToSpeech
        GENERATE_CPP_EXPORTS
    )
endif()

if(ANDROID)
    set_property(TARGET TextToSpeech APPEND PROPERTY QT_ANDROID_BUNDLED_JAR_DEPENDENCIES
        jar/QtAndroidTextToSpeech.jar
    )
    set_property(TARGET TextToSpeech APPEND PROPERTY QT_ANDROID_LIB_DEPENDENCIES
        plugins/texttospeech/libplugins_texttospeech_qttexttospeech_android.so
    )
endif()
