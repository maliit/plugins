# Linker optimization for release build
QMAKE_LFLAGS_RELEASE+=-Wl,--as-needed
# Compiler warnings are error if the build type is debug
QMAKE_CXXFLAGS_DEBUG+=-Werror -O0

CONFIG += no_keywords

# The feature maliit-defines initializes some variables related for MALIIT, such as installation paths
# here have to load it early, to start using the defines immediately
!load(maliit-defines) {
   error(Cannot find $$[QT_INSTALL_DATA]/mkspecs/features/maliit-defines.prf. Probably Maliit framework not installed)
}
# This enables the maliit library for C++ code
CONFIG += maliit-plugins

isEmpty(PREFIX) {
   PREFIX = $$MALIIT_PREFIX
}

isEmpty(LIBDIR) {
   LIBDIR = $$PREFIX/lib
}

isEmpty(MALIIT_DEFAULT_PROFILE) {
   MALIIT_DEFAULT_PROFILE = nokia-n9
}

contains(QT_CONFIG, embedded) {
    CONFIG += qws
}

INSTALL_BIN = $$PREFIX/bin
INSTALL_LIBS = $$LIBDIR
INSTALL_HEADERS = $$PREFIX/include
INSTALL_DOCS = $$PREFIX/share/doc

enable-opengl {
    QT += opengl
    DEFINES += MALIIT_KEYBOARD_HAVE_OPENGL
}

enable-coverage {
    LIBS += -lgcov
    QMAKE_CXXFLAGS += -g -fprofile-arcs -ftest-coverage -O0
    QMAKE_LDFLAGS += -g -fprofile-arcs -ftest-coverage  -O0
}

MALIIT_PACKAGENAME = maliit-plugins
MALIIT_VERSION = $$system(cat $$IN_PWD/VERSION)
