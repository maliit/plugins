include(../config.pri)

TEMPLATE = lib
CONFIG += plugin
TARGET = dummy

DOXYFILE = doxyfile.conf.in

doc.name = doc
doc.CONFIG += target_predeps no_link
doc.output = html/index.html
doc.clean_commands = rm -rf html
doc.clean = ${QMAKE_FILE_IN_BASE}
doc.input = DOXYFILE
doc.commands += sed -e \"s;@VERSION@;$${MALIIT_VERSION};g\" -e \"s;@IN_PWD@;$${IN_PWD};g\" ${QMAKE_FILE_IN} > $${OUT_PWD}/${QMAKE_FILE_IN_BASE} &&
doc.commands += doxygen $${OUT_PWD}/${QMAKE_FILE_IN_BASE}
QMAKE_EXTRA_COMPILERS += doc

htmldocs.files = html
htmldocs.path = $$INSTALL_DOCS/$$MALIIT_PACKAGENAME
htmldocs.CONFIG += no_check_exist directory
INSTALLS += htmldocs

OTHER_FILES += $$DOXYFILE
