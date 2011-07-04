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


#include "mgconfitem_stub.h"
#include "minputmethodhoststub.h"
#include "ut_mkeyboardhost.h"
#include "utils.h"
#include "dummydriver_mkh.h"
#include "mplainwindow.h"

#include <mvirtualkeyboard.h>
#include <mhardwarekeyboard.h>
#include <mkeyboardhost.h>
#include <mvirtualkeyboardstyle.h>
#include <symbolview.h>
#include <sharedhandlearea.h>
#include <mimenginefactory.h>
#include <mtoolbardata.h>
#include <mimtoolbar.h>
#include <layoutsmanager.h>
#include <regiontracker.h>
#include <enginemanager.h>
#include <abstractenginewidgethost.h>
#include <minputmethodnamespace.h>

#include <MNamespace>
#include <MApplication>
#include <MSceneManager>
#include <MTheme>
#include <MWidgetController>
#include <MDialog>

#include <QGraphicsLayout>
#include <QDir>

#include <X11/X.h>
#undef KeyPress
#undef KeyRelease

namespace
{
    const char * const XkbLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/layout");
    const char * const XkbVariantSettingName("/meegotouch/inputmethods/hwkeyboard/variant");

    const QString CorrectionSetting("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const QString CorrectionSettingWithSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
    const QString InputMethodCorrectionEngine("/meegotouch/inputmethods/correctionengine");
    const int SceneRotationTime = 1400; // in ms
    bool gAutoCapsEnabled = true;

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";

    int gShowToolbarWidgetCalls = 0;
    int gHideToolbarWidgetCalls = 0;

    const Qt::Key FnLevelKey = Qt::Key_AltGr;
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;

    int gShowLockOnInfoBannerCallCount = 0;
    int gHideLockOnInfoBannerCallCount = 0;
    int gShowLanguageNotificationCallCount = 0;

    const char * const TargetSettingsName("/meegotouch/target/name");
    const char * const DefaultTargetName("Default");

    DummyDriverMkh stubEngine;
}

namespace QTest
{
    template <>
    char *toString(const QRegion &region)
    {
        QString string;
        QDebug debug(&string);

        debug << region;

        return qstrdup(qPrintable(string));
    }
}

Q_DECLARE_METATYPE(QSet<MInputMethod::HandlerState>)
Q_DECLARE_METATYPE(MInputMethod::HandlerState)
Q_DECLARE_METATYPE(ModifierState)
Q_DECLARE_METATYPE(Ut_MKeyboardHost::TestOpList)
Q_DECLARE_METATYPE(MInputMethod::InputModeIndicator)
Q_DECLARE_METATYPE(QList<MImEngine::DictionaryType>)
Q_DECLARE_METATYPE(MInputMethod::PreeditFace)

// Stubbing..................................................................

QString MVirtualKeyboard::layoutLanguage() const
{
    return QString("fi");
}

bool MVirtualKeyboard::autoCapsEnabled() const
{
    return gAutoCapsEnabled;
}

void MVirtualKeyboard::showLanguageNotification()
{
    ++gShowLanguageNotificationCallCount;
}

bool MHardwareKeyboard::autoCapsEnabled() const
{
    return gAutoCapsEnabled;
}

void MImToolbar::showToolbarWidget(QSharedPointer<const MToolbarData> )
{
    ++gShowToolbarWidgetCalls;
}

void MImToolbar::hideToolbarWidget()
{
    ++gHideToolbarWidgetCalls;
}

void MKeyboardHost::showLockOnInfoBanner(const QString &)
{
    gShowLockOnInfoBannerCallCount++;
}

void MKeyboardHost::hideLockOnInfoBanner()
{
    gHideLockOnInfoBannerCallCount++;
}

MImEngineWordsInterface *EngineManager::engine() const
{
    return &stubEngine;
}

// Actual test...............................................................

void Ut_MKeyboardHost::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *)"ut_mvirtualkeyboardhost",
                                 (char *) "-software" };

    disableQtPlugins();

    MImSettings target(TargetSettingsName);
    target.set(DefaultTargetName); // this value is required by the theme daemon

    app = new MApplication(argc, app_name);
    inputMethodHost = new MInputMethodHostStub;
    window = 0;
    mainWindow = new QWidget;

    MImSettings(MultitouchSettings).set(true);

    qRegisterMetaType<MInputMethod::Orientation>("MInputMethod::Orientation");
    qRegisterMetaType<TestOpList>("TestOpList");
}

void Ut_MKeyboardHost::cleanupTestCase()
{
    delete mainWindow;
    mainWindow = 0;
    delete inputMethodHost;
    inputMethodHost = 0;
    delete app;
    app = 0;
}

void Ut_MKeyboardHost::init()
{
    // Uses dummy driver
    stubEngine.enableCompletion();
    stubEngine.enableCorrection();
    MImSettings engineConfig(InputMethodCorrectionEngine);
    engineConfig.set(QVariant(QString("dummyimdriver")));
    MImSettings config(CorrectionSetting);
    config.set(QVariant(false));

    subject = new MKeyboardHost(inputMethodHost, mainWindow);
    inputMethodHost->clear();
    gAutoCapsEnabled = true;

    window = MPlainWindow::instance();
    QVERIFY(window);
    window->show();
    window->sceneManager()->setOrientationAngle(M::Angle0, MSceneManager::ImmediateTransition);
    QCOMPARE(window->orientationAngle(), M::Angle0);
    subject->handleAppOrientationChanged(MInputMethod::Angle0);
}

void Ut_MKeyboardHost::cleanup()
{
    delete subject;
    subject = 0;
    delete window;
    window = 0;
}

void Ut_MKeyboardHost::testCreate()
{
    QVERIFY(subject != 0);
}

void Ut_MKeyboardHost::testOrientationAngleLocked()
{
    // Our window must not listen to device orientation changes.
    QVERIFY(MPlainWindow::instance()->isOrientationAngleLocked());
}

void Ut_MKeyboardHost::testHandleClick()
{
    MImSettings configCorrection(CorrectionSetting);

    // Enable the corrections
    QVERIFY(subject->preedit.isEmpty());
    inputMethodHost->clear();
    subject->update();
    subject->handleKeyClick(KeyEvent("a"));
    qDebug() << "correctionEnabled:" << subject->correctionEnabled;
    qDebug() << "subject->preedit:" << subject->preedit;
    QCOMPARE(subject->preedit, QString("a"));
    QCOMPARE(inputMethodHost->preedit, QString("a"));

    subject->handleKeyClick(KeyEvent(" ", QEvent::KeyRelease, Qt::Key_Space));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->commit, QString("a "));
    inputMethodHost->clear();

    subject->handleKeyClick(KeyEvent("a"));
    subject->handleKeyClick(KeyEvent("\r", QEvent::KeyRelease, Qt::Key_Return));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->commit, QString("a"));
    QCOMPARE(inputMethodHost->keyEvents.count(), 2);
    QCOMPARE(inputMethodHost->keyEvents[0]->text(), QString("\r"));
    QCOMPARE(inputMethodHost->keyEvents[0]->key(), static_cast<int>(Qt::Key_Return));
    QCOMPARE(inputMethodHost->keyEvents[0]->type(), QEvent::KeyPress);
    QCOMPARE(inputMethodHost->keyEvents[1]->text(), QString("\r"));
    QCOMPARE(inputMethodHost->keyEvents[1]->key(), static_cast<int>(Qt::Key_Return));
    QCOMPARE(inputMethodHost->keyEvents[1]->type(), QEvent::KeyRelease);
    inputMethodHost->clear();

    subject->handleKeyClick(KeyEvent("\r", QEvent::KeyRelease, Qt::Key_Return));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->sendCommitStringCalls, 0);
    QCOMPARE(inputMethodHost->commit, QString(""));
    QCOMPARE(inputMethodHost->keyEvents.count(), 2);
    QCOMPARE(inputMethodHost->keyEvents[0]->text(), QString("\r"));
    QCOMPARE(inputMethodHost->keyEvents[0]->key(), static_cast<int>(Qt::Key_Return));
    QCOMPARE(inputMethodHost->keyEvents[0]->type(), QEvent::KeyPress);
    QCOMPARE(inputMethodHost->keyEvents[1]->text(), QString("\r"));
    QCOMPARE(inputMethodHost->keyEvents[1]->key(), static_cast<int>(Qt::Key_Return));
    QCOMPARE(inputMethodHost->keyEvents[1]->type(), QEvent::KeyRelease);
    inputMethodHost->clear();

    subject->handleKeyClick(KeyEvent("a"));
    subject->handleKeyPress(KeyEvent("\b", QEvent::KeyPress, Qt::Key_Backspace));
    subject->handleKeyRelease(KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace));
    QVERIFY(subject->preedit.isEmpty());
    inputMethodHost->clear();

    // Turn off error correction and word completion
    stubEngine.disableCorrection();
    stubEngine.disableCompletion();
    subject->updateCorrectionState();
    QCOMPARE(EngineManager::instance().engine()->correctionEnabled(), false);
    QCOMPARE(EngineManager::instance().engine()->completionEnabled(), false);
    QCOMPARE(subject->correctionEnabled, false);

    subject->handleKeyClick(KeyEvent("a"));
    subject->handleKeyClick(KeyEvent("\r", QEvent::KeyRelease, Qt::Key_Return));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->commit, QString("a"));
    QCOMPARE(inputMethodHost->keyEvents.count(), 2);
    QCOMPARE(inputMethodHost->keyEvents[0]->text(), QString("\r"));
    QCOMPARE(inputMethodHost->keyEvents[0]->key(), static_cast<int>(Qt::Key_Return));
    QCOMPARE(inputMethodHost->keyEvents[0]->type(), QEvent::KeyPress);
    QCOMPARE(inputMethodHost->keyEvents[1]->text(), QString("\r"));
    QCOMPARE(inputMethodHost->keyEvents[1]->key(), static_cast<int>(Qt::Key_Return));
    QCOMPARE(inputMethodHost->keyEvents[1]->type(), QEvent::KeyRelease);
    inputMethodHost->clear();

    subject->handleKeyClick(KeyEvent("\r", QEvent::KeyRelease, Qt::Key_Return));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->sendCommitStringCalls, 0);
    QCOMPARE(inputMethodHost->commit, QString(""));
    QCOMPARE(inputMethodHost->keyEvents.count(), 2);
    QCOMPARE(inputMethodHost->keyEvents[0]->text(), QString("\r"));
    QCOMPARE(inputMethodHost->keyEvents[0]->key(), static_cast<int>(Qt::Key_Return));
    QCOMPARE(inputMethodHost->keyEvents[0]->type(), QEvent::KeyPress);
    QCOMPARE(inputMethodHost->keyEvents[1]->text(), QString("\r"));
    QCOMPARE(inputMethodHost->keyEvents[1]->key(), static_cast<int>(Qt::Key_Return));
    QCOMPARE(inputMethodHost->keyEvents[1]->type(), QEvent::KeyRelease);
    inputMethodHost->clear();

    subject->handleKeyClick(KeyEvent("m"));
    subject->handleKeyClick(KeyEvent("a"));
    QCOMPARE(inputMethodHost->commit, QString("ma"));
    QVERIFY(subject->preedit.isEmpty());

    // Turn on error correction and word completion
    stubEngine.enableCorrection();
    stubEngine.enableCompletion();
    subject->updateCorrectionState();
    QCOMPARE(EngineManager::instance().engine()->correctionEnabled(), true);
    QCOMPARE(EngineManager::instance().engine()->completionEnabled(), true);
    QCOMPARE(subject->correctionEnabled, true);

    subject->handleKeyPress(KeyEvent("\b", QEvent::KeyPress, Qt::Key_Backspace));
    subject->handleKeyRelease(KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace));
    QVERIFY(subject->preedit.isEmpty());
    QCOMPARE(inputMethodHost->commit, QString("ma"));
    QVERIFY(inputMethodHost->keyEvents.count() != 0);
    inputMethodHost->clear();
}

void Ut_MKeyboardHost::testDirectMode()
{
    QList<KeyEvent> testData;
    QList<Qt::Key> expectedKeys;

    testData << KeyEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace)
             << KeyEvent("\r", QEvent::KeyRelease, Qt::Key_Return)
             << KeyEvent(" ", QEvent::KeyRelease, Qt::Key_Space);
    expectedKeys << Qt::Key_Backspace << Qt::Key_Return << Qt::Key_Space;
    QVERIFY(testData.count() == expectedKeys.count());

    subject->inputMethodMode = MInputMethod::InputMethodModeDirect;

    for (int n = 0; n < testData.count(); ++n) {
        inputMethodHost->clear();
        subject->handleKeyRelease(testData.at(n));
        QCOMPARE(inputMethodHost->sendKeyEventCalls, 1);
        QVERIFY(inputMethodHost->keyEvents.first()->text() == testData.at(n).text());
        QVERIFY(inputMethodHost->keyEvents.first()->key() == expectedKeys.at(n));
        QVERIFY(inputMethodHost->keyEvents.first()->type() == QEvent::KeyRelease);
    }

    for (int n = 0; n < testData.count(); ++n) {
        inputMethodHost->clear();
        subject->handleKeyPress(KeyEvent(testData.at(n), QEvent::KeyPress));
        QCOMPARE(inputMethodHost->sendKeyEventCalls, 1);
        QVERIFY(inputMethodHost->keyEvents.first()->text() == testData.at(n).text());
        QVERIFY(inputMethodHost->keyEvents.first()->key() == expectedKeys.at(n));
        QVERIFY(inputMethodHost->keyEvents.first()->type() == QEvent::KeyPress);
    }
}

void Ut_MKeyboardHost::testNotCrash()
{
    //at least we should not crash
    subject->show();
    subject->hide();
    subject->reset();
    subject->setPreedit("string", -1);
    subject->handleMouseClickOnPreedit(QPoint(0, 0), QRect(0, 0, 1, 1));
}

void Ut_MKeyboardHost::testCorrectionOptions()
{
    stubEngine.enableCompletion();
    stubEngine.enableCorrection();
    QCOMPARE(EngineManager::instance().engine()->correctionEnabled(), true);
    QCOMPARE(EngineManager::instance().engine()->completionEnabled(), true);
    subject->show();

    QVERIFY(EngineManager::instance().engine() != 0);

    // The corrections are enabled
    QCOMPARE(subject->correctionEnabled, true);

    // The corrections are disabled
    stubEngine.disableCorrection();
    // change gconf can't triger the EngineManager::instance() emit signal now.
    // need to call updateCorrectionState() manually.
    subject->updateCorrectionState();
    QCOMPARE(subject->correctionEnabled, false);

    // check when content type is changed
    stubEngine.enableCorrection();
    subject->updateCorrectionState();
    inputMethodHost->contentType_ = MInputMethod::FreeTextContentType;
    subject->update();
    QCOMPARE(subject->correctionEnabled, true);

    inputMethodHost->contentType_ = MInputMethod::NumberContentType;
    subject->update();
    QCOMPARE(subject->correctionEnabled, false);

    inputMethodHost->contentType_ = MInputMethod::PhoneNumberContentType;
    subject->update();
    QCOMPARE(subject->correctionEnabled, false);

    subject->hide();
}

void Ut_MKeyboardHost::testCorrectionSettings_data()
{
    QTest::addColumn<bool>("correctionValid");
    QTest::addColumn<bool>("correctionEnabled");
    QTest::addColumn<bool>("predictionValid");
    QTest::addColumn<bool>("predictionEnabled");
    QTest::addColumn<bool>("result");

    QTest::newRow("Correction and prediction invalid")
            << false << false << false << false << true;
    QTest::newRow("Correction invalid, prediction enabled")
            << false << false << true << true << true;
    QTest::newRow("Correction invalid, prediction disabled")
            << false << false << true << false << false;
    QTest::newRow("Correction enabled, prediction invalid")
            << true << true << false << false << true;
    QTest::newRow("Correction disabled, prediction invalid")
            << true << false << false << false << false;
    QTest::newRow("Correction disabled, prediction disabled")
            << true << false << true << false << false;
    QTest::newRow("Correction disabled, prediction enabled")
            << true << false << true << true << false;
    QTest::newRow("Correction enabled, prediction disabled")
            << true << true << true << false << false;
    QTest::newRow("Correction enabled, prediction enabled")
            << true << true << true << true << true;
}

void Ut_MKeyboardHost::testCorrectionSettings()
{
    QFETCH(bool, correctionValid);
    QFETCH(bool, correctionEnabled);
    QFETCH(bool, predictionValid);
    QFETCH(bool, predictionEnabled);
    QFETCH(bool, result);

    stubEngine.enableCompletion();
    stubEngine.enableCorrection();
    inputMethodHost->contentType_ = MInputMethod::FreeTextContentType;

    subject->show();

    QVERIFY(EngineManager::instance().engine() != 0);

    // Correction comes from MTextEdit inputMethodCorrectionEnabled property.
    // Prediction comes from Qt::InputMethodHint: Qt::ImhNoPredictiveText.
    inputMethodHost->correctionValid_ = correctionValid;
    inputMethodHost->correctionEnabled_ = correctionEnabled;
    inputMethodHost->predictionValid_ = predictionValid;
    inputMethodHost->predictionEnabled_ = predictionEnabled;
    subject->updateCorrectionState();
    QCOMPARE(subject->correctionEnabled, result);

    subject->hide();
}

void Ut_MKeyboardHost::testCorrectionContentTypes_data()
{
    QTest::addColumn<MInputMethod::TextContentType>("contentType");
    QTest::addColumn<bool>("result");

    QTest::newRow("Number field")
            << MInputMethod::NumberContentType << false;
    QTest::newRow("Phone number field")
            << MInputMethod::PhoneNumberContentType << false;
    QTest::newRow("Email field")
            << MInputMethod::EmailContentType << false;
    QTest::newRow("URL field")
            << MInputMethod::UrlContentType << false;
    QTest::newRow("Free text field")
            << MInputMethod::FreeTextContentType << true;
    QTest::newRow("Custom field")
            << MInputMethod::CustomContentType << true;

}

void Ut_MKeyboardHost::testCorrectionContentTypes()
{
    QFETCH(MInputMethod::TextContentType, contentType);
    QFETCH(bool, result);

    stubEngine.enableCompletion();
    stubEngine.enableCorrection();

    subject->show();

    QVERIFY(EngineManager::instance().engine() != 0);

    inputMethodHost->contentType_ = contentType;
    subject->updateCorrectionState();
    QCOMPARE(subject->correctionEnabled, result);

    subject->hide();
}

void Ut_MKeyboardHost::testAutoCaps()
{
    inputMethodHost->surroundingString = "Test string. You can using it!    ";
    inputMethodHost->autoCapitalizationEnabled_ = true;
    subject->correctionEnabled = true;
    inputMethodHost->contentType_ = MInputMethod::FreeTextContentType;

    subject->show();

    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 1;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputMethodHost->cursorPos = 12;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputMethodHost->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);


    subject->hide();
    subject->show();

    inputMethodHost->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 16;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    inputMethodHost->cursorPos = 31;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 33;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    // If cursor is not at 0 position, space should unlatch shift.
    KeyEvent press("", QEvent::KeyPress, Qt::Key_Space);
    KeyEvent release(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // If cursor is not at 0 position, space should not unlatch shift
    // if next cursor position has autoCaps.
    inputMethodHost->cursorPos = 31;
    subject->update();
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    // Pressing Enter should not unlatch shift when autocaps
    // is enabled.
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Return);
    release = KeyEvent(press, QEvent::KeyRelease);
    inputMethodHost->cursorPos = 31;
    subject->update();
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    inputMethodHost->cursorPos = 0;
    subject->update();
    subject->vkbWidget->setShiftState(ModifierLatchedState);

    // When autoCaps is on and shift is latched, any key input except shift and
    // in some special cases space and backspace will turn off shift.
    press = KeyEvent("a", QEvent::KeyPress);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // If there are some preedit, capitalization should be off.
    subject->preedit = "Test";
    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    subject->preedit = "";
    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    // The special case for backspace when autoCaps is on, that is cursor is at 0 position,
    // should not change the shift state.
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    // If cursor is not at 0 position, backspace should change the shift state
    // if the previous cursor position doesn't trigger autoCaps.
    inputMethodHost->cursorPos = 31;
    subject->update();
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // If cursor is not at 0 position, backspace should not change the shift state
    // if the previous cursor position does trigger autoCaps.
    inputMethodHost->cursorPos = 32;
    subject->update();
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    // If cursor is not at 0 position, backspace should not change the shift state
    // if there is text selected since host doesn't know which text is selected.
    // (Shift state will be updated in the widget initiated update() call later.)
    inputMethodHost->cursorPos = 31;
    inputMethodHost->textSelected = true;
    subject->update();
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    inputMethodHost->textSelected = false;

    // Test holding backspace with preedit.
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    inputMethodHost->cursorPos = 18;
    subject->preedit = "You can use";
    subject->preeditCursorPos = -1;
    // initial state: preedit("You can use"), shift state:latched, start holding backspace
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    // press and release backspace before timeout will only delete one character,
    subject->handleKeyPress(press);
    QVERIFY(subject->backspaceTimer.isActive());
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    QCOMPARE(subject->preedit, QString("You can us"));

    // but hold backspace longer than timeout, will delete the whole preedit.
    subject->handleKeyPress(press);
    int interval = subject->backspaceTimer.interval();
    QTest::qWait(interval / 2);
    QVERIFY(subject->backspaceTimer.isActive());
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    QTest::qWait((interval / 2) + 50);
    // final state: preedit(""), shift state:on, after holding backspace enough time.
    QVERIFY(subject->preedit.isEmpty());
    QVERIFY(subject->backspaceTimer.isActive());
    inputMethodHost->cursorPos = 13;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(!subject->backspaceTimer.isActive());
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);

    subject->hide();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);

    // Disable autoCaps
    inputMethodHost->autoCapitalizationEnabled_ = false;
    inputMethodHost->cursorPos = 0;
    subject->show();
    subject->update();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // When shift is latched, any key input except shift will turn off shift.
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    press = KeyEvent("a", QEvent::KeyPress);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // Backspace will also change the shift state when cursor is at 0 position.
    inputMethodHost->cursorPos = 0;
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Backspace);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // Pressing Enter should unlatch shift when autocaps is disabled.
    inputMethodHost->cursorPos = 31;
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
    press = KeyEvent("", QEvent::KeyPress, Qt::Key_Return);
    release = KeyEvent(press, QEvent::KeyRelease);
    subject->handleKeyPress(press);
    subject->handleKeyRelease(release);
    subject->handleKeyClick(release);
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);

    // Test autocaps if autoCaps flag is off from layout
    gAutoCapsEnabled = false;
    inputMethodHost->cursorPos = 0;
    subject->update();
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierClearState);
    gAutoCapsEnabled = true;
}

void Ut_MKeyboardHost::testApplicationOrientationChanged()
{
    MAbstractInputMethod *im = subject;
    MInputMethod::OrientationAngle angles[] = { MInputMethod::Angle0, MInputMethod::Angle90, MInputMethod::Angle180, MInputMethod::Angle270 };

    subject->show();

    for (int i = 0; i < 5; ++i) {
        MInputMethod::OrientationAngle currentAngle = angles[i % 4];
        im->handleAppOrientationChanged(static_cast<int>(currentAngle));
        QTest::qWait(1500);
        QCOMPARE(currentAngle, static_cast<MInputMethod::OrientationAngle>(MPlainWindow::instance()->orientationAngle()));
    }
}

void Ut_MKeyboardHost::testCopyPaste()
{
    inputMethodHost->clear();

    subject->sendCopyPaste(InputMethodNoCopyPaste);
    QVERIFY(inputMethodHost->copyCalls == 0);
    QVERIFY(inputMethodHost->pasteCalls == 0);

    subject->sendCopyPaste(InputMethodCopy);
    QVERIFY(inputMethodHost->copyCalls == 1);
    QVERIFY(inputMethodHost->pasteCalls == 0);

    inputMethodHost->clear();
    subject->sendCopyPaste(InputMethodPaste);
    QVERIFY(inputMethodHost->copyCalls == 0);
    QVERIFY(inputMethodHost->pasteCalls == 1);
}

void Ut_MKeyboardHost::testSendString()
{
    QString testString("bacon");

    inputMethodHost->clear();

    subject->sendString(testString);
    QCOMPARE(inputMethodHost->sendCommitStringCalls, 1);
    QCOMPARE(inputMethodHost->commit, testString);
}

void Ut_MKeyboardHost::testSendStringFromToolbar()
{
    QString preeditString("delicious");
    QString toolbarString("bacon");

    inputMethodHost->clear();
    subject->setPreedit(preeditString, -1);
    subject->sendStringFromToolbar(toolbarString);
    QCOMPARE(inputMethodHost->sendCommitStringCalls, 2);
    QCOMPARE(inputMethodHost->commit, preeditString+toolbarString);

    inputMethodHost->clear();
    subject->sendStringFromToolbar(toolbarString);
    QCOMPARE(inputMethodHost->sendCommitStringCalls, 1);
    QCOMPARE(inputMethodHost->commit, toolbarString);
}

QRegion Ut_MKeyboardHost::region(RegionType type, int index)
{
    switch(type) {
    case ScreenRegion:
        return inputMethodHost->screenRegions.at(index);
        break;

    default:
    case InputMethodArea:
        return inputMethodHost->inputMethodAreas.at(index);
        break;
    }
}

void Ut_MKeyboardHost::testRegionSignals()
{
    // method call counts for region stuff
    int c1 = 0; // region updated
    int c2 = 0; // input method area updated
    inputMethodHost->setScreenRegionCalls = 0;

    subject->show();
    ++c1;
    ++c2;

    // We must immediately get non-empty region so that passthrough window
    // is made visible right before the animation starts.
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);
    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, 0));
    QVERIFY(!region(ScreenRegion, c1 - 1).isEmpty());

    qDebug() << "Passthrough region: " << region(ScreenRegion, c1 - 1);
    qDebug() << "libmeegotouch region: " << region(InputMethodArea, c2 - 1);

    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);

    // In normal input method mode there is no invisible handle with non-zero area
    const QRect zeroSizeInvisibleHandleRect(
        dynamic_cast<QGraphicsWidget*>(subject->sharedHandleArea->layout()->itemAt(0))
        ->geometry().toRect());
    QVERIFY(zeroSizeInvisibleHandleRect.isEmpty());

    // In direct mode an invisible handle is added on top of the keyboard
    // The test is disabed just like the invisible handle code itself (this
    // needs to be updated if it is enabled)
#if 0
    subject->vkbWidget->setInputMethodMode(MInputMethod::InputMethodModeDirect);
    subject->sharedHandleArea->setInputMethodMode(MInputMethod::InputMethodModeDirect);
    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    const QRect invisibleHandleRect(
        dynamic_cast<QGraphicsWidget*>(subject->sharedHandleArea->layout()->itemAt(0))
        ->geometry().toRect());
    QVERIFY(!invisibleHandleRect.isEmpty());
    const QRegion invisibleHandleRegion(
        invisibleHandleRect.translated(0, (region(InputMethodArea, 0).boundingRect().top()
                                           - invisibleHandleRect.height())));

    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, c2 - 1) + invisibleHandleRegion);

    subject->vkbWidget->setInputMethodMode(MInputMethod::InputMethodModeNormal);
    subject->sharedHandleArea->setInputMethodMode(MInputMethod::InputMethodModeNormal);
    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, c2 - 1));
    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, c1 - 3));
#endif

    // In opaque mode, candidate widget has its own window, so no regions are sent to kbhost.
#ifndef DUI_IM_DISABLE_TRANSLUCENCY
    // Ditto for engine correction widget
    AbstractEngineWidgetHost *engineWidgetHost =
        EngineManager::instance().handler()->engineWidgetHost();
    QVERIFY(engineWidgetHost);
    engineWidgetHost->setCandidates((QStringList() << "abc" << "def"));
    engineWidgetHost->showEngineWidget(AbstractEngineWidgetHost::FloatingMode);
    ++c1;
    qApp->processEvents();
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    engineWidgetHost->hideEngineWidget();
    ++c1;
    qApp->processEvents();
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);
    QCOMPARE(region(ScreenRegion, c1 - 1), region(InputMethodArea, c2 - 1));
#endif

    // Symbol view may change input method area if it is of different size than the vkb
    const int c1BeforeSymOpen = c1;
    subject->showSymbolView();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);

    const bool noSymviewChange(subject->symbolView->geometry() == subject->vkbWidget->geometry());

    if (!noSymviewChange) {
        ++c1;
        ++c2;
    }
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    // TODO: make RegionTracker do this kind of optimization automatically
    const bool wasEnabled(RegionTracker::instance().enableSignals(false));
    subject->symbolView->hideSymbolView();
    RegionTracker::instance().enableSignals(wasEnabled);

    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);

    if (!noSymviewChange) {
        ++c1;
        ++c2;
    }
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);

    // the same as before opening it
    QCOMPARE(region(ScreenRegion, c1BeforeSymOpen - 1), region(ScreenRegion, c1 - 1));

    // Hide the keyboard -> empty region and input method area
    subject->hide();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50); // really hidden after animation is finished
    ++c1;
    ++c2;
    QCOMPARE(inputMethodHost->setScreenRegionCalls, c1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, c2);
    QCOMPARE(region(ScreenRegion, c1 - 1), QRegion());
    QCOMPARE(region(InputMethodArea, c2 - 1), QRegion());

    // Regions and rotation

    // Preparation: store 270deg-angle region obtained as safely as possible
    inputMethodHost->clear();

    rotateToAngle(MInputMethod::Angle270);
    subject->show();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);

    QCOMPARE(inputMethodHost->setScreenRegionCalls, 1);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, 1);
    QCOMPARE(region(ScreenRegion, 0), region(InputMethodArea, 0));

    QRegion region270(region(ScreenRegion, 0));
    subject->hide();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);
    rotateToAngle(MInputMethod::Angle0);

    QSignalSpy orientationSpy(window, SIGNAL(orientationChangeFinished(MInputMethod::Orientation)));

    subject->show();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);

    inputMethodHost->clear();

#if 0
    // Rotate three times repeatedly with long and short waits in between.  We
    // should end up with a region identical to that stored in region270.  The
    // wait times mimic user operations that have been found to cause a problem.
    window->setOrientationAngle(MInputMethod::Angle90);
    QTest::qWait(800);
    qDebug() << "Orientations finished:" << orientationSpy.count();
    window->setOrientationAngle(MInputMethod::Angle180);
    QTest::qWait(5);
    window->setOrientationAngle(MInputMethod::Angle270);
    qDebug() << "Orientations finished:" << orientationSpy.count();
    qDebug() << "Waiting for rotation animation to finish...";
    QTest::qWait(SceneRotationTime); // wait until rotation animation is finished
    qDebug() << "Waiting for rotation animation to finish...done!";

    // Sanity checks
    qDebug() << "Orientations finished:" << orientationSpy.count();

    QCOMPARE(window->orientationAngle(), MInputMethod::Angle270);
    QVERIFY(inputMethodHost->setScreenRegionCalls > 0);
    QCOMPARE(inputMethodHost->setInputMethodAreaCalls, inputMethodHost->setScreenRegionCalls);

    qDebug() << "Region after animation is finished:" << subject->vkbWidget->region();

    // Now, is the region sane after those consequtive rotations?
    QCOMPARE(region(ScreenRegion, inputMethodHost->setScreenRegionCalls - 1),
             region(InputMethodArea, inputMethodHost->setInputMethodAreaCalls - 1));

    // Remove the next two lines when QCOMPARE is enabled
    qDebug() << "Actual region:" << region(ScreenRegion, inputMethodHost->setScreenRegionCalls - 1);
    qDebug() << "Expected region:" << region270;
    // This fails at the moment. libmeegotouch/Qt bug suspected.
    //QCOMPARE(region(ScreenRegion, spy.count() - 1), region270);
#endif
}

void Ut_MKeyboardHost::testOptimizedRegionCallCounts_data()
{
    QTest::addColumn<MInputMethod::HandlerState>("beforeShowState");
    QTest::addColumn<MInputMethod::HandlerState>("afterShowState");

    // Use -1 to not to care (yet to be optimized).
    QTest::addColumn<int>("imAreaUpdatesAfterShow");
    QTest::addColumn<int>("regionUpdatesAfterShow");
    QTest::addColumn<int>("imAreaUpdatesAfterStateChange");
    QTest::addColumn<int>("regionUpdatesAfterStateChange");

    QTest::newRow("onscreen -> hardware produces only one update")
            << MInputMethod::OnScreen
            << MInputMethod::Hardware
            << -1 << -1
            << 1 << 1;

    QTest::newRow("hardware -> onscreen produces only one update")
            << MInputMethod::OnScreen
            << MInputMethod::Hardware
            << -1 << -1
            << 1 << 1;

    // This basically tests that the region estimate sent is correct and final.
    QTest::newRow("onscreen state show produces only one update")
            << MInputMethod::OnScreen
            << MInputMethod::OnScreen
            << 1 << 1
            << -1 << -1;

    QTest::newRow("hardware state show produces only one update")
            << MInputMethod::Hardware
            << MInputMethod::Hardware
            << 1 << 1
            << -1 << -1;
}

void Ut_MKeyboardHost::testOptimizedRegionCallCounts()
{
    QFETCH(MInputMethod::HandlerState, beforeShowState);
    QFETCH(MInputMethod::HandlerState, afterShowState);
    QFETCH(int, imAreaUpdatesAfterShow);
    QFETCH(int, regionUpdatesAfterShow);
    QFETCH(int, imAreaUpdatesAfterStateChange);
    QFETCH(int, regionUpdatesAfterStateChange);

    // This test does not test validity of regions. It tests only
    // that unnecessary region or input method area updates are not
    // triggered and forwarded to input method host.

    // On state change we basically have two components affecting region,
    // vkb widget and shared handle area.
    // Set minimum height for both so they will affect region.
    subject->sharedHandleArea->setMinimumHeight(10);
    subject->vkbWidget->setMinimumHeight(10);

    // Set to inital state.
    QSet<MInputMethod::HandlerState> state;
    state << beforeShowState;
    subject->setState(state);

    inputMethodHost->setInputMethodAreaCalls = 0;
    inputMethodHost->setScreenRegionCalls = 0;

    // Show plugin in OnScreen state.
    // Skip animation and update positions directly.
    subject->show();

    // Speed things up.. animation has to be run because it updates widget positions.
    subject->slideUpAnimation.pause();
    subject->slideUpAnimation.setDuration(0);
    subject->slideUpAnimation.resume();
    // We need to wait here 200 ms otherwise the test case can fail in lower delays (e.g. 100 ms).
    QTest::qWait(200);
    QVERIFY(subject->slideUpAnimation.state() == QAbstractAnimation::Stopped);

    // Check call counts.
    if (imAreaUpdatesAfterShow >= 0) {
        QCOMPARE(inputMethodHost->setInputMethodAreaCalls, imAreaUpdatesAfterShow);
    }
    if (regionUpdatesAfterShow >= 0) {
        QCOMPARE(inputMethodHost->setScreenRegionCalls, regionUpdatesAfterShow);
    }

    // Clear call counts.
    inputMethodHost->setInputMethodAreaCalls = 0;
    inputMethodHost->setScreenRegionCalls = 0;

    // Switch to final state.
    state.clear();
    state << afterShowState;
    subject->setState(state);

    // Check call counts.
    qApp->processEvents();
    if (imAreaUpdatesAfterStateChange >= 0) {
        QCOMPARE(inputMethodHost->setInputMethodAreaCalls, imAreaUpdatesAfterStateChange);
    }
    if (regionUpdatesAfterStateChange >= 0) {
        QCOMPARE(inputMethodHost->setScreenRegionCalls, regionUpdatesAfterStateChange);
    }
}

void Ut_MKeyboardHost::testSetState_data()
{
    QSet<MInputMethod::HandlerState> state;

    QTest::addColumn<QSet<MInputMethod::HandlerState> >("state");
    QTest::addColumn<MInputMethod::HandlerState>("expectedParameter");

    QTest::newRow("Empty") << state << MInputMethod::OnScreen;

    state.clear();
    state << MInputMethod::Hardware;
    QTest::newRow("Hardware") << state << MInputMethod::Hardware;

    state.clear();
    state << MInputMethod::OnScreen;
    QTest::newRow("OnScreen") << state << MInputMethod::OnScreen;

    state.clear();
    state << MInputMethod::Accessory;
    QTest::newRow("Accessory") << state << MInputMethod::Accessory;
}

void Ut_MKeyboardHost::testSetState()
{
    QFETCH(QSet<MInputMethod::HandlerState>, state);
    QFETCH(MInputMethod::HandlerState, expectedParameter);

    qDebug() << "Probe state=" << state;

    subject->update();
    subject->setState(state);
    QCOMPARE(subject->activeState, expectedParameter);
}

void Ut_MKeyboardHost::testSymbolKeyClick()
{
    QVERIFY(subject->symbolView);
    subject->show();

    // make sure this gets set
    subject->vkbWidget->setShiftState(ModifierLatchedState);

    // Symbol view is toggled by clicking the Sym button.

    // Initially symbol view is closed.
    QVERIFY(!subject->symbolView->isActive());
    bool symOpenExpected = false;

    for (int clicks = 1; clicks <= 3; ++clicks) {
        subject->handleSymbolKeyClick();

        const bool symOpen = subject->symbolView->isActive();
        symOpenExpected = !symOpenExpected;

        QCOMPARE(symOpen, symOpenExpected);
    }
    // Sym button shouldn't have affected shift status
    QVERIFY(subject->vkbWidget->shiftStatus() == ModifierLatchedState);
}

void Ut_MKeyboardHost::testUpdateSymbolViewLevel()
{
    subject->show();
    QSet<MInputMethod::HandlerState> state;

    //hardware state
    QVERIFY(subject->hardwareKeyboard);
    QSignalSpy spy(subject->hardwareKeyboard, SIGNAL(shiftStateChanged()));

    state << MInputMethod::Hardware;
    subject->update();
    subject->setState(state);
    subject->symbolView->showSymbolView();
    QVERIFY(subject->symbolView->isActive());
    QCOMPARE(subject->symbolView->currentLevel(), 0);

    // test autocaps
    subject->autoCapsEnabled = true; // enable auto caps
    subject->hardwareKeyboard->setAutoCapitalization(true);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    subject->hardwareKeyboard->setAutoCapitalization(false);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierClearState);

    spy.clear();
    //! first shift key press+release will latch the shift modifier, and then switch the symbolview level to 1
    // Note that the native modifier parameters are not correct but that doesn't matter for this test.
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0, 0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierLatchedState);
    QCOMPARE(subject->symbolView->currentLevel(), 1);
    // second shift key press+release will lock the shift modifier, shift state changes but symbolview level stays 1
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0, 0);
    QCOMPARE(spy.count(), 3);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierLockedState);
    QCOMPARE(subject->symbolView->currentLevel(), 1);
    //! third shift key press+release will clear the shift modifier, and switch the symbolview level back to 0
    subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0, 0);
    subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0, 0);
    QCOMPARE(spy.count(), 4);
    QCOMPARE(subject->hardwareKeyboard->modifierState(Qt::ShiftModifier), ModifierClearState);
    QCOMPARE(subject->symbolView->currentLevel(), 0);
    subject->symbolView->hideSymbolView();

    //onscreen state
    QVERIFY(subject->vkbWidget);
    subject->autoCapsEnabled = false; // disable auto caps
    subject->vkbWidget->setShiftState(ModifierClearState);
    state.clear();
    state << MInputMethod::OnScreen;
    subject->setState(state);
    QSignalSpy spy1(subject->vkbWidget, SIGNAL(shiftLevelChanged()));

    QVERIFY(!subject->symbolView->isActive());
    subject->symbolView->showSymbolView();
    QVERIFY(subject->symbolView->isActive());
    QCOMPARE(subject->symbolView->currentLevel(), 0);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierClearState);

    spy1.clear();
    subject->vkbWidget->setShiftState(ModifierLatchedState);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierLatchedState);
    QCOMPARE(spy1.count(), 1);
    QCOMPARE(subject->symbolView->currentLevel(), 1);

    subject->vkbWidget->setShiftState(ModifierLockedState);
    QCOMPARE(spy1.count(), 2);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierLockedState);
    QCOMPARE(subject->symbolView->currentLevel(), 1);

    subject->vkbWidget->setShiftState(ModifierClearState);
    QCOMPARE(spy1.count(), 3);
    QCOMPARE(subject->vkbWidget->shiftStatus(), ModifierClearState);
    QCOMPARE(subject->symbolView->currentLevel(), 0);

    subject->hide();
}

void Ut_MKeyboardHost::testKeyCycle_data()
{
    QTest::addColumn<QString>("preedit");

    QTest::newRow("no preedit") << "";
    QTest::newRow("with preedit") << "preedit";
}

void Ut_MKeyboardHost::testKeyCycle()
{
    QFETCH(QString, preedit);
    QString text = "123";
    KeyEvent event1(text,  QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::CycleSet);
    KeyEvent event2("456", QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::CycleSet);
    KeyEvent space ( " ",  QEvent::KeyRelease, Qt::Key_Space);
    KeyEvent invalid( "",  QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::CycleSet);

    //this value must be greater that MultitapTime in the file mkeyboardhost.cpp
    const int MultitapTime = 2000;

    subject->update();
    inputMethodHost->preedit = "";
    inputMethodHost->commit = "";
    inputMethodHost->cursorPos = 0;

    if (!preedit.isEmpty()) {
        subject->preedit = preedit;
    }

    for (int n = 0; n < event1.text().length(); ++n) {
        subject->handleKeyClick(event1);
        QCOMPARE(inputMethodHost->preedit, preedit + QString(text[n]));
        QCOMPARE(inputMethodHost->commit, QString(""));
    }
    QCOMPARE(inputMethodHost->preedit, preedit + QString(text[text.length() - 1]));
    QCOMPARE(inputMethodHost->commit, QString(""));

    subject->handleKeyClick(event2);
    QCOMPARE(inputMethodHost->commit, preedit + QString(text[text.length() - 1]));
    QCOMPARE(inputMethodHost->preedit, QString(event2.text()[0]));

    inputMethodHost->commit = "";
    inputMethodHost->cursorPos = 0;

    QTest::qWait(MultitapTime);
    subject->handleKeyClick(event2);
    QCOMPARE(inputMethodHost->preedit, QString(event2.text()[0]));
    QCOMPARE(inputMethodHost->commit, QString(event2.text()[0]));

    inputMethodHost->commit  = "";
    inputMethodHost->preedit = "";
    inputMethodHost->cursorPos = 0;

    subject->handleKeyClick(space);
    QCOMPARE(inputMethodHost->preedit, QString(""));
    QCOMPARE(inputMethodHost->commit, QString(event2.text()[0]) + " ");

    // Test cycle key autocommit timeout:
    inputMethodHost->commit = "";
    inputMethodHost->preedit = "";
    inputMethodHost->cursorPos = 0;

    subject->handleKeyClick(event1);
    subject->handleKeyClick(event1);
    QCOMPARE(inputMethodHost->preedit, QString(event1.text()[1]));
    QCOMPARE(inputMethodHost->commit, QString(""));
    inputMethodHost->commit = "";
    inputMethodHost->preedit = "";
    inputMethodHost->cursorPos = 0;
    QTest::qWait(MultitapTime);
    QCOMPARE(inputMethodHost->preedit, QString(""));
    QCOMPARE(inputMethodHost->commit, QString(event1.text()[1]));

    // Empty cycle string should not cause crash:
    subject->handleKeyClick(invalid);
}

void Ut_MKeyboardHost::testShiftState_data()
{
    QTest::addColumn<TestOpList>("operations");
    QTest::addColumn<ModifierState>("expectedShiftState");

    // Shift clicks without autocapitalization
    QTest::newRow("no click, no autocaps") << TestOpList() << ModifierClearState;
    QTest::newRow("click shift once") << (TestOpList() << ClickShift) << ModifierLatchedState;
    QTest::newRow("click shift twice") << (TestOpList() << ClickShift << ClickShift) << ModifierLockedState;
    QTest::newRow("click shift three times")
        << (TestOpList() << ClickShift << ClickShift << ClickShift) << ModifierClearState;

    // Shift clicks with autocapitalization
    QTest::newRow("autocaps on, no click")
        << (TestOpList() << TriggerAutoCaps) << ModifierLatchedState;
    QTest::newRow("autocaps on, click shift once")
        << (TestOpList() << TriggerAutoCaps << ClickShift) << ModifierClearState;
    QTest::newRow("autocaps on, click shift twice")
        << (TestOpList() << TriggerAutoCaps << ClickShift << ClickShift) << ModifierLatchedState;
    QTest::newRow("autocaps on, click shift three times")
        << (TestOpList() << TriggerAutoCaps << ClickShift << ClickShift << ClickShift) << ModifierLockedState;

    // Character clicks, how they clear shift

    QTest::newRow("manually latched shift, character click")
        << (TestOpList() << ClickShift << ClickCharacter) << ModifierClearState;
    QTest::newRow("automatically latched shift, character click")
        << (TestOpList() << TriggerAutoCaps << ClickCharacter) << ModifierClearState;
    QTest::newRow("locked shift, character click")
        << (TestOpList() << ClickShift << ClickShift << ClickCharacter) << ModifierLockedState;

    // Tricky cases

    // The shift+character here does not clear shift because shift was latched by the user and he/she had a reason for it.
    QTest::newRow("manually latched shift, shift+character click")
        << (TestOpList() << ClickShift << ClickCharacterWithShiftDown) << ModifierLatchedState;
    // Here user did not by him/herself latch shift. Any click should clear it.
    QTest::newRow("automatically latched shift, shift+character click")
        << (TestOpList() << TriggerAutoCaps << ClickCharacterWithShiftDown) << ModifierClearState;
}

void Ut_MKeyboardHost::testShiftState()
{
    QFETCH(TestOpList, operations);
    QFETCH(ModifierState, expectedShiftState);

    const KeyEvent shiftClickEvent(QString(), QEvent::KeyRelease, Qt::Key_Shift);

    // We don't use Qt::ShiftModifier for shift state since it does not differentiate between
    // latched+char and held+char. Vkb sets it in both situations.
    const KeyEvent characterClickEvent("a", QEvent::KeyRelease);

    foreach (TestOperation op, operations) {
        switch (op) {
        case ClickShift:
            subject->handleKeyClick(shiftClickEvent);
            break;
        case ClickCharacter:
            subject->handleKeyClick(characterClickEvent);
            break;
        case ClickCharacterWithShiftDown:
            subject->handleKeyPress(shiftClickEvent);
            subject->handleKeyClick(characterClickEvent);
            subject->handleKeyRelease(shiftClickEvent);
            break;
        case TriggerAutoCaps:
            triggerAutoCaps();
            break;
        default:
            QFAIL("invalid test operation");
            return;
        }
    }

    QCOMPARE(subject->vkbWidget->shiftStatus(), expectedShiftState);
}

void Ut_MKeyboardHost::testCommitPreeditOnStateChange()
{
    const QString text("fish");
    subject->setPreedit(text, text.length());
    QSet<MInputMethod::HandlerState> state;
    state << MInputMethod::Hardware;
    subject->setState(state);
    QCOMPARE(inputMethodHost->commit, text);
}

void Ut_MKeyboardHost::testLayoutMenuKeyClick_data()
{
    QTest::addColumn<ModifierState>("shiftState");

    QTest::newRow("Clear")   << ModifierClearState;
    QTest::newRow("Latched") << ModifierLatchedState;
    QTest::newRow("Locked")  << ModifierLockedState;
}

void Ut_MKeyboardHost::testLayoutMenuKeyClick()
{
    //menu key should not toggle shift key
    QFETCH(ModifierState, shiftState);
    KeyEvent menuClickEvent(QString(), QEvent::KeyRelease, Qt::Key_unknown, KeyEvent::LayoutMenu);

    subject->vkbWidget->setShiftState(shiftState);
    subject->handleGeneralKeyClick(menuClickEvent);
    QCOMPARE(subject->vkbWidget->shiftStatus(), shiftState);
}

void Ut_MKeyboardHost::testShiftStateOnFocusChanged_data()
{
    QTest::addColumn<MInputMethod::HandlerState>("state");
    QTest::addColumn<ModifierState>("initialShiftState");
    QTest::addColumn<QString>("surroundingString");
    QTest::addColumn<bool>("autoCapitalizationEnabled");
    QTest::addColumn<int>("cursorPosition");
    QTest::addColumn<ModifierState>("expectedShiftState");

    // corsor is at the begining.
    QTest::newRow("screen lowercase") << MInputMethod::OnScreen << ModifierClearState << QString("Test. ")
                                      << true << 0 << ModifierLatchedState;
    QTest::newRow("screen latched") << MInputMethod::OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 2 << ModifierClearState;
    // cursor is right after a dot.
    QTest::newRow("screen latched") << MInputMethod::OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 6 << ModifierLatchedState;
    QTest::newRow("screen latched") << MInputMethod::OnScreen << ModifierLatchedState << QString("Test. ")
                                    << true << 2  << ModifierClearState;
    QTest::newRow("screen locked") << MInputMethod::OnScreen << ModifierLockedState << QString("Test. ")
                                   << true << 0 << ModifierLockedState;
    QTest::newRow("screen locked") << MInputMethod::OnScreen << ModifierLockedState << QString("Test. ")
                                   << true << 2 << ModifierLockedState;
    // text entry disable autocaps.
    QTest::newRow("screen locked") << MInputMethod::OnScreen << ModifierLatchedState << QString("Test. ")
                                   << false << 0 << ModifierClearState;
}

void Ut_MKeyboardHost::testShiftStateOnFocusChanged()
{
    // all temporary shift state (not capslock) should be reset when
    // focus is changed, and new shift state depends on autocaps
    QFETCH(MInputMethod::HandlerState, state);
    QFETCH(ModifierState, initialShiftState);
    QFETCH(QString, surroundingString);
    QFETCH(bool, autoCapitalizationEnabled);
    QFETCH(int, cursorPosition);
    QFETCH(ModifierState, expectedShiftState);

    QSet<MInputMethod::HandlerState> set;
    set << state;
    subject->setState(set);

    subject->vkbWidget->setShiftState(initialShiftState);

    subject->handleFocusChange(true);
    inputMethodHost->surroundingString = surroundingString;
    inputMethodHost->autoCapitalizationEnabled_ = autoCapitalizationEnabled;
    inputMethodHost->cursorPos = cursorPosition;
    subject->update();

    QCOMPARE(subject->vkbWidget->shiftStatus(), expectedShiftState);

}

void Ut_MKeyboardHost::testShiftStateOnLayoutChanged_data()
{
    QTest::addColumn<QString>("surroundingString");
    QTest::addColumn<bool>("autoCapitalizationEnabled");
    QTest::addColumn<int>("cursorPosition");
    QTest::addColumn<QString>("layout");
    QTest::addColumn<bool>("layoutAutoCapitalization");
    QTest::addColumn<ModifierState>("initialShiftState");
    QTest::addColumn<ModifierState>("expectedShiftState");

    // manually set shift state to shift on, changing layout will turn it back to lowercase according autocaps
    QTest::newRow("screen lowercase") << QString("Test. ") << true << 2 << QString("en_gb")
                                      << true << ModifierLatchedState << ModifierClearState;

    // manually set shift state to shift on, changing layout will keep it according autocaps
    QTest::newRow("screen latched") << QString("Test. ") << true << 0 << QString("fi")
                                    << true << ModifierLatchedState << ModifierLatchedState;

    // manually set shift state to lowercase, changing layout will turn it back to shift on according autocaps
    QTest::newRow("screen latched") << QString("Test. ") << true << 0 << QString("fr")
                                    << true << ModifierClearState << ModifierLatchedState;

    // manually set shift state to shift locked, changing layout will turn it back to shift on according autocaps
    QTest::newRow("screen locked") << QString("Test. ") << true << 0 << QString("en_us")
                                   << true << ModifierLockedState << ModifierLatchedState;

    // manually set shift state to shift on, changing layout will change shift to lowercase because
    // layout disable autocaps.
    QTest::newRow("screen latched") << QString("Test. ") << true << 0 << QString("ar")
                                   << false << ModifierLatchedState << ModifierClearState;

    // manually set shift state to shift locked, changing layout will change shift to lowercase because
    // layout disable autocaps.
    QTest::newRow("screen latched") << QString("Test. ") << true << 0 << QString("ar")
                                   << false << ModifierLockedState << ModifierClearState;
}

void Ut_MKeyboardHost::testShiftStateOnLayoutChanged()
{
    // all temporary shift state (not capslock) should be reset when
    // focus is changed, and new shift state depends on autocaps
    QFETCH(QString, surroundingString);
    QFETCH(bool, autoCapitalizationEnabled);
    QFETCH(int, cursorPosition);
    QFETCH(QString, layout);
    QFETCH(bool, layoutAutoCapitalization);
    QFETCH(ModifierState, initialShiftState);
    QFETCH(ModifierState, expectedShiftState);

    QSet<MInputMethod::HandlerState> set;
    set << MInputMethod::OnScreen;
    subject->setState(set);

    inputMethodHost->surroundingString = surroundingString;
    inputMethodHost->autoCapitalizationEnabled_ = autoCapitalizationEnabled;
    inputMethodHost->cursorPos = cursorPosition;
    subject->vkbWidget->setShiftState(initialShiftState);

    gAutoCapsEnabled = layoutAutoCapitalization;
    subject->handleVirtualKeyboardLayoutChanged(layout);

    QCOMPARE(subject->vkbWidget->shiftStatus(), expectedShiftState);
}

void Ut_MKeyboardHost::rotateToAngle(MInputMethod::OrientationAngle angle)
{
    subject->handleAppOrientationChanged(angle);
    QTest::qWait(SceneRotationTime); // wait until rotation animation is finished
}

void Ut_MKeyboardHost::triggerAutoCaps()
{
    // Set necessary conditions for triggering auto capitalization
    gAutoCapsEnabled = true;
    subject->preedit.clear();
    subject->cursorPos = 0;
    inputMethodHost->contentType_ = MInputMethod::FreeTextContentType;
    inputMethodHost->autoCapitalizationEnabled_ = true;

    // Update
    subject->updateAutoCapitalization();
    QVERIFY(subject->autoCapsTriggered);
}

void Ut_MKeyboardHost::testToolbar()
{
    const QString toolbarName1 = QCoreApplication::applicationDirPath() + "/toolbar1.xml";
    const QString toolbarName2 = QCoreApplication::applicationDirPath() + "/toolbar2.xml";

    QVERIFY2(QFile(toolbarName1).exists(), "toolbar1.xml does not exist");
    QVERIFY2(QFile(toolbarName2).exists(), "toolbar2.xml does not exist");

    QSharedPointer<MToolbarData> toolbar1(new MToolbarData);
    QSharedPointer<MToolbarData> toolbar2(new MToolbarData);
    QSharedPointer<MToolbarData> nothing;
    bool ok;

    ok = toolbar1->loadToolbarXml(toolbarName1);
    QVERIFY2(ok, "toolbar1.xml was not loaded correctly");

    ok = toolbar2->loadToolbarXml(toolbarName2);
    QVERIFY2(ok, "toolbar2.xml was not loaded correctly");

    // verify is showToolbarWidget was called
    subject->setToolbar(toolbar2);
    QCOMPARE(gShowToolbarWidgetCalls, 1);
    QCOMPARE(gHideToolbarWidgetCalls, 0);

    subject->setToolbar(toolbar1);
    // since in this test there is no focus, hideToolbarWidget is not called but
    // toolbarHidePending is set to true
    QCOMPARE(gShowToolbarWidgetCalls, 1);
    QCOMPARE(gHideToolbarWidgetCalls, 0);
    QVERIFY(subject->toolbarHidePending);

    subject->setToolbar(toolbar2);
    gShowToolbarWidgetCalls = 0;
    gHideToolbarWidgetCalls = 0;
    QVERIFY(!subject->toolbarHidePending);

    subject->setToolbar(nothing);
    QCOMPARE(gShowToolbarWidgetCalls, 0);
    QCOMPARE(gHideToolbarWidgetCalls, 0);
    QVERIFY(subject->toolbarHidePending);
}

void Ut_MKeyboardHost::testHandleHwKeyboardStateChanged_data()
{
    QTest::addColumn<QString>("xkbLayout");
    QTest::addColumn<QString>("xkbVariant");
    QTest::addColumn<int>("shiftClickedCount");
    QTest::addColumn<int>("fnClickedCount");
    QTest::addColumn<MInputMethod::InputModeIndicator>("expectIndicator");
    QTest::addColumn<bool>("notificationShowCalled");
    QTest::addColumn<int>("deadKeyCharacterCode");

    QTest::newRow("English clear indicator") << QString("us") << QString("") << 0
                                             << 0 << MInputMethod::LatinLowerIndicator
                                             << false << 0;
    QTest::newRow("English shift latched indicator") << QString("us") << QString("") << 1
                                                     << 0 << MInputMethod::LatinUpperIndicator
                                                     << false << 0;
    QTest::newRow("English shift locked indicator") << QString("us") << QString("") << 2
                                                    << 0 << MInputMethod::LatinLockedIndicator
                                                    << true << 0;
    QTest::newRow("English fn latched indicator") << QString("us") << QString("") << 0
                                                  << 1 << MInputMethod::NumAndSymLatchedIndicator
                                                  << false << 0;
    QTest::newRow("English fn locked indicator") << QString("us") << QString("") << 0
                                                 << 2 << MInputMethod::NumAndSymLockedIndicator
                                                 << true << 0;
    QTest::newRow("Cyrillic clear indicator") << QString("ru") << QString("cyrillic") << 0
                                              << 0 << MInputMethod::CyrillicLowerIndicator
                                              << false << 0;
    QTest::newRow("Cyrillic shift latched indicator") << QString("ru") << QString("cyrillic") << 1
                                                      << 0 << MInputMethod::CyrillicUpperIndicator
                                                      << false << 0;
    QTest::newRow("Cyrillic shift locked indicator") << QString("ru") << QString("cyrillic") << 2
                                                     << 0 << MInputMethod::CyrillicLockedIndicator
                                                     << true << 0;
    QTest::newRow("Latin clear indicator") << QString("ru") << QString("latin") << 0
                                              << 0 << MInputMethod::LatinLowerIndicator
                                              << false << 0;
    QTest::newRow("Latin shift latched indicator") << QString("ru") << QString("latin") << 1
                                                   << 0 << MInputMethod::LatinUpperIndicator
                                                   << false << 0;
    QTest::newRow("Latin shift locked indicator") << QString("us") << QString("latin") << 2
                                                  << 0 << MInputMethod::LatinLockedIndicator
                                                  << true << 0;
    QTest::newRow("Arabic indicator") << QString("ara") << QString("") << 0
                                      << 0 << MInputMethod::ArabicIndicator
                                      << false << 0;
    QTest::newRow("DeadKeyAcute") << QString("br") << QString("") << 2 << 0
                                  << MInputMethod::DeadKeyAcuteIndicator << true << 0x00b4;
    QTest::newRow("DeadKeyCaron") << QString("sk") << QString("") << 2 << 0
                                  << MInputMethod::DeadKeyCaronIndicator << true << 0x02c7;
    QTest::newRow("DeadKeyCircumflex") << QString("br") << QString("") << 2 << 0
                                       << MInputMethod::DeadKeyCircumflexIndicator
                                       << true << 0x005e;
    QTest::newRow("DeadKeyDiaeresis") << QString("fr") << QString("") << 2 << 0
                                      << MInputMethod::DeadKeyDiaeresisIndicator << true << 0x00a8;
    QTest::newRow("DeadKeyGrave") << QString("br") << QString("") << 2 << 0
                                  << MInputMethod::DeadKeyGraveIndicator << true << 0x0060;
    QTest::newRow("DeadKeyTilde") << QString("br") << QString("") << 2 << 0
                                  << MInputMethod::DeadKeyTildeIndicator << true << 0x007e;
}

void Ut_MKeyboardHost::testHandleHwKeyboardStateChanged()
{
    QFETCH(QString, xkbLayout);
    QFETCH(QString, xkbVariant);
    QFETCH(int, shiftClickedCount);
    QFETCH(int, fnClickedCount);
    QFETCH(MInputMethod::InputModeIndicator, expectIndicator);
    QFETCH(bool, notificationShowCalled);
    QFETCH(int, deadKeyCharacterCode);

    LayoutsManager::instance().setXkbMap(xkbLayout, xkbVariant);
    QSet<MInputMethod::HandlerState> states;
    states << MInputMethod::Hardware;
    subject->setState(states);
    subject->handleFocusChange(true);
    gShowLockOnInfoBannerCallCount = 0;

    for (int i = 0; i < shiftClickedCount;  i++) {
        subject->processKeyEvent(QEvent::KeyPress, Qt::Key_Shift, Qt::NoModifier, QString(""), false, 1, 0, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_Shift, Qt::ShiftModifier, QString(""), false, 1, 0, 0, 0);
    }

    for (int i = 0; i < fnClickedCount;  i++) {
        subject->processKeyEvent(QEvent::KeyPress, FnLevelKey, Qt::NoModifier, QString(""), false, 1, 0, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, FnLevelKey, FnLevelModifier, QString(""), false, 1, 0, 0, 0);
    }

    if (deadKeyCharacterCode) {
        MImSettings layoutConfig(XkbLayoutSettingName);
        layoutConfig.set(xkbLayout);
        MImSettings variantConfig(XkbVariantSettingName);
        variantConfig.set(xkbVariant);

        subject->processKeyEvent(QEvent::KeyPress, Qt::Key_unknown, Qt::NoModifier,
                                 QString(QChar(deadKeyCharacterCode)), false, 1, 0, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_unknown, Qt::NoModifier,
                                 QString(QChar(deadKeyCharacterCode)), false, 1, 0, 0, 0);
    }

    QCOMPARE(inputMethodHost->indicator, expectIndicator);
    QCOMPARE(gShowLockOnInfoBannerCallCount, notificationShowCalled ? 1 : 0);

    if (deadKeyCharacterCode) {
        // When state is changed from locked -> dead key -> locked, we don't want a notification
        subject->processKeyEvent(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier,
                                 QString("A"), false, 1, 0, 0, 0);
        subject->processKeyEvent(QEvent::KeyRelease, Qt::Key_A, Qt::NoModifier,
                                 QString("A"), false, 1, 0, 0, 0);
        QCOMPARE(inputMethodHost->indicator, MInputMethod::LatinLockedIndicator);
        QCOMPARE(gShowLockOnInfoBannerCallCount, 1);
    }
}

void Ut_MKeyboardHost::testUserHide()
{
    subject->show();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);

    // Hide without preedit
    inputMethodHost->clear();
    subject->preedit.clear();

    // TODO: test that the animation has finished?  Really useful?
    //QVERIFY(subject->vkbWidget->isFullyVisible());
    subject->userHide();
    // TODO: QVERIFY(!subject->vkbWidget->isFullyVisible());
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);
    QVERIFY(!subject->vkbWidget->isVisible());

    QVERIFY(inputMethodHost->setScreenRegionCalls > 0);
    QVERIFY(region(ScreenRegion, inputMethodHost->setScreenRegionCalls - 1).isEmpty());
    QCOMPARE(inputMethodHost->sendCommitStringCalls, 0);
    QCOMPARE(inputMethodHost->commit, QString(""));

    subject->show();
    QTest::qWait(MKeyboardHost::OnScreenAnimationTime + 50);

    inputMethodHost->clear();
    subject->preedit = "Bacon";

    // Hide with preedit
    subject->userHide();

    QCOMPARE(inputMethodHost->sendCommitStringCalls, 1);
    QCOMPARE(inputMethodHost->commit, QString("Bacon"));
}

void Ut_MKeyboardHost::testWYTIWYSErrorCorrection()
{
    // Pre-edit must not be auto-corrected

    subject->show();
    MImSettings config(CorrectionSetting);
    config.set(QVariant(true));

    QStringList candidates;
    candidates << "a" << "c" << "d";
    stubEngine.setCandidates(candidates);
    stubEngine.setSuggestedCandidateIndexReturnValue(1);

    subject->handleKeyClick(KeyEvent("d"));
    QCOMPARE(inputMethodHost->preedit, QString("d"));

    subject->hide();
}

struct TestSignalEvent {
    const char *str;
    Qt::Key key;
    int expectedPressSignal;
    int expectedReleaseSignal;
};

void Ut_MKeyboardHost::testSignals(MInputMethod::InputMethodMode inputMethodMode, const TestSignalEvent *testEvents)
{
    subject->inputMethodMode = inputMethodMode;

    inputMethodHost->clear();
    for(int i = 0; testEvents[i].str != 0; ++i ) {
        if (testEvents[i].key == Qt::Key_Backspace) {
            QSKIP("Backspace key is known to be broken", SkipSingle);
        }
        inputMethodHost->sendKeyEventCalls = 0;
        subject->handleKeyPress(KeyEvent(testEvents[i].str, QEvent::KeyPress,
                                         testEvents[i].key) );
        QCOMPARE(inputMethodHost->sendKeyEventCalls, testEvents[i].expectedPressSignal);
        inputMethodHost->sendKeyEventCalls = 0;
        subject->handleKeyRelease(KeyEvent(testEvents[i].str, QEvent::KeyRelease,
                                         testEvents[i].key) );
        QCOMPARE(inputMethodHost->sendKeyEventCalls, testEvents[i].expectedReleaseSignal);
    }
}

void Ut_MKeyboardHost::testSignalsInNormalMode()
{
    // only few keys generate signals in normal mode
    const TestSignalEvent testEvents[] = {
        { "k", Qt::Key_K, 0, 0 },
        { "+-", Qt::Key_plusminus, 0, 0 },
        { " ", Qt::Key_Space, 0, 0 },
        { "s", Qt::Key_S, 0, 0 },
        { "\b", Qt::Key_Backspace, 1, 1 },
        { 0, Qt::Key_unknown, 0, 0 }
    };

    testSignals(MInputMethod::InputMethodModeNormal, testEvents);
}

void Ut_MKeyboardHost::testSignalsInDirectMode()
{
    const TestSignalEvent testEvents[] = {
        { "k", Qt::Key_K, 1, 1 },
        { "+-", Qt::Key_plusminus, 1, 1 },
        { " ", Qt::Key_Space, 1, 1 },
        { "s", Qt::Key_S, 1, 1 },
        { "\b", Qt::Key_Backspace, 1, 1 },
        { 0, Qt::Key_unknown, 0, 0 }
    };

    testSignals(MInputMethod::InputMethodModeDirect, testEvents);
}

void Ut_MKeyboardHost::testShowLanguageNotification_data()
{
    QTest::addColumn<MInputMethod::HandlerState>("state");
    QTest::addColumn<int>("expectedCallCount");

    QTest::newRow("OnScreen") << MInputMethod::OnScreen << 1;
    QTest::newRow("Hardware") << MInputMethod::Hardware << 0;
    QTest::newRow("Accessory") << MInputMethod::Accessory << 0;
}

void Ut_MKeyboardHost::testShowLanguageNotification()
{
    QFETCH(MInputMethod::HandlerState, state);
    QFETCH(int, expectedCallCount);
    QSet<MInputMethod::HandlerState> states;

    states << state;

    gShowLanguageNotificationCallCount = 0;
    subject->update();
    subject->setState(states);
    subject->showLanguageNotification();
    QCOMPARE(gShowLanguageNotificationCallCount, expectedCallCount);
}


void Ut_MKeyboardHost::testAutoPunctuation_data()
{
    QTest::addColumn<QChar>("character");
    QTest::addColumn<bool>("autopunctuated");

    QTest::newRow(".") << QChar('.') << true;
    QTest::newRow(",") << QChar(',') << true;
    QTest::newRow("!") << QChar('!') << true;
    QTest::newRow("?") << QChar('?') << true;
    QTest::newRow("a") << QChar('a') << false;
}

void Ut_MKeyboardHost::testAutoPunctuation()
{
    MImSettings configCorrection(CorrectionSetting);
    MImSettings configCorrectionSpace(CorrectionSettingWithSpace);

    QVERIFY(EngineManager::instance().engine() != 0);

    // We need to be sure that the correction is enabled before the test
    configCorrection.set(QVariant(true));
    configCorrectionSpace.set(QVariant(true));
    QTest::qWait(100);
    QVERIFY(EngineManager::instance().engine()->correctionEnabled());
    QVERIFY(EngineManager::instance().engine()->completionEnabled());

    // Real test case
    QFETCH(QChar, character);
    QFETCH(bool, autopunctuated);

    MImSettings config(CorrectionSetting);
    config.set(QVariant(true));

    subject->show();

    QStringList candidates;
    candidates << "foo" << "foobar";
    subject->setPreedit("fo", -1);
    stubEngine.setCandidates(candidates);
    stubEngine.setSuggestedCandidateIndexReturnValue(0);
    inputMethodHost->cursorRectangleReturnValue = QRect(0, 0, 100, 100);
    subject->handleKeyClick(KeyEvent("o"));
    subject->handleKeyClick(KeyEvent(" ", QEvent::KeyRelease, Qt::Key_Space));
    inputMethodHost->commit.clear();
    inputMethodHost->preedit.clear();
    inputMethodHost->cursorPos = 0;
    subject->handleKeyClick(KeyEvent(character, QEvent::KeyRelease));

    if (autopunctuated) {
        QCOMPARE(inputMethodHost->commit, QString(character) + " ");
        QCOMPARE(inputMethodHost->sendKeyEventCalls, 2);
        QCOMPARE(inputMethodHost->keyEvents[0]->key(), static_cast<int>(Qt::Key_Backspace));
        QCOMPARE(inputMethodHost->keyEvents[0]->type(), QEvent::KeyPress);
        QCOMPARE(inputMethodHost->keyEvents[1]->key(), static_cast<int>(Qt::Key_Backspace));
        QCOMPARE(inputMethodHost->keyEvents[1]->type(), QEvent::KeyRelease);
    } else {
        QCOMPARE(inputMethodHost->commit, QString());
        QCOMPARE(inputMethodHost->preedit, QString(character));
        QCOMPARE(inputMethodHost->sendKeyEventCalls, 0);
    }
}

void Ut_MKeyboardHost::testToolbarPosition()
{
    // Position after portrait vkb -> hwkb (landscape) transition

    rotateToAngle(MInputMethod::Angle90);
    subject->show();

    QSet<MInputMethod::HandlerState> states;
    states << MInputMethod::Hardware;
    subject->setState(states);

    rotateToAngle(MInputMethod::Angle0);
    QCOMPARE(subject->sharedHandleArea->pos(),
             QPointF(0, (MPlainWindow::instance()->visibleSceneSize().height()
                         - subject->sharedHandleArea->size().height())));
}

const QChar CursorSign('|');

void Ut_MKeyboardHost::testTogglePlusMinus_data()
{
    // Define what text should look before and after sign change.
    // Sign '|' means cursor position in the text.
    QTest::addColumn<QString>("before");
    QTest::addColumn<QString>("after");

    // Strict "only number" input field tests
    QTest::newRow("Empty") << "|" << "-|";
    QTest::newRow("Only sign 1") << "+|" << "-|";
    QTest::newRow("Only sign 2") << "-|" << "+|";
    QTest::newRow("Only sign 3") << "|+" << "|-";
    QTest::newRow("Only sign 4") << "|-" << "|+";
    QTest::newRow("At the end - no sign") << "523.90|" << "-523.90|";
    QTest::newRow("At the end - plus sign") << "+523.90|" << "-523.90|";
    QTest::newRow("At the end - minus sign") << "-523.90|" << "+523.90|";
    QTest::newRow("In the middle - no sign") << "52|3.90" << "-52|3.90";
    QTest::newRow("In the middle - plus sign") << "+52|3.90" << "-52|3.90";
    QTest::newRow("In the middle - minus sign") << "-52|3.90" << "+52|3.90";
    QTest::newRow("In the begin - no sign") << "|523.90" << "-|523.90";
    QTest::newRow("In the begin - plus sign") << "|+523.90" << "|-523.90";
    QTest::newRow("In the begin - minus sign") << "|-523.90" << "|+523.90";
    QTest::newRow("Before sign - plus sign") << "+|523.90" << "-|523.90";
    QTest::newRow("Before sign - minus sign") << "-|523.90" << "+|523.90";

    // Loose (any content) input field tests
    QTest::newRow("Space 1") << " |" << " -|";
    QTest::newRow("Space 2") << "| " << "-| ";
    QTest::newRow("Space 3") << " | " << " -| ";
    QTest::newRow("Space + only sign 1") << " +|" << " -|";
    QTest::newRow("Space + only sign 2") << " -|" << " +|";
    QTest::newRow("Space + only sign 3") << "+| " << "-| ";
    QTest::newRow("Space + only sign 4") << "-| " << "+| ";
    QTest::newRow("Space + only sign 5") << " |+" << " |-";
    QTest::newRow("Space + only sign 6") << " |-" << " |+";
    QTest::newRow("Space + only sign 7") << "|+ " << "|- ";
    QTest::newRow("Space + only sign 8") << "|- " << "|+ ";
    QTest::newRow("Multiple numbers 1") << "|+1111.11 +222.22+333.33 + 444.44"
                                        << "|-1111.11 +222.22+333.33 + 444.44";
    QTest::newRow("Multiple numbers 2") << "+111|1.11 +222.22+333.33 + 444.44"
                                        << "-111|1.11 +222.22+333.33 + 444.44";
    QTest::newRow("Multiple numbers 3") << "+1111.11| +222.22+333.33 + 444.44"
                                        << "-1111.11| +222.22+333.33 + 444.44";
    QTest::newRow("Multiple numbers 4") << "+1111.11 |+222.22+333.33 + 444.44"
                                        << "+1111.11 |-222.22+333.33 + 444.44";
    QTest::newRow("Multiple numbers 5") << "+1111.11 +|222.22+333.33 + 444.44"
                                        << "+1111.11 -|222.22+333.33 + 444.44";
    QTest::newRow("Multiple numbers 6") << "+1111.11 +222|.22+333.33 + 444.44"
                                        << "+1111.11 -222|.22+333.33 + 444.44";
    QTest::newRow("Multiple numbers 7") << "+1111.11 +222.22|+333.33 + 444.44"
                                        << "+1111.11 -222.22|+333.33 + 444.44";
    QTest::newRow("Multiple numbers 8") << "+1111.11 +222.22+|333.33 + 444.44"
                                        << "+1111.11 +222.22-|333.33 + 444.44";
    QTest::newRow("Multiple numbers 9") << "+1111.11 +222.22+333|.33 + 444.44"
                                        << "+1111.11 +222.22-333|.33 + 444.44";
    QTest::newRow("Multiple numbers 10") << "+1111.11 +222.22+333.33| + 444.44"
                                         << "+1111.11 +222.22-333.33| + 444.44";
    QTest::newRow("Multiple numbers 11") << "+1111.11 +222.22+333.33 |+ 444.44"
                                         << "+1111.11 +222.22+333.33 |- 444.44";
    QTest::newRow("Multiple numbers 12") << "+1111.11 +222.22+333.33 +| 444.44"
                                         << "+1111.11 +222.22+333.33 -| 444.44";
    QTest::newRow("Multiple numbers 13") << "+1111.11 +222.22+333.33 + |444.44"
                                         << "+1111.11 +222.22+333.33 + -|444.44";
    QTest::newRow("Multiple numbers 13") << "+1111.11 +222.22+333.33 + 4|44.44"
                                         << "+1111.11 +222.22+333.33 + -4|44.44";
}

void Ut_MKeyboardHost::testTogglePlusMinus()
{
    QFETCH(QString, before);
    QFETCH(QString, after);

    // Trim before
    QVERIFY(before.count(CursorSign) == 1);
    int beforeCursorPos = before.indexOf(CursorSign);
    QVERIFY(beforeCursorPos >= 0);
    before.remove(CursorSign);

    // Trim after
    QVERIFY(after.count(CursorSign) == 1);
    int afterCursorPos = after.indexOf(CursorSign);
    QVERIFY(afterCursorPos >= 0);
    after.remove(CursorSign);

    // Test
    inputMethodHost->preedit = "";
    inputMethodHost->commit = before;
    inputMethodHost->surroundingString = before;
    inputMethodHost->cursorPos = beforeCursorPos;

    subject->togglePlusMinus();

    //qDebug
    QCOMPARE(inputMethodHost->commit, after);
    QCOMPARE(inputMethodHost->cursorPos, afterCursorPos);
}

void Ut_MKeyboardHost::testPreeditFormat_data()
{
    QTest::addColumn<QString>("preedit");
    QTest::addColumn<QStringList>("candidates");
    QTest::addColumn<QList<MImEngine::DictionaryType> >("candidateSource");
    QTest::addColumn<MInputMethod::PreeditFace>("preeditFormatFace");

    QTest::newRow("Many candidates and preedit in dictionary")
        << "hello"
        << (QStringList()
            << "hello"
            << "yellow"
            << "fellow")
        << (QList<MImEngine::DictionaryType>()
            << MImEngine::DictionaryTypeLanguage
            << MImEngine::DictionaryTypeLanguage
            << MImEngine::DictionaryTypeLanguage)
        << MInputMethod::PreeditDefault;

    QTest::newRow("Many candidates but preedit not in dictionary")
        << "gello"
        << (QStringList()
            << "gello"
            << "yellow"
            << "fellow")
        << (QList<MImEngine::DictionaryType>()
            << MImEngine::DictionaryTypeInvalid
            << MImEngine::DictionaryTypeLanguage
            << MImEngine::DictionaryTypeLanguage)
        << MInputMethod::PreeditDefault;

    QTest::newRow("One candidate and preedit in dictionary")
        << "hello"
        << (QStringList()
            << "hello"
            << "fellow")
        << (QList<MImEngine::DictionaryType>()
            << MImEngine::DictionaryTypeLanguage
            << MImEngine::DictionaryTypeLanguage)
        << MInputMethod::PreeditDefault;

    QTest::newRow("One candidate but preedit not in dictionary")
        << "gello"
        << (QStringList()
            << "gello"
            << "fellow")
        << (QList<MImEngine::DictionaryType>()
            << MImEngine::DictionaryTypeInvalid
            << MImEngine::DictionaryTypeLanguage)
        << MInputMethod::PreeditDefault;

    QTest::newRow("No candidates but preedit in dictionary")
        << "hello"
        << (QStringList()
            << "hello")
        << (QList<MImEngine::DictionaryType>()
            << MImEngine::DictionaryTypeLanguage)
        << MInputMethod::PreeditDefault;

    QTest::newRow("No candidates and preedit not in dictionary")
        << "hello"
        << (QStringList()
            << "hello")
        << (QList<MImEngine::DictionaryType>()
            << MImEngine::DictionaryTypeInvalid)
        << MInputMethod::PreeditNoCandidates;
}

void Ut_MKeyboardHost::testPreeditFormat()
{
    QFETCH(QString, preedit);
    QFETCH(QStringList, candidates);
    QFETCH(QList<MImEngine::DictionaryType>, candidateSource);
    QFETCH(MInputMethod::PreeditFace, preeditFormatFace);

    // Fill engine
    stubEngine.setCandidates(candidates);
    stubEngine.setCandidateSources(candidateSource);

    // Reset host
    inputMethodHost->clear();

    // Set preedit
    subject->setPreedit(preedit, -1);

    // Check results
    QCOMPARE(inputMethodHost->sendPreeditCalls, 1);
    QCOMPARE(inputMethodHost->preedit, preedit);
    QVERIFY(inputMethodHost->preeditFormats_.size() > 0);
    QCOMPARE(inputMethodHost->preeditFormats_[0].preeditFace, preeditFormatFace);
}

QTEST_APPLESS_MAIN(Ut_MKeyboardHost);
