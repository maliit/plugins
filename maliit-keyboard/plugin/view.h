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

#ifndef MALIIT_KEYBOARD_VIEW_H
#define MALIIT_KEYBOARD_VIEW_H

#include <maliit/plugins/abstractinputmethodhost.h>
#include <QtCore>
#include <QtQuick>

namespace MaliitKeyboard {

class View
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(View)

public:
    explicit View(MAbstractInputMethodHost *host,
                  QObject *parent = 0);
    virtual ~View() = 0;

    virtual QQmlContext * context() const = 0;
    virtual QQmlContext * extendedKeysContext() const = 0;
    virtual QQmlContext * magnifierContext() const = 0;

    virtual void show() = 0;
    virtual void hide() = 0;

    virtual void showExtendedKeys() = 0;
    virtual void hideExtendedKeys() = 0;

    virtual void showMagnifier() = 0;
    virtual void hideMagnifier() = 0;

    virtual void setWidth(int width) = 0;
    virtual void setHeight(int height) = 0;
    virtual void setOrigin(const QPoint &origin) = 0;

    virtual void setExtendedKeysWidth(int width) = 0;
    virtual void setExtendedKeysHeight(int height) = 0;
    virtual void setExtendedKeysOrigin(const QPoint &origin) = 0;

    virtual void setMagnifierWidth(int width) = 0;
    virtual void setMagnifierHeight(int height) = 0;
    virtual void setMagnifierOrigin(const QPoint &origin) = 0;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_VIEW_H
