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

#include "magnifier.h"
#include "magnifierhost.h"
#include "mkeyboardmagnifierstyle.h"

#ifdef HAVE_MEEGOTOUCH
#include <mtimestamp.h>
#include <MScalableImage>
#else
#include "style-types.h"
#define mTimestamp(x, y)
#endif

#include <QFont>
#include <QPainter>
#include <float.h>
#include <QAnimationGroup>

#include <QDebug>

Magnifier::Magnifier(MagnifierHost *newHost,
                     QGraphicsItem *parent)
    : MImOverlay()
    , host(newHost)
    , geometryParentItem(parent)
    , hideAnimation(this, "magnitude")
    , currentMagnitude(1)
{
    setParent(newHost);
    hide();
    hideAnimation.setStartValue(qreal(1.0f));
    hideAnimation.setEndValue(qreal(0.0f));
}

Magnifier::~Magnifier()
{}

void Magnifier::setup()
{
    //TODO: should update below values when style update.
    topLeft = QPointF(-host->style()->magnifierSize().width() / 2,
                      -host->style()->magnifierSize().height());

    textArea = QRectF(topLeft.x(),
                      (topLeft.y() + host->style()->magnifierTextMarginTop()),
                      host->style()->magnifierSize().width(),
                      (host->style()->magnifierSize().height()
                       - host->style()->magnifierTextMarginTop()
                       - host->style()->magnifierTextMarginBottom()));

    safetyMargins = QMargins(host->style()->safetyMarginLeft(),  MagnifierHost::InvalidMargin,
                             host->style()->safetyMarginRight(), MagnifierHost::InvalidMargin);

    hideAnimation.setDuration(host->style()->magnifierHideDuration());
    hideAnimation.setEasingCurve(host->style()->magnifierHideEasingCurve());
}

void Magnifier::paint(QPainter *painter,
                      const QStyleOptionGraphicsItem *,
                      QWidget *)
{
    mTimestamp("Magnifier", "start");

    const MKeyboardMagnifierStyleContainer &style = host->style();
    QFont font = style->magnifierFont();
    font.setPixelSize(style->magnifierFontSize());

    QRectF drawingArea(textArea);
    if (!label.isEmpty() && label.at(0).isLower()) {
        drawingArea.adjust(0, 0, 0, -style->magnifierTextMarginBottomLowercase() * 2);
    }

    painter->setFont(font);
    painter->setPen(style->magnifierTextColor());

    const MScalableImage *magnifierImage = host->style()->magnifierImage();
    if (magnifierImage) {
        magnifierImage->draw(topLeft.toPoint(), host->style()->magnifierSize(), painter);
    }
    painter->drawText(drawingArea, Qt::AlignCenter, label);

    mTimestamp("Magnifier", "end");
}

QRectF Magnifier::boundingRect() const
{
    return QRectF(topLeft, host->style()->magnifierSize());
}

void Magnifier::updatePos(const QPointF &keyPos,
                          const QPoint &,
                          const QSize &keySize)
{
    // Because of pos() == boundingRect().center() we need to add the button
    // overlapping offset to the half-height of the magnifier itself.
    const QPointF newPos(keyPos + QPointF(keySize.width() / 2,
                                          (boundingRect().height() / 2)
                                          + host->style()->magnifierButtonOverlap()));

    setMagnitude(hideAnimation.startValue().value<qreal>());
    MagnifierHost::applyConstrainedPosition(this, geometryParentItem, newPos, safetyMargins,
                                            MagnifierHost::UseGraphicsViewItemOrigins);
}

void Magnifier::setLabel(const QString &newLabel)
{
    if (label == newLabel) {
        return;
    }

    label = newLabel;
    update();
}

qreal Magnifier::magnitude() const
{
    return currentMagnitude;
}

void Magnifier::setMagnitude(qreal value)
{
    if (value < 0.0f) {
        return;
    }

    currentMagnitude = value;
    QTransform t;
    const qreal height = host->style()->magnifierSize().height();
    t.translate(0, -height / 2.0f * (1.0f - value));
    t.scale(value, value);
    setTransform(t);
}

void Magnifier::showMagnifier()
{
    hideAnimation.stop();
    setMagnitude(hideAnimation.startValue().value<qreal>());
    show();
}

void Magnifier::addToGroup(QAnimationGroup *group)
{
    group->addAnimation(&hideAnimation);
}

