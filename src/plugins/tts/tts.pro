TEMPLATE = subdirs

unix {
    CONFIG += link_pkgconfig
    packagesExist(speech-dispatcher) {
        SUBDIRS += speechdispatcher
    }
}
