/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

 */



#include "mapplication.h"
#include "mgconfitem_stub.h"
#include "mreactionmaptester.h"
#include "mvirtualkeyboardstyle.h"
#include "mvirtualkeyboard.h"
#include "horizontalswitcher.h"
#include "layoutsmanager.h"
#include "notification.h"
#include "ut_mvirtualkeyboard.h"
#include "mimkeymodel.h"
#include "mimtoolbar.h"
#include "mplainwindow.h"
#include "utils.h"
#include "regiontracker.h"
#include "reactionmappainter.h"

#include <minputmethodnamespace.h>
#include <MNamespace>
#include <MScene>
#include <MSceneManager>
#include <MSceneWindow>
#include <QDebug>
#include <QFile>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QSignalSpy>
#include <QTimer>
#include <QGraphicsLayout>
#include <QGraphicsLinearLayout>
#include <mtexteditmodel.h>
#include <mtheme.h>

namespace
{
    const QString InputMethodSettingName("/meegotouch/inputmethods/onscreen/enabled");
    const QString DefaultLayoutSettingName("/meegotouch/inputmethods/virtualkeyboard/layouts/default");
    const int SceneRotationTime = 1400; // in ms
    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");

    int gDisplayTextCalls = 0;
}

Q_DECLARE_METATYPE(MImKeyBinding::KeyAction);
Q_DECLARE_METATYPE(MInputMethod::SwitchDirection);
Q_DECLARE_METATYPE(ModifierState);

// STUBS

MApplication::~MApplication()
{
}

void Notification::displayText(const QString &message, const QRectF &area)
{
    Q_UNUSED(message);
    Q_UNUSED(area);

    qDebug() << __PRETTY_FUNCTION__ << __FILE__ << __LINE__;
    ++gDisplayTextCalls;
}


void Ut_MVirtualKeyboard::initTestCase()
{
    qRegisterMetaType<MInputMethod::SwitchDirection>("MInputMethod::SwitchDirection");
    qRegisterMetaType<ModifierState>("ModifierState");

    disableQtPlugins();

    static char *argv[2] = {(char *) "ut_mvirtualkeyboard",
                            (char *) "-software"};
    static int argc = 2;

    // This value is required by the theme daemon
    MImSettings(TargetSettingsName).set(DefaultTargetName);

    app = new MApplication(argc, argv);
    RegionTracker::createInstance();
    ReactionMapPainter::createInstance();

    QString InputMethodSetting(InputMethodSettingName);
    MImSettings item1(InputMethodSetting);

    QStringList layoutlist;
    layoutlist << "libmeego-keyboard.so"
               << "/usr/lib/meego-keyboard-tests/layouts/en_gb.xml"
               << "libmeego-keyboard.so"
               << "/usr/lib/meego-keyboard-tests/layouts/fi.xml"
               << "libmeego-keyboard.so"
               << "/usr/lib/meego-keyboard-tests/layouts/ar.xml";
    item1.set(QVariant(layoutlist));

    QString DefaultLayoutSetting(DefaultLayoutSettingName);
    MImSettings item2(DefaultLayoutSetting);
    QString defaultLayout = "/usr/lib/meego-keyboard-tests/layouts/en_gb.xml";
    item2.set(QVariant(defaultLayout));

    // MVirtualkeyboard uses MPlainWindow internally so we need to instantiate it.
    new MPlainWindow; // creates a static instance

    vkbParent = new MSceneWindow;
    vkbParent->setManagedManually(true); // we want the scene window to remain in origin
    vkbParent->setObjectName("SceneWindow");
    // Adds scene window to scene.
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(vkbParent);

    vkbStyleContainer = new MVirtualKeyboardStyleContainer;
    vkbStyleContainer->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    LayoutsManager::createInstance();
}

void Ut_MVirtualKeyboard::cleanupTestCase()
{
    LayoutsManager::destroyInstance();
    delete vkbParent;
    delete vkbStyleContainer;
    vkbStyleContainer = 0;
    delete MPlainWindow::instance();
    delete app;
    app = 0;
    RegionTracker::destroyInstance();
    ReactionMapPainter::destroyInstance();
}

void Ut_MVirtualKeyboard::init()
{
    m_vkb = new MVirtualKeyboard(LayoutsManager::instance(), vkbStyleContainer, vkbParent);

    if (MPlainWindow::instance()->orientationAngle() != M::Angle0)
        rotateToAngle(MInputMethod::Angle0);
}

void Ut_MVirtualKeyboard::cleanup()
{
#ifdef HAVE_REACTIONMAP
    // Stub might have been set to a local instance.
    gMReactionMapStub = &gDefaultMReactionMapStub;
#endif
    delete m_vkb;
    m_vkb = 0;
}

void Ut_MVirtualKeyboard::clickBackspaceTest()
{
#if 0 // DISABLED: functionKeysMap not available
    FlickUpButton *button = 0;

    for (int i = 0; i < numFunctionKeys; i++) {
        const MImKeyModel *key = functionkeySection->getVKBKey(0, 0, i);
        if (key->action == MImKeyModel::ActionBackspace)
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clicked(const QString &)));
    QSignalSpy pressed(m_vkb, SIGNAL(keyPressed(const QChar &)));
    //TODO:there are bugs in mbutton, we can not simulate the keypress and keyrelease...
    m_vkb->pressBackspace();
    button->setDown(false);
    m_vkb->autoBackspace();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal
    QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument

    QCOMPARE(pressed.count(), 1);
    arguments = pressed.takeFirst();
    QVERIFY(arguments.count() == 1);
    QVERIFY(arguments.first().toChar() == QChar('\b'));
#endif
}

void Ut_MVirtualKeyboard::clickSpaceTest()
{
#if 0
    FlickUpButton *button = 0;

    for (int i = 0; i < numFunctionKeys; i++) {
        const MImKeyModel *key = functionkeySection->getVKBKey(0, 0, i);
        if (key->action == MImKeyModel::ActionSpace)
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftOff);

    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    button->click();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == " "); // verify the first argument

    m_vkb->clickShift();
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftOn);
    spy.clear();

    button->click();

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == "\n"); // verify the first argument
#endif
}

void Ut_MVirtualKeyboard::setShiftStateTest()
{
    QList<ModifierState> levels;
    levels << ModifierLatchedState << ModifierClearState
           << ModifierLockedState;
    foreach(ModifierState level, levels) {
        m_vkb->setShiftState(level);
        QCOMPARE(m_vkb->shiftStatus(), level);
    }
}

void Ut_MVirtualKeyboard::clickHyphenTest()
{
#if 0
    FlickUpButton *button = 0;
    for (int i = 0; i < numFunctionKeys; i++) {
        const MImKeyModel *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->label == "-")
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    button->click();
    QCOMPARE(spy.count(), 1);

    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == "-"); // verify the first argument
#endif
}


void Ut_MVirtualKeyboard::clickPunctQuesTest()
{
#if 0
    FlickUpButton *button = 0;
    for (int i = 0; i < numFunctionKeys; i++) {
        const MImKeyModel *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->label == "?")
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    button->click();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == "?"); // verify the first argument
#endif
}

void Ut_MVirtualKeyboard::clickPunctDotTest()
{
#if 0
    FlickUpButton *button = 0;

    for (int i = 0; i < numFunctionKeys; i++) {
        const MImKeyModel *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->label == ".")
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    button->click();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst(); // take the first signal

    QVERIFY(arguments.at(0).toString() == "."); // verify the first argument
#endif
}

void Ut_MVirtualKeyboard::testStateReset()
{
    // Set states that should be changed next time opening vkb.
    m_vkb->setShiftState(ModifierLatchedState); // Shift on
    m_vkb->resetState();
    QCOMPARE(m_vkb->shiftStatus(), ModifierClearState); // Shift should be off
}

void Ut_MVirtualKeyboard::testShiftLevelChange_data()
{
    QTest::addColumn<bool>("enableMultiTouch");
    QTest::addColumn<ModifierState>("initialShiftState");
    QTest::addColumn<bool>("shiftPressed");
    QTest::addColumn<int>("expectedMainLayoutLevel");

    // No multi-touch.
    QTest::newRow("shift cleared, not pressed") << false << ModifierClearState   << false << 0;
    QTest::newRow("shift cleared, pressed")     << false << ModifierClearState   << true  << 0;
    QTest::newRow("shift latched, not pressed") << false << ModifierLatchedState << false << 1;
    QTest::newRow("shift latched, pressed")     << false << ModifierLatchedState << true  << 1;
    QTest::newRow("shift locked, not pressed")  << false << ModifierLockedState  << false << 1;
    QTest::newRow("shift locked, pressed")      << false << ModifierLockedState  << true  << 1;

    // Multi-touch enabled.
    QTest::newRow("mt, shift cleared, not pressed") << true << ModifierClearState   << false << 0;
    QTest::newRow("mt, shift cleared, pressed")     << true << ModifierClearState   << true  << 1;
    QTest::newRow("mt, shift latched, not pressed") << true << ModifierLatchedState << false << 1;
    QTest::newRow("mt, shift latched, pressed")     << true << ModifierLatchedState << true  << 1;
    QTest::newRow("mt, shift locked, not pressed")  << true << ModifierLockedState  << false << 1;
    QTest::newRow("mt, shift locked, pressed")      << true << ModifierLockedState  << true  << 1;
}

void Ut_MVirtualKeyboard::testShiftLevelChange()
{
    QFETCH(bool, enableMultiTouch);
    QFETCH(ModifierState, initialShiftState);
    QFETCH(bool, shiftPressed);
    QFETCH(int, expectedMainLayoutLevel);

    MImAbstractKeyArea *mainKbLayout = static_cast<MImAbstractKeyArea *>(m_vkb->mainKeyboardSwitcher->currentWidget());

    // Enable or disable multi-touch.
    m_vkb->enableMultiTouch = enableMultiTouch;

    m_vkb->setShiftState(initialShiftState);

    QMetaObject::invokeMethod(&m_vkb->eventHandler, "shiftPressed", Q_ARG(bool, shiftPressed));

    QCOMPARE(mainKbLayout->level(), expectedMainLayoutLevel);
}

void Ut_MVirtualKeyboard::flickRightHandlerTest()
{
    QStringList layoutList = MImSettings(InputMethodSettingName).value().toStringList();
    layoutList.removeAll("libmeego-keyboard.so");
    layoutList.sort();
    qDebug() << layoutList;

    // Vkb has just been created and is running an animation.
    QTest::qWait(550);

    QSignalSpy spySwitchRequested(m_vkb,
                                  SIGNAL(pluginSwitchRequired(MInputMethod::SwitchDirection)));
    QVERIFY(spySwitchRequested.isValid());

    // flick right, switch direction left
    QSignalSpy spy(m_vkb, SIGNAL(layoutChanged(const QString &)));
    int index = m_vkb->mainKeyboardSwitcher->current();
    m_vkb->flickRightHandler();
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).count(), 1);
    //depends on the layouts set through MImSettings
    QCOMPARE(spy.at(0).at(0).toString().toLower(), layoutList.at(0).toLower());

    QTest::qWait(550);

    index = m_vkb->mainKeyboardSwitcher->current();
    spySwitchRequested.clear();
    m_vkb->flickRightHandler();
    if (spySwitchRequested.count()) {
        QCOMPARE(index, 0);
        m_vkb->switchLayout(spySwitchRequested.first().first().value<MInputMethod::SwitchDirection>(),
                            false);
    }
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).count(), 1);
    //depends on the layouts set through MImSettings
    QCOMPARE(spy.at(1).at(0).toString().toLower(), layoutList.at(2).toLower());

    QTest::qWait(550);

    index = m_vkb->mainKeyboardSwitcher->current();
    spySwitchRequested.clear();
    m_vkb->flickRightHandler();
    if (spySwitchRequested.count()) {
        QCOMPARE(index, 0);
        m_vkb->switchLayout(spySwitchRequested.first().first().value<MInputMethod::SwitchDirection>(),
                            false);
    }
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(2).count(), 1);
    //depends on the layouts set through MImSettings
    QCOMPARE(spy.at(2).at(0).toString().toLower(), layoutList.at(1).toLower());
}

void Ut_MVirtualKeyboard::flickLeftHandlerTest()
{
    QStringList layoutList = MImSettings(InputMethodSettingName).value().toStringList();
    layoutList.removeAll("libmeego-keyboard.so");
    layoutList.sort();

    // Vkb has just been created and is running an animation.
    QTest::qWait(550);

    QSignalSpy spySwitchRequested(m_vkb,
                                  SIGNAL(pluginSwitchRequired(MInputMethod::SwitchDirection)));
    QVERIFY(spySwitchRequested.isValid());

    QSignalSpy spy(m_vkb, SIGNAL(layoutChanged(const QString &)));
    int index = m_vkb->mainKeyboardSwitcher->current();
    qDebug() << "index1: " << index << " layout:" << m_vkb->currentLayout;
    spySwitchRequested.clear();
    m_vkb->flickLeftHandler();
    if (spySwitchRequested.count()) {
        QCOMPARE(index, layoutList.count() - 1);
        m_vkb->switchLayout(spySwitchRequested.first().first().value<MInputMethod::SwitchDirection>(),
                            false);
    }
    qDebug() << "index2: " << m_vkb->mainKeyboardSwitcher->current() << " lang:" << m_vkb->currentLayout;
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).count(), 1);
    //depends on the layouts set through MImSettings
    QCOMPARE(spy.at(0).at(0).toString().toLower(), layoutList.at(2).toLower());

    QTest::qWait(550);

    index = m_vkb->mainKeyboardSwitcher->current();
    spySwitchRequested.clear();
    m_vkb->flickLeftHandler();
    if (spySwitchRequested.count()) {
        QCOMPARE(index, layoutList.count() - 1);
        m_vkb->switchLayout(spySwitchRequested.first().first().value<MInputMethod::SwitchDirection>(),
                            false);
    }
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).count(), 1);
    //depends on the layouts set through MImSettings
    QCOMPARE(spy.at(1).at(0).toString().toLower(), layoutList.at(0).toLower());

    QTest::qWait(550);

    index = m_vkb->mainKeyboardSwitcher->current();
    spySwitchRequested.clear();
    m_vkb->flickLeftHandler();
    if (spySwitchRequested.count()) {
        QCOMPARE(index, layoutList.count() - 1);
        m_vkb->switchLayout(spySwitchRequested.first().first().value<MInputMethod::SwitchDirection>(),
                            false);
    }
    QVERIFY(m_vkb->mainKeyboardSwitcher->current() != index);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(spy.at(2).count(), 1);
    //depends on the layouts set through MImSettings
    QCOMPARE(spy.at(2).at(0).toString().toLower(), layoutList.at(1).toLower());
}

void Ut_MVirtualKeyboard::loadSymbolViewTemporarilyTest()
{
#if 0 // method not available anymore
    QVariant val(1);
    m_vkb->loadSCVTemporarily();
    QCOMPARE(m_vkb->scv->data(0), val);
#endif
}

void Ut_MVirtualKeyboard::errorCorrectionTest()
{
#if 0
    QSignalSpy spy(m_vkb, SIGNAL(errorCorrectionToggled(bool)));

    m_vkb->disableErrorCorrection();
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.at(0).count(), 1);
    QCOMPARE(spy.at(0).at(0).toBool(), false);

    m_vkb->enableErrorCorrection();
    QCOMPARE(spy.count(), 2);
    QCOMPARE(spy.at(1).count(), 1);
    QCOMPARE(spy.at(1).at(0).toBool(), true);
#endif
}

void Ut_MVirtualKeyboard::setKeyboardType()
{
    m_vkb->setKeyboardType(MInputMethod::NumberContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::Number);

    m_vkb->setKeyboardType(MInputMethod::PhoneNumberContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::PhoneNumber);

    m_vkb->setKeyboardType(MInputMethod::FreeTextContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::General);

    m_vkb->setKeyboardType(MInputMethod::EmailContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::General);

    m_vkb->setKeyboardType(MInputMethod::UrlContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::General);

    m_vkb->setKeyboardType(MInputMethod::CustomContentType);
    QCOMPARE(m_vkb->currentLayoutType, LayoutData::General);
}

void Ut_MVirtualKeyboard::longPressBackSpace()
{
#if 0
    FlickUpButton *button = 0;

    for (int i = 0; i < numFunctionKeys; i++) {
        const MImKeyModel *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->action == MImKeyModel::ActionBackspace)
            button = static_cast<FlickUpButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(button != 0);

    QSignalSpy spy(m_vkb, SIGNAL(clicked(const QString &)));
    spy.clear();
    //TODO:there are bugs in mbutton, we can not simulate the keypress and keyrelease...
    button->setDown(false);
    button->setDown(true);
    m_vkb->pressBackspace();
    QCOMPARE(spy.count(), 0);

    QList<QVariant> arguments;

    spy.clear();
    //500ms, means holding, and one signal should be emit
    QTest::qWait(550);
    QVERIFY(m_vkb->backSpaceTimer->isActive() == true);
    QVERIFY(button->isDown());
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst(); // take the first signal
    QVERIFY(arguments.count() == 1);
    QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument
    spy.clear();
    //another 100ms means repeat auto delete, and one more signal should be emit
    QTest::qWait(100);
    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst(); // take the first signal
    QVERIFY(arguments.count() == 1);
    QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument

    spy.clear();
    QTest::qWait(300);
    QCOMPARE(spy.count(), 3);
    for (int i = 0; i < spy.count(); i++) {
        arguments = spy.takeAt(i); //take each signal
        QVERIFY(arguments.count() == 1);
        QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument
    }

    spy.clear();
    QTest::qWait(500);
    QCOMPARE(spy.count(), 5);
    for (int i = 0; i < spy.count(); i++) {
        arguments = spy.takeAt(i); //take each singal
        QVERIFY(arguments.count() == 1);
        QVERIFY(arguments.at(0).toString() == "\b"); // verify the first argument
    }

    button->setDown(false);
#endif
}

/*
 *  Bug description: symbol keys send incorrect characters
 * in uppercase mode
 */
void Ut_MVirtualKeyboard::bug_130644()
{
#if 0
    FlickUpButton *symbolButton = 0;
    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    QStringList buttonLabels;

    buttonLabels << "'" << "!" << ","; //this list depends on substitute_label from EN_default.xml

    foreach(const QString & label, buttonLabels) {
        //initialization
        qDebug() << "Test button" << label;
        QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftOff);
        m_vkb->setShiftState(MVirtualKeyboard::ShiftOn);
        QVERIFY(m_vkb->shiftStatus() == MVirtualKeyboard::ShiftOn);

        symbolButton = 0;
        foreach(FlickUpButton * button, m_vkb->functionKeysMap.values()) {
            if (button->text() == label) {
                symbolButton = button;
                break;
            }
        }
        QVERIFY(symbolButton != 0); //we found button to click on

        spy.clear();
        //actual testing
        symbolButton->click();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().count(), 1);
        QCOMPARE(spy.first().first().toString(), label);
    }
#endif
}

void Ut_MVirtualKeyboard::symbolKeyTestLowercase()
{
#if 0
    FlickUpButton *symbolButton = 0;
    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    QStringList buttonLabels;

    buttonLabels << "-" << "?" << "."; //this list depends on substitute_label from EN_default.xml

    foreach(const QString & label, buttonLabels) {
        //initialization
        qDebug() << "Test button" << label;
        QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftOff);

        symbolButton = 0;
        foreach(FlickUpButton * button, m_vkb->functionKeysMap.values()) {
            if (button->text() == label) {
                symbolButton = button;
                break;
            }
        }
        QVERIFY(symbolButton != 0); //we found button to click on

        spy.clear();
        //actual testing
        symbolButton->click();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().count(), 1);
        QCOMPARE(spy.first().first().toString(), label);
    }
#endif
}

void Ut_MVirtualKeyboard::symbolKeyTestCapsLock()
{
#if 0
    FlickUpButton *symbolButton = 0;
    ShiftButton *shift = 0;
    QSignalSpy spy(m_vkb, SIGNAL(clickedSymbolButton(const QString &)));
    QStringList buttonLabels;

    buttonLabels << "'" << "!" << ","; //this list depends on substitute_label from EN_default.xml

    QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftOff);

    for (int i = 0; i < numFunctionKeys; i++) {
        const MImKeyModel *key = functionkeySection->getVKBKey(0, 0, i);

        if (key->action == MImKeyModel::ActionShift)
            shift = static_cast<ShiftButton *>(m_vkb->functionKeysMap.value(i));
    }
    QVERIFY(shift != 0);
    shift->lock();

    foreach(const QString & label, buttonLabels) {
        //initialization
        qDebug() << "Test button" << label;
        QCOMPARE(m_vkb->shiftStatus(), MVirtualKeyboard::ShiftLock);

        symbolButton = 0;
        foreach(FlickUpButton * button, m_vkb->functionKeysMap.values()) {
            if (button->text() == label) {
                symbolButton = button;
                break;
            }
        }
        QVERIFY(symbolButton != 0); //we found button to click on

        spy.clear();
        //actual testing
        symbolButton->click();
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().count(), 1);
        QCOMPARE(spy.first().first().toString(), label);
    }
#endif
}

void Ut_MVirtualKeyboard::interceptPress()
{
#if 0
    QSignalSpy spy(m_vkb, SIGNAL(keyPressed(const QChar &)));
    QList<QChar> testData;
    QList<QChar> expected;

    testData << 'a' << QChar(0x21B5) << QChar(0x2192) << QChar(0x2B06)
             << QChar(0x21E7) << QChar(0x21EA);
    expected << 'a' << '\n' << ' ' << QChar() << QChar() << QChar();
    QVERIFY(testData.count() == expected.count());

    for (int n = 0; n < testData.count(); ++n) {
        qDebug() << "test step" << n << testData.at(n);
        m_vkb->interceptPress(testData.at(n));
        QVERIFY(spy.count() == 1);
        QVERIFY(spy.first().count() == 1);
        QVERIFY(spy.first().first() == expected.at(n));
        spy.clear();
    }

    m_vkb->interceptPress(QChar());
    QVERIFY(spy.count() == 0);
#endif
}

/*
 * Bug description: Sym view becomes invisible when shift button
 * is activated
 */
void Ut_MVirtualKeyboard::bug_137295()
{
#if 0 // uses scv directly
    QVERIFY(m_vkb->scv->isActive() == false);
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftOff);

    m_vkb->scv->tabPressed(0);
    QVERIFY(m_vkb->scv->isActive() == true);
    QVERIFY(m_vkb->scv->isVisible() == true);

    m_vkb->clickShift();
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftOn);
    QVERIFY(m_vkb->scv->isActive() == true);
    QVERIFY(m_vkb->scv->isVisible() == true);

    m_vkb->lockShift();
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftLock);
    QVERIFY(m_vkb->scv->isActive() == true);
    QVERIFY(m_vkb->scv->isVisible() == true);

    m_vkb->scv->hide();
    QVERIFY(m_vkb->shiftLevel == MVirtualKeyboard::ShiftLock);
    QVERIFY(m_vkb->scv->isActive() == false);
    QVERIFY(m_vkb->scv->isVisible() == true);
#endif
}

void Ut_MVirtualKeyboard::testReactionMaps()
{
#ifdef HAVE_REACTIONMAP
    MReactionMapTester tester;
    gMReactionMapStub = &tester;

    m_vkb->setLayout(0);

    // Show keyboard
    m_vkb->show();
    QCOMPARE(MPlainWindow::instance()->scene(), m_vkb->scene());

    // Clear with transparent color
    gMReactionMapStub->setTransparentDrawingValue();
    gMReactionMapStub->setTransform(QTransform());
    gMReactionMapStub->fillRectangle(0, 0, gMReactionMapStub->width(), gMReactionMapStub->height());

    QGraphicsView *view(MPlainWindow::instance());
    m_vkb->paintReactionMap(MReactionMap::instance(*view), view);

    // Overall sanity test with grid points throughout the view.
    QVERIFY(tester.testReactionMapGrid(view, 40, 50, m_vkb->mapRectToScene(m_vkb->rect()).toRect(), m_vkb));

    // Check that all buttons are drawn with reactive color.
    QVERIFY(tester.testChildButtonReactiveAreas(view, m_vkb));

    // Switch layout, the chosen kb layouts are all different in terms of reaction maps they generate.
    QSignalSpy updateSignal(&m_vkb->signalForwarder, SIGNAL(requestRepaint()));
    m_vkb->setLayout(1);
    QTest::qWait(600);

    // Currently updating is done via kbhost when it receives region updates.
    // Kbhost is not present so we paint reaction map explicitly.
    QVERIFY(updateSignal.count() == 1);
    m_vkb->paintReactionMap(MReactionMap::instance(*view), view);

    QVERIFY(tester.testReactionMapGrid(view, 40, 50, m_vkb->mapRectToScene(m_vkb->rect()).toRect(), m_vkb));
    QVERIFY(tester.testChildButtonReactiveAreas(view, m_vkb));
#endif
}

// End of test functions!

void Ut_MVirtualKeyboard::rotateToAngle(MInputMethod::OrientationAngle angle)
{
    m_vkb->prepareToOrientationChange();
    MPlainWindow::instance()->setOrientationAngle(static_cast<M::OrientationAngle>(angle));
    QTest::qWait(SceneRotationTime);// wait until MSceneManager::orientationAngle() is updated.
    m_vkb->finalizeOrientationChange();
}

QTEST_APPLESS_MAIN(Ut_MVirtualKeyboard);
