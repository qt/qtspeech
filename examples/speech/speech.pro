TEMPLATE = subdirs
SUBDIRS = hello_speak

qtHaveModule(quick) {
    SUBDIRS += qmlspeech
}

