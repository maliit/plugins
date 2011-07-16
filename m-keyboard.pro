include(config.pri)

CONFIG += ordered 
TARGET = meego-im-plugins
TEMPLATE = subdirs
SUBDIRS = \
    meego-keyboard-quick \

docs {
    include (doc/doc.pri)
}

!notests:!nomeegotouch {
    SUBDIRS += \
        m-keyboard \
        tests \
        fixture_virtualkeyboard \

}

!nomeegotouch {
    SUBDIRS += \
        translations \

}

!nomeegotouch {
    #error-correction
    GCONF_DATA = meego-keyboard.schemas
    isEmpty(GCONF_SCHEMAS_DIR) {
        GCONF_SCHEMAS_DIR = /usr/share/gconf/schemas
    }
    gconf_data.path = $$GCONF_SCHEMAS_DIR
    gconf_data.files = $$GCONF_DATA

    INSTALLS += gconf_data \
}

!noreactionmap {
    DEFINES += HAVE_REACTIONMAP
    CONFIG += meegoreactionmap
}

QMAKE_EXTRA_TARGETS += check-xml
check-xml.target = check-xml
check-xml.CONFIG = recursive

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

QMAKE_CLEAN += build-stamp configure-stamp

