WIDGETS_DIR = ./widgets

PUBLIC_HEADERS += \
    $$WIDGETS_DIR/mimabstractkey.h \
    $$WIDGETS_DIR/mimabstractpopup.h \
    $$WIDGETS_DIR/mimoverlay.h \
    $$WIDGETS_DIR/mimabstractkeyarea.h \
    $$WIDGETS_DIR/mimkeyvisitor.h \
    $$WIDGETS_DIR/mimkey.h \
    $$WIDGETS_DIR/mimkeyarea.h \
    $$WIDGETS_DIR/reactionmappaintable.h \

PUBLIC_STYLE_HEADERS += \
    $$WIDGETS_DIR/mvirtualkeyboardstyle.h \
    $$WIDGETS_DIR/mimabstractkeyareastyle.h \

STYLE_HEADERS += \
    $$PUBLIC_STYLE_HEADERS \
    $$WIDGETS_DIR/widgetbarstyle.h \
    $$WIDGETS_DIR/handlestyle.h \
    $$WIDGETS_DIR/mtoolbarbuttonstyle.h \
    $$WIDGETS_DIR/mimtoolbarstyle.h \
    $$WIDGETS_DIR/mimwordtrackerstyle.h \
    $$WIDGETS_DIR/mimcorrectioncandidateitemstyle.h \
    $$WIDGETS_DIR/mtoolbarlabelstyle.h \
    $$WIDGETS_DIR/wordribbonitemstyle.h \
    $$WIDGETS_DIR/wordribbonstyle.h \
    $$WIDGETS_DIR/wordribbondialogstyle.h \
    $$WIDGETS_DIR/mkeyboardmagnifierstyle.h \
    $$WIDGETS_DIR/notificationstyle.h \
    $$WIDGETS_DIR/keyboardshadowstyle.h \

HEADERS += \
    $$PUBLIC_HEADERS \
    $$STYLE_HEADERS \
    $$WIDGETS_DIR/widgetbar.h \
    $$WIDGETS_DIR/mimcorrectionhost.h \
    $$WIDGETS_DIR/mimcorrectioncandidateitem.h \
    $$WIDGETS_DIR/mimwordtracker.h \
    $$WIDGETS_DIR/mimwordlist.h \
    $$WIDGETS_DIR/mimwordlistitem.h \
    $$WIDGETS_DIR/mimtoolbar.h \
    $$WIDGETS_DIR/mvirtualkeyboard.h \
    $$WIDGETS_DIR/horizontalswitcher.h \
    $$WIDGETS_DIR/notification.h \
    $$WIDGETS_DIR/symbolview.h \
    $$WIDGETS_DIR/mimkey.h \
    $$WIDGETS_DIR/mtoolbarbutton.h \
    $$WIDGETS_DIR/mtoolbarbuttonview.h \
    $$WIDGETS_DIR/mtoolbarlabel.h \
    $$WIDGETS_DIR/handle.h \
    $$WIDGETS_DIR/keyboardshadow.h \
    $$WIDGETS_DIR/grip.h \
    $$WIDGETS_DIR/sharedhandlearea.h \
    $$WIDGETS_DIR/mkeyboardsettingswidget.h \
    $$WIDGETS_DIR/mkeyboardsettingslistitem.h \
    $$WIDGETS_DIR/getcssproperty.h \
    $$WIDGETS_DIR/mtoolbarlabelview.h \
    $$WIDGETS_DIR/mplainwindow.h \
    $$WIDGETS_DIR/mimabstractkeyarea_p.h \
    $$WIDGETS_DIR/mimkeyarea_p.h \
    $$WIDGETS_DIR/abstractenginewidgethost.h \
    $$WIDGETS_DIR/wordribbonitem.h \
    $$WIDGETS_DIR/wordribbonhost.h \
    $$WIDGETS_DIR/wordribbon.h \
    $$WIDGETS_DIR/wordribbondialog.h \
    $$WIDGETS_DIR/wordribbondialogmodel.h \
    $$WIDGETS_DIR/wordribbondialogview.h \
    $$WIDGETS_DIR/magnifierhost.h \
    $$WIDGETS_DIR/magnifier.h \
    $$WIDGETS_DIR/extendedkeys.h \
    $$WIDGETS_DIR/mwidget-wrapper.h \
    $$WIDGETS_DIR/mimrootwidget.h \

INSTALL_HEADERS += $$PUBLIC_HEADERS + $$PUBLIC_STYLE_HEADERS

SOURCES += \
    $$WIDGETS_DIR/widgetbar.cpp \
    $$WIDGETS_DIR/mimcorrectionhost.cpp \
    $$WIDGETS_DIR/mimcorrectioncandidateitem.cpp \
    $$WIDGETS_DIR/mimwordtracker.cpp \
    $$WIDGETS_DIR/mimwordlist.cpp \
    $$WIDGETS_DIR/mimwordlistitem.cpp \
    $$WIDGETS_DIR/mimtoolbar.cpp \
    $$WIDGETS_DIR/mvirtualkeyboard.cpp \
    $$WIDGETS_DIR/horizontalswitcher.cpp \
    $$WIDGETS_DIR/notification.cpp \
    $$WIDGETS_DIR/symbolview.cpp \
    $$WIDGETS_DIR/mimabstractkey.cpp \
    $$WIDGETS_DIR/mimabstractkeyarea.cpp \
    $$WIDGETS_DIR/mimkeyvisitor.cpp \
    $$WIDGETS_DIR/mimkey.cpp \
    $$WIDGETS_DIR/mimkeyarea.cpp \
    $$WIDGETS_DIR/mplainwindow.cpp \
    $$WIDGETS_DIR/mtoolbarbutton.cpp \
    $$WIDGETS_DIR/mtoolbarbuttonview.cpp \
    $$WIDGETS_DIR/mtoolbarlabel.cpp \
    $$WIDGETS_DIR/handle.cpp \
    $$WIDGETS_DIR/keyboardshadow.cpp \
    $$WIDGETS_DIR/grip.cpp \
    $$WIDGETS_DIR/sharedhandlearea.cpp \
    $$WIDGETS_DIR/mkeyboardsettingswidget.cpp \
    $$WIDGETS_DIR/mkeyboardsettingslistitem.cpp \
    $$WIDGETS_DIR/mimoverlay.cpp \
    $$WIDGETS_DIR/mtoolbarlabelview.cpp \
    $$WIDGETS_DIR/reactionmappaintable.cpp \
    $$WIDGETS_DIR/wordribbonitem.cpp \
    $$WIDGETS_DIR/wordribbonhost.cpp \
    $$WIDGETS_DIR/wordribbon.cpp \
    $$WIDGETS_DIR/wordribbondialog.cpp \
    $$WIDGETS_DIR/wordribbondialogmodel.cpp \
    $$WIDGETS_DIR/wordribbondialogview.cpp \
    $$WIDGETS_DIR/magnifierhost.cpp \
    $$WIDGETS_DIR/magnifier.cpp \
    $$WIDGETS_DIR/extendedkeys.cpp \
    $$WIDGETS_DIR/mwidget-wrapper.cpp \
    $$WIDGETS_DIR/mimrootwidget.cpp \

INCLUDEPATH += $$WIDGETS_DIR
DEPENDPATH += $$WIDGETS_DIR
