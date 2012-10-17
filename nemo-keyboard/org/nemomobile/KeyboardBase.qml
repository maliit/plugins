/*
 * This file is part of Maliit plugins
 *
 * Copyright (C) 2012 John Brooks <john.brooks@dereferenced.net>
 * Copyright (C) Jakub Pavelek <jpavelek@live.com>
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

import QtQuick 1.1
import "KeyboardUiConstants.js" as UI

MouseArea {
    id: keyboardBase

    property Item layout

    property Item pressedKey

    onLayoutChanged: if (layout) layout.parent = keyboardBase

    Rectangle {
        id: background
        anchors.fill: parent
        color: UI.BG_COLOR
    }

    Popper {
        id: popper
        z: 10
        target: pressedKey
    }

    /* Mouse handling */
    property int _startX
    property int _startY

    onPressed: {
        _startX = mouse.x
        _startY = mouse.y
        updatePressedKey(mouse.x, mouse.y);
    }

    onPositionChanged: {
        // Hide keyboard if flicked down by >20%
        if (mouse.y - _startY > (height * 0.2)) {
            MInputMethodQuick.userHide();
            pressedKey.pressed = false;
            pressedKey = null;
            return;
        }

        updatePressedKey(mouse.x, mouse.y);
    }

    onReleased: {
        if (pressedKey == null)
            return;

        MInputMethodQuick.sendCommit(pressedKey.text);
        if (!layout.isShiftLocked)
            layout.isShifted = false;

        pressedKey.pressed = false;
        pressedKey = null;
    }

    function updatePressedKey(x, y) {
        var key = keyAt(x, y);
        if (pressedKey !== null && pressedKey !== key)
            pressedKey.pressed = false;
        pressedKey = key;
        if (pressedKey !== null)
            pressedKey.pressed = true;
    }

    function keyAt(x, y) {
        var item = layout;
        x -= layout.x;
        y -= layout.y;

        while ((item = item.childAt(x, y)) != null) {
            if (typeof item.text !== 'undefined') {
                return item;
            }

            // Cheaper mapToItem, assuming we're not using anything fancy.
            x -= item.x;
            y -= item.y;
        }

        return null;
    }
}
 
