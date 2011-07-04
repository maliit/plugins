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

#ifndef MIMOVERLAY_H
#define MIMOVERLAY_H

#ifdef HAVE_MEEGOTOUCH
#include <MSceneWindow>
#else
#include <QGraphicsWidget>
class MSceneWindow : public QGraphicsWidget
{};
#endif

/*!
  * \brief The MImOverLay class could be used to show an plain translucent overlay widget.
  *
  * MImOverlay is a translucent overlay widget, fully covering the visible screen area. While visible,
  * it prevents mouse and touch events from reaching the virtual keyboard or the application.
*/
class MImOverlay : public MSceneWindow
{
    Q_OBJECT

    friend class Ut_MImOverlay;

public:
    //! Constructor
    explicit MImOverlay();

    //! Destructor
    virtual ~MImOverlay();

protected slots:
    virtual void handleOrientationChanged();

protected:
    /*! \reimp */
    virtual bool sceneEvent(QEvent *event);
    /*! \reimp_end */

private:
    Q_DISABLE_COPY(MImOverlay)

    // Caching the GConf setting in order to avoid expensive GConf
    // reads during creation of this overlay widget.
    static bool acceptTouchEventsSetting();
};

#endif
