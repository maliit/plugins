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



#include "ft_mxkb.h"
#include <MApplication>
#include <mplainwindow.h>
#include <minputmethodnamespace.h>
#include <QDebug>
#include <QSignalSpy>
#include <QKeyEvent>
#include "mxkb.h"
#include "utils.h"

#include <X11/X.h>
#include <X11/XKBlib.h>

namespace {
    const unsigned int FnModifierMask = Mod5Mask;
};

void Ft_MXkb::initTestCase()
{
    disableQtPlugins();

    static char *argv[3] = {(char *) "ft_mxkb", (char *) "-software", (char *) "-software"};
    static int argc = 3;
    app = new MApplication(argc, argv);
}

void Ft_MXkb::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ft_MXkb::init()
{
    m_subject = new MXkb;
    m_subject->lockModifiers(ShiftMask | FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));
    QVERIFY(!testModifierLatchedState(FnModifierMask));
}

void Ft_MXkb::cleanup()
{
    m_subject->lockModifiers(ShiftMask | FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));
    QVERIFY(!testModifierLatchedState(FnModifierMask));
    delete m_subject;
}

void Ft_MXkb::testLockModifiers()
{
    // lock shift
    m_subject->lockModifiers(ShiftMask, ShiftMask);
    QVERIFY(testModifierLatchedState(ShiftMask));

    // unlock shift
    m_subject->lockModifiers(ShiftMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));

    // lock fn
    m_subject->lockModifiers(FnModifierMask, FnModifierMask);
    QVERIFY(testModifierLatchedState(FnModifierMask));

    // unlock fn
    m_subject->lockModifiers(FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(FnModifierMask));

    // lock both
    m_subject->lockModifiers(ShiftMask | FnModifierMask, ShiftMask | FnModifierMask);
    QVERIFY(testModifierLatchedState(ShiftMask));
    QVERIFY(testModifierLatchedState(FnModifierMask));

    // unlock both
    m_subject->lockModifiers(ShiftMask | FnModifierMask, 0);
    QVERIFY(!testModifierLatchedState(ShiftMask));
    QVERIFY(!testModifierLatchedState(FnModifierMask));
}

bool Ft_MXkb::testModifierLatchedState(int xModifier) const
{
    Window dummy1, dummy2;
    int dummy3, dummy4, dummy5, dummy6;
    unsigned int mask;
    XQueryPointer(QX11Info::display(), DefaultRootWindow(QX11Info::display()), &dummy1, &dummy2,
                  &dummy3, &dummy4, &dummy5, &dummy6, &mask);
    return (mask & xModifier);
}

QTEST_APPLESS_MAIN(Ft_MXkb);
