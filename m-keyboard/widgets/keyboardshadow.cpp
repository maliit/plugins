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

#include "keyboardshadow.h"

#ifdef HAVE_MEEGOTOUCH
#include <MWidgetModel>
#include <MScalableImage>
#else
#include "style-wrapper.h"
#endif

#include <QDebug>

KeyboardShadow::KeyboardShadow(QGraphicsWidget *parent)
    : MStylableWidget(parent)
{
    resize(size().width(), 0);
}

KeyboardShadow::~KeyboardShadow()
{
}


// This is mostly copied from MStylableWidget; only the drawing y offset and currentSize
// has been changed and some stylistic conventions.
void KeyboardShadow::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem */*option*/) const
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(painter)
#else
    const MWidgetStyle *s(static_cast<const MWidgetStyle*>(style().operator ->()));

    if (!s->backgroundTiles().isValid() && !s->backgroundImage() && !s->backgroundColor().isValid())
        return;

    const qreal oldOpacity(painter->opacity());
    painter->setOpacity(s->backgroundOpacity() * effectiveOpacity());
    const qreal backgroundY(static_cast<qreal>(-s->preferredSize().height()));

    const QSizeF currentSize(boundingRect().size());
    if (s->backgroundTiles().isValid()) {
        s->backgroundTiles()[model()->layoutPosition()]->draw(
            0.0, backgroundY, currentSize.width(), currentSize.height(), painter);
    } else if (s->backgroundImage()) {
        s->backgroundImage()->draw(0.0, backgroundY,
                                   currentSize.width(), currentSize.height(), painter);
    } else { //style background color must be valid
        painter->fillRect(QRectF(QPointF(0, backgroundY), currentSize),
                          QBrush(s->backgroundColor()));
    }
    painter->setOpacity(oldOpacity);
#endif
}


QRectF KeyboardShadow::boundingRect() const
{
#ifdef HAVE_MEEGOTOUCH
    const MWidgetStyle *s(static_cast<const MWidgetStyle*>(style().operator ->()));
    return QRectF(0, -s->preferredSize().height(),
                  s->preferredSize().width(), s->preferredSize().height());
#else
    static QRectF r;
    return r;
#endif
}

void KeyboardShadow::applyStyle()
{
    MStylableWidget::applyStyle();
    resize(size().width(), 0);
}

QSizeF KeyboardShadow::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    QSizeF result(MStylableWidget::sizeHint(which, constraint));
    result.setHeight(0.0);
    return result;
}
