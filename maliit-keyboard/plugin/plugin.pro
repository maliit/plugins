include(../config.pri)

TARGET = maliit-keyboard-plugin
TEMPLATE = lib
LIBS += -L../lib -lmaliit-keyboard -L../view -lmaliit-keyboard-view
INCLUDEPATH += ../lib ../

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

CONFIG += \
    plugin \

enable-legacy {
    CONFIG += meegoimframework
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += maliit-plugins-0.80
    # moc needs the include path
    INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-0.80 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)
}


HEADERS += \
    plugin.h \
    inputmethod.h \
    editor.h \

SOURCES += \
    plugin.cpp \
    inputmethod.cpp \
    editor.cpp \

target.path += $${MALIIT_PLUGINS_DIR}
INSTALLS += target
