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
#include "wordribbonitem.h"
#include "graphicsview.h"

#include "models/keyarea.h"
#include "models/wordribbon.h"

#ifdef MALIIT_KEYBOARD_HAVE_OPENGL
#include <QGLWidget>
#endif

#include <maliit/plugins/abstractwidgetssurface.h>

using Maliit::Plugins::AbstractGraphicsViewSurface;
using Maliit::Plugins::AbstractSurface;
using Maliit::Plugins::AbstractSurfaceFactory;

namespace MaliitKeyboard {

namespace {

const qreal WordRibbonZIndex = 0.0f;
const qreal CenterPanelZIndex = 1.0f;
const qreal ExtendedPanelZIndex = 20.0f;

const qreal ActiveKeyZIndex = 0.0f;

class LayoutItem {
public:
    SharedLayout layout;
    KeyAreaItem *left_item;
    KeyAreaItem *right_item;
    KeyAreaItem *center_item;
    KeyAreaItem *extended_item;
    WordRibbonItem *ribbon_item;

    explicit LayoutItem()
        : layout()
        , left_item(0)
        , right_item(0)
        , center_item(0)
        , extended_item(0)
        , ribbon_item(0)
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
              QGraphicsItem *extended_root)
    {
        if (layout.isNull()) {
            qCritical() << __PRETTY_FUNCTION__
                        << "Invalid layout!";
            return;
        }

        if (not center_item) {
            center_item = new KeyAreaItem(root);
            center_item->setZValue(CenterPanelZIndex);
        }

        if (not extended_item) {
            extended_item = new KeyAreaItem(extended_root);
            extended_item->setZValue(ExtendedPanelZIndex);
        }

        if (not ribbon_item) {
            ribbon_item = new WordRibbonItem(root);
            ribbon_item->setZValue(WordRibbonZIndex);
        }

        center_item->setParentItem(root);
        center_item->setKeyArea(layout->centerPanel(), layout->centerPanelGeometry());
        center_item->update();
        center_item->show();

        extended_item->setParentItem(extended_root);
        extended_item->setKeyArea(layout->extendedPanel(), layout->extendedPanelGeometry());
        extended_item->update();

        ribbon_item->setParentItem(root);
        ribbon_item->setWordRibbon(layout->wordRibbon(), layout->wordRibbonGeometry());
        ribbon_item->update();
        ribbon_item->show();

        if (layout->activePanel() != Layout::ExtendedPanel) {
            extended_item->hide();
        } else {
            extended_item->show();
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

    // If there is no buffer, it probably means we run stand-alone. But when
    // run as a maliit-server plugin, the server takes care of making any
    // background widget visible. Without the server, we have to make the
    // QGraphicsView translucent ourselves:
    if (not buffer) {
        view->setBackgroundBrush(Qt::transparent);
        view->setBackgroundRole(QPalette::NoRole);
        view->setWindowFlags(Qt::FramelessWindowHint);
        view->setAttribute(Qt::WA_NoSystemBackground);
        view->viewport()->setAutoFillBackground(false);
    }

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
        item->setZValue(ActiveKeyZIndex);
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
    Maliit::Plugins::AbstractSurfaceFactory *factory;
    QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> surface;
    QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> extended_surface;
    QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> magnifier_surface;
    QVector<LayoutItem> layout_items;
    QVector<KeyItem *> key_items;
    QVector<KeyItem *> extended_key_items;
    QVector<KeyItem *> magnifier_key_items;

    explicit RendererPrivate()
        : factory(0)
        , surface()
        , extended_surface()
        , magnifier_surface()
        , layout_items()
        , key_items()
        , extended_key_items()
        , magnifier_key_items()
    {}
};

Renderer::Renderer(QObject *parent)
    : QObject(parent)
    , d_ptr(new RendererPrivate)
{}

Renderer::~Renderer()
{}

void Renderer::setSurfaceFactory(AbstractSurfaceFactory *factory)
{
    Q_D(Renderer);
    d->factory = factory;

    // Assuming that factory == 0 means reset:
    if (not d->factory) {
        // Drop references => shared surface instances are eventually deleted:
        d->surface.clear();
        d->extended_surface.clear();
        d->magnifier_surface.clear();
        return;
    }

    d->surface = qSharedPointerDynamicCast<AbstractGraphicsViewSurface>(
        factory->create(AbstractSurface::PositionCenterBottom | AbstractSurface::TypeGraphicsView));

    d->extended_surface = qSharedPointerDynamicCast<AbstractGraphicsViewSurface>(
        factory->create(AbstractSurface::PositionOverlay | AbstractSurface::TypeGraphicsView, d->surface));

    d->magnifier_surface = qSharedPointerDynamicCast<AbstractGraphicsViewSurface>(
        factory->create(AbstractSurface::PositionOverlay | AbstractSurface::TypeGraphicsView, d->surface));
}

const QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> Renderer::surface() const
{
    Q_D(const Renderer);

    return d->surface;
}

const QSharedPointer<Maliit::Plugins::AbstractGraphicsViewSurface> Renderer::extendedSurface() const
{
    Q_D(const Renderer);

    return d->extended_surface;
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
    d->extended_key_items.clear();
    d->magnifier_key_items.clear();
    d->surface->clear();
    d->extended_surface->clear();
    d->magnifier_surface->clear();
}

void Renderer::show()
{
    Q_D(Renderer);

    if (d->surface.isNull()
        || d->extended_surface.isNull()
        || d->magnifier_surface.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Some surfaces not available, cannot show keyboard!"
                    << "Discarding show request.";
        return;
    }

    d->surface->show();

    if (not d->surface->view() || d->layout_items.isEmpty()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No view or no layouts exists!"
                    << "Discarding show request";
        return;
    }

    Q_FOREACH (QGraphicsItem *key_item, d->key_items) {
        key_item->hide();
    }
    Q_FOREACH (QGraphicsItem *key_item, d->extended_key_items) {
        key_item->hide();
    }
    Q_FOREACH (QGraphicsItem *key_item, d->magnifier_key_items) {
        key_item->hide();
    }

    for (int index = 0; index < d->layout_items.count(); ++index) {
        LayoutItem &li(d->layout_items[index]);

        // Show first the extended keys surface before trying to add QGraphicsItem into it
        if (li.layout->activePanel() != Layout::ExtendedPanel) {
            d->extended_surface->hide();
        } else {
            d->extended_surface->setSize(li.layout->extendedPanelGeometry().size());
            d->extended_surface->setRelativePosition(li.layout->extendedPanelOrigin());
            d->extended_surface->show();
        }
        li.show(d->surface->root(), d->extended_surface->root());
        d->surface->setSize(QSize(li.layout->centerPanelGeometry().width(), li.layout->centerPanelGeometry().height() + li.layout->wordRibbonGeometry().height()));
    }
}

void Renderer::hide()
{
    Q_D(Renderer);

    if (d->surface.isNull()
        || d->extended_surface.isNull()
        || d->magnifier_surface.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Some surfaces not available, cannot hide keyboard!"
                    << "Discarding hide request.";
        return;
    }

    Q_FOREACH (LayoutItem li, d->layout_items) {
        li.hide();
    }

    d->surface->hide();
    d->extended_surface->hide();
    d->magnifier_surface->hide();
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

    QVector<KeyItem *> *key_items = layout->activePanel() == Layout::ExtendedPanel ? &d->extended_key_items : &d->key_items;

    int index = 0;
    int magnifier_index = 0;
    // Found the KeyAreaItem, which means layout is known by the renderer, too.
    if (parent) {
        const QVector<Key> &active_keys(layout->activeKeys());

        for (; index < active_keys.count(); ++index) {
            recycleKeyItem(key_items, index, active_keys.at(index), parent);
        }

        if (layout->magnifierKey().valid()) {
            d->magnifier_surface->setSize(layout->magnifierKey().area().size());
            d->magnifier_surface->setRelativePosition(layout->magnifierKeyOrigin());
            d->magnifier_surface->show();
            recycleKeyItem(&d->magnifier_key_items, magnifier_index, layout->magnifierKey(), d->magnifier_surface->root());
            ++magnifier_index;
        } else {
            d->magnifier_surface->hide();
        }
    }

    // Hide remaining, currently unneeded key items:
    for (; index < key_items->count(); ++index) {
        key_items->at(index)->hide();
    }
//    for (; magnifier_index < d->magnifier_key_items.count(); ++magnifier_index) {
//        d->magnifier_key_items.at(magnifier_index)->hide();
//    }
}

void Renderer::onWordCandidatesChanged(const SharedLayout &layout)
{
    Q_D(Renderer);

    if (layout.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Invalid layout.";
        return;
     }

    for (int index = 0; index < d->layout_items.count(); ++index) {
        const LayoutItem &li(d->layout_items.at(index));

        if (li.layout == layout) {
            li.ribbon_item->setWordRibbon(layout->wordRibbon(), layout->wordRibbonGeometry());
            break;
        }
    }

}

} // namespace MaliitKeyboard
