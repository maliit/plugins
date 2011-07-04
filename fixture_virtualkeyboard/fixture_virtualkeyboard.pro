include(../config.pri)

OBJECTS_DIR = .obj
MOC_DIR = .moc
TEMPLATE = lib
TARGET = fixture_virtualkeyboard
CONFIG += plugin meegotouch meegoimframework

target.path = $$[QT_INSTALL_PLUGINS]/tasfixtures


DEPENDPATH += .
INCLUDEPATH += . ../m-keyboard/common/ ../m-keyboard/widgets/ ../m-keyboard/style/

!nomeegotouch {
    DEFINES += HAVE_MEEGOTOUCH
}

# Input
HEADERS += fixture_virtualkeyboard.h \
           tasfixtureplugininterface.h
SOURCES += fixture_virtualkeyboard.cpp

INSTALLS += target

