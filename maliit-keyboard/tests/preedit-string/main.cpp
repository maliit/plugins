/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Copyright (C) 2012-2013 Canonical Ltd
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this list
 * of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * Neither the name of Nokia Corporation nor the names of its contributors may be
 * used to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 * THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "utils.h"

#include "models/area.h"
#include "models/key.h"
#include "models/keyarea.h"
#include "models/text.h"
#include "logic/languagefeatures.h"
#include "logic/layout.h"
#include "view/glass.h"
#include "view/setup.h"
#include "plugin/editor.h"
#include "plugin/updatenotifier.h"
#include "inputmethodhostprobe.h"
#include "wordengineprobe.h"
#include "view/surface.h"

#include <maliit/plugins/updateevent.h>
#include <maliit/namespace.h>

#include <QtTest>
#include <QtCore>
#include <QtGui>

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
#include <QtWidgets>
#endif

using namespace MaliitKeyboard;
Q_DECLARE_METATYPE(Logic::Layout::Orientation)
Q_DECLARE_METATYPE(QList<QMouseEvent*>)

namespace {
const int g_size = 48;
const int g_divider = 3;

QPoint keyOriginLookup(const QString &name)
{
    static const int distance = g_size / g_divider;

    if (name == "a") {
        return QPoint(0, 0);
    } else if (name == "b") {
        return QPoint(distance, 0);
    } else if (name == "c") {
        return QPoint(0, distance);
    } else if (name == "d") {
        return QPoint(distance, distance);
    } else if (name == "space") {
        return QPoint(distance * 2, 0);
    } else if (name == "return") {
        return QPoint(distance * 2, distance);
    }

    return QPoint();
}

Key createKey(Key::Action action,
              const QString &text)
{
    static const QSize size(g_size / g_divider, g_size / g_divider);

    Key result;
    result.setAction(action);
    result.setOrigin(keyOriginLookup(text));
    result.rArea().setSize(size);
    result.rLabel().setText(text);

    return result;
}

// Populate KeyArea with six keys, a, b, c, d, space and return. Notice how the KeyArea
// covers the whole widget. Key width and height equals g_size / g_divider.
// .-----------.
// | a | b |<s>|
// |---|---|---|
// | c | d |<r>|
// `-----------'
KeyArea createAbcdArea()
{
    KeyArea key_area;
    Area area;
    area.setSize(QSize(g_size, g_size));
    key_area.setArea(area);

    key_area.rKeys().append(createKey(Key::ActionInsert, "a"));
    key_area.rKeys().append(createKey(Key::ActionInsert, "b"));
    key_area.rKeys().append(createKey(Key::ActionInsert, "c"));
    key_area.rKeys().append(createKey(Key::ActionInsert, "d"));
    key_area.rKeys().append(createKey(Key::ActionSpace,  "space"));
    key_area.rKeys().append(createKey(Key::ActionReturn, "return"));

    return key_area;
}

QList<QMouseEvent *> createPressReleaseEvent(const QPoint &origin,
                                             Logic::Layout::Orientation orientation = Logic::Layout::Landscape)
{
    static const int offset(g_size / (g_divider * 2));

    QList<QMouseEvent *> result;
    QPoint pos = (orientation == Logic::Layout::Landscape)
                  ? QPoint(origin.x() + offset, origin.y() + offset)
                  : QPoint(origin.y() + offset, g_size - (origin.x() + offset));

    result.append(new QMouseEvent(QKeyEvent::MouseButtonPress, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));
    result.append(new QMouseEvent(QKeyEvent::MouseButtonRelease, pos, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier));

    return result;
}

bool operator==(const Maliit::PreeditTextFormat &a, const Maliit::PreeditTextFormat &b) {
    return ((a.start == b.start) and (a.length == b.length) and (a.preeditFace == b.preeditFace));
}

MImUpdateEvent *createUpdateEvent(const QString &surrounding_text,
                                  int cursor_position)
{
    const char *const cur_pos("cursorPosition");
    QStringList properties_changed(cur_pos);
    QMap<QString, QVariant> update;

    update.insert(cur_pos, cursor_position);
    update.insert("surroundingText", surrounding_text);

    return new MImUpdateEvent(update, properties_changed);
}

} // unnamed namespace

struct BasicSetupTest
{
    BasicSetupTest(bool enable_word_engine = true)
        : editor(EditorOptions(), new Model::Text, new Logic::WordEngineProbe, new Logic::LanguageFeatures, 0)
        , host()
        , notifier()
    {
        editor.setHost(&host);
        editor.wordEngine()->setEnabled(enable_word_engine);

        QObject::connect(&notifier, SIGNAL(cursorPositionChanged(int, QString)),
                         &editor,   SLOT(onCursorPositionChanged(int, QString)));

    }

    Editor editor;
    InputMethodHostProbe host;
    UpdateNotifier notifier;
};

struct SetupTest : public BasicSetupTest
{
    SetupTest(Logic::Layout::Orientation orientation = Logic::Layout::Landscape,
              bool enable_word_engine = true)
        : BasicSetupTest(enable_word_engine)
        , glass()
        , layout(new Logic::Layout)
        , surface(new Surface(0, Maliit::PositionCenterBottom, &host))
        , extended_surface(new Surface(surface.data(), Maliit::PositionOverlay, &host))
        , key_area(createAbcdArea())
    {
        // geometry stuff is usually done by maliit-server, so we need
        // to do it manually here:
        //surface->viewport()->setGeometry(0, 0, g_size, g_size);
        surface->setSceneRect(0, 0, g_size, g_size);
        surface->scene()->setSceneRect(0, 0, g_size, g_size);
        glass.setSurface(surface.data());
        glass.setExtendedSurface(extended_surface.data());
        glass.addLayout(&layout);
        layout.setOrientation(orientation);

        Setup::connectGlassToTextEditor(&glass, &editor);

        layout.setExtendedPanel(key_area);
        layout.setActivePanel(Logic::Layout::ExtendedPanel);
    }

    Glass glass;
    Logic::Layout layout;
    QScopedPointer<Surface> surface;
    QScopedPointer<Surface> extended_surface;
    KeyArea key_area;
};

class TestPreeditString
    : public QObject
{
    Q_OBJECT

private:
    typedef QList<Maliit::PreeditTextFormat> FormatList;

    Q_SLOT void initTestCase()
    {
        qRegisterMetaType<QList<QMouseEvent*> >();
        qRegisterMetaType<Logic::Layout::Orientation>();
        qRegisterMetaType<FormatList>();
    }

    Q_SLOT void test_data()
    {
        QTest::addColumn<Logic::Layout::Orientation>("orientation");
        QTest::addColumn<QList<QMouseEvent*> >("mouse_events");
        QTest::addColumn<QString>("expected_last_preedit_string");
        QTest::addColumn<QString>("expected_commit_string");
        QTest::addColumn<FormatList>("expected_preedit_format");
        QTest::addColumn<bool>("word_engine_enabled");
        QTest::addColumn<int>("expected_cursor_position");

        for (int orientation = 0; orientation < 1; ++orientation) {
            // FIXME: here should be 2          ^
            // FIXME: tests fail for portrait layouts
            const Logic::Layout::Orientation layout_orientation(orientation == 0
                                                                ? Logic::Layout::Landscape
                                                                : Logic::Layout::Portrait);
            QTest::newRow("No mouse events: expect empty commit string, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>())
                << "" << "" << FormatList() << true << 0;

            QTest::newRow("Only return pressed: expect empty commit string, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "" << "" << FormatList() << true << 0;

            QTest::newRow("Release outside of widget: expect empty commit string, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(QPoint(g_size * 2, g_size * 2), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "" << "" << FormatList() << true << 0;

            QTest::newRow("Release button over key 'a': expect commit string 'a', preedit face should be active.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "a" << "a" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditActive)) << true << 0;

            QTest::newRow("Release button over key 'a', but no commit: expect empty commit string.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation))
                << "a" << "" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditActive)) << true << 1;

            QTest::newRow("Release button over keys 'c, b, d, a': expect commit string 'cbda', preedit face should be no candidates")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("c"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("space"), layout_orientation))
                << "cbda" << "cbda " << (FormatList() << Maliit::PreeditTextFormat(0, 4, Maliit::PreeditNoCandidates)) << true << 0;

            QTest::newRow("Typing two words: expect commit string 'ab cd', with last preedit being 'cd', preedit face should be no candidates.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("space"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("c"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "cd" << "ab cd" << (FormatList() << Maliit::PreeditTextFormat(0, 2, Maliit::PreeditNoCandidates)) << true << 0;

            QTest::newRow("Typing one word 'abd': expect commit string 'abd', with last preedit being 'abd', preedit face should be default")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("return"), layout_orientation))
                << "abd" << "abd" << (FormatList() << Maliit::PreeditTextFormat(0, 3, Maliit::PreeditDefault)) << true << 0;

            // TODO: we probably should not sent any preedit formats when word engine is turned off.
            QTest::newRow("Typing one word 'abd' with word engine turned off: expect commit string 'abd', with preedit being last char, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation))
                << "d" << "abd" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditDefault)) << false << 0;

            QTest::newRow("Typing one word 'ab' with word engine turned off: expect commit string 'ab', with preedit being last char, should be no preedit face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("a"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation))
                << "b" << "ab" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditDefault)) << false << 0;

            QTest::newRow("Typing one word 'bd' with word engine turned off: expect commit string 'bd', with preedit being last char, face should be for one char with default face")
                << layout_orientation
                << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("b"), layout_orientation)
                                           << createPressReleaseEvent(keyOriginLookup("d"), layout_orientation))
                << "d" << "bd" << (FormatList() << Maliit::PreeditTextFormat(0, 1, Maliit::PreeditDefault)) << false << 0;

        }
    }

    Q_SLOT void test()
    {
        // FIXME: mikhas: We should have tests for the preedit &
        // preedit correctness stuff, and how it blends with word
        // prediction. I guess you will need to add
        // WordEngine::setSpellChecker() API so that you can inject a
        // fake spellchecker, for the tests. Otherwise, the test would
        // have to be skipped when there's no hunspell/presage, which
        // I wouldn't like to have.

        QFETCH(Logic::Layout::Orientation, orientation);
        QFETCH(QList<QMouseEvent*>, mouse_events);
        QFETCH(QString, expected_last_preedit_string);
        QFETCH(QString, expected_commit_string);
        QFETCH(QList<Maliit::PreeditTextFormat>, expected_preedit_format);
        QFETCH(bool, word_engine_enabled);
        QFETCH(int, expected_cursor_position);

        SetupTest test_setup(orientation, word_engine_enabled);

        Q_FOREACH (QMouseEvent *ev, mouse_events) {
            QApplication::instance()->postEvent(test_setup.surface->viewport(), ev);
        }

        TestUtils::waitForSignal(&test_setup.glass, SIGNAL(keyReleased(Key,Logic::Layout *)));
        QCOMPARE(test_setup.host.lastPreeditString(), expected_last_preedit_string);
        QCOMPARE(test_setup.host.commitStringHistory(), expected_commit_string);
        QCOMPARE(test_setup.host.lastPreeditTextFormatList(), expected_preedit_format);
        QCOMPARE(test_setup.editor.text()->cursorPosition(), expected_cursor_position);
    }

    Q_SLOT void testPreeditActivation_data()
    {
        QTest::addColumn<QString>("surrounding_text");
        QTest::addColumn<int>("cursor_position");
        QTest::addColumn<QString>("expected_preedit");
        QTest::addColumn<int>("expected_replace_start");
        QTest::addColumn<int>("expected_replace_length");
        QTest::addColumn<int>("expected_cursor_position");
        QTest::addColumn<bool>("expected_preedit_string_sent");

        QTest::newRow("'aaa bbb', select first 'a'")
            << "aaa bbbb" // surrounding text
            << 0 // chosen cursor position
            << "aaa" // expected preedit
            << 0 // expected replace start (relative to cursor position in a preedit)
            << 3 // expected replace length
            << 0 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent

        QTest::newRow("'aaa bbb', select second 'a'")
            << "aaa bbbb" // surrounding text
            << 1 // chosen cursor position
            << "aaa" // expected preedit
            << -1 // expected replace start (relative to cursor position in a preedit)
            << 3 // expected replace length
            << 1 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent

        QTest::newRow("'aaa bbb', select third 'a'")
            << "aaa bbbb" // surrounding text
            << 2 // chosen cursor position
            << "aaa" // expected preedit
            << -2 // expected replace start (relative to cursor position in a preedit)
            << 3 // expected replace length
            << 2 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent

        QTest::newRow("'aaa bbb', select space between words")
            << "aaa bbbb" // surrounding text
            << 3 // chosen cursor position
            << "aaa" // expected preedit
            << -3 // expected replace start (relative to cursor position in a preedit)
            << 3 // expected replace length
            << 3 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent

        QTest::newRow("'aaa bbb', select first 'b'")
            << "aaa bbbb" // surrounding text
            << 4 // chosen cursor position
            << "bbbb" // expected preedit
            << 0 // expected replace start (relative to cursor position in a preedit)
            << 4 // expected replace length
            << 0 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent

        QTest::newRow("'aaa bbb', select after last 'b'")
            << "aaa bbbb" // surrounding text
            << 8 // chosen cursor position
            << "bbbb" // expected preedit
            << -4 // expected replace start (relative to cursor position in a preedit)
            << 4 // expected replace length
            << 4 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent

        QTest::newRow("' aaa', select leading space")
            << " aaa" // surrounding text
            << 0 // chosen cursor position
            << "" // expected preedit
            << 0 // expected replace start (relative to cursor position in a preedit)
            << 0 // expected replace length
            << 0 // expected cursor position (relative to the beginning of preedit)
            << false; // whether preedit is sent

        QTest::newRow("'a  b', select space before 'b'")
            << "a  b" // surrounding text
            << 2 // chosen cursor position
            << "" // expected preedit
            << 0 // expected replace start (relative to cursor position in a preedit)
            << 0 // expected replace length
            << 0 // expected cursor position (relative to the beginning of preedit)
            << false; // whether preedit is sent

        QTest::newRow("'a  ', select last space")
            << "a  " // surrounding text
            << 2 // chosen cursor position
            << "" // expected preedit
            << 0 // expected replace start (relative to cursor position in a preedit)
            << 0 // expected replace length
            << 0 // expected cursor position (relative to the beginning of preedit)
            << false; // whether preedit is sent

        QTest::newRow("'a ', select after last space")
            << "a " // surrounding text
            << 2 // chosen cursor position
            << "" // expected preedit
            << 0 // expected replace start (relative to cursor position in a preedit)
            << 0 // expected replace length
            << 0 // expected cursor position (relative to the beginning of preedit)
            << false; // whether preedit is sent

        QTest::newRow("'aaa', select far after last char")
            << "aaa" // surrounding text
            << 200 // chosen cursor position
            << "aaa" // expected preedit
            << -3 // expected replace start (relative to cursor position in a preedit)
            << 3 // expected replace length
            << 3 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent

        QTest::newRow("'aaa', select far before first char")
            << "aaa" // surrounding text
            << -200 // chosen cursor position
            << "aaa" // expected preedit
            << 0 // expected replace start (relative to cursor position in a preedit)
            << 3 // expected replace length
            << 0 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent

        QTest::newRow("' aaa ', select trailing space")
            << " aaa " // surrounding text
            << 4 // chosen cursor position
            << "aaa" // expected preedit
            << -3 // expected replace start (relative to cursor position in a preedit)
            << 3 // expected replace length
            << 3 // expected cursor position (relative to the beginning of preedit)
            << true; // whether preedit is sent
    }

    Q_SLOT void testPreeditActivation()
    {
        QFETCH(QString, surrounding_text);
        QFETCH(int, cursor_position);
        QFETCH(QString, expected_preedit);
        QFETCH(int, expected_replace_start);
        QFETCH(int, expected_replace_length);
        QFETCH(int, expected_cursor_position);
        QFETCH(bool, expected_preedit_string_sent);

        BasicSetupTest test_setup;
        QScopedPointer<MImUpdateEvent> update_event(createUpdateEvent(surrounding_text,
                                                                      cursor_position));

        test_setup.notifier.notify(update_event.data());

        QCOMPARE(test_setup.host.preeditStringSent(), expected_preedit_string_sent);
        if (expected_preedit_string_sent) {
            QCOMPARE(test_setup.host.lastPreeditString(), expected_preedit);
            QCOMPARE(test_setup.host.lastReplaceStart(), expected_replace_start);
            QCOMPARE(test_setup.host.lastReplaceLength(), expected_replace_length);
            QCOMPARE(test_setup.host.lastCursorPos(), expected_cursor_position);
        }
        QCOMPARE(test_setup.editor.text()->preedit(), expected_preedit);
        QCOMPARE(test_setup.editor.text()->cursorPosition(), expected_cursor_position);
    }

    Q_SLOT void testPreeditActivationTyping_data()
    {
        QTest::addColumn<QString>("surrounding_text");
        QTest::addColumn<int>("cursor_position");
        QTest::addColumn<QList<QMouseEvent *> >("events");
        QTest::addColumn<QString>("expected_preedit_string");
        QTest::addColumn<int>("expected_cursor_position");

        QTest::newRow("'aaa bbb', select first 'a', then type 'd'")
            << "aaa bbbb" // surrounding text
            << 0 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "daaa" // expected preedit
            << 1; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'aaa bbb', select second 'a', then type 'd'")
            << "aaa bbbb" // surrounding text
            << 1 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "adaa" // expected preedit
            << 2; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'aaa bbb', select third 'a', then type 'd'")
            << "aaa bbbb" // surrounding text
            << 2 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "aada" // expected preedit
            << 3; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'aaa bbb', select space between words, then type 'd'")
            << "aaa bbbb" // surrounding text
            << 3 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "aaad" // expected preedit
            << 4; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'aaa bbb', select first 'b', then type 'd'")
            << "aaa bbbb" // surrounding text
            << 4 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "dbbbb" // expected preedit
            << 1; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'aaa bbb', select after last 'b', then type 'd'")
            << "aaa bbbb" // surrounding text
            << 8 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "bbbbd" // expected preedit
            << 5; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("' aaa', select leading space, then type 'd'")
            << " aaa" // surrounding text
            << 0 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "d" // expected preedit
            << 1; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'a  b', select space before 'b', then type 'd'")
            << "a  b" // surrounding text
            << 2 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "d" // expected preedit
            << 1; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'a  ', select last space, then type 'd'")
            << "a  " // surrounding text
            << 2 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "d" // expected preedit
            << 1; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'a ', select after last space, then type 'd'")
            << "a " // surrounding text
            << 2 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "d" // expected preedit
            << 1; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'aaa', select far after last char, then type 'd'")
            << "aaa" // surrounding text
            << 200 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "aaad" // expected preedit
            << 4; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("'aaa', select far before first char, then type 'd'")
            << "aaa" // surrounding text
            << -200 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "daaa" // expected preedit
            << 1; // expected cursor position (relative to the beginning of preedit)

        QTest::newRow("' aaa ', select trailing space, then type 'd'")
            << " aaa " // surrounding text
            << 4 // chosen cursor position
            << (QList<QMouseEvent *>() << createPressReleaseEvent(keyOriginLookup("d")))
            << "aaad" // expected preedit
            << 4; // expected cursor position (relative to the beginning of preedit)
    }

    Q_SLOT void testPreeditActivationTyping()
    {
        QFETCH(QString, surrounding_text);
        QFETCH(int, cursor_position);
        QFETCH(QList<QMouseEvent*>, events);
        QFETCH(QString, expected_preedit_string);
        QFETCH(int, expected_cursor_position);

        SetupTest test_setup;
        QScopedPointer<MImUpdateEvent> update_event(createUpdateEvent(surrounding_text,
                                                                      cursor_position));

        test_setup.notifier.notify(update_event.data());

        Q_FOREACH (QMouseEvent *ev, events) {
            QApplication::instance()->postEvent(test_setup.surface->viewport(), ev);
        }

        TestUtils::waitForSignal(&test_setup.glass, SIGNAL(keyReleased(Key,Logic::Layout *)));
        QCOMPARE(test_setup.host.lastPreeditString(), expected_preedit_string);
        QCOMPARE(test_setup.editor.text()->cursorPosition(), expected_cursor_position);
    }
};

QTEST_MAIN(TestPreeditString)
#include "main.moc"
