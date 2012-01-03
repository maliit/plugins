include(../../config.pri)

TEMPLATE = app
TARGET = maliit-keyboard-viewer
target.path = $$INSTALL_BIN

INCLUDEPATH += ../lib ../
LIBS += -L../lib -lmaliit-keyboard -L../view -lmaliit-keyboard-view
SOURCES += main.cpp

contains(QT_MAJOR_VERSION, 4) {
    QT = core gui
} else {
    QT = core gui widgets
}

CONFIG += link_pkgconfig
PKGCONFIG += maliit-plugins-0.80
# moc needs the include path
INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-0.80 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)

INSTALLS += target
