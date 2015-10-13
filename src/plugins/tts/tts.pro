TEMPLATE = subdirs

unix {
    CONFIG += link_pkgconfig
    packagesExist(speech-dispatcher) {
        SUBDIRS += speechdispatcher
    }
}

windows: SUBDIRS += sapi

config_flite {
    SUBDIRS += flite
}
