/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */

#include "mimcorrectioncandidateitem.h"
#include <QTimer>
#include <QFontMetricsF>
#include <QGraphicsSceneMouseEvent>
#include <QTapAndHoldGesture>
#include <QDebug>

namespace {
    const int MPressTimeout = 250;
    const int MReleaseMissDelta = 30;
}

#include <mwidgetcreator.h>
M_REGISTER_WIDGET_NO_CREATE(MImCorrectionCandidateItem)

MImCorrectionCandidateItem::MImCorrectionCandidateItem(const QString &title, QGraphicsItem *parent)
    : MStylableWidget(parent),
      mSelected(false),
      mDown(false),
      mTitle(title),
      styleModeChangeTimer(new QTimer(this)),
      queuedStyleModeChange(false)
      
{
    styleModeChangeTimer->setSingleShot(true);
    connect(styleModeChangeTimer, SIGNAL(timeout()), SLOT(applyQueuedStyleModeChange()));
}

MImCorrectionCandidateItem::~MImCorrectionCandidateItem()
{
}

void MImCorrectionCandidateItem::setTitle(const QString &string)
{
    mTitle = string;
    update();
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
    mDown = false;
    updateStyleMode();

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
}

void MImCorrectionCandidateItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    if (!mDown) {
        return;
    }
    
    mDown = false;
    updateStyleMode();

    QPointF touch = event->scenePos();
    QRectF rect = sceneBoundingRect();
    rect.adjust(-MReleaseMissDelta, -MReleaseMissDelta,
                MReleaseMissDelta, MReleaseMissDelta);
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
    rect.adjust(-MReleaseMissDelta, -MReleaseMissDelta,
                MReleaseMissDelta, MReleaseMissDelta);
    bool pressed = rect.contains(touch);

    if (pressed != mDown) {
        if (pressed) {
            style()->pressFeedback().play();
        } else {
            style()->cancelFeedback().play();
        }
        mDown = pressed;
        updateStyleMode();
    }
}

void MImCorrectionCandidateItem::tapAndHoldGestureEvent(QGestureEvent *event, QTapAndHoldGesture *gesture)
{
    if (gesture->state() == Qt::GestureFinished)
        longTap();

    event->accept(gesture);
}


void MImCorrectionCandidateItem::updateStyleMode()
{
    if (mDown) {
        if (styleModeChangeTimer->isActive()) {
            styleModeChangeTimer->start(MPressTimeout);
            return;
        }
        styleModeChangeTimer->start(MPressTimeout);
        style().setModePressed();
    } else {
        if (isSelected()) {
            style().setModeSelected();
        } else {
            if (styleModeChangeTimer->isActive()) {
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
        QFontMetricsF fm(QFont(style()->font().family(), style()->font().pixelSize()));
        width = fm.width(mTitle);
    }
    width += style()->marginLeft() + style()->marginRight()
             + style()->paddingRight() + style()->paddingLeft();
    return width;
}

void MImCorrectionCandidateItem::connectNotify(const char *signal)
{
    if (QLatin1String(signal) == SIGNAL(longTapped())) {
        updateLongTapConnections();
    }
}

void MImCorrectionCandidateItem::disconnectNotify(const char *signal)
{
    if (QLatin1String(signal) == SIGNAL(longTapped())) {
        updateLongTapConnections();
    }
}

void MImCorrectionCandidateItem::updateLongTapConnections()
{
    if (receivers(SIGNAL(longTapped())) > 0)
        grabGesture(Qt::TapAndHoldGesture);
    else
        ungrabGesture(Qt::TapAndHoldGesture);
}

