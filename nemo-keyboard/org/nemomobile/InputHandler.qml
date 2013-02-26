/****************************************************************************************
**
** Copyright (C) 2013 Jolla Ltd.
** Contact: Pekka Vuorela <pekka.vuorela@jollamobile.com>
** All rights reserved.
**
** This file is part of Maliit plugins
**
** You may use this file under the terms of BSD license as follows:
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**     * Redistributions of source code must retain the above copyright
**       notice, this list of conditions and the following disclaimer.
**     * Redistributions in binary form must reproduce the above copyright
**       notice, this list of conditions and the following disclaimer in the
**       documentation and/or other materials provided with the distribution.
**     * Neither the name of the Jolla Ltd nor the
**       names of its contributors may be used to endorse or promote products
**       derived from this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
** ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
** WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR
** ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
** (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
** LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
** ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************************/

import QtQuick 1.1
import com.meego.maliitquick 1.0

Item {
    property Item pressedKey

    Timer {
        id: autorepeatTimer

        repeat: true
        onTriggered: {
            interval = 80
            if (pressedKey !== null) {
                _handleKeyClick()
            } else {
                stop()
            }
        }
    }

    function _handleKeyPress(key) {
        pressedKey = key

        if (handleKeyPress())
            return

        if (pressedKey.repeat) {
            autorepeatTimer.interval = 800
            autorepeatTimer.start()
        }
    }

    function _handleKeyRelease() {
        pressedKey = null

        if (handleKeyRelease())
            return

        autorepeatTimer.stop()
    }

    function _handleKeyClick() {
        if (handleKeyClick())
            return

        var resetShift = !keyboard.layout.isShiftLocked

        if (pressedKey.key === Qt.Key_Shift) {
            resetShift = false
        }

        if (pressedKey.text.length) {
            MInputMethodQuick.sendCommit(pressedKey.text)
        } else if (pressedKey.key === Qt.Key_Return) {
            MInputMethodQuick.activateActionKey()
        } else if (pressedKey.key === Qt.Key_Backspace) {
            if (MInputMethodQuick.surroundingTextValid && MInputMethodQuick.cursorPosition == 0) {
                resetShift = false
            }

            MInputMethodQuick.sendKey(Qt.Key_Backspace, 0, "\b", Maliit.KeyClick)
        } else  {
            resetShift = false
        }

        if (resetShift)
            keyboard.layout.isShifted = false
    }

    function _reset() {
        autorepeatTimer.stop()
        reset()
    }


    // called when button gets down. can be reimplemented to handle input. return true input is consumed
    function handleKeyPress() {
        return false
    }

    // called when button click was fully done or on autorepeat. can be reimplemented to handle input.
    // return true input was consumed.
    function handleKeyClick() {
        return false
    }

    // called when button got up either by moving out of the button or after click was done.
    // can be reimplemented. return true if input was consumed.
    function handleKeyRelease() {
        return false
    }

    // called when input state needs to be reset
    function reset() {
    }
}
