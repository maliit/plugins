WIDGETS_DIR = ./widgets

INSTALL_HEADERS += \
    $$WIDGETS_DIR/ikeybutton.h \
    $$WIDGETS_DIR/popupbase.h \
    $$WIDGETS_DIR/mvirtualkeyboardstyle.h \
    $$WIDGETS_DIR/keybuttonareastyle.h \
    $$WIDGETS_DIR/popupplugin.h \
    $$WIDGETS_DIR/mimoverlay.h \
    $$WIDGETS_DIR/keybuttonarea.h \
    $$WIDGETS_DIR/singlewidgetbutton.h \
    $$WIDGETS_DIR/singlewidgetbuttonarea.h \

STYLE_HEADERS += \
    $$WIDGETS_DIR/widgetbarstyle.h \
    $$WIDGETS_DIR/mvirtualkeyboardstyle.h \
    $$WIDGETS_DIR/keybuttonareastyle.h \
    $$WIDGETS_DIR/handlestyle.h \
    $$WIDGETS_DIR/mtoolbarbuttonstyle.h \
    $$WIDGETS_DIR/mimtoolbarstyle.h \
    $$WIDGETS_DIR/mimcorrectioncandidatecontainerstyle.h \
    $$WIDGETS_DIR/mimcorrectioncandidateitemstyle.h \
    $$WIDGETS_DIR/mtoolbarlabelstyle.h \

HEADERS += \
    $$INSTALL_HEADERS \
    $$STYLE_HEADERS \
    $$WIDGETS_DIR/widgetbar.h \
    $$WIDGETS_DIR/mimcorrectioncandidatewindow.h \
    $$WIDGETS_DIR/mimcorrectioncandidatewidget.h \
    $$WIDGETS_DIR/mimcorrectioncandidateitem.h \
    $$WIDGETS_DIR/mimcorrectioncandidateitemview.h \
    $$WIDGETS_DIR/mimtoolbar.h \
    $$WIDGETS_DIR/mvirtualkeyboard.h \
    $$WIDGETS_DIR/horizontalswitcher.h \
    $$WIDGETS_DIR/notification.h \
    $$WIDGETS_DIR/symbolview.h \
    $$WIDGETS_DIR/keybuttonarea.h \
    $$WIDGETS_DIR/singlewidgetbutton.h \
    $$WIDGETS_DIR/singlewidgetbuttonarea.h \
    $$WIDGETS_DIR/popupfactory.h \
    $$WIDGETS_DIR/mtoolbarbutton.h \
    $$WIDGETS_DIR/mtoolbarbuttonview.h \
    $$WIDGETS_DIR/mtoolbarlabel.h \
    $$WIDGETS_DIR/handle.h \
    $$WIDGETS_DIR/grip.h \
    $$WIDGETS_DIR/sharedhandlearea.h \
    $$WIDGETS_DIR/mkeyboardsettingswidget.h \
    $$WIDGETS_DIR/getcssproperty.h \
    $$WIDGETS_DIR/mimoverlay.h \
    $$WIDGETS_DIR/mtoolbarlabelview.h \

SOURCES += \
    $$WIDGETS_DIR/widgetbar.cpp \
    $$WIDGETS_DIR/mimcorrectioncandidatewindow.cpp \
    $$WIDGETS_DIR/mimcorrectioncandidatewidget.cpp \
    $$WIDGETS_DIR/mimcorrectioncandidateitem.cpp \
    $$WIDGETS_DIR/mimcorrectioncandidateitemview.cpp \
    $$WIDGETS_DIR/mimtoolbar.cpp \
    $$WIDGETS_DIR/mvirtualkeyboard.cpp \
    $$WIDGETS_DIR/horizontalswitcher.cpp \
    $$WIDGETS_DIR/notification.cpp \
    $$WIDGETS_DIR/symbolview.cpp \
    $$WIDGETS_DIR/keybuttonarea.cpp \
    $$WIDGETS_DIR/singlewidgetbutton.cpp \
    $$WIDGETS_DIR/singlewidgetbuttonarea.cpp \
    $$WIDGETS_DIR/popupbase.cpp \
    $$WIDGETS_DIR/popupfactory.cpp \
    $$WIDGETS_DIR/mtoolbarbutton.cpp \
    $$WIDGETS_DIR/mtoolbarbuttonview.cpp \
    $$WIDGETS_DIR/mtoolbarlabel.cpp \
    $$WIDGETS_DIR/handle.cpp \
    $$WIDGETS_DIR/grip.cpp \
    $$WIDGETS_DIR/sharedhandlearea.cpp \
    $$WIDGETS_DIR/mkeyboardsettingswidget.cpp \
    $$WIDGETS_DIR/mimoverlay.cpp \
    $$WIDGETS_DIR/mtoolbarlabelview.cpp \

INCLUDEPATH += $$WIDGETS_DIR
DEPENDPATH += $$WIDGETS_DIR
