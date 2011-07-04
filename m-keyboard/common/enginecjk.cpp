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

#include "enginecjk.h"

#include <mimenginefactory.h>
#include <mimsettings.h>

#include <QDebug>

namespace
{
    const QString DefaultInputLanguage("zh@pinyin");
    const QString InputMethodFuzzySetting("/meegotouch/inputmethods/virtualkeyboard/fuzzyselected");
    const QString InputMethodWordPredictionSetting("/meegotouch/inputmethods/virtualkeyboard/wordpredictionenabled");
};

EngineCJK::EngineCJK(MAbstractInputMethodHost &imHost, const QString &engineName)
    : AbstractEngine(imHost, engineName),
      inputMethodHost(imHost),
      mEngine(MImEngineFactory::instance()->createEngineWords(engineName)),
      settingFuzzy(new MImSettings(InputMethodFuzzySetting)),
      settingWordPrediction(new MImSettings(InputMethodWordPredictionSetting))
{
    if (mEngine) {
        initializeEngine();
    } else {
        qWarning() << __PRETTY_FUNCTION__ << "Failed to load input engine for "
            << engineName;
    }
}

EngineCJK::~EngineCJK()
{
    if (mEngine)
        MImEngineFactory::instance()->deleteEngine(mEngine);
}

MImEngineWordsInterface *EngineCJK::engine() const
{
    return mEngine;
}

QStringList EngineCJK::supportedLanguages()
{
    QStringList languages;
    languages << "zh" << "ja" << "ko";
    return languages;
}

bool EngineCJK::correctionAcceptedWithSpaceEnabled() const
{
    // not supported setting
    return false;
}

void EngineCJK::initializeEngine()
{
    if (!mEngine)
        return;
    updateEngineLanguage(DefaultInputLanguage);

    connect(settingFuzzy, SIGNAL(valueChanged()),
            this,         SLOT(synchronizeFuzzy()));

    connect(settingWordPrediction, SIGNAL(valueChanged()),
            this,         SLOT(synchronizeWordPrediction()));
}

void EngineCJK::updateEngineLanguage(const QString &language)
{
    if (!mEngine)
        return;
    if (!language.isEmpty()) {
        qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language;

        const QString variant = language.contains("@") ? language.split('@').last()
                                : language;
        // TODO: maybe we should check return values here and in case of failure
        // be always in accurate mode, for example
        mEngine->setLanguage(variant, MImEngine::LanguagePriorityPrimary);
        synchronizeFuzzy();
        synchronizeWordPrediction();
    }
}

void EngineCJK::synchronizeFuzzy()
{
    if (settingFuzzy->value().toBool())
        mEngine->enableCorrection();
    else
        mEngine->disableCorrection();
}

void EngineCJK::synchronizeWordPrediction()
{
    if (settingWordPrediction->value().toBool())
        mEngine->enablePrediction();
    else
        mEngine->disablePrediction();
}
