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
#ifndef MIMABSTRACTKEYAREASTYLE_H
#define MIMABSTRACTKEYAREASTYLE_H

#ifdef HAVE_MEEGOTOUCH
#include "style-wrapper.h"

/*!
    \brief Style for MImAbstractKeyArea derived classes.
*/
class M_EXPORT MImAbstractKeyAreaStyle : public MWidgetStyle
{
    Q_OBJECT
    M_STYLE(MImAbstractKeyAreaStyle)

public:
    M_STYLE_ATTRIBUTE(QColor, fontColor, FontColor)
    M_STYLE_ATTRIBUTE(QFont, font, Font)
    M_STYLE_ATTRIBUTE(qreal, fontOpacity, FontOpacity)
    M_STYLE_ATTRIBUTE(QFont, secondaryFont, SecondaryFont)

    M_STYLE_ATTRIBUTE(int, labelMarginTop, LabelMarginTop)
    M_STYLE_ATTRIBUTE(int, labelMarginLeftWithSecondary, LabelMarginLeftWithSecondary)
    M_STYLE_ATTRIBUTE(int, secondaryLabelSeparation, SecondaryLabelSeparation)

    M_STYLE_ATTRIBUTE(int, longPressTimeout, LongPressTimeout)
    M_STYLE_ATTRIBUTE(int, idleVkbTimeout, IdleVkbTimeout)
    M_STYLE_ATTRIBUTE(int, flickGestureTimeout, FlickGestureTimeout)

    M_STYLE_ATTRIBUTE(qreal, flickGestureThresholdRatio, FlickGestureThresholdRatio)
    M_STYLE_ATTRIBUTE(qreal, touchpointHorizontalGravity, TouchpointHorizontalGravity)
    M_STYLE_ATTRIBUTE(qreal, touchpointVerticalGravity, TouchpointVerticalGravity)
    M_STYLE_ATTRIBUTE(qreal, touchpointVerticalOffset, TouchpointVerticalOffset)

    M_STYLE_ATTRIBUTE(QSize, size, Size)
    M_STYLE_ATTRIBUTE(qreal, keyHeightSmall, KeyHeightSmall)
    M_STYLE_ATTRIBUTE(qreal, keyHeightMedium, KeyHeightMedium)
    M_STYLE_ATTRIBUTE(qreal, keyHeightLarge,  KeyHeightLarge)
    M_STYLE_ATTRIBUTE(qreal, keyHeightXLarge, KeyHeightXLarge)
    M_STYLE_ATTRIBUTE(qreal, keyHeightXxLarge, KeyHeightXxLarge)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackground, KeyBackground)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDisabled, KeyBackgroundDisabled)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundPressed, KeyBackgroundPressed)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSelected, KeyBackgroundSelected)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundPressedSelected, KeyBackgroundPressedSelected)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundHighlighted, KeyBackgroundHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundPressedHighlighted, KeyBackgroundPressedHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSelectedHighlighted, KeyBackgroundSelectedHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundPressedSelectedHighlighted, KeyBackgroundPressedSelectedHighlighted)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecial, KeyBackgroundSpecial)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialDisabled, KeyBackgroundSpecialDisabled)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialPressed, KeyBackgroundSpecialPressed)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialSelected, KeyBackgroundSpecialSelected)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialPressedSelected, KeyBackgroundSpecialPressedSelected)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialHighlighted, KeyBackgroundSpecialHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialPressedHighlighted, KeyBackgroundSpecialPressedHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialSelectedHighlighted, KeyBackgroundSpecialSelectedHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundSpecialPressedSelectedHighlighted, KeyBackgroundSpecialPressedSelectedHighlighted)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkey, KeyBackgroundDeadkey)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeyDisabled, KeyBackgroundDeadkeyDisabled)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeyPressed, KeyBackgroundDeadkeyPressed)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeySelected, KeyBackgroundDeadkeySelected)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeyPressedSelected, KeyBackgroundDeadkeyPressedSelected)

    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeyHighlighted, KeyBackgroundDeadkeyHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeyPressedHighlighted, KeyBackgroundDeadkeyPressedHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeySelectedHighlighted, KeyBackgroundDeadkeySelectedHighlighted)
    M_STYLE_PTR_ATTRIBUTE(MScalableImage *, keyBackgroundDeadkeyPressedSelectedHighlighted, KeyBackgroundDeadkeyPressedSelectedHighlighted)

    M_STYLE_ATTRIBUTE(QSize, requiredKeyIconMargins, RequiredKeyIconMargins)

    M_STYLE_ATTRIBUTE(bool, useFixedKeyWidth, UseFixedKeyWidth)

    M_STYLE_ATTRIBUTE(qreal, keyWidthSmall, KeyWidthSmall)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthSmallFixed, KeyWidthSmallFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthMedium, KeyWidthMedium)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthMediumFixed, KeyWidthMediumFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthLarge,  KeyWidthLarge)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthLargeFixed,  KeyWidthLargeFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthXLarge, KeyWidthXLarge)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthXLargeFixed, KeyWidthXLargeFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthXxLarge, KeyWidthXxLarge)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthXxLargeFixed, KeyWidthXxLargeFixed)

    M_STYLE_ATTRIBUTE(qreal, keyWidthStretched, KeyWidthStretched)
    M_STYLE_ATTRIBUTE(qreal,  keyWidthStretchedFixed, KeyWidthStretchedFixed)

    // Backspace key icon
    M_STYLE_ATTRIBUTE(QSize,   keyBackspaceIconSize, KeyBackspaceIconSize)
    M_STYLE_ATTRIBUTE(QString, keyBackspaceIconId, KeyBackspaceIconId)
    M_STYLE_ATTRIBUTE(QString, keyBackspaceIconIdRtl, KeyBackspaceIconIdRtl)

    M_STYLE_ATTRIBUTE(QSize,   keyBackspaceCompactIconSize, KeyBackspaceCompactIconSize)
    M_STYLE_ATTRIBUTE(QString, keyBackspaceCompactIconId, KeyBackspaceCompactIconId)
    M_STYLE_ATTRIBUTE(QString, keyBackspaceCompactIconIdRtl, KeyBackspaceCompactIconIdRtl)

    // Menu key icon
    M_STYLE_ATTRIBUTE(QSize, keyMenuIconSize, KeyMenuIconSize)
    M_STYLE_ATTRIBUTE(QString, keyMenuIconId, KeyMenuIconId)
    M_STYLE_ATTRIBUTE(QString, keyMenuIconIdRtl, KeyMenuIconIdRtl)

    M_STYLE_ATTRIBUTE(QSize,   keyMenuCompactIconSize, KeyMenuCompactIconSize)
    M_STYLE_ATTRIBUTE(QString, keyMenuCompactIconId, KeyMenuCompactIconId)
    M_STYLE_ATTRIBUTE(QString, keyMenuCompactIconIdRtl, KeyMenuCompactIconIdRtl)

    // Enter key icon
    M_STYLE_ATTRIBUTE(QSize, keyEnterIconSize, KeyEnterIconSize)
    M_STYLE_ATTRIBUTE(QString, keyEnterIconId, KeyEnterIconId)
    M_STYLE_ATTRIBUTE(QString, keyEnterIconIdHighlighted, KeyEnterIconIdHighlighted)
    M_STYLE_ATTRIBUTE(QString, keyEnterIconIdRtl, KeyEnterIconIdRtl)
    M_STYLE_ATTRIBUTE(QString, keyEnterIconIdRtlHighlighted, KeyEnterIconIdRtlHighlighted)

    M_STYLE_ATTRIBUTE(QSize,   keyEnterCompactIconSize, KeyEnterCompactIconSize)
    M_STYLE_ATTRIBUTE(QString, keyEnterCompactIconId, KeyEnterCompactIconId)
    M_STYLE_ATTRIBUTE(QString, keyEnterCompactIconIdHighlighted, KeyEnterCompactIconIdHighlighted)
    M_STYLE_ATTRIBUTE(QString, keyEnterCompactIconIdRtl, KeyEnterCompactIconIdRtl)
    M_STYLE_ATTRIBUTE(QString, keyEnterCompactIconIdRtlHighlighted, KeyEnterCompactIconIdRtlHighlighted)

    // Tab icon
    M_STYLE_ATTRIBUTE(QSize, keyTabIconSize, KeyTabIconSize)
    M_STYLE_ATTRIBUTE(QString, keyTabIconId, KeyTabIconId)

    M_STYLE_ATTRIBUTE(QSize,   keyTabCompactIconSize, KeyTabCompactIconSize)
    M_STYLE_ATTRIBUTE(QString, keyTabCompactIconId, KeyTabCompactIconId)

    // Shift / capslock icon
    M_STYLE_ATTRIBUTE(QSize, keyShiftIconSize, KeyShiftIconSize)
    M_STYLE_ATTRIBUTE(QString, keyShiftIconId, KeyShiftIconId)
    M_STYLE_ATTRIBUTE(QString, keyShiftIconIdSelected, KeyShiftIconIdSelected)
    M_STYLE_ATTRIBUTE(QString, keyShiftUppercaseIconId, KeyShiftUppercaseIconId)
    M_STYLE_ATTRIBUTE(QString, keyShiftUppercaseIconIdSelected, KeyShiftUppercaseIconIdSelected)

    M_STYLE_ATTRIBUTE(QSize, keyShiftCompactIconSize, KeyShiftCompactIconSize)
    M_STYLE_ATTRIBUTE(QString, keyShiftCompactIconId, KeyShiftCompactIconId)
    M_STYLE_ATTRIBUTE(QString, keyShiftCompactIconIdSelected, KeyShiftCompactIconIdSelected)
    M_STYLE_ATTRIBUTE(QString, keyShiftUppercaseCompactIconId, KeyShiftUppercaseCompactIconId)
    M_STYLE_ATTRIBUTE(QString, keyShiftUppercaseCompactIconIdSelected, KeyShiftUppercaseCompactIconIdSelected)


    M_STYLE_ATTRIBUTE(bool, drawButtonBoundingRects, DrawButtonBoundingRects)
    M_STYLE_ATTRIBUTE(bool, drawButtonRects, DrawButtonRects)
    M_STYLE_ATTRIBUTE(bool, debugTouchPoints, DebugTouchPoints)
    M_STYLE_ATTRIBUTE(bool, drawReactiveAreas, drawReactiveAreas)

    M_STYLE_ATTRIBUTE(qreal, keyMarginLeft, KeyMarginLeft)
    M_STYLE_ATTRIBUTE(qreal, keyMarginTop, KeyMarginTop)
    M_STYLE_ATTRIBUTE(qreal, keyMarginRight, KeyMarginRight)
    M_STYLE_ATTRIBUTE(qreal, keyMarginBottom, KeyMarginBottom)

    M_STYLE_ATTRIBUTE(bool, syncStyleModeWithKeyCount, SyncStyleModeWithKeyCount)
    M_STYLE_ATTRIBUTE(bool, commitPreviousKeyOnPress, CommitPreviousKeyOnPress)
    M_STYLE_ATTRIBUTE(bool, autoPadding, AutoPadding)

    // Key font colors
    M_STYLE_ATTRIBUTE(QColor, keyPressedFontColor, KeyPressedFontColor)
    M_STYLE_ATTRIBUTE(QColor, keySelectedFontColor, KeySelectedFontColor)
    M_STYLE_ATTRIBUTE(QColor, keyDisabledFontColor, KeyDisabledFontColor)
    M_STYLE_ATTRIBUTE(QColor, keyHighlightedFontColor, KeyHighlightedFontColor)
};

class M_EXPORT MImAbstractKeyAreaStyleContainer : public MWidgetStyleContainer
{
    M_STYLE_CONTAINER(MImAbstractKeyAreaStyle)

    //! \brief Extra style modes for MImAbstractKeyArea
    //!
    //! Based on amount of keys in a given layout. Switches to style mode when
    //! loading layout (i.e., during construction of MImAbstractKeyArea).
    M_STYLE_MODE(Keys10)
    M_STYLE_MODE(Keys11)
    M_STYLE_MODE(Keys12)
    M_STYLE_MODE(Keys13)
    M_STYLE_MODE(Keys14)
    M_STYLE_MODE(Keys15)
    M_STYLE_MODE(Keys30)
    M_STYLE_MODE(Keys31)
    M_STYLE_MODE(Keys32)
    M_STYLE_MODE(Keys33)
    M_STYLE_MODE(Keys34)
    M_STYLE_MODE(Keys35)
    M_STYLE_MODE(Keys36)
    M_STYLE_MODE(Keys37)
    M_STYLE_MODE(Keys38)
    M_STYLE_MODE(Keys39)
    M_STYLE_MODE(Keys40)
    M_STYLE_MODE(Keys41)
    M_STYLE_MODE(Keys42)
    M_STYLE_MODE(Keys43)
    M_STYLE_MODE(Keys44)
    M_STYLE_MODE(Keys45)

#ifdef UNIT_TEST
    friend class Ut_MImAbstractKeyArea;
#endif
};

#endif // HAVE_MEEGOTOUCH
#endif // MIMABSTRACTKEYAREASTYLE_H
