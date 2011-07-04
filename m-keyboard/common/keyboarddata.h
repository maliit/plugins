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



#ifndef KEYBOARDDATA_H
#define KEYBOARDDATA_H

#include "layoutdata.h"
#include "mimkeymodel.h"

#include <QHash>
#include <QList>
#include <QString>

class QDomElement;
struct ParseParameters;

/*!
 * \class KeyboardData
 * \brief KeyboardData corresponds to a keyboard defined in a layout XML file
 * Therefore it is a collection of LayoutModels for a language.
 */
class KeyboardData
{
    Q_DISABLE_COPY(KeyboardData)

public:
    /*!
    * \brief Constructor
    * private class
    */
    explicit KeyboardData();

    /*!
    * \brief Destructor
    */
    ~KeyboardData();

    /*!
    * \brief Load keyboard and its layouts from the specified xml file.
    * The file is loaded from the path /usr/share/meegotouch/virtual-keyboard/layout/.
    * This method can be called multiple times to reload another keyboard file,
    * whether previous call failed or not.
    * \param fileName XML file name without directory part, or an absolute path
    * \return true if loading succeeded, false otherwise
    */
    bool loadNokiaKeyboard(const QString &fileName);

    //! \return keyboard language
    QString language() const;

    //! \return keyoard title
    QString title() const;

    //! \return layout file name.
    QString layoutFile() const;

    //! \return whether autocaps is enabled.
    bool autoCapsEnabled() const;

    /*!
     * \brief Get layout model by type and orientation
     * \param type type
     * \param orientation orientation
     * \param portraitFallback if true and if no landscape layout is available, return
     * portrait layout instead (if available)
     */
    const LayoutData *layout(LayoutData::LayoutType type, MInputMethod::Orientation orientation,
                             bool portraitFallback = true) const;

private:
    /*!
    * \brief Implements keyboard loading
    * \param filename filename as passed to loadNokiaKeyboard
    * \param params holds current state of parser
    * \param importedLayout indicates if this layout file is imported from within another.
    */
    bool loadNokiaKeyboardImpl(const QString &fileName, ParseParameters &params,
                               bool importedLayout = false);

    bool findLayoutFile(QString &foundAbsoluteFilename) const;

    //! \brief Get layout model by type and orientation
    LayoutData *layoutPrivate(LayoutData::LayoutType type, MInputMethod::Orientation orientation,
                              bool portraitFallback = true) const;

    /*!
    * \brief Translate alignmentString to Qt::Alignment
    */
    static Qt::Alignment alignment(const QString &alignmentString, bool vertical);

    /*!
    * \brief Translate orientationString to MInputMethod::Orientation
    */
    static MInputMethod::Orientation orientation(const QString &orientationString);

    //! \brief Translate key action string into a MImKeyBinding::KeyAction
    static MImKeyBinding::KeyAction keyActionFromString(const QString &typeStr);

    //! Parse XML tag for layout
    void parseTagLayout(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for binding
    void parseTagBinding(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for section
    void parseTagSection(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for row
    void parseTagRow(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for key
    void parseTagKey(const QDomElement &element, ParseParameters &params);

    //! Parse XML tag for layout spacers
    void parseTagSpacer(const QDomElement &element, ParseParameters &params);

    //! Type of tag parser methods
    typedef void (KeyboardData::*TagParser)(const QDomElement &, ParseParameters &);

    /*!
     * \brief Helper method for parsing children of an element
     * \param element Element whose children are to be parsed
     * \param params Parsing state
     * \param tag1 compulsory, name of expected child tag
     * \param parser1 compulsory, parser for the tag1
     * \param tag2 optional, name of another allowed child tag
     * \param parser2 compulsory with tag2, parser for the tag2
     */
    void parseChildren(const QDomElement &element, ParseParameters &params,
                       const char * const tag1, TagParser parser1, const char * const tag2 = NULL,
                       TagParser parser2 = NULL);

    //! Parse XML tag for import
    void parseTagImport(const QDomElement &element, ParseParameters &params);

    static MImKeyModel::StyleType toStyleType(const QString &attributeValue);

    static MImKeyModel::WidthType toWidthType(const QString &attributeValue);

    static LayoutSection::RowHeightType toHeightType(const QString &attributeValue);

    static bool toBoolean(const QString &attributeValue);

protected:
    LayoutData *currentLayout;

    //! keyboard attribute
    QString keyboardVersion;
    QString keyboardTitle;
    QString keyboardLanguage;
    QString keyboardCatalog;
    bool keyboardAutoCapsEnabled;

    QString layoutFileName;
    QList<LayoutData *> layouts;
    QHash<QString, LayoutData::LayoutType> layoutTypeMap;
};

#endif
