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

import QtQuick 2.0
import "KeyboardUiConstants.js" as UI
import com.meego.maliitquick 1.0

MouseArea {
    id: keyboard

    property Item layout
    property Item pressedKey

    onLayoutChanged: if (layout) layout.parent = keyboard

    // Can be changed to PreeditTestHandler to have another mode of input
    InputHandler {
        id: inputHandler
    }

    Popper {
        id: popper
        z: 10
        target: pressedKey
    }
    Timer {
        id: pressTimer
        interval: 500
    }

    Rectangle {
        id: tracker
        width: 30
        height: 30
        radius: 30
        border.width: 2
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        z: 100
        color: "#0078bd"
        Timer {
            id: movetimer
            interval: 150
            property int key
            onTriggered: {
                MInputMethodQuick.sendKey(key)
            }
        }

        MouseArea {
            width: 45
            height: 45
            anchors.horizontalCenter: parent.horizontalCenter
            anchors.verticalCenter: parent.verticalCenter
            property int _startlX
            property int _startlY
            onPressed: {
                _startlX = mouse.x
                _startlY = mouse.y
            }

            onPositionChanged: {
                if ((mouse.y + _startlY < (height))) {
                    movetimer.key = Qt.Key_Up
                    movetimer.start()
                }else if ((mouse.y - _startlY > (height))) {
                    movetimer.key = Qt.Key_Down
                    movetimer.start()
                }else if ((mouse.x - _startlX < (width))) {
                    movetimer.key = Qt.Key_Left
                    movetimer.start()
                }else if ((mouse.x + _startlX > (width))) {
                    movetimer.key = Qt.Key_Right
                    movetimer.start()
                }
            }
        }
    }

    BorderImage {
        width: 480; height: 318;
        border { left: 1; top: 4; right: 1; bottom:0 }
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat
        source: "vkb-body.png"
    }

    Connections {
        target: MInputMethodQuick
        onCursorPositionChanged: {
            applyAutocaps()
        }
        onFocusTargetChanged: {
            if (activeEditor) {
                resetKeyboard()
                applyAutocaps()
            }
        }
        onInputMethodReset: {
            inputHandler._reset()
        }
    }

    /* Mouse handling */
    property int _startX
    property int _startY

    onPressed: {
        if (keyboard.childAt(mouse.x, mouse.y) == tracker) {
            mouse.accepted = false
        }

        _startX = mouse.x
        _startY = mouse.y
        pressTimer.start()
        updatePressedKey(mouse.x, mouse.y)
    }

    onPositionChanged: {
        // Hide keyboard on flick down
        if (pressTimer.running && (mouse.y - _startY > (height * 0.3))) {
            MInputMethodQuick.userHide()
            if (pressedKey) {
                inputHandler._handleKeyRelease()
                pressedKey.pressed = false
            }
            pressedKey = null
            return
        }

        updatePressedKey(mouse.x, mouse.y)
    }

    onCanceled: {
        if (pressedKey) {
            pressedKey.pressed = false
            pressedKey = null
        }
    }

    onReleased: {
        if (pressedKey === null)
            return

        inputHandler._handleKeyClick()
        pressedKey.clicked()
        inputHandler._handleKeyRelease()

        pressedKey.pressed = false
        pressedKey = null
    }

    function updatePressedKey(x, y) {
        var key = keyAt(x, y)
        if (pressedKey === key)
            return

        inputHandler._handleKeyRelease()

        if (pressedKey !== null)
            pressedKey.pressed = false

        pressedKey = key

        if (pressedKey !== null) {
            pressedKey.pressed = true
            inputHandler._handleKeyPress(pressedKey)
        }
    }

    function keyAt(x, y) {
        var item = layout
        x -= layout.x
        y -= layout.y

        while ((item = item.childAt(x, y)) != null) {
            if (typeof item.text !== 'undefined' && typeof item.pressed !== 'undefined') {
                return item
            }

            // Cheaper mapToItem, assuming we're not using anything fancy.
            x -= item.x
            y -= item.y
        }

        return null
    }

    function resetKeyboard() {
        if (!layout)
            return
        layout.isShifted = false
        layout.isShiftLocked = false
        layout.inSymView = false
        layout.inSymView2 = false
        inputHandler._reset()
    }

    function applyAutocaps() {
        if (MInputMethodQuick.surroundingTextValid
                && MInputMethodQuick.contentType === Maliit.FreeTextContentType
                && MInputMethodQuick.autoCapitalizationEnabled
                && !MInputMethodQuick.hiddenText
                && layout && layout.isShiftLocked === false) {
            var position = MInputMethodQuick.cursorPosition
            var text = MInputMethodQuick.surroundingText.substring(0, position)

            if (position == 0
                    || (position == 1 && text[0] === " ")
                    || (position >= 2 && text[position - 1] === " "
                        && ".?!".indexOf(text[position - 2]) >= 0)) {
                layout.isShifted = true
            } else {
                layout.isShifted = false
            }
        }
    }
}
