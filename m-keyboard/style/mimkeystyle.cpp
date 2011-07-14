/*
 * This file is part of meego-keyboard 
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

 */

#include "mimkeystyle.h"

namespace {
    const char * const ThemeDirectory =
        "/usr/share/themes/blanco/meegotouch/images/theme/meegotouch-virtual-keyboard/";

    // TODO: unify with theme folder? Or have only one big SVG even?
    const char * const IconsDirectory =
        "/usr/share/themes/base/meegotouch/icons/";
}

MImKeyStylingContext::MImKeyStylingContext(MaliitKeyboard::KeyState newState,
                                           MaliitKeyboard::KeyStyle newStyle,
                                           MaliitKeyboard::KeyOverrides newOverrides)
    : state(newState)
    , style(newStyle)
    , overrides(newOverrides)
{}

MImKeyGeometryContext::MImKeyGeometryContext(MaliitKeyboard::KeyWidth newWidth,
                                             MaliitKeyboard::KeyHeight newHeight,
                                             MInputMethod::Orientation newOrientation)
    : width(newWidth)
    , height(newHeight)
    , orientation(newOrientation)
{}

MImKeyStyle::MImKeyStyle(const QLatin1String &newStyleClassName)
    : styleClassName(newStyleClassName)
{}

MImKeyStyle::~MImKeyStyle()
{}

QPixmap MImKeyStyle::background(const MImKeyStylingContext &context) const
{
    QString fileName = QString("%1/%2-%3.png").arg(ThemeDirectory).arg("meegotouch-keyboard");

    QStringList keyName;
    keyName.append("key");

    if (context.style == MaliitKeyboard::KeyStyleSpecial) {
        keyName.replace(0, "function-key");
    }

    // FIXME: action-key is never used in original LMT-based styling code?

    if (context.overrides & MaliitKeyboard::KeyOverrideHighlighted) {
        keyName.append("highlighted");
    }

    switch (context.state) {
    case MaliitKeyboard::KeyStatePressed:
        keyName.append("pressed");
        break;

    // FIXME: Ignoring press-selected state, because I don't think we really need it.
    case MaliitKeyboard::KeyStateSelected:
        keyName.append("selected");
        break;

    case MaliitKeyboard::KeyStateDisabled:
        keyName.append("disabled");
        break;

    default:
        break;
    }

    return QPixmap(fileName.arg(keyName.join("-")));
}

QPixmap MImKeyStyle::icon(const MImKeyStylingContext &context,
                          MaliitKeyboard::KeyAction action,
                          const QSizeF size) const
{
    // TODO: compute correct file name, take context and size constraint into account
    Q_UNUSED(context)
    Q_UNUSED(size)

    QString fileName = QString("%1/%2").arg(IconsDirectory);

    switch (action) {
    case MaliitKeyboard::KeyActionReturn:
        fileName.arg("icon-m-input-methods-enter.svg");
        break;

    case MaliitKeyboard::KeyActionShift:
        fileName.arg("icon-m-input-methods-shift.svg");
        break;

    case MaliitKeyboard::KeyActionBackspace:
        fileName.arg("icon-m-input-methods-backspace.svg");
        break;

    default:
        fileName.arg("");
        break;
    }

    return QPixmap(fileName);
}

QFont MImKeyStyle::font(const MImKeyStylingContext &context,
                        const QSizeF size) const
{
    // TODO: compute correct font
    Q_UNUSED(context)
    Q_UNUSED(size)
    return QFont("Nokia pure", 26);
}

QColor MImKeyStyle::fontColor(const MImKeyStylingContext &context) const
{
    // TODO: compute correct color
    Q_UNUSED(context)
    return QColor(Qt::white);
}

QSizeF MImKeyStyle::size(const MImKeyGeometryContext &context) const
{
    // TODO: compute correct size
    Q_UNUSED(context)
    return QSizeF(70, 40);
}

QMargins MImKeyStyle::margins(const MImKeyGeometryContext &context) const
{
    // TODO: compute correct margins
    Q_UNUSED(context)
    return QMargins(6, 0, 6, 12);
}
