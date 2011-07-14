COMMON_DIR = ./common

INSTALL_HEADERS += \
    $$COMMON_DIR/mimkeymodel.h \
    $$COMMON_DIR/keyevent.h \
    $$COMMON_DIR/layoutdata.h \
    $$COMMON_DIR/mkeyboardcommon.h \
    $$COMMON_DIR/reactionmappainter.h \
    $$COMMON_DIR/regiontracker.h \
    $$COMMON_DIR/mimfontpool.h \

HEADERS += \
    $$INSTALL_HEADERS \
    $$COMMON_DIR/reactionmappainter_p.h \
    $$COMMON_DIR/regiontracker_p.h \
    $$COMMON_DIR/keyboarddata.h \
    $$COMMON_DIR/layoutsmanager.h \
    $$COMMON_DIR/mxkb.h \
    $$COMMON_DIR/mhardwarekeyboard.h \
    $$COMMON_DIR/hwkbcharloops.h \
    $$COMMON_DIR/hwkbcharloopsmanager.h \
    $$COMMON_DIR/hwkbdeadkeymapper.h \
    $$COMMON_DIR/keyeventhandler.h \
    $$COMMON_DIR/flickgesture.h \
    $$COMMON_DIR/flickgesturerecognizer.h \
    $$COMMON_DIR/keyboardmapping.h \
    $$COMMON_DIR/simplefilelog.h \
    $$COMMON_DIR/mimreactionmap.h \
    $$COMMON_DIR/reactionmapwrapper.h \
    $$COMMON_DIR/abstractengine.h \
    $$COMMON_DIR/enginedefault.h \
    $$COMMON_DIR/enginemanager.h \
    $$COMMON_DIR/enginehandler.h \
    $$COMMON_DIR/cjklogicstatemachine.h \
    $$COMMON_DIR/enginecjk.h \
    $$COMMON_DIR/namespace.h \

SOURCES += \
    $$COMMON_DIR/keyboarddata.cpp \
    $$COMMON_DIR/keyevent.cpp \
    $$COMMON_DIR/layoutdata.cpp \
    $$COMMON_DIR/layoutsmanager.cpp \
    $$COMMON_DIR/mimkeymodel.cpp \
    $$COMMON_DIR/mxkb.cpp \
    $$COMMON_DIR/mhardwarekeyboard.cpp \
    $$COMMON_DIR/hwkbcharloops.cpp \
    $$COMMON_DIR/hwkbcharloopsmanager.cpp \
    $$COMMON_DIR/hwkbdeadkeymapper.cpp \
    $$COMMON_DIR/keyeventhandler.cpp \
    $$COMMON_DIR/flickgesture.cpp \
    $$COMMON_DIR/flickgesturerecognizer.cpp \
    $$COMMON_DIR/keyboardmapping.cpp \
    $$COMMON_DIR/reactionmappainter.cpp \
    $$COMMON_DIR/regiontracker.cpp \
    $$COMMON_DIR/simplefilelog.cpp \
    $$COMMON_DIR/enginedefault.cpp \
    $$COMMON_DIR/enginemanager.cpp \
    $$COMMON_DIR/cjklogicstatemachine.cpp \
    $$COMMON_DIR/enginecjk.cpp \
    $$COMMON_DIR/mimfontpool.cpp \

INCLUDEPATH += $$COMMON_DIR
DEPENDPATH += $$COMMON_DIR
