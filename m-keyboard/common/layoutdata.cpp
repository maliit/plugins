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



#include "layoutdata.h"
#include "mimkeymodel.h"

#include <QtAlgorithms>
#include <QDebug>

const QString LayoutData::mainSection("main");
const QString LayoutData::functionkeySection("functionkey");
const QString LayoutData::symbolsSymSection("symbols Sym");

LayoutSection::Row::~Row()
{
    qDeleteAll(keys);
}

LayoutData::LayoutData()
    : layoutOrientation(MInputMethod::Landscape),
      layoutType(General),
      isUniformFontSize(false)
{
}

LayoutData::~LayoutData()
{
}

LayoutData::LayoutType LayoutData::type() const
{
    return layoutType;
}

MInputMethod::Orientation LayoutData::orientation() const
{
    return layoutOrientation;
}

int LayoutData::numSections() const
{
    return sections.size();
}

LayoutData::SharedLayoutSection LayoutData::section(int index) const
{
    if (index < 0 || index >= sections.size()) {
        return SharedLayoutSection(0);
    } else {
        return sections[index];
    }
}

LayoutData::SharedLayoutSection LayoutData::section(const QString &name) const
{
    SharedLayoutSection layoutSection = sectionMap.value(name);

    if (layoutSection.isNull()) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Could not find requested section in current layout:"
                   << "'" << name << "'\n"
                   << " * Available sections:"
                   << sectionMap;

        layoutSection = SharedLayoutSection(new LayoutSection);
    }

    return layoutSection;
}

LayoutSection::LayoutSection()
    : mMaxColumns(0),
      movable(false),
      sectionType(Sloppy),
      isUniformFontSize(false)
{
}

LayoutSection::LayoutSection(const QString &characters, bool rtl)
    : mMaxColumns(0),
      movable(false),
      sectionName("<dynamic section>"),
      sectionType(Sloppy),
      isUniformFontSize(false)
{
    Row *currentRow = new Row;
    currentRow->heightType = Medium;
    rows.append(currentRow);

    for (int i = 0; i < characters.length(); ++i) {
        const QChar currentCharacter(characters.at(i));

        if (currentCharacter == '\n') {
            currentRow = new Row;
            currentRow->heightType = Medium;
            rows.append(currentRow);

            // Don't append the newline character itself to this row:
            continue;
        }

        MImKeyModel *key(new MImKeyModel(MImKeyModel::NormalStyle, MImKeyModel::Medium, true, rtl));
        currentRow->keys.append(key);

        MImKeyBinding *binding(new MImKeyBinding(characters[i]));
        key->setBinding(*binding, false);
        key->setBinding(*binding, true);

        mMaxColumns = qMax<int>(mMaxColumns, currentRow->keys.count());
    }
}

LayoutSection::~LayoutSection()
{
    qDeleteAll(rows);
}

const QString &LayoutSection::name() const
{
    return sectionName;
}

LayoutSection::Type LayoutSection::type() const
{
    return sectionType;
}

int LayoutSection::maxColumns() const
{
    return mMaxColumns;
}

int LayoutSection::rowCount() const
{
    return rows.count();
}

int LayoutSection::columnsAt(int row) const
{
    return (isInvalidRow(row) ? 0
                              : rows[row]->keys.count());
}

int LayoutSection::keyCount() const
{
    int numKeys = 0;

    for (QList<Row *>::const_iterator rowIter(rows.begin());
         rowIter != rows.end(); ++rowIter) {
         numKeys += (*rowIter)->keys.count();
    }

    return numKeys;
}

QList<int> LayoutSection::spacerIndices(int row) const
{
    return (isInvalidRow(row) ? QList<int>()
                              : rows[row]->spacerIndices);
}

MImKeyModel *LayoutSection::keyModel(int row, int column) const
{
    return (isInvalidCell(row, column) ? 0
                                       : rows[row]->keys[column]);
}

LayoutSection::RowHeightType LayoutSection::rowHeightType(int row) const
{
    return (isInvalidRow(row) ? LayoutSection::Medium
                              : rows[row]->heightType);
}

bool LayoutSection::uniformFontSize() const
{
    return isUniformFontSize;
}

bool LayoutSection::isInvalidRow(int row) const
{
    return ((row < 0) || (row >= rows.count()));
}

bool LayoutSection::isInvalidCell(int row, int column) const
{
    return ((column < 0) || isInvalidRow(row) || (column >= rows[row]->keys.count()));
}
