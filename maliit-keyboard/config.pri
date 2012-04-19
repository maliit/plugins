include(../config.pri)

MALIIT_KEYBOARD_DATA_DIR = "$${MALIIT_PLUGINS_DATA_DIR}/org/maliit"

DEFINES += MALIIT_PLUGINS_DATA_DIR=\\\"$${MALIIT_PLUGINS_DATA_DIR}\\\"
DEFINES += MALIIT_KEYBOARD_DATA_DIR=\\\"$${MALIIT_KEYBOARD_DATA_DIR}\\\"

unix {
    MALIIT_STATIC_PREFIX=lib
    MALIIT_STATIC_SUFFIX=.a
    MALIIT_DYNAMIC_PREFIX=lib
    MALIIT_DYNAMIC_SUFFIX=.so
}

win32 {
    # qmake puts libraries in subfolders on Windows
    release {
        MALIIT_STATIC_PREFIX=release/lib
        MALIIT_DYNAMIC_PREFIX=release/
    }
    debug {
        MALIIT_STATIC_PREFIX=debug/lib
        MALIIT_DYNAMIC_PREFIX=debug/
    }

    MALIIT_STATIC_SUFFIX=.a
    MALIIT_DYNAMIC_SUFFIX=.dll
}

defineReplace(maliitStaticLib) {
    return($${MALIIT_STATIC_PREFIX}$${1}$${MALIIT_STATIC_SUFFIX})
}

defineReplace(maliitDynamicLib) {
    return($${MALIIT_DYNAMIC_PREFIX}$${1}$${MALIIT_DYNAMIC_SUFFIX})
}

MALIIT_KEYBOARD_TARGET = maliit-keyboard
MALIIT_KEYBOARD_VIEW_TARGET = maliit-keyboard-view
MALIIT_KEYBOARD_PLUGIN_TARGET = maliit-keyboard-plugin

MALIIT_KEYBOARD_LIB = maliit-keyboard/lib/$$maliitStaticLib($${MALIIT_KEYBOARD_TARGET})
MALIIT_KEYBOARD_VIEW_LIB = maliit-keyboard/view/$$maliitStaticLib($${MALIIT_KEYBOARD_VIEW_TARGET})
MALIIT_KEYBOARD_PLUGIN_LIB = maliit-keyboard/plugin/$$maliitDynamicLib($${MALIIT_KEYBOARD_PLUGIN_TARGET})
