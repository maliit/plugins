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

#include "mplainwindow.h"

#include <mabstractinputmethodhost.h>
#include <mimsettings.h>

#ifdef HAVE_MEEGOTOUCH
#include <MSceneManager>
#include <MTimestamp>
#include <MComponentData>
#endif

#include <QDebug>

namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
}

MPlainWindow *MPlainWindow::m_instance = 0;

MPlainWindow *MPlainWindow::instance()
{
    return m_instance;
}

MPlainWindow::MPlainWindow(const MAbstractInputMethodHost *newHost,
                           QWidget *parent)
#ifdef HAVE_MEEGOTOUCH
    : MWindow(parent)
#else
    : MImGraphicsView(parent)
#endif
    , host(newHost)
{
    if (m_instance)
        qFatal("There can be only one instance of MPlainWindow");

    m_instance = this;

    // This *does not* prevent plugins from activating multitouch through
    // QGraphicsItem::setAcceptTouchEvents, as the first (enabling) call to
    // that method *will* set the WA_AcceptTouchEvents attribute on all
    // attached viewports (this was probably done in Qt to add some
    // convenience for sloppy programmers).
    //
    // Setting this attribute explicitely here is supposed to guard against
    // changes in above mentioned (undocumented!) "convenience", as this is
    // what the documentation suggests [1].
    //
    // [1] http://doc.trolltech.com/4.6/qtouchevent.html#enabling-touch-events
    if (MImSettings(MultitouchSettings).value().toBool()) {
        setAttribute(Qt::WA_AcceptTouchEvents);
    }

    ungrabGesture(Qt::TapAndHoldGesture);
    ungrabGesture(Qt::PinchGesture);
    ungrabGesture(Qt::PanGesture);
    ungrabGesture(Qt::SwipeGesture);
    ungrabGesture(Qt::TapGesture);
}

MPlainWindow::~MPlainWindow()
{
    m_instance = 0;

#ifdef HAVE_MEEGOTOUCH
    delete sceneManager();
#endif
}

#ifdef HAVE_MEEGOTOUCH
bool MPlainWindow::viewportEvent(QEvent *event)
{
#ifdef M_TIMESTAMP
    QString start;
    QString end;
    start = QString("%1_start").arg(event->type());
    end = QString("%1_end").arg(event->type());

    mTimestamp("MPlainWindow", start);
#endif

    bool result = MWindow::viewportEvent(event);

#ifdef M_TIMESTAMP
    mTimestamp("MPlainWindow", end);
#endif

    return result;
}

void MPlainWindow::drawBackground(QPainter *painter, const QRectF &rect)
{
    if (not host || rect.isEmpty()) {
        return;
    }

    const QPixmap bg(host->background());
    if (not bg.isNull()) {
        painter->drawPixmap(rect, bg, rect);
    }
}
#endif
