/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
 * Copyright (C) 2012 One Laptop per Child Association
 * Copyright (C) 2013 Openismus GmbH
 *
 * Contact: maliit-discuss@lists.maliit.org
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
 *
 */

#include <QApplication>
#include <QDesktopWidget>
#include <QGraphicsItem>

#include <maliit/plugins/abstractinputmethodhost.h>

#include "surface.h"

namespace MaliitKeyboard
{

namespace
{

class RootItem
    : public QGraphicsItem
{
public:
    RootItem();

    void setRect(const QRectF &rect);
    virtual QRectF boundingRect() const;
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *widget);

private:
    QRectF m_rect;
};

} // unnamed namespace

class SurfacePrivate
{
public:
    SurfacePrivate(Maliit::Position position,
                   MAbstractInputMethodHost *host);

    QScopedPointer<RootItem> m_root_item;
    Maliit::Position m_position;
    QPoint m_relative_position;
    MAbstractInputMethodHost *m_host;
};

RootItem::RootItem()
    : QGraphicsItem(),
      m_rect()
{
    setFlag(QGraphicsItem::ItemHasNoContents);
}

void RootItem::setRect(const QRectF &rect)
{
    m_rect = rect;
}

QRectF RootItem::boundingRect() const
{
    return m_rect;
}

void RootItem::paint(QPainter *painter,
                     const QStyleOptionGraphicsItem *option,
                     QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);
}

SurfacePrivate::SurfacePrivate(Maliit::Position position,
                               MAbstractInputMethodHost *host)
    : m_root_item(),
      m_position(position),
      m_relative_position(),
      m_host(host)
{}

Surface::Surface(Surface *parent,
                 Maliit::Position position,
                 MAbstractInputMethodHost *host)
    : QGraphicsView(parent),
      d_ptr(new SurfacePrivate(position, host))
{
    setAttribute(Qt::WA_ShowWithoutActivating);
    setAutoFillBackground(false);
    setBackgroundRole(QPalette::NoRole);
    setBackgroundBrush(Qt::transparent);
    // This is a workaround for non-compositing window managers. Apparently
    // setting this attribute while using such WMs may cause garbled
    // painting of VKB.
#ifndef DISABLE_TRANSLUCENT_BACKGROUND_HINT
    setAttribute(Qt::WA_TranslucentBackground);
#endif
    viewport()->setAutoFillBackground(false);

    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    setOptimizationFlags(QGraphicsView::DontClipPainter |
                         QGraphicsView::DontSavePainterState);
    setFrameShape(QFrame::NoFrame);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setScene(new QGraphicsScene(this));
}

Surface::~Surface()
{}

QGraphicsItem *Surface::root() const
{
    Q_D(const Surface);

    return d->m_root_item.data();
}

void Surface::setSize(const QSize &a_size)
{
    Q_D(Surface);

    if (isWindow() && d->m_position == Maliit::PositionCenterBottom) {
        const QSize desktopSize = QApplication::desktop()->screenGeometry().size();

        setGeometry(QRect(QPoint((desktopSize.width() - a_size.width()) / 2,
                                 desktopSize.height() - a_size.height()), a_size));
    } else {
        resize(a_size);
    }

    const QRect rect(QPoint(), size());

    setSceneRect(rect);
    if (not d->m_root_item.isNull ()) {
        d->m_root_item->setRect(rect);
    }
}

void Surface::setRelativePosition(const QPoint &position)
{
    Q_D(Surface);

    d->m_relative_position = position;

    QPoint parentPosition(0, 0);
    QWidget *parentWidget(qobject_cast<QWidget*>(parent()));

    if (parentWidget) {
        if (isWindow() && !parentWidget->isWindow()) {
            parentPosition = parentWidget->mapToGlobal(QPoint(0, 0));
        } else if (!isWindow() && parentWidget->isWindow()) {
            // do nothing
        } else {
            parentPosition = parentWidget->pos();
        }
    }
    move(parentPosition + position);
}

QPoint Surface::relativePosition() const
{
    Q_D(const Surface);

    return d->m_relative_position;
}

QPoint Surface::translateEventPosition(const QPoint &eventPosition,
                                       Surface *eventSurface) const
{
    if (not eventSurface) {
        return eventPosition;
    }

    return -pos() + eventPosition + eventSurface->pos();
}

bool Surface::event(QEvent *event)
{
    if (event && event->type() == QEvent::WinIdChange) {
        Q_D(Surface);

        d->m_host->registerWindow(windowHandle(), d->m_position);
    }

    return QGraphicsView::event(event);
}

void Surface::showEvent(QShowEvent *event)
{
    Q_D(Surface);

    QGraphicsView::showEvent(event);

    if (d->m_root_item.isNull ()) {
        d->m_root_item.reset(new RootItem);

        scene()->addItem(d->m_root_item.data());
        d->m_root_item->setRect(QRect(QPoint(), size()));
        d->m_root_item->show();
    }
}

} // namespace MaliitKeyboard
