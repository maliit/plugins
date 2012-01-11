include(../../config.pri)

TEMPLATE = app
TARGET = maliit-keyboard-viewer
target.path = $$INSTALL_BIN

INCLUDEPATH += ../lib ../
LIBS += -L../lib -lmaliit-keyboard -L../view -lmaliit-keyboard-view
SOURCES += main.cpp

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

INSTALLS += target
