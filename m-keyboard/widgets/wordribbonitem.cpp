#include "wordribbonitem.h"

#ifdef HAVE_MEEGOTOUCH
#include <MScalableImage>
#else
#include "style-types.h"
#endif

#include <QString>
#include <QtCore>
#include <QtGui>

namespace {
    static const int FocusZoneMargin = 30;
}

WordRibbonItem::WordRibbonItem(WordRibbon::ItemStyleMode mode, MWidget* parent):
        MStylableWidget(parent),
        label(""),
        mPositionIndex(-1),
        isMousePressCancelled(false),
        highlightEffectEnabled(true),
        forceMaxWidth(-1),
        state(NormalState),
        mode(mode)
{
    if (textPen.color() != style()->fontColor()) {
        textPen.setColor(style()->fontColor());
    }
 
    recalculateItemSize();
}


WordRibbonItem::~WordRibbonItem()
{
}

void WordRibbonItem::drawBackground(QPainter *painter, const QStyleOptionGraphicsItem *) const
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(painter)
#else
    const WordRibbonItemStyle *s = static_cast<const WordRibbonItemStyle *>(style().operator ->());

    if (!s->backgroundImage() && !s->backgroundImagePressed() && !s->backgroundImageSelected())
        return ;

    qreal oldOpacity = painter->opacity();
    painter->setOpacity(s->backgroundOpacity() * effectiveOpacity());

    QSizeF currentSize = size() - QSizeF(s->marginLeft() + s->marginRight(), s->marginTop() + s->marginBottom());

    switch (state) {
    case NormalState:
        if (s->backgroundImage())
            s->backgroundImage()->draw(0.0, 0.0, currentSize.width(), currentSize.height(), painter);
        break;
    case SelectedState:
        if (s->backgroundImageSelected())
            s->backgroundImageSelected()->draw(0.0, 0.0, currentSize.width(), currentSize.height(), painter);
        break;
    case PressState:
        if (s->backgroundImagePressed())
            s->backgroundImagePressed()->draw(0.0, 0.0, currentSize.width(), currentSize.height(), painter);
        break;
    default:
        break;
    }
    painter->setOpacity(oldOpacity);
#endif
}

void WordRibbonItem::drawContents(QPainter *painter, const QStyleOptionGraphicsItem *option) const
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(painter)
    Q_UNUSED(option)
#else
    if (label.length() == 0) {
        //No content inside candidate item.
        MStylableWidget::drawContents(painter, option);
        return ;
    }

    painter->setFont(drawFont);
    painter->setPen(textPen);

    if (mode == WordRibbon::RibbonStyleMode)
        painter->drawText(contentRect, Qt::AlignCenter, label);
    else
        painter->drawText(contentRect, Qt::AlignLeft, label);
#endif
}

QSizeF WordRibbonItem::sizeHint(Qt::SizeHint which, const QSizeF &) const
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(which)
    static QSize s;
    return s;
#else
    QSizeF size;

    switch (which) {
    case Qt::MinimumSize:
        size = minimumSize;
        break;

    case Qt::PreferredSize:
        size = preferredSize;
        break;

    case Qt::MaximumSize:
        if (forceMaxWidth > 0) {
            size = preferredSize;
            size.setWidth(forceMaxWidth);
        }
        else {
            size = style()->maximumSize();
        }
        break;

    default:
        break;
    }

    return size;
#endif
}

void WordRibbonItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!paddingRect.contains(event->pos().toPoint())) {
        isMousePressCancelled = true;
        return ;
    }

    isMousePressCancelled = false;
    updateStyleState(PressState);

    emit mousePressed();
}

void WordRibbonItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
#ifdef HAVE_MEEGOTOUCH
    if (isMousePressCancelled)
        return ;
    else {
        // Only show "highlighted" state for RibbonStyleMode here.
        if (mode == WordRibbon::RibbonStyleMode)
            highlight();

        emit mouseReleased();
    }
#endif
}

void WordRibbonItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(event)
#else
    if (isMousePressCancelled)
        return ;

    if (mode == WordRibbon::DialogStyleMode) {
        if (!paddingRect.contains(event->pos().toPoint())) {
            // In "DialogStyleMode", when the touch point has been moved
            // out of the area of current item, its "Pressed" state should
            // be cancelled.
            isMousePressCancelled = true;
            clearPress();
        }
    } else {
        QRect moveRect = paddingRect.adjusted(-FocusZoneMargin,
                                              -FocusZoneMargin,
                                              +FocusZoneMargin,
                                              +FocusZoneMargin);
        if (!moveRect.contains(event->pos().toPoint())) {
            // In "RibbonStyleMode", even if the touch point is moved out of
            // the valid touch area of current item, its "Highlighted" state
            // should be kept.
            isMousePressCancelled = true;
            highlight();
        }
    }
#endif
}

#ifdef HAVE_MEEGOTOUCH
void WordRibbonItem::cancelEvent(MCancelEvent *)
{
    isMousePressCancelled = true;
    clearPress();
}
#endif

void WordRibbonItem::setText(const QString &text)
{
    label = text;
    applyStyle();
    recalculateItemSize();
    update();
    updateGeometry();
}

void WordRibbonItem::clearText()
{
    setText("");
}

QString WordRibbonItem::text()
{
    return label;
}

void WordRibbonItem::enableHighlight()
{
    highlightEffectEnabled = true; 
}

void WordRibbonItem::disableHighlight()
{
    highlightEffectEnabled = false; 
}

void WordRibbonItem::highlight()
{
    if (!highlightEffectEnabled
        || highlighted()) {
        return ;
    }

    updateStyleState(SelectedState);
}

void WordRibbonItem::press()
{
    if (!pressed())
        updateStyleState(PressState);
}

void WordRibbonItem::clearHighlight()
{
    if (highlighted())
        updateStyleState(NormalState);
}

void WordRibbonItem::clearPress()
{
    if (pressed())
        updateStyleState(NormalState);
}

bool WordRibbonItem::highlighted() const
{
    if (!highlightEffectEnabled) {
        return false;
    }
    return state == SelectedState;
}

bool WordRibbonItem::pressed() const
{
    return state == PressState;
}

void WordRibbonItem::setPositionIndex(int index)
{
    mPositionIndex = index;
}

int WordRibbonItem::positionIndex() const
{
    return mPositionIndex;
}

void WordRibbonItem::applyStyle()
{
#ifdef HAVE_MEEGOTOUCH
    if (mode == WordRibbon::DialogStyleMode) {
        // In WordRibbon::DialogStyleMode the margins and paddings are depending
        // on the length of the label
        switch (label.length()) {
        case 1:
            style().setModeDialogstyleoneword();
            break;
        case 2:
            style().setModeDialogstyletwoword();
            break;
        case 3:
            style().setModeDialogstylethreeword();
            break;
        default:
            style().setModeDialogstyleseveralword();
            break;
        }
    } else {
        // In WordRibbon::RibbonStyleMode the margins and paddings are depending
        // on the length of the label
        switch (label.length()) {
        case 0:
            style().setModeDefault();
            break;
        case 1:
            style().setModeRibbonstyleoneword();
            break;
        default:
            style().setModeRibbonstyleseveralword();
            break;
        }
    }
#endif
}

void WordRibbonItem::recalculateItemSize()
{
#ifdef HAVE_MEEGOTOUCH
    int paddingLeft, paddingRight, paddingTop, paddingBottom;
    int marginLeft, marginRight, marginTop, marginBottom;

    paddingLeft = style()->paddingLeft();
    paddingRight = style()->paddingRight();
    paddingTop = style()->paddingTop();
    paddingBottom = style()->paddingBottom();
    marginLeft = style()->marginLeft();
    marginRight = style()->marginRight();
    marginTop = style()->marginTop();
    marginBottom = style()->marginBottom();

    minimumSize = style()->minimumSize();
    drawFont = style()->font();

    if (label.length() == 0) {
        preferredSize = minimumSize;
    } else {
        // If label has characters, then the size of this widget
        // need to be update by calculating padding and margin.
        QFontMetrics fm(drawFont);
        QSize textSize = fm.size(Qt::TextSingleLine, label);

        if (mode == WordRibbon::DialogStyleMode) {
            // In WordRibbon::DialogStyleMode, reduce font size for long label
            // to fit the item width
            int maxWidth = forceMaxWidth > 0 ?
                           forceMaxWidth :
                           style()->maximumSize().width();

            int currentWidth = textSize.width() + 
                             marginLeft + marginRight +
                             paddingLeft + paddingRight;

            while (currentWidth > maxWidth) {
                if (drawFont.pixelSize() <= 5) {
                    // too small
                    break;
                }
                drawFont.setPixelSize(drawFont.pixelSize() - 1);
                textSize = fm.size(Qt::TextSingleLine, label);

                currentWidth = textSize.width() + 
                               marginLeft + marginRight +
                               paddingLeft + paddingRight;
            }
        }

        preferredSize.setWidth(textSize.width() +
                               marginLeft + marginRight +
                               paddingLeft + paddingRight);

        preferredSize.setHeight(textSize.height() +
                                marginTop + marginBottom +
                                paddingTop + paddingBottom);

        minimumSize = preferredSize;
    }

    setMinimumSize(minimumSize);
    setPreferredSize(preferredSize);

    QSizeF tmpSize = preferredSize - QSizeF(marginLeft + marginRight,
                                            marginTop + marginBottom);

    paddingRect = QRect(marginLeft, marginTop,
                        tmpSize.width(), tmpSize.height());

    tmpSize = tmpSize - QSizeF(paddingLeft + paddingRight,
                               paddingTop + paddingBottom);

    // ContentRect represents the drawing text area used in drawContents() function.
    // Don't need to calculate margin area.
    // Becuase in MWidgetView::paint(), it will call:
    // painter->translate(horizontalMargin, d->margins.top());
    contentRect = QRect(paddingLeft,
                        paddingTop,
                        tmpSize.width(),
                        tmpSize.height());

    resize(preferredSize);
#endif
}

void WordRibbonItem::updateStyleState(ItemState newState)
{
    state = newState;
    switch (state) {
    case NormalState:
        textPen.setColor(style()->fontColor());
        break;
    case SelectedState:
        textPen.setColor(style()->selectedFontColor());
        break;
    case PressState:
        textPen.setColor(style()->pressedFontColor());
        break;
    default:
        break;
    }

    update();
}

void WordRibbonItem::setMaxWidth(int width)
{
    if (width < style()->minimumSize().width())
        return;

    forceMaxWidth = width;
}
