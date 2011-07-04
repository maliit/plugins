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



#include "ut_mtoolbarbutton.h"
#include "mtoolbarbutton.h"
#include "mapplication.h"
#include "utils.h"
#include <mtoolbardata.h>
#include <mtoolbarlayout.h>
#include "mgconfitem_stub.h"

#include <QFile>


namespace
{
    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");

    QString ToolbarFileName  = "/testtoolbar.xml";

    QSharedPointer<MToolbarItem> findItem(QSharedPointer<MToolbarData> toolbar, const QString &name)
    {
        if (!toolbar)
            return QSharedPointer<MToolbarItem>();

        foreach(const QSharedPointer<MToolbarItem> &item, toolbar->items()) {
            if (item->name() == name)
                return item;
        }

        return QSharedPointer<MToolbarItem>();
    }

}


// Stubbing..................................................................

void MToolbarButton::setIconFile(const QString &newIconFile)
{
    //reimplement to avoid checking the exist of icon file.
    iconFile = newIconFile;
}

void Ut_MToolbarButton::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mtoolbarbutton",
                                  (char *) "-software" };
    // this value is required by the theme daemon
    MImSettings(TargetSettingsName).set(DefaultTargetName);

    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);

    ToolbarFileName = QCoreApplication::applicationDirPath() + ToolbarFileName;
    QVERIFY(QFile::exists(ToolbarFileName));
}

void Ut_MToolbarButton::init()
{
    //fill up toolbar with some data
    toolbarData = QSharedPointer<MToolbarData>(new MToolbarData);
    bool ok = toolbarData->loadToolbarXml(ToolbarFileName);
    QVERIFY(ok);
}


void Ut_MToolbarButton::cleanup()
{
    subject.clear();
}

void Ut_MToolbarButton::cleanupTestCase()
{
    toolbarData.clear();
    delete app;
    app = 0;
}

void Ut_MToolbarButton::testStylingName_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<QString>("styleName");

    QTest::newRow("text") << "text" << "MToolbarTextButton";
    QTest::newRow("icon") << "icon" << "MToolbarIconButton";
    QTest::newRow("text highlighted") << "textHighlighted" << "MToolbarTextButtonHighlighted";
    QTest::newRow("icon highlighted") << "iconHighlighted" << "MToolbarIconButtonHighlighted";
}

void Ut_MToolbarButton::testStylingName()
{
    QFETCH(QString, name);
    QFETCH(QString, styleName);

    QSharedPointer<MToolbarItem> item = findItem(toolbarData, name);
    QVERIFY(item);

    subject = QSharedPointer<MToolbarButton>(new MToolbarButton(item));
    QCOMPARE(subject->styleName(), styleName);
}

QTEST_APPLESS_MAIN(Ut_MToolbarButton);

