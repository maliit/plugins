include(../config.pri)

VERSION = 0.1.0
TARGET = maliit-keyboard
TEMPLATE = lib
QT = core gui declarative

MALIIT_KEYBOARD_DIR=$${MALIIT_PLUGINS_DIR}/maliit-keyboard
MALIIT_KEYBOARD_FILE=$${MALIIT_KEYBOARD_DIR}/maliit-keyboard.qml
DEFINES += "MALIIT_KEYBOARD_FILE=$${MALIIT_KEYBOARD_FILE}"

CONFIG += \
    plugin \

enable-legacy {
    CONFIG += meegoimframework meegoimquick
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += maliit-plugins-quick-0.80
    # moc needs the include path
    INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-quick-0.80 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)
}

SOURCES += \
    maliitkeyboardplugin.cpp \

HEADERS += \
    maliitkeyboardplugin.h \

qml.files = qml/maliit-keyboard.qml qml/assets
qml.path = $${MALIIT_KEYBOARD_DIR}

target.path += $${MALIIT_PLUGINS_DIR}

INSTALLS += target qml
