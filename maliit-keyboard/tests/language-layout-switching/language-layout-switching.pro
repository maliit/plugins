include(../../config.pri)
include(../common-check.pri)

TOP_BUILDDIR = $${OUT_PWD}/../../..
TARGET = language-layout-switching
TEMPLATE = app
QT = core testlib

INCLUDEPATH += ../../lib ../../
LIBS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}
PRE_TARGETDEPS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}

HEADERS += \

SOURCES += \
    main.cpp \

include(../../config-bottom.pri)
