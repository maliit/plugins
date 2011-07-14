include(../common_top.pri)

TEMPLATE = app
CONFIG += QtTest meegotouch
DEPENDPATH += .
INCLUDEPATH += .

include(../common_check.pri)

STYLE_HEADERS += $$WIDGETS_DIR/mimabstractkeyareastyle.h

# Input
HEADERS += ut_mimkey.h \
           $$WIDGETS_DIR/mimabstractkey.h \
           $$WIDGETS_DIR/mimkey.h \
           $$WIDGETS_DIR/mplainwindow.h \
           $$COMMON_DIR/mimkeymodel.h \
           $$COMMON_DIR/keyevent.h \
           $$COMMON_DIR/mimfontpool.h \
           $$STYLE_DIR/mimkeystyle.h \
           $$STYLE_HEADERS

SOURCES += ut_mimkey.cpp \
           $$WIDGETS_DIR/mimabstractkey.cpp \
           $$WIDGETS_DIR/mimkey.cpp \
           $$WIDGETS_DIR/mplainwindow.cpp \
           $$COMMON_DIR/mimkeymodel.cpp \
           $$COMMON_DIR/keyevent.cpp \
           $$COMMON_DIR/mimfontpool.cpp \
           $$STYLE_DIR/mimkeystyle.cpp \

