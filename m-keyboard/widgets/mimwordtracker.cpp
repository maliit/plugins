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



#include "regiontracker.h"
#include "mimwordtracker.h"
#include "mimcorrectioncandidateitem.h"
#include "reactionmapwrapper.h"
#include "mplainwindow.h"
#include "mkeyboardhost.h"
#include "mimrootwidget.h"

#include <QGraphicsLinearLayout>
#include <QtGlobal>
#include <QDebug>

#include <mimsettings.h>

#ifdef HAVE_MEEGOTOUCH
#include <MScalableImage>
#include <MSceneManager>
#include <mwidgetcreator.h>
M_REGISTER_WIDGET_NO_CREATE(MImWordTracker)
#endif

namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSetting = "/meegotouch/inputmethods/multitouch/enabled";

    const char * const WordTrackerObjectName = "CorrectionWordTracker";
    const int DefaultShowHideFrames = 100;
    const int DefaultShowHideTime = 400;
    const int DefaultShowHideInterval = 30;

    bool isWordTrackerIntersectingKeyboard(const QRect &cursorRect,
                                           int wordTrackerHeight,
                                           int keyboardHeight)
    {
        const int wordTrackerBottom = cursorRect.bottom() + wordTrackerHeight;

        const int vkbTop(
#ifdef HAVE_MEEGOTOUCH
            MPlainWindow::instance()->sceneManager()->visibleSceneSize().height() - keyboardHeight
#else
            // FIXME: just an educated guess, for now.
            854 - keyboardHeight
#endif
        );

        return (wordTrackerBottom > vkbTop);
    };
};


MImWordTracker::MImWordTracker(QGraphicsWidget *container)
    : MStylableWidget(container),
      containerWidget(container),
      mIdealWidth(0),
      candidateItem(new MImCorrectionCandidateItem("", this)),
      pointerXOffset(0),
      uponCursor(false)
{
    containerWidget->setObjectName("WordTrackerContainer");
    RegionTracker::instance().addRegion(*containerWidget);

    // By default multi-touch is disabled
    setAcceptTouchEvents(MImSettings(MultitouchSetting).value().toBool());
    // Listen for visibility and position changes
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    setFlag(QGraphicsItem::ItemSendsScenePositionChanges, true);

    setObjectName(WordTrackerObjectName);

    mainLayout = new QGraphicsLinearLayout(Qt::Vertical);
    mainLayout->setSpacing(0);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);
    mainLayout->addItem(candidateItem);
    mainLayout->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    connect(candidateItem, SIGNAL(clicked()), this, SLOT(select()));
    connect(candidateItem, SIGNAL(longTapped()), this, SLOT(longTap()));

#ifdef HAVE_MEEGOTOUCH
    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);
#endif

    setupTimeLine();
    containerWidget->hide();
}

MImWordTracker::~MImWordTracker()
{
}

void MImWordTracker::setCandidate(const QString &string)
{
    mCandidate = string;
    if (isVisible()
        && showHideTimeline.state() == QTimeLine::Running
        && showHideTimeline.direction() == QTimeLine::Backward) {
        // don't update during hiding animation
        return;
    }
    candidateItem->setTitle(string);

    mIdealWidth = candidateItem->idealWidth();;
    // not less than minimum width
    if (mIdealWidth < minimumSize().width())
        mIdealWidth = minimumSize().width();

    mIdealWidth += style()->paddingLeft() + style()->paddingRight()
                   + style()->marginLeft() + style()->marginRight();
    setPreferredWidth(mIdealWidth);
}

QString MImWordTracker::candidate() const
{
    return mCandidate;
}

qreal MImWordTracker::idealWidth() const
{
    return mIdealWidth;
}

qreal MImWordTracker::pointerHeight() const
{
    const QSize pointerSize = style()->wordtrackerPointerSize();
    if (pointerSize.isEmpty())
        return 0;
    else
        return (pointerSize.height() - style()->wordtrackerPointerOverlap());
}

void MImWordTracker::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const
{
    MStylableWidget::drawBackground(painter, option);
    const QSize pointerSize = style()->wordtrackerPointerSize();
    if (style()->wordtrackerPointerImage() && !pointerSize.isEmpty()) {
        QRect rect = QRect(uponCursor ? (pointerXOffset + pointerSize.width()) : pointerXOffset,
                           uponCursor ? (size().height() - style()->wordtrackerPointerOverlap()
                                          + pointerSize.height())
                           : (style()->wordtrackerPointerOverlap() - pointerSize.height()),
                           pointerSize.width(),
                           pointerSize.height());

        if (uponCursor) {
            // rotate the pointer
            painter->save();
            QPointF widgetOrigin = QPointF(mapToScene(rect.x(), rect.y()));
            QTransform transform = QTransform::fromTranslate(widgetOrigin.x(), widgetOrigin.y());
            const int angle = (MKeyboardHost::instance()->rootWidget()->orientationAngle() + 180) % 360;
            transform.rotate(angle);
            painter->setTransform(transform, false);
            painter->drawPixmap(QRect(QPoint(0, 0), pointerSize), *style()->wordtrackerPointerImage()->pixmap());
            painter->restore();
        } else {
            style()->wordtrackerPointerImage()->draw(rect, painter);
        }
    }
}

QVariant MImWordTracker::itemChange(GraphicsItemChange change, const QVariant &value)
{
    // When the widget appears it generates visibility and position change event,
    // therefore the visibility event is ignored in this case.
    if ((change == QGraphicsItem::ItemVisibleChange && isVisible())
        || (change == QGraphicsItem::ItemScenePositionHasChanged))
        emit makeReactionMapDirty();

#ifdef HAVE_MEEGOTOUCH
    return MWidgetController::itemChange(change, value);
#else
    return QGraphicsWidget::itemChange(change, value);
#endif
}

void MImWordTracker::select()
{
    if (showHideTimeline.state() == QTimeLine::Running) {
        // Ignore select actions during animation.
        return;
    }
    if (!mCandidate.isEmpty()) {
        emit candidateClicked(mCandidate);
    }
}

void MImWordTracker::longTap()
{
    if (showHideTimeline.state() == QTimeLine::Running) {
        // Ignore select actions during animation.
        return;
    }
    if (!mCandidate.isEmpty()) {
        emit longTapped();
    }
}

void MImWordTracker::setupTimeLine()
{
    int showHideFrames = style()->showHideFrames();
    showHideFrames = (showHideFrames > 0) ? showHideFrames : DefaultShowHideFrames;
    int showHideTime = style()->showHideTime();
    showHideTime = (showHideTime > 0) ? showHideTime : DefaultShowHideTime;
    int showHideInterval = style()->showHideInterval();
    showHideInterval = (showHideInterval > 0) ? showHideInterval : DefaultShowHideInterval;

    showHideTimeline.setCurveShape(QTimeLine::EaseInCurve);
    showHideTimeline.setFrameRange(0, showHideFrames);
    showHideTimeline.setDuration(showHideTime);
    showHideTimeline.setUpdateInterval(showHideInterval);
    connect(&showHideTimeline, SIGNAL(frameChanged(int)), this, SLOT(fade(int)), Qt::UniqueConnection);
    connect(&showHideTimeline, SIGNAL(finished()), this, SLOT(showHideFinished()), Qt::UniqueConnection);
}

void MImWordTracker::fade(int frame)
{
    int showHideFrames = showHideTimeline.endFrame();
    showHideFrames = (showHideFrames > 0) ? showHideFrames : DefaultShowHideFrames;
    const qreal opacity = qreal(frame) / showHideFrames;
    parentWidget()->setOpacity(opacity);
    parentWidget()->update();
}


void MImWordTracker::showHideFinished()
{
    const bool hiding = (showHideTimeline.direction() == QTimeLine::Backward);

    if (hiding) {
        containerWidget->hide();
    }
}

void MImWordTracker::appear(bool withAnimation)
{
    if (!isVisible()) {
        if (withAnimation) {
            showHideTimeline.setDirection(QTimeLine::Forward);
            if (showHideTimeline.state() != QTimeLine::Running) {
                showHideTimeline.start();
            }
        }
        containerWidget->show();
        show();
    } else {
        containerWidget->update();
    }
}

void MImWordTracker::disappear(bool withAnimation)
{
    if (!isVisible())
        return;

    if (withAnimation) {
        showHideTimeline.setDirection(QTimeLine::Backward);
        if (showHideTimeline.state() != QTimeLine::Running) {
            showHideTimeline.start();
        }
    } else {
        containerWidget->hide();
    }
}

void MImWordTracker::setPosition(const QRect &cursorRect)
{
    if (cursorRect.isNull())
        return;

    const QSize pointerSize = style()->wordtrackerPointerSize();
    const int sceneWidth = MKeyboardHost::instance()->rootWidget()->geometry().width();
    QSizeF containerSize = preferredSize();
    containerSize.setHeight(containerSize.height() + pointerHeight());

    int keyboardHeight = MKeyboardHost::instance()->keyboardHeight();

    int wordTrackerHeight = containerSize.height() + style()->wordtrackerPointerTopMargin()
                            + style()->wordtrackerCursorYOffset();
    // check whether there is enough place to place word tracker under cursor.
    uponCursor = isWordTrackerIntersectingKeyboard(cursorRect,
                                                   wordTrackerHeight,
                                                   keyboardHeight);
    int containerPositionX = 0, containerPositionY = 0;

    containerPositionY
        = uponCursor ?
          (cursorRect.top() - style()->wordtrackerPointerTopMargin() - containerSize.height()) :
          cursorRect.bottom() + style()->wordtrackerPointerTopMargin() + style()->wordtrackerCursorYOffset();

    // TODO: limit the word tracker to visible widget part rather than to whole input area
    containerPositionY = qBound<int>(0,
                                     containerPositionY,
                                     MKeyboardHost::instance()->rootWidget()->geometry().height()
                                     - keyboardHeight
                                     - containerSize.height());

    containerPositionX = cursorRect.left() + cursorRect.width()/2
                         - style()->wordtrackerPointerLeftMargin() - pointerSize.width()/2
                         - style()->wordtrackerCursorXOffset();
    containerPositionX = qBound<int>(style()->wordtrackerLeftMargin(),
                         containerPositionX,
                         sceneWidth - mIdealWidth - style()->wordtrackerRightMargin());

    QRectF widgetGeometry, containerGeometry;
    containerGeometry = QRectF(QPoint(containerPositionX, containerPositionY), containerSize);

    widgetGeometry = uponCursor ? QRectF(QPointF(0, 0), preferredSize())
                     : QRectF(QPointF(0, pointerHeight()), preferredSize());

    if (!pointerSize.isEmpty()) {
        // pointerXOffset is the related x offset for the cursor from the left side of word tracker.
        pointerXOffset = cursorRect.left() + cursorRect.width()/2
                         - pointerSize.width()/2 - containerPositionX;
        pointerXOffset = qBound<qreal>(style()->wordtrackerPointerLeftMargin(),
                                       pointerXOffset,
                                       mIdealWidth - style()->wordtrackerPointerRightMargin()
                                       - pointerSize.width());
    }

    containerWidget->setGeometry(containerGeometry);
    setGeometry(widgetGeometry);

    if (isVisible()) {
        containerWidget->update();
    }
}

void MImWordTracker::onThemeChangeCompleted()
{
    // reset time line
    setupTimeLine();
}

void MImWordTracker::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    Q_UNUSED(view);
#else
    if (!isVisible())
        return;

    // Clear all with inactive color.
    reactionMap->setTransform(this, view);
    reactionMap->setInactiveDrawingValue();
    reactionMap->fillRectangle(geometry());

    // Draw the actual word tracker area.
    reactionMap->setDrawingValue(MImReactionMap::Press, MImReactionMap::Release);
    reactionMap->fillRectangle(geometry());
#endif // HAVE_REACTIONMAP
}
