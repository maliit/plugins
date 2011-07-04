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



#ifndef UT_MVIRTUALKEYBOARDHOST_H
#define UT_MVIRTUALKEYBOARDHOST_H

#include <QList>
#include <QtTest/QtTest>
#include <QObject>
#include <QWidget>

#include <minputmethodnamespace.h>

class MApplication;
class MKeyboardHost;
class MInputMethodHostStub;
class MWindow;
struct TestSignalEvent;

class Ut_MKeyboardHost : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MKeyboardHost *subject;
    MInputMethodHostStub *inputMethodHost;
    MWindow *window;
    QWidget *mainWindow;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testCreate();
    void testOrientationAngleLocked();
    void testHandleClick();
    void testDirectMode();
    void testNotCrash();
    void testCorrectionOptions();
    void testCorrectionSettings_data();
    void testCorrectionSettings();
    void testCorrectionContentTypes_data();
    void testCorrectionContentTypes();

    void testAutoCaps();
    void testApplicationOrientationChanged();

    void testCopyPaste();

    void testSendString();
    void testSendStringFromToolbar();

    void testRegionSignals();
    void testOptimizedRegionCallCounts_data();
    void testOptimizedRegionCallCounts();

    void testSetState_data();
    void testSetState();

    void testSymbolKeyClick();
    void testUpdateSymbolViewLevel();

    void testKeyCycle_data();
    void testKeyCycle();

    void testShiftState_data();
    void testShiftState();

    void testCommitPreeditOnStateChange();

    void testLayoutMenuKeyClick_data();
    void testLayoutMenuKeyClick();

    void testShiftStateOnFocusChanged_data();
    void testShiftStateOnFocusChanged();

    void testShiftStateOnLayoutChanged_data();
    void testShiftStateOnLayoutChanged();

    void testToolbar();

    void testHandleHwKeyboardStateChanged_data();
    void testHandleHwKeyboardStateChanged();

    void testUserHide();

    void testWYTIWYSErrorCorrection();

    void testSignalsInNormalMode();
    void testSignalsInDirectMode();

    void testShowLanguageNotification_data();
    void testShowLanguageNotification();

    void testAutoPunctuation_data();
    void testAutoPunctuation();

    void testToolbarPosition();

    void testTogglePlusMinus_data();
    void testTogglePlusMinus();

    void testPreeditFormat_data();
    void testPreeditFormat();

private:
    void rotateToAngle(MInputMethod::OrientationAngle);
    void triggerAutoCaps();
    void testSignals(MInputMethod::InputMethodMode inputMode, const TestSignalEvent *testEvents);

public:
    enum TestOperation {
        ClickShift,
        ClickCharacter,
        ClickCharacterWithShiftDown,
        TriggerAutoCaps
    };

    enum RegionType {
        ScreenRegion,
        InputMethodArea
    };

    QRegion region(RegionType type, int index);
    typedef QList<TestOperation> TestOpList;
};

#endif
