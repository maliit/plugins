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



#include "ut_wordribbondialog.h"
#include "wordribbondialog.h"
#include "wordribbondialogmodel.h"
#include "mwidget.h"
#include "mplainwindow.h"
#include "utils.h"
#include "regiontracker.h"
#include "mwidget-wrapper.h"

#include <MTheme>
#include <MList>
#include <QtTest/QTest>
#include <QObject>
#include <QDebug>
#include <QStringList>
#include <QStringListModel>
#include <QSignalSpy>
#include <QGraphicsSceneMouseEvent>
#include <MSceneManager>

#include <QMetaType>
#include <MApplication>
#include <MApplicationWindow>

#include <MButton>
#include <MDialog>
#include <MSceneWindow>
#include <MLayout>

#include <mbuttonmodel.h>



void Ut_WordRibbonDialog::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_wordribbondialog",
                                  (char *) "-local-theme" };
    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);
    view = new MPlainWindow;

}

void Ut_WordRibbonDialog::cleanupTestCase()
{
    delete view;
    view = 0;
    delete app;
    app = NULL;
}

void Ut_WordRibbonDialog::init()
{
    RegionTracker::createInstance();
    appWin = new MWindow;
    QVERIFY(appWin != 0);
    subject = new WordRibbonDialog();
    QVERIFY(subject != 0);

    QApplication::processEvents(QEventLoop::WaitForMoreEvents, 10);
}

void Ut_WordRibbonDialog::cleanup()
{
    delete subject;
    subject = 0;
    delete appWin;
    appWin = 0;
    RegionTracker::destroyInstance();
}

void Ut_WordRibbonDialog::testSetCandidates()
{
    subject->appear(appWin);

    QStringList list;
    list << "aaa" << "bbb" << "ccc" << "d" << "e" << "f" << "g" << "h" << "i"
          << "j" << "k" << "llll" << "mmm" << "n" << "o" << "pp" << "q" << "r";

    subject->setCandidates(list, "test");
    QVERIFY(subject->candidatesList == list);
    QVERIFY(subject->dataModel->rowCount() > 2);
    if (subject->dataModel->rowCount() > 2) {
        QStringList tmpList;
        for (int i = 0; i < subject->dataModel->rowCount(); ++i) {
            tmpList += subject->dataModel->data(subject->dataModel->index(i), Qt::DisplayRole).toStringList();
        }
        QVERIFY(tmpList == list);
    }

    list.clear();
    list << "a";

    subject->setCandidates(list, "test");
    QVERIFY(subject->candidatesList == list);
    QVERIFY(subject->dataModel->rowCount() == 1);
    if (subject->dataModel->rowCount() == 1) {
        QStringList tmpList;
        for (int i = 0; i < subject->dataModel->rowCount(); ++i) {
            tmpList += subject->dataModel->data(subject->dataModel->index(i), Qt::DisplayRole).toStringList();
        }
        QVERIFY(tmpList == list);
    }

    list.clear();

    subject->setCandidates(list, "test");
    QVERIFY(subject->candidatesList == list);
    QVERIFY(subject->dataModel->rowCount() == 0);
}

QTEST_APPLESS_MAIN(Ut_WordRibbonDialog);


