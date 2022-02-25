# Generated from sapi.pro.

#####################################################################
## QTextToSpeechPluginSapi Plugin:
#####################################################################

qt_internal_add_plugin(QTextToSpeechPluginSapi
    OUTPUT_NAME qtexttospeech_sapi
    PLUGIN_TYPE texttospeech
    SOURCES
        qtexttospeech_sapi.cpp qtexttospeech_sapi.h
        qtexttospeech_sapi_plugin.cpp qtexttospeech_sapi_plugin.h
    LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::TextToSpeech
)

#### Keys ignored in scope 1:.:.:sapi.pro:<TRUE>:
# OTHER_FILES = "sapi_plugin.json"
# QMAKE_CFLAGS = "--Zc:strictStrings"
# QMAKE_CFLAGS_RELEASE = "--Zc:strictStrings"
# QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO = "--Zc:strictStrings"
# QMAKE_CXXFLAGS_RELEASE = "--Zc:strictStrings"
# QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO = "--Zc:strictStrings"

## Scopes:
#####################################################################

qt_internal_extend_target(QTextToSpeechPluginSapi CONDITION MSVC
    COMPILE_OPTIONS
        /Zc:strictStrings
)

qt_internal_extend_target(QTextToSpeechPluginSapi CONDITION MINGW
    LIBRARIES
        ole32
        sapi
        uuid
)
