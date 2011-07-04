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

#ifndef EXTENDEDKEYS_H
#define EXTENDEDKEYS_H

#include "reactionmappaintable.h"

#include <mimoverlay.h>
#include <mimkeyarea.h>
#include <mimabstractkeyarea.h>
#include <QSize>
#include <QPointer>
#include <QPropertyAnimation>
#include <memory>

class MagnifierHost;
class MImAbstractKey;
class QWidget;
class QPainter;
class QStyleOptionGraphicsItem;
class QAnimationGroup;

//! \internal
//! \brief Allows access to protected areas of SingleWidgetButtonArea
class ExtendedKeysArea
    : public MImKeyArea
{
public:
    static ExtendedKeysArea *create(const LayoutData::SharedLayoutSection &section,
                                    QGraphicsWidget *parent)
    {
        ExtendedKeysArea *keyArea(new ExtendedKeysArea(section, parent));
        keyArea->init();
        return keyArea;
    }

    virtual ~ExtendedKeysArea()
    {}

    virtual bool sceneEvent(QEvent *ev)
    {
        bool result = MImKeyArea::sceneEvent(ev);

        if (ev->type() == QEvent::GraphicsSceneMouseRelease) {
            lockVerticalMovement(false);
        }

        return result;
    }

protected:
    explicit ExtendedKeysArea(const LayoutData::SharedLayoutSection &section,
                              QGraphicsWidget *parent)
        : MImKeyArea(section, parent)
    {
        // Used by MATTI:
        setObjectName("VirtualKeyboardExtendedArea");
        lockVerticalMovement(true);
    }

    friend class ExtendedKeys;

};
//! \internal_end

class ExtendedKeys
    : public MImOverlay, public ReactionMapPaintable
{
    Q_OBJECT

    //! \brief Item's magnitude
    //!
    //! This value changes scale of the widget, but keeps anchor point
    //! at the same position.
    //! \sa setAnchorPoint
    Q_PROPERTY(qreal magnitude READ magnitude WRITE setMagnitude)

private:
    //! host allows interaction with other Magnifier plugin parts.
    MagnifierHost *host;

    //! mainArea allows interaction with VKB.
    QPointer<MImAbstractKeyArea> mainArea;

    //! The actual widget showing the extended keys.
    std::auto_ptr<ExtendedKeysArea> extKeysArea;

    //! Used for non-multitouch mode
    bool hideOnNextMouseRelease;

    //! Show animation
    QPropertyAnimation showAnimation;

    //! Current magnitude. Default value is 1.
    qreal currentMagnitude;

    //! Anchor point for animation.
    QPointF anchorPoint;

private:
    //! Return current magnitude.
    qreal magnitude() const;

    //! Set magnitude to given value.
    void setMagnitude(qreal value);

public:
    explicit ExtendedKeys(MagnifierHost *host,
                          MImAbstractKeyArea *mainArea);
    virtual ~ExtendedKeys();

    /*! \reimp */
    bool isPaintable() const;
    bool isFullScreen() const;
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);
    /*! \reimp_end */

    //! \brief Set anchor point for animation.
    //! \param anchor Some position inside key area in scene coordinates.
    void setAnchorPoint(const QPointF &anchor);

    //! Add extended keys area to given animation \a group.
    void addToGroup(QAnimationGroup *group);

public slots:
    //! \brief Creates (and shows) a KeyButtonArea from labels.
    //! \param origin Usually the center of the active button in the main key area,
    //!        used to position the new extended area.
    //! \param tappedScenePos Position where finger was tapped, in scene coordinates.
    //!        Used to apply initial press on the new extended area widget.
    //! \param labels The labels to use for the buttons.
    void showExtendedArea(const QPointF &origin,
                          const QPointF &tappedScenePos,
                          const QString &labels);

    //! \brief Hide extended area
    void hideExtendedArea();

protected:
    //! \reimp
    virtual QVariant itemChange(GraphicsItemChange change,
                                const QVariant &value);
    virtual bool sceneEventFilter(QGraphicsItem *watched, QEvent *event);
    //! \reimp_end
};

#endif // EXTENDEDKEYS_H
