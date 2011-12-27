/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

#include "models/key.h"
#include "models/keyarea.h"
#include "models/layout.h"
#include "view/glass.h"
#include "plugin/editor.h"

#include <mabstractinputmethodhost.h>
#include <mimplugindescription.h>
#include <QtCore>
#include <QtTest>
#include <QWidget>
#include <QList>
#include <QMouseEvent>

using namespace MaliitKeyboard;
Q_DECLARE_METATYPE(Layout::Orientation)
Q_DECLARE_METATYPE(QList<QMouseEvent*>)

namespace {
const int g_size = 64;

KeyArea createAbcdArea(int size)
{
    KeyArea key_area;
    key_area.rect = QRect(0, 0, size, size);

    Key keyA;
    keyA.setText("a");
    keyA.setRect(QRect(0, 0, size / 2, size / 2));
    key_area.keys.append(keyA);

    Key keyB;
    keyB.setText("b");
    keyB.setRect(QRect(size / 2, 0, size / 2, size / 2));
    key_area.keys.append(keyB);

    Key keyC;
    keyC.setText("c");
    keyC.setRect(QRect(0, size / 2, size / 2, size / 2));
    key_area.keys.append(keyC);

    Key keyD;
    keyD.setText("d");
    keyD.setRect(QRect(size / 2, size / 2, size / 2, size / 2));
    key_area.keys.append(keyD);

    return key_area;
}

QMouseEvent *createReleaseEvent(int x,
                                int y,
                                Layout::Orientation orientation)
{
    return new QMouseEvent(QKeyEvent::MouseButtonRelease,
                           (orientation == Layout::Landscape) ? QPoint(x, y) : QPoint(y, g_size - x),
                           Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
}
}

class InputMethodHostProbe
    : public MAbstractInputMethodHost
{
private:
    QString m_commit_string_history;

public:
    QString commitStringHistory() const
    {
        return m_commit_string_history;
    }

    void sendCommitString(const QString &string,
                          int replace_start,
                          int replace_length,
                          int cursor_pos)
    {
        Q_UNUSED(replace_start)
        Q_UNUSED(replace_length)
        Q_UNUSED(cursor_pos)

        m_commit_string_history.append(string);
    }

    // unused reimpl
    int contentType(bool&) {return 0;}
    bool correctionEnabled(bool&) {return false;}
    bool predictionEnabled(bool&) {return false;}
    bool autoCapitalizationEnabled(bool&) {return false;}
    bool surroundingText(QString&, int&) {return false;}
    bool hasSelection(bool&) {return false;}
    int inputMethodMode(bool&) {return 0;}
    QRect preeditRectangle(bool&) {return QRect();}
    QRect cursorRectangle(bool&) {return QRect();}
    int anchorPosition(bool&) {return 0;}
    QString selection(bool&) {return QString();}
    void sendPreeditString(const QString&, const QList<MInputMethod::PreeditTextFormat>&, int, int, int) {}
    void sendKeyEvent(const QKeyEvent&, MInputMethod::EventRequestType) {}
    void notifyImInitiatedHiding() {}
    void copy() {}
    void paste() {}
    void setRedirectKeys(bool) {}
    void setDetectableAutoRepeat(bool) {}
    void setGlobalCorrectionEnabled(bool) {}
    void setInputModeIndicator(MInputMethod::InputModeIndicator) {}
    void switchPlugin(MInputMethod::SwitchDirection) {}
    void switchPlugin(const QString&) {}
    void setScreenRegion(const QRegion&) {}
    void setInputMethodArea(const QRegion&) {}
    void setSelection(int, int) {}
    void setOrientationAngleLocked(bool) {}
    QList<MImPluginDescription> pluginDescriptions(MInputMethod::HandlerState) const {return QList<MImPluginDescription>();}
};

class TestCommitString
    : public QObject
{
    Q_OBJECT

private:
    QScopedPointer<QApplication> m_app;


    Q_SLOT void initTestCase()
    {
        qRegisterMetaType<QList<QMouseEvent*> >();
        qRegisterMetaType<Layout::Orientation>();
        m_app.reset(TestUtils::createApplication("commit-string"));
    }

    Q_SLOT void test_data()
    {
        const int s_fourth = g_size / 4;
        QTest::addColumn<Layout::Orientation>("orientation");
        QTest::addColumn<QList<QMouseEvent*> >("mouse_events");
        QTest::addColumn<QString>("expected_commit_string_history");

        for (int orientation = 0; orientation < 2; ++orientation) {
            const Layout::Orientation layout_orientation(orientation == 0 ? Layout::Landscape
                                                                          : Layout::Portrait);
            QTest::newRow("No mouse events: expect empty commit string.")
                << layout_orientation
                << (QList<QMouseEvent *>())
                << "";

            QTest::newRow("Release outside of widget: expect empty commit string.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createReleaseEvent(g_size * 2, g_size * 2, layout_orientation))
                << "";

            QTest::newRow("Release button over key 'a': expect commit string 'a'.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createReleaseEvent(s_fourth, s_fourth, layout_orientation))
                << "a";

            QTest::newRow("Release button over keys 'c, b, d, a': expect commit string 'cbda'.")
                << layout_orientation
                << (QList<QMouseEvent *>() << createReleaseEvent(s_fourth, s_fourth * 3, layout_orientation)
                                           << createReleaseEvent(s_fourth * 3, s_fourth, layout_orientation)
                                           << createReleaseEvent(s_fourth * 3, s_fourth * 3, layout_orientation)
                                           << createReleaseEvent(s_fourth, s_fourth, layout_orientation))
                << "cbda";
        }
    }

    Q_SLOT void test()
    {
        QFETCH(Layout::Orientation, orientation);
        QFETCH(QList<QMouseEvent*>, mouse_events);
        QFETCH(QString, expected_commit_string_history);

        QWidget window;
        Glass glass;
        Editor editor;
        InputMethodHostProbe host;
        SharedLayout layout(new Layout);

        window.setGeometry(0, 0, g_size, g_size);
        glass.setWindow(&window);
        glass.addLayout(layout);
        editor.setHost(&host);
        layout->setOrientation(orientation);

        // TODO: Make sure this maps to how key events are handled between
        // Glass, InputMethod and Editor in reality.
        connect(&glass,  SIGNAL(keyReleased(Key,SharedLayout)),
                &editor, SLOT(onKeyReleased(Key)));

        // Populate KeyArea with four keys, a, b, c, d. Notice how the KeyArea
        // covers the whole widget.
        // .-------.
        // | a | b |
        // |---|---|
        // | c | d |
        // `-------'
        const KeyArea &key_area(createAbcdArea(g_size));

        layout->setCenterPanel(key_area);
        layout->setActivePanel(Layout::CenterPanel);

        Q_FOREACH (QMouseEvent *ev, mouse_events) {
            m_app->postEvent(&window, ev);
        }

        TestUtils::waitForSignal(&glass, SIGNAL(keyReleased(Key,SharedLayout)));
        QCOMPARE(host.commitStringHistory(), expected_commit_string_history);
    }
};

QTEST_APPLESS_MAIN(TestCommitString)
#include "main.moc"
