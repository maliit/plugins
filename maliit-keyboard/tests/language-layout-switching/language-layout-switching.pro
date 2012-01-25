include(../../config.pri)
include(../common-check.pri)

TOP_BUILDDIR = $${OUT_PWD}/../../..
TARGET = language-layout-switching
TEMPLATE = app
QT = core testlib

INCLUDEPATH += ../ ../../lib ../../
LIBS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}

HEADERS += \
    ../utils.h \

SOURCES += \
    ../utils.cpp \
    main.cpp \

