

#### Inputs



#### Libraries

qt_find_package(Flite PROVIDED_TARGETS Flite::Flite)
qt_find_package(ALSA PROVIDED_TARGETS ALSA::ALSA)
qt_find_package(SpeechDispatcher PROVIDED_TARGETS SpeechDispatcher::SpeechDispatcher)


#### Tests



#### Features

qt_feature("flite" PRIVATE
    LABEL "Flite"
    CONDITION Flite_FOUND
)
qt_feature("flite_alsa" PRIVATE
    LABEL "Flite with ALSA"
    CONDITION Flite_FOUND AND ALSA_FOUND
)
qt_feature("speechd" PUBLIC
    LABEL "Speech Dispatcher"
    AUTODETECT UNIX
    CONDITION SpeechDispatcher_FOUND
)
