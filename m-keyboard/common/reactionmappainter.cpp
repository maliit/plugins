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

#include "reactionmappainter.h"
#include "reactionmappainter_p.h"

#include <mplainwindow.h>
#ifdef HAVE_REACTIONMAP
#include <mreactionmap.h>
#endif

#include "reactionmappaintable.h"

// ReactionMapPainterPrivate......................................................

ReactionMapPainterPrivate::ReactionMapPainterPrivate()
{
    repaintTimer.setSingleShot(true);
    connect(&repaintTimer, SIGNAL(timeout()), this, SLOT(repaint()));
}

void ReactionMapPainterPrivate::addWidget(ReactionMapPaintable &widget)
{
    connect(&widget.signalForwarder, SIGNAL(requestRepaint()),
            this, SLOT(requestRepaint()), Qt::UniqueConnection);
    connect(&widget.signalForwarder, SIGNAL(requestClear()),
            this, SLOT(clear()), Qt::UniqueConnection);
    widgets.push_back(&widget);
}

void ReactionMapPainterPrivate::removeWidget(const ReactionMapPaintable &widget)
{
    const int pos(widgets.indexOf(const_cast<ReactionMapPaintable*>(&widget)));

    if (pos >= 0) {
        widgets.remove(pos);
    }
}

void ReactionMapPainterPrivate::clear()
{
#ifndef HAVE_REACTIONMAP
    return;
#else
    const QList<QGraphicsView *> views = MPlainWindow::instance()->scene()->views();

    // Draw invisible color to all reaction maps
    foreach (QGraphicsView *view, views) {
        MReactionMap *reactionMap = MReactionMap::instance(*view);

        if (reactionMap) {
            reactionMap->setDrawingValue(MReactionMap::Transparent, MReactionMap::Transparent);
            reactionMap->setTransform(QTransform());
            reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());
        }
    }
#endif
}


void ReactionMapPainterPrivate::requestRepaint()
{
    if (repaintTimer.isActive())
        return;

    // The reaction map painting is queued because if it is scheduled too early then
    // the widget geometries are not ready yet and wrong reaction maps will be
    // painted.
    repaintTimer.start(30);
}

void ReactionMapPainterPrivate::repaint()
{
#ifndef HAVE_REACTIONMAP
    return;
#else
    const QList<QGraphicsView *> views = MPlainWindow::instance()->scene()->views();

    clear();
    // Draw all reaction maps
    foreach (QGraphicsView *view, views) {
        MReactionMap *reactionMap = MReactionMap::instance(*view);

        if (!reactionMap) {
            continue;
        }

        // Draw the first full-screen and paintable widget and go to the
        // next reaction map
        bool fullScreenWidget = false;

        foreach (ReactionMapPaintable *widget, widgets) {

            if (widget->isFullScreen() && widget->isPaintable())
            {
                widget->paintReactionMap(reactionMap, view);
                fullScreenWidget = true;
                break;
            }
        }
        // Don't draw non-fullscreen widgets and go to the next reaction map
        if (fullScreenWidget)
            continue;
        // Draw the non-fullscreen and paintable widgets
        foreach (ReactionMapPaintable *widget, widgets) {

            if (widget->isPaintable()) {
                widget->paintReactionMap(reactionMap, view);
            }
        }
    }
#endif
}


// ReactionMapPainter.............................................................

ReactionMapPainter *ReactionMapPainter::singleton = 0;

ReactionMapPainter::ReactionMapPainter()
    : d_ptr(new ReactionMapPainterPrivate)
{
}

ReactionMapPainter::~ReactionMapPainter()
{
    delete d_ptr;
}

void ReactionMapPainter::createInstance()
{
    Q_ASSERT(!singleton);
    if (!singleton) {
        singleton = new ReactionMapPainter();
    }
}

void ReactionMapPainter::destroyInstance()
{
    Q_ASSERT(singleton);
    delete singleton;
    singleton = 0;
}

void ReactionMapPainter::addWidget(ReactionMapPaintable &widget)
{
    Q_D(ReactionMapPainter);
    d->addWidget(widget);
}

void ReactionMapPainter::removeWidget(const ReactionMapPaintable &widget)
{
    Q_D(ReactionMapPainter);
    d->removeWidget(widget);
}

void ReactionMapPainter::clear()
{
    Q_D(ReactionMapPainter);
    d->clear();
}

void ReactionMapPainter::repaint()
{
    Q_D(ReactionMapPainter);
    d->requestRepaint();
}
