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

#ifndef MALIIT_KEYBOARD_CANVAS_H
#define MALIIT_KEYBOARD_CANVAS_H

#include "view.h"

namespace MaliitKeyboard {

class CanvasPrivate;

class Canvas
    : public View
{
    Q_OBJECT
    Q_DISABLE_COPY(Canvas)
    Q_DECLARE_PRIVATE(Canvas)

public:
    explicit Canvas(MAbstractInputMethodHost *host,
                    QObject *parent = 0);
    virtual ~Canvas();

    virtual QQmlContext * context() const;
    virtual QQmlContext * extendedKeysContext() const;
    virtual QQmlContext * magnifierContext() const;

    virtual void show();
    virtual void hide();

    virtual void showExtendedKeys();
    virtual void hideExtendedKeys();

    virtual void showMagnifier();
    virtual void hideMagnifier();

    virtual void setWidth(int width);
    virtual void setHeight(int height);
    virtual void setOrigin(const QPoint &origin);

    virtual void setExtendedKeysWidth(int width);
    virtual void setExtendedKeysHeight(int height);
    virtual void setExtendedKeysOrigin(const QPoint &origin);

    virtual void setMagnifierWidth(int width);
    virtual void setMagnifierHeight(int height);
    virtual void setMagnifierOrigin(const QPoint &origin);

private:
    const QScopedPointer<CanvasPrivate> d_ptr;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_CANVAS_H
