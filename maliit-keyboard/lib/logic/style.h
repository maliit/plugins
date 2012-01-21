/*
 * This file is part of Maliit Plugins
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

#ifndef MALIIT_KEYBOARD_STYLE_H
#define MALIIT_KEYBOARD_STYLE_H

#include "models/layout.h"
#include "models/keydescription.h"

#include <QtCore>

namespace MaliitKeyboard {

class StylePrivate;

class Style
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Style)
    Q_DECLARE_PRIVATE(Style)

public:
    explicit Style(QObject *parent = 0);
    virtual ~Style();

    void setProfile(const QString &profile);
    void setStyleName(const QString &name);

    QByteArray keyAreaBackground() const;
    QByteArray keyBackground(KeyDescription::Style style,
                             KeyDescription::State state) const;
    QByteArray icon(KeyDescription::Icon icon,
                    KeyDescription::State state) const;

    QByteArray fontName(const QByteArray &group_id = QByteArray()) const;
    qreal fontSize(const QByteArray &group_id = QByteArray()) const;

    qreal keyHeight(Layout::Orientation orientation) const;
    qreal keyWidth(Layout::Orientation orientation,
                   KeyDescription::Width width) const;
    qreal keyAreaWidth(Layout::Orientation orientation) const;

    qreal keyMargin(Layout::Orientation orientation) const;
    qreal keyAreaPadding(Layout::Orientation orienation) const;

    qreal verticalExtendedKeysOffset(Layout::Orientation orientation) const;
    qreal extendedKeysSafetyMargin(Layout::Orientation orientation) const;

private:
    const QScopedPointer<StylePrivate> d_ptr;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_STYLE_H
