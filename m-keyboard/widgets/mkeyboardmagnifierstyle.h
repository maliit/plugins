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

#ifndef MKEYBOARDMAGNIFIERSTYLE_H
#define MKEYBOARDMAGNIFIERSTYLE_H

#include "style-wrapper.h"
#include <QFont>
#include <QEasingCurve>

class MScalableImage;

/*!
 * \class MKeyboardMagnifierStyle
 * \brief This class provides access to style attributes for virtual keyboard magnifier
 */
class MKeyboardMagnifierStyle: public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(MKeyboardMagnifierStyle)

public:
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, magnifierImage, magnifierImage)
    M_STYLE_ATTRIBUTE(QSize, magnifierSize, magnifierSize)
    M_STYLE_ATTRIBUTE(int, magnifierButtonOverlap, MagnifierButtonOverlap)
    M_STYLE_ATTRIBUTE(QFont, magnifierFont, MagnifierFont)
    M_STYLE_ATTRIBUTE(int, magnifierFontSize, MagnifierFontSize)
    M_STYLE_ATTRIBUTE(QColor, magnifierTextColor, MagnifierTextColor)
    M_STYLE_ATTRIBUTE(int, magnifierTextMarginTop, MagnifierTextMarginTop)
    M_STYLE_ATTRIBUTE(int, magnifierTextMarginBottom, MagnifierTextMarginBottom)
    M_STYLE_ATTRIBUTE(int, magnifierTextMarginBottomLowercase, MagnifierTextMarginBottomLowercase)
    M_STYLE_ATTRIBUTE(int, magnifierHideDelay, MagnifierHideDelay)
    M_STYLE_ATTRIBUTE(int, safetyMarginLeft, SafetyMarginLeft)
    M_STYLE_ATTRIBUTE(int, safetyMarginRight, SafetyMarginRight)

    M_STYLE_ATTRIBUTE(int, extendedKeysOffset, ExtendedKeysOffset)
    M_STYLE_ATTRIBUTE(QEasingCurve, extendedKeysShowEasingCurve, ExtendedKeysShowEasingCurve)
    M_STYLE_ATTRIBUTE(int, extendedKeysShowDuration, ExtendedKeysShowDuration)

    M_STYLE_ATTRIBUTE(QEasingCurve, magnifierHideEasingCurve, MagnifierHideEasingCurve)
    M_STYLE_ATTRIBUTE(int, magnifierHideDuration, MagnifierHideDuration)
};

class MKeyboardMagnifierStyleContainer: public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(MKeyboardMagnifierStyle)

    M_STYLE_MODE(Landscape)
    M_STYLE_MODE(Portrait)
};

#endif
