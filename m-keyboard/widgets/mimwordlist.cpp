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


#include "regiontracker.h"
#include "mimwordlist.h"
#include "mimwordlistitem.h"
#include "reactionmapwrapper.h"
#include "mwidget-wrapper.h"

#include <QGraphicsLinearLayout>
#include <QDebug>
#include <QString>

#ifdef HAVE_MEEGOTOUCH
#include <mwidgetcreator.h>
#include <MSeparator>
M_REGISTER_WIDGET_NO_CREATE(MImWordList)
#endif

namespace
{
    const char * const WordListObjectName = "CorrectionWordList";

#ifdef HAVE_MEEGOTOUCH
    void addItem(QGraphicsLinearLayout *layout, MWidgetController *item, int position)
    {
        item->setSelected(false);
        layout->insertItem(position, item);
        item->setVisible(true);
        item->setEnabled(true);
        item->setLayoutPosition(M::VerticalCenterPosition);
    }

    void removeItem(QGraphicsLinearLayout *layout, MWidgetController *item)
    {
        layout->removeItem(item);
        item->setVisible(false);
    }
#endif // HAVE_MEEGOTOUCH
}

MImWordList::MImWordList()
#ifdef HAVE_MEEGOTOUCH
    : MDialog()
#else
    : QGraphicsWidget()
#endif
{
    RegionTracker::instance().addRegion(*this);
    // for MATTI
    setObjectName(WordListObjectName);

#ifdef HAVE_MEEGOTOUCH
    MWidget *contentWidget = new MWidget(this);
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    contentWidget->setLayout(mainLayout);

    addToDictionaryItem = new MImWordListItem(contentWidget);
    //% "Add to dictionary"
    addToDictionaryItem->setTitle(qtTrId("qtn_vkb_dictionary_add"));
    addToDictionaryItem->setVisible(false);
    connect(addToDictionaryItem, SIGNAL(clicked()), this, SLOT(select()));
    mainLayout->addItem(addToDictionaryItem);

    addToDictionarySeparator = new MSeparator(contentWidget);
    addToDictionarySeparator->setStyleName("CommonItemDivider");
    addToDictionarySeparator->setVisible(false);
    mainLayout->addItem(addToDictionarySeparator);

    for (int i = 0; i < MaxCandidateCount; i++) {
        candidateItems[i] = new MImWordListItem(contentWidget);
        candidateItems[i]->setVisible(false);
        connect(candidateItems[i], SIGNAL(clicked()), this, SLOT(select()));
        mainLayout->addItem(candidateItems[i]);
    }
    setCentralWidget(contentWidget);
#endif
    hide();
}

MImWordList::~MImWordList()
{
}

void MImWordList::setCandidates(const QStringList &candidates, bool typedWordIsInDictionary)
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(candidates)
    Q_UNUSED(typedWordIsInDictionary)
#else
    if (candidates.isEmpty()) {
        qWarning() << __PRETTY_FUNCTION__ << "empty candidates list";
    } else {
        // First candidate, the originally typed word, is show in the dialog header
        setTitle(candidates[0]);
    }

    int candidateItemsStartIndex = 0;
    if (typedWordIsInDictionary) {
        mCandidates = candidates.mid(1, MaxCandidateCount);

        removeItem(mainLayout, addToDictionaryItem);
        removeItem(mainLayout, addToDictionarySeparator);
    } else {
        mCandidates = candidates.mid(0, MaxCandidateCount);
        candidateItemsStartIndex = 1;

        addItem(mainLayout, addToDictionaryItem, 0);
        addItem(mainLayout, addToDictionarySeparator, 1);
    }

    for (int i = 0; i < MaxCandidateCount; i++) {
        if (i < mCandidates.count() && i >= candidateItemsStartIndex) {
            candidateItems[i]->setTitle(mCandidates.at(i));
            addItem(mainLayout, candidateItems[i], -1);
        } else {
            removeItem(mainLayout, candidateItems[i]);
        }
    }
    mainLayout->invalidate();
#endif
}

QStringList MImWordList::candidates() const
{
    return mCandidates;
}

void MImWordList::select()
{
#ifdef HAVE_MEEGOTOUCH
    // ignore the select actions during animation
    if (!isVisible() || sceneWindowState() == MSceneWindow::Appearing
        || sceneWindowState() == MSceneWindow::Disappearing) {
        return;
    }
    MImWordListItem *item = qobject_cast<MImWordListItem *> (sender());
    if (item) {
        const QString candidate = (item == addToDictionaryItem) ? title() : item->title();
        emit candidateClicked(candidate);
    }
#endif
}

void MImWordList::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    return;
#else
    if (!isVisible())
        return;
    // word list take whole screen. And inner contentitem will play their
    // default feedback. Don't need reaction map.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(QTransform());
    reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());
#endif // HAVE_REACTIONMAP
}
