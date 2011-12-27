CONFIG += ordered
TEMPLATE = subdirs
SUBDIRS = \
    language-layout-switching \
    commit-string \

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
