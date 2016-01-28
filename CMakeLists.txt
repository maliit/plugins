cmake_minimum_required(VERSION 3.1)

project(maliit-plugins VERSION 0.99.1)

option(enable-maliit-keyboard "Build the C++ reference keyboard (Maliit Keyboard)" ON)
option(enable-nemo-keyboard "Build the QML reference keyboard (Nemo Keyboard)" ON)
option(enable-presage "Use presage to calculate word candidates (maliit-keyboard-plugin only)" ON)
option(enable-hunspell "Use hunspell for error correction (maliit-keyboard-plugin only)" ON)
option(enable-preedit "Always commit characters and never use preedit (maliit-keyboard-plugin only)" ON)
option(enable-tests "Build tests" ON)
option(enable-docs "Build documentation" ON)

set(MALIIT_DEFAULT_PROFILE "nokia-n9" CACHE STRING "Default keyboard style")

# Windows compatibility

if (WIN32 )
    if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set (CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/../maliit" CACHE PATH "default install path" FORCE )
    endif()
	
	set (CMAKE_PREFIX_PATH "${CMAKE_INSTALL_PREFIX}" FORCE)
endif()

# Install paths

if(NOT DEFINED INCLUDE_INSTALL_DIR)
    set(INCLUDE_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/include" CACHE PATH
            "Installation directory for C/C++ header files [CMAKE_INSTALL_PREFIX/include]")
endif()

if(NOT DEFINED LIB_INSTALL_DIR)
    set(LIB_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/lib" CACHE PATH
            "Installation directory for object code libraries [CMAKE_INSTALL_PREFIX/lib]")
endif()

if(NOT DEFINED SHARE_INSTALL_PREFIX)
    set(SHARE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}/share" CACHE PATH
            "Installation directory for read-only architecture-independent data [CMAKE_INSTALL_PREFIX/share]")
endif()

set(HUNSPELL_DICT_PATH "/usr/share/myspell" CACHE PATH "Directory containing hunspell dictionary files")
set(MALIIT_PLUGINS_DATA_DIR "${SHARE_INSTALL_PREFIX}/maliit/plugins" CACHE PATH "Directory containing maliit-plugins data")
set(MALIIT_KEYBOARD_DATA_DIR "${MALIIT_PLUGINS_DATA_DIR}/org/maliit" CACHE PATH "Directory containing maliit-keyboard data")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

find_package(MaliitPlugins 0.99.1 REQUIRED)

if(enable-maliit-keyboard)
    set(MALIIT_KEYBOARD_SOURCES
            maliit-keyboard/lib/logic/state-machines/abstractstatemachine.cpp
            maliit-keyboard/lib/logic/state-machines/abstractstatemachine.h
            maliit-keyboard/lib/logic/state-machines/deadkeymachine.cpp
            maliit-keyboard/lib/logic/state-machines/deadkeymachine.h
            maliit-keyboard/lib/logic/state-machines/shiftmachine.cpp
            maliit-keyboard/lib/logic/state-machines/shiftmachine.h
            maliit-keyboard/lib/logic/state-machines/viewmachine.cpp
            maliit-keyboard/lib/logic/state-machines/viewmachine.h
            maliit-keyboard/lib/logic/abstractlanguagefeatures.cpp
            maliit-keyboard/lib/logic/abstractlanguagefeatures.h
            maliit-keyboard/lib/logic/abstracttexteditor.cpp
            maliit-keyboard/lib/logic/abstracttexteditor.h
            maliit-keyboard/lib/logic/abstractwordengine.cpp
            maliit-keyboard/lib/logic/abstractwordengine.h
            maliit-keyboard/lib/logic/eventhandler.cpp
            maliit-keyboard/lib/logic/eventhandler.h
            maliit-keyboard/lib/logic/hitlogic.cpp
            maliit-keyboard/lib/logic/hitlogic.h
            maliit-keyboard/lib/logic/keyareaconverter.cpp
            maliit-keyboard/lib/logic/keyareaconverter.h
            maliit-keyboard/lib/logic/keyboardloader.cpp
            maliit-keyboard/lib/logic/keyboardloader.h
            maliit-keyboard/lib/logic/languagefeatures.cpp
            maliit-keyboard/lib/logic/languagefeatures.h
            maliit-keyboard/lib/logic/layouthelper.cpp
            maliit-keyboard/lib/logic/layouthelper.h
            maliit-keyboard/lib/logic/layoutupdater.cpp
            maliit-keyboard/lib/logic/layoutupdater.h
            maliit-keyboard/lib/logic/spellchecker.cpp
            maliit-keyboard/lib/logic/spellchecker.h
            maliit-keyboard/lib/logic/style.cpp
            maliit-keyboard/lib/logic/style.h
            maliit-keyboard/lib/logic/wordengine.cpp
            maliit-keyboard/lib/logic/wordengine.h
            maliit-keyboard/lib/models/area.cpp
            maliit-keyboard/lib/models/area.h
            maliit-keyboard/lib/models/font.cpp
            maliit-keyboard/lib/models/font.h
            maliit-keyboard/lib/models/key.cpp
            maliit-keyboard/lib/models/key.h
            maliit-keyboard/lib/models/keyarea.cpp
            maliit-keyboard/lib/models/keyarea.h
            maliit-keyboard/lib/models/keyboard.h
            maliit-keyboard/lib/models/keydescription.h
            maliit-keyboard/lib/models/label.cpp
            maliit-keyboard/lib/models/label.h
            maliit-keyboard/lib/models/layout.cpp
            maliit-keyboard/lib/models/layout.h
            maliit-keyboard/lib/models/styleattributes.cpp
            maliit-keyboard/lib/models/styleattributes.h
            maliit-keyboard/lib/models/text.cpp
            maliit-keyboard/lib/models/text.h
            maliit-keyboard/lib/models/wordcandidate.cpp
            maliit-keyboard/lib/models/wordcandidate.h
            maliit-keyboard/lib/models/wordribbon.cpp
            maliit-keyboard/lib/models/wordribbon.h
            maliit-keyboard/lib/parser/alltagtypes.h
            maliit-keyboard/lib/parser/layoutparser.cpp
            maliit-keyboard/lib/parser/layoutparser.h
            maliit-keyboard/lib/parser/tagbinding.cpp
            maliit-keyboard/lib/parser/tagbinding.h
            maliit-keyboard/lib/parser/tagbindingcontainer.cpp
            maliit-keyboard/lib/parser/tagbindingcontainer.h
            maliit-keyboard/lib/parser/tagextended.cpp
            maliit-keyboard/lib/parser/tagextended.h
            maliit-keyboard/lib/parser/tagkey.cpp
            maliit-keyboard/lib/parser/tagkey.h
            maliit-keyboard/lib/parser/tagkeyboard.cpp
            maliit-keyboard/lib/parser/tagkeyboard.h
            maliit-keyboard/lib/parser/taglayout.cpp
            maliit-keyboard/lib/parser/taglayout.h
            maliit-keyboard/lib/parser/tagmodifiers.cpp
            maliit-keyboard/lib/parser/tagmodifiers.h
            maliit-keyboard/lib/parser/tagrow.cpp
            maliit-keyboard/lib/parser/tagrow.h
            maliit-keyboard/lib/parser/tagrowcontainer.cpp
            maliit-keyboard/lib/parser/tagrowcontainer.h
            maliit-keyboard/lib/parser/tagrowelement.cpp
            maliit-keyboard/lib/parser/tagrowelement.h
            maliit-keyboard/lib/parser/tagsection.cpp
            maliit-keyboard/lib/parser/tagsection.h
            maliit-keyboard/lib/parser/tagspacer.cpp
            maliit-keyboard/lib/parser/tagspacer.h
            maliit-keyboard/lib/coreutils.cpp
            maliit-keyboard/lib/coreutils.h)

    set(maliit-keyboard-libraries Qt5::Core)
    set(maliit-keyboard-definitions HUNSPELL_DICT_PATH="${HUNSPELL_DICT_PATH}"
            MALIIT_PLUGINS_DATA_DIR="${MALIIT_PLUGINS_DATA_DIR}"
            MALIIT_KEYBOARD_DATA_DIR="${MALIIT_KEYBOARD_DATA_DIR}")
    set(maliit-keyboard-include-dirs)

    if(enable-presage)
        find_package(Presage REQUIRED)
        if(PRESAGE_FOUND)
            list(APPEND maliit-keyboard-definitions HAVE_PRESAGE)
            list(APPEND maliit-keyboard-libraries ${PRESAGE_LIBRARIES})
            list(APPEND maliit-keyboard-include-dirs ${PRESAGE_INCLUDE_DIRS})
        endif()
    endif()

    if(enable-hunspell)
        find_package(Hunspell REQUIRED)
        list(APPEND maliit-keyboard-definitions HAVE_HUNSPELL)
        list(APPEND maliit-keyboard-libraries ${HUNSPELL_LIBRARIES})
        list(APPEND maliit-keyboard-include-dirs ${HUNSPELL_INCLUDE_DIRS})
    endif()

    if(NOT enable-preedit)
        list(APPEND maliit-keyboard-definitions DISABLE_PREEDIT)
    endif()

    add_library(maliit-keyboard STATIC ${MALIIT_KEYBOARD_SOURCES})
    target_link_libraries(maliit-keyboard ${maliit-keyboard-libraries})
    target_include_directories(maliit-keyboard PUBLIC maliit-keyboard/lib)
    target_include_directories(maliit-keyboard PRIVATE ${maliit-keyboard-include-dirs})
    target_compile_definitions(maliit-keyboard PRIVATE ${maliit-keyboard-definitions})

    set(MALIIT_KEYBOARD_VIEW_SOURCES
            maliit-keyboard/view/abstractfeedback.cpp
            maliit-keyboard/view/abstractfeedback.h
            maliit-keyboard/view/nullfeedback.cpp
            maliit-keyboard/view/nullfeedback.h
            maliit-keyboard/view/soundfeedback.cpp
            maliit-keyboard/view/soundfeedback.h)

    add_library(maliit-keyboard-view STATIC ${MALIIT_KEYBOARD_VIEW_SOURCES})
    target_link_libraries(maliit-keyboard-view Maliit::Plugins maliit-keyboard)

    set(MALIIT_KEYBOARD_EDITOR_SOURCES
            maliit-keyboard/plugin/editor.cpp
            maliit-keyboard/plugin/editor.h
            maliit-keyboard/plugin/updatenotifier.cpp
            maliit-keyboard/plugin/updatenotifier.h)

    add_library(maliit-keyboard-editor STATIC ${MALIIT_KEYBOARD_EDITOR_SOURCES})
    target_link_libraries(maliit-keyboard-editor Maliit::Plugins maliit-keyboard)

    set(MALIIT_KEYBOARD_PLUGIN_SOURCES
            maliit-keyboard/plugin/inputmethod.cpp
            maliit-keyboard/plugin/inputmethod.h
            maliit-keyboard/plugin/maliitcontext.cpp
            maliit-keyboard/plugin/maliitcontext.h
            maliit-keyboard/plugin/plugin.cpp
            maliit-keyboard/plugin/plugin.h)

    add_library(maliit-keyboard-plugin MODULE ${MALIIT_KEYBOARD_PLUGIN_SOURCES})
    target_link_libraries(maliit-keyboard-plugin Maliit::Plugins maliit-keyboard maliit-keyboard-view maliit-keyboard-editor)
    target_include_directories(maliit-keyboard-plugin PRIVATE maliit-keyboard)
    target_compile_definitions(maliit-keyboard-plugin PRIVATE
            MALIIT_PLUGINS_DATA_DIR="${MALIIT_PLUGINS_DATA_DIR}"
            MALIIT_KEYBOARD_DATA_DIR="${MALIIT_KEYBOARD_DATA_DIR}"
            MALIIT_DEFAULT_PROFILE="${MALIIT_DEFAULT_PROFILE}")

    add_executable(maliit-keyboard-benchmark maliit-keyboard/benchmark/main.cpp)
    target_link_libraries(maliit-keyboard-benchmark maliit-keyboard)
endif()

if(enable-docs)
    find_package(Doxygen REQUIRED)

    configure_file(doc/doxyfile.conf.in Doxyfile @ONLY)

    add_custom_target(doc ALL
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM)
endif()

# Installation

install(FILES LICENSE NEWS README
        DESTINATION ${SHARE_INSTALL_PREFIX}/doc/maliit-plugins)

if(enable-maliit-keyboard)
    install(TARGETS maliit-keyboard-benchmark maliit-keyboard-plugin
            RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
            LIBRARY DESTINATION ${LIB_INSTALL_DIR}/maliit/plugins)
    install(DIRECTORY maliit-keyboard/data/languages
            DESTINATION ${MALIIT_PLUGINS_DATA_DIR})
    install(DIRECTORY maliit-keyboard/data/styles
            DESTINATION ${MALIIT_KEYBOARD_DATA_DIR})
    install(FILES maliit-keyboard/qml/Keyboard.qml maliit-keyboard/qml/maliit-keyboard.qml
                  maliit-keyboard/qml/maliit-keyboard-extended.qml maliit-keyboard/qml/maliit-magnifier.qml
            DESTINATION ${MALIIT_KEYBOARD_DATA_DIR})
endif()

if(enable-nemo-keyboard)
    install(FILES nemo-keyboard/nemo-keyboard.qml
            DESTINATION ${LIB_INSTALL_DIR}/maliit/plugins)
    install(DIRECTORY nemo-keyboard/org
            DESTINATION ${MALIIT_PLUGINS_DATA_DIR})
endif()

if(enable-docs)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/html/
            DESTINATION ${SHARE_INSTALL_PREFIX}/doc/maliit-plugins-doc)
endif()

if(enable-tests)
    enable_testing()

    find_package(Qt5Test)

    add_library(test-utils STATIC
            maliit-keyboard/tests/common/inputmethodhostprobe.cpp
            maliit-keyboard/tests/common/inputmethodhostprobe.h
            maliit-keyboard/tests/common/utils-gui.cpp
            maliit-keyboard/tests/common/utils.cpp
            maliit-keyboard/tests/common/utils.h)

    target_link_libraries(test-utils PUBLIC Qt5::Core Qt5::Gui Qt5::Test maliit-keyboard-editor)
    target_include_directories(test-utils INTERFACE maliit-keyboard/tests/common maliit-keyboard/tests maliit-keyboard)

    function(create_test name)
        set(_extra_sources ${ARGV})
        list(REMOVE_AT _extra_sources 0)

        add_executable(${name}
                maliit-keyboard/tests/${name}/${name}.cpp
                ${_extra_sources})

        target_link_libraries(${name} test-utils ${_libs})

        add_test(${name} ${name})

        set(test_targets ${test_targets} ${name} PARENT_SCOPE)
    endfunction()

    create_test(language-layout-loading)
    target_compile_definitions(language-layout-loading PRIVATE
            -DTEST_DATADIR="${CMAKE_SOURCE_DIR}/maliit-keyboard/tests/language-layout-loading"
            -DTEST_MALIIT_KEYBOARD_DATADIR="${CMAKE_SOURCE_DIR}/maliit-keyboard/tests/language-layout-loading")
    create_test(language-layout-switching
            maliit-keyboard/tests/ut_editor/wordengineprobe.cpp
            maliit-keyboard/tests/ut_editor/wordengineprobe.h)
    create_test(repeat-backspace)
    create_test(ut_editor
            maliit-keyboard/tests/ut_editor/wordengineprobe.cpp
            maliit-keyboard/tests/ut_editor/wordengineprobe.h)
    create_test(preedit-string
            maliit-keyboard/tests/preedit-string/wordengineprobe.cpp
            maliit-keyboard/tests/preedit-string/wordengineprobe.h)
    create_test(word-candidates
            maliit-keyboard/tests/word-candidates/wordengineprobe.cpp
            maliit-keyboard/tests/word-candidates/wordengineprobe.h)

    set_property(TEST ${test_targets} PROPERTY ENVIRONMENT
            MALIIT_PLUGINS_DATADIR=${CMAKE_SOURCE_DIR}/maliit-keyboard/data)

endif()