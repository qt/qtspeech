# Generated from tests.pro.

if(NOT TARGET Qt::Test)
    cmake_minimum_required(VERSION 3.15.0)
    project(QtSpeechTests LANGUAGES C CXX) # special case
    find_package(Qt6 ${PROJECT_VERSION} REQUIRED COMPONENTS BuildInternals Core Test TextToSpeech) # special case
    qt_set_up_standalone_tests_build()
endif()

qt_build_tests()
