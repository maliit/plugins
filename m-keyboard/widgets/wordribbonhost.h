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

#ifndef WORDRIBBONHOST_H
#define WORDRIBBONHOST_H

#include "reactionmappaintable.h"
#include "abstractenginewidgethost.h"
#include "mwidget-wrapper.h"
#include <QContiguousCache>
#include <QGraphicsWidget>
#include <QPointer>
#include <QString>

class WordRibbon;
class WordRibbonDialog;
class MReactionMap;

class WordRibbonHost : public AbstractEngineWidgetHost, public ReactionMapPaintable
{
    Q_OBJECT

    friend class Ut_WordRibbonHost;

public:
    WordRibbonHost(MWidget *w, QObject* parent = 0);
    virtual ~WordRibbonHost();

    // Reimplement for AbstractEngineWidgetHost.
    /*! \reimp */
    virtual QGraphicsWidget *engineWidget() const;
    virtual QGraphicsWidget *inlineWidget() const;
    virtual bool isActive() const;
    virtual void setTitle(QString &title);
    virtual void setCandidates(const QStringList &candidates);
    virtual void appendCandidates(int startPos, const QStringList &candidate);
    virtual QStringList candidates() const;
    virtual void showEngineWidget(DisplayMode mode);
    virtual void hideEngineWidget();
    virtual DisplayMode displayMode() const;
    virtual void watchOnWidget(QGraphicsWidget *widget);
    virtual void setPosition(const QRect &cursorRect);
    virtual void handleNavigationKey(NaviKey key);
    virtual int suggestedWordIndex() const;
    virtual void prepareToOrientationChange();
    virtual void finalizeOrientationChange();
    virtual void reset();
    virtual void setPageIndex(int index);
    virtual void handleAppOrientationChanged();
    /*! \reimp_end */

    // Reimplement for ReactionMapPaintable.
    /*! \reimp */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);
    bool isPaintable() const;
    bool isFullScreen() const;
    /*! \reimp_end */

private:
    void clearCandidate();
    bool isEmpty();
    void fetchMoreCandidates();
    
private slots:
    void updatePosition();
    void openWordRibbonDialog();
    void handleDialogClosed();

signals:
    void sizeUpdated(const QSize );

private:
    QList<QPointer<QGraphicsWidget> > watchedWidgets;
    WordRibbon* wordRibbon;
    WordRibbonDialog* ribbonDialog;
    int uniqueIndex;
    QString dialogTitle;
    QContiguousCache<QString> candidatesCache;
};

#endif
