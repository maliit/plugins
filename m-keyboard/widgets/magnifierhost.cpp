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

#include "magnifierhost.h"
#include "mimabstractkey.h"

#include <QDebug>
#include <QRectF>
#include <QDebug>

namespace {
    QString computeExtendedLabelsFromKey(const MImAbstractKey *key)
    {
        QString labels;

        if (key) {
            if (key->isNormalKey()) {
                labels.append(key->binding().label());
            }

            labels.append(key->binding().extendedLabels());
        }

        return labels;
    }
}

void MagnifierHost::applyConstrainedPosition(QGraphicsItem *target,
                                             QGraphicsItem *geometryParentItem,
                                             const QPointF &newPos,
                                             const QMargins &safetyMargins,
                                             MagnifierHost::OriginPolicy policy)
{
    if (!target || !geometryParentItem || !target->parentItem()) {
        qWarning() << __PRETTY_FUNCTION__
                   << "No target or missing parent item - cannot compute constraints.";
        return;
    }

    QRectF parentRect = geometryParentItem->boundingRect().adjusted(safetyMargins.left(),
                                                                    safetyMargins.top(),
                                                                    - safetyMargins.right(),
                                                                    - safetyMargins.bottom());

    QPointF newPosVsRealParent;
    if (geometryParentItem != target->parentItem()) {
        newPosVsRealParent = target->parentItem()->mapFromItem(geometryParentItem, newPos);
        parentRect.translate(newPosVsRealParent - newPos);
    } else {
        newPosVsRealParent = newPos;
    }
    // further, all coordinates are of real parent

    QRectF targetRect = target->mapRectToParent(target->boundingRect());

    // The target's position will most likely not match with its bounding
    // rect's top left corner. So computing the targetRect's new position
    // needs to account for that.
    // Also keep in mind that the target's pos() is in parent's
    // coordinates.
    targetRect.translate(newPosVsRealParent - target->pos());

    if (safetyMargins.left() != InvalidMargin && (targetRect.left() < parentRect.left())) {
        targetRect.moveLeft(parentRect.left());
    }

    if (safetyMargins.top() != InvalidMargin && (targetRect.top() < parentRect.top())) {
        targetRect.moveTop(parentRect.top());
    }

    if (safetyMargins.right() != InvalidMargin && (targetRect.right() > parentRect.right())) {
        targetRect.moveRight(parentRect.right());
    }

    if (safetyMargins.bottom() != InvalidMargin && (targetRect.bottom() > parentRect.bottom())) {
        targetRect.moveBottom(targetRect.bottom());
    }

    switch(policy) {
    case MagnifierHost::UseGraphicsViewItemOrigins:
        target->setPos(targetRect.center());
        break;

    case MagnifierHost::UseMImKeyAreaOrigins:
        target->setPos(targetRect.topLeft());
        break;
    }
}

MagnifierHost::MagnifierHost()
{
    styleContainer.initialize(QString(), QString(), 0);
    hideDelayTimer.setSingleShot(true);
    hideDelayTimer.setInterval(styleContainer->magnifierHideDelay());

    connect(&hideDelayTimer, SIGNAL(timeout()),
            this,            SLOT(hide()),
            Qt::QueuedConnection);
}

MagnifierHost::~MagnifierHost()
{
    reset();
}

void MagnifierHost::setMainArea(MImAbstractKeyArea *mainArea)
{
    reset();

    if (not mainArea) {
        return;
    }

    setParent(mainArea);
    magnifier = QPointer<Magnifier>(new Magnifier(this, mainArea));
    magnifier->setup();

    extKeys = QPointer<ExtendedKeys>(new ExtendedKeys(this, mainArea));

    // old animations were already destroyed together with previous
    // values of magnifier and extKeys, so we do not need to clean animationGroup
    magnifier->addToGroup(&animationGroup);
    extKeys->addToGroup(&animationGroup);
}

const MKeyboardMagnifierStyleContainer &MagnifierHost::style() const
{
    return styleContainer;
}

void MagnifierHost::updatePos(const QPointF &keyPos,
                              const QPoint &screenPos,
                              const QSize &keySize)
{
    magnifier->updatePos(keyPos, screenPos, keySize);
}

void MagnifierHost::cancel()
{
    if (magnifier->isVisible() && !extKeys->isVisible()) {
        hideDelayTimer.start();
    }
}

void MagnifierHost::handleKeyPressedOnMainArea(MImAbstractKey *key,
                                               const KeyContext &)
{
    if (!key) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Invalid key press detected, or MagnifierHost not enabled!";
        return;
    }

    if (key->label().isEmpty()
        || !key->isNormalKey()) {
        magnifier->hide();
        return;
    }

    hideDelayTimer.stop();
    magnifier->setLabel(key->label());
    magnifier->showMagnifier();
}

void MagnifierHost::handleLongKeyPressedOnMainArea(MImAbstractKey *key,
                                                   const KeyContext &keyContext)
{
    if (!key) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Invalid key press detected, or MagnifierHost not enabled!";
        return;
    }

    QString labels = computeExtendedLabelsFromKey(key);

    if (labels.count() < 2) {
        // It usually makes no sense to show an extended area for one key.
        return;
    }

    QPointF buttonPos = key->buttonRect().topLeft();
    buttonPos.rx() += key->buttonRect().width() / 2;
    extKeys->setAnchorPoint(magnifier->mapToScene(magnifier->boundingRect().center()));
    extKeys->showExtendedArea(buttonPos,
                              keyContext.scenePos,
                              labels);
    animationGroup.start();
}

bool MagnifierHost::isVisible() const
{
    return (magnifier->isVisible() || extKeys->isVisible());
}

void MagnifierHost::setVisible(bool visible)
{
    magnifier->setVisible(visible);
    extKeys->setVisible(visible);
}

void MagnifierHost::hide()
{
    setVisible(false);
}

void MagnifierHost::reset()
{
    delete magnifier;
    delete extKeys;

    hideDelayTimer.stop();
}
