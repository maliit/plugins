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

#include "canvas.h"

namespace MaliitKeyboard {
class CanvasPrivate
{};

Canvas::Canvas(MAbstractInputMethodHost *host,
               QObject *parent)
    : View(host, parent)
    , d_ptr(new CanvasPrivate)
{
    Q_UNUSED(host)
}

Canvas::~Canvas() {}

QQmlContext * Canvas::context() const {return 0;}
QQmlContext * Canvas::extendedKeysContext() const {return 0;}
QQmlContext * Canvas::magnifierContext() const {return 0;}

void Canvas::show() {}
void Canvas::hide() {}

void Canvas::showExtendedKeys() {}
void Canvas::hideExtendedKeys() {}

void Canvas::showMagnifier() {}
void Canvas::hideMagnifier() {}

void Canvas::setWidth(int width) {Q_UNUSED(width)}
void Canvas::setHeight(int height) {Q_UNUSED(height)}
void Canvas::setOrigin(const QPoint &origin) {Q_UNUSED(origin)}

void Canvas::setExtendedKeysWidth(int width) {Q_UNUSED(width)}
void Canvas::setExtendedKeysHeight(int height) {Q_UNUSED(height)}
void Canvas::setExtendedKeysOrigin(const QPoint &origin) {Q_UNUSED(origin)};

void Canvas::setMagnifierWidth(int width) {Q_UNUSED(width)}
void Canvas::setMagnifierHeight(int height) {Q_UNUSED(height)}
void Canvas::setMagnifierOrigin(const QPoint &origin) {Q_UNUSED(origin)}
} // namespace MaliitKeyboard
