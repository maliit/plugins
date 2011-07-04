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

#include "handle.h"
#include "mimtoolbar.h"
#include "sharedhandlearea.h"
#include "regiontracker.h"
#include "keyboardshadow.h"
#include "mplainwindow.h"
#include "style-wrapper.h"
#include "mkeyboardhost.h"
#include "mimrootwidget.h"

#include <QDebug>
#include <QGraphicsLinearLayout>

#include <limits>


SharedHandleArea::SharedHandleArea(MImToolbar &toolbar, QGraphicsWidget *parent)
    : MWidget(parent),
      mainLayout(*new QGraphicsLinearLayout(Qt::Vertical, this)),
      invisibleHandle(*new Handle(this)),
      keyboardShadow(*new KeyboardShadow(this)),
      zeroSizeInvisibleHandle(*new QGraphicsWidget(this)),
      toolbar(toolbar),
      inputMethodMode(MInputMethod::InputMethodModeNormal)
{
    setObjectName("SharedHandleArea");
    hide();

    zeroSizeInvisibleHandle.setObjectName("zeroSizeInvisibleHandle");

    mainLayout.setContentsMargins(0, 0, 0, 0);
    mainLayout.setSpacing(0);

    invisibleHandle.setObjectName("InvisibleHandle");
    invisibleHandle.hide();
    zeroSizeInvisibleHandle.setMaximumSize(0, 0);
    zeroSizeInvisibleHandle.show();
    mainLayout.addItem(&zeroSizeInvisibleHandle);
    connectHandle(invisibleHandle);

    mainLayout.addItem(&toolbar);
    mainLayout.setAlignment(&toolbar, Qt::AlignCenter);

    keyboardShadow.setObjectName("KeyboardShadow");
    keyboardShadow.show();
    keyboardShadow.setZValue(1);
    mainLayout.addItem(&keyboardShadow);

    RegionTracker::instance().addRegion(toolbar);
    RegionTracker::instance().addInputMethodArea(toolbar);
    RegionTracker::instance().setGeometryProxy(toolbar, *this);

    connect(this, SIGNAL(visibleChanged()), this, SLOT(updatePosition()));
}


SharedHandleArea::~SharedHandleArea()
{
}


void SharedHandleArea::connectHandle(const Handle &handle)
{
    connect(&handle, SIGNAL(flickLeft(const FlickGesture &)), this, SIGNAL(flickLeft(const FlickGesture &)));
    connect(&handle, SIGNAL(flickRight(const FlickGesture &)), this, SIGNAL(flickRight(const FlickGesture &)));
    connect(&handle, SIGNAL(flickUp(const FlickGesture &)), this, SIGNAL(flickUp(const FlickGesture &)));
    connect(&handle, SIGNAL(flickDown(const FlickGesture &)), this, SIGNAL(flickDown(const FlickGesture &)));
}


void SharedHandleArea::updateInvisibleHandleVisibility()
{
    // For now we never enable the invisible handle.  The code is kept here until it's
    // certain the invisible handle won't be needed anymore.  Note: if you enable this,
    // you probably need to connect flick signals of this class and enable corresponding
    // ut_mkeyboardhost region test code.  If/When you remove this, you can also remove
    // setInputMethodMode, handleToolbarTypeChange and typeChanged signal from MImToolbar,
    // connectHandle and in fact everything related to handles from this file and the
    // header.  You also need to update addRegion and rename this class to something more
    // meaningful.
#if 0
    // Toggle invisible gesture handle area on/off
    const bool showInvisibleHandle(inputMethodMode == MInputMethod::InputMethodModeDirect
                                   && !standardToolbar);
    QGraphicsWidget &previousItem(*dynamic_cast<QGraphicsWidget *>(
                                      mainLayout.itemAt(InvisibleHandleIndex)));
    mainLayout.removeItem(&previousItem);
    previousItem.hide();
    QGraphicsWidget &newItem(showInvisibleHandle ? invisibleHandle : zeroSizeInvisibleHandle);
    mainLayout.insertItem(InvisibleHandleIndex, &newItem);
    newItem.setVisible(showInvisibleHandle);
    updatePosition();
#endif
}

void SharedHandleArea::setInputMethodMode(const MInputMethod::InputMethodMode mode)
{
    inputMethodMode = mode;
    updateInvisibleHandleVisibility();
}


void SharedHandleArea::updatePosition()
{
    qreal bottom(std::numeric_limits<qreal>::max());
    bool widgetVisible(false);

    foreach (const QGraphicsWidget *widget, watchedWidgets) {
        if (widget && widget->isVisible()) {
            bottom = qMin(widget->pos().y(), bottom);
            widgetVisible = true;
        }
    }

    const QPointF newPos(0, bottom - size().height());

    if (widgetVisible && (newPos != pos())) {
        setPos(newPos);
    }
}

void SharedHandleArea::watchOnWidget(QGraphicsWidget *widget)
{
    if (!widget) {
        return;
    }

    connect(widget, SIGNAL(yChanged()), this, SLOT(updatePosition()));
    connect(widget, SIGNAL(visibleChanged()), this, SLOT(updatePosition()));
    watchedWidgets.append(widget);
    updatePosition();

    watchedWidgets.removeAll(QPointer<QGraphicsWidget>()); //remove all invalid pointers
}

void SharedHandleArea::finalizeOrientationChange()
{
    //set proper width
#ifdef HAVE_MEEGOTOUCH
    resize(MPlainWindow::instance()->visibleSceneSize().width(),
           size().height());
#else
    resize(MKeyboardHost::instance()->rootWidget()->size().width(),
           size().height());
#endif
    updatePosition();
}

int SharedHandleArea::shadowHeight() const
{
    if (keyboardShadow.isVisible()) {
        return keyboardShadow.size().height();
    } else {
        return 0;
    }
}

bool SharedHandleArea::event(QEvent *e)
{
    if (e->type() == QEvent::GraphicsSceneResize) {
        // when share handler area is resized,need to call updatePosition
        // to make sure the toolbar in the proper position.
        updatePosition();
    }
    return MWidget::event(e);
}
