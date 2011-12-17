include(../config.pri)

TARGET = maliit-keyboard-viewer
TEMPLATE = app

INCLUDEPATH += ../lib ../
LIBS += -L../lib -lmaliit-keyboard -L../view -lmaliit-keyboard-view
SOURCES += main.cpp

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

target.path = $$INSTALL_BIN
INSTALLS += target
