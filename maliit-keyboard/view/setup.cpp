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
#include "logic/layoutupdater.h"

namespace MaliitKeyboard {
namespace Setup {

void connectGlassToLayoutUpdater(Glass *glass,
                                 LayoutUpdater *updater)
{

    QObject::connect(glass,   SIGNAL(keyboardClosed()),
                     updater, SLOT(clearActiveKeysAndMagnifier()));

    QObject::connect(glass,   SIGNAL(switchLeft(SharedLayout)),
                     updater, SLOT(clearActiveKeysAndMagnifier()));

    QObject::connect(glass,   SIGNAL(switchRight(SharedLayout)),
                     updater, SLOT(clearActiveKeysAndMagnifier()));

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
}

void connectGlassToRenderer(Glass *glass,
                            Renderer *renderer)
{
    QObject::connect(glass, SIGNAL(keyboardClosed()),
                     renderer, SLOT(hide()));
}

void connectLayoutUpdaterToRenderer(LayoutUpdater *updater,
                                    Renderer *renderer)
{
    QObject::connect(updater,  SIGNAL(layoutChanged(SharedLayout)),
                     renderer, SLOT(onLayoutChanged(SharedLayout)));

    QObject::connect(updater,  SIGNAL(keysChanged(SharedLayout)),
                     renderer, SLOT(onKeysChanged(SharedLayout)));
}

}} // namespace Setup, MaliitKeyboard
