CONFIG += ordered
TEMPLATE = subdirs
SUBDIRS = \
          ut_widgetbar \
          ut_layoutdata \
          ut_mvirtualkeyboard \
          ut_mkeyboardhost \
          ut_mkeyboardplugin \
          ut_mimcorrectionhost \
          ut_mimwordlist \
          ut_mimwordtracker \
          ut_mimkey \
          ut_mimabstractkeyarea \
          ut_flickrecognizer \
          ut_symbolview \
          ut_notification \
          ut_mimkeymodel \
          ut_horizontalswitcher \
          ut_layoutsmanager \
          ut_mimtoolbar \
          ut_mhardwarekeyboard \
          ut_hwkbcharloopsmanager \
          ut_mkeyboardsettings \
          ut_mkeyboardsettingswidget \
          bm_mimabstractkeyarea \
          bm_symbols \
          ft_mxkb \
          ut_sharedhandlearea \
          bm_painting \
          ut_mtoolbarbutton \
          ut_wordribbonitem \
          ut_wordribbonhost \
          ut_wordribbondialog \
          ut_wordribbondialogmodel \
          ut_mimfontpool \
          ut_magnifier \

include (test-layouts/test-layouts.pri)

target.commands += $$system(touch tests.xml)
target.path = /usr/share/meego-keyboard-tests
target.files += qtestlib2junitxml.xsl runtests.sh tests.xml
INSTALLS += target

QMAKE_EXTRA_TARGETS += check-xml
check-xml.target = check-xml
check-xml.CONFIG = recursive

QMAKE_EXTRA_TARGETS += check
check.target = check
check.CONFIG = recursive

