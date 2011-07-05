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

#include "mimcorrectioncandidateitem.h"
#include <QTimer>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QTapAndHoldGesture>
#include <QPainter>
#include <QDebug>

#include <MTheme>

namespace {
    const int DefaultPressTimeout = 250;
    const int DefaultReleaseMissDelta = 30;
    const int DefaultLongTapTimeout = 600;
}

#include <mwidgetcreator.h>
M_REGISTER_WIDGET_NO_CREATE(MImCorrectionCandidateItem)

MImCorrectionCandidateItem::MImCorrectionCandidateItem(const QString &title, QGraphicsItem *parent)
    : MStylableWidget(parent),
      mSelected(false),
      mDown(false),
      mTitle(title),
      styleModeChangeTimer(),
      longTapTimer(),
      queuedStyleModeChange(false)
      
{
    styleModeChangeTimer.setSingleShot(true);
    connect(&styleModeChangeTimer, SIGNAL(timeout()), SLOT(applyQueuedStyleModeChange()));

    connect(this, SIGNAL(visibleChanged()),
            this, SLOT(handleVisibilityChanged()));

    setupLongTapTimer();
    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);
}

MImCorrectionCandidateItem::~MImCorrectionCandidateItem()
{
}

void MImCorrectionCandidateItem::setTitle(const QString &string)
{
    if (mTitle != string) {
        mTitle = string;
        update();
    }
}

QString MImCorrectionCandidateItem::title() const
{
    return mTitle;
}

void MImCorrectionCandidateItem::setSelected(bool select)
{
    mSelected = select;
    if (mSelected)
        style().setModeSelected();
    else 
        style().setModeDefault();
}

bool MImCorrectionCandidateItem::isSelected() const
{
    return mSelected;
}

void MImCorrectionCandidateItem::click()
{
    qDebug() << __PRETTY_FUNCTION__;
    emit clicked();
}

void MImCorrectionCandidateItem::longTap()
{
    qDebug() << __PRETTY_FUNCTION__;
    // Clear down state and update style.
    if (mDown) {
        mDown = false;
        updateStyleMode();
    }

    emit longTapped();
}

void MImCorrectionCandidateItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (mDown)
        return;
    
    style()->pressFeedback().play();
    mDown = true;
    updateStyleMode();
    longTapTimer.start();
}

void MImCorrectionCandidateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (!mDown) {
        return;
    }
    longTapTimer.stop();
    
    mDown = false;
    updateStyleMode();

    QPointF touch = event->scenePos();
    QRectF rect = sceneBoundingRect();
    int releaseMissDelta = style()->releaseMissDelta();
    releaseMissDelta = (releaseMissDelta > 0) ? releaseMissDelta : DefaultReleaseMissDelta;
    rect.adjust(-releaseMissDelta, -releaseMissDelta,
                releaseMissDelta, releaseMissDelta);
    bool pressed = rect.contains(touch);

    if (pressed) {
        style()->releaseFeedback().play();
        click();
    }
}

void MImCorrectionCandidateItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();

    QPointF touch = event->scenePos();
    QRectF rect = sceneBoundingRect();
    int releaseMissDelta = style()->releaseMissDelta();
    releaseMissDelta = (releaseMissDelta > 0) ? releaseMissDelta : DefaultReleaseMissDelta;
    rect.adjust(-releaseMissDelta, -releaseMissDelta,
                releaseMissDelta, releaseMissDelta);
    bool pressed = rect.contains(touch);

    if (pressed != mDown) {
        longTapTimer.stop();
        if (pressed) {
            style()->pressFeedback().play();
        } else {
            style()->cancelFeedback().play();
        }
        mDown = pressed;
        updateStyleMode();
    }
}


void MImCorrectionCandidateItem::updateStyleMode()
{
    if (mDown) {
        int pressTimeout = style()->pressTimeout();
        pressTimeout = (pressTimeout > 0) ? pressTimeout : DefaultPressTimeout;
        if (styleModeChangeTimer.isActive()) {
            styleModeChangeTimer.start(pressTimeout);
            return;
        }
        styleModeChangeTimer.start(pressTimeout);
        style().setModePressed();
    } else {
        if (isSelected()) {
            style().setModeSelected();
        } else {
            if (styleModeChangeTimer.isActive()) {
                queuedStyleModeChange = true;
                return;
            }
            style().setModeDefault();
        }
    }

    applyStyle();
    update();
}

void MImCorrectionCandidateItem::applyQueuedStyleModeChange()
{
    if (queuedStyleModeChange) {
        queuedStyleModeChange = false;
        updateStyleMode();
    }
}

void MImCorrectionCandidateItem::drawContents(QPainter *painter, const QStyleOptionGraphicsItem *option) const
{
    Q_UNUSED(option);
    if (!mTitle.isEmpty()) {
        painter->setFont(style()->font());
        painter->setPen(style()->fontColor());
        QSizeF s = size() - QSizeF(style()->marginLeft() + style()->marginRight(),
                                   style()->marginTop() + style()->marginBottom());
        painter->drawText(QRectF(0, 0, s.width(), s.height()), Qt::AlignCenter, mTitle); 
    }
}

qreal MImCorrectionCandidateItem::idealWidth() const
{
    qreal width = 0.0;
    if (!mTitle.isEmpty()) {
        QFontMetricsF fm(style()->font());
        width = fm.width(mTitle);
    }
    width += style()->marginLeft() + style()->marginRight()
             + style()->paddingRight() + style()->paddingLeft();
    return width;
}

void MImCorrectionCandidateItem::handleVisibilityChanged()
{
    //clear select and down state when hidden
    if (!isVisible()) {
        if (mDown || mSelected) {
            mSelected = false;
            mDown =false;
            updateStyleMode();
        }
    }
}

void MImCorrectionCandidateItem::setupLongTapTimer()
{
    longTapTimer.setSingleShot(true);
    int longTapTimeout = style()->longTapTimeout();
    longTapTimer.setInterval(longTapTimeout);
    connect(&longTapTimer, SIGNAL(timeout()), SLOT(longTap()), Qt::UniqueConnection);
}

void MImCorrectionCandidateItem::onThemeChangeCompleted()
{
    // reset long tap timer
    setupLongTapTimer();
}
