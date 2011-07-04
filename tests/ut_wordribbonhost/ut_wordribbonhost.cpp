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

#include "ut_wordribbonhost.h"
#include "wordribbonhost.h"
#include "utils.h"
#include "regiontracker.h"
#include "mplainwindow.h"
#include "reactionmappainter.h"
#include "mkeyboardhost.h"
#include "minputmethodhoststub.h"

#include <MApplication>
#include <MButton>
#include <MLabel>
#include <MSceneManager>
#include <MSceneWindow>
#include <MNamespace>

#include <QGraphicsLayout>
#include <mtoolbaritem.h>

void Ut_WordRibbonHost::initTestCase()
{
    MApplication::setLoadMInputContext(false);
    static char *argv[2] = { (char *) "ut_wordribbonhost",
                             (char *) "-local-theme" };
    static int argc = 2;
    disableQtPlugins();
    app = new MApplication(argc, argv);

    inputMethodHost = new MInputMethodHostStub;
    mainWindow = new QWidget;
    keyboardHost = new MKeyboardHost(inputMethodHost, mainWindow);

    // MImCorrectionHost uses this internally
    if (MPlainWindow::instance()->orientationAngle() != M::Angle0) {
        MPlainWindow::instance()->setOrientationAngle(M::Angle0);
        QTest::qWait(1000);
    }
    parentWindow = new MSceneWindow;
    parentWindow->setManagedManually(true); // we want the scene window to remain in origin
    // Adds scene window to scene.
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(parentWindow);
}

void Ut_WordRibbonHost::cleanupTestCase()
{
    delete parentWindow;
    delete MPlainWindow::instance();
    delete keyboardHost;
    delete mainWindow;
    delete inputMethodHost;
    delete app;
    app = 0;
}

void Ut_WordRibbonHost::init()
{
    subject = new WordRibbonHost(0, 0);
}

void Ut_WordRibbonHost::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_WordRibbonHost::testsetCandidates()
{
    QStringList candidates;
    QStringList expectedWord;

    candidates << "中华人民共和国" << "芬兰赫尔辛基" << "啊";
    subject->setCandidates(candidates);

    candidates << "中文" << "中文" << "中文"<<"中文";
    subject->setCandidates(candidates);

    subject->finalizeOrientationChange();
    subject->setCandidates(candidates);
    
    subject->setCandidates(candidates);
    QVERIFY(subject->candidates().count() != 0 );
    subject->setCandidates(QStringList());
    QCOMPARE(subject->candidates().count(), 0);
}

void Ut_WordRibbonHost::testShowEngineWidget()
{
    QStringList candidates;
    candidates << "中华人民共和国" << "芬兰赫尔辛基" << "啊";
    subject->setCandidates(candidates);
    subject->showEngineWidget(AbstractEngineWidgetHost::DockedMode);
    QVERIFY(subject->isActive());
    QCOMPARE(subject->displayMode(), AbstractEngineWidgetHost::DockedMode);
}

void Ut_WordRibbonHost::testHideEngineWidget()
{
    QStringList candidates;
    candidates << "中华人民共和国" << "芬兰赫尔辛基" << "啊";
    subject->setCandidates(candidates);
    subject->hideEngineWidget();
    QVERIFY(!subject->isActive());
}

void Ut_WordRibbonHost::testSetTitle()
{
    QString title("芬兰");
    subject->setTitle(title);
    QCOMPARE(subject->dialogTitle, title);
}

void Ut_WordRibbonHost::testSetPageIndex()
{
    QStringList candidates;
    candidates << "中华" << "芬兰" << "啊";
    subject->setCandidates(candidates);
    // test no crash?
    subject->setPageIndex(0);
}

void Ut_WordRibbonHost::testFetchCandidates()
{
    QStringList totalCandidateList;
    QStringList oneTimeCandidateList;

    oneTimeCandidateList << "0" << "1" << "2" <<"3" <<"4" <<"5" <<"6";
    totalCandidateList.append(oneTimeCandidateList);

    subject->setCandidates(oneTimeCandidateList);
    subject->fetchMoreCandidates();
    QCOMPARE(compareCandidates(totalCandidateList), true);
}

void Ut_WordRibbonHost::testHandleNavigationKey()
{
    QSignalSpy spyClick(subject,SIGNAL(candidateClicked(QString,int)));
    QCOMPARE(spyClick.count(), 0);

    QStringList candidates;
    candidates << "中华" << "芬兰" << "啊";
    subject->finalizeOrientationChange();
    subject->setCandidates(candidates);
    subject->handleNavigationKey(AbstractEngineWidgetHost::NaviKeyOk);
    QCOMPARE(spyClick.count(), 1);
    QList<QVariant> arguments1 = spyClick.takeFirst();
    QVERIFY(arguments1.at(0).toString() == "中华");
    QVERIFY(arguments1.at(1).toInt() == 0);
}

void Ut_WordRibbonHost::testOpenDialog()
{
    QStringList candidateList;
    candidateList << "0" << "1" << "2" <<"3" <<"4" <<"5" <<"6";
    subject->setCandidates(candidateList);

    subject->openWordRibbonDialog();
    QVERIFY(subject->isActive());
    QCOMPARE(subject->displayMode(), AbstractEngineWidgetHost::DialogMode);
}

bool Ut_WordRibbonHost::compareCandidates(QStringList totalCandidateList)
{
    if (subject->candidatesCache.count() != totalCandidateList.count()) {
        qDebug() <<"WARNING: Ut_WordRibbonHost::compareCandidates() Length not equal!"
                 <<"totalCandidateList length = " <<totalCandidateList.count()
                 <<"subject->candidatesCache length = " <<subject->candidatesCache.count();
        return false;
    }

    QString eachCachedString;
    for (int i = 0; i < subject->candidatesCache.count(); ++i) {
        eachCachedString = subject->candidatesCache.at(i);
        if (eachCachedString != totalCandidateList.at(i))
            return false;
    }
    return true;
}

QTEST_APPLESS_MAIN(Ut_WordRibbonHost);
