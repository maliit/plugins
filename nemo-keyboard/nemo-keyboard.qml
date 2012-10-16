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

        LandscapeVKB {
            id: vkb_landscape
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        PortraitVKB {
            id: vkb_portrait
            anchors.bottom: parent.bottom
            anchors.horizontalCenter: parent.horizontalCenter
        }

        Component.onCompleted: {
            MInputMethodQuick.actionKeyOverride.setDefaultIcon("icon-m-input-methods-enter.svg")
            MInputMethodQuick.actionKeyOverride.setDefaultLabel("")
        }
    }

    focus: true
    state: MInputMethodQuick.appOrientation

    states: [
        State {
            name: "0"

            StateChangeScript {
                script: MInputMethodQuick.setInputMethodArea(
                    Qt.rect((MInputMethodQuick.screenWidth - vkb_landscape.width) / 2,
                            MInputMethodQuick.screenHeight - vkb_landscape.height,
                            vkb_landscape.width, vkb_landscape.height))
            }

            PropertyChanges {
                target: root
                rotation: 0
                width: parent.width
                height: parent.height
                x: 0
                y: 0
            }

            PropertyChanges {
                target: vkb_portrait;
                opacity: 0
            }

            PropertyChanges {
                target: vkb_landscape;
                opacity: 1
            }
        },

        State {
            name: "90"

            StateChangeScript {
                script: MInputMethodQuick.setInputMethodArea(
                    Qt.rect(0, (MInputMethodQuick.screenHeight - vkb_portrait.width) / 2,
                            vkb_portrait.height, vkb_portrait.width))
            }

            PropertyChanges {
                target: root
                rotation: 90
                width: parent.height
                height: parent.width
                x: (parent.width - parent.height) / 2
                y: (parent.height - parent.width) / 2
            }

            PropertyChanges {
                target: vkb_portrait
                opacity: 1
            }

            PropertyChanges {
                target: vkb_landscape;
                opacity: 0
            }
        },

        State {
            name: "180"

            StateChangeScript {
                script: MInputMethodQuick.setInputMethodArea(
                    Qt.rect((MInputMethodQuick.screenWidth - vkb_landscape.width) / 2,
                            0, vkb_landscape.width, vkb_landscape.height))
            }

            PropertyChanges {
                target: root
                rotation: 180
                width: parent.width
                height: parent.height
                x: 0
                y: 0
            }

            PropertyChanges {
                target: vkb_portrait;
                opacity: 0
            }

            PropertyChanges {
                target: vkb_landscape;
                opacity: 1
            }
        },

        State {
            name: "270"

            StateChangeScript {
                script: MInputMethodQuick.setInputMethodArea(
                    Qt.rect(MInputMethodQuick.screenWidth - vkb_portrait.height,
                            (MInputMethodQuick.screenHeight - vkb_portrait.width) / 2,
                            vkb_portrait.height, vkb_portrait.width))
            }

            PropertyChanges {
                target: root
                rotation: 270
                width: parent.height
                height: parent.width
                x: (parent.width - parent.height) / 2
                y: (parent.height - parent.width) / 2
            }

            PropertyChanges {
                target: vkb_portrait
                opacity: 1
            }

            PropertyChanges {
                target: vkb_landscape;
                opacity: 0
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "*"

            RotationAnimation {
                target: root;
                duration: 400;
                easing.type: Easing.InOutQuad
            }

            PropertyAnimation {
                targets: [vkb_landscape, vkb_portrait];
                properties: "opacity";
                duration: 400;
                easing.type: Easing.InOutQuad
            }
        }
    ]

}

