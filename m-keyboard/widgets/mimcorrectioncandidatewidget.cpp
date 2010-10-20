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



#include "mimcorrectioncandidatewidget.h"
#include "mimcorrectioncandidateitem.h"

#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QDebug>
#include <QString>

#include <MSceneManager>
#include <mreactionmap.h>
#include <mplainwindow.h>
#include <MGConfItem>
#include <MScalableImage>
#include <MTheme>

#include <mwidgetcreator.h>
M_REGISTER_WIDGET_NO_CREATE(MImCorrectionCandidateWidget)

namespace
{
    const int CandidatesPreeditGap = 10; // Gap between pre-edit and candidates
    const int MinimumCandidateWidget = 64; // Minimum length of a candidate.
    const char * const CandidatesListObjectName = "CorrectionCandidateList";
    const char * const CandidatesItemObjectName = "CorrectionCandidateItem";
    const char * const CandidatesItemLabelObjectName = "CorrectionCandidateItemTitle";

    const int ShowHideFrames = 100;
    const int ShowHideTime = 400;
    const int ShowHideInterval = 10;

};


MImCorrectionCandidateContainer::MImCorrectionCandidateContainer(QGraphicsItem *parent)
    : MStylableWidget(parent),
      width(0)
{
}

void MImCorrectionCandidateContainer::setCandidateMaximumWidth(qreal w)
{
    width = w + style()->paddingLeft() + style()->paddingRight()
            + style()->marginLeft() + style()->marginRight();
}

qreal MImCorrectionCandidateContainer::idealWidth() const
{
    return width;
}

int MImCorrectionCandidateContainer::pointerHeight() const
{
    return (style()->wordtrackerPointerSize().height() - style()->wordtrackerPointerOverlap());
}

void MImCorrectionCandidateContainer::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const
{
    MStylableWidget::drawBackground(painter, option);
    if (style()->wordtrackerPointerImage()) {
         const QSize pointerSize = style()->wordtrackerPointerSize();
         QRect rect = QRect((idealWidth() - pointerSize.width())/2,
                            style()->wordtrackerPointerOverlap() - pointerSize.height(),
                            pointerSize.width(),
                            pointerSize.height());
         style()->wordtrackerPointerImage()->draw(rect, painter);
    }
}

MImCorrectionCandidateWidget::MImCorrectionCandidateWidget()
    : MImOverlay(),
      rotationInProgress(false),
      candidatePosition(0, 0),
      sceneManager(MPlainWindow::instance()->sceneManager()),
      containerWidget(new MImCorrectionCandidateContainer(this)),
      currentMode(MImCorrectionCandidateWidget::PopupMode)
{
    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    containerWidget->setLayout(mainLayout);
    for (int i = 0; i < MaxCandidateCount; i++) {
        candidateItems[i] = new MImCorrectionCandidateItem("", containerWidget); 
        candidateItems[i]->setVisible(false);
        connect(candidateItems[i], SIGNAL(clicked()), this, SLOT(select()));
    }

    setupTimeLine();
}


MImCorrectionCandidateWidget::~MImCorrectionCandidateWidget()
{
}

void MImCorrectionCandidateWidget::setCandidates(const QStringList candidateList)
{
    candidates.clear();
    candidates = candidateList.mid(0, MaxCandidateCount);
    suggestionString.clear();
    if (candidates.isEmpty()) {
        return;
    }
    suggestionString = candidates.at(0);

    // Calculate the width for MContentItem dynamically.
    // To ensure the word tracker could be shown.
    qreal width = 0;
    MImCorrectionCandidateItem candidateItem;
    if (candidates.at(0) == m_preeditString && candidates.count() > 1) {
        candidateItem.setTitle(candidates.at(1));
    } else {
        candidateItem.setTitle(candidates.at(0));
    }

    width = candidateItem.idealWidth();;
    // not less than minimum width
    if (width < containerWidget->minimumSize().width())
        width = containerWidget->minimumSize().width();
    containerWidget->setCandidateMaximumWidth(width);
}

void MImCorrectionCandidateWidget::setPreeditString(const QString &string)
{
    m_preeditString = string;
}

QPoint MImCorrectionCandidateWidget::position() const
{
    return candidatePosition;
}

QString MImCorrectionCandidateWidget::preeditString() const
{
    return m_preeditString;
}

void MImCorrectionCandidateWidget::setPosition(const QPoint &position)
{
    candidatePosition = position;

    int sceneWidth = sceneManager->visibleSceneSize().width();
    candidatePosition.setX(qBound(0, candidatePosition.x(), (int)(sceneWidth - containerWidget->idealWidth())));
}

void MImCorrectionCandidateWidget::setPosition(const QRect &preeditRect)
{
    if (preeditRect.isNull() || !preeditRect.isValid()) {
        candidatePosition = QPoint(0, 0);
        return;
    }

    QPoint position;
    int sceneWidth = sceneManager->visibleSceneSize().width();

    // Set horizontal position
    if (preeditRect.right() + MinimumCandidateWidget < sceneWidth) {
        // the right side correction widget is aligned with the right side of
        // pre-edit rectangle + MinimumCandidateWidget
        position.setX(preeditRect.right() + MinimumCandidateWidget - containerWidget->idealWidth());
    } else {
        // Align to left side of scene rect
        position.setX(sceneWidth - containerWidget->idealWidth());
    }

    // Set vertical position
    // Vertically the candidatesWidget is below the pre-edit + CandidatesPreeditGap.
    position.setY(preeditRect.bottom() + CandidatesPreeditGap);

    // Records the position.
    candidatePosition = position;
}

void MImCorrectionCandidateWidget::showWidget(MImCorrectionCandidateWidget::CandidateMode mode)
{
    currentMode = mode;

    if (candidates.isEmpty()) {
        hideWidget(false);
        return;
    }

    if (currentMode == PopupMode) {
        suggestionString = candidates.at(0);
        if (suggestionString == m_preeditString && candidates.count() > 1) {
            suggestionString = candidates.at(1);
        }
        mainLayout->addItem(candidateItems[0]);
        candidateItems[0]->setTitle(suggestionString);
        candidateItems[0]->setSelected(false);
        candidateItems[0]->setVisible(true);
        connect(candidateItems[0], SIGNAL(longTapped()), this, SLOT(longTap()));
        for (int i = 1; i < MaxCandidateCount; i++) {
            mainLayout->removeItem(candidateItems[i]);
            candidateItems[i]->setVisible(false);
        }
    } else {
        disconnect(candidateItems[0], SIGNAL(longTapped()), this, SLOT(longTap()));
        for (int i = 0; i < candidates.count(); i++) {
            candidateItems[i]->setSelected(false);
            mainLayout->addItem(candidateItems[i]);
            candidateItems[i]->setTitle(candidates.at(i));
            candidateItems[i]->setVisible(true);
        }
        int selectedIndex = candidates.indexOf(m_preeditString);
        candidateItems[selectedIndex]->setSelected(true);
    }
    

    if (currentMode == SuggestionListMode) {
        // Re-calculate the width for suggestion list.
        // To ensure all words in candidate list could be shown.
        qreal width = 0;
        for (int i = 0; i < candidates.count(); i++) {
            width = qMax(width, candidateItems[i]->idealWidth());
        }
        qDebug() << "preffered with:" << width;

        // not less than minimum width
        if (width < containerWidget->minimumSize().width())
            width = containerWidget->minimumSize().width();

        qDebug() << "set candidate width:" << containerWidget->idealWidth();
        containerWidget->setCandidateMaximumWidth(width);
        int sceneWidth = sceneManager->visibleSceneSize().width();
        candidatePosition.setX(qBound(0, candidatePosition.x(), (int)(sceneWidth - containerWidget->idealWidth())));
    }
    qDebug() << "fitted width:" << containerWidget->idealWidth();

    // So set the container widget's width to idealWidth,
    // to make it have the enough width to show whole words.
    containerWidget->setPreferredWidth(containerWidget->idealWidth());

    QRectF widgetRect, containerRect;

    if (currentMode == PopupMode) {
        // PopupMode only require its own geometry.
        QSizeF size = containerWidget->preferredSize();
        size.setHeight(size.height() + containerWidget->pointerHeight());
        widgetRect = QRectF(candidatePosition, size);
        containerRect = QRectF(QPointF(0, containerWidget->pointerHeight()),
                               containerWidget->preferredSize());
    } else {
        // SuggestionListMode require a modal window, thus require whole screen
        widgetRect = QRectF(0, 0, sceneManager->visibleSceneSize().width(),
                            sceneManager->visibleSceneSize().height());

        QPoint position = candidatePosition;
        position.setY(position.y() + containerWidget->pointerHeight());
        containerRect = QRectF(position, containerWidget->preferredSize());
    }
    setGeometry(widgetRect);
    containerWidget->setGeometry(containerRect);
    mainLayout->invalidate();

    if (!isVisible()) {
        showHideTimeline.setDirection(QTimeLine::Forward);
        if (showHideTimeline.state() != QTimeLine::Running) {
            showHideTimeline.start();
        }
        show();
    } else {
        update();
        emit regionUpdated(region());
    }
}

void MImCorrectionCandidateWidget::hideWidget(bool withAnimation)
{
    if (withAnimation) {
        showHideTimeline.setDirection(QTimeLine::Backward);
        if (showHideTimeline.state() != QTimeLine::Running) {
            showHideTimeline.start();
        }
    } else {
        hide();
    }
}

MImCorrectionCandidateWidget::CandidateMode MImCorrectionCandidateWidget::candidateMode() const
{
    return currentMode;
}

void MImCorrectionCandidateWidget::hideEvent(QHideEvent *event)
{
    MWidget::hideEvent(event);
    emit regionUpdated(QRegion());
}

QString MImCorrectionCandidateWidget::suggestion() const
{
    return suggestionString;
}

void MImCorrectionCandidateWidget::prepareToOrientationChange()
{
    if (isVisible()) {
        rotationInProgress = true;
        hideWidget();
    }
}

void MImCorrectionCandidateWidget::finalizeOrientationChange()
{
    if (rotationInProgress) {
        showWidget();
        rotationInProgress = false;
    }
}

void MImCorrectionCandidateWidget::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
    // Clear all with inactive color.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(QTransform());
    reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());

    // Draw the actual candidate candidatesWidget area.
    reactionMap->setTransform(this, view);
    reactionMap->setReactiveDrawingValue();
    //reactionMap->fillRectangle(candidatesWidget->geometry());
    reactionMap->fillRectangle(containerWidget->geometry());
}

bool MImCorrectionCandidateWidget::sceneEvent(QEvent *e)
{
    qDebug() << __PRETTY_FUNCTION__;

    if (e->type() == QEvent::GraphicsSceneMouseRelease) {
        hideWidget();
    } else if (e->type() == QEvent::GraphicsSceneMouseMove) {
        qDebug() << "mouse move!";
        int selectedIndex = candidates.indexOf(m_preeditString);
        candidateItems[selectedIndex]->setSelected(true);
    }
    return MImOverlay::sceneEvent(e);
}

QRegion MImCorrectionCandidateWidget::region() const
{
    if (!isVisible())
        return QRegion();

    if (currentMode == SuggestionListMode) {
        return mapRectToScene(QRect(QPoint(0, 0), sceneManager->visibleSceneSize())).toRect();
    } else {
        return mapRectToScene(containerWidget->geometry()).toRect();
    }
}

void MImCorrectionCandidateWidget::setupTimeLine()
{
    showHideTimeline.setCurveShape(QTimeLine::EaseInCurve);
    showHideTimeline.setFrameRange(0, ShowHideFrames);
    showHideTimeline.setDuration(ShowHideTime);
    showHideTimeline.setUpdateInterval(ShowHideInterval);
    connect(&showHideTimeline, SIGNAL(frameChanged(int)), this, SLOT(fade(int)));
    connect(&showHideTimeline, SIGNAL(finished()), this, SLOT(showHideFinished()));
}

void MImCorrectionCandidateWidget::fade(int frame)
{
    const float opacity = float(frame) / ShowHideFrames;
    this->setOpacity(opacity);
    update();
}


void MImCorrectionCandidateWidget::showHideFinished()
{
    qDebug() << __PRETTY_FUNCTION__;
    const bool hiding = (showHideTimeline.direction() == QTimeLine::Backward);

    if (hiding) {
        hide();
    }
}

void MImCorrectionCandidateWidget::handleVisibilityChanged()
{
    if (!isVisible()) {
        emit regionUpdated(QRegion());
    } else {
        // Extend overlay window to whole screen area.
        emit regionUpdated(region());
    }
}

void MImCorrectionCandidateWidget::handleOrientationChanged()
{
    //do nothing
}

void MImCorrectionCandidateWidget::select()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (showHideTimeline.state() == QTimeLine::Running) {
        // Ignore select actions during animation.
        return;
    }

    MImCorrectionCandidateItem *item = qobject_cast<MImCorrectionCandidateItem *> (sender());
    if (item) {
        for (int i = 0; i < candidates.count(); i++) {
            if (candidateItems[i] != item)
                candidateItems[i]->setSelected(false);
        }
        item->setSelected(true);
        const QString candidate = item->title();
        suggestionString = candidate;
        emit candidateClicked(candidate);
    }
    hideWidget();
}

void MImCorrectionCandidateWidget::longTap()
{
    qDebug() << __PRETTY_FUNCTION__;
    if (!isVisible())
        return;

    showWidget(SuggestionListMode);
}
