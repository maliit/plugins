# to be included at bottom of .pro files
enable-presage {
    DEFINES += HAVE_PRESAGE
    LIBS += -lpresage
}

enable-hunspell {
    DEFINES += HAVE_HUNSPELL
    LIBS += -lhunspell
}
