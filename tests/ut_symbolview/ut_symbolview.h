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



#ifndef UT_SYMBOLVIEW_H
#define UT_SYMBOLVIEW_H

#include <QtTest/QTest>
#include <QObject>

#include <minputmethodnamespace.h>

class MApplication;
class MImAbstractKey;
class MImAbstractKeyArea;
class MSceneWindow;
class MVirtualKeyboard;
class MVirtualKeyboardStyleContainer;
class SymbolView;

class Ut_SymbolView : public QObject
{
    Q_OBJECT
private:
    MApplication *app;
    MVirtualKeyboardStyleContainer *style;
    SymbolView *subject;
    MSceneWindow *parent;
    QString testLayoutFile;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testReactiveButtonAreas_data();
    void testReactiveButtonAreas();
    void testReactiveWholeScreen_data();
    void testReactiveWholeScreen();

    void testOthers();
    void testChangeToOpenMode();
    void testChangeTab_data();
    void testChangeTab();
    void testHideWithFlick_data();
    void testHideWithFlick();
    void testSetLayout();
    void testHardwareState();
    void testSetTemporarilyHidden();
    void testAutomaticCloseOnKeyClick_data();
    void testAutomaticCloseOnKeyClick();

private:
    void rotateToAngle(MInputMethod::OrientationAngle angle);
    MImAbstractKey *keyAt(MImAbstractKeyArea *symPage,
                          unsigned int row,
                          unsigned int column) const;
};

#endif
