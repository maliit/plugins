include(../config.pri)

# Revert linker optimization for release build of the this target.
# It causes problem in the integration.
QMAKE_LFLAGS_RELEASE-=-Wl,--as-needed

TEMPLATE = lib
TARGET = $$qtLibraryTarget(meego-keyboard)

OBJECTS_DIR = .obj
MOC_DIR = .moc
M_MGEN_OUTDIR = .gen

# we have this line temporarily until new libmeegotouch without rpath is integrated
QT += xml

LIBS += -lmeegoimengine
CONFIG += plugin meegotouch meegoimengine meegoimframework

CONFIG += link_pkgconfig
PKGCONFIG += gconf-2.0 xkbfile

# coverage flags are off per default, but can be turned on via qmake COV_OPTION=on
for(OPTION,$$list($$lower($$COV_OPTION))){
    isEqual(OPTION, on){
        QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
        LIBS += -lgcov
        CONFIG -= release
        CONFIG += debug
    }
}

!noreactionmap {
    DEFINES += HAVE_REACTIONMAP
    CONFIG += meegoreactionmap
}

QMAKE_CLEAN += *.gcno *.gcda

include(common/common.pri)
include(widgets/widgets.pri)
include(theme/theme.pri)
include(layouts/layouts.pri)

HEADERS += \
    mkeyboardhost.h \
    mkeyboardsettings.h \
    mkeyboardplugin.h \
    mkeyboardhost_p.h \
    mimlayouttitleparser.h

SOURCES += \
    mkeyboardhost.cpp \
    mkeyboardsettings.cpp \
    mkeyboardplugin.cpp \
    mimlayouttitleparser.cpp

target.path += $$system(pkg-config --variable pluginsdir MeegoImFramework)

install_headers.path = /usr/include/meego-keyboard
install_headers.files = $$INSTALL_HEADERS


INSTALLS += \
    target \
    install_headers \

QMAKE_EXTRA_TARGETS += check-xml
check-xml.depends = lib$${TARGET}.so
check-xml.commands = $$system(true)

QMAKE_EXTRA_TARGETS += check
check.depends = lib$${TARGET}.so
check.commands = $$system(true)
