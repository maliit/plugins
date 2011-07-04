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



#include "ut_mkeyboardsettings.h"
#include <QDebug>
#include <mgconfitem_stub.h>
#include <mkeyboardsettings.h>
#include <memory>

namespace
{
    const QString SettingsImCorrection("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const QString SettingsImCorrectionSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
};

// Test init/deinit..........................................................

void Ut_MKeyboardSettings::initTestCase()
{
}

void Ut_MKeyboardSettings::cleanupTestCase()
{
}

void Ut_MKeyboardSettings::init()
{
}

void Ut_MKeyboardSettings::cleanup()
{
}

// Tests.....................................................................

void Ut_MKeyboardSettings::testNoCrash()
{
    std::auto_ptr<MKeyboardSettings> subject(new MKeyboardSettings);
}

void Ut_MKeyboardSettings::testErrorCorrection()
{
    MImSettings errorCorrectionSetting(SettingsImCorrection);

    std::auto_ptr<MKeyboardSettings> subject(new MKeyboardSettings);

    errorCorrectionSetting.set(QVariant(false));
    QCOMPARE(subject->errorCorrection(), errorCorrectionSetting.value().toBool());

    errorCorrectionSetting.set(QVariant(true));
    QCOMPARE(subject->errorCorrection(), errorCorrectionSetting.value().toBool());


    subject->setErrorCorrection(true);
    QCOMPARE(errorCorrectionSetting.value().toBool(), true);
    QCOMPARE(subject->errorCorrection(), errorCorrectionSetting.value().toBool());

    subject->setErrorCorrection(false);
    QCOMPARE(errorCorrectionSetting.value().toBool(), false);
    QCOMPARE(subject->errorCorrection(), errorCorrectionSetting.value().toBool());
}


void Ut_MKeyboardSettings::testCorrectionSpace()
{
    MImSettings correctionSpaceSetting(SettingsImCorrectionSpace);

    std::auto_ptr<MKeyboardSettings> subject(new MKeyboardSettings);

    correctionSpaceSetting.set(QVariant(false));
    QCOMPARE(subject->correctionSpace(), correctionSpaceSetting.value().toBool());

    correctionSpaceSetting.set(QVariant(true));
    QCOMPARE(subject->correctionSpace(), correctionSpaceSetting.value().toBool());


    subject->setCorrectionSpace(true);
    QCOMPARE(correctionSpaceSetting.value().toBool(), true);
    QCOMPARE(subject->correctionSpace(), correctionSpaceSetting.value().toBool());

    subject->setCorrectionSpace(false);
    QCOMPARE(correctionSpaceSetting.value().toBool(), false);
    QCOMPARE(subject->correctionSpace(), correctionSpaceSetting.value().toBool());
}

QTEST_APPLESS_MAIN(Ut_MKeyboardSettings);

