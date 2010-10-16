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



#ifndef SINGLEWIDGETBUTTONAREA_H
#define SINGLEWIDGETBUTTONAREA_H

#include "keybuttonarea.h"
#include "singlewidgetbutton.h"

#include <MScalableImage>
#include <QTextLayout>

/*!
 * \brief SingleWidgetButtonArea is an implementation of KeyButtonArea which
 * does not use separate widgets for buttons, but instead draws them explicitly.
 */
class SingleWidgetButtonArea : public KeyButtonArea
{
public:
    SingleWidgetButtonArea(const LayoutData::SharedLayoutSection &sectionModel,
                           bool usePopup = false,
                           QGraphicsWidget *parent = 0);

    virtual ~SingleWidgetButtonArea();

    //! \reimp
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *view);
    virtual QRectF boundingRect() const;
    virtual void setShiftState(ModifierState newShiftState);
    //! \reimp_end

protected:
    /*! \reimp */
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint) const;
    virtual void drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view);
    virtual void updateButtonGeometriesForWidth(int availableWidth);
    virtual IKeyButton *keyAt(const QPoint &pos) const;
    virtual void modifiersChanged(bool shift, QChar accent = QChar());
    virtual void onThemeChangeCompleted();
    /*! \reimp_end */

private:
    //! \brief Creates buttons for key data models
    void loadKeys();

    //! \brief Builds QTextLayout representation of current button labels for faster drawing.
    void buildTextLayout();

    qreal computeWidgetHeight() const;

    //! \brief Return preferred height for a row
    qreal preferredRowHeight(int row) const;

    //! \brief Get the maximum width in this widget, in normalized units
    qreal maxNormalizedWidth() const;

    //! \brief Normalized width for a particular VKBDataKey.
    qreal normalizedKeyWidth(const VKBDataKey *key) const;


    //! \brief Draws background.
    void drawKeyBackground(QPainter *painter, const SingleWidgetButton *button);

    struct ButtonRow {
        QList<SingleWidgetButton*> buttons;
        QVector<QPair<qreal, qreal> > buttonOffsets;

        //! each row can have one stretch button
        SingleWidgetButton *stretchButton;
    };

    typedef QVector<ButtonRow> ButtonRowList;
    typedef ButtonRowList::iterator RowIterator;
    typedef ButtonRowList::const_iterator ConstRowIterator;

    ButtonRowList rowList;
    qreal widgetHeight;
    qreal mMaxNormalizedWidth;

    QVector<QPair<int, int> > rowOffsets;

    //! Shift button is stored here if current layout has a shift button.
    SingleWidgetButton *shiftButton;

    QTextLayout textLayout;
    bool textDirty;

    bool equalWidthButtons;

#ifdef UNIT_TEST
    friend class Ut_KeyButtonArea;
    friend class Ut_KeyEventHandler;
    friend class Bm_KeyButtonArea; //benchmarks
#endif

    friend class FixtureVirtualKeyboard; //!< needed for testing
};

#endif // SINGLEWIDGETBUTTONAREA_H
