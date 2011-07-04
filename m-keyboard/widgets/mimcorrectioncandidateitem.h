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

#ifndef MIMCORRECTIONCANDIDATEITEM_H
#define MIMCORRECTIONCANDIDATEITEM_H

#include <QTimer>
#include "style-wrapper.h"
#include "mimcorrectioncandidateitemstyle.h"

class MImCorrectionCandidateItem: public MStylableWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(MImCorrectionCandidateItem)

    Q_PROPERTY(QString title READ title WRITE setTitle)

public:
    explicit MImCorrectionCandidateItem(const QString &title = QString(), QGraphicsItem *parent = 0);

    virtual ~MImCorrectionCandidateItem();

    /*
     * \brief Sets title label.
     */
    void setTitle(const QString &);

    /*
     * \brief Returns current title label.
     */
    QString title() const;

    /*!
     * \brief Returns the ideal width of the container widget.
     *
     * The ideal width is the actually used width of the title together with margins and paddings.
     */
    qreal idealWidth() const;
    
    /*!
     * \brief Select item.
     */
    void setSelected(bool);

    /*
     * \brief Returns selected state.
     */
    bool isSelected() const;

public Q_SLOTS:
    /*!
     \brief Makes the list cell to send clicked() signal.
     */
     void click();

    /*!
     *\brief Makes the list cell to send longTapped signal.
     *\param pos The position of the tap.
     */
    void longTap();

protected:
    /*! \reimp */
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void drawContents(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
    /*! \reimp_end */

Q_SIGNALS:
    /*!
     * \brief The signal is emitted when the item is clicked.
     */
    void clicked();

    /*!
     * \brief The signal is emitted when the item has been tapped and holded.
     */
    void longTapped();

private Q_SLOTS:
    void applyQueuedStyleModeChange();

    void handleVisibilityChanged();

    /*!
     * Update stored style stuffs when the theme changed.
     */
    void onThemeChangeCompleted();

private:
    void updateStyleMode();

    void setupLongTapTimer();

    bool mSelected;
    bool mDown;
    QString mTitle;
    QTimer styleModeChangeTimer;
    QTimer longTapTimer;
    bool queuedStyleModeChange;

    M_STYLABLE_WIDGET(MImCorrectionCandidateItemStyle)
};

#endif
