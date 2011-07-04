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

#include "enginedefault.h"
#include "enginemanager.h"

#include <mimenginefactory.h>
#include <mimsettings.h>
#include <QDebug>

namespace
{
    const QString DefaultInputLanguage("en_GB");
    const QString CorrectionSetting("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    const QString CorrectionSettingWithSpace("/meegotouch/inputmethods/virtualkeyboard/correctwithspace");
    const QString InputMethodNextWordPredictionSetting("/meegotouch/inputmethods/virtualkeyboard/nextwordpredictionenabled");
    const bool DefaultCorrectionSettingOption = true;
    const bool DefaultCorrectionSettingAcceptedWithSpaceOption = false;
    const int MaximumErrorCorrectionCandidate = 5;
};

EngineDefault::EngineDefault(MAbstractInputMethodHost &imHost, const QString &engineName)
    : AbstractEngine(imHost, engineName),
      inputMethodHost(imHost),
      mEngine(MImEngineFactory::instance()->createEngineWords(engineName)),
      settingCorrection(new MImSettings(CorrectionSetting)),
      settingCorrectionSpace(new MImSettings(CorrectionSettingWithSpace)),
      settingNextWordPrediction(new MImSettings(InputMethodNextWordPredictionSetting))
{
    if (mEngine) {
        initializeEngine();
    } else {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to load correction default engine";
    }
}

EngineDefault::~EngineDefault()
{
    if (mEngine)
        MImEngineFactory::instance()->deleteEngine(mEngine);
}

MImEngineWordsInterface *EngineDefault::engine() const
{
    return mEngine;
}

QStringList EngineDefault::supportedLanguages()
{
    return QStringList();
}

void EngineDefault::initializeEngine()
{
    if (!mEngine)
        return;
    updateEngineLanguage(DefaultInputLanguage);
    connect(settingCorrection, SIGNAL(valueChanged()),
            this,              SLOT(synchronizeCorrectionSetting()));

    connect(this,                                SIGNAL(correctionSettingChanged()),
            &EngineManager::instance(), SIGNAL(correctionSettingChanged()));

    connect(settingNextWordPrediction, SIGNAL(valueChanged()),
            this,                      SLOT(synchronizeNextWordPrediction()));
}

void EngineDefault::synchronizeCorrectionSetting()
{
    bool correction = settingCorrection->value(DefaultCorrectionSettingOption).toBool();

    if (!correction) {
        mEngine->disableCorrection();
        mEngine->disableCompletion();
    } else {
        mEngine->enableCorrection();
        mEngine->enableCompletion();
    }

    emit correctionSettingChanged();
}

void EngineDefault::synchronizeNextWordPrediction()
{
    if (settingNextWordPrediction->value().toBool())
        mEngine->enablePrediction();
    else
        mEngine->disablePrediction();
}

void EngineDefault::updateEngineLanguage(const QString &language)
{
    if (!mEngine)
        return;
    if (!language.isEmpty()) {
        qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language;

        // TODO: maybe we should check return values here and in case of failure
        // be always in accurate mode, for example
        mEngine->setLanguage(language, MImEngine::LanguagePriorityPrimary);
        synchronizeCorrectionSetting();
        synchronizeNextWordPrediction();
        mEngine->disablePrediction();
        mEngine->setMaximumCandidates(MaximumErrorCorrectionCandidate);
        mEngine->setExactWordPositionInList(MImEngine::ExactInListFirst);
    }
}

