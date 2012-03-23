LOGIC_DIR = ./logic

HEADERS += \
    hitlogic.h \
    layoutupdater.h \
    keyboardloader.h \
    keyareaconverter.h \
    style.h \
    spellchecker.h \
    wordengine.h \

SOURCES += \
    hitlogic.cpp \
    layoutupdater.cpp \
    keyboardloader.cpp \
    keyareaconverter.cpp \
    style.cpp \
    spellchecker.cpp \
    wordengine.cpp \

DEPENDPATH += $$LOGIC_DIR

include(state-machines/state-machines.pri)


