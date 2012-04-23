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

#ifndef MALIIT_KEYBOARD_GLASS_H
#define MALIIT_KEYBOARD_GLASS_H

#include "models/key.h"
#include "models/keyarea.h"
#include "models/layout.h"

#include <QtGui>

namespace MaliitKeyboard {

class GlassPrivate;

class Glass
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Glass)
    Q_DECLARE_PRIVATE(Glass)

public:
    explicit Glass(QObject *parent = 0);
    virtual ~Glass();

    void setWindow(QWidget *window);

    void addLayout(const SharedLayout &layout);
    void clearLayouts();

    Q_SIGNAL void keyPressed(const Key &key,
                             const SharedLayout &layout);
    Q_SIGNAL void keyLongPressed(const Key &key,
                                 const SharedLayout &layout);
    Q_SIGNAL void keyReleased(const Key &key,
                              const SharedLayout &layout);
    Q_SIGNAL void keyEntered(const Key &key,
                             const SharedLayout &layout);
    Q_SIGNAL void keyExited(const Key &key,
                            const SharedLayout &layout);
    Q_SIGNAL void keyAreaPressed(Layout::Panel panel, const SharedLayout &layout);
    Q_SIGNAL void keyAreaReleased(Layout::Panel panel, const SharedLayout &layout);
    Q_SIGNAL void switchLeft(const SharedLayout &layout);
    Q_SIGNAL void switchRight(const SharedLayout &layout);
    Q_SIGNAL void keyboardClosed();

protected:
    //! \reimp
    virtual bool eventFilter(QObject *obj,
                             QEvent *ev);
    //! \reimp_end

private:
    Q_SLOT void onLongPressTriggered();
    bool handlePressReleaseEvent(QEvent *ev);

    const QScopedPointer<GlassPrivate> d_ptr;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_GLASS_H
