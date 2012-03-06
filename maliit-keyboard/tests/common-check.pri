QMAKE_EXTRA_TARGETS += check
check.target = check

enable-legacy {
    framework_pc = MeegoImFramework
} else {
    framework_pc = maliit-plugins-0.80
}

framework_libdirs = $$system(pkg-config --libs-only-L $$framework_pc | tr \' \' \'\\n\' | grep ^-L | cut -d L -f 2- | tr \'\\n\' \':\')
# Note: already contains : delimiters, including one at the end

qws {
    unittest_arguments += -qws
}

check.commands = \
    LD_LIBRARY_PATH=$$framework_libdirs$$[QT_INSTALL_LIBS]:$(LD_LIBRARY_PATH) \
    ./$$TARGET $$unittest_arguments

check.depends += $$TARGET

LIBS += ../common/libtests-common.a
POST_TARGETDEPS += ../common/libtests-common.a
INCLUDEPATH += ../common
