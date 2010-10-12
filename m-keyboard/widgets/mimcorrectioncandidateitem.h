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

#ifndef MIMCORRECTIONCANDIDATEITEM_H
#define MIMCORRECTIONCANDIDATEITEM_H
#include <MStylableWidget>
#include <MLabel>
#include "mimcorrectioncandidateitemstyle.h"
#include <QTextDocument>

class QTimer;

class MImCorrectionCandidateItem: public MStylableWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(MImCorrectionCandidateItem)
public:
    explicit MImCorrectionCandidateItem(const QString &title = QString(), QGraphicsItem *parent = 0);

    virtual ~MImCorrectionCandidateItem();

    void setTitle(const QString &);

    QString title() const;

    /*!
     * \brief Returns the ideal width of the container widget.
     *
     * The ideal width is the actually used width of the title together with margins and paddings.
     */
    qreal idealWidth() const;
    
    void setSelected(bool);

    bool selected() const;

public Q_SLOTS:
    /*!
     \brief Makes the list cell to send clicked() signal.
     */
     void click();

protected:
    //!reimp
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void drawContents(QPainter *painter, const QStyleOptionGraphicsItem *option) const;
    //!reimp_end

Q_SIGNALS:
     /*!
      \brief The signal is emitted when the item is clicked.
      */
    void clicked();

private Q_SLOTS:
    void applyQueuedStyleModeChange();

private:
    bool mSelected;
    bool mDown;
    QString mTitle;
    QTimer *styleModeChangeTimer;
    bool queuedStyleModeChange;

    void updateStyleMode();

    M_STYLABLE_WIDGET(MImCorrectionCandidateItemStyle)
};

#endif
