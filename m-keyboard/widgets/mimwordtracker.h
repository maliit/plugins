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



#ifndef MIMWORDTRACKER_H
#define MIMWORDTRACKER_H

#include <MStylableWidget>
#include "mimwordtrackerstyle.h"

#include <QTimeLine>
#include <QGraphicsView>

class QGraphicsWidget;
class QGraphicsLinearLayout;
class MImCorrectionCandidateItem;
class MReactionMap;

/*!
 * \brief The MImWordTracker class is used to show error correction word tracker.
 */
class MImWordTracker : public MStylableWidget
{
    Q_OBJECT
    friend class Ut_MImWordTracker;
    friend class Ut_MImCorrectionCandidateWidget;

public:
    //! Constructor
    explicit MImWordTracker(QGraphicsWidget *container);

    //! Destructor
    ~MImWordTracker();

    /*!
     * \brief Set suggested candidate.
     */
    void setCandidate(const QString &);

    /*!
     * \brief Returns the suggested candidate.
     */
    QString candidate() const;

    /*!
     * \brief Returns the ideal width of the word tracker.
     *
     * The ideal width is the actually used width of the word tracker together with margins and paddings.
     */
    qreal idealWidth() const;

    /*!
     * \brief Returns the height of pointer for word tracker.
     */
    qreal pointerHeight() const;

    /*!
     * \brief Appears word tracker widget with or without default animation.
     */
    void appear(bool withAnimation = false);

    /*!
     * \brief Disappear word tracker widget with or without default animation.
     */
    void disappear(bool withAnimation = false);

    /*!
     * \brief Sets the position for word tracker according cursor rectangle.
     */
    void setPosition(const QRect &cursorRect);

    /*!
     * \brief Draw its reactive areas onto the reaction map
     */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

signals:
    //! Emitted when word tracker needs reaction map update
    void makeReactionMapDirty();

    //! Emitted when word tracker is clicked.
    void candidateClicked(const QString &);

    /*!
     * \brief The signal is emitted when the word tracker has been tapped and holded.
     */
    void longTapped();

protected slots:
    void select();

    void longTap();

    /*!
     * Method to fade the vkb during transition
     */
    void fade(int);

    /*!
     * This function gets called when fading is finished
     */
    void showHideFinished();

    /*!
     * Update stored style stuffs when the theme changed.
     */
    void onThemeChangeCompleted();

protected:
    /*! \reimp */
    virtual void drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
    QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    /*! \reimp_end */

private:
    void setupTimeLine();

    QGraphicsWidget *containerWidget;
    QString mCandidate;
    int mIdealWidth;
    QGraphicsLinearLayout *mainLayout;
    MImCorrectionCandidateItem *candidateItem;
    QTimeLine showHideTimeline;
    qreal pointerXOffset;
    bool uponCursor;

    M_STYLABLE_WIDGET(MImWordTrackerStyle)
};

#endif
