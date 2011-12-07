/*
 * This file is part of Maliit plugins
 *
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

import QtQuick 1.0
import "KeyboardUiConstants.js" as UI


Column {
    anchors.fill: parent
    anchors.topMargin: 8
    anchors.horizontalCenter: parent.horizontalCenter
    spacing: 16
    property variant row1:["q1€", "w2£", "e3$", "r4¥", "t5₹", "y6%", "u7<", "i8>", "o9[", "p0]"]
    property variant row2: ["a*`", "s#^", "d+|", "f-_", "g=§", "h({", "j)}", "k?¿", "l!¡"]
    property variant row3: ["z@«", "x~»", "c/\"", "v\\“", "b'”", "n;„", "m:&"]
    property variant accents_row1: ["", "", "eèéêë", "", "tþ", "yý", "uûùúü", "iîïìí", "oöôòó", ""]
    property variant accents_row2: ["aäàâáãå", "", "dð", "", "", "", "", "", ""]
    property variant accents_row3: ["", "", "cç", "", "", "nñ", ""]

    property int columns: Math.max(row1.length, row2.length, row3.length)
    property int keyWidth: (columns == 11) ? UI.portraitWidthNarrow : UI.portraitWidth
    property int keyHeight: UI.portraitHeight
    property int keyMargin: (columns == 11) ? UI.portraitMarginNarrow : UI.portraitMargin
    property bool isShifted: false
    property bool isShiftLocked: false
    property bool inSymView: false
    property bool inSymView2: false

    property string layoutName: "English (UK)"

    Row { //Row 1
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: keyMargin
        Repeater {
            model: row1
            CharacterKey {
                width: keyWidth; height: keyHeight
                caption: row1[index][0]
                captionShifted: row1[index][0].toUpperCase()
                symView: row1[index][1]
                symView2: row1[index][2]
            }
        }
    } //end Row1

    Row { //Row 2
        anchors.horizontalCenter: parent.horizontalCenter

        spacing: keyMargin
        Repeater {
            model: row2
            CharacterKey {
                width: keyWidth; height: keyHeight
                caption: row2[index][0]
                captionShifted: row2[index][0].toUpperCase()
                symView: row2[index][1]
                symView2: row2[index][2]
            }
        }
    }

    Row { //Row 3
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: (columns == 11) ? 32 : 16
        FunctionKey {
            width: UI.PORTRAIT_SHIFT_WIDTH; height: keyHeight
            icon: inSymView ? ""
                            : (isShiftLocked) ? "icon-m-input-methods-capslock.svg"
                                              : (isShifted) ? "icon-m-input-methods-shift-uppercase.svg"
                                                            : "icon-m-input-methods-shift-lowercase.svg"

            caption: inSymView ? (inSymView2 ? "2/2" : "1/2") : ""

            onClickedPass: {
                if (inSymView) {
                    inSymView2 = !inSymView2
                } else {
                    isShifted = (!isShifted)
                    isShiftLocked = false
                }
            }
            onPressedAndHoldPass: {
                if (!inSymView) {
                    isShifted = true
                    isShiftLocked = true
                }
            }
        }
        Row {
            spacing: keyMargin
            Repeater {
                model: row3
                CharacterKey {
                    width: keyWidth; height: keyHeight
                    caption: row3[index][0]
                    captionShifted: row3[index][0].toUpperCase()
                    symView: row3[index][1]
                    symView2: row3[index][2]
                }
            }
        }
        FunctionKey {
            width: UI.PORTRAIT_SHIFT_WIDTH; height: keyHeight
            icon: "icon-m-input-methods-backspace.svg"
            onClickedPass: { MInputMethodQuick.sendCommit("\b"); }
        }
    }

    Row { //Row 4
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: (columns == 11) ? 19 : 16
        FunctionKey {
            width: UI.PORTRAIT_OTT_WIDTH; height: keyHeight
            caption: "?123"
            onClickedPass: { inSymView = (!inSymView) }
        }
        Row {
            spacing: 8
            CharacterKey { caption: ","; captionShifted: ","; width: 56; height: keyHeight; sizeType: "keyboard-key-56x60.png" }
            CharacterKey { caption: " "; width: 136; height: keyHeight; sizeType: "keyboard-key-136x60.png" }
            CharacterKey { caption: "."; captionShifted: "."; width: 56; height: keyHeight; sizeType: "keyboard-key-56x60.png" }
        }
        FunctionKey {
            width: UI.PORTRAIT_OTT_WIDTH; height: keyHeight
            icon: MInputMethodQuick.actionKeyOverride.icon
            caption: MInputMethodQuick.actionKeyOverride.label
            onReleased: {
                MInputMethodQuick.activateActionKey()
            }
        }
    }
}

