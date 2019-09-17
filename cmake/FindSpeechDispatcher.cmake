include(FindPkgConfig)

pkg_check_modules(SpeechDispatcher "speech-dispatcher" IMPORTED_TARGET GLOBAL)

if (TARGET PkgConfig::SpeechDispatcher)
    add_library(SpeechDispatcher::SpeechDispatcher ALIAS PkgConfig::SpeechDispatcher)
endif()
