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
#include <QTimer>

namespace MaliitKeyboard {

EditorOptions::EditorOptions()
    : backspace_auto_repeat_delay(500)
    , backspace_auto_repeat_interval(300)
{
}

class EditorPrivate
{
public:
    MAbstractInputMethodHost *host;
    QTimer auto_repeat_backspace_timer;
    bool backspace_sent;
    EditorOptions options;

    explicit EditorPrivate(const EditorOptions &newOptions);
};

EditorPrivate::EditorPrivate(const EditorOptions &newOptions)
    : host(0)
    , backspace_sent(false)
    , options(newOptions)
{
    auto_repeat_backspace_timer.setSingleShot(true);
}

Editor::Editor(const EditorOptions &newOptions, QObject *parent)
    : QObject(parent)
    , d_ptr(new EditorPrivate(newOptions))
{
    connect(&d_ptr->auto_repeat_backspace_timer, SIGNAL(timeout()),
            this, SLOT(autoRepeatBackspace()));
}

Editor::~Editor()
{}

void Editor::setHost(MAbstractInputMethodHost *host)
{
    Q_D(Editor);
    d->host = host;
}

void Editor::onKeyPressed(const Key &key)
{
    Q_D(Editor);

    if (key.action() == Key::ActionBackspace) {
        d->backspace_sent = false;
        d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_delay);
    }
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
        if (not d->backspace_sent) {
            QKeyEvent ev(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
            d->host->sendKeyEvent(ev);
        }
        d->auto_repeat_backspace_timer.stop();
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

void Editor::onKeyEntered(const Key &key)
{
    Q_D(Editor);

    if (key.action() == Key::ActionBackspace) {
        d->backspace_sent = false;
        d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_delay);
    }
}

void Editor::onKeyExited(const Key &key)
{
    Q_D(Editor);

    if (key.action() == Key::ActionBackspace) {
        d->auto_repeat_backspace_timer.stop();
    }
}

// TODO: this implementation does not take into account following features:
// 1) preedit string
//      if there is preedit then first call to autoRepeatBackspace should clean it completely
//      and following calls should remove remaining text character by character
// 2) multitouch
//      it is not completely clean how to handle multitouch for backspace,
//      but we can follow the strategy from meego-keyboard - release pressed
//      key when user press another one at the same time. Then we do not need to
//      change anything in this method
void Editor::autoRepeatBackspace()
{
    Q_D(Editor);

    if (not d->host) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No host found, forgot to set it?";
        return;
    }

    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    d->host->sendKeyEvent(ev);
    d->backspace_sent = true;
    d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_interval);
}

} // namespace MaliitKeyboard
