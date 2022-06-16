qt_internal_add_module(TextToSpeech
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
        Qt::QmlIntegration
    PUBLIC_LIBRARIES
        Qt::Core
    PRIVATE_MODULE_INTERFACE
        Qt::CorePrivate
)


if(TARGET Qt::Qml)
    qt_internal_add_qml_module(TextToSpeechQml
        URI "QtTextToSpeech"
        VERSION "${PROJECT_VERSION}"
        PLUGIN_TARGET TextToSpeechQml
        SOURCES
            qtexttospeech_qmltypes_p.h
        LIBRARIES
            Qt::TextToSpeech
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

qt_internal_add_docs(TextToSpeech
    doc/qttexttospeech.qdocconf
)
