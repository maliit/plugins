include(../config.pri)
include(view.pri)

TOP_BUILDDIR = $${OUT_PWD}/../..
VERSION = 0.2.0
TARGET = $${MALIIT_KEYBOARD_VIEW_TARGET}
TEMPLATE = lib
CONFIG += staticlib
INCLUDEPATH = ../lib
LIBS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}

PRE_TARGETDEPS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

HEADERS += \
    utils.h \
    setup.h \
    abstracttexteditor.h \
    glass.h \
    renderer.h \
    keyareaitem.h \
    keyitem.h \
    wordribbonitem.h \
    abstractfeedback.h \
    nullfeedback.h \
    surface.h \

SOURCES += \
    utils.cpp \
    setup.cpp \
    abstracttexteditor.cpp \
    glass.cpp \
    renderer.cpp \
    keyareaitem.cpp \
    keyitem.cpp \
    wordribbonitem.cpp \
    abstractfeedback.cpp \
    nullfeedback.cpp \
    surface.cpp \

enable-qt-mobility {
    HEADERS += soundfeedback.h
    SOURCES += soundfeedback.cpp
}

disable-background-translucency {
    DEFINES += DISABLE_TRANSLUCENT_BACKGROUND_HINT
}

include(../word-prediction.pri)
