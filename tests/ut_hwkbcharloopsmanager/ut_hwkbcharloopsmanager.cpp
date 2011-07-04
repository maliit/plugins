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



#include <mgconfitem_stub.h>
#include "hwkbcharloopsmanager.h"
#include "hwkbcharloops.h"
#include "ut_hwkbcharloopsmanager.h"
#include "utils.h"
#include <QDebug>
#include <memory>


namespace
{
    const QString SystemDisplayLanguage("/meegotouch/i18n/language");
}

bool HwKbCharLoopsManager::loadCharLoops(const QString &fileName)
{
    Q_UNUSED(fileName);
    qDeleteAll(charLoops);
    charLoops.clear();

    charLoops.insert("en_gb", new HwKbCharacterLoops("en_gb", ""));
    charLoops.insert("fi", new HwKbCharacterLoops("fi", ""));
    charLoops.insert("ar", new HwKbCharacterLoops("ar", ""));
    return true;
}

void Ut_HwKbCharLoopsManager::initTestCase()
{
    disableQtPlugins();
    m_subject = new HwKbCharLoopsManager;
}

void Ut_HwKbCharLoopsManager::cleanupTestCase()
{
    delete m_subject;
    m_subject = 0;
}

void Ut_HwKbCharLoopsManager::init()
{
    QVERIFY(m_subject->setCharLoopsLanguage("en_gb"));
}

void Ut_HwKbCharLoopsManager::cleanup()
{
}

void Ut_HwKbCharLoopsManager::testLoadLanguage()
{
    QVERIFY(m_subject->setCharLoopsLanguage("en_gb"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("en_gb"));
    QVERIFY(m_subject->setCharLoopsLanguage("fi"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("fi"));
    QVERIFY(m_subject->setCharLoopsLanguage("ar"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("ar"));
}

void Ut_HwKbCharLoopsManager::testSyncLanguage()
{
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("en_gb"));
    MImSettings systemDisplayLanguage(SystemDisplayLanguage);
    systemDisplayLanguage.set(QVariant("fi"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("fi"));
    systemDisplayLanguage.set(QVariant("ar"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("ar"));

    // If no specific match is found, less specific match is used, if available
    systemDisplayLanguage.set(QVariant("fi_FO"));
    QCOMPARE(m_subject->currentCharLoopLanguage, QString("fi_FO"));
    QVERIFY(m_subject->current);
    QCOMPARE(m_subject->current->language, QString("fi"));
}

QTEST_APPLESS_MAIN(Ut_HwKbCharLoopsManager);

