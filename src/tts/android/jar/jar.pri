load(qt_build_paths)
CONFIG += java

DESTDIR = $$MODULE_BASE_OUTDIR/jar
API_VERSION = android-10

PATHPREFIX = $$PWD/src/org/qtproject/qt5/android/speech

JAVACLASSPATH += $$PWD/src
JAVASOURCES += $$PATHPREFIX/QtSpeech.java

# install
target.path = $$[QT_INSTALL_PREFIX]/jar
INSTALLS += target

OTHER_FILES += $$JAVASOURCES
