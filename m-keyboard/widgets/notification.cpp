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

namespace
{
    const int FPS = 20; // Frames per second
}

#include "mvirtualkeyboardstyle.h"
#include "notification.h"

#include <QDebug>
#include <QFontMetrics>
#include <QPainter>

#ifdef HAVE_MEEGOTOUCH
#include <MSceneManager>
#include <MScalableImage>
#endif
#include <float.h>


Notification::Notification(QGraphicsWidget *parent)
    : MStylableWidget(parent),
      opacity(0),
      frameCount(1)
{
    // Notification sets its own absolute opacity
    setFlag(ItemIgnoresParentOpacity, true);
    setZValue(FLT_MAX);

    visibilityTimer.setSingleShot(true);

    connect(&fadeTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(updateOpacity(int)));
    connect(&fadeTimeLine, SIGNAL(finished()),
            this, SLOT(fadingFinished()));

    connect(&visibilityTimer, SIGNAL(timeout()),
            this, SLOT(fadeOut()));

    hide();
}


Notification::~Notification()
{
}


void
Notification::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    painter->setFont(font);
    painter->setRenderHint(QPainter::Antialiasing);

    const MScalableImage *backgroundImage = style()->backgroundImage();

    const qreal oldOpacity = painter->opacity();
    painter->setOpacity(oldOpacity * style()->backgroundOpacity());
    if (backgroundImage) {
        backgroundImage->draw(boundingRect().toRect(), painter);
    } else {
        painter->setPen(border);
        painter->setBrush(background);
        const int rounding = style()->rounding();
        painter->drawRoundedRect(rect(), rounding, rounding);
    }
    painter->setOpacity(oldOpacity);

    painter->setPen(textColor);
    // Draw the normalized message
    QRectF textRect = rect();
    textRect.adjust(style()->paddingLeft(),
                    style()->paddingTop(),
                    -style()->paddingRight(),
                    -style()->paddingBottom());
    painter->drawText(textRect, Qt::AlignCenter | Qt::TextWordWrap, message);
}


void
Notification::displayText(const QString &msg, const QRectF &area)
{
    // Normalize the message. Use line breaks if the message
    // does not fit into the screen and set the geometry
    setMessageAndGeometry(msg, area);
    // Start to fade in
    fadeIn();
}

void Notification::applyStyle()
{
    font = style()->font();
    font.setPixelSize(style()->fontSize());
    border = style()->borderColor();
    background = style()->backgroundColor();
    textColor = style()->textColor();
    opacity = style()->opacity();
    fadeTimeLine.setDuration(style()->fadeTime());
    visibilityTimer.setInterval(style()->holdTime());
    frameCount = style()->fadeTime() / FPS;
    fadeTimeLine.setFrameRange(0, frameCount);
}


void
Notification::updateOpacity(int frameNumber)
{
    setOpacity(qreal(frameNumber) / qreal(frameCount) * opacity);
    update();
}


void
Notification::fadingFinished()
{
    if (fadeTimeLine.direction() == QTimeLine::Forward) {
        // Notification is now shown an will be visible
        // as long as visibility timer runs.
        visibilityTimer.start();
    } else {
        // We have faded out
        hide();
    }
}


void Notification::fadeOut()
{
    fadeTimeLine.setDirection(QTimeLine::Backward);
    fadeTimeLine.start();
}


void Notification::fadeIn()
{
    // Handle fadeIn request in four different states:
    // not visible, fading in, visible, fading out

    if (fadeTimeLine.state() == QTimeLine::NotRunning) {
        if (isVisible()) {
            // Already visible so just prolong the visibility time
            // by restarting timer.
            visibilityTimer.start();
        } else {
            // The common case, begin fading in from hidden state.
            setOpacity(0.0);
            show();

            // Begin fading in
            fadeTimeLine.setDirection(QTimeLine::Forward);
            fadeTimeLine.start();
        }
    } else {
        // Fading in progress

        if (fadeTimeLine.direction() == QTimeLine::Forward) {
            // New fade in requested while fading in.
            // Do nothing, i.e. continue ongoing fading.
        } else {
            fadeTimeLine.toggleDirection();
        }
    }
}

void Notification::setMessageAndGeometry(const QString &msg, const QRectF &area)
{
    message = msg;
    setGeometry(QRectF(0, 0, area.width(), area.height()));
}

