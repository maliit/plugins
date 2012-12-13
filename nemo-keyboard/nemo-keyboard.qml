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

import QtQuick 2.0
import org.nemomobile 1.0

Item {
    id: canvas

    width: MInputMethodQuick.screenWidth
    height: MInputMethodQuick.screenHeight

    function updateIMArea() {
        if (!MInputMethodQuick.active)
            return

        var x = 0, y = 0, width = 0, height = 0;
        var angle = MInputMethodQuick.appOrientation

        switch (angle) {
        case 0:
            y = MInputMethodQuick.screenHeight - vkb_landscape.height
        case 180:
            x = (MInputMethodQuick.screenWidth - vkb_landscape.width) / 2
            width = vkb_landscape.width
            height = vkb_landscape.height
            break;

        case 270:
            x = MInputMethodQuick.screenWidth - vkb_portrait.height
        case 90:
            y = (MInputMethodQuick.screenHeight - vkb_portrait.width) / 2
            width = vkb_portrait.height
            height = vkb_portrait.width
            break;
        }

        MInputMethodQuick.setInputMethodArea(Qt.rect(x, y, width, height))
        MInputMethodQuick.setScreenRegion(Qt.rect(x, y, width, height))
    }

    Item {
        // container at the of current orientation. allows actual keyboard to show relative to that.
        id: root

        property bool landscape: MInputMethodQuick.appOrientation == 0 || MInputMethodQuick.appOrientation == 180

        width: landscape ? parent.width : parent.height
        height: 1
        transformOrigin: Item.TopLeft
        rotation: MInputMethodQuick.appOrientation
        x: MInputMethodQuick.appOrientation == 180 || MInputMethodQuick.appOrientation == 270
           ? parent.width : 0
        y: MInputMethodQuick.appOrientation == 0 || MInputMethodQuick.appOrientation == 270
           ? parent.height : 0

        onRotationChanged: updateIMArea()

        KeyboardBase {
            id: keyboard
            layout: root.landscape ? vkb_landscape : vkb_portrait
            width: layout ? layout.width : 0
            height: layout ? layout.height : 0
            anchors.horizontalCenter: parent.horizontalCenter

            onHeightChanged: {
                if (MInputMethodQuick.active)
                    y = -height
            }

            EnglishLandscape {
                id: vkb_landscape
                visible: keyboard.layout == vkb_landscape
            }

            EnglishPortrait {
                id: vkb_portrait
                visible: keyboard.layout == vkb_portrait
            }

            Connections {
                target: MInputMethodQuick
                onActiveChanged: {
                    if (MInputMethodQuick.active) {
                        hideAnimation.stop()
                        showAnimation.start()
                    } else {
                        showAnimation.stop()
                        hideAnimation.start()
                    }
                }
            }

            SequentialAnimation {
                id: hideAnimation

                ScriptAction {
                    script: MInputMethodQuick.setInputMethodArea(Qt.rect(0, 0, 0, 0))
                }

                NumberAnimation {
                    target: keyboard
                    property: "y"
                    to: 0
                    duration: 500
                    easing.type: Easing.InOutCubic
                }

                ScriptAction {
                    script: MInputMethodQuick.setScreenRegion(Qt.rect(0, 0, 0, 0))
                }
            }

            SequentialAnimation {
                id: showAnimation

                ScriptAction {
                    script: {
                        canvas.visible = true // framework currently initially hides. Make sure visible
                        updateIMArea()
                    }
                }

                NumberAnimation {
                    target: keyboard
                    property: "y"
                    to: -keyboard.height
                    duration: 500
                    easing.type: Easing.InOutCubic
                }
            }
        }

        Component.onCompleted: {
            MInputMethodQuick.actionKeyOverride.setDefaultIcon("icon-m-input-methods-enter.svg")
            MInputMethodQuick.actionKeyOverride.setDefaultLabel("")
        }
    }
}

