include(../../config.pri)
include(../common_check.pri)

TARGET = language-layout-switching
TEMPLATE = app
QT = core testlib

INCLUDEPATH += ../ ../../lib ../../
LIBS += -L../../lib -lmaliit-keyboard

HEADERS += \
    ../utils.h \

SOURCES += \
    ../utils.cpp \
    main.cpp \

