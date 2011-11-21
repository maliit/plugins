# Linker optimization for release build
QMAKE_LFLAGS_RELEASE+=-Wl,--as-needed
# Compiler warnings are error if the build type is debug
QMAKE_CXXFLAGS_DEBUG+=-Werror

enable-legacy {
    MALIIT_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir MeegoImFramework)
} else {
    MALIIT_PLUGINS_DIR=$$system(pkg-config --variable pluginsdir maliit-plugins-0.80)
}
