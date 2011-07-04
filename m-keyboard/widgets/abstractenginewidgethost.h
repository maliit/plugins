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

#ifndef ABSTRACTENGINEWIDGETHOST_H
#define ABSTRACTENGINEWIDGETHOST_H

#include <QGraphicsWidget>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QRect>

/*!
  \class AbstractEngineWidgetHost
  \brief The AbstractEngineWidgetHost class is used to show error correction/prediction
  candidate widget.
*/
class AbstractEngineWidgetHost : public QObject
{
    Q_OBJECT

    friend class Ut_AbstractEngineWidgetHost;

public:

    //! Navigation key type.
    //FIXME: Do we need it?
    enum NaviKey {
        NaviKeyOk, 
        NaviKeyLeft, 
        NaviKeyRight, 
        NaviKeyUp, 
        NaviKeyDown
    };

    //! DisplayMode is used by showEngineWidget.
    enum DisplayMode {
        FloatingMode,       //!< the engine widget is floating and following the cursor position.
        DockedMode,         //!< the engine widget is docked to virtual keyboard.
        DialogMode          //!< the engine widget is a dialog.
    };

    /*! Constructor
     *
     */
    explicit AbstractEngineWidgetHost(MWidget *window, QObject *parent = 0)
        : QObject(parent)
    {
        Q_UNUSED(window);
        Q_UNUSED(parent);
    };

    /*! Destructor
     *
     */
    virtual ~AbstractEngineWidgetHost() {};

    /*!
     * \brief Returns the pointer of active engine widget.
     */
    virtual QGraphicsWidget *engineWidget() const = 0;

    /*!
     * \brief Returns true if engine widget is active.
     */
    virtual bool isActive() const = 0;

    /*!
     * \brief Set the title for engine widget if necessary.
     */
    virtual void setTitle(QString &title) = 0;

    /*! 
     * \brief Sets the candidates to engine widget.
     */
    virtual void setCandidates(const QStringList &candidates) = 0;

    /*!
     * \brief Appends the candidates to engine widget.
     */
    virtual void appendCandidates(int startPos, const QStringList &candidate) = 0;

    /*!
     * \brief Returns the candidates currently used by engine widget.
     */
    virtual QStringList candidates() const = 0;

    /*!
     * \brief Shows engine widget with different \a mode.
     *
     * \sa DisplayMode.
     */
    virtual void showEngineWidget(DisplayMode mode = FloatingMode) = 0;

    /*!
     * \brief Returns the widget which is used in inline modes (eg. Docked or Floating) 
     *
     * \sa DisplayMode.
     */
    virtual QGraphicsWidget *inlineWidget() const = 0;

    /*!
     * \brief Hides engine widget.
     */
    virtual void hideEngineWidget() = 0;

    /*!
     * \brief Returns current display mode for the engine widget.
     *
     * \sa DisplayMode.
     */
    virtual DisplayMode displayMode() const = 0;

    /*!
     * \brief Ask engine widget to watch on position and visibility of given \a widget.
     *
     *  Used for DockedMode.
     */
    virtual void watchOnWidget(QGraphicsWidget *widget) = 0;

    /*!
     * \brief Sets the position of FloatingMode engine widget based on cursor rectangle.
     */
    virtual void setPosition(const QRect &cursorRect) = 0;

    /*!
     * \brief Handle Navikey input.
     */
    virtual void handleNavigationKey(NaviKey key) = 0;

    /*!
     * \brief Returns the current suggested word index.
     *
     * The returned index is the word presenting on FloatingMode or the one clicked by user in
     * the DockedMode/DialogMode.
     */
    virtual int suggestedWordIndex() const = 0;

    //! Prepare orientation change
    virtual void prepareToOrientationChange() = 0;

    //! Finalize orientation change
    virtual void finalizeOrientationChange() = 0;

    //! Clear stored candidates and hide engine widget.
    virtual void reset() = 0;

    /*!
     * \brief Sets the index of present page (for DockedMode).
     */
    virtual void setPageIndex(int index = 0) = 0;

    //! is called when the application has actually completed changing orientation
    virtual void handleAppOrientationChanged() = 0;
signals:
    //! This signal is emitted when a candidate is clicked
    void candidateClicked(const QString &selectedWord, int wordIndex);

private:
    Q_DISABLE_COPY(AbstractEngineWidgetHost)
};

#endif
