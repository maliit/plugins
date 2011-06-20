include(config.pri)

CONFIG += ordered 
TARGET = meego-im-plugins
TEMPLATE = subdirs
SUBDIRS = \
    meego-keyboard-quick \

CONFIG(docs) {
    include (doc/doc.pri)
}

!notests:!nomeegotouch {
    SUBDIRS += \
        tests \
        fixture_virtualkeyboard \

}

!nomeegotouch {
    SUBDIRS += \
        m-keyboard \
        translations \

}

#error-correction 
GCONF_DATA = meego-keyboard.schemas
isEmpty(GCONF_SCHEMAS_DIR) {
    GCONF_SCHEMAS_DIR = /usr/share/gconf/schemas
}
gconf_data.path = $$GCONF_SCHEMAS_DIR
gconf_data.files = $$GCONF_DATA

INSTALLS += gconf_data \ 

QMAKE_EXTRA_TARGETS += check-xml
check-xml.target = check-xml
check-xml.CONFIG = recursive

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

QMAKE_CLEAN += build-stamp configure-stamp

