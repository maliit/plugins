TEMPLATE = subdirs
SUBDIRS = \
    common \
    language-layout-switching \
    commit-string \

language-layout-switching.depends = common
commit-string.depends = common

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
