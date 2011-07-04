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

#ifndef MIMWORDTRACKERSTYLE_H
#define MIMWORDTRACKERSTYLE_H

#include "style-wrapper.h"

class MImWordTrackerStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(MImWordTrackerStyle)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *,  wordtrackerPointerImage, WordtrackPointerImage)
    M_STYLE_ATTRIBUTE(QSize, wordtrackerPointerSize, WordtrackerPointerSize)
    M_STYLE_ATTRIBUTE(int, wordtrackerPointerOverlap, WordtrackerPointerOverlap)
    M_STYLE_ATTRIBUTE(int, wordtrackerPointerTopMargin, WordtrackerPointerTopMargin)
    M_STYLE_ATTRIBUTE(int, wordtrackerPointerLeftMargin, WordtrackerPointerLeftMargin)
    M_STYLE_ATTRIBUTE(int, wordtrackerPointerRightMargin, WordtrackerPointerRightMargin)
    M_STYLE_ATTRIBUTE(int, wordtrackerLeftMargin, WordtrackerLeftMargin)
    M_STYLE_ATTRIBUTE(int, wordtrackerRightMargin, WordtrackerRightMargin)
    M_STYLE_ATTRIBUTE(int, wordtrackerCursorXOffset, WordtrackerCursorXOffset)
    M_STYLE_ATTRIBUTE(int, wordtrackerCursorYOffset, WordtrackerCursorYOffset)

    M_STYLE_ATTRIBUTE(int, showHideFrames, ShowHideFrames)
    M_STYLE_ATTRIBUTE(int, showHideTime, ShowHideTime)
    M_STYLE_ATTRIBUTE(int, showHideInterval, ShowHideInterval)
};

class M_EXPORT MImWordTrackerStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(MImWordTrackerStyle)
};

#endif

