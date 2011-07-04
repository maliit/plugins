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



#ifndef UT_MVIRTUALKEYBOARD_H
#define UT_MVIRTUALKEYBOARD_H

#include "layoutdata.h"
#include <QObject>
#include <QSharedPointer>
#include <QtTest/QTest>

class MApplication;
class MVirtualKeyboard;
class MVirtualKeyboardStyleContainer;
class MImAbstractKeyArea;
class QGraphicsScene;
class LayoutSection;
class MSceneWindow;


class Ut_MVirtualKeyboard : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MVirtualKeyboard *m_vkb;
    MSceneWindow *vkbParent;
    MVirtualKeyboardStyleContainer *vkbStyleContainer;
    const LayoutData::SharedLayoutSection functionkeySection;
    int numFunctionKeys;

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    void clickBackspaceTest();
    void clickSpaceTest();
    void setShiftStateTest();
    void clickHyphenTest();
    void clickPunctQuesTest();
    void clickPunctDotTest();
    void testStateReset();
    void testShiftLevelChange_data();
    void testShiftLevelChange();
    void flickRightHandlerTest();
    void flickLeftHandlerTest();
    void loadSymbolViewTemporarilyTest();
    void errorCorrectionTest();
    void setKeyboardType();
    void longPressBackSpace();
    void bug_130644();
    void symbolKeyTestLowercase();
    void symbolKeyTestCapsLock();
    void interceptPress();
    void bug_137295();
    void testReactionMaps();

signals:
    void fade(int);

private:
    void rotateToAngle(MInputMethod::OrientationAngle angle);
};

#endif

