LOGIC_DIR = ./logic

HEADERS += \
    layoutupdater.h \
    keyboardloader.h \
    keyareaconverter.h \
    style.h \

SOURCES += \
    layoutupdater.cpp \
    keyboardloader.cpp \
    keyareaconverter.cpp \
    style.cpp \

DEPENDPATH += $$LOGIC_DIR

include(state-machines/state-machines.pri)
