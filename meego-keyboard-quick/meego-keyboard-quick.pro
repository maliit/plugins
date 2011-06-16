VERSION = 0.1.0
TARGET = meego-keyboard-quick
TEMPLATE = lib
QT = core gui declarative

!nomeegotouch {
    MEEGO_IM_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir MeegoImFramework)
} else {
    MEEGO_IM_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir maliit-plugins-0.20)
}
MEEGO_KEYBOARD_QUICK_DIR=$${MEEGO_IM_PLUGINS_DIR}/meego-keyboard-qml
MEEGO_KEYBOARD_QUICK_FILE=$${MEEGO_KEYBOARD_QUICK_DIR}/meego-keyboard.qml
DEFINES += "MEEGO_KEYBOARD_QUICK_FILE=$${MEEGO_KEYBOARD_QUICK_FILE}"

CONFIG += \
    plugin \

!nomeegotouch {
    CONFIG += meegoimframework meegoimquick
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += maliit-plugins-quick-0.20
    # moc needs the include path
    INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-quick-0.20 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)
}

SOURCES += \
    meegokeyboardquickplugin.cpp \

HEADERS += \
    meegokeyboardquickplugin.h \

qml.files = qml/meego-keyboard.qml qml/assets
qml.path = $${MEEGO_KEYBOARD_QUICK_DIR}

target.path += $${MEEGO_IM_PLUGINS_DIR}

INSTALLS += target qml
