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


#include "wordribbon.h"
#include "wordribbonstyle.h"
#include "wordribbonitem.h"
#include "wordribbonhost.h"
#include "mimreactionmap.h"
#include "mplainwindow.h"
#include "reactionmapwrapper.h"
#include "mkeyboardhost.h"
#include "mimrootwidget.h"

#include <QtGui>
#include <QtCore>

#ifdef HAVE_MEEGOTOUCH
#include <MButton>
#include <MSceneManager>
#else
class MButton : public QGraphicsWidget
{
public:
    enum ViewType {
        iconType
    };

    explicit MButton(QObject *) : QGraphicsWidget() {}
    void setViewType(ViewType) {}
};
#endif

namespace {
    static const int CacheSize = 10;
}

WordRibbon::WordRibbon(ItemStyleMode mode, MWidget* parent):
        MStylableWidget(parent),
        numVisibleItems(0),
        moreButton(0),
        highlightItemIndex(-1),
        mode(mode)
{
    setObjectName("WordRibbonObj");
    initializeSubWidgets();

    if (mode == DialogStyleMode) {
        style().setModeDialogmode();
    }
}

WordRibbon::~WordRibbon()
{
    for (int i = 0;i < itemList.count(); i ++) {
        WordRibbonItem *item = itemList.at(i);
        delete item;
        itemList.removeAt(i);
    }

    if (moreButton) {
        delete moreButton;
    }
    cachedStringList.clear();
}

void WordRibbon::repopulate(const QStringList &candidateList)
{
    if (candidateList.count() == 0) {
        clear();
        return ;
    }

    clearItems();
    clearHighlightedItem();
    if (cachedStringList != candidateList)
        cachedStringList = candidateList;

    int totalSpace = contentRect.width();
    int spaceBetweenItems = style()->spaceBetween();

    WordRibbonItem* item = 0;
    QString itemText;
    QPoint itemPoint = contentRect.topLeft().toPoint();
    int index = 0;
    QList<int> itemRightHandSpaceList;

    while (index < candidateList.count() && totalSpace > 0) {
        itemText = candidateList.at(index);

        if (index > itemList.count() - 1) {
            int count = 0;

            // Create approximate number of new candidate items.
            while (true) {
                item = new WordRibbonItem(mode, this);
                //Set the object name for TDriver test.
                item->setObjectName(QString("WordRibbonItemObj-%1").arg(itemList.count()));
                item->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
                connect(item, SIGNAL(mousePressed()), this, SLOT(handleItemPressed()));
                connect(item, SIGNAL(mouseReleased()), this, SLOT(handleItemClicked()));
                item->setVisible(false);
                itemList.append(item);
                count += item->sizeHint(Qt::PreferredSize).width();
                if (count > totalSpace) {
                    break;
                }
            }
        }

        item = itemList.at(index);
        item->setText(itemText);
        item->setPositionIndex(index);
        int itemWidth = item->sizeHint(Qt::PreferredSize).width();
        totalSpace -= itemWidth;

        if (totalSpace >= 0) {
            item->setVisible(true);
            item->setPos(itemPoint);
            itemRightHandSpaceList.append(totalSpace);
            itemPoint.setX(itemPoint.x() + itemWidth + spaceBetweenItems);
            totalSpace -= spaceBetweenItems;
            ++numVisibleItems;
        } else {
            break;
        }
        ++index;
    }

    if (mode == RibbonStyleMode) {
        if (numVisibleItems < candidateList.count()) {
            int dialogButtonWidth = moreButton->size().width();
            int lastItemIndex = numVisibleItems - 1;

            if (lastItemIndex >= itemRightHandSpaceList.size()) {
                qWarning() << __PRETTY_FUNCTION__
                           << "Got more candidates than what's available in itemRightHandSpaceList";
            } else {
                while(lastItemIndex >=0
                      && itemRightHandSpaceList.at(lastItemIndex) < dialogButtonWidth) {
                    WordRibbonItem *item = itemList.at(lastItemIndex);
                    item->setVisible(false);
                    --lastItemIndex;
                    --numVisibleItems;
                }
            }

            moreButton->setX(contentRect.x() + contentRect.width() 
                             - moreButton->size().width());
            moreButton->setY(contentRect.y());
            moreButton->show();
        } else {
            moreButton->hide();
        }
    }

    itemRightHandSpaceList.clear();
    setHighlightedItem(0);

    update();
}

void WordRibbon::clear()
{
    cachedStringList.clear();
    clearItems();
    clearHighlightedItem();
}

bool WordRibbon::isEmpty() const
{
    return itemList.count() > 0;
}

void WordRibbon::setHighlightedItem(int index)
{
    if (mode == DialogStyleMode)
        return;

    if (0 <= index && index < itemList.count()) {
        highlightItemIndex = index;
        WordRibbonItem *item = itemList.at(highlightItemIndex);
        item->highlight();
    }
}

void WordRibbon::clearHighlightedItem()
{
    if (mode == DialogStyleMode)
        return;

    if (0 <= highlightItemIndex && highlightItemIndex < itemList.count()) {
        WordRibbonItem *item = itemList.at(highlightItemIndex);
        item->clearHighlight();
        highlightItemIndex = -1;
    }
}

void WordRibbon::handleNavigationKey(WordRibbonHost::NaviKey key)
{
    if (mode == DialogStyleMode)
        return;

    switch(key) {
    case WordRibbonHost::NaviKeyOk:
        {
            WordRibbonItem *item = itemList.at(highlightItemIndex);
            QString emitStr = item->text();
            int selectedIndex = item->positionIndex();

            if (!emitStr.isEmpty() && selectedIndex >= 0) {
                emit itemClicked(emitStr, selectedIndex);
            }
        }
        break;
    //Not handled
    case WordRibbonHost::NaviKeyLeft:
    case WordRibbonHost::NaviKeyRight:
    case WordRibbonHost::NaviKeyUp:
    case WordRibbonHost::NaviKeyDown:
    default:
        break;
    }
}

void WordRibbon::handleItemPressed()
{
    clearHighlightedItem();

    //Record new highlighted candidate item.
    QObject* object = sender();
    WordRibbonItem* sendItem = qobject_cast<WordRibbonItem* > (object);

    if (sendItem != 0) {
        QString emitObjectName = sendItem->objectName();
        QStringList splitName = emitObjectName.split("-");
        highlightItemIndex = splitName.last().toInt();
    }
}

void WordRibbon::handleItemClicked()
{
    QObject* object = sender();
    WordRibbonItem* sendItem = qobject_cast<WordRibbonItem* > (object);

    if (sendItem != 0) {
        QString emitStr = sendItem->text();
        int selectedIndex = sendItem->positionIndex();

        if(!emitStr.isEmpty() && selectedIndex >= 0) {
            emit itemClicked(emitStr, selectedIndex);
        }
    }
}

void WordRibbon::initializeSubWidgets()
{
    for (int i = 0; i < CacheSize; ++i) {
        WordRibbonItem *item = new WordRibbonItem(mode, this);
        //Set the object name for TDriver test.
        item->setObjectName(QString("WordRibbonItemObj-%1").arg(i));
        item->setPositionIndex(i);
        connect(item, SIGNAL(mousePressed()), this, SLOT(handleItemPressed()));
        connect(item, SIGNAL(mouseReleased()), this, SLOT(handleItemClicked()));
        item->setVisible(false);
        itemList.append(item);
    }

    if (mode == RibbonStyleMode) {
        moreButton = new MButton(this);
        moreButton->setObjectName("MoreButton");
        moreButton->setViewType(MButton::iconType);
        moreButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        connect(moreButton, SIGNAL(clicked()), this, SIGNAL(moreCandidatesRequested()));
    }
}

void WordRibbon::resizeEvent(QGraphicsSceneResizeEvent * event)
{
    // Need to reset style mode when mode is DialogStyleMode.
    if (mode == DialogStyleMode) {
        style().setModeDialogmode();
    }
    reCalculateContentRect(event->newSize());
    repopulate(cachedStringList);
}

void WordRibbon::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    Q_UNUSED(view);
    return;
#else
    if (!isVisible())
        return;

    reactionMap->setTransform(this, view);
    reactionMap->setInactiveDrawingValue();
    reactionMap->fillRectangle(boundingRect());

    reactionMap->setDrawingValue(MImReactionMap::Press, MImReactionMap::Release);

    QRectF rect;
    for (int i = 0; i < numVisibleItems; ++i) {
        rect = itemList.at(i)->boundingRect();
        WordRibbonItem *item = itemList.at(i);
        rect.moveTopLeft(item->pos());
        reactionMap->fillRectangle(rect);
    }

    if (moreButton && moreButton->isVisible()) {
        rect = moreButton->boundingRect();
        rect.moveTopLeft(QPointF(moreButton->pos()));
        reactionMap->fillRectangle(rect);
    }
#endif
}

void WordRibbon::finalizeOrientationChange()
{
    // Force the style orientation mode to be right.
#ifdef HAVE_MEEGOTOUCH
    const M::Orientation orientation = MPlainWindow::instance()->sceneManager()->orientation();
    if (orientation == M::Landscape)
#else
    const MInputMethod::Orientation orientation = MKeyboardHost::instance()->rootWidget()->orientation();
    if (orientation == MInputMethod::Landscape)
#endif
        style().setModeLandscape();
    else
        style().setModePortrait();

    resize(style()->preferredSize());
}

void WordRibbon::reCalculateContentRect(const QSizeF &size)
{
    QSizeF contentSize = size - QSizeF(style()->marginLeft() + style()->marginRight(),
                                       style()->marginTop() + style()->marginBottom());

    contentSize -= QSizeF(style()->paddingLeft() + style()->paddingRight(),
                          style()->paddingTop() + style()->paddingBottom());

    contentRect = QRectF(QPointF(style()->marginLeft() + style()->paddingLeft(),
                                 style()->marginTop() + style()->paddingTop()),
                         contentSize);
}

void WordRibbon::clearItems()
{
    for (int i = 0; i < itemList.count(); ++i) {
        WordRibbonItem *item = itemList.at(i);
        item->setVisible(false);
    }

    if (mode == RibbonStyleMode)
        moreButton->setVisible(false);

    numVisibleItems = 0;
}

int WordRibbon::capacity(int width, QStringList candidateList, int startPos)
{
    if (width <= 0) {
        qDebug() <<"!Warning: WordRibbon::capacity() arguments error!";
        return -1;
    }

    int c = 0;
    int index = startPos;

    WordRibbonItem item(WordRibbon::DialogStyleMode, this);

    while (true) {
        // Retrieve each candidate item text.
        QString itemText;
        if (0 <= index && index < candidateList.count()) {
            itemText = candidateList.at(index);
        } else {
            qDebug() << "!Warning: WordRibbon::capacity() Index out of range!";
            return c;
        }
        item.setText(itemText);
        width -= (item.sizeHint(Qt::PreferredSize)).width();

        if (width < 0)
            break;

        ++c;
        ++index;
    }
    return c;
}


