include(../config.pri)

TARGET = dummy
TEMPLATE = lib

languages.path = $$MALIIT_PLUGINS_DATA_DIR
languages.files = languages/

styles.path = $$ORG_MALIIT_DIR
styles.files = styles/

images.path = $$ORG_MALIIT_DIR
images.files = images/

INSTALLS += languages styles images
