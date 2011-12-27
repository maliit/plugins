include(../../config.pri)

TARGET = language-layout-switching
TEMPLATE = app
QT = core testlib

check.commands = ./language-layout-switching

INCLUDEPATH += ../ ../../lib ../../
LIBS += -L../../lib -lmaliit-keyboard

HEADERS += \
    ../utils.h \

SOURCES += \
    ../utils.cpp \
    main.cpp \

