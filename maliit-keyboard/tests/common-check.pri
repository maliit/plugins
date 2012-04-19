QMAKE_EXTRA_TARGETS += check
check.target = check

# This enables the maliit library for C++ code
CONFIG += maliit-plugins

qws {
    test_arguments += -qws
}

DATA_DIR = $$OUT_PWD/../../data

check.commands = \
    MALIIT_PLUGINS_DATADIR=$$DATA_DIR \
    MALIIT_KEYBOARD_DATADIR=$$DATA_DIR \
    LD_LIBRARY_PATH=$$MALIIT_INSTALL_LIBS:$$[QT_INSTALL_LIBS]:$(LD_LIBRARY_PATH) \
    ./$$TARGET $$test_arguments

check.depends += $$TARGET

LIBS += ../common/$$maliitStaticLib(tests-common)
POST_TARGETDEPS += ../common/$$maliitStaticLib(tests-common)
INCLUDEPATH += ../common
