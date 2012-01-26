include(../../config.pri)
include(../common-check.pri)

TOP_BUILDDIR = $${OUT_PWD}/../../..
TARGET = commit-string
TEMPLATE = app
QT = core testlib gui

!contains(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

enable-legacy {
    CONFIG += meegoimframework
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += maliit-plugins-0.80
    # moc needs the include path
    INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-0.80 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)
}

INCLUDEPATH += ../ ../../lib ../../
LIBS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB} $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_VIEW_LIB} $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_PLUGIN_LIB}
PRE_TARGETDEPS += $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_LIB} $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_VIEW_LIB} $${TOP_BUILDDIR}/$${MALIIT_KEYBOARD_PLUGIN_LIB}

HEADERS += \
    ../utils.h \

SOURCES += \
    ../utils.cpp \
    main.cpp \

