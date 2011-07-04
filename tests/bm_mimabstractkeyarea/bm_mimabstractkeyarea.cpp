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



#include "bm_mimabstractkeyarea.h"
#include "mimkeyarea.h"
#include "keyboarddata.h"
#include "utils.h"

#include <MApplication>
#include <MTheme>

#include <QDir>


void Bm_MImAbstractKeyArea::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "bm_mimabstractkeyarea",
                                 (char *) "-software" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);
}

void Bm_MImAbstractKeyArea::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Bm_MImAbstractKeyArea::init()
{
    keyboard = 0;
    subject = 0;
}

void Bm_MImAbstractKeyArea::cleanup()
{
    delete subject;
    subject = 0;
    delete keyboard;
    keyboard = 0;
}

void Bm_MImAbstractKeyArea::benchmarkPreDraw_data()
{
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    QFileInfoList files;
    QFileInfo info;

    QTest::addColumn<QString>("filename");
    filters << "??.xml";
    files = dir.entryInfoList(filters);
    for (int n = files.count() - 1; n >= 0; --n) {
        info = files.at(n);
        QTest::newRow(info.fileName().toLatin1().constData()) << info.fileName();
    }
    for (int n = files.count() - 1; n >= 0; --n) {
        info = files.at(n);
        QTest::newRow(info.fileName().toLatin1().constData()) << info.fileName();
    }
}

void Bm_MImAbstractKeyArea::benchmarkPreDraw()
{
    QFETCH(QString, filename);

    keyboard = new KeyboardData;
    QVERIFY(keyboard->loadNokiaKeyboard(filename));
    subject = MImKeyArea::create(keyboard->layout(LayoutData::General, MInputMethod::Landscape)->section(LayoutData::mainSection),
                                 false, 0);

    QBENCHMARK {
        subject->updateKeyGeometries(864);
        subject->updateKeyModifiers();
    }
}

void Bm_MImAbstractKeyArea::benchmarkLoadXML_data()
{
    QDir dir("/usr/share/meegotouch/virtual-keyboard/layouts/");
    QStringList filters;
    QFileInfoList files;
    QFileInfo info;

    QTest::addColumn<QString>("filename");
    filters << "??.xml";
    files = dir.entryInfoList(filters);
    for (int n = files.count() - 1; n >= 0; --n) {
        info = files.at(n);
        QTest::newRow(info.fileName().toLatin1().constData()) << info.fileName();
    }
}

// TODO: This test should not be here anymore.
//       Maybe test initialization speed of widget with section data model?
void Bm_MImAbstractKeyArea::benchmarkLoadXML()
{
    QFETCH(QString, filename);

    QBENCHMARK {
        for (int n = 0; n < 5; ++n) {
            keyboard = new KeyboardData;
            QVERIFY(keyboard->loadNokiaKeyboard(filename));
            delete keyboard;
            keyboard = 0;
        }
    }
}

QTEST_APPLESS_MAIN(Bm_MImAbstractKeyArea);
