qt_internal_add_test(tst_qtexttospeech
    SOURCES
        tst_qtexttospeech.cpp
    PUBLIC_LIBRARIES
        Qt::TextToSpeechPrivate
)

qt_find_package(SpeechDispatcher PROVIDED_TARGETS SpeechDispatcher::SpeechDispatcher)
qt_internal_extend_target(tst_qtexttospeech CONDITION QT_FEATURE_speechd
    PUBLIC_LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
