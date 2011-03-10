/* * This file is part of meego-im-framework *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 l* and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */

#include "mplainwindow.h"

#include <mabstractinputmethodhost.h>
#include <MSceneManager>
#include <MTimestamp>
#include <MGConfItem>
#include <MComponentData>

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
    : MWindow(parent)
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
    if (MGConfItem(MultitouchSettings).value().toBool()) {
        setAttribute(Qt::WA_AcceptTouchEvents);
    }

    ungrabGesture(Qt::TapAndHoldGesture);
    ungrabGesture(Qt::PinchGesture);
    ungrabGesture(Qt::PanGesture);
    ungrabGesture(Qt::SwipeGesture);
    ungrabGesture(Qt::TapGesture);

    // TO BE REMOVED
    // Workaround to resolve the plugin switching issue, should be
    // removed after imframework has proper solution.
    hide();
}

MPlainWindow::~MPlainWindow()
{
    m_instance = 0;
    delete sceneManager();
}

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
