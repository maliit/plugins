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

import QtQuick 2.0
import "KeyboardUiConstants.js" as UI
import com.meego.maliitquick 1.0 as Maliit

Maliit.Tooltip {
    id: popper
    property Item target: null
    width: MInputMethodQuick.appOrientation == 0 || MInputMethodQuick.appOrientation == 180 ? popperImage.width : popperImage.height
    height: MInputMethodQuick.appOrientation == 0 || MInputMethodQuick.appOrientation == 180 ? popperImage.height : popperImage.width
    
    alignment: {
        switch (MInputMethodQuick.appOrientation) {
            case 0: return Qt.AlignHCenter|Qt.AlignTop;
            case 180: return Qt.AlignHCenter|Qt.AlignBottom;
            case 90: return Qt.AlignVCenter|Qt.AlignRight;
            case 270: return Qt.AlignVCenter|Qt.AlignLeft;
            case 0:
            default: return Qt.AlignHCenter|Qt.AlignTop;
        }
    }

    Image {
        id: popperImage
        source: "popper.png"
        opacity: 0
        anchors.centerIn: parent
        rotation: MInputMethodQuick.appOrientation
        transformOrigin: Item.Center

        Text {
            id: popperText
            text: ""
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.family: "sans"
            font.pixelSize: UI.FONT_SIZE_POPPER
            font.bold: true
            color: UI.TEXT_COLOR_POPPER
        }

        states: State {
            name: "active"
            when: target !== null && target.showPopper

            PropertyChanges {
                target: popperText
                text: target.text
            }

            PropertyChanges {
                target: popperImage
                opacity: 1
            }

            PropertyChanges {
                target: popper

                visualParent: target
                visible: true
            }
        }

        transitions: Transition {
            from: "active"

            SequentialAnimation {
                PauseAnimation {
                    duration: 50
                }
                PropertyAction {
                    target: popperImage
                    properties: "opacity"
                }
                PropertyAction {
                    target: popperText
                    property: "text"
                }
            }
        }
    }
}
