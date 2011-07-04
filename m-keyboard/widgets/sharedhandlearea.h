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

#ifndef SHAREDHANDLEAREA_H
#define SHAREDHANDLEAREA_H

#ifndef HAVE_MEEGOTOUCH
#include <QGraphicsWidget>

class SharedHandleArea
    : public QGraphicsWidget
{};
#else
#include <MWidget>
#include <MNamespace>

#include <QList>
#include <QPointer>

class QGraphicsLinearLayout;
class FlickGesture;
class MImToolbar;
class Handle;

/*!
  \brief SharedHandleArea represents the handle area shared between the vkb and the symbol
  view

  The purpose of shared handle area is to contain an invisible handle (non-zero size only
  in direct mode without close button) and the toolbar, in that order, and to stay on top
  of either vkb or the symbol view, whichever is in a higher position.  The invisible
  handle is currently disabled, though.
*/
class SharedHandleArea : public MWidget
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor
     * \param toolbar toolbar widget
     * \param parent Parent object.
     */
    explicit SharedHandleArea(MImToolbar &toolbar, QGraphicsWidget *parent = 0);

    //! Destructor
    virtual ~SharedHandleArea();

    //! Set input method mode
    void setInputMethodMode(M::InputMethodMode mode);

    /*!
     * \brief Ask handle area to watch on position and visibility of given \a widget.
     */
    void watchOnWidget(QGraphicsWidget *widget);

    //! Update position and geometry when orientation is changed
    void finalizeOrientationChange();

    int shadowHeight() const;
signals:
    void flickUp(const FlickGesture &gesture);
    void flickDown(const FlickGesture &gesture);
    void flickLeft(const FlickGesture &gesture);
    void flickRight(const FlickGesture &gesture);

protected:
    // \reimp
    virtual bool event(QEvent *);
    // \reimp_end

private:
    //! Connect signals from a \a handle widget
    void connectHandle(const Handle &handle);

    //! Toggles visibility based on handleToolbarTypeChange and setInputMethodMode calls
    void updateInvisibleHandleVisibility();

private slots:
    //! \brief Move toolbar when other widgets are visible and they are moved.
    void updatePosition();

private:
    enum LayoutIndex {
        InvisibleHandleIndex,
        ToolbarIndex,
        KeyboardShadowIndex
    };

    QGraphicsLinearLayout &mainLayout;

    //! Invisible gesture handle used only in direct mode
    Handle &invisibleHandle;

    //! Drop shadow on top of everything
    MWidget &keyboardShadow;

    //! Dummy widget we use in place of the invisible gesture handle when it's
    //! not ... err, visible (in its usual invisible way)
    QGraphicsWidget &zeroSizeInvisibleHandle;

    //! Widgets define position of SharedHandleArea: this object
    //! should be placed above other widgets.
    QList<QPointer<QGraphicsWidget> > watchedWidgets;

    MImToolbar &toolbar;

    M::InputMethodMode inputMethodMode;
};
#endif // HAVE_MEEGOTOUCH

#endif
