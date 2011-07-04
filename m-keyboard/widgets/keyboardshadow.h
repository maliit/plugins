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

#ifndef KEYBOARDSHADOW_H
#define KEYBOARDSHADOW_H

#include "keyboardshadowstyle.h"

#include "style-wrapper.h"

/*!
  \brief Dummy widget for drawing keyboard shadow on top of the keyboard and over the
  possibly visible toolbar

  In reality the widget has zero height but it nevertheless draws background based on
  style's preferred size; this way it can be put to a vertical layout with the toolbar and
  it will go over the toolbar.

  The background is drawn into negative y direction; that is, bounding rect extends to
  -(preferred size.heigth).
*/
class KeyboardShadow : public MStylableWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     * \param parent Parent object.
     */
    explicit KeyboardShadow(QGraphicsWidget *parent = 0);

    //! Destructor
    virtual ~KeyboardShadow();

    //! \reimp
    void drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
    QRectF boundingRect() const;
    void applyStyle();
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    //! \reimp_end

private:
    M_STYLABLE_WIDGET(KeyboardShadowStyle)
};

#endif
