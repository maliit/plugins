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



#include "widgetbar.h"
#include "mwidget-wrapper.h"

#include <QDebug>
#include <QGraphicsLinearLayout>
#include <QPixmap>

// No use for us but prevents MClassFactory bitching about it.
#ifdef HAVE_MEEGOTOUCH
#include <MWidgetCreator>
M_REGISTER_WIDGET_NO_CREATE(WidgetBar)
#endif

WidgetBar::WidgetBar(QGraphicsItem *parent)
    : MStylableWidget(parent),
      mainLayout(*new QGraphicsLinearLayout(Qt::Horizontal, this))
{
    mainLayout.setSpacing(0); // Spacing is handled by dividers.
    mainLayout.setContentsMargins(0, 0, 0, 0);
}

WidgetBar::~WidgetBar()
{
}

int WidgetBar::count() const
{
    return widgets.count();
}

void WidgetBar::insert(int index, MWidget *widget, bool isAvailable)
{
    Q_ASSERT(widget);

    if (index < 0 || index > count()) {
        qWarning() << "Invalid index given when inserting a widget to WidgetBar.";
        return;
    }

    widgets.insert(index, widget);

    if (isAvailable) {
        mainLayout.insertItem(index, widget);
        mainLayout.setAlignment(widget, Qt::AlignVCenter); // In case we have widgets that differs in height.
    }

    if (-1 != widget->metaObject()->indexOfSignal("availabilityChanged()")) {
        connect(widget, SIGNAL(availabilityChanged()),
                this, SLOT(updateLayout()));
    }
}

void WidgetBar::append(MWidget *widget, bool isAvailable)
{
    insert(count(), widget, isAvailable);
}

MWidget *WidgetBar::widgetAt(int index) const
{
    MWidget *widget = 0;
    if (index >= 0 && index < count()) {
        widget = widgets.at(index);
    }
    return widget;
}

bool WidgetBar::contains(const MWidget *widget) const
{
    return widgets.contains(const_cast<MWidget *>(widget));
}

int WidgetBar::indexOf(const MWidget *widget) const
{
    return widgets.indexOf(const_cast<MWidget *>(widget));
}

void WidgetBar::cleanup()
{
    widgets.removeAll(QPointer<MWidget>());
}

void WidgetBar::mousePressEvent(QGraphicsSceneMouseEvent *)
{
    // Stop propagating
}

QSizeF WidgetBar::sizeHint(Qt::SizeHint which, const QSizeF &constraint) const
{
    // if there is no visible items in the widgetbar, just return empty size.
    if (count() <= 0)
        return QSizeF(0, 0);
    else
        return MStylableWidget::sizeHint(which, constraint);
}

void WidgetBar::updateLayout()
{
    MWidget *widget = qobject_cast<MWidget *>(sender());

    if (!widget) {
        return;
    }

    int index = widgets.indexOf(widget);

    if (index < 0) {
        return;
    }

    if (!widget->isVisible()) {
        //widget is hidden, so just remove it from layout
        mainLayout.removeItem(widget);
    } else {
        //widget is shown, so put it into layout before closest
        //visible neighbor on right side

        //find next visible widget
        ++index;
        while (index < widgets.count() && !widgets.at(index)->isVisible()) {
            ++index;
        }
        int layoutIndex = 0;
        if (index < widgets.count()) {
            //get index of found widget from the layout
            layoutIndex = layoutIndexOf(widgets.at(index));
        } else {
            layoutIndex = mainLayout.count();
        }
        mainLayout.insertItem(layoutIndex, widget);
    }
    emit regionUpdated();
}

int WidgetBar::layoutIndexOf(const MWidget *widget) const
{
    int layoutIndex = 0;

    while (layoutIndex < mainLayout.count()
            && mainLayout.itemAt(layoutIndex) != widget) {
        ++layoutIndex;
    }

    return layoutIndex;
}

