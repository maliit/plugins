include(../config.pri)

VERSION = 0.2.0
TARGET = $${MALIIT_KEYBOARD_TARGET}
TEMPLATE = lib
QT = core
CONFIG += staticlib

include(models/models.pri)
include(logic/logic.pri)
include(parser/parser.pri)

HEADERS += coreutils.h
SOURCES += coreutils.cpp
