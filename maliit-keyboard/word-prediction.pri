# to be included at bottom of .pro files
enable-presage {
    DEFINES += HAVE_PRESAGE
    LIBS += -lpresage
}

enable-hunspell {
    CONFIG += link_pkgconfig
    PKGCONFIG += hunspell
    DEFINES += HAVE_HUNSPELL
}

disable-preedit|!enable-presage:!enable-hunspell {
    DEFINES += DISABLE_PREEDIT
}
