TEMPLATE = subdirs

exists( ../../3rdparty/pocketsphinx/src/libpocketsphinx/.libs ): exists ( ../../3rdparty/sphinxbase/src/libsphinxbase/.libs ) {
    SUBDIRS += pocketsphinx
} else {
    message( "PocketSphinx and/or SphinxBase not built, skipping PocketSphinx plug-in compilation" )
}
