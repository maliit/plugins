include(../config.pri)

VERSION = 0.7.0
TARGET = meego-keyboard
TEMPLATE = lib
QT = core gui xml

# Revert linker optimization for release build of the this target.
# It causes problem in the integration.
QMAKE_LFLAGS_RELEASE-=-Wl,--as-needed

!nomeegotouch {
    MEEGO_IM_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir MeegoImFramework)
} else {
    MEEGO_IM_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir maliit-plugins-0.80)
}

LIBS += -lmeegoimengine
CONFIG += \
    plugin \
    meegoimengine \

# allows us to use hidden Maliit FW API, such as MImSetting:
DEFINES += MALIIT_FRAMEWORK_USE_INTERNAL_API

!nomeegotouch {
    CONFIG += meegoimframework meegotouch
    DEFINES += HAVE_MEEGOTOUCH
} else {
    PKGCONFIG += maliit-plugins-0.80
    # moc needs the include path
    INCLUDEPATH += $$system(pkg-config --cflags maliit-plugins-0.80 | tr \' \' \'\\n\' | grep ^-I | cut -d I -f 2-)
    DEFINES += HAVE_MALIIT
}

!noreactionmap {
    DEFINES += HAVE_REACTIONMAP
    CONFIG += meegoreactionmap
}

CONFIG += link_pkgconfig
PKGCONFIG += gconf-2.0 xkbfile

OBJECTS_DIR = .obj
MOC_DIR = .moc
M_MGEN_OUTDIR = .gen


# coverage flags are off per default, but can be turned on via qmake COV_OPTION=on
for(OPTION,$$list($$lower($$COV_OPTION))){
    isEqual(OPTION, on){
        QMAKE_CXXFLAGS += -ftest-coverage -fprofile-arcs -fno-elide-constructors
        LIBS += -lgcov
        CONFIG -= release
        CONFIG += debug
    }
}

QMAKE_CLEAN += *.gcno *.gcda

include(common/common.pri)
include(widgets/widgets.pri)
include(theme/theme.pri)
include(layouts/layouts.pri)
include(style/style.pri)

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

QMAKE_EXTRA_TARGETS += check-xml
check-xml.depends = lib$${TARGET}.so
check-xml.commands = $$system(true)

QMAKE_EXTRA_TARGETS += check
check.depends = lib$${TARGET}.so
check.commands = $$system(true)

target.path += $${MEEGO_IM_PLUGINS_DIR}

INSTALLS += target
