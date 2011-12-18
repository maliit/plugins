/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 *
 */

#include "renderer.h"
#include "keyareaitem.h"
#include "keyitem.h"
#include "graphicsview.h"
#include "models/keyarea.h"

#ifdef MALIIT_KEYBOARD_HAVE_OPENGL
#include <QGLWidget>
#endif

namespace MaliitKeyboard {

namespace {

QRect mapToScreenCoordinates(const QRectF &rect,
                             Layout::Orientation o)
{
    switch (o) {
    case Layout::Landscape:
        return rect.toRect();
    case Layout::Portrait:
        return QRect(rect.y(), rect.x(), rect.height(), rect.width());
    }

    return QRect();
}

class LayoutItem {
public:
    SharedLayout layout;
    KeyAreaItem *left_item;
    KeyAreaItem *right_item;
    KeyAreaItem *center_item;
    KeyAreaItem *extended_item;
    QRegion region;

    explicit LayoutItem()
        : layout()
        , left_item(0)
        , right_item(0)
        , center_item(0)
        , extended_item(0)
        , region()
    {}

    KeyAreaItem *activeItem() const
    {
        if (layout.isNull()) {
            qCritical() << __PRETTY_FUNCTION__
                        << "Invalid layout!";
            return 0;
        }

        switch(layout->activePanel()) {
        case Layout::LeftPanel:
            return left_item;

        case Layout::RightPanel:
            return right_item;

        case Layout::CenterPanel:
            return center_item;

        case Layout::ExtendedPanel:
            return extended_item;

        default:
            qCritical() << __PRETTY_FUNCTION__
                        << "Invalid case - should not be reached!"
                        << layout->activePanel();
            return 0;
        }

        qCritical() << __PRETTY_FUNCTION__
                    << "Should not be reached!";
        return 0;
    }

    void show(QGraphicsItem *root,
              QRegion *region)
    {
        if (layout.isNull() || not region) {
            qCritical() << __PRETTY_FUNCTION__
                        << "Invalid region or layout!";
            return;
        }

        if (not center_item) {
            center_item = new KeyAreaItem(root);
        }

        if (not extended_item) {
            extended_item = new KeyAreaItem(root);
        }

        center_item->setParentItem(root);
        center_item->setKeyArea(layout->centerPanel());
        center_item->update();
        center_item->show();
        *region |= QRegion(mapToScreenCoordinates(layout->centerPanel().rect, layout->orientation()));

        extended_item->setParentItem(root);
        extended_item->setKeyArea(layout->extendedPanel());
        extended_item->update();

        if (layout->activePanel() != Layout::ExtendedPanel) {
            extended_item->hide();
        } else {
            extended_item->show();
            *region |= QRegion(mapToScreenCoordinates(layout->extendedPanel().rect, layout->orientation()));
        }

        root->show();
    }

    void hide()
    {
        if (left_item) {
            left_item->hide();
        }

        if (right_item) {
            right_item->hide();
        }

        if (center_item) {
            center_item->hide();
        }

        if (extended_item) {
            extended_item->hide();
        }
    }
};

class RootItem
    : public QGraphicsItem
{
private:
    QRectF m_rect;

public:
    explicit RootItem(QGraphicsItem *parent = 0)
        : QGraphicsItem(parent)
        , m_rect()
    {
        setFlag(QGraphicsItem::ItemHasNoContents);
    }

    void setRect(const QRectF &rect)
    {
        m_rect = rect;
    }

    virtual QRectF boundingRect() const
    {
        return m_rect;
    }

    virtual void paint(QPainter *,
                       const QStyleOptionGraphicsItem *,
                       QWidget *)
    {}
};

QGraphicsView * createView(QWidget *widget,
                           AbstractBackgroundBuffer *buffer)
{
    GraphicsView *view = new GraphicsView(widget);
    view->setBackgroundBuffer(buffer);
    view->resize(widget->size());
    view->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    view->setOptimizationFlags(QGraphicsView::DontClipPainter | QGraphicsView::DontSavePainterState);
    QGraphicsScene *scene = new QGraphicsScene(view);
    view->setScene(scene);
    view->setSceneRect(widget->rect());
    view->setFrameShape(QFrame::NoFrame);
    view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

#ifdef MALIIT_KEYBOARD_HAVE_OPENGL
    QGLWidget *gl_widget = new QGLWidget;
    if (gl_widget->isValid()) {
        view->setViewport(gl_widget);
        view->setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    } else {
        delete gl_widget;
    }
#endif

    scene->setSceneRect(widget->rect());
    view->show();

    return view;
}

void recycleKeyItem(QVector<KeyItem *> *key_items,
                    int index,
                    const Key &key,
                    QGraphicsItem *parent)
{
    KeyItem *item = 0;

    if (index >= key_items->count()) {
        item = new KeyItem;
        key_items->append(item);
    } else {
        item = key_items->at(index);
    }

    item->setParentItem(parent);
    item->setKey(key);
    item->show();
}

} // namespace

class RendererPrivate
{
public:
    QWidget *window;
    QScopedPointer<QGraphicsView> view;
    AbstractBackgroundBuffer *buffer;
    QRegion region;
    QVector<LayoutItem> layout_items;
    QVector<KeyItem *> key_items;
    RootItem *root;

    explicit RendererPrivate()
        : window(0)
        , view(0)
        , buffer(0)
        , region()
        , layout_items()
        , key_items()
        , root(0)
    {}
};

Renderer::Renderer(QObject *parent)
    : QObject(parent)
    , d_ptr(new RendererPrivate)
{}

Renderer::~Renderer()
{}

void Renderer::setWindow(QWidget *window,
                         AbstractBackgroundBuffer *buffer)
{
    Q_D(Renderer);
    d->window = window;

    d->buffer = buffer;
    d->view.reset(createView(d->window, d->buffer));
}

QRegion Renderer::region() const
{
    Q_D(const Renderer);
    return d->region;
}

QWidget * Renderer::viewport() const
{
    Q_D(const Renderer);
    return d->view->viewport();
}

void Renderer::addLayout(const SharedLayout &layout)
{
    Q_D(Renderer);

    LayoutItem li;
    li.layout = layout;
    d->layout_items.append(li);
}

void Renderer::clearLayouts()
{
    Q_D(Renderer);

    d->layout_items.clear();
    d->key_items.clear();
    d->root = 0;
    d->view->scene()->clear();
}

void Renderer::show()
{
    Q_D(Renderer);

    const QRect &rect(d->view->rect());

    if (not d->root) {
        d->view->scene()->addItem(d->root = new RootItem);
        d->root->setRect(rect);
        d->root->show();
    }

    if (not d->view || d->layout_items.isEmpty()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No view or no layouts exists, aborting!";
    }

    Q_FOREACH (QGraphicsItem *key_item, d->key_items) {
        key_item->hide();
    }

    d->region = QRegion();
    Layout::Orientation orientation = Layout::Landscape;

    for (int index = 0; index < d->layout_items.count(); ++index) {
        LayoutItem &li(d->layout_items[index]);

        if (li.layout) {
            orientation = li.layout->orientation(); // last layout wins ...
        }

        li.show(d->root, &d->region);
    }

    switch (orientation) {
    case Layout::Landscape:
        d->root->setRect(rect);
        d->root->setRotation(0);
        d->root->setPos(0, 0);
        break;

    case Layout::Portrait:
        d->root->setRect(QRectF(rect.x(), rect.y(), rect.height(), rect.width()));
        d->root->setRotation(-90);
        d->root->setPos(0, rect.height());
        break;
    }

    d->view->show();
    d->view->raise();
    Q_EMIT regionChanged(d->region);
}

void Renderer::hide()
{
    Q_D(Renderer);

    Q_FOREACH (LayoutItem li, d->layout_items) {
        li.hide();
    }

    d->view->hide();
    d->region = QRegion();
    Q_EMIT regionChanged(d->region);
}

void Renderer::onLayoutChanged(const SharedLayout &layout)
{
    Q_UNUSED(layout)
    show();
}

void Renderer::onKeysChanged(const SharedLayout &layout)
{
    if (layout.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Invalid layout.";
        return;
     }

    Q_D(Renderer);

    if (d->key_items.count() > 10) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Unusal amount of key items:" << d->key_items.count()
                   << ", amount of active keys:" << layout->activeKeys().count();
    }

    KeyAreaItem *parent = 0;
    for (int index = 0; index < d->layout_items.count(); ++index) {
        const LayoutItem &li(d->layout_items.at(index));

        if (li.layout == layout) {
            parent = li.activeItem();
            break;
        }
    }

    int index = 0;
    // Found the KeyAreaItem, which means layout is known by the renderer, too.
    if (parent) {
        const QVector<Key> &active_keys(layout->activeKeys());

        for (; index < active_keys.count(); ++index) {
            recycleKeyItem(&d->key_items, index, active_keys.at(index), parent);
        }

        if (layout->magnifierKey().valid()) {
            recycleKeyItem(&d->key_items, index, layout->magnifierKey(), parent);
            ++index;
        }
    }

    // Hide remaining, currently unneeded key items:
    for (; index < d->key_items.count(); ++index) {
        d->key_items.at(index)->hide();
    }
}

} // namespace MaliitKeyboard
