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

#include "ut_mimabstractkeyarea.h"
#include "mimabstractkeyarea_p.h"
#include "mimkeyarea.h"
#include "mimkeyarea_p.h"
#include "mimkey.h"
#include "flickgesturerecognizer.h"
#include "keyboarddata.h"
#include "mimkeymodel.h"
#include "mplainwindow.h"
#include "mimabstractpopup.h"
#include "magnifierhost.h"
#include "utils.h"
#include "regiontracker.h"
#include "reactionmappainter.h"

#include <MApplication>
#include <MScene>
#include <MSceneWindow>
#include <MSceneManager>
#include <MTheme>
#include <MNamespace>

#include <QDir>
#include <QGraphicsLayout>
#include <QGraphicsSceneMouseEvent>
#include <QTouchEvent>

#include <algorithm>
#include <memory>

Q_DECLARE_METATYPE(KeyEvent);
Q_DECLARE_METATYPE(MImAbstractKey*);
Q_DECLARE_METATYPE(const MImAbstractKey*);

typedef QList<QPointF> PointList;
Q_DECLARE_METATYPE(PointList);

namespace
{
    const QString TestLayoutFilePath = "/usr/lib/meego-keyboard-tests/layouts/";
};

typedef QTouchEvent::TouchPoint (*TpCreator)(int id,
                                             Qt::TouchPointState state,
                                             const QPointF &scenePos,
                                             const QPointF &lastScenePos);

typedef QList<QTouchEvent::TouchPoint> TpList;
typedef QList<MImAbstractKey *> ButtonList;
typedef QList<MImAbstractKey::ButtonState> ButtonStateList;
typedef QList<ButtonStateList> TpButtonStateMatrix;

Q_DECLARE_METATYPE(TpCreator);
Q_DECLARE_METATYPE(TpList);
Q_DECLARE_METATYPE(ButtonList);
Q_DECLARE_METATYPE(ButtonStateList);
Q_DECLARE_METATYPE(TpButtonStateMatrix);
Q_DECLARE_METATYPE(QList<int>);
Q_DECLARE_METATYPE(QList<bool>);

Q_DECLARE_METATYPE(MImAbstractKey::ButtonState);
Q_DECLARE_METATYPE(QList<MImKeyBinding::KeyAction>);
Q_DECLARE_METATYPE(Ut_MImAbstractKeyArea::TestOpList);
Q_DECLARE_METATYPE(Ut_MImAbstractKeyArea::TouchOpList);
Q_DECLARE_METATYPE(QSharedPointer<KeyboardData>);

// QTest::touchEvent() does not set any point as primary
// so we use a mocked version of isPrimary().
bool QTouchEvent::TouchPoint::isPrimary() const
{
    return id() == 0;
}

class TestPopup
    : public MImAbstractPopup
{
private:
    bool mVisible;

public:
    explicit TestPopup()
        : mVisible(false)
    {}

    virtual ~TestPopup()
    {}

    //! \reimp
    void setMainArea(MImAbstractKeyArea *)
    {}

    void updatePos(const QPointF &,
                   const QPoint &,
                   const QSize &)
    {}

    void cancel()
    {
        mVisible = false;
    }

    void handleKeyPressedOnMainArea(MImAbstractKey *,
                                    const KeyContext &)
    {
        mVisible = true;
    }

    void handleLongKeyPressedOnMainArea(MImAbstractKey *,
                                        const KeyContext &)
    {
        mVisible = true;
    }

    bool isVisible() const
    {
        return mVisible;
    }

    void setVisible(bool visible)
    {
        mVisible = visible;
    }
    //! \reimp_end
};

void Ut_MImAbstractKeyArea::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_mimabstractkeyarea",
                                 (char *) "-software"};

    disableQtPlugins();
    app = new MApplication(argc, app_name);
    RegionTracker::createInstance();

    qRegisterMetaType<KeyEvent>();
    qRegisterMetaType<const MImAbstractKey*>();
    qRegisterMetaType<MImAbstractKey*>();
    qRegisterMetaType<MImAbstractKey::ButtonState>();
    qRegisterMetaType<TestOpList>("TestOpList");
    qRegisterMetaType<TouchOpList>("TouchOpList");
    qRegisterMetaType<KeyContext>("KeyContext");

    sceneWindow = createMSceneWindow(new MPlainWindow); // also create singleton

    FlickGestureRecognizer::registerSharedRecognizer();

    MPlainWindow::instance()->grabGesture(FlickGestureRecognizer::sharedGestureType());
    MPlainWindow::instance()->show();
    QTest::qWaitForWindowShown(MPlainWindow::instance());
}

void Ut_MImAbstractKeyArea::cleanupTestCase()
{
    FlickGestureRecognizer::unregisterSharedRecognizer();
    delete sceneWindow;
    delete MPlainWindow::instance();
    RegionTracker::destroyInstance();
    delete app;
    app = 0;
}

void Ut_MImAbstractKeyArea::init()
{
    subject = 0;
    keyboard = 0;
}

void Ut_MImAbstractKeyArea::cleanup()
{
    delete subject;
    delete keyboard;
    keyboard = 0;
}

void Ut_MImAbstractKeyArea::testLandscapeBoxSize()
{
    QSize box;
    QDir dir(TestLayoutFilePath);
    QStringList filters;
    int fileCount = 0;

    changeOrientation(MInputMethod::Angle0);

    filters << "??.xml";
    foreach(QFileInfo info, dir.entryInfoList(filters)) {
        delete subject;
        subject = 0;
        delete keyboard;
        keyboard = new KeyboardData;
        qDebug() << "Loading layout file" << info.absoluteFilePath();
        QVERIFY(keyboard->loadNokiaKeyboard(info.absoluteFilePath()));
        subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
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

void Ut_MImAbstractKeyArea::testPortraitBoxSize()
{
    QSize box;
    QDir dir(TestLayoutFilePath);
    QStringList filters;
    int fileCount = 0;

    changeOrientation(MInputMethod::Angle90);

    filters << "??.xml";
    foreach(QFileInfo info, dir.entryInfoList(filters)) {
        delete subject;
        subject = 0;
        delete keyboard;
        keyboard = new KeyboardData;
        qDebug() << "Loading layout file" << info.absoluteFilePath();
        QVERIFY(keyboard->loadNokiaKeyboard(info.absoluteFilePath()));
        subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Portrait)->section(LayoutData::mainSection),
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

void Ut_MImAbstractKeyArea::testPaint()
{
    //at least we should not chrash here
    QImage *image = new QImage(QSize(864, 480), QImage::Format_ARGB32_Premultiplied);
    QPainter painter;
    QVERIFY(painter.begin(image));

    //initialization
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "en_us.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);
    subject->resize(defaultLayoutSize());
    MPlainWindow::instance()->scene()->addItem(subject);

    //actual testing
    subject->paint(&painter, 0, 0);
}

void Ut_MImAbstractKeyArea::testDeadkeys_data()
{
    QTest::addColumn<bool>("setOverride");
    QTest::addColumn<bool>("enabled");

    // TODO: does not really test override flag; it's always true:
    QTest::newRow("SingleWidgetArea enabled key")  << true << true;
    QTest::newRow("SingleWidgetArea disabled key") << true << false;
}

void Ut_MImAbstractKeyArea::testDeadkeys()
{
    QFETCH(bool, setOverride);
    QFETCH(bool, enabled);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-deadkey.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());
    QSignalSpy spy(subject, SIGNAL(keyClicked(const MImAbstractKey*, const KeyContext &)));
    MImAbstractKey *key = 0;
    QList<int> positions;
    int i;
    positions << 0 << 1 << 2 << 5 << 6;

    QSharedPointer<MKeyOverride> override(new MKeyOverride("dead"));
    override->setEnabled(enabled);
    QMap<QString, QSharedPointer<MKeyOverride> > overrides;
    overrides["dead"] = override;

    QStringList *list = 0;

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
    QVERIFY(key == subject->findKey("dead"));
    QVERIFY(key->isDeadKey());
    QString c = QChar(0x00B4);
    QCOMPARE(key->label(), c);
    if (setOverride) {
        subject->setKeyOverrides(overrides);
    }

    //click at deadkey for the first time, just lock the deadkey, won't emit cliked() signal
    clickKey(key);
    QCOMPARE(spy.count(), 0);

    list = enabled ? &lowerDKUnicodes : &lowerUnicodes;
    //check the alphas, should be with deadkey
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), list->at(i));
    }

    //test for shift status
    subject->switchLevel(1);
    list = enabled ? &upperDKUnicodes : &upperUnicodes;
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), list->at(i));
    }

    //after unlock the dead key, test the shift status
    SpecialKeyFinder finder(SpecialKeyFinder::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&finder);

    if (enabled) {
        QVERIFY(finder.deadKey());
        subject->unlockDeadKeys(finder.deadKey());
        for (i = 0; i < positions.count(); i++) {
            QCOMPARE(keyAt(0, positions[i])->label(), upperUnicodes.at(i));
        }
    } else {
        QVERIFY(!finder.deadKey());
    }

    //test for shift off status
    subject->switchLevel(0);
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }

    // Lock deadkey again.
    clickKey(key);
    list = enabled ? &lowerDKUnicodes : &lowerUnicodes;
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), list->at(i));
    }

    clickKey(keyAt(0, 0));
    //key release on not deadkey, will emit clicked() signal
    QCOMPARE(spy.count(), 1);
    //any keypress, the deadkey should be unlocked
    for (i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }
}

void Ut_MImAbstractKeyArea::testSelectedDeadkeys()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-deadkey.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);

    // Pick two deadkeys to play around with.
    MImAbstractKey *deadkey1 = keyAt(0, 10); // row 1, column 11
    MImAbstractKey *deadkey2 = keyAt(1, 10); // row 2, column 11
    MImAbstractKey *regularKey = keyAt(0, 0); // first key, top left

    QVERIFY(deadkey1 && deadkey1->isDeadKey());
    QVERIFY(deadkey2 && deadkey2->isDeadKey());
    QVERIFY(regularKey && !regularKey->isDeadKey());

    QCOMPARE(deadkey1->state(), MImAbstractKey::Normal);
    QCOMPARE(deadkey2->state(), MImAbstractKey::Normal);
    QCOMPARE(regularKey->state(), MImAbstractKey::Normal);

    // Press dead key down
    clickKey(deadkey1);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Selected);

    // Release it by clicking regular key
    clickKey(regularKey);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Normal);

    // Down again
    clickKey(deadkey1);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Selected);

    // Release it by clicking itself again.
    clickKey(deadkey1);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Normal);

    // Down again
    clickKey(deadkey1);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Selected);

    // Release it by clicking the other dead key.
    clickKey(deadkey2);
    QCOMPARE(deadkey1->state(), MImAbstractKey::Normal);
    QCOMPARE(deadkey2->state(), MImAbstractKey::Selected);
}

void Ut_MImAbstractKeyArea::testTwoDeadInOne_data()
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

void Ut_MImAbstractKeyArea::testTwoDeadInOne()
{
    QFETCH(TestOpList, operations);
    QFETCH(QString, expectedCharacterLabel);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-layout.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);

    MImAbstractKey *deadkey = keyAt(2, 8); // accents ´ and ¨
    MImAbstractKey *characterKey = keyAt(0, 2); // e, éë, ÉË

    QVERIFY(deadkey);
    QVERIFY(deadkey->model().binding(false)->isDead());
    QVERIFY(deadkey->model().binding(true)->isDead());

    foreach (TestOperation op, operations) {
        switch (op) {
        case TestOpClickDeadKey:
            clickKey(deadkey);
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

void Ut_MImAbstractKeyArea::testExtendedLabels()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-layout.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);

    const MImAbstractKey *eKey(keyAt(0, 2)); // e, ...
    QCOMPARE(eKey->model().binding(false)->extendedLabels(), QString("%1%2").arg(QChar(0xea)).arg(QChar(0xe8)));
    QCOMPARE(eKey->model().binding(true)->extendedLabels(), QString("%1%2").arg(QChar(0xca)).arg(QChar(0xc8)));
}

void Ut_MImAbstractKeyArea::testKeyId()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-layout.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);

    const MImAbstractKey *enterKey(keyAt(3, 6));
    QCOMPARE(enterKey->model().binding(false)->action(), MImKeyBinding::ActionReturn);
    QCOMPARE(enterKey->model().id(), QString("actionKey"));

    const MImAbstractKey *dotKey(keyAt(3, 5));
    QCOMPARE(dotKey->model().id(), QString());

    MImAbstractKey *found = subject->findKey("invalid-id");
    QVERIFY(found == 0);

    found = subject->findKey("actionKey");
    QVERIFY(found == enterKey);
}

void Ut_MImAbstractKeyArea::testContentType_data()
{
    QTest::addColumn<QSharedPointer<KeyboardData> >("keyboardData");

    QSharedPointer<KeyboardData> keyboardData(new KeyboardData);
    QVERIFY(keyboardData->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-layout.xml")));

    QTest::newRow("first pass") << keyboardData;
    QTest::newRow("second pass") << keyboardData;
}

void Ut_MImAbstractKeyArea::testContentType()
{
    QFETCH(QSharedPointer<KeyboardData>, keyboardData);
    subject = MImKeyArea::create(keyboardData->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);

    const MImAbstractKey *key = subject->findKey("emailUrlKey");
    QVERIFY(key);
    subject->setContentType(MInputMethod::FreeTextContentType);
    QCOMPARE(key->model().binding(false)->label(), QString("z"));
    QCOMPARE(key->model().binding(true)->label(),  QString("Z"));

    subject->setContentType(MInputMethod::EmailContentType);
    QCOMPARE(key->model().binding(false)->label(), QString("@"));
    QCOMPARE(key->model().binding(true)->label(),  QString("@"));

    subject->setContentType(MInputMethod::UrlContentType);
    QCOMPARE(key->model().binding(false)->label(), QString("/"));
    QCOMPARE(key->model().binding(true)->label(),  QString("/"));
}

void Ut_MImAbstractKeyArea::testImportedLayouts()
{
    // This test uses files test-importer.xml, test-import1.xml, and test-import2.xml.
    // The first imported file test-import1.xml defines some landscape and portrait stuff, while
    // the second imported file test-import2.xml redefines the portrait stuff.

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-importer.xml")));
    const LayoutData *model = keyboard->layout(LayoutData::General, MInputMethod::Landscape);
    QVERIFY(model);
    subject = MImKeyArea::create(model->section(LayoutData::mainSection),
                                 false, 0);

    // Check that language and title information are not overwritten by imported files.
    QCOMPARE(keyboard->title(), QString("test-importer-title"));
    QCOMPARE(keyboard->language(), QString("test-importer-language"));

    // First imported defines landscape layout with key labeled "1".
    // Second one doesn't define landscape layout.
    changeOrientation(MInputMethod::Angle0);
    QCOMPARE(keyAt(0, 0)->label(), QString("1"));
    QCOMPARE(model->section(LayoutData::mainSection)->keyModel(1, 0)->binding(false)->label(),
             QString("func1"));

    // Second imported defines portrait layout with key labeled "2"
    delete subject;
    model = keyboard->layout(LayoutData::General, MInputMethod::Portrait);
    subject = MImKeyArea::create(model->section(LayoutData::mainSection),
                                 false, 0);
    changeOrientation(MInputMethod::Angle90);
    QCOMPARE(keyAt(0, 0)->label(), QString("2"));
    QCOMPARE(model->section(LayoutData::mainSection)->keyModel(1, 0)->binding(false)->label(),
             QString("func2"));
}

void Ut_MImAbstractKeyArea::testPopup()
{
    TpCreator createTp = &createTouchPoint;
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "en_us.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);
    subject->setPopup(new TestPopup);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    const QPoint mousePos(subject->style()->paddingTop() + 1, subject->style()->paddingLeft() + 1); // approximately the top left key on layout
    QVERIFY(subject->popup());

    subject->d_ptr->touchPointPressed(createTp(0, Qt::TouchPointPressed,
                                               subject->mapToScene(mousePos),
                                               QPointF()));

    QVERIFY(subject->popup()->isVisible());
    subject->d_ptr->touchPointReleased(createTp(0, Qt::TouchPointReleased,
                                                subject->mapToScene(mousePos),
                                                subject->mapToScene(mousePos)));
}

void Ut_MImAbstractKeyArea::testPopupOwnership()
{
    // Will crash if instance was already created elsewhere, but there is no
    // way to check against that, as ReactionMapPainter::instance() will
    // abort (instead of returning 0), if no instance exists.
    ReactionMapPainter::createInstance();

    QPointer<MagnifierHost> first(new MagnifierHost);
    QPointer<MagnifierHost> second(new MagnifierHost);
    QPointer<MagnifierHost> third(new MagnifierHost);
    subject = createEmptyArea();

    // test ownership transfer:
    subject->setPopup(first.data());
    QCOMPARE(first->parent(), subject);

    subject->setPopup(second.data());
    QCOMPARE(second->parent(), subject);
    QVERIFY(not first);

    // test self-assignment protection:
    subject->setPopup(second.data());
    QVERIFY(second);
    QCOMPARE(second->parent(), subject);

    subject->setPopup(0);
    QVERIFY(not second);

    subject->setPopup(third.data());
    delete subject;
    subject = 0;
    QVERIFY(not third);
}

void Ut_MImAbstractKeyArea::testInitialization()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "en_us.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);
    subject->resize(defaultLayoutSize());
}

void Ut_MImAbstractKeyArea::testShiftCapsLock()
{
    // Load any layout that has shift
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "en_us.xml")));
    const LayoutData *layout = keyboard->layout(LayoutData::General, MInputMethod::Landscape);
    QVERIFY(layout);
    const LayoutData::SharedLayoutSection section = layout->section(LayoutData::mainSection);

    subject = MImKeyArea::create(section, false, 0);

    MImKey *shiftButton = static_cast<MImKeyArea *>(subject)->d_ptr->shiftKey;
    QVERIFY(shiftButton);
    QVERIFY(shiftButton->state() == MImAbstractKey::Normal);

    subject->setShiftState(ModifierLockedState);
    QVERIFY(shiftButton->state() == MImAbstractKey::Selected);

    subject->setShiftState(ModifierClearState);
    QVERIFY(shiftButton->state() == MImAbstractKey::Normal);
}

void Ut_MImAbstractKeyArea::testOverridenKey_data()
{
    QTest::addColumn<QStringList>("keyIds");
    // this item defines whether we should set override or not.
    // Other colums are ignored if this one contains false
    QTest::addColumn<QList<bool> >("override");
    QTest::addColumn<QList<bool> >("enable");
    QTest::addColumn<QList<bool> >("highlight");

    // there are no overriden attributes
    QTest::newRow("no override") << (QStringList() << "key0" << "key1" << "key2")
                                 << (QList<bool>() << false  << false  << false)
                                 << (QList<bool>() << true   << true   << true)
                                 << (QList<bool>() << false  << false  << false);

    // key0 is highlighted, but key1 is not
    QTest::newRow("highlight") << (QStringList() << "key0" << "key1" << "key2")
                               << (QList<bool>() << true   << true  << false)
                               << (QList<bool>() << true   << true   << true)
                               << (QList<bool>() << true   << false  << false);

    // key0 is disabled, but others are enabled
    QTest::newRow("disable key0") << (QStringList() << "key0" << "key1" << "key2")
                                  << (QList<bool>() << true   << true   << false)
                                  << (QList<bool>() << false  << true   << true)
                                  << (QList<bool>() << false  << false  << false);

    // key2 is disabled, but others are enabled
    QTest::newRow("disable key2") << (QStringList() << "key0" << "key1" << "key2")
                                  << (QList<bool>() << false  << true   << true)
                                  << (QList<bool>() << true   << true   << false)
                                  << (QList<bool>() << false  << false  << false);

    // key0 and key2 are disabled, but keyy1 is enabled
    // key2 is highlighted
    QTest::newRow("disable both") << (QStringList() << "key0" << "key1" << "key2")
                                  << (QList<bool>() << true   << false  << true)
                                  << (QList<bool>() << false  << true   << false)
                                  << (QList<bool>() << false  << false  << true);
}

/*
 * This test verifies whether we have proper signals when keys are clicked
 */
void Ut_MImAbstractKeyArea::testOverridenKey()
{
    QFETCH(QStringList, keyIds);
    QFETCH(QList<bool>, override);
    QFETCH(QList<bool>, enable);
    QFETCH(QList<bool>, highlight);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-normalkey.xml")));
    const LayoutData *layout = keyboard->layout(LayoutData::General, MInputMethod::Landscape);
    QVERIFY(layout);
    const LayoutData::SharedLayoutSection section = layout->section(LayoutData::mainSection);

    subject = MImKeyArea::create(section, false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    QMap<QString, QSharedPointer<MKeyOverride> > overrides;

    for (int i = 0; i < keyIds.size(); ++i) {
        if (override.at(i)) {
            QSharedPointer<MKeyOverride> override(new MKeyOverride(keyIds.at(i)));
            overrides[keyIds.at(i)] = override;
        }
    }

    if (!overrides.isEmpty()) {
        subject->setKeyOverrides(overrides);
    }

    // this loop invokes MImKeyArea::updateKeyAttributes() indirectly,
    // so we should not merge it with previous one
    for (int i = 0; i < keyIds.size(); ++i) {
        if (override.at(i)) {
            QSharedPointer<MKeyOverride> override(overrides[keyIds.at(i)]);
            override->setEnabled(enable.at(i));
            override->setHighlighted(highlight.at(i));
        }
    }


    const MImAbstractKey *key0 = keyAt(0, 0);
    const MImAbstractKey *key1 = keyAt(0, 1);
    const MImAbstractKey *key2 = keyAt(1, 1);

    QVERIFY(key0);
    QVERIFY(key1);
    QVERIFY(key2);

    QVERIFY(key0 == subject->findKey("key0"));
    QVERIFY(key1 == subject->findKey("key1"));
    QVERIFY(key2 == subject->findKey("key2"));

    QSignalSpy pressed(subject, SIGNAL(keyPressed(const MImAbstractKey*, const KeyContext &)));
    QSignalSpy released(subject, SIGNAL(keyReleased(const MImAbstractKey*, const KeyContext &)));
    QSignalSpy clicked(subject, SIGNAL(keyClicked(const MImAbstractKey*, const KeyContext &)));

    QVERIFY(pressed.isValid());
    QVERIFY(released.isValid());
    QVERIFY(clicked.isValid());

    int pressExpected = 0;
    int clickExpected = 0;
    int releaseExpected = 0;

    const QPoint pos0 = key0->buttonRect().center().toPoint();
    QTouchEvent::TouchPoint tp0 = createTouchPoint(0, Qt::TouchPointPressed, pos0, QPointF(-1, -1));

    const QPoint pos1 = key1->buttonRect().center().toPoint();
    QTouchEvent::TouchPoint tp1 = createTouchPoint(1, Qt::TouchPointPressed, pos1, QPointF(-1, -1));

    const QPoint pos2 = key2->buttonRect().center().toPoint();
    QTouchEvent::TouchPoint tp2 = createTouchPoint(0, Qt::TouchPointMoved, pos2, pos1);
    QTouchEvent::TouchPoint tp3 = createTouchPoint(0, Qt::TouchPointReleased, pos2, pos2);

    // click key0
    subject->d_ptr->touchPointPressed(tp0);
    if (key0->enabled()) {
        QCOMPARE(pressed.count(), 1);
        QVERIFY(pressed.at(0).first().value<const MImAbstractKey*>() == key0);
    } else {
        QCOMPARE(pressed.count(), 0);
    }
    QCOMPARE(released.count(), 0);
    QCOMPARE(clicked.count(), 0);

    subject->d_ptr->touchPointReleased(tp0);
    if (key0->enabled()) {
        QCOMPARE(pressed.count(), 1);
        QCOMPARE(released.count(), 1);
        QVERIFY(released.at(0).first().value<const MImAbstractKey*>() == key0);
        QCOMPARE(clicked.count(), 1);
        QVERIFY(clicked.at(0).first().value<const MImAbstractKey*>() == key0);
    } else {
        QCOMPARE(pressed.count(), 0);
        QCOMPARE(released.count(), 0);
        QCOMPARE(clicked.count(), 0);
    }

    pressed.clear();
    released.clear();
    clicked.clear();
    pressExpected = 0;
    clickExpected = 0;
    releaseExpected = 0;

    // click key0 and key1
    subject->d_ptr->touchPointPressed(tp0);
    if (key0->enabled()) {
        ++pressExpected;
        QCOMPARE(pressed.count(), pressExpected);
        QVERIFY(pressed.last().first().value<const MImAbstractKey*>() == key0);
    } else {
        QCOMPARE(pressed.count(), pressExpected);
    }
    QCOMPARE(released.count(), 0);
    QCOMPARE(clicked.count(), 0);

    subject->d_ptr->touchPointPressed(tp1);
    ++pressExpected;
    QCOMPARE(pressed.count(), pressExpected);
    QVERIFY(pressed.last().first().value<const MImAbstractKey*>() == key1);
    QCOMPARE(released.count(), 0);
    if (key0->enabled()) {
        ++clickExpected;
        QCOMPARE(clicked.count(), clickExpected);
        QVERIFY(clicked.last().first().value<const MImAbstractKey*>() == key0);
    } else {
        QCOMPARE(clicked.count(), clickExpected);
    }

    subject->d_ptr->touchPointReleased(tp0);
    subject->d_ptr->touchPointReleased(tp1);
    QCOMPARE(pressed.count(), pressExpected);
    QCOMPARE(released.count(), 1);
    QVERIFY(released.at(0).first().value<const MImAbstractKey*>() == key1);
    ++clickExpected;
    QCOMPARE(clicked.count(), clickExpected);
    QVERIFY(clicked.last().first().value<const MImAbstractKey*>() == key1);

    pressed.clear();
    released.clear();
    clicked.clear();
    pressExpected = 0;
    clickExpected = 0;
    releaseExpected = 0;

    // press key0, move touch point to key2 and release it
    subject->d_ptr->touchPointPressed(tp0);
    if (key0->enabled()) {
        ++pressExpected;
        QCOMPARE(pressed.count(), pressExpected);
        QVERIFY(pressed.last().first().value<const MImAbstractKey*>() == key0);
    } else {
        QCOMPARE(pressed.count(), pressExpected);
    }

    subject->d_ptr->touchPointMoved(tp2);
    if (key2->enabled()) {
        ++pressExpected;
        QCOMPARE(pressed.count(), pressExpected);
        QVERIFY(pressed.last().first().value<const MImAbstractKey*>() == key2);
    } else {
        QCOMPARE(pressed.count(), pressExpected);
    }

    subject->d_ptr->touchPointReleased(tp3);
    if (key2->enabled()) {
        ++releaseExpected;
        QCOMPARE(released.count(), releaseExpected);
        QVERIFY(released.last().first().value<const MImAbstractKey*>() == key2);
        ++clickExpected;
        QCOMPARE(clicked.count(), clickExpected);
        QVERIFY(clicked.last().first().value<const MImAbstractKey*>() == key2);
    } else {
        QCOMPARE(released.count(), releaseExpected);
        QCOMPARE(clicked.count(), clickExpected);
    }
}

void Ut_MImAbstractKeyArea::testRtlKeys_data()
{
    QTest::addColumn<MInputMethod::Orientation>("orientation");
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QList<MImKeyBinding::KeyAction> >("expectedRtlKeys");

    const QString ar(QString(TestLayoutFilePath + "ar.xml"));
    const QString en_gb(QString(TestLayoutFilePath + "en_gb.xml"));
    QList<MImKeyBinding::KeyAction> rtlKeys;
    const QList<MImKeyBinding::KeyAction> nothing;

    rtlKeys << MImKeyBinding::ActionBackspace;

    QTest::newRow("SingleWidgetArea Landscape Arabic")
        << MInputMethod::Landscape
        << ar
        << rtlKeys;

    QTest::newRow("SingleWidgetArea Portrait Arabic" )
        << MInputMethod::Portrait
        << ar
        << rtlKeys;

    QTest::newRow("SingleWidgetArea Landscape English")
        << MInputMethod::Landscape
        << en_gb
        << nothing;

    QTest::newRow("SingleWidgetArea Portrait English" )
        << MInputMethod::Portrait
        << en_gb
        << nothing;
}

void Ut_MImAbstractKeyArea::testRtlKeys()
{
    QFETCH(MInputMethod::Orientation, orientation);
    QFETCH(QString, fileName);
    QFETCH(QList<MImKeyBinding::KeyAction>, expectedRtlKeys);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(fileName));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, orientation)->section(LayoutData::mainSection),
                                 false, 0);

    MImKeyArea *keyArea = dynamic_cast<MImKeyArea *>(subject);

    QVERIFY2(keyArea != 0, "Unknown type of button area");
    for (int row = 0; row < keyArea->rowCount(); ++row) {
        for (int column = 0; column < keyArea->sectionModel()->columnsAt(row); ++column) {
            MImKey *key = keyArea->d_ptr->rowList[row].keys[column];
            QVERIFY(key != 0);
            if (expectedRtlKeys.contains(key->model().binding()->action())) {
                QVERIFY(key->model().rtl());
                QVERIFY2(key->iconId().contains("-rtl-"), "This is not RTL icon");
            } else if (!key->iconId().isEmpty()) {
                QVERIFY(!key->model().rtl());
                QVERIFY2(!key->iconId().contains("-rtl-"), "This is not LTR icon");
            }
        }
    }
}

void Ut_MImAbstractKeyArea::testLongKeyPress()
{
    TpCreator createTp = &createTouchPoint;
    const int LongPressTimeOut = 1500; //this value depends on timeout of long press

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "en_us.xml")));

    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 true, 0);

    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());

    MImKey *key0 = dynamic_cast<MImKey *>(keyAt(0, 0));
    MImKey *key1 = dynamic_cast<MImKey *>(keyAt(1, 3));

    QVERIFY(key0);
    QVERIFY(key1);

    QPoint point0 = key0->buttonBoundingRect().center().toPoint();
    QTouchEvent::TouchPoint tp0(createTp(0, Qt::TouchPointPressed,
                                         subject->mapToScene(point0),
                                         QPointF()));

    QPoint point1 = key1->buttonBoundingRect().center().toPoint();
    QTouchEvent::TouchPoint tp1(createTp(1, Qt::TouchPointPressed,
                                         subject->mapToScene(point1),
                                         QPointF()));

    QSignalSpy spy(subject, SIGNAL(longKeyPressed(const MImAbstractKey*, const KeyContext &)));

    QVERIFY(spy.isValid());

    // click is not long press
    subject->d_ptr->touchPointPressed(tp0);
    subject->d_ptr->touchPointReleased(tp1);
    QVERIFY(spy.isEmpty());
    QTest::qWait(LongPressTimeOut);
    QVERIFY(spy.isEmpty());

    // long press on the key
    subject->d_ptr->touchPointPressed(tp0);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const MImAbstractKey *>(), key0);
    spy.clear();
    subject->d_ptr->touchPointReleased(tp0);

    // long press with multitouch
    subject->d_ptr->touchPointPressed(tp0);
    subject->d_ptr->touchPointPressed(tp1);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const MImAbstractKey *>(), key1);
    spy.clear();
    subject->d_ptr->touchPointReleased(tp0);
    subject->d_ptr->touchPointReleased(tp1);
    QVERIFY(spy.isEmpty());

    // long press after movement
    subject->d_ptr->touchPointPressed(tp0);
    tp0.setStartScenePos(QPointF(52, 37));
    subject->d_ptr->touchPointMoved(tp0);
    QTest::qWait(LongPressTimeOut);
    QCOMPARE(spy.count(), 1);
    QVERIFY(spy.first().count() > 0);
    QCOMPARE(spy.first().first().value<const MImAbstractKey *>(), key0);
    spy.clear();
    subject->d_ptr->touchPointReleased(tp0);
    QVERIFY(spy.isEmpty());

    // TODO? long press should be detected if last pressed key is not released
}

void Ut_MImAbstractKeyArea::testKeyLayout()
{
    const int margin = 5;
    const QSize size(50, 50);
    subject = Ut_MImAbstractKeyArea::createArea("Q", size);

    MImAbstractKeyAreaStyle *s = const_cast<MImAbstractKeyAreaStyle *>(subject->style().operator->());
    s->setPaddingLeft(margin);
    s->setPaddingRight(margin);
    s->setPaddingTop(margin);
    s->setPaddingBottom(margin);

    subject->updateKeyGeometries(size.width());

    QVERIFY(!subject->keyAt(QPoint(-1, -1)));
    QVERIFY(subject->keyAt(QPoint(0, 0)));
    QVERIFY(subject->keyAt(QPoint(size.width() / 2, size.height() / 2)));
    QVERIFY(subject->keyAt(QPoint(size.width(), size.height())));
    QVERIFY(!subject->keyAt(QPoint(size.width() + 1, size.height() + 1)));
}

void Ut_MImAbstractKeyArea::testTouchPoints_data()
{
    TpCreator createTp = &createTouchPoint;

    QTest::addColumn<int>("expectedClickedSignals");
    QTest::addColumn<QString>("labels");
    QTest::addColumn<QSize>("size");

    QTest::addColumn<TpList>("touchPoints");
    QTest::addColumn<TpButtonStateMatrix>("expectedStates");

    QTest::newRow("single button")
        << 1 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointReleased, QPointF(16, 20), QPointF(12, 24)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("single button, commit before next hit")
        << 2 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(1, Qt::TouchPointPressed, QPointF(16, 20), QPointF(-1, -1))
                     << createTp(1, Qt::TouchPointReleased, QPointF(16, 20), QPointF(12, 24))
                     << createTp(0, Qt::TouchPointReleased, QPointF(16, 20), QPointF(16, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("2 buttons, 3 touchpoint transactions")
        << 3 << "ab" << QSize(100, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(1, Qt::TouchPointPressed, QPointF(80, 20), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointReleased, QPointF(16, 20), QPointF(12, 24))
                     << createTp(1, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(1, Qt::TouchPointReleased, QPointF(16, 20), QPointF(16, 24))
                     << createTp(0, Qt::TouchPointReleased, QPointF(80, 20), QPointF(80, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal));

    QTest::newRow("move into button, release")
        << 1 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(-1, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(2, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(10, 10), QPointF(2, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(12, 14), QPointF(10, 10))
                     << createTp(0, Qt::TouchPointReleased, QPointF(12, 14), QPointF(12, 14)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("move over button, release outside")
        << 0 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(-1, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(2, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(10, 10), QPointF(2, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(51, 51), QPointF(10, 10))
                     << createTp(0, Qt::TouchPointReleased, QPointF(51, 51), QPointF(51, 51)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("tap inside button, release outside")
        << 0 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 12), QPointF(12, 12))
                     << createTp(0, Qt::TouchPointMoved, QPointF(2, -1), QPointF(12, 12))
                     << createTp(0, Qt::TouchPointMoved, QPointF(10, 10), QPointF(2, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(30, 30), QPointF(10, 10))
                     << createTp(0, Qt::TouchPointReleased, QPointF(51, 51), QPointF(30, 30)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("move into button, press on button with second touchpoint, release first")
        << 2 << "a" << QSize(50, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(-1, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(2, -1), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(10, 10), QPointF(2, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(12, 14), QPointF(10, 10))
                     << createTp(1, Qt::TouchPointPressed, QPointF(20, 20), QPointF(20, 20))
                     << createTp(0, Qt::TouchPointReleased, QPointF(12, 14), QPointF(12, 14))
                     << createTp(0, Qt::TouchPointMoved, QPointF(51, 51), QPointF(20, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal));

    QTest::newRow("sudden move from a to b")
        << 1 << "ab" << QSize(100, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(30, 20), QPointF(12, 24))
                     << createTp(0, Qt::TouchPointMoved, QPointF(80, 20), QPointF(30, 20))
                     << createTp(0, Qt::TouchPointReleased, QPointF(80, 20), QPointF(80, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal));

    QTest::newRow("sudden release while moving from a to b")
        << 0 << "ab" << QSize(100, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(30, 20), QPointF(12, 24))
                     << createTp(0, Qt::TouchPointReleased, QPointF(80, 20), QPointF(30, 20)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal));

    QTest::newRow("sudden release while moving from a to b, mixed with other touchpoint transaction")
        << 2 << "ab" << QSize(100, 50)
        << (TpList() << createTp(0, Qt::TouchPointPressed, QPointF(12, 24), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointMoved, QPointF(30, 20), QPointF(12, 24))
                     << createTp(1, Qt::TouchPointPressed, QPointF(70, 16), QPointF(-1, -1))
                     << createTp(0, Qt::TouchPointReleased, QPointF(80, 20), QPointF(30, 20))
                     << createTp(1, Qt::TouchPointMoved, QPointF(-1, -1), QPointF(70, 16)))
        << (TpButtonStateMatrix() << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Pressed
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Pressed)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal)
                                  << (ButtonStateList() << MImAbstractKey::Normal
                                                        << MImAbstractKey::Normal));
}

void Ut_MImAbstractKeyArea::testTouchPoints()
{
    // How it works:
    // For each touchpoint, I send it to the appropiate handler in KBA.
    // I then compare the button state against the expected button state.
    // It might look a bit complex, but actually it isn't.
    // touchPoints is just a series of synthetic Qt touchpoints.
    // The expectedStates maxtrix has strlen(labels) rows and
    // count(touchPoints) columns.

    QFETCH(int, expectedClickedSignals);
    QFETCH(QString, labels);
    QFETCH(QSize, size);
    QFETCH(TpList, touchPoints);
    QFETCH(TpButtonStateMatrix, expectedStates);

    subject = Ut_MImAbstractKeyArea::createArea(labels, size);
    QSignalSpy spy(subject, SIGNAL(keyClicked(const MImAbstractKey*, const KeyContext &)));

    ButtonList tracedButtons;
    for (int i = 0; i < labels.count(); ++i) {
        tracedButtons << keyAt(0, i);
    }

    for (int i = 0; i < touchPoints.size(); ++i) {
        QTouchEvent::TouchPoint tp = touchPoints.at(i);

        switch (tp.state()) {
        case Qt::TouchPointPressed:
            subject->d_ptr->touchPointPressed(tp);
            break;

        case Qt::TouchPointMoved:
            subject->d_ptr->touchPointMoved(tp);
            break;

        case Qt::TouchPointReleased:
            subject->d_ptr->touchPointReleased(tp);
            break;

        default:
            break;
        }

        const ButtonStateList &bsl = expectedStates.at(i);
        for (int k = 0; k < bsl.size(); ++k) {
            QCOMPARE(tracedButtons.at(k)->state(), bsl.at(k));
        }
    }

    QCOMPARE(spy.count(), expectedClickedSignals);
}

void Ut_MImAbstractKeyArea::testReset()
{
    const QSize size(50, 50);
    subject = Ut_MImAbstractKeyArea::createArea("Q", size, true);
    TpCreator createTp = &createTouchPoint;

    MPlainWindow::instance()->scene()->addItem(subject);

    const QPoint mousePos(20, 20);
    QVERIFY(subject->popup());

    QTouchEvent::TouchPoint tp(0);
    tp.setScreenPos(mousePos);
    subject->d_ptr->touchPointPressed(createTp(0, Qt::TouchPointPressed,
                                        subject->mapToScene(mousePos),
                                        QPointF()));

    QVERIFY(subject->popup()->isVisible());
    const MImKey *key = dynamic_cast<const MImKey *>(MImAbstractKey::lastActiveKey());
    QVERIFY(key);
    QCOMPARE(key->parentItem(), subject);
    QCOMPARE(key->touchPointCount(), 1);


    std::auto_ptr<MImAbstractKeyArea> otherArea(Ut_MImAbstractKeyArea::createEmptyArea());
    MImKeyModel otherKeyModel;
    QSharedPointer<MImKey::StylingCache> otherKeyCache;
    MImFontPool fontPool(true);
    fontPool.setDefaultFont(otherArea->baseStyle()->font());
    MImKey otherKey(otherKeyModel, subject->baseStyle(), *otherArea, otherKeyCache, fontPool);
    QVERIFY(otherKey.parentItem() != subject);

    otherKey.increaseTouchPointCount();
    QCOMPARE(otherKey.touchPointCount(), 1);
    QCOMPARE(&otherKey, MImAbstractKey::lastActiveKey());

    subject->reset();
    QCOMPARE(key->touchPointCount(), 0);
    QVERIFY(!subject->popup()->isVisible());
    // reset should only affect keys in subject:
    QCOMPARE(otherKey.touchPointCount(), 1);
    QCOMPARE(&otherKey, MImAbstractKey::lastActiveKey());
}

void Ut_MImAbstractKeyArea::testStyleModesFromKeyCount_data()
{
    const char *const keys30 = "qwertyuiopqwertyuiopqwertyuiop";

    QTest::addColumn<QString>("keys");
    QTest::addColumn<QString>("expectedStyleMode");

    QTest::newRow("no keys") << QString() << QString();
    QTest::newRow("10 keys") << QString("qwertyuiop") << QString("keys10");
    QTest::newRow("11 keys") << QString("qwertyuiopa") << QString("keys11");
    QTest::newRow("12 keys") << QString("qwertyuiopas") << QString("keys12");
    QTest::newRow("13 keys") << QString("qwertyuiopasd") << QString("keys13");
    QTest::newRow("14 keys") << QString("qwertyuiopasdf") << QString("keys14");
    QTest::newRow("15 keys") << QString("qwertyuiopasdfg") << QString("keys15");

    QTest::newRow("20 keys") << QString("qwertyuiopasdfghjklö") << QString("");
    QTest::newRow("30 keys") << QString(keys30) << QString("keys30");
    QTest::newRow("35 keys") << QString("%1qwert").arg(keys30) << QString("keys35");
    QTest::newRow("40 keys") << QString("%1qwertyuiop").arg(keys30) << QString("keys40");
    QTest::newRow("45 keys") << QString("%1qwertyuiopasdfg").arg(keys30) << QString("keys45");
    QTest::newRow("50 keys") << QString("%1qwertyuiopasdfghjklö").arg(keys30) << QString("");
}

void Ut_MImAbstractKeyArea::testStyleModesFromKeyCount()
{
    QFETCH(QString, keys);
    QFETCH(QString, expectedStyleMode);

    subject = createArea(keys, QSize(50, 50));
    QCOMPARE(subject->baseStyle().currentMode(), expectedStyleMode);
}

void Ut_MImAbstractKeyArea::testLockVerticalMovement_data()
{
    const QRectF area(0, 0, 100, 100);
    const QSize keyAreaSize(area.width() * 0.5, area.height() * 0.5);
    const QRectF outside(area.adjusted(20, 20, -20, -20));
    const QRectF inside(area.adjusted(40, 40, -40, -40));

    QTest::addColumn<bool>("lockVerticalMovement");
    QTest::addColumn<QRectF>("area");
    QTest::addColumn<QSize>("keyAreaSize");
    QTest::addColumn<QString>("sectionContents");
    QTest::addColumn<QString>("expectedLabel");
    QTest::addColumn<PointList>("hitPoints");

    // Expected hit area in overlay mode, shown as '+':
    // .----+++++++----.
    // |    +++++++    |
    // |   .+++++++.   |
    // |   |+++Q+++| <==== key area (centered)
    // |   `+++++++´   |
    // |    +++++++    |
    // `----+++++++----´

    QTest::newRow("no hits")
        << false << area << keyAreaSize << "Q" << ""
        << (PointList() << outside.topLeft() << outside.topRight()
                        << outside.bottomLeft() << outside.bottomRight());

    QTest::newRow("no hits in multi-row area")
        << false << area << keyAreaSize << "QW\nAS" << ""
        << (PointList() << outside.topLeft() << outside.topRight()
                        << outside.bottomLeft() << outside.bottomRight());

    QTest::newRow("bullseye")
        << false << area << keyAreaSize << "Q" << "Q"
        << (PointList() << inside.topLeft() << inside.topRight()
                        << inside.bottomLeft() << inside.bottomRight());

    QTest::newRow("locked movement but no hits")
        << true << area << keyAreaSize << "Q" << ""
        << (PointList() << outside.topLeft() << outside.topRight()
                        << outside.bottomLeft() << outside.bottomRight());

    QTest::newRow("locked movement bullseye")
        << true << area << keyAreaSize << "Q" << "Q"
        << (PointList() << inside.topLeft() << inside.topRight()
                        << inside.bottomLeft() << inside.bottomRight()
                        << QPointF(area.center().x(), area.top())
                        << QPointF(area.center().x(), area.bottom()));

    QTest::newRow("locked movement, hitting upper area of multi-row area")
        << true << area << keyAreaSize << "QW\nAS" << "Q"
        << (PointList() << inside.topLeft()
                        << QPointF(inside.topLeft().x(), area.top()));

    QTest::newRow("locked movement, hitting lower area of multi-row area")
        << true << area << keyAreaSize << "QW\nAS" << "S"
        << (PointList() << inside.bottomRight()
                        << QPointF(inside.bottomRight().x(), area.bottom()));
}

void Ut_MImAbstractKeyArea::testLockVerticalMovement()
{
    QFETCH(bool, lockVerticalMovement);
    QFETCH(QRectF, area);
    QFETCH(QSize, keyAreaSize);
    QFETCH(QString, sectionContents);
    QFETCH(QString, expectedLabel);
    QFETCH(PointList, hitPoints);

    QGraphicsScene sc;
    sc.setSceneRect(area);
    // scene takes ownership:
    sc.addItem(subject = createArea(sectionContents, keyAreaSize));
    // center it:
    subject->setPos((area.width() - keyAreaSize.width()) * 0.5,
                    (area.height() - keyAreaSize.height()) * 0.5);

    static_cast<MImKeyArea *>(subject)->lockVerticalMovement(lockVerticalMovement);

    foreach (const QPointF &hp, hitPoints) {
        // TODO: test crashes on failure (because of scene cleanup)
        // Is it better to just leak a scene instance instead?
        const MImAbstractKey *const foundKey = subject->keyAt(subject->mapFromScene(hp).toPoint());
        const QString foundLabel(foundKey ? foundKey->label() : QString());
        QCOMPARE(foundLabel, expectedLabel);
    }

    subject = 0;
}

void Ut_MImAbstractKeyArea::testFlickEvent_data()
{
    QTest::addColumn<bool>("keyIsPressed");
    QTest::addColumn<Qt::GestureState>("gestureState");
    QTest::addColumn<bool>("expectedSignal");
    QTest::addColumn<MImAbstractKey::ButtonState>("buttonState");

    QTest::newRow("signal")       << true  << Qt::GestureCanceled << true  << MImAbstractKey::Normal;
    QTest::newRow("key released") << false << Qt::GestureCanceled << false << MImAbstractKey::Normal;
    QTest::newRow("no gesture")   << true  << Qt::NoGesture       << false << MImAbstractKey::Pressed;
}

void Ut_MImAbstractKeyArea::testFlickEvent()
{
    QFETCH(bool, keyIsPressed);
    QFETCH(Qt::GestureState, gestureState);
    QFETCH(bool, expectedSignal);
    QFETCH(MImAbstractKey::ButtonState, buttonState);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-layout.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);

    QSignalSpy spy(subject, SIGNAL(keyCancelled(const MImAbstractKey*, const KeyContext &)));
    QVERIFY(spy.isValid());

    MImKey *backspace = dynamic_cast<MImKey *>(subject->findKey("backspace"));
    QVERIFY(backspace);

    if (keyIsPressed) {
        MPlainWindow::instance()->scene()->addItem(subject);
        subject->resize(defaultLayoutSize());

        QPoint point = backspace->buttonBoundingRect().center().toPoint();
        QTouchEvent::TouchPoint tp(createTouchPoint(0, Qt::TouchPointPressed,
                                                    subject->mapToScene(point),
                                                    QPointF()));
        // press backspace key
        subject->d_ptr->touchPointPressed(tp);
    }

    subject->d_ptr->handleFlickGesture(FlickGesture::Right, gestureState);

    if (expectedSignal) {
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.first().first().value<const MImAbstractKey*>(), (const MImAbstractKey*)backspace);
    } else {
        QCOMPARE(spy.count(), 0);
    }
    QCOMPARE(backspace->state(), buttonState);
}

void Ut_MImAbstractKeyArea::testTouchPointCount_data()
{
    QTest::addColumn<TouchOpList>("touchOpList");
    QTest::addColumn<QPoint>("keyToCheckPos");
    QTest::addColumn<int>("expectedTouchPointCount");

    const QPoint key1(3, 3);
    const QPoint key2(0, 0);

    QTest::newRow("no events")
        << TouchOpList()
        << key1 << 0;

    QTest::newRow("press")
        << (TouchOpList() << TouchTestOperation(Press, key1))
        << key1 << 1;

    QTest::newRow("press & release")
        << (TouchOpList()
            << TouchTestOperation(Press, key1)
            << TouchTestOperation(Release, key1))
        << key1 << 0;

    QTest::newRow("stuck key bug due to invalid lastPos")
        << (TouchOpList()
            << TouchTestOperation(Press, key1, 0)
            << TouchTestOperation(Press, key2, 1)
            << TouchTestOperation(Release, key1, 0) // from now on mouse follows tp=1
            << TouchTestOperation(Move, key2, 1) // Update mouse event's position with tp=1.
            << TouchTestOperation(Press, key1, 0) // No new mouse press event because it was pressed all along.
            << TouchTestOperation(Move, key1, 0)) // Mouse move event from key2 (lastpos) to key1. Event's lastpos needs tweaking if used.
        << key1 << 1;
}

void Ut_MImAbstractKeyArea::testTouchPointCount()
{
    QFETCH(TouchOpList, touchOpList);
    QFETCH(QPoint, keyToCheckPos);
    QFETCH(int, expectedTouchPointCount);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-layout.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);

    MWindow *window = MPlainWindow::instance();
    window->scene()->addItem(subject);

    subject->setAcceptTouchEvents(true);
    subject->setZValue(1);
    subject->resize(defaultLayoutSize());

    std::set<int> activeTouchPoints;
    int mouseEventFollowId = -1;
    QPointF mouseLastPos;

    foreach (TouchTestOperation op, touchOpList) {

        MImAbstractKey *key = keyAt(op.keyPos.y(), op.keyPos.x());
        QPointF keyPos = key->buttonRect().center();
        QVERIFY(key && subject->keyAt(keyPos.toPoint()) == key);

        switch (op.event) {
        case Press: {
                activeTouchPoints.insert(op.touchPointId);

                touchEvent(window, activeTouchPoints,
                           Press, op.touchPointId, keyPos.toPoint());

                if (mouseEventFollowId < 0) { // First press, need to send mouse press event.
                    mouseEventFollowId = op.touchPointId;

                    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMousePress);
                    // Last pos gets reset at press.
                    mouseEvent.setLastPos(keyPos);
                    mouseEvent.setLastScenePos(keyPos);
                    mouseEvent.setPos(keyPos);
                    mouseEvent.setScenePos(keyPos);
                    mouseLastPos = keyPos;

                    subject->mousePressEvent(&mouseEvent);
                } else if (op.touchPointId < mouseEventFollowId) {
                    // Follow touch point with the smallest id.
                    mouseEventFollowId = op.touchPointId;
                }
                break;
            }
        case Move: {
                touchEvent(window, activeTouchPoints,
                           Move, op.touchPointId, keyPos.toPoint());

                if (op.touchPointId == mouseEventFollowId) {
                    QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseMove);
                    mouseEvent.setLastPos(mouseLastPos);
                    mouseEvent.setLastScenePos(mouseLastPos);
                    mouseEvent.setPos(keyPos);
                    mouseEvent.setScenePos(keyPos);
                    mouseLastPos = keyPos;

                    subject->mouseMoveEvent(&mouseEvent);
                }
                break;
            }
        case Release: {
                activeTouchPoints.erase(op.touchPointId);

                touchEvent(window, activeTouchPoints,
                           Release, op.touchPointId, keyPos.toPoint());

                if (op.touchPointId == mouseEventFollowId) {
                    if (activeTouchPoints.empty()) {

                        // No touchpoints to follow -> release.

                        QGraphicsSceneMouseEvent mouseEvent(QEvent::GraphicsSceneMouseRelease);
                        mouseEvent.setLastPos(mouseLastPos);
                        mouseEvent.setLastScenePos(mouseLastPos);
                        mouseEvent.setPos(keyPos);
                        mouseEvent.setScenePos(keyPos);
                        mouseLastPos = keyPos;

                        subject->mouseReleaseEvent(&mouseEvent);

                        mouseEventFollowId = -1;
                    } else {
                        mouseEventFollowId = *activeTouchPoints.begin();
                    }
                }
                break;
            }
        }
    }

    MImAbstractKey *keyToCheck = keyAt(keyToCheckPos.y(), keyToCheckPos.x());
    QVERIFY(keyToCheck);
    QCOMPARE(keyToCheck->touchPointCount(), expectedTouchPointCount);
}

void Ut_MImAbstractKeyArea::testResetActiveKeys()
{
    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(QString(TestLayoutFilePath + "test-deadkey.xml")));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);
    MPlainWindow::instance()->scene()->addItem(subject);
    subject->resize(defaultLayoutSize());
    QSignalSpy spy(subject, SIGNAL(keyClicked(const MImAbstractKey*, const KeyContext &)));
    QVERIFY(spy.isValid());

    MImAbstractKey *key = 0;
    QStringList *list = 0;
    QList<int> positions;
    positions << 0 << 1 << 2 << 5 << 6;

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

    for (int i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }

    key = keyAt(2, 8); // row 3, column 9
    QVERIFY(key != 0);
    QVERIFY(key == subject->findKey("dead"));
    QVERIFY(key->isDeadKey());
    QString c = QChar(0x00B4);
    QCOMPARE(key->label(), c);

    //click at deadkey for the first time,
    //just lock the deadkey, won't emit cliked() signal
    clickKey(key);
    QCOMPARE(spy.count(), 0);

    list = &lowerDKUnicodes;
    //check the alphas, should be with deadkey
    for (int i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), list->at(i));
    }

    SpecialKeyFinder deadKeyFinder(SpecialKeyFinder::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&deadKeyFinder);
    QVERIFY(deadKeyFinder.deadKey());

    subject->resetActiveKeys();

    // after reset active keys, dead keys will be reset.
    for (int i = 0; i < positions.count(); i++) {
        QCOMPARE(keyAt(0, positions[i])->label(), lowerUnicodes.at(i));
    }
    deadKeyFinder = SpecialKeyFinder(SpecialKeyFinder::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&deadKeyFinder);
    QVERIFY(!deadKeyFinder.deadKey());

    // check touch point count before/after resetActiveKeys
    TpCreator createTp = &createTouchPoint;

    const QPoint mousePos(20, 20);
    QTouchEvent::TouchPoint tp(0);
    tp.setScreenPos(mousePos);
    subject->d_ptr->touchPointPressed(createTp(0, Qt::TouchPointPressed,
                                        subject->mapToScene(mousePos),
                                        QPointF()));

    const MImKey *activeKey = dynamic_cast<const MImKey *>(MImAbstractKey::lastActiveKey());
    QVERIFY(activeKey);
    QCOMPARE(activeKey->parentItem(), subject);
    QCOMPARE(activeKey->touchPointCount(), 1);

    subject->resetActiveKeys();
    QCOMPARE(activeKey->touchPointCount(), 0);
}

void Ut_MImAbstractKeyArea::touchEvent(QWidget *window,
                                       const std::set<int> &activeTouchPoints,
                                       Ut_MImAbstractKeyArea::TouchEvent event,
                                       int id, QPoint pos)
{
    // QTouchEventSequence is designed so that at compile time it is known
    // how many operations are needed. We don't, and we have to work around it.
    // The events are sent once the temporary object returned by QTest::touchEvent()
    // is destroyed. It is not possible to save a copy of this object (private copy ctor and assignment).
    // As a default value for stationary() calls we can use the id that is actually being
    // pressed/moved/released since it will eventually override the stationary point.

#define TOUCH_SEQUENCE_STATIONARY() \
    stationary(activeTouchPoints.find(0) != activeTouchPoints.end() ? 0 : id) \
    .stationary(activeTouchPoints.find(1) != activeTouchPoints.end() ? 1 : id) \
    .stationary(activeTouchPoints.find(2) != activeTouchPoints.end() ? 2 : id) \
    .stationary(activeTouchPoints.find(3) != activeTouchPoints.end() ? 3 : id) \
    .stationary(activeTouchPoints.find(4) != activeTouchPoints.end() ? 4 : id) \
    .stationary(activeTouchPoints.find(5) != activeTouchPoints.end() ? 5 : id)

    switch (event) {
    case Press:
        QTest::touchEvent(window).TOUCH_SEQUENCE_STATIONARY().press(id, pos);
        break;
    case Move:
        QTest::touchEvent(window).TOUCH_SEQUENCE_STATIONARY().move(id, pos);
        break;
    case Release:
        QTest::touchEvent(window).TOUCH_SEQUENCE_STATIONARY().release(id, pos);
        break;
    }
#undef TOUCH_SEQUENCE_STATIONARY
}

void Ut_MImAbstractKeyArea::changeOrientation(MInputMethod::OrientationAngle angle)
{
    if (MPlainWindow::instance()->orientationAngle() != static_cast<M::OrientationAngle>(angle)) {
        MPlainWindow::instance()->setOrientationAngle(static_cast<M::OrientationAngle>(angle));
        //timeout depends on duration of orientation animation
        QTest::qWait(1000);
    }
}

QSize Ut_MImAbstractKeyArea::defaultLayoutSize()
{
    // Take visible scene size as layout size, but reduce keyboard's paddings first from its width.
    // The height value is ignored since MImAbstractKeyAreas determine their own height.
    return MPlainWindow::instance()->visibleSceneSize()
            - QSize(subject->style()->paddingLeft() + subject->style()->paddingRight(), 0);
}

// Helper method to get key in certain row and column from current subject.
MImAbstractKey *Ut_MImAbstractKeyArea::keyAt(unsigned int row, unsigned int column) const
{
    // If this fails there is something wrong with the test.
    Q_ASSERT(subject);
    Q_ASSERT(row < static_cast<unsigned int>(subject->rowCount()));
    Q_ASSERT(column < static_cast<unsigned int>(subject->sectionModel()->columnsAt(row)));

    MImAbstractKey *key = 0;

    MImKeyArea *buttonArea = dynamic_cast<MImKeyArea *>(subject);
    if (buttonArea) {
        key = buttonArea->d_ptr->rowList[row].keys[column];
    }

    return key;
}

void Ut_MImAbstractKeyArea::clickKey(MImAbstractKey *key)
{
    subject->d_func()->click(key, KeyContext());
}

MImAbstractKeyArea *Ut_MImAbstractKeyArea::createEmptyArea()
{
    return Ut_MImAbstractKeyArea::createArea(QString(), QSize());
}

MImAbstractKeyArea *Ut_MImAbstractKeyArea::createArea(const QString &labels,
                                                      const QSize &size,
                                                      bool usePopup)
{
    LayoutData::SharedLayoutSection section;
    section = LayoutData::SharedLayoutSection(new LayoutSection(labels));

    MImKeyArea *keyArea = MImKeyArea::create(LayoutData::SharedLayoutSection(section), false, 0);
    keyArea->setPopup(usePopup ? new TestPopup : 0);

    // Reset the style:
    MImAbstractKeyAreaStyle *s = const_cast<MImAbstractKeyAreaStyle *>(keyArea->style().operator->());

    s->setMarginLeft(0);
    s->setMarginTop(0);
    s->setMarginRight(0);
    s->setMarginBottom(0);

    // Key-affecting margins and paddings:
    const int padding = 1;
    const int margin = 1;

    s->setKeyMarginLeft(margin);
    s->setKeyMarginTop(margin);
    s->setKeyMarginRight(margin);
    s->setKeyMarginBottom(margin);

    s->setPaddingLeft(padding);
    s->setPaddingTop(padding);
    s->setPaddingRight(padding);
    s->setPaddingBottom(padding);

    // Key geometry:
    const int heightConsumedByKeyMarginsAndPaddings(2 * padding + (section->rowCount() - 1) * margin);
    s->setKeyHeightMedium((size.height() / qMax<int>(1, section->rowCount()))
                          - heightConsumedByKeyMarginsAndPaddings);

    const int widthConsumedByKeyMarginsAndPaddings(2 * padding + (section->maxColumns() - 1) * margin);
    s->setKeyWidthMediumFixed((size.width() /qMax<int>(1, section->maxColumns()))
                              - widthConsumedByKeyMarginsAndPaddings);

    // After changing the key's height it's necessary to recompute the cached
    // widget height. This situation usually does not happen under real
    // conditions, but indicates buggy caching, of course.
    keyArea->d_func()->cachedWidgetHeight = keyArea->d_func()->computeWidgetHeight();

    s->setKeyWidthMedium(1.0);

    // Behaviour:
    s->setTouchpointHorizontalGravity(0);
    s->setTouchpointVerticalGravity(0);
    s->setAutoPadding(false);

    keyArea->resize(size);

    return keyArea;
}


QTEST_APPLESS_MAIN(Ut_MImAbstractKeyArea);
