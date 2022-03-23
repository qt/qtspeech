qt_internal_add_test(tst_qtexttospeech
    SOURCES
        tst_qtexttospeech.cpp
    PUBLIC_LIBRARIES
        Qt::TextToSpeechPrivate
)

# for a standalone test build we have to find_package SpeechDispatcher again
find_package(SpeechDispatcher)
qt_internal_extend_target(tst_qtexttospeech CONDITION QT_FEATURE_speechd
    PUBLIC_LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
