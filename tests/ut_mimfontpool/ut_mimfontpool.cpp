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



#include "ut_mimfontpool.h"
#include "utils.h"
#include "mgconfitem_stub.h"

#include <MApplication>

#include <QFile>


namespace
{
    const QString TargetSettingsName("/meegotouch/target/name");
    const QString DefaultTargetName("Default");
}


void Ut_MImFontPool::initTestCase()
{
    static int dummyArgc = 2;
    static char *dummyArgv[2] = { (char *) "./ut_mimfontallocator",
                                  (char *) "-software" };
    // this value is required by the theme daemon
    MImSettings(TargetSettingsName).set(DefaultTargetName);

    disableQtPlugins();
    app = new MApplication(dummyArgc, dummyArgv);

    style = QSharedPointer<MImAbstractKeyAreaStyleContainer>(new MImAbstractKeyAreaStyleContainer);
    style->initialize("", "", 0);
}

void Ut_MImFontPool::init()
{
    subject = QSharedPointer<MImFontPool>(new MImFontPool(true));
    subject->setDefaultFont((*style)->font());
}


void Ut_MImFontPool::cleanup()
{
    subject.clear();
}

void Ut_MImFontPool::cleanupTestCase()
{
    style.clear();
    delete app;
    app = 0;
}

void Ut_MImFontPool::testSharedFont()
{
    MImSharedKeyFontData sharedFont = subject->font(true);

    QVERIFY(subject->font(true) == sharedFont);
    QVERIFY(subject->font(false) != sharedFont);

    subject->reset();

    QVERIFY(subject->font(true) != sharedFont);
    QVERIFY(subject->font(true) == subject->font(true));
}

void Ut_MImFontPool::testDedicatedFont()
{
    subject = QSharedPointer<MImFontPool>(new MImFontPool(false));
    subject->setDefaultFont((*style)->font());

    MImSharedKeyFontData sharedFont = subject->font(true);

    QVERIFY(subject->font(true) != sharedFont);
    QVERIFY(subject->font(false) != sharedFont);

    subject->reset();

    QVERIFY(subject->font(true) != sharedFont);
    QVERIFY(subject->font(true) != subject->font(true));
}

QTEST_APPLESS_MAIN(Ut_MImFontPool);

