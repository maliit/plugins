/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 *
 */

#include "utils.h"
#include "models/key.h"
#include "models/keyarea.h"
#include "models/layout.h"
#include "logic/layoutupdater.h"

#include <QtCore>
#include <QtTest>

using namespace MaliitKeyboard;

class TestLanguageLayoutSwitching
    : public QObject
{
    Q_OBJECT

private:
    QScopedPointer<QCoreApplication> m_app;


    Q_SLOT void initTestCase()
    {
        m_app.reset(TestUtils::createCoreApplication("language-layout-switching"));
    }

    Q_SLOT void test_data()
    {
        QTest::addColumn<QString>("keyboard_id");
        QTest::addColumn<int>("expected_key_count");

        QTest::newRow("Using invalid keyboard id: expect empty KeyArea.")
            << "invalid_language_layout_id" << 0;

        QTest::newRow("Using 'en_gb' as keyboard id: expect KeyArea with 33 keys.")
            << "en_gb" << 33;

        QTest::newRow("Using 'de' as keyboard id: expect KeyArea with 36 keys.")
            << "de" << 36;
    }

    Q_SLOT void test()
    {
        QFETCH(QString, keyboard_id);
        QFETCH(int, expected_key_count);

        LayoutUpdater layout_updater;
        SharedLayout layout(new Layout);
        layout_updater.setLayout(layout);

        layout_updater.setActiveKeyboardId(keyboard_id);
        TestUtils::waitForSignal(&layout_updater, SIGNAL(layoutChanged(SharedLayout)));

        QCOMPARE(layout->activePanel(), Layout::CenterPanel);
        QCOMPARE(layout->activeKeyArea().keys().count(), expected_key_count);
    }
};

QTEST_APPLESS_MAIN(TestLanguageLayoutSwitching)
#include "main.moc"
