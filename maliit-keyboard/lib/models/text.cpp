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

#include "text.h"

namespace MaliitKeyboard {
namespace Model {

Text::Text()
    : m_preedit()
    , m_surrounding()
    , m_surrounding_offset(0)
{}

QString Text::preedit() const
{
    return m_preedit;
}

void Text::setPreedit(const QString &preedit)
{
    m_preedit = preedit;
}

void Text::appendToPreedit(const QString &appendix)
{
    m_preedit.append(appendix);
}

void Text::commitPreedit()
{
    // FIXME: Guessing the surrounding text like this might not be quite right;
    // we would expect the text editor to just update the surrounding text.
    // Raises the question whether we should have commitPreedit here at all,
    // but it does preserve some consistency at least.
    m_surrounding = m_preedit;
    m_surrounding_offset = m_preedit.length();
    m_preedit.clear();
}

QString Text::surrounding() const
{
    return m_surrounding;
}

QString Text::surroundingLeft() const
{
    return m_surrounding.left(m_surrounding_offset);
}

QString Text::surroundingRight() const
{
    return m_surrounding.mid(m_surrounding_offset);
}

void Text::setSurrounding(const QString &surrounding)
{
    m_surrounding = surrounding;
}

uint Text::surroundingOffset() const
{
    return m_surrounding_offset;
}

void Text::setSurroundingOffset(uint offset)
{
    m_surrounding_offset = offset;
}

}} // namespace Model, MaliitKeyboard
