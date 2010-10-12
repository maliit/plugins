/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */



#include "ut_vkbdatakey.h"
#include "utils.h"
#include <vkbdatakey.h>

#include <MApplication>
#include <MTheme>

#include <QSignalSpy>
#include <QDebug>


Q_DECLARE_METATYPE(Qt::Key)


void Ut_VKBDataKey::initTestCase()
{
    static int argc = 2;
    static char *app_name[2] = { (char *) "ut_vkbdatakey",
                                 (char *) "-local-theme" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);
}

void Ut_VKBDataKey::cleanupTestCase()
{
    delete app;
    app = 0;
}

void Ut_VKBDataKey::init()
{
    subject = new VKBDataKey;
}

void Ut_VKBDataKey::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_VKBDataKey::testCreate()
{
    QVERIFY(subject != 0);
}

void Ut_VKBDataKey::testAccent()
{
    const QString label = "a";
    QChar accentedChars[] = { 0xE0, 0xE1, };
    QChar accents[] = { 0x60, 0xB4, 0x5E, };
    QList<QChar> testAccents;
    QList<QChar> testExpected;

    testAccents
            << accents[0]   //set valid accent
            << QChar()      //set empty accent
            << accents[2]   //set invalid accent: it is not defined for this key
            << QChar(0xA8); //set invalid accent: there is no correponding label

    testExpected
            << accentedChars[0]
            << label[0]
            << label[0]
            << label[0];

    KeyBinding *noShiftBinding = new KeyBinding;
    subject->bindings[VKBDataKey::NoShift] = noShiftBinding;
    noShiftBinding->keyLabel = label;
    noShiftBinding->accented_labels = QString(accentedChars,
                                     sizeof(accentedChars) / sizeof(accentedChars[0]));
    noShiftBinding->accents = QString(accents,
                                      sizeof(accents) / sizeof(accents[0]));

    for (int i = 0; i < testAccents.count(); ++i) {
        QCOMPARE(subject->binding(false)->accented(testAccents.at(i)).at(0),
                 testExpected.at(i));
        QCOMPARE(subject->toKeyEvent(QEvent::KeyRelease, testAccents.at(i), false).text().at(0),
                 testExpected.at(i));
    }
    QCOMPARE(subject->binding(true), static_cast<KeyBinding *>(0));
}

void Ut_VKBDataKey::testKeyCode_data()
{
    QTest::addColumn<QString>("label");
    QTest::addColumn<Qt::Key>("keyCode");
    QTest::newRow("a") << "a" << Qt::Key_A;
    QTest::newRow("d") << "d" << Qt::Key_D;
    QTest::newRow("o") << "o" << Qt::Key_O;
    QTest::newRow("b") << "b" << Qt::Key_B;
    QTest::newRow("e") << "e" << Qt::Key_E;
}

void Ut_VKBDataKey::testKeyCode()
{
    QFETCH(QString, label);
    QFETCH(Qt::Key, keyCode);

    KeyBinding *noShiftBinding = new KeyBinding;
    subject->bindings[VKBDataKey::NoShift] = noShiftBinding;
    noShiftBinding->keyLabel = label;

    QCOMPARE(static_cast<Qt::Key>(subject->toKeyEvent(QEvent::KeyPress, false).toQKeyEvent().key()),
             keyCode);
}

QTEST_APPLESS_MAIN(Ut_VKBDataKey);

