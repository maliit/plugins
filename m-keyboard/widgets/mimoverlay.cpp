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



#include "mimoverlay.h"
#include "mplainwindow.h"
#include "mkeyboardhost.h"
#include "mimrootwidget.h"

#ifdef HAVE_MEEGOTOUCH
#include <MSceneManager>
#endif

#include <mimsettings.h>
#include <QString>
#include <float.h>

namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSetting = "/meegotouch/inputmethods/multitouch/enabled";
};

MImOverlay::MImOverlay()
    : MSceneWindow()
{
#ifdef HAVE_MEEGOTOUCH
    setManagedManually(true);
    if (MPlainWindow::instance()) {
        MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(this);
    }
#endif
    // The z-value should always be more than vkb and text widget's z-value
    setZValue(FLT_MAX);

    // By default multi-touch is disabled
    setAcceptTouchEvents(acceptTouchEventsSetting());

    if (MPlainWindow::instance()) {
#ifdef HAVE_MEEGOTOUCH
        setGeometry(QRectF(QPointF(0, 0), MPlainWindow::instance()->sceneManager()->visibleSceneSize()));
        connect(MPlainWindow::instance()->sceneManager(), SIGNAL(orientationChanged(M::Orientation)),
#else
        setGeometry(QRectF(QPointF(0, 0), MKeyboardHost::instance()->rootWidget()->size()));
        connect(MKeyboardHost::instance()->rootWidget(), SIGNAL(orientationChanged(MInputMethod::Orientation)),
#endif
                this, SLOT(handleOrientationChanged()));
    }
    hide();
}

MImOverlay::~MImOverlay()
{
}

bool MImOverlay::sceneEvent(QEvent *e)
{
    MSceneWindow::sceneEvent(e);

    // eat all touch and mouse events to avoid these events
    // go to the background virtual keyboard.
    e->setAccepted(e->isAccepted()
                   || e->type() == QEvent::TouchBegin
                   || e->type() == QEvent::GraphicsSceneMousePress);
    return e->isAccepted();
}

void MImOverlay::handleOrientationChanged()
{
#ifdef HAVE_MEEGOTOUCH
    if (MPlainWindow::instance()) {
        setGeometry(QRectF(QPointF(0, 0), MPlainWindow::instance()->sceneManager()->visibleSceneSize()));
    }
#else
    setGeometry(QRectF(QPointF(0, 0), MKeyboardHost::instance()->rootWidget()->size()));
#endif
}

bool MImOverlay::acceptTouchEventsSetting()
{
    static bool gConfRead = false;
    static bool touchEventsAccepted = false;
    if (!gConfRead) {
        touchEventsAccepted = MImSettings(MultitouchSetting).value().toBool();
        gConfRead = true;
    }
    return touchEventsAccepted;
}
