include(../../config.pri)

TEMPLATE = app
TARGET = maliit-keyboard-benchmark
target.path = $$INSTALL_BIN

INCLUDEPATH += ../lib
LIBS += -L../lib -lmaliit-keyboard
SOURCES += main.cpp

QT += core
INSTALLS += target
