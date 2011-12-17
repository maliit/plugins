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

#include "key.h"

namespace MaliitKeyboard {

Key::Key()
    : m_action(ActionInsert)
    , m_label()
    , m_rect()
    , m_margins()
    , m_background_borders()
    , m_background()
    , m_icon()
{}

bool Key::valid() const
{
    return (not m_rect.isEmpty());
}

Key::Action Key::action() const
{
    return m_action;
}

void Key::setAction(Action action)
{
    m_action = action;
}

KeyLabel Key::label() const
{
    return m_label;
}

void Key::setLabel(const KeyLabel &label)
{
    m_label = label;
}

QRect Key::rect() const
{
    return m_rect;
}

void Key::setRect(const QRect &rect)
{
    m_rect = rect;
}

QMargins Key::margins() const
{
    return m_margins;
}

void Key::setMargins(const QMargins &margins)
{
    m_margins = margins;
}

QMargins Key::backgroundBorders() const
{
    return m_background_borders;
}

void Key::setBackgroundBorders(const QMargins &borders)
{
    m_background_borders = borders;
}

QByteArray Key::background() const
{
    return m_background;
}

void Key::setBackground(const QByteArray &background)
{
    m_background = background;
}

QByteArray Key::icon() const
{
    return m_icon;
}

void Key::setIcon(const QByteArray &icon)
{
    m_icon = icon;
}

bool operator==(const Key &lhs,
                const Key &rhs)
{
    // FIXME: introduce comparison for labels.
    return (lhs.rect() == rhs.rect() && lhs.label().text() == rhs.label().text());
}

bool operator!=(const Key &lhs,
                const Key &rhs)
{
    return (not (lhs == rhs));
}

} // namespace MaliitKeyboard
