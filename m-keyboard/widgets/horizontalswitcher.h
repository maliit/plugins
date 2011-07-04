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


#ifndef HORIZONTALSWITCHER_H
#define HORIZONTALSWITCHER_H

#include <QObject>
#include <QGraphicsWidget>
#include <QGraphicsItemAnimation>
#include <QTimeLine>
#include <QMap>

#ifdef HAVE_MALIIT
#include <maliit/namespace.h>
#else
#include <MNamespace>
#endif

class MKeyOverride;

class HorizontalSwitcher : public QGraphicsWidget
{
    Q_OBJECT

public:
    //! SwitchDirection tells which neighbour should be shown next.
    //! \sa switchTo()
    enum SwitchDirection {
        Left, Right
    };

    explicit HorizontalSwitcher(QGraphicsItem *parent = 0);
    virtual ~HorizontalSwitcher();

    //! \brief Slide to the next widget of current with given direction.
    //!
    //! Does nothing if less than two widgets added.
    //! Does not loop by default if end reached. You can use setLooping()
    //! method to change this behaviour.
    void switchTo(SwitchDirection direction);

    /*!
     * \brief Returns true if it is not possible to switch to the next
     * widget of current with given direction without looping.
     * \param direction Direction of switching
     */
    bool isAtBoundary(SwitchDirection direction) const;

    //! \brief Show given widget without animation.
    //!        The widget must be already added.
    void setCurrent(QGraphicsWidget *widget);

    //! \brief Show widget with given index without animation
    void setCurrent(int index);

    //! \return Currently shown widget's index or -1 if no widget is shown
    int current() const;

    //! \return Currently shown widget or NULL if no widget is shown
    QGraphicsWidget *currentWidget() const;

    //! \return Widget in a given \a index or NULL if index is invalid
    QGraphicsWidget *widget(int index);

    //! \return Number of widgets in the switcher.
    int count() const;

    //! \return true if animation running, false otherwise
    bool isRunning() const;

    //! \brief Tell switcher should it loop when trying to move to
    //!        index after the last one.
    void setLooping(bool enable);

    //! \brief Set duration for sliding animation.
    void setDuration(int ms);

    //! \brief Set easing curve for sliding animation
    void setEasingCurve(const QEasingCurve &curve);

    //! \brief Add widget to switcher. Takes ownership.
    //!
    //! Indices for widgets are added in the order they were added.
    //! If the container was empty (index at -1), then addWidget updates the
    //! current index to point to widget and shows it.
    void addWidget(QGraphicsWidget *widget);

    //! \brief Remove all widgets. Ownership changed to caller.
    //!
    //! Sets current index to -1.
    void removeAll();

    //! \brief Deletes all widgets
    //!
    //! Sets current index to -1.
    void deleteAll();

    /*!
     * \brief Returns whether animation is enabled.
     */
    bool isAnimationEnabled() const;

    /*!
     * \brief Enables or disables animation.
     */
    void setAnimationEnabled(bool enabled);

    /*!
     * \brief Uses custom key overrides which is defined by \a overrides.
     */
    void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides);

     /*!
     * \brief Sets the current content type (handles email/url overrides).
     */
    void setContentType(M::TextContentType type);

    //! \brief Enable or disable gesture recognitions during horizontal movement.
    //! Gestures are always enabled if we have more than one widget in switcher.
    void enableSinglePageHorizontalFlick(bool enable);

signals:
    /*! \brief Signals the beginning of a switch.
     *         This is emitted even if there is no animation.
     *  \param current Index of current widget that is about to be hidden,
     *                  or -1 if there is no current.
     *  \param next Index of the next widget that is shown.
     */
    void switchStarting(int current, int next);

    void switchStarting(QGraphicsWidget *current, QGraphicsWidget *next);

    /*! \brief Signals the ending of a switch.
     *  \param previous Index of widget that is now hidden,
     *                  or -1 if there was no previous.
     *  \param current Index of the current visible widget.
     */
    void switchDone(int previous, int current);

    void switchDone(QGraphicsWidget *previous, QGraphicsWidget *current);

protected:
    //! \reimp
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    //! \reimp_end

private slots:
    void finishAnimation();

private:
    bool isValidIndex(int index) const;

    //! Inform key areas whether they should recognize horizontal gestures or not.
    void updateHorizontalFlickRecognition();

    int currentIndex;
    QList<QGraphicsWidget *> slides;
    QGraphicsItemAnimation enterAnim;
    QGraphicsItemAnimation leaveAnim;
    QTimeLine animTimeLine;
    bool loopingEnabled;
    bool playAnimations;
    bool m_enableSinglePageFlick;

    friend class Ut_HorizontalSwitcher;
};

#endif
