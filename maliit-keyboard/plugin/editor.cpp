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

#include "editor.h"

#include <QtGui/QKeyEvent>

namespace MaliitKeyboard {

class EditorPrivate
{
public:
    MAbstractInputMethodHost *host;

    explicit EditorPrivate()
        : host(0)
    {}
};

Editor::Editor(QObject *parent)
    : QObject(parent)
    , d_ptr(new EditorPrivate)
{}

Editor::~Editor()
{}

void Editor::setHost(MAbstractInputMethodHost *host)
{
    Q_D(Editor);
    d->host = host;
}

void Editor::onKeyReleased(const Key &key)
{
    Q_D(Editor);

    if (not d->host) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No host found, forgot to set it?";
        return;
    }

    switch(key.action()) {
    case Key::ActionInsert:
        d->host->sendCommitString(key.text());
        break;

    case Key::ActionBackspace: {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
        d->host->sendKeyEvent(ev);
     } break;

    case Key::ActionReturn: {
        QKeyEvent ev(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        d->host->sendKeyEvent(ev);
    } break;

    case Key::ActionSpace:
        d->host->sendCommitString(" ");
        break;

    default:
        break;
    }
}

} // namespace MaliitKeyboard
