# Linker optimization for release build
QMAKE_LFLAGS_RELEASE+=-Wl,--as-needed
# Compiler warnings are error if the build type is debug
QMAKE_CXXFLAGS_DEBUG+=-Werror

CONFIG += no_keywords

enable-legacy {
    MALIIT_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir MeegoImFramework)
    MALIIT_PLUGINS_DATA_DIR=$$system(pkg-config --variable pluginsdatadir MeegoImFramework)
} else {
    MALIIT_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir maliit-plugins-0.80)
    MALIIT_PLUGINS_DATA_DIR=$$system(pkg-config --variable pluginsdatadir maliit-plugins-0.80)
}

isEmpty(PREFIX) {
   PREFIX = /usr
}

isEmpty(LIBDIR) {
   LIBDIR = $$PREFIX/lib
}

INSTALL_BIN = $$PREFIX/bin
INSTALL_LIBS = $$LIBDIR
INSTALL_HEADERS = $$PREFIX/include
INSTALL_DOCS = $$PREFIX/share/doc

# Fallback in case framework pkg-config does not have information
# about what plugin data directory should be (framework version < 0.81.1)
isEmpty(MALIIT_PLUGINS_DATA_DIR) {
    MALIIT_PLUGINS_DATA_DIR=$$PREFIX/share/maliit/plugins
}

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
