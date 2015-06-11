TEMPLATE = app

QT += qml quick widgets speechrecognition

SOURCES += main.cpp

RESOURCES += qml.qrc \
    grammar.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)
