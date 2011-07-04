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

#include "ut_mkeyboardsettingswidget.h"
#include <mgconfitem_stub.h>
#include <mkeyboardsettingswidget.h>
#include "utils.h"

#include <MApplication>
#include <MSceneManager>
#include <MList>
#include <MButton>
#include <MBasicListItem>
#include <MDialog>
#include <MLabel>

#include <QGraphicsLayout>
#include <QModelIndex>
#include <QSignalSpy>

namespace {
    const QString SettingsImCorrection("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const QString SettingsImCorrectionSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
};

// Stubbing..................................................................
int MSceneManager::execDialog(MDialog *dialog)
{   
    // avoid MSceneManager create private even loop
    appearSceneWindowNow(dialog);
    return 0;
}

void Ut_MKeyboardSettingsWidget::initTestCase()
{
    static char *argv[2] = { (char *) "ut_mkeyboardsettingswidget",
                             (char *) "-software" };
    static int argc = 2;
    disableQtPlugins();
    app = new MApplication(argc, argv);
    settingsObject = new MKeyboardSettings();
}

void Ut_MKeyboardSettingsWidget::cleanupTestCase()
{
    delete settingsObject;
    delete app;
}

void Ut_MKeyboardSettingsWidget::init()
{
    subject = new MKeyboardSettingsWidget(settingsObject);
    QVERIFY(subject->errorCorrectionSwitch);
    QVERIFY(subject->correctionSpaceSwitch);
}

void Ut_MKeyboardSettingsWidget::cleanup()
{
    delete subject;
    subject = 0;
}

void Ut_MKeyboardSettingsWidget::testKeyboardCorrectionSettings()
{
    MImSettings errorCorrectionSetting(SettingsImCorrection);
    MImSettings correctionSpaceSetting(SettingsImCorrectionSpace);

    // Check the error correction setting alone
    settingsObject->setErrorCorrection(true);
    QCOMPARE(subject->errorCorrectionSwitch->isChecked(), true);
    settingsObject->setErrorCorrection(false);
    QCOMPARE(subject->errorCorrectionSwitch->isChecked(), false);

    // Check the "select with space" setting alone
    // Note: Error correction must be enabled
    settingsObject->setErrorCorrection(true);
    settingsObject->setCorrectionSpace(true);
    QCOMPARE(subject->correctionSpaceSwitch->isChecked(), true);
    settingsObject->setCorrectionSpace(false);
    QCOMPARE(subject->correctionSpaceSwitch->isChecked(), false);
    // Check if switching the error correction off disables the "select with space"
    settingsObject->setErrorCorrection(true);
    settingsObject->setCorrectionSpace(true);
    QCOMPARE(subject->correctionSpaceSwitch->isChecked(), true);
    settingsObject->setErrorCorrection(false);
    QCOMPARE(subject->correctionSpaceSwitch->isEnabled(), false);
    QCOMPARE(subject->correctionSpaceSwitch->isChecked(), false);

    // Check the behaviour of the error correction toggle button alone
    QSignalSpy spyErrorCorrection(subject->errorCorrectionSwitch, SIGNAL(toggled(bool)));
    subject->setErrorCorrectionState(true);
    QCOMPARE(spyErrorCorrection.count(), 1);
    QCOMPARE(spyErrorCorrection.takeFirst().at(0).toBool(), true);
    QCOMPARE(errorCorrectionSetting.value().toBool(), true);

    spyErrorCorrection.clear();
    subject->setErrorCorrectionState(false);
    QCOMPARE(spyErrorCorrection.count(), 1);
    QCOMPARE(spyErrorCorrection.takeFirst().at(0).toBool(), false);
    QCOMPARE(errorCorrectionSetting.value().toBool(), false);

    // Check the behaviour of the error correction toggle button alone
    QSignalSpy spyCorrectionSpace(subject->correctionSpaceSwitch, SIGNAL(toggled(bool)));
    subject->setCorrectionSpaceState(true);
    QCOMPARE(spyCorrectionSpace.count(), 1);
    QCOMPARE(spyCorrectionSpace.takeFirst().at(0).toBool(), true);
    QCOMPARE(correctionSpaceSetting.value().toBool(), true);

    spyCorrectionSpace.clear();
    subject->setCorrectionSpaceState(false);
    QCOMPARE(spyCorrectionSpace.count(), 1);
    QCOMPARE(spyCorrectionSpace.takeFirst().at(0).toBool(), false);
    QCOMPARE(correctionSpaceSetting.value().toBool(), false);

    // Check if switching the error correction off disables the "select with space"
    subject->setErrorCorrectionState(true);
    subject->setCorrectionSpaceState(true);
    spyErrorCorrection.clear();
    spyCorrectionSpace.clear();
    // Switch off the error correction
    subject->setErrorCorrectionState(false);
    QCOMPARE(spyErrorCorrection.count(), 1);
    QCOMPARE(spyErrorCorrection.takeFirst().at(0).toBool(), false);
    QCOMPARE(errorCorrectionSetting.value().toBool(), false);
    // Check if the "select with space" setting has been disabled
    QCOMPARE(spyCorrectionSpace.count(), 1);
    QCOMPARE(spyCorrectionSpace.takeFirst().at(0).toBool(), false);
    QCOMPARE(correctionSpaceSetting.value().toBool(), false);
}

QTEST_APPLESS_MAIN(Ut_MKeyboardSettingsWidget);
