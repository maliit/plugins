include(../config.pri)

VERSION = 0.2.0
TARGET = $${MALIIT_KEYBOARD_TARGET}
TEMPLATE = lib
QT = gui core
CONFIG += staticlib

include(models/models.pri)
include(logic/logic.pri)
include(parser/parser.pri)
