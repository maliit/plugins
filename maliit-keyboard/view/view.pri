# config.pri has to be included before this file.

!disable-qt-mobility {
    CONFIG += mobility
    MOBILITY += feedback
    DEFINES += HAVE_QT_MOBILITY
}
