# Generated from qtspeech.pro.

cmake_minimum_required(VERSION 3.15.0)

project(QtSpeech
    VERSION 6.0.0
    DESCRIPTION "Qt Speech Libraries"
    HOMEPAGE_URL "https://qt.io/"
    LANGUAGES CXX C
)

find_package(Qt6 ${PROJECT_VERSION} CONFIG REQUIRED
    COMPONENTS
        BuildInternals
        Core
    OPTIONAL_COMPONENTS
        Gui)

qt_build_repo()
