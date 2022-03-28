qt_internal_add_plugin(QTextToSpeechMockPlugin
    OUTPUT_NAME qtexttospeech_mock
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_mock.cpp qtexttospeech_mock.h
        qtexttospeech_mock_plugin.cpp qtexttospeech_mock_plugin.h
    LIBRARIES
        Qt::Core
        Qt::TextToSpeech
)
