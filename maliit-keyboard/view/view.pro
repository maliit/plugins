include(../../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard-view
TEMPLATE = lib
INCLUDEPATH=../lib
LIB += -L../lib -lmaliit-keyboard

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

HEADERS += \
    glass.h \
    renderer.h \
    keyrenderer.h \
    keyareaitem.h \
    keyitem.h \
    abstractbackgroundbuffer.h \
    graphicsview.h \
    abstractsurface.h

SOURCES += \
    glass.cpp \
    renderer.cpp \
    keyrenderer.cpp \
    keyareaitem.cpp \
    keyitem.cpp \
    abstractbackgroundbuffer.cpp \
    graphicsview.cpp \
    abstractsurface.cpp

target.path += $$INSTALL_LIBS
INSTALLS += target



