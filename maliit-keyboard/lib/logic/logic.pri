LOGIC_DIR = ./logic

HEADERS += \
    hitlogic.h \
    layoutupdater.h \
    keyboardloader.h \
    keyareaconverter.h \
    style.h \

SOURCES += \
    hitlogic.cpp \
    layoutupdater.cpp \
    keyboardloader.cpp \
    keyareaconverter.cpp \
    style.cpp \

DEPENDPATH += $$LOGIC_DIR

include(state-machines/state-machines.pri)


