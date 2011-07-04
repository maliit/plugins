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

#ifndef STYLE_WRAPPER_H
#define STYLE_WRAPPER_H

#ifdef HAVE_MEEGOTOUCH
#include <MStylableWidget>
#include <MWidgetStyle>
#else
#include "style-types.h"
#include "mwidget-wrapper.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

// Dummy macros:
#define M_EXPORT
#define M_STYLABLE_WIDGET(x)
#define M_STYLE(x) \

// Those macros expand correctly, but the created containers aren't used:
#define M_STYLE_CONTAINER(STYLE) \
public:\
    const STYLE *operator->() const \
    { \
        static STYLE style; \
        return &style; \
    }

#define M_STYLE_MODE(MODE) \
public: \
    void setMode ## MODE() {}

#define M_STYLE_ATTRIBUTE(T, G, S) \
public: \
    T G () const { static T x; return x; } \
    void set ## S ( const T& ) {}

#define M_STYLE_PTR_ATTRIBUTE(T, G, S) \
public: \
    T G () const { T x = 0; return x; } \
    void set ## S ( T ) {}

class MStylableWidget
    : public MWidget
{
    Q_OBJECT

public:
    explicit MStylableWidget(QGraphicsItem *item)
        : MWidget(item)
    {}

    void drawBackground(QPainter *, const QStyleOptionGraphicsItem *) const {}
    void setStyleName(const QString &) {}
};

#endif // HAVE_MEEGOTOUCH
#endif // STYLE_WRAPPER_H

