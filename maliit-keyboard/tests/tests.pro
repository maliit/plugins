TEMPLATE = subdirs
SUBDIRS = \
    common \
    language-layout-switching \
    preedit-string \
    repeat-backspace \
    rendering \

language-layout-switching.depends = common
preedit-string.depends = common
repeat-backspace.depends = common

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive
