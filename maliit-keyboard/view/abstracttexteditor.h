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

#ifndef MALIIT_KEYBOARD_TEXTEDITOR_H
#define MALIIT_KEYBOARD_TEXTEDITOR_H

#include "models/key.h"
#include "models/wordcandidate.h"
#include "models/text.h"

#include <QtCore>
#include <QtGui/QKeyEvent>

namespace MaliitKeyboard {

struct EditorOptions
{
    EditorOptions();
    // all delays are in milliseconds
    int backspace_auto_repeat_delay; // delay before first automatically repeated key
    int backspace_auto_repeat_interval; // interval between automatically repeated keys
};

class AbstractTextEditorPrivate;

class AbstractTextEditor
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractTextEditor)
    Q_DECLARE_PRIVATE(AbstractTextEditor)

public:
    enum ReplacementPolicy {
        ReplaceAndCommit, // Will replace, add space & commit
        ReplaceOnly,
    };

    Q_ENUMS(ReplacementPolicy)

    explicit AbstractTextEditor(const EditorOptions &options,
                                const Model::SharedText &text,
                                QObject *parent = 0);
    virtual ~AbstractTextEditor() = 0;

    Model::SharedText text() const;

    Q_SLOT void onKeyPressed(const Key &key);
    Q_SLOT void onKeyReleased(const Key &key);
    Q_SLOT void onKeyEntered(const Key &key);
    Q_SLOT void onKeyExited(const Key &key);
    Q_SLOT void replacePreedit(const QString &replacement,
                               ReplacementPolicy policy = ReplaceAndCommit);
    Q_SLOT void clearPreedit();

    Q_SIGNAL void textChanged(const Model::SharedText &text);
    Q_SIGNAL void keyboardClosed();

private:
    const QScopedPointer<AbstractTextEditorPrivate> d_ptr;

    virtual void sendPreeditString(const QString &preedit) = 0;
    virtual void sendCommitString(const QString &commit) = 0;
    virtual void sendKeyEvent(const QKeyEvent &ev) = 0;

    void commitPreedit();
    Q_SLOT void autoRepeatBackspace();
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_ABSTRACTTEXTEDITOR_H
