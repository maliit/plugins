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


#include "horizontalswitcher.h"
#include "mimabstractkey.h"
#include "mimabstractkeyarea.h"

#include <QGraphicsSceneResizeEvent>
#include <QGraphicsScene>
#include <QDebug>

namespace
{
    const int SwitchDuration = 500;
    const int SwitchFrames = 300;


    void setHorizontalFlickRecognition(QGraphicsWidget *widget, bool recognition)
    {
        MImAbstractKeyArea * const keyArea = qobject_cast<MImAbstractKeyArea *>(widget);

        if (keyArea) {
            keyArea->enableHorizontalFlick(recognition);
        }
    }
}

HorizontalSwitcher::HorizontalSwitcher(QGraphicsItem *parent) :
    QGraphicsWidget(parent),
    currentIndex(-1),
    animTimeLine(SwitchDuration),
    loopingEnabled(false),
    playAnimations(true),
    m_enableSinglePageFlick(true)
{
    setFlag(QGraphicsItem::ItemHasNoContents); // doesn't paint itself anything
    setObjectName("HorizontalSwitcher");

    animTimeLine.setFrameRange(0, SwitchFrames);

    enterAnim.setTimeLine(&animTimeLine);
    leaveAnim.setTimeLine(&animTimeLine);

    connect(&animTimeLine, SIGNAL(finished()), this, SLOT(finishAnimation()));
}

HorizontalSwitcher::~HorizontalSwitcher()
{
    if (isRunning())
        finishAnimation();

    // Delete all widgets that were not removed with removeWidget().
    qDeleteAll(slides);
    slides.clear();
}

void HorizontalSwitcher::switchTo(SwitchDirection direction)
{
    if (isRunning()) {
        finishAnimation();
    }

    if (slides.count() < 2 ||
        (!loopingEnabled && isAtBoundary(direction))) {
        return;
    }

    int newIndex = (direction == Left ? (currentIndex - 1)
                                      : (currentIndex + 1) % slides.count());

    if (newIndex < 0) {
        newIndex += slides.count();
    }

    QGraphicsWidget *currentWidget = slides.at(currentIndex);
    QGraphicsWidget *nextWidget = slides.at(newIndex);

    // Current item is about to leave
    leaveAnim.setItem(currentWidget);
    currentWidget->setEnabled(false);

    // New item is about to enter
    enterAnim.setItem(nextWidget);
    nextWidget->setEnabled(false);

    // reset current and next key area
    MImAbstractKeyArea *const currentKeyArea = dynamic_cast<MImAbstractKeyArea *>(currentWidget);
    if (currentKeyArea) {
        currentKeyArea->resetActiveKeys();
    }

    MImAbstractKeyArea *const nextKeyArea = dynamic_cast<MImAbstractKeyArea *>(nextWidget);
    if (nextKeyArea) {
        nextKeyArea->resetActiveKeys();
    }

    // Try to fit current size.
    nextWidget->resize(size());

    currentIndex = newIndex;
    emit switchStarting(currentIndex, newIndex);
    emit switchStarting(currentWidget, nextWidget);

    if (!playAnimations) {
        nextWidget->setPos(0.0, 0.0);
        nextWidget->show();
        finishAnimation();
    } else {
        nextWidget->setPos((direction == Right ? size().width()
                                               : -(nextWidget->size().width())), 0.0);
        enterAnim.setPosAt(0.0, nextWidget->pos());
        enterAnim.setPosAt(1.0, QPointF(0.0, 0.0));
        leaveAnim.setPosAt(0.0, currentWidget->pos());
        leaveAnim.setPosAt(1.0, QPointF((direction == Right ? -(currentWidget->size().width())
                                                            : size().width()), 0.0));

        // Enable painting background in black during playing animations.
        setFlag(QGraphicsItem::ItemHasNoContents, false);

        nextWidget->show();
        animTimeLine.start();
    }
}

bool HorizontalSwitcher::isAtBoundary(SwitchDirection direction) const
{
    return (currentIndex == (direction == Left ? 0
                                               : slides.count() - 1));
}

void HorizontalSwitcher::setCurrent(QGraphicsWidget *widget)
{
    if (!widget || !slides.contains(widget)) {
        qWarning() << "HorizontalSwitcher::setCurrent() - "
                   << "Cannot set switcher to specified widget. Add widget to switcher first?";
        return;
    }

    setCurrent(slides.indexOf(widget));
}

void HorizontalSwitcher::setCurrent(int index)
{
    if (isValidIndex(index) && index != currentIndex) {
        int oldIndex = -1;
        QGraphicsWidget *old = 0;

        if (isValidIndex(currentIndex)) {
            oldIndex = currentIndex;
            old = slides.at(currentIndex);
        }

        currentIndex = index;

        QGraphicsWidget *widget = slides.at(index);
        widget->setPos(0, 0);
        widget->resize(size());
        widget->show();

        // Ultimately might lead to a reaction map update in MKeyboardHost,
        // has no other purpose:
        emit switchDone(old, widget);

        updateGeometry();

        if (old) {
            old->hide();

            MImAbstractKeyArea *const keyArea = dynamic_cast<MImAbstractKeyArea *>(old);
            if (keyArea) {
                keyArea->modifiersChanged(false);
                keyArea->resetActiveKeys();
            }
        }
    }
}

int HorizontalSwitcher::current() const
{
    return (slides.isEmpty() ? -1 : currentIndex);
}

QGraphicsWidget *HorizontalSwitcher::currentWidget() const
{
    return (current() < 0 ? 0
                          : slides.at(currentIndex));
}

QGraphicsWidget *HorizontalSwitcher::widget(int index)
{
    return (isValidIndex(index) ? slides.at(index)
                                : 0);
}

int HorizontalSwitcher::count() const
{
    return slides.count();
}

bool HorizontalSwitcher::isRunning() const
{
    return (animTimeLine.state() == QTimeLine::Running);
}

void HorizontalSwitcher::setLooping(bool enable)
{
    loopingEnabled = enable;
}

void HorizontalSwitcher::setDuration(int ms)
{
    animTimeLine.setDuration(ms);
    animTimeLine.setFrameRange(0, ms * SwitchFrames / qMax(1, SwitchDuration));
}

void HorizontalSwitcher::setEasingCurve(const QEasingCurve &curve)
{
    animTimeLine.setEasingCurve(curve);
}

void HorizontalSwitcher::addWidget(QGraphicsWidget *widget)
{
    if (!widget) {
        return;
    }

    widget->setParentItem(this);
    widget->setPreferredWidth(size().width());

    slides.append(widget);
    widget->hide();

    // HS was empty before, this was the first widget added:
    if (slides.size() == 1) {
        setCurrent(0);
    }

    switch(count()) {
    case 1:
        setHorizontalFlickRecognition(widget, m_enableSinglePageFlick);
        break;
    case 2:
        setHorizontalFlickRecognition(slides.at(0), true);
        setHorizontalFlickRecognition(slides.at(1), true);
        break;
    default:
        setHorizontalFlickRecognition(widget, true);
        break;
    }
}

void HorizontalSwitcher::removeAll()
{
    foreach(QGraphicsWidget * slide, slides) {
        slide->setParentItem(0);

        if (slide->scene()) {
            slide->scene()->removeItem(slide);
        }
    }

    slides.clear();
    currentIndex = -1;
    updateGeometry();
}


void HorizontalSwitcher::deleteAll()
{
    qDeleteAll(slides);
    slides.clear();
    currentIndex = -1;
    updateGeometry();
}


void HorizontalSwitcher::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    QGraphicsWidget *widget = currentWidget();
    if (widget) {
        widget->resize(event->newSize());
    }
}


QSizeF HorizontalSwitcher::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    // return the size hint of the currently visible widget
    QGraphicsWidget *widget = currentWidget();
    QSizeF hint;

    if (widget) {
        hint = widget->effectiveSizeHint(which, constraint);
    } else {
        hint = QGraphicsWidget::sizeHint(which, constraint);
    }

    return hint;
}

void HorizontalSwitcher::finishAnimation()
{
    int oldIndex = -1;

    // Hide old item
    QGraphicsWidget *old = static_cast<QGraphicsWidget *>(leaveAnim.item());
    if (old) {
        oldIndex = slides.indexOf(old);
        old->setEnabled(true);
        old->hide();
    }

    // Clear transformations
    leaveAnim.clear();
    enterAnim.clear();

    animTimeLine.stop();
    // Restore original painting flag.
    setFlag(QGraphicsItem::ItemHasNoContents);

    // Discard cached sizehint info before telling that the switch is done.
    updateGeometry();

    if (currentWidget()) {
        currentWidget()->setEnabled(true);
    }

    emit switchDone(oldIndex, currentIndex);
    emit switchDone(old, slides.at(currentIndex));
}

bool HorizontalSwitcher::isValidIndex(int index) const
{
    return (index >= 0 && index < slides.size());
}

bool HorizontalSwitcher::isAnimationEnabled() const
{
    return playAnimations;
}

void HorizontalSwitcher::setAnimationEnabled(bool enabled)
{
    if (playAnimations != enabled) {
        if (isRunning())
            finishAnimation();

        playAnimations = enabled;
    }
}

void HorizontalSwitcher::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    for (int i = 0; i < count(); ++i) {
        MImAbstractKeyArea *mainKba = qobject_cast<MImAbstractKeyArea *>(widget(i));
        if (mainKba) {
            mainKba->setKeyOverrides(overrides);
        }
    }
}

void HorizontalSwitcher::setContentType(MInputMethod::TextContentType type)
{
    foreach(QGraphicsWidget *slide, slides) {
        MImAbstractKeyArea *mainKba = qobject_cast<MImAbstractKeyArea *>(slide);
        if (mainKba) {
            mainKba->setContentType(type);
        }
    }
}

void HorizontalSwitcher::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    // Make the background black during playing animations.
    painter->fillRect(rect(), Qt::black);
}

void HorizontalSwitcher::enableSinglePageHorizontalFlick(bool enable)
{
    if (m_enableSinglePageFlick == enable) {
        return;
    }

    m_enableSinglePageFlick = enable;
    updateHorizontalFlickRecognition();
}

void HorizontalSwitcher::updateHorizontalFlickRecognition()
{
    const bool enable = (m_enableSinglePageFlick || (count() > 1));

    foreach(QGraphicsWidget *slide, slides) {
        MImAbstractKeyArea *keyArea = qobject_cast<MImAbstractKeyArea *>(slide);
        if (keyArea) {
            keyArea->enableHorizontalFlick(enable);
        }
    }
}

