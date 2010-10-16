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



#include "mvirtualkeyboardstyle.h"
#include "singlewidgetbuttonarea.h"

#include <QDebug>
#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QTextCharFormat>
#include <QTextLine>

#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <mplainwindow.h>
#include <mreactionmap.h>
#include <MTheme>

namespace {
    template<class T>
    int binaryRangeFind(T value,
                        const QVector<QPair<T, T> > &offsets)
    {
        int lowerBound = 0;
        int upperBound = offsets.size() - 1;

        while (lowerBound <= upperBound) {
            const int pivot = (lowerBound + upperBound) / 2;
            const QPair<T, T> &current = offsets.at(pivot);

            if (value < current.first) {
                upperBound = pivot - 1;
            } else if (value > current.second) {
                lowerBound = pivot + 1;
            } else {
                return pivot;
            }
        }

        // not found:
        return -1;
    }
}

SingleWidgetButtonArea::SingleWidgetButtonArea(const LayoutData::SharedLayoutSection &sectionModel,
                                               bool usePopup,
                                               QGraphicsWidget *parent)
    : KeyButtonArea(sectionModel, usePopup, parent),
      rowList(sectionModel->rowCount()),
      widgetHeight(computeWidgetHeight()),
      mMaxNormalizedWidth(maxNormalizedWidth()),
      shiftButton(0),
      textDirty(false),
      equalWidthButtons(true)
{
    textLayout.setCacheEnabled(true);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    loadKeys();
}

SingleWidgetButtonArea::~SingleWidgetButtonArea()
{
    // Release any key that might be pressed before destroying them.
    clearActiveKeys();

    for (RowIterator rowIter(rowList.begin()); rowIter != rowList.end(); ++rowIter) {
        qDeleteAll(rowIter->buttons);
        rowIter->buttons.clear();
    }
}

QSizeF SingleWidgetButtonArea::sizeHint(Qt::SizeHint which, const QSizeF &/*constraint*/) const
{
    int width = 0;
    if (which == Qt::MaximumSize) {
        // We're willing to grow as much as we can. Some parent widget
        // will apply a constraint for this.
        width = QWIDGETSIZE_MAX;
    }
    return QSizeF(width, widgetHeight);
}

void SingleWidgetButtonArea::drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view)
{
    reactionMap->setTransform(this, view);
    reactionMap->setReactiveDrawingValue();

    foreach (const ButtonRow &row, rowList) {
        foreach (const SingleWidgetButton *const button, row.buttons) {
            reactionMap->fillRectangle(button->buttonBoundingRect());
        }
    }
}

void SingleWidgetButtonArea::loadKeys()
{
    const int numRows = rowCount();

    RowIterator rowIter(rowList.begin());

    for (int row = 0; row != numRows; ++row, ++rowIter) {
        const int numColumns = sectionModel()->columnsAt(row);

        rowIter->stretchButton = 0;

        // Add buttons
        for (int col = 0; col < numColumns; ++col) {
            // Parameters to fetch from base class.
            VKBDataKey *dataKey = sectionModel()->vkbKey(row, col);
            SingleWidgetButton *button = new SingleWidgetButton(*dataKey, baseStyle(), *this);

            // TODO: Remove restriction to have only one shift button per layout?
            if (dataKey->binding()->action() == KeyBinding::ActionShift) {
                shiftButton = button;
            }

            // Only one stretching item per row.
            if (!rowIter->stretchButton) {
                rowIter->stretchButton = (dataKey->width() == VKBDataKey::Stretched ? button : 0);
            }

            rowIter->buttons.append(button);
        }
    }

    updateGeometry();
}

void SingleWidgetButtonArea::buildTextLayout()
{
    textDirty = false;

    textLayout.clearLayout();

    QList<QTextLayout::FormatRange> formatList;
    QTextCharFormat secondaryFormat;
    secondaryFormat.setFont(baseStyle()->secondaryFont());

    // QTextLayout requires text content to be set before creating QTextLines.
    // While concatenating the text, also build 'additional formats' used for secondary
    // labels. This must be done before QTextLayout::beginLayout().
    QString labelContent;

    foreach (const ButtonRow &row, rowList) {

        foreach (const SingleWidgetButton *button, row.buttons) {
            // primary label
            QString label = button->label();

            if (!label.isEmpty()) {
                // Add whitespace for QTextLine to be able to cut.
                labelContent += label + " ";

                // try secondary label
                label = button->secondaryLabel();

                if (!label.isEmpty()) {
                    // Add formatting for this secondary label.
                    QTextLayout::FormatRange formatRange = {labelContent.length(), label.length(), secondaryFormat};
                    formatList.append(formatRange);
                    labelContent += label + " ";
                }
            }
        }
    }

    // Apply the formats
    if (!formatList.isEmpty()) {
        textLayout.setAdditionalFormats(formatList);
    }

    QFontMetrics fm(baseStyle()->font());
    QFontMetrics secondaryFm(secondaryFormat.font());
    const int labelHeight = fm.height();
    const int secondaryLabelHeight = secondaryFm.height();
    const int topMargin = baseStyle()->labelMarginTop();
    const int labelLeftWithSecondary = baseStyle()->labelMarginLeftWithSecondary();
    const int secondarySeparation = baseStyle()->secondaryLabelSeparation();
    const bool landscape = (MPlainWindow::instance()->orientation() == M::Landscape);

    textLayout.setFont(baseStyle()->font());
    textLayout.setText(labelContent);

    textLayout.beginLayout();

    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        // rowHasSecondaryLabel is needed for the vertical alignment of
        // secondary label purposes.
        bool rowHasSecondaryLabel = false;
        foreach (SingleWidgetButton *button, row->buttons) {
            if (!button->secondaryLabel().isEmpty()) {
                rowHasSecondaryLabel = true;
            }
        }

        foreach (SingleWidgetButton *button, row->buttons) {

            const QString &label(button->label());
            const QString &secondary(button->secondaryLabel());
            QPoint labelPos;
            QPoint secondaryLabelPos;

            // We must not create a new QTextLine if there is no label.
            if (label.isEmpty()) {
                continue;
            }

            const QRectF &buttonRect = button->cachedButtonRect;

            if (!rowHasSecondaryLabel) {
                // All horizontally centered.
                labelPos = fm.boundingRect(buttonRect.x(),
                                           buttonRect.y(),
                                           buttonRect.width(),
                                           buttonRect.height(),
                                           Qt::AlignCenter,
                                           label).topLeft();
            } else {
                // Calculate position for both primary and secondary label.
                // We only have secondary labels in phone number layouts (with sym being exception)
                // so this follows their styling.

                // In landscape the secondary labels are below the primary ones. In portrait,
                // secondary labels are horizontally next to primary labels.
                if (landscape) {
                    // primary label: horizontally centered, top margin defines y
                    // secondary: horizontally centered, primary bottom + separation margin defines y
                    const int primaryY = buttonRect.top() + topMargin;
                    labelPos.setX(buttonRect.center().x() - fm.width(label) / 2);
                    labelPos.setY(primaryY);
                    if (!secondary.isEmpty()) {
                        secondaryLabelPos.setX(buttonRect.center().x() - secondaryFm.width(secondary) / 2);
                        secondaryLabelPos.setY(primaryY + labelHeight + secondarySeparation);
                    }
                } else {
                    // primary label: horizontally according to left margin, vertically centered
                    // secondary: horizontally on right of primary + separation margin, vertically centered
                    const int primaryX = buttonRect.left() + labelLeftWithSecondary;
                    labelPos.setX(primaryX);
                    labelPos.setY(buttonRect.center().y() - labelHeight / 2);
                    if (!secondary.isEmpty()) {
                        secondaryLabelPos.setX(primaryX + fm.width(label) + secondarySeparation);
                        secondaryLabelPos.setY(buttonRect.center().y() - secondaryLabelHeight / 2);
                    }
                }
            }

            // We now have positions, let's create some QTextLines.

            // Create the primary label
            QTextLine line = textLayout.createLine();
            if (!line.isValid()) {
                // We are getting out of sync anyway so no point in continuing.
                goto endLayout;
            }
            line.setNumColumns(label.length()); // will be seeked forward until next whitespace
            line.setPosition(labelPos);

            // Same for secondary label
            if (!secondary.isEmpty()) {
                line = textLayout.createLine();
                if (!line.isValid()) {
                    goto endLayout;
                }
                line.setNumColumns(secondary.length());
                line.setPosition(secondaryLabelPos);
            }
        }
    }

endLayout:
    textLayout.endLayout();
}

qreal SingleWidgetButtonArea::computeWidgetHeight() const
{
    qreal height = -(baseStyle()->spacingVertical())
                   + baseStyle()->paddingTop()
                   + baseStyle()->paddingBottom();

    for (int index = 0; index < rowList.count(); ++index) {
        height += preferredRowHeight(index);
        height += baseStyle()->spacingVertical();
    }

    return qMax<qreal>(0.0, height);
}

void SingleWidgetButtonArea::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const MScalableImage *background = baseStyle()->backgroundImage();

    if (background) {
        background->draw(boundingRect().toRect(), painter);
    }

    // Draw images first.
    foreach (const ButtonRow &row, rowList) {
        foreach (const SingleWidgetButton *button, row.buttons) {

            // Note: we should always get scalable images directly from style container.
            // Caching pointers of the images is very danger, because images could be
            // deleted by mtheme daemon in some cases (e.g. display language is changed).

            // Draw button background.
            drawKeyBackground(painter, button);

            // Draw icon.
            button->drawIcon(button->cachedButtonRect.toRect(), painter);
        }
    }

    if (textDirty) {
        buildTextLayout();
    }
    // Draw text next.
    painter->setPen(baseStyle()->fontColor());
    textLayout.draw(painter, QPoint());
}

void SingleWidgetButtonArea::drawKeyBackground(QPainter *painter,
                                               const SingleWidgetButton *button)
{
    if (!button) {
        return;
    }

    const MScalableImage *background = 0;

    switch (button->state()) {

    case IKeyButton::Normal:
        switch (button->key().style()) {
        case VKBDataKey::SpecialStyle:
            background = baseStyle()->keyBackgroundSpecial();
            break;
        case VKBDataKey::DeadkeyStyle:
            background = baseStyle()->keyBackgroundDeadkey();
            break;
        case VKBDataKey::NormalStyle:
        default:
            background = baseStyle()->keyBackground();
            break;
        }
        break;

    case IKeyButton::Pressed:
        switch (button->key().style()) {
        case VKBDataKey::SpecialStyle:
            background = baseStyle()->keyBackgroundSpecialPressed();
            break;
        case VKBDataKey::DeadkeyStyle:
            background = baseStyle()->keyBackgroundDeadkeyPressed();
            break;
        case VKBDataKey::NormalStyle:
        default:
            background = baseStyle()->keyBackgroundPressed();
            break;
        }
        break;

    case IKeyButton::Selected:
        switch (button->key().style()) {
        case VKBDataKey::SpecialStyle:
            background = baseStyle()->keyBackgroundSpecialSelected();
            break;
        case VKBDataKey::DeadkeyStyle:
            background = baseStyle()->keyBackgroundDeadkeySelected();
            break;
        case VKBDataKey::NormalStyle:
        default:
            background = baseStyle()->keyBackgroundSelected();
            break;
        }
        break;

    default:
        break;
    }

    if (background) {
        background->draw(button->cachedButtonRect.toRect(), painter);
    }
}

IKeyButton *SingleWidgetButtonArea::keyAt(const QPoint &pos) const
{
    const int numRows = rowList.count();

    if (numRows == 0) {
        return 0;
    }

    const int rowIndex = binaryRangeFind<int>(pos.y(), rowOffsets);

    if (rowIndex == -1) {
        return 0;
    }

    const ButtonRow &currentRow = rowList.at(rowIndex);
    const int buttonIndex = binaryRangeFind<qreal>(pos.x(), currentRow.buttonOffsets);

    if (buttonIndex == -1) {
        return 0;
    }

    return currentRow.buttons.at(buttonIndex);
}

void SingleWidgetButtonArea::setShiftState(ModifierState newShiftState)
{
    if (shiftButton) {
        shiftButton->setModifiers(newShiftState != ModifierClearState);
        shiftButton->setSelected(newShiftState == ModifierLockedState);
    }
}

void SingleWidgetButtonArea::modifiersChanged(const bool shift, const QChar accent)
{
    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        foreach (SingleWidgetButton *button, row->buttons) {
            // Shift button and selected keys are detached from the normal level changing.
            if (button != this->shiftButton
                && button->state() != IKeyButton::Selected) {
                button->setModifiers(shift, accent);
            }
        }
    }

    textDirty = true;

}

void SingleWidgetButtonArea::updateButtonGeometriesForWidth(const int newAvailableWidth)
{
    if (sectionModel()->maxColumns() == 0) {
        return;
    }

    widgetHeight = computeWidgetHeight();
    rowOffsets.clear();

    const qreal HorizontalSpacing = baseStyle()->spacingHorizontal();
    const qreal VerticalSpacing = baseStyle()->spacingVertical();

    // The following code cannot handle negative width:
    int availableWidth = qMax<qreal>(0, newAvailableWidth
                                        - (baseStyle()->paddingLeft() + baseStyle()->paddingRight()));

    const qreal normalizedWidth = qMax<qreal>(1.0, mMaxNormalizedWidth);
    const qreal availableWidthForButtons = availableWidth - ((normalizedWidth - 1) * HorizontalSpacing);
    mRelativeButtonBaseWidth = availableWidthForButtons / normalizedWidth;
    emit relativeButtonBaseWidthChanged(mRelativeButtonBaseWidth);

    // This is used to update the button rectangles
    qreal y = baseStyle()->paddingTop();

    // Button margins
    const qreal leftMargin = HorizontalSpacing / 2;
    const qreal rightMargin = HorizontalSpacing - leftMargin;
    const qreal topMargin = VerticalSpacing / 2;
    const qreal bottomMargin = VerticalSpacing - topMargin;

    QRectF br; // button bounding rectangle
    const qreal rowListFactor = (rowList.count() > 1 ? 1 : 0);

    for (RowIterator row(rowList.begin()); row != rowList.end(); ++row) {
        const qreal rowHeight = preferredRowHeight(row - rowList.begin());
        br.setHeight(rowHeight + baseStyle()->spacingVertical() * rowListFactor);

        row->buttonOffsets.clear();

        // Store the row offsets for fast key lookup:
        const int lastRowOffset = (rowOffsets.isEmpty()) ? baseStyle()->paddingTop()
                                                           - (baseStyle()->spacingVertical() / 2) * rowListFactor
                                                         : rowOffsets.at(rowOffsets.count() - 1).second;

        rowOffsets.append(QPair<int, int>(lastRowOffset,
                                          lastRowOffset + static_cast<int>(br.height() + 0.5)));

        // Update row width
        qreal rowWidth = 0;
        foreach (SingleWidgetButton *button, row->buttons) {
            button->width = button->preferredWidth(mRelativeButtonBaseWidth, HorizontalSpacing);
            rowWidth += button->width + HorizontalSpacing;
        }
        rowWidth -= HorizontalSpacing;

        qreal availableWidthForSpacers = 0;
        const QList<int> spacerIndices = sectionModel()->spacerIndices(row - rowList.begin());
        int spacerCount = row->stretchButton ? spacerIndices.count() + 1
                                             : spacerIndices.count();

        if (row->stretchButton) {
            rowWidth -=  row->stretchButton->width;

            // Handle the case of one stretch button/no other spacer elments directly:
            if (spacerCount == 1) {
                row->stretchButton->width = availableWidth - rowWidth;
                rowWidth = availableWidth;
                spacerCount = 0;
            }
        }

        if ((spacerCount > 0) && (availableWidth > rowWidth)) {
            availableWidthForSpacers = (availableWidth - rowWidth) / spacerCount;

            if (row->stretchButton) {
                row->stretchButton->width = availableWidthForSpacers;
            }
        }

        // We can precalculate button rectangles.
        br.moveTop(y - (rowList.count() > 1 ? topMargin : 0));

        // A spacer with an index of -1 means it was put before any button in that row.
        // Also add layout padding:
        qreal x = baseStyle()->paddingLeft() + spacerIndices.count(-1) * availableWidthForSpacers;

        for (int buttonIndex = 0; buttonIndex < row->buttons.count(); ++buttonIndex) {
            SingleWidgetButton *button = row->buttons.at(buttonIndex);

            br.moveLeft(x - leftMargin);
            br.setWidth(button->width + leftMargin + rightMargin);

            // save it (but cover up for rounding errors, ie, extra spacing pixels):
            button->cachedBoundingRect = br.adjusted(-1, 0, 1, 0);
            button->cachedButtonRect = br.adjusted(leftMargin, topMargin, -rightMargin, -bottomMargin);

            // Store the button offsets for fast key lookup:
            row->buttonOffsets.append(QPair<qreal, qreal>(button->cachedBoundingRect.left(),
                                                          button->cachedBoundingRect.right()));

            // Increase x to the next button bounding rect border.
            x += button->width + HorizontalSpacing;

            // Our spacerIndex is a multi-set, hence we need to add
            // availableWidthForSpacers for every ocurrence of spacerIndex:
            x += spacerIndices.count(buttonIndex) * availableWidthForSpacers;
        }

        y += br.height();
    }

    // Positions may have changed, rebuild text layout.
    textDirty = true;
}

QRectF SingleWidgetButtonArea::boundingRect() const
{
    return QRectF(0, 0, size().width(), widgetHeight);
}

qreal SingleWidgetButtonArea::preferredRowHeight(int row) const
{
    const qreal normalHeight = baseStyle()->keyHeight();

    switch (sectionModel()->rowHeightType(row)) {

    case LayoutSection::Small:
        return normalHeight * baseStyle()->rowHeightSmall();
        break;

    case LayoutSection::Medium:
        return normalHeight * baseStyle()->rowHeightMedium();
        break;

    case LayoutSection::Large:
        return normalHeight * baseStyle()->rowHeightLarge();
        break;

    case LayoutSection::XLarge:
        return normalHeight * baseStyle()->rowHeightXLarge();
        break;

    case LayoutSection::XxLarge:
        return normalHeight * baseStyle()->rowHeightXxLarge();
        break;
    }

    return 0.0;
}

qreal SingleWidgetButtonArea::maxNormalizedWidth() const
{
    qreal maxRowWidth = 0.0;

    for (int j = 0; j < sectionModel()->rowCount(); ++j) {
        qreal rowWidth = 0.0;
        for (int i = 0; i < sectionModel()->columnsAt(j); ++i) {
            const VKBDataKey *key = sectionModel()->vkbKey(j, i);
            rowWidth += normalizedKeyWidth(key);
        }
        maxRowWidth = qMax(maxRowWidth, rowWidth);
    }
    return maxRowWidth;
}

qreal SingleWidgetButtonArea::normalizedKeyWidth(const VKBDataKey *key) const
{
    switch(key->width()) {
    case VKBDataKey::Small:
        return baseStyle()->keyWidthSmall();

    case VKBDataKey::Medium:
    case VKBDataKey::Stretched:
        return baseStyle()->keyWidthMedium();

    case VKBDataKey::Large:
        return baseStyle()->keyWidthLarge();

    case VKBDataKey::XLarge:
        return baseStyle()->keyWidthXLarge();

    case VKBDataKey::XxLarge:
        return baseStyle()->keyWidthXxLarge();
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not compute normalized width from style";
    return 0.0;
}

void SingleWidgetButtonArea::onThemeChangeCompleted()
{
    mMaxNormalizedWidth = maxNormalizedWidth();
    KeyButtonArea::onThemeChangeCompleted();
    buildTextLayout();
}
