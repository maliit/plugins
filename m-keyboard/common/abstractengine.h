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

#ifndef ABSTRACTENGINE_H
#define ABSTRACTENGINE_H

#include <QObject>
#include <QString>
#include <QStringList>

class MAbstractInputMethodHost;
class MImEngineWordsInterface;
class MImSettings;

/*!
  \class AbstractEngine

  \brief The AbstractEngine class creates, initializes and keeps the MImEngineWordsInterface
   for the default input method engine.
*/
class AbstractEngine : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractEngine)

public:
    AbstractEngine(MAbstractInputMethodHost &, const QString &) {};

    /*!
     *\brief Returns the supported language list.
     */
    static QStringList supportedLanguages() {return QStringList();};
    
    /*!
     * \brief Returns the pointer to error correction/prediction engine.
     */
    virtual MImEngineWordsInterface *engine() const = 0;

public slots:
    //! This slot is called when virtual keyboard language is changed.
    virtual void updateEngineLanguage(const QString &language) = 0;

signals:
    //! This signal is emitted when the correction setting is changed
    void correctionSettingChanged();

private:
    friend class Ut_AbstractEngine;
};

#endif
