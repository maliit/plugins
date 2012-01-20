QMAKE_EXTRA_TARGETS += check
check.target = check

framework_libdirs = $$system(pkg-config --libs-only-L maliit-plugins-0.80 | tr \' \' \'\n\' | grep ^-L | cut -d L -f 2- | tr \'\n\' \':\')
# Note: already contains : delimiters, including one at the end

check.commands = \
    LD_LIBRARY_PATH=$$framework_libdirs$(LD_LIBRARY_PATH) \
    ./$$TARGET

check.depends += $$TARGET
