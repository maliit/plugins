# Linker optimization for release build
QMAKE_LFLAGS_RELEASE+=-Wl,--as-needed
# Compiler warnings are error if the build type is debug
QMAKE_CXXFLAGS_DEBUG+=-Werror

enable-legacy {
    MALIIT_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir MeegoImFramework)
    MALIIT_PLUGINS_DATA_DIR=$$system(pkg-config --variable pluginsdatadir MeegoImFramework)
} else {
    MALIIT_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir maliit-plugins-0.80)
    MALIIT_PLUGINS_DATA_DIR=$$system(pkg-config --variable pluginsdatadir maliit-plugins-0.80)
}

MALIIT_PACKAGENAME = maliit-plugins
MALIIT_VERSION = $$system(cat $$IN_PWD/VERSION)
