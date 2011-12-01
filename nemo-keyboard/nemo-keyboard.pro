include(../config.pri)

VERSION = 0.1.0
TARGET = nemo-keyboard
TEMPLATE = lib
QT = core gui declarative

NEMO_KEYBOARD_DIR=$${MALIIT_PLUGINS_DIR}/nemo-keyboard
NEMO_KEYBOARD_FILE=$${NEMO_KEYBOARD_DIR}/nemo-keyboard.qml
DEFINES += "NEMO_KEYBOARD_FILE=$${NEMO_KEYBOARD_FILE}"

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
    nemo-keyboard-plugin.cpp \

HEADERS += \
    nemo-keyboard-plugin.h \

qml.files = qml/nemo-keyboard.qml qml/assets
qml.path = $${NEMO_KEYBOARD_DIR}

target.path += $${MALIIT_PLUGINS_DIR}

INSTALLS += target qml
