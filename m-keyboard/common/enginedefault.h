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

#ifndef ENGINEDEFAULT_H
#define ENGINEDEFAULT_H

#include "abstractengine.h"

class MAbstractInputMethodHost;
class MImEngineWordsInterface;
class MImSettings;

/*!
  \class EngineDefault

  \brief The EngineDefault class creates, initializes and keeps the MImEngineWordsInterface
   for the default input method engine.
*/
class EngineDefault : public AbstractEngine
{
    Q_OBJECT
    Q_DISABLE_COPY(EngineDefault)

public:
    EngineDefault(MAbstractInputMethodHost &imHost, const QString &engineName);
    virtual ~EngineDefault();

    /*!
     *\brief Returns the supported language list.
     */
    static QStringList supportedLanguages();

    //! reimp
    virtual MImEngineWordsInterface *engine() const;
    //! reimp_end

public slots:
    //! This slot is called when virtual keyboard language is changed.
    virtual void updateEngineLanguage(const QString &language);

private slots:
    //! Synchronize correction setting
    void synchronizeCorrectionSetting();

    //! Synchronize next word prediction setting
    void synchronizeNextWordPrediction();

protected:
    void initializeEngine();

private:
    MAbstractInputMethodHost &inputMethodHost;

    MImEngineWordsInterface *mEngine;
    //! default input method error correction setting
    MImSettings *settingCorrection;
    MImSettings *settingCorrectionSpace;
    MImSettings *settingNextWordPrediction;
    MImSettings *settingEngineName;

    QString engineLanguage;

    friend class Ut_EngineDefault;
};

#endif
