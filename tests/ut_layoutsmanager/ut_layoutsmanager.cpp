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



#include "ut_layoutsmanager.h"
#include <QDebug>
#include <mgconfitem_stub.h>
#include <keyboarddata.h>
#include <layoutdata.h>
#include <layoutsmanager.h>
#include <memory>


namespace
{
    const QString LayoutListSettingName("/meegotouch/inputmethods/virtualkeyboard/layouts");
    //const QString NumberFormatSettingName("/meegotouch/inputmethods/numberformat");
    const QString NumberFormatSettingName("/meegotouch/i18n/lc_numeric");
    const QString LatinNumberFormat("latin");
    const QString ArabicNumberFormat("ar");
    const QString RussianNumberFormat("ru");
    const QString NumberKeyboardFileArabic("number_ar.xml");
    const QString NumberKeyboardFileLatin("number.xml");
    const QString PhoneNumberKeyboardFileArabic("phonenumber_ar.xml");
    const QString PhoneNumberKeyboardFileLatin("phonenumber.xml");
    const QString PhoneNumberKeyboardFileRussian("phonenumber_ru.xml");
    const QString SymbolKeyboardFileCommon("hwsymbols_common.xml");
    const QString SymbolKeyboardFileChinese("hwsymbols_chinese.xml");
    // Our keyboard loader stub fails for filenames not in this list
    QStringList LoadableKeyboards;
}

// Partial stubbing to control (fake) loading of layout files................

class TestLayoutModel : public LayoutData
{
public:
    TestLayoutModel(const QString &id);

    const QString modelId;
};

TestLayoutModel::TestLayoutModel(const QString &id)
    : modelId(id)
{
}

bool KeyboardData::loadNokiaKeyboard(const QString &fileName)
{
    qDeleteAll(layouts);
    layouts.clear();
    // ignore case sensitive in file name.
    if (LoadableKeyboards.contains(fileName, Qt::CaseInsensitive)) {
        layouts.append(new TestLayoutModel(fileName));
        return true;
    }
    return false;
}

const LayoutData *KeyboardData::layout(LayoutData::LayoutType /* type */,
                                       M::Orientation /* orientation */,
                                       bool /* portraitFallback */) const
{
    return layouts.empty() ? NULL : layouts.at(0);
}


// Test init/deinit..........................................................

void Ut_LayoutsManager::initTestCase()
{
}

void Ut_LayoutsManager::cleanupTestCase()
{
}

void Ut_LayoutsManager::init()
{
    LoadableKeyboards.clear();
    LoadableKeyboards << "fi.xml" << "ru.xml" << "ar.xml";
    MGConfItem layoutListSetting(LayoutListSettingName);
    QStringList layoutList;
    layoutList << "fi.xml" << "ru.xml" << "ar.xml";
    layoutListSetting.set(QVariant(layoutList));
}

void Ut_LayoutsManager::cleanup()
{
}


// Tests.....................................................................

void Ut_LayoutsManager::testLayouts()
{
    LoadableKeyboards.clear();
    LoadableKeyboards << "fi.xml" << "ru.xml" << "ar.xml" << "en_gb.xml";
    MGConfItem layoutListSetting(LayoutListSettingName);
    QStringList layoutList;
    layoutList << "fi.xml" << "ru.xml" << "ar.xml" << "en_gb.xml";
    layoutListSetting.set(QVariant(layoutList));
    std::auto_ptr<LayoutsManager> subject(new LayoutsManager);
    foreach (const QString &layout, layoutList) {
        QVERIFY(subject->layoutFileList().contains(layout));
    }
}

void Ut_LayoutsManager::testNumberLayouts()
{
    // region number setting decide the number/phone number format
    // Number layout:
    //
    // - Regional number setting: Arabic -> Arabic layout
    // - Any other regional number setting -> Latin layout
    std::auto_ptr<LayoutsManager> subject(new LayoutsManager);
    MGConfItem numberFormatSetting(NumberFormatSettingName);

    // No default when nothing could be loaded
    const TestLayoutModel *layout = dynamic_cast<const TestLayoutModel *>(
                                        subject->layout("fi.xml", LayoutData::Number, M::Landscape));
    QVERIFY(!layout);

    // Latin is used when it can be loaded and when number format setting has no value
    LoadableKeyboards << NumberKeyboardFileLatin;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // Also when number format setting is Arabic and Arabic cannot be loaded
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // And of course when number format setting is Latin and it can be loaded
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // And when the number format setting is invalid
    numberFormatSetting.set(QVariant("invalid"));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // We don't want to use Arabic as fallback if Latin cannot be loaded, even if
    // Arabic can be loaded
    LoadableKeyboards.clear();
    LoadableKeyboards << NumberKeyboardFileArabic;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, M::Landscape));
    QVERIFY(!layout);
    // Asking with Arabic layout shouldn't make a difference as this
    // stuff works solely based on the number format setting.
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar.xml", LayoutData::Number, M::Landscape));
    QVERIFY(!layout);

    // If, however, number format is set to Arabic and we can load it,
    // Arabic number layout is used with all layouts
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileArabic);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar.xml", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileArabic);

    // We can change the setting to Latin on the fly
    LoadableKeyboards << NumberKeyboardFileLatin;
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi,xml", LayoutData::Number, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);
}

void Ut_LayoutsManager::testPhoneNumberLayouts()
{
    // region number setting decide the phone number format
    // Phone number layout:
    //
    // - Regional number setting: Arabic -> Arabic layout
    // - Regional number setting: Russian -> Russian layout
    // - Otherwise -> Latin layout
    //
    MGConfItem numberFormatSetting(NumberFormatSettingName);
    LoadableKeyboards << NumberKeyboardFileLatin << NumberKeyboardFileArabic;

    // If we can't load any phone number keyboard, we can't have a layout
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    std::auto_ptr<LayoutsManager> subject(new LayoutsManager);
    const TestLayoutModel *layout = dynamic_cast<const TestLayoutModel *>(
                                        subject->layout("fi.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(!layout);

    // our number format is Arabic...
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    LoadableKeyboards << PhoneNumberKeyboardFileLatin;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);
    // ...or latin
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);

    qDebug() << "Foo";
    // In normal operation with Latin number format we get...
    LoadableKeyboards << PhoneNumberKeyboardFileArabic << PhoneNumberKeyboardFileRussian;
    subject.reset(new LayoutsManager);
    // ...Russian phone numbers for Russian number format
    numberFormatSetting.set(QVariant(RussianNumberFormat));
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileRussian);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileRussian);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileRussian);

    // But if we switch to Arabic number format, we always get Arabic phone numbers
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    // That is, for Russian language...
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileArabic);
    // ...and, say, Finnish...
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileArabic);
    // ...and of course for Arabic.
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar.xml", LayoutData::PhoneNumber, M::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileArabic);
}

void Ut_LayoutsManager::testHardwareSymLayout_data()
{
    const QString &SymbolKeyboardFileDefault(SymbolKeyboardFileCommon);

    QTest::addColumn<QString>("xkbLayout");
    QTest::addColumn<QString>("expectedHwSymFile");

    QTest::newRow("non-existing hw layout") << "non-existing" << SymbolKeyboardFileDefault;
    QTest::newRow("finnish hw layout") << "fi" << SymbolKeyboardFileCommon;
    QTest::newRow("german hw layout") << "de" << SymbolKeyboardFileCommon;
    QTest::newRow("polish hw layout") << "po" << SymbolKeyboardFileCommon;
    QTest::newRow("chinese hw layout") << "cn" << SymbolKeyboardFileChinese;
    QTest::newRow("arabic hw layout") << "ara" << SymbolKeyboardFileCommon;
}

void Ut_LayoutsManager::testHardwareSymLayout()
{
    QFETCH(QString, xkbLayout);
    QFETCH(QString, expectedHwSymFile);

    LoadableKeyboards.clear();
    LoadableKeyboards
            << SymbolKeyboardFileCommon
            << SymbolKeyboardFileChinese;

    std::auto_ptr<LayoutsManager> subject(new LayoutsManager);

    subject->setXkbMap(xkbLayout, "");

    const TestLayoutModel *testLayout = dynamic_cast<const TestLayoutModel *>(
        subject->hardwareLayout(LayoutData::General, M::Landscape));

    QVERIFY(testLayout);
    QCOMPARE(testLayout->modelId, expectedHwSymFile);
}

QTEST_APPLESS_MAIN(Ut_LayoutsManager);

