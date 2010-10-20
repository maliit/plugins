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



#ifndef MIMCORRECTIONCANDIDATEWIDGET_H
#define MIMCORRECTIONCANDIDATEWIDGET_H

#include <QModelIndex>
#include <MWidget>
#include <MStylableWidget>
#include <QTimeLine>
#include "mimcorrectioncandidatecontainerstyle.h"
#include "mimoverlay.h"

class MSceneManager;
class MReactionMap;
class QStringListModel;
class QGraphicsLinearLayout;
class MImCorrectionCandidateItem;

class MImCorrectionCandidateContainer: public MStylableWidget
{
    Q_OBJECT
public:
    explicit MImCorrectionCandidateContainer(QGraphicsItem *parent = 0);

    void setCandidateMaximumWidth(qreal);

    /*!
     * \brief Returns the ideal width of the container widget.
     *
     * The ideal width is the actually used width of the candidate widgets together with margins and paddings.
     */
    qreal idealWidth() const;

    /*!
     * \brief Returns the height of pointer for word tracker.
     */
    int pointerHeight() const;

protected:
    //! reimp
    virtual void drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
    //! reimp_end

private:
    int width;

    M_STYLABLE_WIDGET(MImCorrectionCandidateContainerStyle)
};

/*!
  \class MImCorrectionCandidateWidget
  \brief The MImCorrectionCandidateWidget class is used to show error correction candidate list
*/
class MImCorrectionCandidateWidget: public MImOverlay
{
    Q_OBJECT

    friend class Ut_MImCorrectionCandidateWidget;

public:
    //! CandidateMode is used by showWidget and setMode.
    enum CandidateMode {
        PopupMode,  //!< popup
        SuggestionListMode //!< word suggestion list
    };

    /*! Constructor
     *
     */
    explicit MImCorrectionCandidateWidget();

    /*! Destructor
     *
     */
    ~MImCorrectionCandidateWidget();

    /*! Set the candidate list
     *
     */
    void setCandidates(QStringList candidate);

    /*!
     * \brief Shows candidate widget with different \a mode.
     *
     * \sa CandidateMode.
     */
    virtual void showWidget(CandidateMode mode = PopupMode);

    /*!
     * \brief Hides candidate widget with or without animation according \a withAnimation.
     *
     * \param withAnimation if ture hide widget by useing animation.
     */
    virtual void hideWidget(bool withAnimation = true);


    /*!
     * \brief Returns current used mode for the candidate widget.
     *
     * \sa CandidateMode.
     */
    CandidateMode candidateMode() const;

    /*! \brief Sets the position of candidate list. The list cannot be outside screen.
     *
     */
    void setPosition(const QPoint &pos);

    /*!
     * \brief Sets the position of candidate list based on pre-edit rectangle.
     *
     * Candidate list is put horizontally next to the pre-edit rectangle. Right side
     * of the rectangle is preferred but left side will be used if there is not enough
     * space. Vertically list will be in the middle of the rectangle.
     */
    void setPosition(const QRect &preeditRect);

    /*! Returns the index of preedit string in the candidate list.
     */
    QString suggestion() const;

    /*! Sets the preedit string
     */
    void setPreeditString(const QString &);

    /*! Returns the actual position
     * \return QPoint Returns the actual set position
     */
    QPoint position() const;


    /*! Returns the Preedit String
     */
    QString preeditString() const;

    /*!
     * Draw its reactive areas onto the reaction map
     */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    //! Prepare virtual keyboard for orientation change
    void prepareToOrientationChange();

    //! Finalize orientation change
    void finalizeOrientationChange();

    enum {
        MaxCandidateCount = 5
    };

signals:
    /*! Updates the preedit word
     */
    void candidateClicked(const QString &);

    /*! Updates the screen region used by the widget
     */
    void regionUpdated(const QRegion &);

protected:
    /*! \reimp */
    virtual void hideEvent(QHideEvent *event);
    virtual bool sceneEvent(QEvent *event);
    /*! \reimp_end */

protected slots:
    /*!
     * Method to fade the vkb during transition
     */
    void fade(int);

    /*!
     * This function gets called when fading is finished
     */
    void showHideFinished();

    /*! \reimp */
    virtual void handleVisibilityChanged();
    virtual void handleOrientationChanged();
    /*! \reimp_end */

    void select();

    void longTap();

private:
    QRegion region() const;

    void setupTimeLine();

    bool rotationInProgress;
    QString m_preeditString;
    QPoint candidatePosition;
    MSceneManager *sceneManager;
    MImCorrectionCandidateContainer *containerWidget;
    QStringList candidates;
    CandidateMode currentMode;
    QTimeLine showHideTimeline;
    QString suggestionString;
    QGraphicsLinearLayout *mainLayout;
    MImCorrectionCandidateItem *candidateItems[MaxCandidateCount];

    Q_DISABLE_COPY(MImCorrectionCandidateWidget)
};

#endif
