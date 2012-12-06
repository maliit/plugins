PARSER_DIR = ./parser

HEADERS += \
    parser/layoutparser.h \
    parser/tagbinding.h \
    parser/tagkeyboard.h \
    parser/tagkey.h \
    parser/taglayout.h \
    parser/tagrowelement.h \
    parser/tagrow.h \
    parser/tagsection.h \
    parser/tagspacer.h

SOURCES += \
    parser/layoutparser.cpp \
    parser/tagbinding.cpp \
    parser/tagkeyboard.cpp \
    parser/tagkey.cpp \
    parser/taglayout.cpp \
    parser/tagrow.cpp \
    parser/tagrowelement.cpp \
    parser/tagsection.cpp \
    parser/tagspacer.cpp

DEPENDPATH += $$PARSER_DIR
