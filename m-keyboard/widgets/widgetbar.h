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



#ifndef WIDGETBAR_H
#define WIDGETBAR_H

#include "widgetbarstyle.h"

#include "style-wrapper.h"

#include <QList>
#include <QPointer>

class MWidget;
class QGraphicsLinearLayout;

/*!
    \brief WidgetBar displays widgets in a horizontal bar.

    Widgets can be delimited with a divider. Divider hides away when widget is down
    or checked.

    Widgets are not owned by this class, though they are reparented.
*/
class WidgetBar : public MStylableWidget
{
    Q_OBJECT
public:
    explicit WidgetBar(QGraphicsItem *parent = 0);

    virtual ~WidgetBar();

    //! \brief Number of widgets added to this widget.
    int count() const;

    //! \brief Inserts a \a widget to specified \a index.
    //! \param isAvailable Contains true if \a widget should appear on the screen
    //!
    //! \a widget must provide availabilityChanged() signal.
    void insert(int index, MWidget *widget, bool isAvailable);

    //! \brief Appends \a widget to the right end of the row.
    //! \param isAvailable Contains true if \a widget should appear on the screen
    void append(MWidget *widget, bool isAvailable);

    //! \brief Retrieves a \a widget at specific \a index.
    MWidget *widgetAt(int index) const;

    //! \brief Check if \a widget has been added.
    bool contains(const MWidget *widget) const;

    //! \brief Returns index of specified \a widget or -1
    // if widget was not found
    int indexOf(const MWidget *widget) const;

    //! Remove all pointers to destroyed widgets from
    // internal lists
    void cleanup();

public:
    //! \reimp
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    //! \reimp_end

signals:
    //! Emitted when availability of inserted widgets and thus the region changes.
    //! Not emitted when widgets are removed/inserted!
    void regionUpdated();

private slots:
    //! Update layout when some child widget is shown or hidden.
    void updateLayout();

protected:
    //! \reimp
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    //! \reimp_end

private:
    //! Return index of given \a widget inside layout
    int layoutIndexOf(const MWidget *widget) const;

private:
    Q_DISABLE_COPY(WidgetBar)

    typedef QList<QPointer<MWidget> > WidgetList;

    QGraphicsLinearLayout &mainLayout;

    WidgetList widgets;

    M_STYLABLE_WIDGET(WidgetBarStyle)

    friend class Ut_WidgetBar;
};

#endif
