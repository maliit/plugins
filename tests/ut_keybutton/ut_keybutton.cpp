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



#include "ut_keybutton.h"

#include "keybuttonareastyle.h"
#include "singlewidgetbutton.h"
#include "singlewidgetbuttonarea.h"
#include "vkbdatakey.h"
#include "utils.h"

#include <MApplication>
#include <MTheme>

#include <QSignalSpy>
#include <QDebug>

void Ut_KeyButton::initTestCase()
{
    static int argc = 2;
    static char *app_name[] = { (char*) "ut_keybutton",
                                (char *) "-local-theme" };

    disableQtPlugins();
    app = new MApplication(argc, app_name);

    style = new KeyButtonAreaStyleContainer;
    style->initialize("", "", 0);

    parent = new QGraphicsWidget;
    dataKey = createDataKey();
}

void Ut_KeyButton::cleanupTestCase()
{
    delete style;
    delete dataKey;
    delete app;
    app = 0;
    delete parent;
}

void Ut_KeyButton::init()
{
    subject = new SingleWidgetButton(*dataKey, *style, *parent);
}

void Ut_KeyButton::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_KeyButton::testSetModifier_data()
{
    QTest::addColumn<bool>("shift");
    QTest::addColumn<QChar>("accent");
    QTest::addColumn<QString>("expectedLabel");

    QChar grave(L'`');
    QChar aigu(L'´');
    QChar circonflexe(L'^');

    QTest::newRow("no shift, no accent")            << false << QChar() << "a";
    QTest::newRow("no shift, l'accent grave")       << false << grave << QString(L'à');
    QTest::newRow("no shift, l'accent aigu")        << false << aigu << QString(L'á');
    QTest::newRow("no shift, l'accent circonflexe") << false << circonflexe << QString(L'â');
    QTest::newRow("shift, no accent")               << true  << QChar() << "A";
    QTest::newRow("shift, l'accent grave")          << true  << grave << QString(L'À');
}

void Ut_KeyButton::testSetModifier()
{
    QFETCH(bool, shift);
    QFETCH(QChar, accent);
    QFETCH(QString, expectedLabel);

    subject->setModifiers(shift, accent);
    QCOMPARE(subject->label(), expectedLabel);
}

void Ut_KeyButton::testKey()
{
    QCOMPARE(&subject->key(), dataKey);
}

void Ut_KeyButton::testBinding()
{
    bool shift = false;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), dataKey->binding(shift));

    shift = true;
    subject->setModifiers(shift);
    QCOMPARE(&subject->binding(), dataKey->binding(shift));
}

void Ut_KeyButton::testIsDead()
{
    VKBDataKey *key = new VKBDataKey;
    KeyBinding *binding = new KeyBinding;
    key->bindings[VKBDataKey::NoShift] = binding;

    IKeyButton *subject = new SingleWidgetButton(*key, *style, *parent);

    for (int i = 0; i < 2; ++i) {
        bool isDead = (i != 0);
        binding->dead = isDead;
        QCOMPARE(subject->isDeadKey(), isDead);
    }

    delete subject;
    delete key;
}

VKBDataKey *Ut_KeyButton::createDataKey()
{
    VKBDataKey *key = new VKBDataKey;

    KeyBinding *binding1 = new KeyBinding;
    binding1->keyLabel = "a";
    binding1->dead = false;
    binding1->accents = "`´^¨";
    binding1->accented_labels = QString(L'à') + L'á' + L'á' + L'â' + L'ä';

    KeyBinding *binding2 = new KeyBinding;
    binding2->keyLabel = "A";
    binding2->dead = false;
    binding2->accents = "`´^¨";
    binding2->accented_labels = QString(L'À') + L'Á' + L'Â' + L'Ä';

    key->bindings[VKBDataKey::NoShift] = binding1;
    key->bindings[VKBDataKey::Shift] = binding2;

    return key;
}

QTEST_APPLESS_MAIN(Ut_KeyButton);
