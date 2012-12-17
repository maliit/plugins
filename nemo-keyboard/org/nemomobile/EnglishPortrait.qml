/*
 * This file is part of Maliit plugins
 *
 * Copyright (C) Jakub Pavelek <jpavelek@live.com>
 * Copyright (C) 2012 John Brooks <john.brooks@dereferenced.net>
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

Column {
    id: keyArea

    width: UI.PORTRAIT_WIDTH
    height: UI.PORTRAIT_HEIGHT

    property int topPadding: UI.portraitVerticalPadding
    property int bottomPadding: UI.portraitVerticalPadding
    property int leftPadding: UI.portraitHorizontalPadding
    property int rightPadding: UI.portraitHorizontalPadding

    property bool isShifted
    property bool isShiftLocked
    property bool inSymView
    property bool inSymView2

    property variant row1: ["q1€", "w2£", "e3$", "r4¥", "t5₹", "y6%", "u7<", "i8>", "o9[", "p0]"]
    property variant row2: ["a*`", "s#^", "d+|", "f-_", "g=§", "h({", "j)}", "k?¿", "l!¡"]
    property variant row3: ["z@«", "x~»", "c/\"", "v\\“", "b'”", "n;„", "m:&"]
    property variant accents_row1: ["", "", "eèéêë", "", "tþ", "yý", "uûùúü", "iîïìí", "oöôòó", ""]
    property variant accents_row2: ["aäàâáãå", "", "dð", "", "", "", "", "", ""]
    property variant accents_row3: ["", "", "cç", "", "", "nñ", ""]

    property int keyHeight: UI.portraitHeight

    Row { //Row 1
        anchors.horizontalCenter: parent.horizontalCenter
        Repeater {
            model: row1
            PortraitCharacterKey {
                caption: row1[index][0]
                captionShifted: row1[index][0].toUpperCase()
                symView: row1[index][1]
                symView2: row1[index][2]
            }
        }
    } //end Row1

    Row { //Row 2
        anchors.horizontalCenter: parent.horizontalCenter
        Repeater {
            model: row2
            PortraitCharacterKey {
                caption: row2[index][0]
                captionShifted: row2[index][0].toUpperCase()
                symView: row2[index][1]
                symView2: row2[index][2]
            }
        }
    }

    Row { //Row 3
        anchors.horizontalCenter: parent.horizontalCenter
        FunctionKey {
            width: UI.PORTRAIT_SHIFT_WIDTH
            height: keyHeight
            topPadding: keyArea.topPadding
            leftPadding: keyArea.leftPadding
            rightPadding: keyArea.rightPadding
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
            Repeater {
                model: row3
                PortraitCharacterKey {
                    caption: row3[index][0]
                    captionShifted: row3[index][0].toUpperCase()
                    symView: row3[index][1]
                    symView2: row3[index][2]
                }
            }
        }

        FunctionKey {
            width: UI.PORTRAIT_SHIFT_WIDTH
            height: keyHeight
            topPadding: keyArea.topPadding
            leftPadding: keyArea.leftPadding
            rightPadding: keyArea.rightPadding

            icon: "icon-m-input-methods-backspace.svg"
            repeat: true
            onClickedPass: MInputMethodQuick.sendCommit("\b")
        }
    }

    Row { //Row 4
        anchors.horizontalCenter: parent.horizontalCenter
        FunctionKey {
            width: UI.PORTRAIT_OTT_WIDTH + 20 // extra reactive area on left
            height: keyHeight
            topPadding: keyArea.topPadding
            leftPadding: keyArea.leftPadding + 20
            rightPadding: keyArea.rightPadding

            caption: inSymView ? "ABC" : "?123"
            onClickedPass: { inSymView = (!inSymView) }
        }

        PortraitCharacterKey {
            caption: ","
            captionShifted: ","
            width: 56
            sizeType: "keyboard-key-56x60.png"
        }
        PortraitCharacterKey {
            caption: " "
            captionShifted: " "
            width: 136
            sizeType: "keyboard-key-136x60.png"
        }
        PortraitCharacterKey {
            caption: "."
            captionShifted: "."
            width: 56
            sizeType: "keyboard-key-56x60.png"
        }

        FunctionKey {
            width: UI.PORTRAIT_OTT_WIDTH + 20
            height: keyHeight
            topPadding: keyArea.topPadding
            leftPadding: keyArea.leftPadding
            rightPadding: keyArea.rightPadding + 20

            icon: MInputMethodQuick.actionKeyOverride.icon
            repeat: true
            caption: MInputMethodQuick.actionKeyOverride.label
            onClickedPass: MInputMethodQuick.activateActionKey()
        }
    }
}

