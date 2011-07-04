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



#ifndef LAYOUTDATA_H
#define LAYOUTDATA_H

#include <minputmethodnamespace.h>
#include <QHash>
#include <QList>
#include <QSharedPointer>
#include <QString>

class MImKeyModel;

/*!
 * \brief LayoutSection represents a named area of keys in a layout
 */
class LayoutSection
{
    Q_DISABLE_COPY(LayoutSection)

public:
    enum Type {
        //! The section uses sloppy mode by default. This is the default value.
        Sloppy = 0,
        //! The section uses discrete layout
        NonSloppy
    };

    //! Row height type
    enum RowHeightType {
        Small,
        Medium,
        Large,
        XLarge,
        XxLarge,
    };

    ~LayoutSection();
    LayoutSection();

    //! \brief Construct a layout section with standard insert-action buttons with labels that
    //! correspond to characters in the \a characters string.
    //!
    //! The MImKeyModels of the section will have just one binding for all levels.
    //! \param characters labels for the keys
    //! \param rtl passed directly to all keys of the section
    explicit LayoutSection(const QString &characters, bool rtl = false);

    //! \return section name
    const QString &name() const;

    //! \return section type
    Type type() const;

    /*!
     * \brief Get the maximum columns in this section
     * \return the maximum columns in this section
     */
    int maxColumns() const;

    /*!
     * \brief Get the maximum number of rows in this section
     * \return the maximum number of rows in this section
     */
    int rowCount() const;

    /*!
     * \brief Get the number of columns in the specified section and row
     * \param row number
     * \return the number of columns in the specified section and row
     */
    int columnsAt(int row) const;

    //! \brief Number of keys in the section
    int keyCount() const;

    //! \brief Get indices with layout spacers for given row
    //! \param row number
    QList<int> spacerIndices(int row) const;

    /*!
     * \brief Get key at specified row, column and section
     * \param row
     * \param column
     * \return key
     */
    MImKeyModel *keyModel(int row, int column) const;

    //! \brief Return the row height class for given row.
    RowHeightType rowHeightType(int row) const;

    //! \brief Return true if keys are allowed to use same font size.
    bool uniformFontSize() const;

private:
    bool isInvalidRow(int row) const;
    bool isInvalidCell(int row, int column) const;

    struct Row {
        Row()
            : heightType (LayoutSection::Medium)
        {}

        ~Row();

        QList<MImKeyModel *> keys;
        LayoutSection::RowHeightType heightType;

        // Index of a spacer refers to right side of a key;
        // -1 means spacer before first key.
        QList<int> spacerIndices;
    };

    int mMaxColumns;
    bool movable;
    QString sectionName;
    // TODO: remove? we only have one section type now
    Type sectionType;
    QList<Row *> rows;
    bool isUniformFontSize;

    friend class KeyboardData;
    friend class LayoutModel;
    friend class ParseParameters;
};

/*!
 * \class LayoutData
 * \brief LayoutData represents a keyboard layout of certain type and orientation
 */
class LayoutData
{
    Q_DISABLE_COPY(LayoutData)

public:
    typedef QSharedPointer<LayoutSection> SharedLayoutSection;

    //! Type of layout
    enum LayoutType {
        General = 0,
        Url,
        Email,
        Number,
        PhoneNumber,
        Common,
        NumLayoutTypes
    };

    static const QString mainSection;
    static const QString functionkeySection;
    static const QString symbolsSymSection;

    //! \brief Constructor
    LayoutData();

    //! \brief Destructor
    virtual ~LayoutData();

    //! \return the number of sections in this layout
    int numSections() const;

    //! \return a section by section index
    SharedLayoutSection section(int index) const;

    /*! \return a section by section name.  if there are several
     * identically named sections, it is unspecified which one is
     * returned
     */
    SharedLayoutSection section(const QString &name) const;

    //! \return layout type
    LayoutType type() const;

    //! \return layout orientation
    MInputMethod::Orientation orientation() const;

protected:
    MInputMethod::Orientation layoutOrientation;
    LayoutType layoutType;
    //! this is the top level data structure of a layout
    QList<SharedLayoutSection> sections;
    //! we keep sections also in a hash table for fast name based lookup
    QHash<QString, SharedLayoutSection> sectionMap;
    bool isUniformFontSize;

    friend class KeyboardData;
};

#endif //LAYOUTMODEL_H
