TEMPLATE = subdirs

QMAKE_DOCS = $$PWD/doc/qtspeech.qdocconf
load(qt_docs)

SUBDIRS = tts plugins
android:SUBDIRS += tts/android

plugins.depends = tts
