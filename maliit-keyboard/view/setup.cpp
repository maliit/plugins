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

#include "setup.h"
#include "glass.h"
#include "renderer.h"
#include "abstracttexteditor.h"
#include "abstractfeedback.h"

#include "models/key.h"
#include "models/wordcandidate.h"
#include "models/layout.h"
#include "models/text.h"

#include "logic/layoutupdater.h"
#include "logic/wordengine.h"

namespace MaliitKeyboard {
namespace Setup {

void connectAll(Glass *glass,
                LayoutUpdater *updater,
                Renderer *renderer,
                AbstractTextEditor *editor,
                Logic::WordEngine *engine,
                AbstractFeedback *feedback)
{
    connectGlassToLayoutUpdater(glass, updater);
    connectGlassToTextEditor(glass, editor);
    connectGlassToFeedback(glass, feedback);

    connectLayoutUpdaterToRenderer(updater, renderer);
    connectLayoutUpdaterToTextEditor(updater, editor);

    connectWordEngineToLayoutUpdater(engine, updater);
    connectTextEditorToWordEngine(editor, engine);
}

void connectGlassToLayoutUpdater(Glass *glass,
                                 LayoutUpdater *updater)
{
    QObject::connect(glass,   SIGNAL(switchLeft(SharedLayout)),
                     updater, SLOT(clearActiveKeysAndMagnifier()));

    QObject::connect(glass,   SIGNAL(switchRight(SharedLayout)),
                     updater, SLOT(clearActiveKeysAndMagnifier()));

    // Connect key signals to key signal handlers:
    QObject::connect(glass,   SIGNAL(keyPressed(Key,SharedLayout)),
                     updater, SLOT(onKeyPressed(Key,SharedLayout)));

    QObject::connect(glass,   SIGNAL(keyLongPressed(Key,SharedLayout)),
                     updater, SLOT(onKeyLongPressed(Key,SharedLayout)));

    QObject::connect(glass,   SIGNAL(keyReleased(Key,SharedLayout)),
                     updater, SLOT(onKeyReleased(Key,SharedLayout)));

    QObject::connect(glass,   SIGNAL(keyEntered(Key,SharedLayout)),
                     updater, SLOT(onKeyEntered(Key,SharedLayout)));

    QObject::connect(glass,   SIGNAL(keyExited(Key,SharedLayout)),
                     updater, SLOT(onKeyExited(Key,SharedLayout)));

    // Connect word candidate signals to word candidate handlers:
    QObject::connect(glass,   SIGNAL(wordCandidatePressed(WordCandidate,SharedLayout)),
                     updater, SLOT(onWordCandidatePressed(WordCandidate,SharedLayout)));

    QObject::connect(glass,   SIGNAL(wordCandidateReleased(WordCandidate,SharedLayout)),
                     updater, SLOT(onWordCandidateReleased(WordCandidate,SharedLayout)));
}

void connectGlassToTextEditor(Glass *glass,
                              AbstractTextEditor *editor)
{
    QObject::connect(glass,  SIGNAL(keyPressed(Key,SharedLayout)),
                     editor, SLOT(onKeyPressed(Key)));

    QObject::connect(glass,  SIGNAL(keyReleased(Key,SharedLayout)),
                     editor, SLOT(onKeyReleased(Key)));

    QObject::connect(glass,  SIGNAL(keyEntered(Key,SharedLayout)),
                     editor, SLOT(onKeyEntered(Key)));

    QObject::connect(glass,  SIGNAL(keyExited(Key,SharedLayout)),
                     editor, SLOT(onKeyExited(Key)));

    QObject::connect(editor, SIGNAL(keyboardClosed()),
                     glass,  SIGNAL(keyboardClosed()));
}

void connectGlassToFeedback (Glass *glass,
                             AbstractFeedback *feedback)
{
    QObject::connect(glass,    SIGNAL(keyPressed(Key,SharedLayout)),
                     feedback, SLOT(onKeyPressed()));
    QObject::connect(glass,    SIGNAL(keyReleased(Key,SharedLayout)),
                     feedback, SLOT(onKeyReleased()));
    QObject::connect(glass,    SIGNAL(switchLeft(SharedLayout)),
                     feedback, SLOT(onLayoutChanged()));
    QObject::connect(glass,    SIGNAL(switchRight(SharedLayout)),
                     feedback, SLOT(onLayoutChanged()));
    QObject::connect(glass,    SIGNAL(keyboardClosed()),
                     feedback, SLOT(onKeyboardHidden()));
}

void connectLayoutUpdaterToTextEditor(LayoutUpdater *updater,
                                      AbstractTextEditor *editor)
{
    QObject::connect(updater, SIGNAL(wordCandidateSelected(QString)),
                     editor,  SLOT(replacePreedit(QString)));
}

void connectLayoutUpdaterToRenderer(LayoutUpdater *updater,
                                    Renderer *renderer)
{
    QObject::connect(updater,  SIGNAL(layoutChanged(SharedLayout)),
                     renderer, SLOT(onLayoutChanged(SharedLayout)));

    QObject::connect(updater,  SIGNAL(keysChanged(SharedLayout)),
                     renderer, SLOT(onKeysChanged(SharedLayout)));

    QObject::connect(updater,  SIGNAL(wordCandidatesChanged(SharedLayout)),
                     renderer, SLOT(onWordCandidatesChanged(SharedLayout)));
}

void connectWordEngineToLayoutUpdater(Logic::WordEngine *engine,
                                      LayoutUpdater *updater)
{
    QObject::connect(engine,  SIGNAL(candidatesUpdated(QStringList)),
                     updater, SLOT(onCandidatesUpdated(QStringList)));
}

void connectTextEditorToWordEngine(AbstractTextEditor *editor,
                                   Logic::WordEngine *engine)
{
    QObject::connect(editor, SIGNAL(textChanged(Model::SharedText)),
                     engine, SLOT(onTextChanged(Model::SharedText)));
}

}} // namespace Setup, MaliitKeyboard
