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



#ifndef HWKBCHARLOOPS_H
#define HWKBCHARLOOPS_H

#include <minputmethodnamespace.h>
#include <QString>
#include <QHash>

/*!
 * \brief HwKbCharacterLoops represents the hardware keyboard character loops.
 *
 * The hardware keyboard character loops store the accented characters' loops for each language.
 * In hardware keyboard state, if a character key is pressed when Sym key is held down, an accented character
 * for that letter is shown. If the same character key is pressed repeatedly holding the Sym key down,
 * the accented character will be changed to next one in a looping fashion (when last accented is reached,
 * the first is shown again).
 * The exact content of the Sym + Character loops are defined for each language.
 *
 */
class HwKbCharacterLoops
{
    Q_DISABLE_COPY(HwKbCharacterLoops)

public:
    /*!
    * \brief Constructor
    */
    HwKbCharacterLoops(const QString &language, const QString &name);

    /*!
    * \brief Destructor
    */
    ~HwKbCharacterLoops();

private:
    /*!
    * \brief Disable default construction
    */
    HwKbCharacterLoops();

    QString language;
    QString name;
    //! Charater loops
    QHash<QChar, QString> loops;

    friend class HwKbCharLoopsManager;
    friend class Ut_HwKbCharLoopsManager;
};
#endif
