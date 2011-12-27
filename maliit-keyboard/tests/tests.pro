CONFIG += ordered
TEMPLATE = subdirs
SUBDIRS = \
    language-layout-switching \

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
