/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */



#include "mreactionmaptester.h"

#include "mgconfitem_stub.h"
#include "mvirtualkeyboard.h"
#include "mvirtualkeyboardstyle.h"
#include "horizontalswitcher.h"
#include "keyevent.h"
#include "mimabstractkeyarea.h"
#include "layoutsmanager.h"
#include "regiontracker.h"
#include "reactionmappainter.h"
#include "symbolview.h"
#include "ut_symbolview.h"
#include "mplainwindow.h"
#include "utils.h"

#include <MApplication>
#include <MButton>
#include <MScene>
#include <MSceneManager>
#include <MSceneWindow>
#include <MTheme>

#include <QDir>
#include <QGraphicsLinearLayout>
#include <QSignalSpy>

namespace
{
    const QString InputMethodSettingName("/meegotouch/inputmethods/virtualkeyboard/layouts");
    const QString LocalTestLayout(TESTLAYOUTFILEPATH_LOCAL);
    const QString InstalledTestLayout(TESTLAYOUTFILEPATH_INSTALLED);

    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");

    const int SceneRotationTime = 1400;
} // namespace

Q_DECLARE_METATYPE(QList<QPoint>)
Q_DECLARE_METATYPE(HorizontalSwitcher::SwitchDirection)

void Ut_SymbolView::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_symbolview",
                                 (char *) "-software" };

    if (QFile::exists(LocalTestLayout)) {
        testLayoutFile = LocalTestLayout;
    } else if (QFile::exists(InstalledTestLayout)) {
        testLayoutFile = InstalledTestLayout;
    } else {
        QFAIL(QString("Test symbol layout does not exist.\n"
                      "Checked " + LocalTestLayout + " and " + InstalledTestLayout + ".").toAscii().data());
    }

    qDebug() << "Using test symbol layout from file: " << testLayoutFile;

    disableQtPlugins();

    MGConfItem target(TargetSettingsName);
    target.set(DefaultTargetName); // this value is required by the theme daemon

    app = new MApplication(argc, app_name);
    style = new MVirtualKeyboardStyleContainer;
    style->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    ReactionMapPainter::createInstance();

    MGConfItem inputMethodSetting(InputMethodSettingName);

    QStringList langlist;
    langlist << testLayoutFile;
    inputMethodSetting.set(QVariant(langlist));

    LayoutsManager::createInstance();

    qRegisterMetaType<KeyEvent>("KeyEvent");

    new MPlainWindow;
    RegionTracker::createInstance();
}

void Ut_SymbolView::cleanupTestCase()
{
    RegionTracker::destroyInstance();
    ReactionMapPainter::destroyInstance();
    delete MPlainWindow::instance();
    LayoutsManager::destroyInstance();
    delete style;
    style = 0;
    delete app;
    app = 0;
}

void Ut_SymbolView::init()
{
    subject = new SymbolView(LayoutsManager::instance(), style, testLayoutFile);

    // Add to scene so reaction maps are drawn.
    // SymView needs scene window as parent so positions itself correctly
    // in the scene in portrait mode.
    parent = new MSceneWindow;
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(parent);
    subject->setParentItem(parent);
}

void Ut_SymbolView::cleanup()
{
    // Make sure default stub is restored.
    gMReactionMapStub = &gDefaultMReactionMapStub;

    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(parent);
    subject->setParentItem(0);
    delete parent;
    delete subject;
    subject = 0;
}

void Ut_SymbolView::testReactiveButtonAreas_data()
{
    QTest::addColumn<int>("orientationAngle");

    QTest::newRow("Angle 0") << 0;
    QTest::newRow("Angle 90") << 90;
}

void Ut_SymbolView::testReactiveButtonAreas()
{
    QSKIP( "skipping testReactiveButtonAreas", SkipSingle );

    QFETCH(int, orientationAngle);;

    rotateToAngle(static_cast<M::OrientationAngle>(orientationAngle));

    MReactionMapTester tester;
    gMReactionMapStub = &tester;

    QGraphicsView *view = MPlainWindow::instance();

    // Clear with transparent color
    gMReactionMapStub->setTransparentDrawingValue();
    gMReactionMapStub->setTransform(QTransform());
    gMReactionMapStub->fillRectangle(0, 0, gMReactionMapStub->width(), gMReactionMapStub->height());

    subject->showSymbolView();

    subject->paintReactionMap(MReactionMap::instance(view), view);

    // Check that all buttons are covered by reactive area
    QVERIFY(tester.testChildButtonReactiveAreas(view, subject));

    // Test the next tab also.
    subject->switchToNextPage();
    QTest::qWait(600);
    QVERIFY(!subject->pageSwitcher->isRunning());

    // However since we don't have MKeyboardHost here we call this directly again.
    gMReactionMapStub->setTransparentDrawingValue();
    gMReactionMapStub->setTransform(QTransform());
    gMReactionMapStub->fillRectangle(0, 0, gMReactionMapStub->width(), gMReactionMapStub->height());
    subject->paintReactionMap(MReactionMap::instance(view), view);

    // Check that all buttons are covered by reactive area
    QVERIFY(tester.testChildButtonReactiveAreas(view, subject));

    // Following coordinates will be given in subject coordinates.
    gMReactionMapStub->setTransform(subject, view);

    // Check locations that should be inactive. This test is really only for
    // testing that at least something is drawn inactive, and not only reactive
    // areas are drawn.
    // Inactive areas should be drawn because second page in testsymbols.xml
    // has second row with spacers at the beginning and at the end of the row.
    const QRectF br(subject->boundingRect());
    QList<QPoint> inactiveLocations;
    inactiveLocations
            << QPoint(br.left(), br.bottom() - 1) // Left margin, second row
            << QPoint(br.right() - 1, br.bottom() - 1); // Right margin, second row

    foreach(const QPointF & pos, inactiveLocations) {
        QCOMPARE(tester.colorAt(pos), MReactionMapTester::Inactive);
    }
}

void Ut_SymbolView::testReactiveWholeScreen_data()
{
    QTest::addColumn<int>("orientationAngle");

    QTest::newRow("Angle 0") << 0;
    QTest::newRow("Angle 90") << 90;
    QTest::newRow("Angle 180") << 180;
    QTest::newRow("Angle 270") << 270;
}

void Ut_SymbolView::testReactiveWholeScreen()
{
    QFETCH(int, orientationAngle);

    rotateToAngle(static_cast<M::OrientationAngle>(orientationAngle));

    MReactionMapTester tester;
    gMReactionMapStub = &tester;
    // Clear with transparent
    gMReactionMapStub->setTransparentDrawingValue();
    gMReactionMapStub->setTransform(QTransform());
    gMReactionMapStub->fillRectangle(0, 0, gMReactionMapStub->width(), gMReactionMapStub->height());

    subject->showSymbolView();

    subject->paintReactionMap(MReactionMap::instance(MPlainWindow::instance()), MPlainWindow::instance());

    const bool gridpass = tester.testReactionMapGrid(
                              MPlainWindow::instance(),
                              20, 50, subject->interactiveRegion(),
                              subject);

    QVERIFY(gridpass);

    gMReactionMapStub = &gDefaultMReactionMapStub;
}


void Ut_SymbolView::testOthers()
{
    //at least we should not crash
    subject->organizeContent();

    subject->hideSymbolView();
    subject->showSymbolView();
}

void Ut_SymbolView::testChangeToOpenMode()
{
    subject->showSymbolView();
    QVERIFY(subject->isActive() == true);
    subject->hideSymbolView();
    QVERIFY(subject->isActive() == false);
}

void Ut_SymbolView::testChangeTab_data()
{
    QTest::addColumn<int>("initialTabIndex");
    QTest::addColumn<HorizontalSwitcher::SwitchDirection>("direction");
    QTest::addColumn<int>("finalTabIndex"); // expected

    QTest::newRow("Next tab")   << 0 << HorizontalSwitcher::Right << 1;
    QTest::newRow("Prev tab")   << 1 << HorizontalSwitcher::Left << 0;
}

void Ut_SymbolView::testChangeTab()
{
    QFETCH(int, initialTabIndex);
    QFETCH(HorizontalSwitcher::SwitchDirection, direction);
    QFETCH(int, finalTabIndex);

    subject->pageSwitcher->setDuration(0);
    subject->pageSwitcher->setCurrent(initialTabIndex);

    QCOMPARE(subject->activePage, subject->pageSwitcher->current());

    if (direction == HorizontalSwitcher::Right) {
        subject->switchToNextPage();
    } else if (direction == HorizontalSwitcher::Left){
        subject->switchToPrevPage();
    }

    QCOMPARE(subject->activePage, finalTabIndex);
}

void Ut_SymbolView::testHideWithFlick_data()
{
    QTest::addColumn<int>("tabIndex");
    QTest::newRow("Sym tab") << 0;
    QTest::newRow("Ace tab") << 1;
}


void Ut_SymbolView::testHideWithFlick()
{
    QFETCH(int, tabIndex);
    QVERIFY(subject->pageSwitcher && subject->pageSwitcher->count() >= tabIndex);
    MImAbstractKeyArea *page = static_cast<MImAbstractKeyArea *>(subject->pageSwitcher->widget(tabIndex));

    // Using qtest rows we dont need to worry about symbol view's show/hide timers
    // ignoring the call because subject is re-created every time.
    subject->showSymbolView();
    QCOMPARE(subject->isActive(), true);
    emit page->flickDown();
    QCOMPARE(subject->isActive(), false);
}


void Ut_SymbolView::testSetLayout()
{
    // set an invalid layout, at least it won't crash
    // and still has the laste valid layout.
    QString oldLayout = subject->currentLayout;
    subject->setLayout("ThisLayoutShouldNotExist");
    QVERIFY(subject->currentLayout != "ThisLayoutShouldNotExist");
    QCOMPARE(subject->currentLayout, oldLayout);
}

void Ut_SymbolView::testHardwareState()
{
    subject->setKeyboardState(MInputMethod::OnScreen);
    QCOMPARE(subject->activeState, MInputMethod::OnScreen);

    // Make sure were in landscape mode when in hardware state
    rotateToAngle(M::Angle0);

    subject->setKeyboardState(MInputMethod::Hardware);
    QCOMPARE(subject->activeState, MInputMethod::Hardware);
}

void Ut_SymbolView::testSetTemporarilyHidden()
{
    QCOMPARE(subject->activity, SymbolView::Inactive);

    subject->setTemporarilyHidden(true);
    QCOMPARE(subject->activity, SymbolView::Inactive);

    subject->setTemporarilyHidden(false);
    QCOMPARE(subject->activity, SymbolView::Inactive);

    subject->showSymbolView();

    QTest::qWait(200);

    QCOMPARE(subject->activity, SymbolView::Active);

    subject->setTemporarilyHidden(false);
    QCOMPARE(subject->activity, SymbolView::Active);

    subject->setTemporarilyHidden(true);
    QCOMPARE(subject->activity, SymbolView::TemporarilyInactive);

    QTest::qWait(200);

    subject->setTemporarilyHidden(false);
    QCOMPARE(subject->activity, SymbolView::Active);
}

void Ut_SymbolView::testQuickPick_data()
{
    typedef QList<QPoint> KeyList;
    QTest::addColumn<KeyList>("keyList");
    QTest::addColumn<bool>("expectedIsActive");

    const QPoint quickPickKey(0, 0); // '@'
    const QPoint normalKey(1, 0);

    QTest::newRow("Normal key click, symbol view stays open.")
        << (KeyList() << normalKey) << true;

    QTest::newRow("Quick key click, symbol view closes")
        << (KeyList() << quickPickKey) << false;

    // Symbol view doesn't close if quick pick key is not the first
    // to be clicked.
    QTest::newRow("Quick pick not first 1") << (KeyList() << normalKey << quickPickKey) << true;
    QTest::newRow("Quick pick not first 2") << (KeyList() << normalKey << normalKey << quickPickKey) << true;
}

void Ut_SymbolView::testQuickPick()
{
    QFETCH(QList<QPoint>, keyList);
    QFETCH(bool, expectedIsActive);

    MImAbstractKeyArea *page = static_cast<MImAbstractKeyArea *>(subject->pageSwitcher->widget(0));

    subject->showSymbolView();
    QCOMPARE(subject->isActive(), true);

    foreach (QPoint keyLocation, keyList) {
        MImAbstractKey *key = keyAt(page, keyLocation.y(), keyLocation.x());
        QVERIFY(key);

        page->click(key);
    }

    QCOMPARE(subject->isActive(), expectedIsActive);
}

void Ut_SymbolView::rotateToAngle(M::OrientationAngle angle)
{
    subject->prepareToOrientationChange();
    MPlainWindow::instance()->setOrientationAngle(angle);
    QTest::qWait(SceneRotationTime);// wait until MSceneManager::orientationAngle() is updated.
    subject->finalizeOrientationChange();
}

// Helper method to get key in certain row and column from current subject.
MImAbstractKey *Ut_SymbolView::keyAt(MImAbstractKeyArea *symPage,
                                     unsigned int row,
                                     unsigned int column) const
{
    Q_ASSERT(symPage
             && (row < static_cast<unsigned int>(symPage->rowCount()))
             && (column < static_cast<unsigned int>(symPage->sectionModel()->columnsAt(row))));

    MImAbstractKey *key = 0;

    MImKeyArea *buttonArea = dynamic_cast<MImKeyArea *>(symPage);
    if (buttonArea) {
        key = buttonArea->rowList[row].keys[column];
    }

    return key;
}


QTEST_APPLESS_MAIN(Ut_SymbolView);
