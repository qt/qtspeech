# Generated from sapi.pro.

#####################################################################
## qtexttospeech_sapi Plugin:
#####################################################################

add_qt_plugin(qtexttospeech_sapi
    TYPE texttospeech
    CLASS_NAME QTextToSpeechPluginSapi
    SOURCES
        qtexttospeech_sapi.cpp qtexttospeech_sapi.h
        qtexttospeech_sapi_plugin.cpp qtexttospeech_sapi_plugin.h
    PUBLIC_LIBRARIES
        Qt::Core
        Qt::Gui
        Qt::TextToSpeech
    # COMPILE_OPTIONS # special case
    #    --Zc:strictStrings # special case
)

#### Keys ignored in scope 1:.:.:sapi.pro:<TRUE>:
# OTHER_FILES = "sapi_plugin.json"
# PLUGIN_CLASS_NAME = "QTextToSpeechPluginSapi"
# PLUGIN_TYPE = "texttospeech"
# QMAKE_CFLAGS = "--Zc:strictStrings"
# QMAKE_CFLAGS_RELEASE = "--Zc:strictStrings"
# QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO = "--Zc:strictStrings"
# QMAKE_CXXFLAGS_RELEASE = "--Zc:strictStrings"
# QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO = "--Zc:strictStrings"
# _LOADED = "qt_plugin"

## Scopes:
#####################################################################

extend_target(qtexttospeech_sapi CONDITION mingw
    PUBLIC_LIBRARIES
        ole32
        sapi
        uuid
)
