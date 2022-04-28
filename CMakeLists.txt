# Generated from qtspeech.pro.

cmake_minimum_required(VERSION 3.16)

include(.cmake.conf)
project(QtSpeech
    VERSION "${QT_REPO_MODULE_VERSION}"
    DESCRIPTION "Qt Speech Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED COMPONENTS BuildInternals Core)
find_package(Qt6 ${PROJECT_VERSION} CONFIG OPTIONAL_COMPONENTS Gui Widgets Test Multimedia Qml)

qt_build_repo()
