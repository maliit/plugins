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

#ifdef HAVE_MEEGOTOUCH
#include "wordribbonhost.h"
#include "wordribbon.h"
#include "wordribbondialog.h"
#include "regiontracker.h"
#include "enginemanager.h"
#include "reactionmapwrapper.h"

#include <mimenginewordsinterface.h>
#include <MSceneManager>
#include <mplainwindow.h>
#include <MStylableWidget>

#include <limits>
#include <QtGui>
#include <QtCore>

namespace {
    const int MaximumWordRibbonDialogCount = 100;
    const int InitialCandidateCount = 20;
}

WordRibbonHost::WordRibbonHost(MWidget *w, QObject *parent):
        AbstractEngineWidgetHost(w, parent),
        ReactionMapPaintable(),
        wordRibbon(new WordRibbon(WordRibbon::RibbonStyleMode, w)),
        ribbonDialog(new WordRibbonDialog()),
        uniqueIndex(-1),
        candidatesCache(MaximumWordRibbonDialogCount)
{
    setObjectName("WordRibbonHost");

    bool ok = false;
    ribbonDialog->setVisible(false);
    connect(ribbonDialog, SIGNAL(finished(int)), this, SLOT(handleDialogClosed()));
    connect(ribbonDialog, SIGNAL(candidateClicked(QString,int)), this, SIGNAL(candidateClicked(QString,int)));
    // The dialog goes on top -> Clear the reaction maps
    connect(ribbonDialog, SIGNAL(displayEntered()), &signalForwarder, SIGNAL(requestClear()));
    // The dialog disappears -> Repaint the reaction maps
    connect(ribbonDialog, SIGNAL(displayExited()), &signalForwarder, SIGNAL(requestRepaint()));

    ok = connect(wordRibbon, SIGNAL(itemClicked(QString, int)), this, SIGNAL(candidateClicked(QString, int)));
    Q_ASSERT(ok);
    ok = connect(wordRibbon, SIGNAL(moreCandidatesRequested()), this, SLOT(openWordRibbonDialog()));
    Q_ASSERT(ok);
    wordRibbon->hide();

    RegionTracker::instance().addRegion(*wordRibbon);
    RegionTracker::instance().addInputMethodArea(*wordRibbon);
}

WordRibbonHost::~WordRibbonHost()
{
    if(ribbonDialog != NULL)
    {
        delete ribbonDialog;
        ribbonDialog = NULL;
    }
}

void WordRibbonHost::setTitle(QString &title)
{
    dialogTitle = title;
}

void WordRibbonHost::setCandidates(const QStringList &candidates)
{
    if (candidates.isEmpty()) {
        clearCandidate();
    }
    else {
        candidatesCache.clear();
        if (candidates.count() > candidatesCache.capacity()) {
            candidatesCache.setCapacity(candidates.count() );
        }
        for (int i = 0; i < candidates.count(); ++i) {
            candidatesCache.insert(i, candidates[i]);
        }
        wordRibbon->repopulate(candidates);
    }

    // Request reaction map repaint.
    signalForwarder.emitRequestRepaint();
}

void WordRibbonHost::watchOnWidget(QGraphicsWidget *widget)
{
    if (!widget) {
        return;
    }

    wordRibbon->setZValue(widget->zValue());
    connect(widget, SIGNAL(yChanged()), this, SLOT(updatePosition()));
    connect(widget, SIGNAL(visibleChanged()), this, SLOT(updatePosition()));
    watchedWidgets.append(widget);
    updatePosition();

    watchedWidgets.removeAll(QPointer<QGraphicsWidget>()); //remove all invalid pointers
}

void WordRibbonHost::prepareToOrientationChange()
{
}

void WordRibbonHost::finalizeOrientationChange()
{
    wordRibbon->finalizeOrientationChange();

    if (ribbonDialog->isVisible()) {
        ribbonDialog->finalizeOrientationChange();
    }
}

void WordRibbonHost::handleAppOrientationChanged()
{
}

void WordRibbonHost::showEngineWidget(DisplayMode mode)
{
    if (mode != AbstractEngineWidgetHost::DockedMode)
        return;

    wordRibbon->show();
    updatePosition();
}

void WordRibbonHost::hideEngineWidget()
{
    wordRibbon->hide();

    updatePosition();
}

void WordRibbonHost::clearCandidate()
{
    candidatesCache.clear();
    wordRibbon->clear();
    ribbonDialog->accept();

    // Request reaction map repaint.
    signalForwarder.emitRequestRepaint();
}

void WordRibbonHost::openWordRibbonDialog()
{
    //must fetch before show full dialog
    fetchMoreCandidates();

    QStringList list;
    for (int i = candidatesCache.firstIndex();
        i <= candidatesCache.lastIndex(); ++i) {
        list << candidatesCache.at(i);
    }
    ribbonDialog->setCandidates(list, dialogTitle);
    ribbonDialog->appear(MPlainWindow::instance());
}

void WordRibbonHost::handleDialogClosed()
{
}

void WordRibbonHost::updatePosition()
{
    qreal bottom(std::numeric_limits<qreal>::max());
    bool widgetVisible(false);

    foreach (const QGraphicsWidget *widget, watchedWidgets) {
        if (widget && widget->isVisible()) {
            bottom = qMin(widget->pos().y(), bottom);
            widgetVisible = true;
        }
    }

    const QPointF newPos(0, bottom - wordRibbon->size().height());
    if (widgetVisible) {
        wordRibbon->setPos(newPos);
    }
}

void WordRibbonHost::handleNavigationKey(NaviKey key)
{
    if (isEmpty())
        return;

    wordRibbon->handleNavigationKey(key);
}

AbstractEngineWidgetHost::DisplayMode WordRibbonHost::displayMode() const
{
    if (ribbonDialog->isVisible())
        return AbstractEngineWidgetHost::DialogMode;
    else
        return AbstractEngineWidgetHost::DockedMode;
}

void WordRibbonHost::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
    if (ribbonDialog->isVisible())
        ribbonDialog->paintReactionMap(reactionMap, view);
    else
        wordRibbon->paintReactionMap(reactionMap, view);
}

bool WordRibbonHost::isPaintable() const
{
    return isActive();
}

bool WordRibbonHost::isFullScreen() const
{
    return displayMode() == AbstractEngineWidgetHost::DialogMode;
}

void WordRibbonHost::appendCandidates(int startPos, const QStringList &candidate)
{
    if (startPos + candidate.count() > this->candidatesCache.capacity()) {
        this->candidatesCache.setCapacity(startPos + candidate.count());
    }
    for (int i = 0; i < candidate.count(); ++i) {
        this->candidatesCache.insert(startPos + i, candidate[i]);
    }
}

void WordRibbonHost::fetchMoreCandidates()
{
    // No need to fetch if it the cache is already full
    if ((candidatesCache.lastIndex() + 1) >= MaximumWordRibbonDialogCount)
        return;

    // fetch more candidates if there's room in the cache
    int requestLength = MaximumWordRibbonDialogCount - candidatesCache.lastIndex() - 1;
    int requestStartPos = candidatesCache.lastIndex() + 1;

    QStringList candidatesFromEngine;

    EngineManager *em = &EngineManager::instance();
    if (em && em->engine()) {
        candidatesFromEngine = EngineManager::instance().engine()->candidates(requestStartPos, requestLength);
        appendCandidates(requestStartPos, candidatesFromEngine);
    } else {
        qWarning() << __PRETTY_FUNCTION__ << "No engine found, cannot fetch candidates!";
    }
}

void WordRibbonHost::setPageIndex(int index)
{
    if (index != 0)
        return;

    if (candidatesCache.isEmpty() )
        return ;

    QStringList tmpList;
    for (int i = 0; i < InitialCandidateCount && i < candidatesCache.count(); ++i) {
        tmpList << this->candidatesCache.at(i);
    }
    wordRibbon->repopulate(tmpList);
}

bool WordRibbonHost::isActive() const
{
    return (wordRibbon->isVisible() || ribbonDialog->isVisible() );
}

QStringList WordRibbonHost::candidates() const
{
    QStringList retList;
    for(int i = 0; i < candidatesCache.count(); i++) {
        retList << candidatesCache.at(i);
    }
    return retList;
}

void WordRibbonHost::setPosition(const QRect &cursorRect)
{
    Q_UNUSED(cursorRect);
}

bool WordRibbonHost::isEmpty()
{
    return candidatesCache.isEmpty();
}

int WordRibbonHost::suggestedWordIndex() const
{
    return -1;
}

void WordRibbonHost::reset()
{
    clearCandidate();
}

QGraphicsWidget * WordRibbonHost::engineWidget() const
{
    QGraphicsWidget *widget = 0;

    if (isActive()) {
        widget = wordRibbon->isVisible() ? qobject_cast<QGraphicsWidget *>(wordRibbon)
                 : qobject_cast<QGraphicsWidget *>(ribbonDialog);
    }
    return widget;
}

QGraphicsWidget * WordRibbonHost::inlineWidget() const
{
    return qobject_cast<QGraphicsWidget *>(wordRibbon);
}

#endif // HAVE_MEEGOTOUCH
