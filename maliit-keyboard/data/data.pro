include(../config.pri)

TARGET = dummy
TEMPLATE = lib

languages.path = $$MALIIT_PLUGINS_DATA_DIR
languages.files = languages/

styles.path = $$MALIIT_KEYBOARD_DATA_DIR
styles.files = styles/

images.path = $$MALIIT_KEYBOARD_DATA_DIR
images.files = images/

sounds.path = $$MALIIT_KEYBOARD_DATA_DIR
sounds.files = sounds/

INSTALLS += languages styles images sounds
