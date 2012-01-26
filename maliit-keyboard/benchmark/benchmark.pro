include(../config.pri)

TOP_BUILDDIR = $${OUT_PWD}/../..
TEMPLATE = app
TARGET = maliit-keyboard-benchmark
target.path = $$INSTALL_BIN

INCLUDEPATH += ../lib
LIBS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}
PRE_TARGETDEPS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB}
SOURCES += main.cpp

QT = core
INSTALLS += target
