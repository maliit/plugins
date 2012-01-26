include(../config.pri)

TOP_BUILDDIR = $${OUT_PWD}/../..
TARGET = $${MALIIT_KEYBOARD_PLUGIN_TARGET}
TEMPLATE = lib
LIBS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB} $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_VIEW_LIB}
PRE_TARGETDEPS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_VIEW_LIB}
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
