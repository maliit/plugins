LOGIC_DIR = ./logic

HEADERS += \
    logic/hitlogic.h \
    logic/layouthelper.h \
    logic/layoutupdater.h \
    logic/keyboardloader.h \
    logic/keyareaconverter.h \
    logic/style.h \
    logic/spellchecker.h \
    logic/abstracttexteditor.h \
    logic/abstractwordengine.h \
    logic/wordengine.h \
    logic/abstractlanguagefeatures.h \
    logic/languagefeatures.h \
    logic/eventhandler.h \

SOURCES += \
    logic/hitlogic.cpp \
    logic/layouthelper.cpp \
    logic/layoutupdater.cpp \
    logic/keyboardloader.cpp \
    logic/keyareaconverter.cpp \
    logic/style.cpp \
    logic/spellchecker.cpp \
    logic/abstracttexteditor.cpp \
    logic/abstractwordengine.cpp \
    logic/wordengine.cpp \
    logic/abstractlanguagefeatures.cpp \
    logic/languagefeatures.cpp \
    logic/eventhandler.cpp \

DEFINES += HUNSPELL_DICT_PATH=\\\"$$HUNSPELL_DICT_PATH\\\"

DEPENDPATH += $$LOGIC_DIR

include(state-machines/state-machines.pri)
