TEMPLATE = subdirs

QMAKE_DOCS = $$PWD/doc/qtspeech.qdocconf
load(qt_docs)

SUBDIRS = tts
android: SUBDIRS += android

