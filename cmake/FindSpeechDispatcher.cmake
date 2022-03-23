# We can't create the same interface imported target multiple times, CMake will complain if we do
# that. This can happen if the find_package call is done in multiple different subdirectories.
if(TARGET SpeechDispatcher::SpeechDispatcher)
    set(SpeechDispatcher_FOUND 1)
    return()
endif()

find_package(PkgConfig QUIET)

pkg_check_modules(SpeechDispatcher "speech-dispatcher" IMPORTED_TARGET GLOBAL)

if (TARGET PkgConfig::SpeechDispatcher)
    add_library(SpeechDispatcher::SpeechDispatcher ALIAS PkgConfig::SpeechDispatcher)
endif()
