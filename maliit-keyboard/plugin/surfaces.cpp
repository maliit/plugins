/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2013 Michael Hasselmann
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

#include "surfaces.h"

namespace MaliitKeyboard {


void makeQuickViewTransparent(QQuickView *view)
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    view->setFormat(format);
    view->setColor(QColor(Qt::transparent));
}


QQuickView *getSurface (MAbstractInputMethodHost *host)
{
    QScopedPointer<QQuickView> view(new QQuickView (0));
    host->registerWindow (view.data(), Maliit::PositionCenterBottom);
    makeQuickViewTransparent(view.data());

    return view.take ();
}

QQuickView *getOverlaySurface (MAbstractInputMethodHost *host, QQuickView *parent)
{
    QScopedPointer<QQuickView> view(new QQuickView (0));
    view->setTransientParent(parent);
    host->registerWindow (view.data(), Maliit::PositionOverlay);
    makeQuickViewTransparent(view.data());

    return view.take ();
}

const QString g_maliit_keyboard_qml(MALIIT_KEYBOARD_DATA_DIR "/maliit-keyboard.qml");
const QString g_maliit_keyboard_extended_qml(MALIIT_KEYBOARD_DATA_DIR "/maliit-keyboard-extended.qml");
const QString g_maliit_magnifier_qml(MALIIT_KEYBOARD_DATA_DIR "/maliit-magnifier.qml");


class SurfacesPrivate
{
public:
    QScopedPointer<QQuickView> surface;
    QScopedPointer<QQuickView> extended_surface;
    QScopedPointer<QQuickView> magnifier_surface;

    explicit SurfacesPrivate(MAbstractInputMethodHost *host);
};


SurfacesPrivate::SurfacesPrivate(MAbstractInputMethodHost *host)
    : surface(getSurface(host))
    , extended_surface(getOverlaySurface(host, surface.data()))
    , magnifier_surface(getOverlaySurface(host, surface.data()))
{
    // TODO: Figure out whether two views can share one engine.
    QQmlEngine *const engine(surface->engine());
    engine->addImportPath(MALIIT_KEYBOARD_DATA_DIR);

    QQmlEngine *const extended_engine(extended_surface->engine());
    extended_engine->addImportPath(MALIIT_KEYBOARD_DATA_DIR);

    QQmlEngine *const magnifier_engine(magnifier_surface->engine());
    magnifier_engine->addImportPath(MALIIT_KEYBOARD_DATA_DIR);
}


Surfaces::Surfaces(MAbstractInputMethodHost *host,
                   QObject *parent)
    : View(host, parent)
    , d_ptr(new SurfacesPrivate(host))
{}


Surfaces::~Surfaces()
{}


QQmlContext * Surfaces::context() const
{
    Q_D(const Surfaces);
    return d->surface->engine()->rootContext();
}


QQmlContext * Surfaces::extendedKeysContext() const
{
    Q_D(const Surfaces);
    return d->extended_surface->engine()->rootContext();
}


QQmlContext * Surfaces::magnifierContext() const
{
    Q_D(const Surfaces);
    return d->magnifier_surface->engine()->rootContext();
}


void Surfaces::show()
{
    Q_D(Surfaces);
    static bool is_initialized = false;

    const QRect &rect = d->surface->screen()->availableGeometry();
    const int width = d->surface->width();
    const int height = d->surface->height();

    d->surface->setGeometry(QRect(QPoint(rect.x() + (rect.width() - width) / 2,
                                         rect.y() + rect.height() - height),
                                  QSize(width, height)));


    if (not is_initialized) {
        d->surface->setSource(QUrl::fromLocalFile(g_maliit_keyboard_qml));
        d->extended_surface->setSource(QUrl::fromLocalFile(g_maliit_keyboard_extended_qml));
        d->magnifier_surface->setSource(QUrl::fromLocalFile(g_maliit_magnifier_qml));
        is_initialized = true;
    }

    d->surface->show();
    d->extended_surface->show();
    d->magnifier_surface->show();
}


void Surfaces::hide()
{
    Q_D(Surfaces);
    d->surface->hide();
    d->extended_surface->hide();
    d->magnifier_surface->hide();
}


void Surfaces::showExtendedKeys()
{
    Q_D(Surfaces);
    d->extended_surface->show();
}


void Surfaces::hideExtendedKeys()
{
    Q_D(Surfaces);
    d->extended_surface->hide();
}


void Surfaces::showMagnifier()
{
    Q_D(Surfaces);
    d->magnifier_surface->show();
}


void Surfaces::hideMagnifier()
{
    Q_D(Surfaces);
    d->magnifier_surface->hide();
}


void Surfaces::setWidth(int width)
{
    Q_D(Surfaces);
    d->surface->setWidth(width);
}


void Surfaces::setHeight(int height)
{
    Q_D(Surfaces);
    d->surface->setHeight(height);
}


void Surfaces::setOrigin(const QPoint &origin)
{
    Q_D(Surfaces);
    d->surface->setPosition(origin);
}


void Surfaces::setExtendedKeysWidth(int width)
{
    Q_D(Surfaces);
    d->extended_surface->setWidth(width);
}


void Surfaces::setExtendedKeysHeight(int height)
{
    Q_D(Surfaces);
    d->extended_surface->setHeight(height);
}


void Surfaces::setExtendedKeysOrigin(const QPoint &origin)
{
    Q_D(Surfaces);
    d->extended_surface->setPosition(d->surface->position() + origin);
}


void Surfaces::setMagnifierWidth(int width)
{
    Q_D(Surfaces);
    d->magnifier_surface->setWidth(width);
}


void Surfaces::setMagnifierHeight(int height)
{
    Q_D(Surfaces);
    d->magnifier_surface->setHeight(height);
}


void Surfaces::setMagnifierOrigin(const QPoint &origin)
{
    Q_D(Surfaces);
    d->magnifier_surface->setPosition(d->surface->position() + origin);
}


} // namespace MaliitKeyboard
