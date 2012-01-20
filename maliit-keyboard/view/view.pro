include(../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard-view
TEMPLATE = lib
CONFIG += staticlib
INCLUDEPATH = ../lib
LIBS += -L../lib -lmaliit-keyboard

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

HEADERS += \
    utils.h \
    setup.h \
    glass.h \
    renderer.h \
    keyrenderer.h \
    keyareaitem.h \
    keyitem.h \
    abstractbackgroundbuffer.h \
    graphicsview.h \

SOURCES += \
    utils.cpp \
    setup.cpp \
    glass.cpp \
    renderer.cpp \
    keyrenderer.cpp \
    keyareaitem.cpp \
    keyitem.cpp \
    abstractbackgroundbuffer.cpp \
    graphicsview.cpp \
