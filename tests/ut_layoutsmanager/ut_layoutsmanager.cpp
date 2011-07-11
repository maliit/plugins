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



#include "ut_layoutsmanager.h"
#include <QDebug>
#include <mgconfitem_stub.h>
#include <keyboarddata.h>
#include <layoutdata.h>
#include <layoutsmanager.h>
#include <memory>


namespace
{
    const QString LayoutListSettingName("/meegotouch/inputmethods/onscreen/enabled");
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
                                       MInputMethod::Orientation /* orientation */,
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
    MImSettings layoutListSetting(LayoutListSettingName);
    QStringList layoutList;
    layoutList << "libmeego-keyboard.so" << "fi.xml"
               << "libmeego-keyboard.so" << "ru.xml"
               << "libmeego-keyboard.so" << "ar.xml";
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
    MImSettings layoutListSetting(LayoutListSettingName);
    QStringList layoutList;
    layoutList << "libmeego-keyboard.so" << "fi.xml"
               << "libmeego-keyboard.so" << "ru.xml"
               << "libmeego-keyboard.so" << "ar.xml"
               << "libmeego-keyboard.so" << "en_gb.xml";
    layoutListSetting.set(QVariant(layoutList));
    std::auto_ptr<LayoutsManager> subject(new LayoutsManager);
    foreach (const QString &layout, layoutList) {
        QVERIFY(layout == "libmeego-keyboard.so" || subject->layoutFileList().contains(layout));
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
    MImSettings numberFormatSetting(NumberFormatSettingName);

    // No default when nothing could be loaded
    const TestLayoutModel *layout = dynamic_cast<const TestLayoutModel *>(
                                        subject->layout("fi.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(!layout);

    // Latin is used when it can be loaded and when number format setting has no value
    LoadableKeyboards << NumberKeyboardFileLatin;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // Also when number format setting is Arabic and Arabic cannot be loaded
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // And of course when number format setting is Latin and it can be loaded
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // And when the number format setting is invalid
    numberFormatSetting.set(QVariant("invalid"));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileLatin);

    // We don't want to use Arabic as fallback if Latin cannot be loaded, even if
    // Arabic can be loaded
    LoadableKeyboards.clear();
    LoadableKeyboards << NumberKeyboardFileArabic;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(!layout);
    // Asking with Arabic layout shouldn't make a difference as this
    // stuff works solely based on the number format setting.
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(!layout);

    // If, however, number format is set to Arabic and we can load it,
    // Arabic number layout is used with all layouts
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileArabic);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar.xml", LayoutData::Number, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, NumberKeyboardFileArabic);

    // We can change the setting to Latin on the fly
    LoadableKeyboards << NumberKeyboardFileLatin;
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi,xml", LayoutData::Number, MInputMethod::Landscape));
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
    MImSettings numberFormatSetting(NumberFormatSettingName);
    LoadableKeyboards << NumberKeyboardFileLatin << NumberKeyboardFileArabic;

    // If we can't load any phone number keyboard, we can't have a layout
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    std::auto_ptr<LayoutsManager> subject(new LayoutsManager);
    const TestLayoutModel *layout = dynamic_cast<const TestLayoutModel *>(
                                        subject->layout("fi.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(!layout);

    // our number format is Arabic...
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    LoadableKeyboards << PhoneNumberKeyboardFileLatin;
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);
    // ...or latin
    numberFormatSetting.set(QVariant(LatinNumberFormat));
    subject.reset(new LayoutsManager);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileLatin);

    qDebug() << "Foo";
    // In normal operation with Latin number format we get...
    LoadableKeyboards << PhoneNumberKeyboardFileArabic << PhoneNumberKeyboardFileRussian;
    subject.reset(new LayoutsManager);
    // ...Russian phone numbers for Russian number format
    numberFormatSetting.set(QVariant(RussianNumberFormat));
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileRussian);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileRussian);
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileRussian);

    // But if we switch to Arabic number format, we always get Arabic phone numbers
    numberFormatSetting.set(QVariant(ArabicNumberFormat));
    // That is, for Russian language...
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ru.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileArabic);
    // ...and, say, Finnish...
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("fi.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
    QVERIFY(layout);
    QCOMPARE(layout->modelId, PhoneNumberKeyboardFileArabic);
    // ...and of course for Arabic.
    layout = dynamic_cast<const TestLayoutModel *>(
                 subject->layout("ar.xml", LayoutData::PhoneNumber, MInputMethod::Landscape));
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
        subject->hardwareLayout(LayoutData::General, MInputMethod::Landscape));

    QVERIFY(testLayout);
    QCOMPARE(testLayout->modelId, expectedHwSymFile);
}

QTEST_APPLESS_MAIN(Ut_LayoutsManager);

