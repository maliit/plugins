include(../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard
TEMPLATE = lib
QT = core

include(models/models.pri)
include(logic/logic.pri)
include(parser/parser.pri)

target.path += $$INSTALL_LIBS
INSTALLS += target

