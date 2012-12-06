LOGIC_DIR = ./logic

HEADERS += \
    logic/hitlogic.h \
    logic/layoutupdater.h \
    logic/keyboardloader.h \
    logic/keyareaconverter.h \
    logic/style.h \
    logic/spellchecker.h \
    logic/wordengine.h \

SOURCES += \
    logic/hitlogic.cpp \
    logic/layoutupdater.cpp \
    logic/keyboardloader.cpp \
    logic/keyareaconverter.cpp \
    logic/style.cpp \
    logic/spellchecker.cpp \
    logic/wordengine.cpp \

DEPENDPATH += $$LOGIC_DIR

include(state-machines/state-machines.pri)
