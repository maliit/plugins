/*
 * This file is part of Maliit plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Jakub Pavelek <jpavelek@live.com>
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

KeyBase  {
    id: aCharKey

    property string caption
    property string captionShifted
    property string symView
    property string symView2
    property string sizeType: "keyboard-key-43x60.png"
    property int fontSize: UI.FONT_SIZE
    property alias text: key_label.text
    property alias imagesrc: bgimage.source

    BorderImage {
        id: bgImage
        width: 38; height: 60
        border {left: 1; top:4; right:1;bottom:0}
        horizontalTileMode: BorderImage.Repeat
        verticalTileMode: BorderImage.Repeat
        source: parent.pressed ? "keyboard-key-portrait-pressed.png" : "keyboard-key-portrait.png"
        anchors.fill: parent
        anchors.leftMargin: leftPadding
        anchors.rightMargin: rightPadding
        anchors.topMargin: topPadding
        anchors.bottomMargin: bottomPadding
    }

    Text {
        id: key_label
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: "sans"
        font.pixelSize: fontSize
        font.bold: true
        color: UI.TEXT_COLOR
        text: (inSymView && symView.length) > 0 ? (inSymView2 ? symView2 : symView)
                                                : (isShifted ? captionShifted : caption)
    }
}

