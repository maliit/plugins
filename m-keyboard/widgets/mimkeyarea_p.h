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

#ifndef MIMKEYAREA_P_H
#define MIMKEYAREA_P_H

#include "mimabstractkeyarea.h"
#include "mimabstractkeyarea_p.h"
#include "mimkey.h"
#include "mimfontpool.h"
#include "mimkeystyle.h"

#include <QSize>
#include <QFontMetrics>
#include <QSharedPointer>

class MImKeyArea;
class MImKeyModel;
class MImKey;

class QPainter;

//! \internal
class MImKeyAreaPrivate : public MImAbstractKeyAreaPrivate
{
public:
    Q_DECLARE_PUBLIC(MImKeyArea)

    //! \brief Constructor
    //! \param newSection Section that is shown by this key area
    //! \param owner Pointer to key area which owns this object
    MImKeyAreaPrivate(const LayoutData::SharedLayoutSection &newSection,
                      MImKeyArea *owner);

    //! \brief Destructor
    virtual ~MImKeyAreaPrivate();

    //! \brief Creates buttons for key data models.
    void loadKeys();

    //! \brief Returns the new height of the key area.
    qreal computeWidgetHeight() const;

    //! \brief Return preferred key height for a row.
    //! \param row the index of the queried row
    qreal preferredKeyHeight(int row) const;

    //! \brief Compute the maximum width in this widget, in normalized units.
    qreal computeMaxNormalizedWidth() const;

    //! \brief Returns normalized width for a particular MImKeyModel.
    //! \param model the key model to be queried
    qreal normalizedKeyWidth(const MImKeyModel *model) const;

    //! \brief Draws button rects/bounding rects, for debugging purposes.
    //! \param painter the painter to be used
    //! \param key key for which rects/bounding rects shall be drawn
    //! \param drawBoundingRects whether to draw bounding rects
    //! \param drawRects whether to draw rects
    void drawDebugRects(QPainter *painter,
                        const MImAbstractKey *key,
                        bool drawBoundingRects,
                        bool drawRects) const;

    //! \brief Draws reactive areas of buttons, for debugging purposes.
    //! \param painter the painter to be used
    void drawDebugReactiveAreas(QPainter *painter);

    //! \brief Clears all information about key identifiers
    void clearKeyIds();

    //! \brief Register new key having identifier
    //! \param key Pointer to key which id should be registered
    void registerKeyId(MImKey *key);

    //! \brief Helper struct to store a row of keys.
    struct KeyRow {
        QList<MImKey*> keys; //!< keys in a row
        QVector<QPair<qreal, qreal> > keyOffsets; //!< cached offsets for faster key lookups
    };

    typedef QVector<KeyRow> KeyRowList;
    typedef KeyRowList::iterator RowIterator;
    typedef KeyRowList::const_iterator ConstRowIterator;

    MImKeyArea * const q_ptr;

    KeyRowList rowList; //!< stores all rows of this key area
    qreal cachedWidgetHeight; //!< cached widget height
    qreal mMaxNormalizedWidth; //!< maximal normalized width, for all rows
    QVector<QPair<qreal, qreal> > rowOffsets; //!< cached offsets for faster key lookups
    MImKey *shiftKey; //!< stores shift key, if available in this key area
    bool equalWidthKeys; //!< whether to assume equal width for all keys
    int WidthCorrection; //!< width correction for Arabic layouts
    QSharedPointer<MImKey::StylingCache> stylingCache; //!< Cached information about current styling
    QList<MImKey *> idToKey; //!< Contains information about keys which have identifiers
    MImKey *toggleKey; //!< stores toggle key, if available in this key area
    MImKey *composeKey; //!< stores compose key, if available in this key area
    MImFontPool fontPool; //!< decides which keys should share the same font object
    bool lockVerticalMovement; //!< whether vertical movement is considered
    QScopedPointer<MImKeyStyle> keyStyle; //!< Key related styling, shared by all keys in same key area

};
//! \internal_end

#endif

