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

#include "view.h"

namespace MaliitKeyboard {
//! \class View
//! \brief A view abstraction to handle platform-specific keyboard representations.
//!
//! On X11, we want to handle the view differently as for instance on Wayland.
//! The latter allows usage of subsurfaces, whereas the former doesn't work too
//! well with multiple property-driven views:
//!   X11 expects property changes to take place in a certain order and cannot
//! cope with rapid changes to XWindows at all (for instance, window placement
//! or window visibility).
//!   The Surfaces class handles multiple surfaces and should closely match
//! Wayland's surface handling (including sub-surfaces). The Canvas class on
//! the other hand represents one drawable area and targets X11. Therefore, it
//! contains of a single QQuickView with enough space on top of the main
//! keyboard area to also draw extended keys and the key magnifier.

class ViewPrivate
{};


View::View(MAbstractInputMethodHost *host,
           QObject *parent)
    : QObject(parent)
{
    Q_UNUSED(host)
}


View::~View()
{}


//! \brief Returns primary QML context.
//!
//! Should never return null.
QQmlContext * View::context() const
{
    return 0;
}


//! \brief Returns a secondary QML context.
//!
//! Can return null, in which case the primary QML context shall be used.
//! See context().
QQmlContext * View::extendedKeysContext() const
{
    return 0;
}


//! \brief Returns a secondary QML context.
//!
//! Can return null, in which case the primary QML context shall be used.
//! See context().
QQmlContext * View::magnifierContext() const
{
    return 0;
}


//! \brief Shows the primary view.
//!
//! Propagates to secondary views.
void View::show() {}


//! \brief Hides the primary view.
//!
//! Propagates to secondary views.
void View::hide() {}


//! \brief Shows secondary view for extended keys.
//!
//! Transient to primary view.
void View::showExtendedKeys() {}


//! \brief Hides secondary view for extended keys.
//!
//! Transient to primary view.
void hideExtendedKeys() {}


//! \brief Shows secondary view for magnifier.
//!
//! Transient to primary view.
void View::showMagnifier() {}


//! \brief Hides secondary view for magnifier.
//!
//! Transient to primary view.
void View::hideMagnifier() {}


//! \brief Sets width of primary view.
void View::setWidth(int width) {Q_UNUSED(width)}


//! \brief Sets height of primary view.
void View::setHeight(int height) {Q_UNUSED(height)}


//! \brief Sets origin of primary view.
void View::setOrigin(const QPoint &origin) {Q_UNUSED(origin)}


//! \brief Sets width of secondary view for extended keys.
void View::setExtendedKeysWidth(int width) {Q_UNUSED(width)}


//! \brief Sets height of secondary view for extended keys.
void View::setExtendedKeysHeight(int height) {Q_UNUSED(height)}


//! \brief Sets origin of secondary view for extended keys.
void View::setExtendedKeysOrigin(const QPoint &origin) {Q_UNUSED(origin)};


//! \brief Sets width of secondary view for magnifier.
void View::setMagnifierWidth(int width) {Q_UNUSED(width)}


//! \brief Sets height of secondary view for magnifier.
void View::setMagnifierHeight(int height) {Q_UNUSED(height)}


//! \brief Sets origin of secondary view for magnifier.
void View::setMagnifierOrigin(const QPoint &origin) {Q_UNUSED(origin)}


} // namespace MaliitKeyboard
