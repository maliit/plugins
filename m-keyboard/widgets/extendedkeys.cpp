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

#include "extendedkeys.h"
#include "magnifierhost.h"
#include "reactionmapwrapper.h"

#include <regiontracker.h>
#include <layoutdata.h>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QMargins>
#include <QAnimationGroup>

#ifdef HAVE_MEEGOTOUCH
#include <MFeedback>
#include <MCancelEvent>
#endif

namespace {
    QSize queryKeyAreaSize(const MImAbstractKeyArea *keyArea,
                           int keyCount)
    {
        const MImAbstractKeyAreaStyleContainer &style(keyArea->baseStyle());

        return QSize(style->keyWidthMediumFixed() * keyCount
                     + style->keyMarginLeft() * (keyCount - 1)
                     + style->keyMarginRight() * (keyCount - 1)
                     + style->paddingLeft()
                     + style->paddingRight(),
                     keyArea->size().height());
    }

    /*
     * Align given keyArea to put interspace between some keys
     * on top of origin position
     */
    void alignExtendedKeyArea(MImAbstractKeyArea *keyArea,
                      int keyCount,
                      const QPointF &origin,
                      int extendedKeysOffset,
                      const QMargins &safetyMargins)
    {
        if (!keyArea || keyCount < 2) {
            return;
        }

        const MImAbstractKeyAreaStyleContainer &style(keyArea->baseStyle());
        const int paddingL = style->paddingLeft();
        const int keyWidth = style->keyWidthMediumFixed();
        const int keyMarginL = style->keyMarginLeft();
        const int keyMarginR = style->keyMarginRight();
        // Position of interspace between first and second key.
        // This line actually depends in MImKey implementation.
        const int firstPosCandidate = paddingL + keyWidth + keyMarginR;

        // Select interspace which should be placed on top of origin.
        // n==0 points to interspace between first and second keys
        int n = qRound((origin.x() - keyArea->pos().x()
                        - firstPosCandidate)
                       / (keyWidth + keyMarginL + keyMarginR));
        // We do not want to put right border of last key on top
        // of origin position, so upper bound is "keyCount - 2"
        // instead of "keyCount -1"
        n = qBound(0, n, keyCount - 2);

        const QPointF correction(firstPosCandidate
                                 + n * (keyWidth + keyMarginL + keyMarginR),
                                 keyArea->boundingRect().bottom());

        const QPointF pos(origin
                          - correction
                          + QPointF(0, extendedKeysOffset));

        MagnifierHost::applyConstrainedPosition(keyArea, keyArea->parentItem(), pos,
                                                safetyMargins);
    }
}

ExtendedKeys::ExtendedKeys(MagnifierHost *newHost,
                           MImAbstractKeyArea *newMainArea)
    : MImOverlay()
    , ReactionMapPaintable()
    , host(newHost)
    , mainArea(newMainArea)
    , hideOnNextMouseRelease(false)
    , showAnimation(this, "magnitude")
    , currentMagnitude(1)
{
    setObjectName("ExtendedKeys"); // needed by MATTI (but useful otherwise too)
    RegionTracker::instance().addRegion(*this);

    setFlags(QGraphicsItem::ItemHasNoContents);
    setParent(newHost);

    showAnimation.setStartValue(0.0f);
    showAnimation.setEndValue(1.0f);
}

ExtendedKeys::~ExtendedKeys()
{}

bool ExtendedKeys::isPaintable() const
{
    return isVisible();
}

bool ExtendedKeys::isFullScreen() const
{
    // It acts as fullscreen
    return true;
}

void ExtendedKeys::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
    // Cast is needed for MImAbstractKeyArea * caused by protected reimplementation
    static_cast<MImAbstractKeyArea *>(extKeysArea.get())->drawReactiveAreas(reactionMap, view);
}

void ExtendedKeys::showExtendedArea(const QPointF &origin,
                                    const QPointF &tappedScenePos,
                                    const QString &labels)
{
    LayoutData::SharedLayoutSection section(new LayoutSection(labels));

    // Custom haptic feedback for this popup being appeared
#ifdef HAVE_MEEGOTOUCH
    MFeedback::play("priority2_vkb_popup_press");
#endif
    // TODO: disable swipe gestures from extended keys area
    extKeysArea.reset(ExtendedKeysArea::create(section, this));
    extKeysArea->setStyleName("ExtendedKeys");

    // Send clicked signal as if it was from mainArea.
    connect(extKeysArea.get(), SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)),
            mainArea,          SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)));

    connect(extKeysArea.get(), SIGNAL(keyClicked(const MImAbstractKey *, const KeyContext &)),
            host,              SLOT(hide()));

    connect(extKeysArea.get(), SIGNAL(displayExited()),
            host,              SLOT(hide()));

    extKeysArea->resize(queryKeyAreaSize(extKeysArea.get(), extKeysArea->maxColumns()));
    extKeysArea->setParentItem(this);

    // Install scene event filter to detect keyarea mouse release. If first
    // release didn't end up emitting a keyClicked() signal, this popup will
    // be hidden always on the second release.
    extKeysArea->installSceneEventFilter(this); // After adding to same scene.
    hideOnNextMouseRelease = false;

    // Through the main area, we can access the VKB coordinate system:
    const QPointF originMapped(mainArea->mapToItem(this, origin));
    const QPointF correction(extKeysArea->boundingRect().center().x(),
                             extKeysArea->boundingRect().bottom());
    const QPointF extKeysPos(originMapped
                             - correction
                             + QPointF(0, host->style()->extendedKeysOffset()));

    const MKeyboardMagnifierStyleContainer &style = host->style();
    QMargins safetyMargins(style->safetyMarginLeft(),  MagnifierHost::InvalidMargin,
                           style->safetyMarginRight(), MagnifierHost::InvalidMargin);
    MagnifierHost::applyConstrainedPosition(extKeysArea.get(), extKeysArea->parentItem(), extKeysPos,
                                            safetyMargins);

    if (extKeysArea->pos() != extKeysPos) {
        alignExtendedKeyArea(extKeysArea.get(), labels.count(), originMapped,
                             host->style()->extendedKeysOffset(), safetyMargins);
    }

    showAnimation.setEasingCurve(host->style()->extendedKeysShowEasingCurve());
    showAnimation.setDuration(host->style()->extendedKeysShowDuration());

    setMagnitude(showAnimation.startValue().value<qreal>());
    show();

    // ExtendedKeys is shown on long-press. Therefore we will give mouse grab to the new
    // extKeysArea.  mainArea (MImAbstractKeyArea) does some state reseting once it loses
    // mouse grab, but we'll nevertheless send it MCancelEvent for the following reasons:
    //
    // * Bug NB#248227: cancel event handler has a workaround that prevents further touch
    // events from being handled.
    // * We don't need to reset touch point counts of active keys here
    // * What should be reset in MImAbstractKeyArea::ungrabMouseEvent and why is not
    // really well defined at the moment.
#ifdef HAVE_MEEGOTOUCH
    MCancelEvent cancel;
    mainArea->scene()->sendEvent(mainArea, &cancel);
#endif

    // Grab will be removed when widget is hidden.
    extKeysArea->grabMouse();

    QGraphicsSceneMouseEvent press(QEvent::GraphicsSceneMousePress);
    press.setPos(extKeysArea->mapFromScene(tappedScenePos));
    press.setLastPos(press.pos());
    press.setScenePos(tappedScenePos);
    press.setLastScenePos(press.scenePos());
    scene()->sendEvent(extKeysArea.get(), &press);

    // Update the reaction maps right now
    signalForwarder.emitRequestRepaint();
    // Update the reaction maps if the popup disappears
    connect(extKeysArea.get(), SIGNAL(displayExited()),
            &signalForwarder, SIGNAL(requestRepaint()));
}

QVariant ExtendedKeys::itemChange(GraphicsItemChange change,
                                  const QVariant &value)
{
    if (change == QGraphicsItem::ItemVisibleChange
        && !value.toBool()) {
        extKeysArea->removeSceneEventFilter(this);
    }
    return MImOverlay::itemChange(change, value);
}

bool ExtendedKeys::sceneEventFilter(QGraphicsItem *, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneMouseRelease) {
        if (hideOnNextMouseRelease) {
            hideOnNextMouseRelease = false;
            // Hide after the release is processed.
            QMetaObject::invokeMethod(host, "hide", Qt::QueuedConnection);
        } else {
            hideOnNextMouseRelease = true;
        }
    }
    return false; // Allow event to reach extKeysArea.
}

qreal ExtendedKeys::magnitude() const
{
    return currentMagnitude;
}

void ExtendedKeys::setMagnitude(qreal value)
{
    if (value < 0.0f) {
        return;
    }

    currentMagnitude = value;
    QTransform t;
    t.translate(anchorPoint.x() * (1.0f - value),
                anchorPoint.y() * (1.0f - value));
    t.scale(value, value);
    setTransform(t);
}

void ExtendedKeys::hideExtendedArea()
{
    showAnimation.stop();
    hide();
}

void ExtendedKeys::setAnchorPoint(const QPointF &anchor)
{
    anchorPoint = mapFromScene(anchor);
}

void ExtendedKeys::addToGroup(QAnimationGroup *group)
{
    group->addAnimation(&showAnimation);
}

