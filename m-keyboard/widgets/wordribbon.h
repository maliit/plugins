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


#ifndef WORDRIBBON_H
#define WORDRIBBON_H

#include "style-wrapper.h"
#include "wordribbonstyle.h"
#include "wordribbonhost.h"

class WordRibbonItem;
class MButton;
class MReactionMap;

/*!
 * \brief WordRibbon is a widget which displays a list of candidate items
 */
class WordRibbon: public MStylableWidget
{
    Q_OBJECT
    M_STYLABLE_WIDGET(WordRibbonStyle)

    friend class Ut_WordRibbon;

public:
    /*!
     * \brief The style mode used to display the item
     */
    enum ItemStyleMode { 
        RibbonStyleMode, //! Style when the items is displayed inline
        DialogStyleMode  //! Style when the items is displayed in a dialog
    };

    WordRibbon(ItemStyleMode mode, MWidget* parent = 0);
    virtual ~WordRibbon();

    virtual void resizeEvent(QGraphicsSceneResizeEvent * event);

    virtual void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * \brief Repopulates the item list
     */
    void repopulate(const QStringList &newWordRibbon);

    /*!
     * \brief Clear all items
     */
    void clear();

    /*!
     * \brief Returns whether the list is empty or not
     */
    bool isEmpty() const;

    /*!
     * \brief Highlights an item as specified in the index
     */
    void setHighlightedItem(int index);

    /*!
     * \brief Clears highlighted item
     */
    void clearHighlightedItem();

    /*!
     * \brief Handles navigation key
     */
    void handleNavigationKey(WordRibbonHost::NaviKey key);

    /*!
     * \brief Returns the number of items that the ribbon could fit
     * \param width The allocated width
     * \param candidateList The new list of the candidate that would be fit into
     * \param startPos The starting position in the list
     */
    int capacity(int width, QStringList candidateList, int startPos);
public slots:
    void finalizeOrientationChange();

signals:
    /*!
     * \brief Emitted when an item is clicked
     * \param label The label of the item
     * \param index The index of the item on the list
     */
    void itemClicked(const QString &label, int index);

    /*!
     * \brief Emitted when more candidates is requested
     * by pressing more candidate button
     */
    void moreCandidatesRequested();

private slots:
    void handleItemPressed();
    void handleItemClicked();

private:
    void initializeSubWidgets();
    void reCalculateContentRect(const QSizeF &size);
    void clearItems();

private:
    QStringList cachedStringList;
    // Sub widgets
    QList<WordRibbonItem* > itemList;
    int numVisibleItems;

    QRectF contentRect;

    MButton* moreButton;

    int highlightItemIndex;
    ItemStyleMode mode;
};

#endif // WORDRIBBON_H
