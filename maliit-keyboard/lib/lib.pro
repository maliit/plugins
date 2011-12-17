include(../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard
TEMPLATE = lib

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

include(models/models.pri)
include(logic/logic.pri)
include(parser/parser.pri)

target.path += $$INSTALL_LIBS
INSTALLS += target

