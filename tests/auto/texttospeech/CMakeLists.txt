# Generated from texttospeech.pro.

#####################################################################
## tst_qtexttospeech Test:
#####################################################################

qt_internal_add_test(tst_qtexttospeech
    SOURCES
        tst_qtexttospeech.cpp
    PUBLIC_LIBRARIES
        Qt::TextToSpeechPrivate
)

## Scopes:
#####################################################################

qt_find_package(SpeechDispatcher PROVIDED_TARGETS SpeechDispatcher::SpeechDispatcher) # special case
qt_internal_extend_target(tst_qtexttospeech CONDITION QT_FEATURE_speechd
    PUBLIC_LIBRARIES
        SpeechDispatcher::SpeechDispatcher
)
