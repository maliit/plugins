TARGET = tests-common
TEMPLATE = lib
CONFIG += staticlib

SOURCES += \
           utils.cpp \
           utils-gui.cpp \
           inputmethodhostprobe.cpp \

HEADERS += \
           utils.h \
           inputmethodhostprobe.h \

OBJECTS_DIR = .obj
MOC_DIR = .moc

QMAKE_EXTRA_TARGETS += check
check.target = check
check.command = $$system(true)
check.depends += libtests-common.a

enable-legacy {
    CONFIG += meegoimframework
} else {
    CONFIG += link_pkgconfig
    PKGCONFIG += maliit-plugins-0.80
    # moc needs the include path
    INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-0.80 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)
}
