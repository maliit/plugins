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



#ifndef HWKBCHARLOOPSMANAGER_H
#define HWKBCHARLOOPSMANAGER_H

#include <MNamespace>
#include <mimsettings.h>
#include <QObject>
#include <QHash>
#include <QChar>
#include <QString>

class QDomElement;
struct HwKbCharLoopParseParameters;
class HwKbCharacterLoops;

/*!
 * \brief HwKbCharLoopsManager managers the hardware keyboard character loops.
 *
 * The hardware keyboard character loops store the accented characters' loops for each language.
 * HwKbCharLoopsManager loads the loops from
 * /usr/share/meegotouch/virtual-keyboard/layouts/hwkbcharloops.xml
 * and managers them. Using characterLoop() one can get the related accented characters for a character.
 */
class HwKbCharLoopsManager : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(HwKbCharLoopsManager)
public:
    /*!
    * \brief Constructor
    */
    HwKbCharLoopsManager();

    /*!
    * \brief Destructor
    */
    ~HwKbCharLoopsManager();

    /*
     * \brief Returns the character loops for the \a c with current system display language.
     */
    QString characterLoop(const QChar &c) const;

private slots:
    void syncLanguage();

private:
    /*!
    * \brief Load character loops from the specified xml file.
    * The file is loaded from the path /usr/share/meegotouch/virtual-keyboard/layout/.
    * \param fileName XML file name without directory part
    * \return true if loading succeeded, false otherwise
    * \post \a current is set to 0
    */
    bool loadCharLoops(const QString &fileName);

    /*!
    * \brief Implements character loops loading
    * \param filename filename as passed to loadCharLoops
    * \param params holds current state of parser
    */
    bool loadCharLoopsImpl(const QString &fileName, HwKbCharLoopParseParameters  &params);

    //! Parse XML tag for language
    void parseTagLanguage(const QDomElement &element, HwKbCharLoopParseParameters &params);

    //! Parse XML tag for character
    void parseTagCharacter(const QDomElement &element, HwKbCharLoopParseParameters &params);

    //! Type of tag parser methods
    typedef void (HwKbCharLoopsManager::*TagParser)(const QDomElement &, HwKbCharLoopParseParameters &);

    /*!
     * \brief Helper method for parsing children of an element
     * \param element Element whose children are to be parsed
     * \param params Parsing state
     * \param tag1 compulsory, name of expected child tag
     * \param parser1 compulsory, parser for the tag1
     */
    void parseChildren(const QDomElement &element, HwKbCharLoopParseParameters &params,
                       const QString *tag1, TagParser parser1);

    /*!
     * \brief set the active language for character loops.
     * \param language language code which is defined by ISO standards (see ISO 639-1 and ISO 3166-1 alpha-2)
     * and Internet standards (see IETF language tag, RFC 4646 and RFC 4647). e.g. en_gb, en_us, fi, ar.
     * Note: currentCharLoopLanguage will be set to record this language code, even fail to find
     * character loops for \a language. In this case, query characterLoop() will return empty text.
     * \return true if character loops is found for \a language, false if failure or language does not exist.
     */
    bool setCharLoopsLanguage(const QString &language);

    //! All character loops arranged by language.
    //! The map key is language name as read from settings.
    QHash<QString, HwKbCharacterLoops *> charLoops;

    //! Loops for the current language
    const HwKbCharacterLoops *current;

    //! MImSettings for system display language.
    //! The settings are set by control panel applet.
    MImSettings configLanguage;

    //! Current language for the character loop
    QString currentCharLoopLanguage;

    //! Keyboard attribute
    QString keyboardVersion;

    friend class Ut_HwKbCharLoopsManager;
};

#endif
