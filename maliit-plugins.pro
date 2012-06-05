include(config.pri)

!isEmpty(HELP) {
    # Output help
    help_string = \
        Important build options: \
        \\n\\t PREFIX: Install prefix for tools. Default: Maliit framework prefix \
        \\n\\t\\t Note that the plugins themselves are installed under Maliit framework prefix \
        \\n\\t LIBDIR: Library install directory. Default: $$PREFIX/lib \
        \\nRecognised CONFIG flags: \
        \\n\\t enable-presage: Use presage to calculate word candidates (maliit-keyboard-plugin only) \
        \\n\\t enable-hunspell: Use hunspell for error correction (maliit-keyboard-plugin only) \
        \\n\\t enable-qt-mobility: Enable use of QtMobility (enables sound and haptic feedback) \
        \\n\\t notests: Do not attempt to build tests \
        \\n\\t nodoc : Do not build documentation \
        \\nInfluential environment variables: \
        \\n\\t QMAKEFEATURES A mkspecs/features directory list to look for features. \
        \\n\\t\\t Use it if a dependency is installed to non-default location. \
        \\n\\t\\t Note that it must also be passed to qmake calls in subdirectories. \
        \\nExamples: \
        \\n\\t qmake \
        \\n\\t qmake PREFIX=/usr LIBDIR=/usr/lib64 \

    !build_pass:system(echo -e \"$$help_string\")
} else {
    config_string = Tip: Run qmake HELP=1 for a list of all supported build options

    !build_pass:system(echo -e \"$$config_string\")
}

CONFIG += ordered 
TEMPLATE = subdirs
SUBDIRS = \
    nemo-keyboard \
    maliit-keyboard \

!nodoc:SUBDIRS += doc

DIST_NAME = $$MALIIT_PACKAGENAME-$$MALIIT_VERSION
DIST_PATH = $$OUT_PWD/$$DIST_NAME
TARBALL_SUFFIX = .tar.bz2
TARBALL_PATH = $$DIST_PATH$$TARBALL_SUFFIX

# The 'make dist' target
# Creates a tarball
QMAKE_EXTRA_TARGETS += dist
dist.target = dist
dist.commands += git archive HEAD --prefix=$$DIST_NAME/ | bzip2 > $$TARBALL_PATH;
dist.commands += md5sum $$TARBALL_PATH | cut -d \' \' -f 1 > $$DIST_PATH\\.md5
