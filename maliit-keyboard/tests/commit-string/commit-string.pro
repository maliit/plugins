include(../../config.pri)

TARGET = commit-string
TEMPLATE = app
QT = core testlib gui

!contains(QT_MAJOR_VERSION, 4) {
    QT += widgets
}

check.commands = ./commit-string

enable-legacy {
    CONFIG += meegoimframework
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += maliit-plugins-0.80
    # moc needs the include path
    INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-0.80 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)
}

INCLUDEPATH += ../ ../../lib ../../
LIBS += -L../../lib -lmaliit-keyboard -L../../view -lmaliit-keyboard-view ../../plugin/libmaliit-keyboard-plugin.so

HEADERS += \
    ../utils.h \

SOURCES += \
    ../utils.cpp \
    main.cpp \

