include(../../config.pri)

VERSION = 0.2.0
TARGET = maliit-keyboard
TEMPLATE = lib
DEFINES += MALIIT_PLUGINS_DATA_DIR=\\\"$${MALIIT_PLUGINS_DATA_DIR}\\\"

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

ORG_MALIIT_DIR = "$${MALIIT_PLUGINS_DATA_DIR}/org/maliit"
DEFINES += MALIIT_KEYBOARD_IMAGES_DIR=\\\"$${ORG_MALIIT_DIR}/images\\\"
DEFINES += MALIIT_KEYBOARD_STYLES_DIR=\\\"$${ORG_MALIIT_DIR}/styles\\\"

include(models/models.pri)
include(logic/logic.pri)
include(parser/parser.pri)

target.path += $$INSTALL_LIBS
languages.path = $$MALIIT_PLUGINS_DATA_DIR
languages.files = languages/
images.path = $$ORG_MALIIT_DIR
images.files = images/
styles.path = $$ORG_MALIIT_DIR
styles.files = styles/

INSTALLS += target languages images styles

