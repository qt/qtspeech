# Generated from texttospeech.pro.

#####################################################################
## tst_qtexttospeech Test:
#####################################################################

add_qt_test(tst_qtexttospeech
    GUI
    SOURCES
        tst_qtexttospeech.cpp
    LIBRARIES
        Qt::TextToSpeechPrivate
    PUBLIC_LIBRARIES
        Qt::TextToSpeech
)

## Scopes:
#####################################################################

qt_find_package(SpeechDispatcher PROVIDED_TARGETS SpeechDispatcher::SpeechDispatcher) # special case
extend_target(tst_qtexttospeech CONDITION QT_FEATURE_speechd
    PUBLIC_LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
