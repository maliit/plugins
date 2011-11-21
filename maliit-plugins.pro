include(config.pri)

!isEmpty(HELP) {
    # Output help
    help_string = \
        Important build options: \
        \\n\\t \
        \\nRecognised CONFIG flags: \
        \\n\\t enable-legacy : Build for legacy mode (using meego-im ABI/API compatabible maliit-framework) \
        \\nInfluential environment variables: \
        \\n\\t PKG_CONFIG_PATH: Specify the pkg-config files to use for dependencies \
        \\nExamples: \
        \\n\\t qmake \
        \\n\\t qmake CONFIG+=enable-legacy \

    !build_pass:system(echo \"$$help_string\")
} else {
    config_string = Tip: Run qmake HELP=1 for a list of all supported build options

    !build_pass:system(echo \"$$config_string\")
}

CONFIG += ordered 
TARGET = meego-im-plugins
TEMPLATE = subdirs
SUBDIRS = \
    meego-keyboard-quick \

