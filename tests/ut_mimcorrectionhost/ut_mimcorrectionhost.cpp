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



#include "ut_mimcorrectionhost.h"
#include "abstractenginewidgethost.h"
#include "mkeyboardhost.h"
#include "enginemanager.h"
#include "mimwordtracker.h"
#include "mimwordlist.h"
#include "reactionmappainter.h"
#include "utils.h"
#include "minputmethodhoststub.h"
#include "mplainwindow.h"

#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QSignalSpy>
#include <MSceneManager>
#include <MSceneWindow>
#include <MNamespace>

Q_DECLARE_METATYPE(AbstractEngineWidgetHost::DisplayMode)

void Ut_MImCorrectionHost::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimcorrectioncandidatewidget",
                                  (char *) "-software" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);

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


void Ut_MImCorrectionHost::init()
{
    m_subject = new MImCorrectionHost(0, parentWindow);

    if (MPlainWindow::instance()->orientationAngle() != M::Angle0) {
        MPlainWindow::instance()->setOrientationAngle(M::Angle0);
        QTest::qWait(1000);
    }
}


void Ut_MImCorrectionHost::cleanup()
{
    delete m_subject;
}

void Ut_MImCorrectionHost::cleanupTestCase()
{
    delete parentWindow;
    delete MPlainWindow::instance();
    delete keyboardHost;
    delete mainWindow;
    delete inputMethodHost;
    delete app;
    app = 0;
}

void Ut_MImCorrectionHost::checkShowWidget()
{
    QStringList candidates;
    candidates << "1" << "2" << "3" << "4" << "5";
    m_subject->setCandidates(candidates);
    // At least, no crash for show and hide
    m_subject->showEngineWidget(AbstractEngineWidgetHost::FloatingMode);
    m_subject->hideEngineWidget();
    m_subject->showEngineWidget(AbstractEngineWidgetHost::DialogMode);
    m_subject->hideEngineWidget();
}

void Ut_MImCorrectionHost::checkModes()
{
    QStringList candidates;
    candidates << "1" << "2" << "3" << "4" << "5";
    m_subject->setCandidates(candidates);
    m_subject->showEngineWidget(AbstractEngineWidgetHost::FloatingMode);
    QCOMPARE(m_subject->displayMode(),  AbstractEngineWidgetHost::FloatingMode);
    m_subject->showEngineWidget(AbstractEngineWidgetHost::DialogMode);
    QCOMPARE(m_subject->displayMode(), AbstractEngineWidgetHost::DialogMode);
}

void Ut_MImCorrectionHost::checkCandidatesAndPreedit_data()
{
    QTest::addColumn<QStringList>("candidates");

    QTest::newRow("test1")  << (QStringList() << "1" << "2" << "3" << "4" << "5");
    QTest::newRow("test2")  << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh");
}

void Ut_MImCorrectionHost::checkCandidatesAndPreedit()
{
    QFETCH(QStringList, candidates);

    m_subject->setCandidates(candidates);
    QCOMPARE(candidates, m_subject->candidates());
}

void Ut_MImCorrectionHost::checkSuggestion_data()
{
    QTest::addColumn<QStringList>("candidates");
    QTest::addColumn<AbstractEngineWidgetHost::DisplayMode>("displayMode");
    QTest::addColumn<QString>("clickedCandidate");

    QTest::newRow("testWordTrackerMode") << (QStringList() << "1" << "2" << "3" << "4" << "5")
                            << AbstractEngineWidgetHost::FloatingMode
                            << "3";
    QTest::newRow("testWordListMode") << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh")
                            << AbstractEngineWidgetHost::DialogMode
                            << "gh";
}

void Ut_MImCorrectionHost::checkSuggestion()
{
    QFETCH(QStringList, candidates);
    QFETCH(AbstractEngineWidgetHost::DisplayMode, displayMode);
    QFETCH(QString, clickedCandidate);

    QSignalSpy spy(m_subject, SIGNAL(candidateClicked(const QString &, int)));

    m_subject->setCandidates(candidates);
    m_subject->showEngineWidget(displayMode);
    if (displayMode == AbstractEngineWidgetHost::DialogMode) {
        QTest::qWait(600);
    }
    QCOMPARE(m_subject->candidates(), candidates);
    // default suggestion is the first one in candidate list which is
    // different with preedit
    QCOMPARE(m_subject->suggestedWordIndex(), 1);

    m_subject->handleCandidateClicked(clickedCandidate);

    // suggestion is the clciked word
    QCOMPARE(m_subject->suggestedWordIndex(), candidates.indexOf(clickedCandidate));
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.first().first().toString(), clickedCandidate);
}

void Ut_MImCorrectionHost::checkpendingCandidatesUpdate_data()
{
    QTest::addColumn<QStringList>("candidates");
    QTest::addColumn<AbstractEngineWidgetHost::DisplayMode>("displayMode");
    QTest::addColumn<QStringList>("updatedCandidates");

    QTest::newRow("testWordTrackerMode") << (QStringList() << "1" << "2" << "3" << "4" << "5")
                            << AbstractEngineWidgetHost::FloatingMode
                            << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh");
    QTest::newRow("testWordListMode") << (QStringList() << "ab" << "cd" << "ef" << "fg" << "gh")
                            << AbstractEngineWidgetHost::DialogMode
                            << (QStringList() << "1" << "2" << "3" << "4" << "5");
}

void Ut_MImCorrectionHost::checkpendingCandidatesUpdate()
{
    QFETCH(QStringList, candidates);
    QFETCH(AbstractEngineWidgetHost::DisplayMode, displayMode);
    QFETCH(QStringList, updatedCandidates);

    QCOMPARE(m_subject->isActive(), false);
    QCOMPARE(m_subject->pendingCandidatesUpdate, false);
    m_subject->setCandidates(candidates);
    QCOMPARE(m_subject->pendingCandidatesUpdate, true);

    QVERIFY(m_subject->candidates() != m_subject->wordList->candidates());

    m_subject->showEngineWidget(displayMode);
    QTest::qWait(600);

    QCOMPARE(m_subject->pendingCandidatesUpdate, false);
    // only update the wordtracker or wordlist according current mode
    if (displayMode == AbstractEngineWidgetHost::FloatingMode) {
        QVERIFY(m_subject->candidates() != m_subject->wordList->candidates());
    } else {
        QVERIFY(m_subject->candidates() == m_subject->wordList->candidates());
    }

    m_subject->setCandidates(updatedCandidates);
    QCOMPARE(m_subject->pendingCandidatesUpdate, false);

    if (displayMode == AbstractEngineWidgetHost::FloatingMode) {
        QVERIFY(m_subject->candidates() != m_subject->wordList->candidates());
    } else {
        QVERIFY(m_subject->candidates() == m_subject->wordList->candidates());
    }
}

QTEST_APPLESS_MAIN(Ut_MImCorrectionHost);

