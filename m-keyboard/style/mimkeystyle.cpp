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

#ifndef EXPERIMENTAL_STYLING
#include "getcssproperty.h"
#include <MScalableImage>
#endif

namespace {
    const char * const ThemeDirectory =
        "/usr/share/themes/blanco/meegotouch/images/theme/meegotouch-virtual-keyboard";

    // TODO: unify with theme folder? Or have only one big SVG even?
    const char * const IconsDirectory =
        "/usr/share/themes/base/meegotouch/icons";

    struct StyleNode
    {
    public:
        typedef QHash<QLatin1String, QVariant> AttributesMap;

        const StyleNode *parent;
        AttributesMap attributes; //!< contains default values suitable for both orientations
        AttributesMap landscapeAttributes; //!< overrides values in default attributes when using landscape mode
        AttributesMap portraitAttributes; //!< ovverides values in default attributes when using portrait mode

        explicit StyleNode(const StyleNode *newParent = 0)
            : parent(newParent)
        {}

        QVariant findAttributeWithOrientation(const QString &name,
                                              MInputMethod::Orientation orientation) const
        {
            const AttributesMap &map(orientation == MInputMethod::Landscape ? landscapeAttributes
                                                                            : portraitAttributes);

            const QVariant result(map.value(QLatin1String(name.toAscii().constData())));

            if (not result.isValid()) {
                return findAttribute(name);
            }

            return result;
        }


        QVariant findAttribute(const QString &name) const
        {
            const QVariant result(attributes.value(QLatin1String(name.toAscii().constData())));

            // Initiates recursive lookup:
            if (not result.isValid() && parent) {
                return parent->findAttribute(name);
            }

            return result;
        }
    };

    // "Element.{Portrait, Landscape}:customMode =>
    //    StyleNode "Element:customMode" with parent = ?
    // * Element:Mode is a valid style node!
    // * Element is a valid style mode!
    QHash<QLatin1String, const StyleNode *> availableStyleContainers;

    // Test data
    void fillStyleContainers()
    {
        qRegisterMetaType<QMargins>("QMargins");

        typedef QLatin1String QL1S;
        StyleNode *mimKey = new StyleNode; // Default
        StyleNode *extKey = new StyleNode(mimKey); // Overrides some stuff
        StyleNode *mimKeyChild = new StyleNode(mimKey); // 100% transient to MImKey

        StyleNode::AttributesMap &m = mimKey->attributes;
        QVariant margins;
        margins.setValue(QMargins(16, 16, 16, 16));
        m.insert(QL1S("key-background-borders"), margins);

        m.insert(QL1S("key-background"), "meegotouch-keyboard-key");
        m.insert(QL1S("key-background-pressed"), "meegotouch-keyboard-key-pressed");
        m.insert(QL1S("key-background-selected"), "meegotouch-keyboard-key-selected");
        m.insert(QL1S("key-background-disabled"), "meegotouch-keyboard-key-disabled");

        m.insert(QL1S("key-background-highlighted"), "meegotouch-keyboard-key-highlighted");
        m.insert(QL1S("key-background-highlighted-pressed"), "meegotouch-function-key-highlighted-pressed");
        m.insert(QL1S("key-background-highlighted-selected"), "meegotouch-keyboard-key-highlighted-selected");
        m.insert(QL1S("key-background-highlighted-disabled"), "meegotouch-keyboard-key-highlighted-disabled");

        m.insert(QL1S("key-background-special"), "meegotouch-keyboard-function-key");
        m.insert(QL1S("key-background-special-pressed"), "meegotouch-keyboard-function-key-pressed");
        m.insert(QL1S("key-background-special-selected"), "meegotouch-keyboard-function-key-selected");
        m.insert(QL1S("key-background-special-disabled"), "meegotouch-keyboard-function-key-disabled");

        m.insert(QL1S("key-background-special-highlighted"), "meegotouch-keyboard-function-key-highlighted");
        m.insert(QL1S("key-background-special-highlighted-pressed"), "meegotouch-keyboard-function-key-highlighted-pressed");
        m.insert(QL1S("key-background-special-highlighted-selected"), "meegotouch-keyboard-function-key-highlighted-selected");
        m.insert(QL1S("key-background-special-highlighted-disabled"), "meegotouch-keyboard-function-key-highlighted-disabled");

        StyleNode::AttributesMap &e = extKey->attributes;
        e.insert(QL1S("key-backound"), "meegotouch-keyboard-accent-magnifier-background");
        e.insert(QL1S("key-backound-pressed"), "");
        e.insert(QL1S("key-backound-selected"), "");
        e.insert(QL1S("key-backound-disabled"), "");

        availableStyleContainers.insert(QL1S("MImKey"), mimKey);
        availableStyleContainers.insert(QL1S("ExtendedKey"), extKey);
        availableStyleContainers.insert(QL1S("MImKeyChild"), mimKeyChild);
    }
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

#ifdef EXPERIMENTAL_STYLING
MImKeyStyle::MImKeyStyle(const QLatin1String &newStyleClassName)
    : styleClassName(newStyleClassName)
{
    // FIXME: move to proper place
    fillStyleContainers();
}
#else
MImKeyStyle::MImKeyStyle(const QLatin1String &newStyleClassName,
                         const MImAbstractKeyAreaStyleContainer &newStyleContainer)
    : styleClassName(newStyleClassName)
    , styleContainer(newStyleContainer)
{
    qRegisterMetaType<const MScalableImage *>();
}
#endif

MImKeyStyle::~MImKeyStyle()
{}

MImGraphicsAsset MImKeyStyle::background(const MImKeyStylingContext &context) const
{
    QString fileName = QString("%1/%2.png").arg(ThemeDirectory);

    QStringList keyName;
    keyName.append("key");
    keyName.append("background");

    if (context.style == MaliitKeyboard::KeyStyleSpecial) {
        keyName.append("special");
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

// TODO: fix action key highlighting
#ifdef EXPERIMENTAL_STYLING
    const StyleNode *container(availableStyleContainers.value(styleClassName));
    if (container) {
        const QString joinedKeyName(keyName.join("-"));
        return MImGraphicsAsset(joinedKeyName, QMargins(),
                                QPixmap(fileName.arg(container->findAttribute(joinedKeyName).toString())));
    }

    return MImGraphicsAsset();
#else
    const QString identifier(keyName.join("-"));

    for (int index = 1; index < keyName.size(); ++index) {
        keyName[index][0] = keyName.at(index).at(0).toUpper();
    }

    const QString joinedKeyName(keyName.join(""));
    const MScalableImage *img = getCSSProperty<const MScalableImage *>(styleContainer, joinedKeyName, false);
    return MImGraphicsAsset(identifier, QMargins(),
                            (img ? *img->pixmap() : QPixmap()));
#endif

}

MImGraphicsAsset MImKeyStyle::icon(const MImKeyStylingContext &context,
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

    return MImGraphicsAsset(fileName, QMargins(), QPixmap(fileName));
}

QFont MImKeyStyle::font(const MImKeyStylingContext &context,
                        const QString &text,
                        const QSizeF size) const
{
    // TODO: compute correct font
    Q_UNUSED(context)
    Q_UNUSED(text)
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
