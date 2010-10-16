/* * This file is part of m-keyboard *
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

#include "ut_keybuttonarea.h"
#include "singlewidgetbuttonarea.h"
#include "singlewidgetbutton.h"
#include "flickgesturerecognizer.h"
#include "../ut_flickrecognizer/flickutil.h"
#include "keyboarddata.h"
#include "vkbdatakey.h"
#include "mplainwindow.h"
#include "popupbase.h"
#include "utils.h"

#include <MApplication>
#include <MScene>
#include <MSceneManager>
#include <MTheme>

#include <QDir>
#include <QGraphicsLayout>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>

#include <algorithm>

Q_DECLARE_METATYPE(KeyEvent);
Q_DECLARE_METATYPE(IKeyButton*);
Q_DECLARE_METATYPE(const IKeyButton*);

typedef KeyButtonArea *(*KBACreator)(const LayoutData::SharedLayoutSection &section,
                                     bool usePopup,
                                     QGraphicsWidget *parent);

Q_DECLARE_METATYPE(KBACreator);
Q_DECLARE_METATYPE(IKeyButton::ButtonState);
Q_DECLARE_METATYPE(QList<KeyBinding::KeyAction>);
Q_DECLARE_METATYPE(Ut_KeyButtonArea::TestOpList);

KeyButtonArea *createSingleWidgetKeyButtonArea(const LayoutData::SharedLayoutSection &section,
                                               bool usePopup = false,
                                               QGraphicsWidget *parent = 0)
{
    return new SingleWidgetButtonArea(section, usePopup, parent);
}


void Ut_KeyButtonArea::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_keybuttonarea",
                                 (char *) "-local-theme" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);

    qRegisterMetaType<KeyEvent>();
    qRegisterMetaType<const IKeyButton*>();
    qRegisterMetaType<IKeyButton*>();
    qRegisterMetaType<IKeyButton::ButtonState>();
    qRegisterMetaType<TestOpList>("TestOpList");

    createMScene(new MPlainWindow); // also create singleton

    FlickGestureRecognizer::registerSharedRecognizer();

    MPlainWindow::instance()->grabGesture(FlickGestureRecognizer::sharedGestureType());
    MPlainWindow::instance()->show();
    QTest::qWaitForWindowShown(MPlainWindow::instance());
}

void Ut_KeyButtonArea::cleanupTestCase()
{
    FlickGestureRecognizer::unregisterSharedRecognizer();
    delete MPlainWindow::instance();
    delete app;
    app = 0;
}

void Ut_KeyButtonArea::init()
{
    subject = 0;
    keyboard = 0;
}

void Ut_KeyButtonArea::cleanup()
{
    delete subject;
    delete keyboard;
    keyboard = 0;
}

void Ut_KeyButtonArea::testLandscapeBoxSize_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testLandscapeBoxSize()
{
    QFETCH(KBACreator, createKba);

    QSize box;
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    int fileCount = 0;

    changeOrientation(M::Angle0);

    filters << "??.xml";
    foreach(QFileInfo info, dir.entryInfoList(filters)) {
        delete subject;
        subject = 0;
        delete keyboard;
        keyboard = new KeyboardData;
        qDebug() << "Loading layout file" << info.fileName();
        QVERIFY(keyboard->loadNokiaKeyboard(info.fileName()));
        subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                            false, 0);
        MPlainWindow::instance()->scene()->addItem(subject);

        subject->resize(defaultLayoutSize());

        for (int level = 0; level < 2; level++) {
            subject->switchLevel(level);

            box = keyAt(0, 0)->buttonRect().size().toSize();
            QVERIFY(subject->sectionModel()->rowCount() > 0);
            QVERIFY(subject->sectionModel()->columnsAt(0) > 0);
            qDebug() << "Current level" << level << "; Box size=" << box;
            QVERIFY(box.height() >= 0 && box.height() <= (280/subject->sectionModel()->rowCount()));
            QVERIFY(box.width() >= 00 && box.width() <= (864/subject->sectionModel()->columnsAt(0)));
        }
        ++fileCount;
    }
    QVERIFY(fileCount > 0);
}

void Ut_KeyButtonArea::testPortraitBoxSize_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testPortraitBoxSize()
{
    QFETCH(KBACreator, createKba);

    QSize box;
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    int fileCount = 0;

    changeOrientation(M::Angle90);

    filters << "??.xml";
    foreach(QFileInfo info, dir.entryInfoList(filters)) {
        delete subject;
        subject = 0;
        delete keyboard;
        keyboard = new KeyboardData;
        qDebug() << "Loading layout file" << info.fileName();
        QVERIFY(keyboard->loadNokiaKeyboard(info.fileName()));
        subject = createKba(keyboard->layout(LayoutData::General, M::Portrait)->section(LayoutData::mainSection),
                            false, 0);
        MPlainWindow::instance()->scene()->addItem(subject);

        subject->resize(defaultLayoutSize());
        for (int level = 0; level < 2; level++) {
            subject->switchLevel(level);

            box = keyAt(0, 0)->buttonRect().size().toSize();
            qDebug() << "Current level" << level << "; Box size=" << box << subject->size();
            QVERIFY(box.height() >= 50 && box.height() <= 80);
            QVERIFY(box.width() >= 30 && box.width() <= 50);
        }
        ++fileCount;
    }
    QVERIFY(fileCount > 0);
}

void Ut_KeyButtonArea::testLabelPosition_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testLabelPosition()
{
    QFETCH(KBACreator, createKba);

    QList<QPoint> positions;
    QList<const VKBDataKey *> outcome;
    QSize buttonSize;
    int startX;
    const int rowIndex = 2;
    const IKeyButton *button = 0;

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);

    subject->resize(defaultLayoutSize());

    const IKeyButton *firstButton = keyAt(0, 0);

    button = keyAt(2, 0); // Third row's first button
    buttonSize = button->buttonBoundingRect().size().toSize();
    startX = button->buttonBoundingRect().x();
    positions << QPoint(-50, -50)                       // not on any key
              << firstButton->buttonRect().topLeft().toPoint() + QPoint(-1, -1) // reactive margin should get this
              << QPoint(-buttonSize.width(), 0)         // not on any key
              << QPoint(0, -buttonSize.height())        // not on any key
              << QPoint(1, 100 * buttonSize.height())   // not on any key
              << QPoint(startX + buttonSize.width() + subject->style()->paddingLeft() + subject->style()->spacingHorizontal()*2 + 1,
                        subject->style()->paddingTop() + (buttonSize.height() + subject->style()->spacingVertical()) * rowIndex + 1)
              << QPoint(startX + buttonSize.width() * 100 + 1, buttonSize.height() * rowIndex + 1); // not on any key
    outcome << 0
            << subject->section->vkbKey(0, 0)
            << 0
            << 0
            << 0
            << subject->section->vkbKey(rowIndex, 1)
            << 0;
    QVERIFY(positions.count() == outcome.count());
    for (int n = 0; n < positions.count(); ++n) {
        qDebug() << "test position" << positions.at(n);
        button = subject->keyAt(positions.at(n));
        const VKBDataKey *result = (button ? &button->key() : 0);
        QCOMPARE(result, outcome.at(n));
    }
}

void Ut_KeyButtonArea::testSceneEvent_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testSceneEvent()
{
    QFETCH(KBACreator, createKba);

    //initialization
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    // Skip test for multitouch, since there are no mouse events:
    if (subject->acceptTouchEvents()) {
        return;
    }

    QGraphicsSceneMouseEvent *press = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMousePress);
    QGraphicsSceneMouseEvent *release = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseRelease);
    QGraphicsSceneMouseEvent *move = new QGraphicsSceneMouseEvent(QEvent::GraphicsSceneMouseMove);
    QSignalSpy spy(subject, SIGNAL(keyClicked(const IKeyButton*, const QString&, bool)));
    QSignalSpy spyPressed(subject, SIGNAL(keyPressed(const IKeyButton*, const QString&, bool)));

    QVERIFY(spy.isValid());
    QVERIFY(spyPressed.isValid());

    press->setPos(QPoint(subject->style()->paddingTop() + 1, subject->style()->paddingLeft() + 1));
    release->setPos(QPoint(subject->style()->paddingTop() + 10, subject->style()->paddingLeft() + 10));
    move->setPos(QPoint(subject->style()->paddingTop() + 10, subject->style()->paddingLeft() + 10));

    //actual testing
    subject->sceneEvent(press);
    QCOMPARE(spyPressed.count(), 1);
    QCOMPARE(spy.count(), 0);
    subject->sceneEvent(move);
    QCOMPARE(spy.count(), 0);
    subject->sceneEvent(release);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spyPressed.count(), 1);

    delete press;
    delete move;
    delete release;
}

void Ut_KeyButtonArea::testPaint_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testPaint()
{
    QFETCH(KBACreator, createKba);

    //at least we should not chrash here
    QImage *image = new QImage(QSize(864, 480), QImage::Format_ARGB32_Premultiplied);
    QPainter painter;
    QVERIFY(painter.begin(image));

    //initialization
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        false, 0);
    subject->resize(defaultLayoutSize());
    MPlainWindow::instance()->scene()->addItem(subject);

    subject->touchPointPressed(QPoint(20, 20), 0); // top left button
    //actual testing
    subject->paint(&painter, 0, 0);
}

void Ut_KeyButtonArea::testDeadkeys_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testDeadkeys()
{
    QFETCH(KBACreator, createKba);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("fr.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());
    QSignalSpy spy(subject, SIGNAL(keyClicked(const IKeyButton*, const QString&, bool)));
    IKeyButton *key = 0;
    QList<int> positions;
    int i;
    positions << 0 << 1 << 2 << 5 << 6;

    QVERIFY(spy.isValid());

    //!!! z is the character that won't be changed when deadkey is locked
    QStringList lowerUnicodes;
    lowerUnicodes << QChar('a') << QChar('z') << QChar('e') << QChar('y') << QChar('u');
    QStringList upperUnicodes; //upper case
    foreach(QString c, lowerUnicodes)
    upperUnicodes << c.toUpper();

    //Unicode for alphas with deadkeys:  á << Unicod é  unicode for  ý<<< unicode for  ú
    QStringList lowerDKUnicodes;
    lowerDKUnicodes << QChar(0x00e1) << QChar('z') << QChar(0x00e9) << QChar(0x00fd) << QChar(0x00fa);
    QStringList upperDKUnicodes; // upper case
    foreach(QString c, lowerDKUnicodes)
    upperDKUnicodes << c.toUpper();

    //test for unlock deadkey status
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }

    key = keyAt(2, 8); // row 3, column 9
    QVERIFY(key != 0);
    QVERIFY(key->isDeadKey());
    QString c = QChar(0x00B4);
    QCOMPARE(key->label(), c);
    subject->click(key);
    //click at deadkey for the first time, just lock the deadkey, won't emit cliked() signal
    QCOMPARE(spy.count(), 0);

    //check the alphas, should be with deadkey
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerDKUnicodes.at(i));
    }

    //test for shift status
    subject->switchLevel(1);
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), upperDKUnicodes.at(i));
    }

    //after unlock the dead key, test the shift status
    subject->unlockDeadkeys();
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), upperUnicodes.at(i));
    }

    //test for shift off status
    subject->switchLevel(0);
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }

    // Lock deadkey again.
    subject->click(key);
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerDKUnicodes.at(i));
    }

    subject->click(keyAt(0, 0));
    //key release on not deadkey, will emit clicked() signal
    QCOMPARE(spy.count(), 1);
    //any keypress, the deadkey should be unlocked
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }
}

void Ut_KeyButtonArea::testSelectedDeadkeys()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("fr.xml"));
    subject = createSingleWidgetKeyButtonArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                                              false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);

    // Pick two deadkeys to play around with.
    IKeyButton *deadkey1 = keyAt(2, 8); // row 3, column 7
    IKeyButton *deadkey2 = keyAt(2, 9); // row 3, column 8
    IKeyButton *regularKey = keyAt(0, 0); // first key, top left

    QVERIFY(deadkey1 && deadkey1->isDeadKey());
    QVERIFY(deadkey2 && deadkey2->isDeadKey());
    QVERIFY(regularKey && !regularKey->isDeadKey());

    QCOMPARE(deadkey1->state(), IKeyButton::Normal);
    QCOMPARE(deadkey2->state(), IKeyButton::Normal);
    QCOMPARE(regularKey->state(), IKeyButton::Normal);

    // Press dead key down
    subject->click(deadkey1);
    QCOMPARE(deadkey1->state(), IKeyButton::Selected);

    // Release it by clicking regular key
    subject->click(regularKey);
    QCOMPARE(deadkey1->state(), IKeyButton::Normal);

    // Down again
    subject->click(deadkey1);
    QCOMPARE(deadkey1->state(), IKeyButton::Selected);

    // Release it by clicking itself again.
    subject->click(deadkey1);
    QCOMPARE(deadkey1->state(), IKeyButton::Normal);

    // Down again
    subject->click(deadkey1);
    QCOMPARE(deadkey1->state(), IKeyButton::Selected);

    // Release it by clicking the other dead key.
    subject->click(deadkey2);
    QCOMPARE(deadkey1->state(), IKeyButton::Normal);
    QCOMPARE(deadkey2->state(), IKeyButton::Selected);
}

void Ut_KeyButtonArea::testTwoDeadInOne_data()
{
    QTest::addColumn<TestOpList>("operations");
    QTest::addColumn<QString>("expectedCharacterLabel");

    QTest::newRow("no dead key")
        << TestOpList() << "e";
    QTest::newRow("dead key clicked")
        << (TestOpList() << TestOpClickDeadKey) << QString(L'é');
    QTest::newRow("click dead key, shift on")
        << (TestOpList() << TestOpClickDeadKey << TestOpSetShiftOn) << QString(L'É');
    QTest::newRow("shift on, click dead key")
        << (TestOpList() << TestOpSetShiftOn << TestOpClickDeadKey) << QString(L'Ë');
    QTest::newRow("shift on, click dead key, shift off")
        << (TestOpList() << TestOpSetShiftOn << TestOpClickDeadKey << TestOpSetShiftOff) << QString(L'ë');
}

void Ut_KeyButtonArea::testTwoDeadInOne()
{
    QFETCH(TestOpList, operations);
    QFETCH(QString, expectedCharacterLabel);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("test-layout.xml"));
    subject = createSingleWidgetKeyButtonArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                                              false, 0);

    IKeyButton *deadkey = keyAt(2, 8); // accents ´ and ¨
    IKeyButton *characterKey = keyAt(0, 2); // e, éë, ÉË

    QVERIFY(deadkey);
    QVERIFY(deadkey->key().binding(false)->isDead());
    QVERIFY(deadkey->key().binding(true)->isDead());

    foreach (TestOperation op, operations) {
        switch (op) {
        case TestOpClickDeadKey:
            {
                KeyButtonArea::TouchPointInfo tpi;
                subject->click(deadkey);
            }
            break;
        case TestOpSetShiftOn:
            subject->switchLevel(1);
            break;
        case TestOpSetShiftOff:
            subject->switchLevel(0);
            break;
        default:
            QFAIL("Unsupported operation");
            break;
        }
    }

    QCOMPARE(characterKey->label(), expectedCharacterLabel);
}

void Ut_KeyButtonArea::testExtendedLabels()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("test-layout.xml"));
    subject = createSingleWidgetKeyButtonArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                                              false, 0);

    const IKeyButton *eKey(keyAt(0, 2)); // e, ...
    QCOMPARE(eKey->key().binding(false)->extendedLabels(), QString("%1%2").arg(QChar(0xea)).arg(QChar(0xe8)));
    QCOMPARE(eKey->key().binding(true)->extendedLabels(), QString("%1%2").arg(QChar(0xca)).arg(QChar(0xc8)));
}

void Ut_KeyButtonArea::testImportedLayouts_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testImportedLayouts()
{
    QFETCH(KBACreator, createKba);

    // This test uses files test-importer.xml, test-import1.xml, and test-import2.xml.
    // The first imported file test-import1.xml defines some landscape and portrait stuff, while
    // the second imported file test-import2.xml redefines the portrait stuff.

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("test-importer.xml"));
    const LayoutData *model = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(model);
    subject = createKba(model->section(LayoutData::mainSection),
                        false, 0);

    // Check that language and title information are not overwritten by imported files.
    QCOMPARE(keyboard->title(), QString("test-importer-title"));
    QCOMPARE(keyboard->language(), QString("test-importer-language"));

    // First imported defines landscape layout with key labeled "1".
    // Second one doesn't define landscape layout.
    changeOrientation(M::Angle0);
    QCOMPARE(keyAt(0, 0)->label(), QString("1"));
    QCOMPARE(model->section(LayoutData::mainSection)->vkbKey(1, 0)->binding(false)->label(),
             QString("func1"));

    // Second imported defines portrait layout with key labeled "2"
    delete subject;
    model = keyboard->layout(LayoutData::General, M::Portrait);
    subject = createKba(model->section(LayoutData::mainSection),
                        false, 0);
    changeOrientation(M::Angle90);
    QCOMPARE(keyAt(0, 0)->label(), QString("2"));
    QCOMPARE(model->section(LayoutData::mainSection)->vkbKey(1, 0)->binding(false)->label(),
             QString("func2"));
}

void Ut_KeyButtonArea::testPopup_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testPopup()
{
    QFETCH(KBACreator, createKba);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        true, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    const QPoint mousePos(subject->style()->paddingTop() + 1, subject->style()->paddingLeft() + 1); // approximately the top left key on layout
    const int touchId = 0;
    QVERIFY(&subject->popup());

    // Test popup activation
    // direct call
    // Popup won't show up unless it is given a position. We give it via a mouse press.
    //popup should be shown immediately, so we comment out this part of test
#if 0
    subject->touchPointPressed(mousePos, touchId);
    subject->popupStart();
    QVERIFY(subject->isPopupActive());
    subject->popup->hidePopup();
    QVERIFY(!subject->isPopupActive());
    subject->touchPointReleased(mousePos, touchId);

    subject->touchPointPressed(mousePos, touchId);
    QVERIFY(!subject->isPopupActive());
    subject->touchPointReleased(mousePos, touchId);
#endif

    subject->touchPointPressed(mousePos, touchId);
    QVERIFY(subject->popup().isVisible());
    subject->touchPointReleased(mousePos, touchId);
}

void Ut_KeyButtonArea::testInitialization_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::newRow("SingleWidgetArea") << &createSingleWidgetKeyButtonArea;
}

void Ut_KeyButtonArea::testInitialization()
{
    QFETCH(KBACreator, createKba);
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    subject = createKba(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                        false, 0);
    subject->resize(defaultLayoutSize());

}

void Ut_KeyButtonArea::testShiftCapsLock()
{
    // Load any layout that has shift
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    const LayoutData *layout = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(layout);
    const LayoutData::SharedLayoutSection section = layout->section(LayoutData::mainSection);

    subject = createSingleWidgetKeyButtonArea(section,
                                              false, 0);

    SingleWidgetButton *shiftButton = static_cast<SingleWidgetButtonArea *>(subject)->shiftButton;
    QVERIFY(shiftButton);
    QVERIFY(shiftButton->state() == IKeyButton::Normal);

    subject->setShiftState(ModifierLockedState);
    QVERIFY(shiftButton->state() == IKeyButton::Selected);

    subject->setShiftState(ModifierClearState);
    QVERIFY(shiftButton->state() == IKeyButton::Normal);
}

void Ut_KeyButtonArea::testMultiTouch()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));
    const LayoutData *layout = keyboard->layout(LayoutData::General, M::Landscape);
    QVERIFY(layout);
    const LayoutData::SharedLayoutSection section = layout->section(LayoutData::mainSection);

    subject = createSingleWidgetKeyButtonArea(section,
                                              false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    const IKeyButton *key0 = keyAt(0, 0);
    const IKeyButton *key1 = keyAt(1, 0);
    const IKeyButton *key2 = keyAt(0, 1);

    QVERIFY(key0);
    QVERIFY(key1);
    QVERIFY(key2);

    QSignalSpy pressed(subject, SIGNAL(keyPressed(const IKeyButton*, const QString&, bool)));
    QSignalSpy released(subject, SIGNAL(keyReleased(const IKeyButton*, const QString&, bool)));
    QSignalSpy clicked(subject, SIGNAL(keyClicked(const IKeyButton*, const QString&, bool )));

    QVERIFY(pressed.isValid());
    QVERIFY(released.isValid());
    QVERIFY(clicked.isValid());

    const QPoint pos0 = key0->buttonRect().center().toPoint();
    const QPoint pos1 = key1->buttonRect().center().toPoint();
    const QPoint pos2 = key2->buttonRect().center().toPoint();

    /*
     * Verify following conditions:
     * 1) signals are emitted in correct order
     * 2) every signal corresponds to correct key
     */
    subject->touchPointPressed(pos0, 0);
    QCOMPARE(pressed.count(), 1);
    QVERIFY(pressed.at(0).first().value<const IKeyButton*>() == key0);
    QCOMPARE(released.count(), 0);
    QCOMPARE(clicked.count(), 0);

    subject->touchPointPressed(pos1, 1);
    QCOMPARE(pressed.count(), 2);
    QVERIFY(pressed.at(1).first().value<const IKeyButton*>() == key1);
    QCOMPARE(released.count(), 0);
    QCOMPARE(clicked.count(), 0);

    subject->touchPointReleased(pos0, 0);
    subject->touchPointReleased(pos1, 1);
    QCOMPARE(pressed.count(), 2);
    QCOMPARE(released.count(), 2);
    QVERIFY(released.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(released.at(1).first().value<const IKeyButton*>() == key1);
    QCOMPARE(clicked.count(), 2);
    QVERIFY(clicked.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(clicked.at(1).first().value<const IKeyButton*>() == key1);

    pressed.clear();
    released.clear();
    clicked.clear();

    // Verify if could click on some keys while other key is pressed
    subject->touchPointPressed(pos0, 0);
    subject->touchPointPressed(pos1, 1);
    subject->touchPointReleased(pos0, 0);
    subject->touchPointPressed(pos2, 0);
    subject->touchPointReleased(pos2, 0);
    subject->touchPointReleased(pos1, 1);

    QCOMPARE(pressed.count(), 3);
    QCOMPARE(released.count(), 3);
    QCOMPARE(clicked.count(), 3);

    QVERIFY(pressed.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(pressed.at(1).first().value<const IKeyButton*>() == key1);
    QVERIFY(pressed.at(2).first().value<const IKeyButton*>() == key2);

    QVERIFY(released.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(released.at(1).first().value<const IKeyButton*>() == key2);
    QVERIFY(released.at(2).first().value<const IKeyButton*>() == key1);

    QVERIFY(clicked.at(0).first().value<const IKeyButton*>() == key0);
    QVERIFY(clicked.at(1).first().value<const IKeyButton*>() == key2);
    QVERIFY(clicked.at(2).first().value<const IKeyButton*>() == key1);
}

void Ut_KeyButtonArea::testRtlKeys_data()
{
    QTest::addColumn<KBACreator>("createKba");
    QTest::addColumn<M::Orientation>("orientation");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QList<KeyBinding::KeyAction> >("expectedRtlKeys");

    const QString ar("ar.xml");
    const QString en_gb("en_gb.xml");
    QList<KeyBinding::KeyAction> rtlKeys;
    const QList<KeyBinding::KeyAction> nothing;

    rtlKeys << KeyBinding::ActionBackspace;

    QTest::newRow("SingleWidgetArea Landscape Arabic")
        << &createSingleWidgetKeyButtonArea
        << M::Landscape
        << ar
        << rtlKeys;

    QTest::newRow("SingleWidgetArea Portrait Arabic" )
        << &createSingleWidgetKeyButtonArea
        << M::Portrait
        << ar
        << rtlKeys;

    QTest::newRow("SingleWidgetArea Landscape English")
        << &createSingleWidgetKeyButtonArea
        << M::Landscape
        << en_gb
        << nothing;

    QTest::newRow("SingleWidgetArea Portrait English" )
        << &createSingleWidgetKeyButtonArea
        << M::Portrait
        << en_gb
        << nothing;
}

void Ut_KeyButtonArea::testRtlKeys()
{
    QFETCH(KBACreator, createKba);
    QFETCH(M::Orientation, orientation);
    QFETCH(QString, fileName);
    QFETCH(QList<KeyBinding::KeyAction>, expectedRtlKeys);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(fileName));
    subject = createKba(keyboard->layout(LayoutData::General, orientation)->section(LayoutData::mainSection),
                        false, 0);

    SingleWidgetButtonArea *buttonArea = dynamic_cast<SingleWidgetButtonArea *>(subject);

    QVERIFY2(buttonArea != 0, "Unknown type of button area");
    for (int row = 0; row < buttonArea->rowCount(); ++row) {
        for (int column = 0; column < buttonArea->sectionModel()->columnsAt(row); ++column) {
            SingleWidgetButton *button = buttonArea->rowList[row].buttons[column];
            QVERIFY(button != 0);
            if (expectedRtlKeys.contains(button->key().binding()->action())) {
                QVERIFY(button->key().rtl());
                QVERIFY2(button->iconId().contains("-rtl-"), "This is not RTL icon");
            } else if (!button->iconId().isEmpty()) {
                QVERIFY(!button->key().rtl());
                QVERIFY2(!button->iconId().contains("-rtl-"), "This is not LTR icon");
            }
        }
    }
}

void Ut_KeyButtonArea::testLongKeyPress()
{
    const int LongPressTimeOut = 600; //this value depends on timeout of long press

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard("en_us.xml"));

    subject = createSingleWidgetKeyButtonArea(keyboard->layout(LayoutData::General, M::Landscape)->section(LayoutData::mainSection),
                                              true, 0);

    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    SingleWidgetButton *key0 = dynamic_cast<SingleWidgetButton *>(keyAt(0, 0));
    SingleWidgetButton *key1 = dynamic_cast<SingleWidgetButton *>(keyAt(1, 3));

    QVERIFY(key0);
    QVERIFY(key1);

    QPoint point0 = key0->buttonBoundingRect().center().toPoint();
    QPoint point1 = key1->buttonBoundingRect().center().toPoint();

    QSignalSpy spy(subject, SIGNAL(longKeyPressed(const IKeyButton*, const QString &, bool)));

    QVERIFY(spy.isValid());

    // click is not long press
    subject->touchPointPressed(point0, 0);
    subject->touchPointReleased(point0, 0);
    QVERIFY(spy.isEmpty());
    QTest::qWait(LongPressTimeOut);
    QVERIFY(spy.isEmpty());

    // long press on the key
    subject->touchPointPressed(point0, 0);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const IKeyButton *>(), key0);
    spy.clear();
    subject->touchPointReleased(point0, 0);

    // long press with multitouch
    subject->touchPointPressed(point0, 0);
    subject->touchPointPressed(point1, 1);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const IKeyButton *>(), key1);
    spy.clear();
    subject->touchPointReleased(point0, 0);
    subject->touchPointReleased(point1, 1);
    QVERIFY(spy.isEmpty());

    // long press after movement
    subject->touchPointPressed(point0, 0);
    subject->touchPointMoved(point1, 0);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const IKeyButton *>(), key1);
    spy.clear();
    subject->touchPointReleased(point0, 0);
    QVERIFY(spy.isEmpty());

    // long press should be detected if last pressed key is not released
    subject->touchPointPressed(point0, 0);
    subject->touchPointPressed(point1, 1);
    subject->touchPointReleased(point0, 0);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const IKeyButton *>(), key1);
    spy.clear();
    subject->touchPointReleased(point1, 1);
}

void Ut_KeyButtonArea::changeOrientation(M::OrientationAngle angle)
{
    if (MPlainWindow::instance()->orientationAngle() != angle) {
        MPlainWindow::instance()->setOrientationAngle(angle);
        //timeout depends on duration of orientation animation
        QTest::qWait(1000);
    }
}

QSize Ut_KeyButtonArea::defaultLayoutSize()
{
    // Take visible scene size as layout size, but reduce keyboard's paddings first from its width.
    // The height value is ignored since KeyButtonAreas determine their own height.
    return MPlainWindow::instance()->visibleSceneSize()
            - QSize(subject->style()->paddingLeft() + subject->style()->paddingRight(), 0);
}

// Helper method to get key in certain row and column from current subject.
IKeyButton *Ut_KeyButtonArea::keyAt(unsigned int row, unsigned int column) const
{
    // If this fails there is something wrong with the test.
    Q_ASSERT(subject
             && (row < static_cast<unsigned int>(subject->rowCount()))
             && (column < static_cast<unsigned int>(subject->sectionModel()->columnsAt(row))));

    IKeyButton *key = 0;

    SingleWidgetButtonArea *buttonArea = dynamic_cast<SingleWidgetButtonArea *>(subject);
    if (buttonArea) {
        key = buttonArea->rowList[row].buttons[column];
    }

    return key;
}

QTEST_APPLESS_MAIN(Ut_KeyButtonArea);
