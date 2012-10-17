/*
 * This file is part of Maliit plugins
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

import QtQuick 1.1
import org.nemomobile 1.0

Item {
    id: canvas
    transformOrigin: Item.Center
    width: MInputMethodQuick.screenWidth
    height: MInputMethodQuick.screenHeight

    Item {
        id: root
        transformOrigin: Item.Center
        width: parent.width
        height: parent.height

        KeyboardBase {
            id: keyboard
            width: layout ? layout.width : 0
            height: layout ? layout.height : 0
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter

            EnglishLandscape {
                id: vkb_landscape
                visible: keyboard.layout == vkb_landscape
            }

            EnglishPortrait {
                id: vkb_portrait
                visible: keyboard.layout == vkb_portrait
            }
        }

        Component.onCompleted: {
            MInputMethodQuick.actionKeyOverride.setDefaultIcon("icon-m-input-methods-enter.svg")
            MInputMethodQuick.actionKeyOverride.setDefaultLabel("")
        }
    }

    focus: true

    function updateIMArea() {
        var x = 0, y = 0, width = 0, height = 0;
        var angle = MInputMethodQuick.appOrientation;

        switch (angle) {
        case 0:
            y = MInputMethodQuick.screenHeight - vkb_landscape.height;
        case 180:
            x = (MInputMethodQuick.screenWidth - vkb_landscape.width) / 2;
            width = vkb_landscape.width;
            height = vkb_landscape.height;
            break;

        case 270:
            x = MInputMethodQuick.screenWidth - vkb_portrait.height;
        case 90:
            y = (MInputMethodQuick.screenHeight - vkb_portrait.width) / 2;
            width = vkb_portrait.height;
            height = vkb_portrait.width;
            break;
        }

        MInputMethodQuick.setInputMethodArea(Qt.rect(x, y, width, height));
    }

    states: [
        State {
            name: "landscape"
            when: MInputMethodQuick.appOrientation == 0 || MInputMethodQuick.appOrientation == 180

            StateChangeScript {
                script: updateIMArea();
            }

            PropertyChanges {
                target: root
                rotation: MInputMethodQuick.appOrientation
                x: 0
                y: 0
            }

            PropertyChanges {
                target: keyboard
                layout: vkb_landscape
            }
        },

        State {
            name: "portrait"
            when: MInputMethodQuick.appOrientation == 90 || MInputMethodQuick.appOrientation == 270

            StateChangeScript {
                script: updateIMArea();
            }

            PropertyChanges {
                target: root
                rotation: MInputMethodQuick.appOrientation
                x: (parent.width - parent.height) / 2
            }

            PropertyChanges {
                target: keyboard
                layout: vkb_portrait
            }
        }
    ]
}

