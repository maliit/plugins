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

#ifndef MAGNIFIERHOST_H
#define MAGNIFIERHOST_H

#include "mimabstractpopup.h"
#include "mkeyboardmagnifierstyle.h"
#include "mimoverlay.h"
#include "magnifier.h"
#include "extendedkeys.h"
#include "mimabstractkeyarea.h"

#include <QObject>
#include <QWeakPointer>
#include <QMargins>
#include <QGraphicsItem>
#include <QPointF>
#include <QSequentialAnimationGroup>

class MagnifierHost
    : public QObject, public MImAbstractPopup
{
    Q_OBJECT

private:
    QPointer<Magnifier> magnifier;
    QPointer<ExtendedKeys> extKeys;

    //! Timer for delay hiding magnifier.
    QTimer hideDelayTimer;

    //! Magnifier style container.
    MKeyboardMagnifierStyleContainer styleContainer;

    //! Transition from magnifier to extended keys
    QSequentialAnimationGroup animationGroup;

public:
    //! The Magnifier item is the only item in our code base using the
    //! conventional QGraphicsView semantics for an item's position and
    //! bounding rect computation (pos() is the center of the item). Our
    //! MImKeyArea assumes that the position is the top left corner of an item.
    enum OriginPolicy {
        UseGraphicsViewItemOrigins, //!< pos() == boundingRect().center()
        UseMImKeyAreaOrigins        //!< pos() == boundingRect().topLeft()
    };

    explicit MagnifierHost();
    virtual ~MagnifierHost();

    //! Moves target item to new position, respecting the parent's bounding
    //! box and the safety margins.
    //! Negative safety margins imply that the parent's bounding box shall be
    //! ignored for those directions.
    //! \geometryParentItem, if it is different from parentItem of target,
    //!     is used as constraint for the target
    //! \newPos is in coordinates of geometryParentItem
    static void applyConstrainedPosition(QGraphicsItem *target,
                                         QGraphicsItem *parentItem,
                                         const QPointF &newPos,
                                         const QMargins &safetyMargins,
                                         OriginPolicy policy = UseMImKeyAreaOrigins);

    const MKeyboardMagnifierStyleContainer &style() const;

    //! \reimp
    virtual void setMainArea(MImAbstractKeyArea *mainArea);
    virtual void updatePos(const QPointF &keyPos,
                           const QPoint &screenPos,
                           const QSize &keySize);

    virtual void cancel();
    virtual void handleKeyPressedOnMainArea(MImAbstractKey *key,
                                            const KeyContext &keyContext);

    virtual void handleLongKeyPressedOnMainArea(MImAbstractKey *key,
                                                const KeyContext &keyContext);

    virtual bool isVisible() const;
    virtual void setVisible(bool visible);
    //! \reimp_end

    //! This value idenfifies margin which disables
    //! verification in applyConstrainedPosition()
    static const int InvalidMargin = 1000000;

public slots:
    //! Hide Magnifier and ExtendedKeys
    //! Since ExtendedKeys usually hides on key press, it is recommended to
    //! call this slot async. Otherwise, unexpected event sequences such as
    //! 1. touch press, 2. hide, 3. mouse press can happen.
    //! \sa QTimer::singleShot, Qt::QueuedConnection
    void hide();

private:
    //! Resets instance. Need to use setMainArea before it becomes usable again.
    void reset();
};

#endif // MagnifierHost_H
