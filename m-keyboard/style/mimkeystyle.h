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

#ifndef MIMKEYSTYLE_H
#define MIMKEYSTYLE_H

#include "namespace.h"
#include "mimgraphicsasset.h"

#include <minputmethodnamespace.h>
#include <QtCore>
#include <QtGui>
#include <QMargins>

// TODO: Move to proper place.
Q_DECLARE_METATYPE(QMargins)

//! Context (look&feel) helper for MImKeyStyle
struct MImKeyStylingContext
{
public:
    MaliitKeyboard::KeyState state;
    MaliitKeyboard::KeyStyle style;
    MaliitKeyboard::KeyOverrides overrides;

    explicit MImKeyStylingContext(MaliitKeyboard::KeyState newState,
                                  MaliitKeyboard::KeyStyle newStyle,
                                  MaliitKeyboard::KeyOverrides newOverrides = MaliitKeyboard::KeyOverrideNone);
};

//! Context (geometry) helper for MImKeyStyle
struct MImKeyGeometryContext
{
public:
    MaliitKeyboard::KeyWidth width;
    MaliitKeyboard::KeyHeight height;
    MInputMethod::Orientation orientation;

    explicit MImKeyGeometryContext(MaliitKeyboard::KeyWidth newWidth,
                                   MaliitKeyboard::KeyHeight newHeight,
                                   MInputMethod::Orientation newOrientation);
};

//! This class can be queried for style information regarding keys.
//! Instances of this class are supposed to be stateless; all relevant
//! information is given through a styling context parameter.
//! One style instance can be used for an arbitrary amount of keys.
class MImKeyStyle
{
private:
    const QLatin1String styleClassName; //!< Keys with same style class name will use same styling information.

public:
    //! C'tor
    //! @param newStyleClassName the style class name of a group of keys
    explicit MImKeyStyle(const QLatin1String &newStyleClassName);

    //! D'tor
    virtual ~MImKeyStyle();

    //! Returns background depending on context.
    MImGraphicsAsset background(const MImKeyStylingContext &context) const;

    //! Returns backspace key icon depending on context, where size is an optional constraint.
    MImGraphicsAsset icon(const MImKeyStylingContext &context,
                          MaliitKeyboard::KeyAction action,
                          const QSizeF size = QSizeF()) const;

    //! Returns font depending on context, where size is an optional constraint.
    QFont font(const MImKeyStylingContext &context,
               const QString &text,
               const QSizeF size = QSizeF()) const;

    //! Returns font color depending on context.
    QColor fontColor(const MImKeyStylingContext &context) const;

    //! Returns size depending on context.
    QSizeF size(const MImKeyGeometryContext &context) const;

    //! Returns key margins depending on context.
    QMargins margins(const MImKeyGeometryContext &context) const;
};

#endif // MIMKEYSTYLE_H
