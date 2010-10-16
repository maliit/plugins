/* * This file is part of meego-keyboard *
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
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */



#include "mimoverlay.h"
#include "mvirtualkeyboard.h"

#include <MSceneManager>
#include <MScene>
#include <MSceneWindow>
#include <MGConfItem>
#include <mplainwindow.h>

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QString>
#include <float.h>

namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSetting = "/meegotouch/inputmethods/multitouch/enabled";

    MVirtualKeyboard *findVKB(const QGraphicsScene *scene)
    {
        MVirtualKeyboard *found = 0;

        foreach (QGraphicsItem *item, scene->items()) {
            found = dynamic_cast<MVirtualKeyboard *>(item);

            if (found) {
                break;
            }
        }

        return found;
    }
}


MImOverlay::MImOverlay()
    : MWidget()
    , parentWindow(new MSceneWindow)
{
    setParentItem(parentWindow);
    parentWindow->setManagedManually(true);
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(parentWindow);
    // The z-value should always be more than vkb and text widget's z-value
    parentWindow->setZValue(FLT_MAX);

    // By default multi-touch is disabled
    setAcceptTouchEvents(MGConfItem(MultitouchSetting).value().toBool());

    setGeometry(QRectF(QPointF(0, 0), MPlainWindow::instance()->sceneManager()->visibleSceneSize()));

    connect(this, SIGNAL(visibleChanged()),
            this, SLOT(handleVisibilityChanged()));

    connect(MPlainWindow::instance()->sceneManager(), SIGNAL(orientationChanged(M::Orientation)),
            this, SLOT(handleOrientationChanged()));

    MVirtualKeyboard *vkb = findVKB(MPlainWindow::instance()->scene());
    connect(this, SIGNAL(regionUpdated(QRegion)),
            vkb,  SLOT(sendVKBRegion(QRegion)));

    hide();

}

MImOverlay::~MImOverlay()
{
}

bool MImOverlay::sceneEvent(QEvent *e)
{
    MWidget::sceneEvent(e);

    // eat all the touch and mouse press/release  events to avoid these events
    // go to the background virtual keyboard.
    e->setAccepted(e->isAccepted()
                   || e->type() == QEvent::TouchBegin
                   || e->type() == QEvent::TouchUpdate
                   || e->type() == QEvent::TouchEnd
                   || e->type() == QEvent::GraphicsSceneMousePress
                   || e->type() == QEvent::GraphicsSceneMouseRelease);
    return e->isAccepted();
}

void MImOverlay::handleVisibilityChanged()
{
    if (!isVisible()) {
        emit regionUpdated(QRegion());
    } else {
        // Extend overlay window to whole screen area.
        emit regionUpdated(mapRectToScene(QRect(QPoint(0, 0), MPlainWindow::instance()->sceneManager()->visibleSceneSize())).toRect());
    }
}

void MImOverlay::handleOrientationChanged()
{
    setGeometry(QRectF(QPointF(0, 0), MPlainWindow::instance()->sceneManager()->visibleSceneSize()));
    if (isVisible()) {
        emit regionUpdated(mapRectToScene(QRect(QPoint(0, 0), MPlainWindow::instance()->sceneManager()->visibleSceneSize())).toRect());
    }
}
