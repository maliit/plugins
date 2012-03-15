QMAKE_EXTRA_TARGETS += check
check.target = check

# This enables the maliit library for C++ code
CONFIG += maliit-plugins

check.commands = \
    LD_LIBRARY_PATH=$$MALIIT_INSTALL_LIBS:$$[QT_INSTALL_LIBS]:$(LD_LIBRARY_PATH) \
    ./$$TARGET

check.depends += $$TARGET

LIBS += ../common/libtests-common.a
POST_TARGETDEPS += ../common/libtests-common.a
INCLUDEPATH += ../common
