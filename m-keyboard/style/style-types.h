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

#ifndef STYLE_TYPES_H
#define STYLE_TYPES_H

#ifndef HAVE_MEEGOTOUCH
#include <QObject>
#include <QFont>
#include <QSize>
#include <QColor>
#include <QPixmap>

// Dummy MFeedback
class MFeedback
    : public QObject
{
    Q_OBJECT

public:
    MFeedback() {}
    explicit MFeedback(const QString &) {}
    Q_SLOT void play() {}
};

// Dummy MScalableImage:
class MScalableImage
    : public QObject
{
    Q_OBJECT

public:
    QPixmap *pixmap() const {static QPixmap pm; return &pm;}
    void draw(const QRect &, QPainter *) const {}
    void draw(const QRectF &, QPainter *) const {}
    void draw(const QPoint &, const QSize &, QPainter *) const {}
};

// Dummy MWidgetStyle:
#define RTN_STATIC_TYPE(T) {static T t; return t;}
#define RTN_STATIC_FONT RTN_STATIC_TYPE(QFont)
#define RTN_STATIC_COLOR RTN_STATIC_TYPE(QColor)

class MWidgetStyle
    : public QObject
{
    Q_OBJECT

public:
    // from MImCorrectionCandidateItem:
    MFeedback &pressFeedback() const {static MFeedback fb; return fb;}
    MFeedback &releaseFeedback() const {return pressFeedback();}
    MFeedback &cancelFeedback() const {return pressFeedback();}

    qreal releaseMissDelta() const {return 0;}
    qreal pressTimeout() const {return 0;}
    qreal longTapTimeout() const {return 0;}

    QFont font() const RTN_STATIC_FONT
    QColor fontColor() const RTN_STATIC_COLOR

    qreal marginLeft() const {return 0;}
    qreal marginTop() const {return 0;}
    qreal marginRight() const {return 0;}
    qreal marginBottom() const {return 0;}

    qreal paddingLeft() const {return 0;}
    qreal paddingTop() const {return 0;}
    qreal paddingRight() const {return 0;}
    qreal paddingBottom() const {return 0;}

    QSize wordtrackerPointerSize() const {static QSize s; return s;}
    int  wordtrackerPointerOverlap() const {return 0;}

    int wordtrackerPointerTopMargin() const {return 0;}
    int wordtrackerPointerLeftMargin() const {return 0;}
    int wordtrackerPointerRightMargin() const {return 0;}
    int wordtrackerLeftMargin() const {return 0;}
    int wordtrackerRightMargin() const {return 0;}
    int wordtrackerCursorXOffset() const {return 0;}
    int wordtrackerCursorYOffset() const {return 0;}
    MScalableImage *wordtrackerPointerImage() const {return 0;}

    int showHideFrames() const {return 0;}
    int showHideTime() const {return 0;}
    int showHideInterval() const {return 0;}

    // from Notification:
    MScalableImage *backgroundImage() const {return 0;}
    qreal backgroundOpacity() const {return 0;}
    int fontSize() const {return 0;}
    QColor borderColor() const RTN_STATIC_COLOR
    QColor backgroundColor() const RTN_STATIC_COLOR
    QColor textColor() const RTN_STATIC_COLOR
    int opacity() const {return 0;}
    int holdTime() const {return 0;}
    int fadeTime() const {return 0;}
    int rounding() const {return 0;}

    // from MImAbstractKeyArea:
    bool commitPreviousKeyOnPress() const {return true;}
    int longPressTimeout() const {return 500;}
    int idleVkbTimeout() const {return 300;}
    qreal touchpointHorizontalGravity() const {return 40;}
    qreal touchpointVerticalGravity() const {return 20;}
    qreal touchpointVerticalOffset() const {return 0;}
    bool syncStyleModeWithKeyCount() const {return true;}
    bool debugTouchPoints() const {return false;}
    qreal flickGestureThresholdRatio() const {return 0.5;}
    qreal flickGestureTimeout() const {return 300;}

    // from MImKey:
    int labelMarginTop() const {return 0;}
    int labelMarginLeftWithSecondary() const {return 0;}
    bool secondaryLabelSeparation() const {return false;}
    QFont secondaryFont() const RTN_STATIC_FONT

    int keyWidthSmallFixed() const {return 20;}
    int keyWidthMediumFixed() const {return 30;}
    int keyWidthLargeFixed() const {return 40;}
    int keyWidthXLargeFixed() const {return 50;}
    int keyWidthXxLargeFixed() const {return 60;}
    int keyWidthStretchedFixed() const {return 0;}

    qreal keyWidthSmall() const {return 0.5;}
    qreal keyWidthMedium() const {return 1;}
    qreal keyWidthLarge() const {return 1.2;}
    qreal keyWidthXLarge() const {return 1.5;}
    qreal keyWidthXxLarge() const {return 2;}
    qreal keyWidthStretched() const {return 0;}

    QSize requiredKeyIconMargins() const {static QSize s; return s;}
    QColor keyHighlightedFontColor() const RTN_STATIC_COLOR
    QColor keyPressedFontColor() const RTN_STATIC_COLOR
    QColor keySelectedFontColor() const RTN_STATIC_COLOR
    QColor keyDisabledFontColor() const RTN_STATIC_COLOR

    // from MImKeyArea:
    int keyMarginLeft() const {return 0;}
    int keyMarginTop() const {return 0;}
    int keyMarginRight() const {return 0;}
    int keyMarginBottom() const {return 0;}
    int keyHeight() const {return 20;}
    int keyHeightSmall() const {return 20;}
    int keyHeightMedium() const {return 30;}
    int keyHeightLarge() const {return 40;}
    int keyHeightXLarge() const {return 50;}
    int keyHeightXxLarge() const {return 60;}

    bool useFixedKeyWidth() const {return true;}
    bool autoPadding() const {return false;}
    bool drawButtonRects() const {return false;}
    bool drawButtonBoundingRects() const {return false;}
    bool drawReactiveAreas() const {return false;}
    qreal fontOpacity() const {return 1;}

    QSize size() const RTN_STATIC_TYPE(QSize)

    // from WordRibbonItem:
    QColor selectedFontColor() const RTN_STATIC_COLOR
    QColor pressedFontColor() const RTN_STATIC_COLOR
    QSize minimumSize() const RTN_STATIC_TYPE(QSize)

    // from WordRibbon:
    qreal spaceBetween() const {return 0;}
    QSize preferredSize() const RTN_STATIC_TYPE(QSize)
};

// Dummy MWidgetStyleContainer:
class MWidgetStyleContainer
{
public:
    const MWidgetStyle *operator->() const {static MWidgetStyle style; return &style;}

    void setModePressed() {}
    void setModeSelected() {}
    void setModeDefault() {}

    // from MImAbstractKeyArea:
    void setModeKeys10() {}
    void setModeKeys11() {}
    void setModeKeys12() {}
    void setModeKeys13() {}
    void setModeKeys14() {}
    void setModeKeys15() {}

    void setModeKeys30() {}
    void setModeKeys31() {}
    void setModeKeys32() {}
    void setModeKeys33() {}
    void setModeKeys34() {}
    void setModeKeys35() {}
    void setModeKeys36() {}
    void setModeKeys37() {}
    void setModeKeys38() {}
    void setModeKeys39() {}
    void setModeKeys40() {}
    void setModeKeys41() {}
    void setModeKeys42() {}
    void setModeKeys43() {}
    void setModeKeys44() {}
    void setModeKeys45() {}

    // from WordRibbon:
    void setModeDialogmode() {}
    void setModeLandscape() {}
    void setModePortrait() {}

    // from MagnifierHost:
    void initialize(const QString &, const QString&, void *) {}
};

// Dummy MImAbstractKeyAreaStyleContainer:
typedef MWidgetStyleContainer MImAbstractKeyAreaStyleContainer;
#endif // HAVE_MEEGOTOUCH
#endif // STYLE_TYPES_H
