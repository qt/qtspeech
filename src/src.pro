TEMPLATE = subdirs

QMAKE_DOCS = $$PWD/doc/qtspeech.qdocconf
load(qt_docs)

SUBDIRS = tts asr plugins
android:SUBDIRS += tts/android
plugins.depends += asr
