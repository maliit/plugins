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

#ifndef MALIIT_KEYBOARD_RENDERER_H
#define MALIIT_KEYBOARD_RENDERER_H

#include "abstractsurface.h"
#include "models/layout.h"

#include <QtGui>

namespace MaliitKeyboard {

class RendererPrivate;

class Renderer
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Renderer)
    Q_DECLARE_PRIVATE(Renderer)

public:
    explicit Renderer(QObject *parent = 0);
    virtual ~Renderer();

    void setWindow(AbstractSurface *surface,
                   AbstractSurface *magnifierSurface);

    QRegion region() const;
    Q_SIGNAL void regionChanged(const QRegion &region);

    QWidget * viewport() const;

    void addLayout(const SharedLayout &layout);
    void clearLayouts();

    Q_SLOT void show();
    Q_SLOT void hide();

    Q_SLOT void onLayoutChanged(const SharedLayout &layout);
    Q_SLOT void onKeysChanged(const SharedLayout &layout);


private:
    const QScopedPointer<RendererPrivate> d_ptr;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_RENDERER_H
