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

#include "ut_sharedhandlearea.h"

#include "reactionmappainter.h"
#include <utils.h>
#include <sharedhandlearea.h>
#include <mgconfitem_stub.h>
#include <mplainwindow.h>
#include <MSceneWindow>
#include <regiontracker.h>

#include <MApplication>
#include <MScene>

namespace
{
    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");
}

void Ut_SharedHandleArea::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_sharedhandlearea",
        (char *) "-software"  };
    // this value is required by the theme daemon
    MImSettings(TargetSettingsName).set(DefaultTargetName);

    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);
    RegionTracker::createInstance();
    ReactionMapPainter::createInstance();

    sceneWindow = createMSceneWindow(new MPlainWindow); // also create singleton

    parent = new QGraphicsWidget;
    MPlainWindow::instance()->scene()->addItem(parent);
}

void Ut_SharedHandleArea::cleanupTestCase()
{
    delete parent;
    delete sceneWindow;
    delete MPlainWindow::instance();
    RegionTracker::destroyInstance();
    ReactionMapPainter::destroyInstance();
    delete app;
}

void Ut_SharedHandleArea::init()
{
    imToolbar = new MImToolbar;
    subject = new SharedHandleArea(*imToolbar, parent);
}

void Ut_SharedHandleArea::cleanup()
{
    delete subject;
    delete imToolbar;
}

void Ut_SharedHandleArea::testWatching()
{
    QGraphicsWidget *widget1 = new QGraphicsWidget(parent);
    QGraphicsWidget *widget2 = new QGraphicsWidget(parent);
    QRectF pos1(10, 20, 30, 40);
    QRectF pos2(10, 200, 30, 40);

    widget1->setGeometry(pos1);
    widget2->setGeometry(pos2);

    subject->watchOnWidget(widget1);
    subject->watchOnWidget(widget2);

    QCOMPARE(subject->geometry().bottom(), widget1->geometry().top());

    pos1.setTop(30);
    widget1->setGeometry(pos1);
    QCOMPARE(subject->geometry().bottom(), widget1->geometry().top());

    pos2.setTop(100);
    widget2->setGeometry(pos2);
    QCOMPARE(subject->geometry().bottom(), widget1->geometry().top());

    widget1->hide();
    QCOMPARE(subject->geometry().bottom(), widget2->geometry().top());

    widget1->show();
    QCOMPARE(subject->geometry().bottom(), widget1->geometry().top());

    delete widget1;
    delete widget2;
}

QTEST_APPLESS_MAIN(Ut_SharedHandleArea)

