# Generated from hello_speak.pro.

cmake_minimum_required(VERSION 3.14)
project(hello_speak LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if(NOT DEFINED INSTALL_EXAMPLESDIR)
  set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/speech/hello_speak")

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
find_package(Qt6 COMPONENTS TextToSpeech)

qt_add_executable(hello_speak
    main.cpp
    mainwindow.cpp mainwindow.h mainwindow.ui
)
set_target_properties(hello_speak PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
)
target_link_libraries(hello_speak PUBLIC
    Qt::Core
    Qt::Gui
    Qt::TextToSpeech
    Qt::Widgets
)

install(TARGETS hello_speak
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
