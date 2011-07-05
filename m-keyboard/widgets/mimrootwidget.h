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

#ifndef MIMROOTWIDGET_H
#define MIMROOTWIDGET_H

#include <minputmethodnamespace.h>
#include <QtCore>
#include <QtGui>

class MImRootWidgetPrivate;
//! Helper class to facilitate scene rotation inside a QGraphicsView. Can also
//! be used to query (orientation-independent) widget geometry.
//! This component is a passive and translucent fullscreen overlay. Orientation
//! changes (which affects this widget through item rotation) are driven from
//! the outside.

// TODO: Use layout so that children of this widget get resized by layout, e.g.
// on orientation change.
// TODO: Add Q_PROPERTY for orientation angle?
// TODO: Design width, height and geometry as properties so that changes can be
// propagated correctly? Not everything can be solved as layout.
class MImRootWidget
    : public QGraphicsWidget
{
    Q_OBJECT

public:
    explicit MImRootWidget(QGraphicsItem *parent = 0);
    virtual ~MImRootWidget();

    //! Returns the current orientation angle.
    MInputMethod::OrientationAngle orientationAngle() const;

    //! Sets the new orientation angle.
    void setOrientationAngle(MInputMethod::OrientationAngle angle);

    //! Emitted after orientation angle changed.
    Q_SIGNAL void orientationAngleChanged(MInputMethod::OrientationAngle angle);

    //! Returns the current orientation, based on orientation angle.
    MInputMethod::Orientation orientation() const;

private:
    Q_DECLARE_PRIVATE(MImRootWidget);
    MImRootWidgetPrivate * const d_ptr;
};

#endif // MIMROOTWIDGET_H

