if(NOT TARGET Qt::Multimedia)
    message(WARNING "The test case requires Qt::Multimedia to verify the presence of audio devices")
    return()
endif()

qt_internal_add_test(tst_qtexttospeech
    SOURCES
        tst_qtexttospeech.cpp
    PUBLIC_LIBRARIES
        Qt::TextToSpeechPrivate
        Qt::Multimedia
)

# on macOS we need to run a Cocoa event dispatcher
qt_internal_extend_target(tst_qtexttospeech CONDITION MACOS
    PUBLIC_LIBRARIES
        Qt::Gui
)

# for a standalone test build we have to find_package SpeechDispatcher again
find_package(SpeechDispatcher)
qt_internal_extend_target(tst_qtexttospeech CONDITION QT_FEATURE_speechd
    PUBLIC_LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
