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

#include "abstracttexteditor.h"
#include "models/wordribbon.h"

namespace MaliitKeyboard {

EditorOptions::EditorOptions()
    : backspace_auto_repeat_delay(500)
    , backspace_auto_repeat_interval(300)
{}

class AbstractTextEditorPrivate
{
public:
    QTimer auto_repeat_backspace_timer;
    bool backspace_sent;
    EditorOptions options;
    Model::SharedText text;

    explicit AbstractTextEditorPrivate(const EditorOptions &new_options,
                                       const Model::SharedText &new_text);
    bool valid() const;
};

AbstractTextEditorPrivate::AbstractTextEditorPrivate(const EditorOptions &new_options,
                                                     const Model::SharedText &new_text)
    : auto_repeat_backspace_timer()
    , backspace_sent(false)
    , options(new_options)
    , text(new_text)
{
    auto_repeat_backspace_timer.setSingleShot(true);
    (void) valid();
}

bool AbstractTextEditorPrivate::valid() const
{
    const bool is_invalid(text.isNull());

    if (is_invalid) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Invalid text model! The text editor will not function properly.";
    }

    return (not is_invalid);
}

AbstractTextEditor::AbstractTextEditor(const EditorOptions &options,
                                       const Model::SharedText &text,
                                       QObject *parent)
    : QObject(parent)
    , d_ptr(new AbstractTextEditorPrivate(options, text))
{
    connect(&d_ptr->auto_repeat_backspace_timer, SIGNAL(timeout()),
            this,                                SLOT(autoRepeatBackspace()));
}

AbstractTextEditor::~AbstractTextEditor()
{}

Model::SharedText AbstractTextEditor::text() const
{
    Q_D(const AbstractTextEditor);
    return d->text;
}

void AbstractTextEditor::onKeyPressed(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (not d->valid()) {
        return;
    }

    if (key.action() == Key::ActionBackspace) {
        sendCommitString(d->text->preedit());
        d->text->commitPreedit();
        Q_EMIT textChanged(d->text);

        d->backspace_sent = false;
        d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_delay);
    }
}

void AbstractTextEditor::onKeyReleased(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (not d->valid()) {
        return;
    }

    const QString &text(key.label().text());
    Qt::Key event_key = Qt::Key_unknown;

    switch(key.action()) {
    case Key::ActionInsert:
        d->text->appendToPreedit(text);
        sendPreeditString(d->text->preedit());
        Q_EMIT textChanged(d->text);
        break;

    case Key::ActionBackspace: {
        commitPreedit();

        if (not d->backspace_sent) {
            event_key = Qt::Key_Backspace;
        }

        d->auto_repeat_backspace_timer.stop();
     } break;

    case Key::ActionSpace:
        d->text->appendToPreedit(" ");
        commitPreedit();
        break;

    case Key::ActionReturn:
        event_key = Qt::Key_Return;
        break;

    case Key::ActionClose:
        Q_EMIT keyboardClosed();
        break;

    case Key::ActionLeft:
        event_key = Qt::Key_Left;
        break;

    case Key::ActionUp:
        event_key = Qt::Key_Up;
        break;

    case Key::ActionRight:
        event_key = Qt::Key_Right;
        break;

    case Key::ActionDown:
        event_key = Qt::Key_Down;
        break;

    default:
        break;
    }

    if (event_key != Qt::Key_unknown) {
        commitPreedit();
        QKeyEvent ev(QEvent::KeyPress, event_key, Qt::NoModifier);
        sendKeyEvent(ev);
    }
}

void AbstractTextEditor::onKeyEntered(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (key.action() == Key::ActionBackspace) {
        d->backspace_sent = false;
        d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_delay);
    }
}

void AbstractTextEditor::onKeyExited(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (key.action() == Key::ActionBackspace) {
        d->auto_repeat_backspace_timer.stop();
    }
}

void AbstractTextEditor::replacePreedit(const QString &replacement,
                                        ReplacementPolicy policy)
{
    Q_D(AbstractTextEditor);

    if (not d->valid()) {
        return;
    }

    d->text->setPreedit(replacement);

    switch (policy) {
    case ReplaceOnly:
        Q_EMIT textChanged(d->text);
        break;

    case ReplaceAndCommit:
        d->text->appendToPreedit(" ");
        commitPreedit();
        break;
    }
}

void AbstractTextEditor::commitPreedit()
{
    Q_D(AbstractTextEditor);

    if (not d->valid() || d->text->preedit().isEmpty()) {
        return;
    }

    sendCommitString(d->text->preedit());
    d->text->commitPreedit();
    Q_EMIT textChanged(d->text);
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
void AbstractTextEditor::autoRepeatBackspace()
{
    Q_D(AbstractTextEditor);

    QKeyEvent ev(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
    sendKeyEvent(ev);
    d->backspace_sent = true;
    d->auto_repeat_backspace_timer.start(d->options.backspace_auto_repeat_interval);
}

} // namespace MaliitKeyboard
