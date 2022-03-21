qt_internal_add_plugin(QTextToSpeechIosPlugin
    OUTPUT_NAME qtexttospeech_speechios
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_ios.h qtexttospeech_ios.mm
        qtexttospeech_ios_plugin.cpp qtexttospeech_ios_plugin.h
    LIBRARIES
        ${FWAVFoundation}
        ${FWFoundation}
        Qt::Core
        Qt::Gui
        Qt::TextToSpeech
)
