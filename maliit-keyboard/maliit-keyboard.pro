CONFIG += ordered 
TEMPLATE = subdirs
SUBDIRS = \
    lib \
    view \
    plugin \
    viewer \
    data \
    benchmark \
    tests \

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
