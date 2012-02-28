include(../../config-plugin.pri)

TARGET = tests-common
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
           utils.cpp \
           utils-gui.cpp \
           inputmethodhostprobe.cpp \

HEADERS += \
           utils.h \
           inputmethodhostprobe.h \

OBJECTS_DIR = .obj
MOC_DIR = .moc

QMAKE_EXTRA_TARGETS += check
check.target = check
check.command = $$system(true)
check.depends += libtests-common.a
