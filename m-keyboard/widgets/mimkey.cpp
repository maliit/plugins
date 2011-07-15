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



#include "mimkey.h"
#include "mimkeyarea.h"
#include "mvirtualkeyboardstyle.h"
#include "getcssproperty.h"
#include "mimkeystyle.h"

#include <mplainwindow.h>

#include <MTheme>
#include <MScalableImage>
#include <MTimestamp>

#include <QPainter>
#include <QFileInfo>

namespace {
    qreal computeWidth(qreal unit, qreal spacing, const qreal newScaling)
    {
        const qreal scaling = qMax<qreal>(0.0, newScaling);
        return ((scaling * unit) + (qMax<qreal>(0.0, scaling - 1) * spacing));
    }

    // Check whenever a text with a specified font and font size is not bigger than a rectangle area
    // Returns true if the content fits into the rectangle area, otherwise false.
    bool checkFontSize(QFont *font, int size, bool pixelSize, const QString &text, const QRect &boundingRect)
    {
        if (pixelSize) {
            font->setPixelSize(size);
        } else {
            font->setPointSize(size);
        }

        const QFontMetrics fontMetrics(*font);
        const QRect textBounds = fontMetrics.boundingRect(text);

        if (textBounds.width() <= boundingRect.width()
            && textBounds.height() <= boundingRect.height()) {
            return true;
        }
        return false;
    }

    // Modify font to make text fit into boundingRect
    void scaleDownFont(QFont *font, const QString &text, const QRect &boundingRect)
    {
        // Fonts can either be specified in points or pixels
        int fontSize = font->pixelSize();
        const bool usesPixelSize = (fontSize == -1) ? false : true;

        if (!usesPixelSize) {
            fontSize = font->pointSize();
            Q_ASSERT(fontSize != -1);
        }

        int first = 1;
        int last = fontSize;

        if (checkFontSize(font, last, usesPixelSize, text, boundingRect))
            return;

        // Minimum font size is 1
        while (first <= last) {
            int middle = (first+last) / 2;
            bool result = checkFontSize(font, middle, usesPixelSize, text, boundingRect);

            if (result)
                first = middle+1;
            else
                last = middle-1;
        }
    }

    const char * const KeyBackground = "keyBackground";

    const char * const SpecialStyleName = "Special";
    const char * const DeadkeyStyleName = "Deadkey";
    const char * const NormalStyleName = "";

    const char * const NormalStateName = "";
    const char * const PressedStateName = "Pressed";
    const char * const SelectedStateName = "Selected";
    const char * const DisabledStateName = "Disabled";

    const char * const HighlightedName = "Highlighted";

    // Return style name by it's type
    QString style2name(MImKeyModel::StyleType styleType)
    {
        switch (styleType) {
        case MImKeyModel::SpecialStyle:
            return SpecialStyleName;
        case MImKeyModel::DeadkeyStyle:
            return DeadkeyStyleName;
            break;
        case MImKeyModel::NormalStyle:
            return NormalStateName;
            break;
        }

        return QString();
    }

    // TODO: Make this a properly shared style between keys of same area.
    // TOOD: Make name based on actual style name/class type of key area.
    MImKeyStyle gKeyStyle(QLatin1String("MImKey"));
}


MImKey::StylingCache::StylingCache()
    : primary(QFont()),
    secondary(QFont())
{
}

MImKey::IconInfo::IconInfo()
    : pixmap(0)
{
}

MImKey::IconInfo::~IconInfo()
{
    if (pixmap) {
        MTheme::releasePixmap(pixmap);
    }
}

MImKey::Geometry::Geometry()
    : width(0.0)
    , height(0.0)
    , marginLeft(0.0)
    , marginTop(0.0)
    , marginRight(0.0)
    , marginBottom(0.0)
{}

MImKey::Geometry::Geometry(qreal newWidth,
                           qreal newHeight,
                           qreal newMarginLeft,
                           qreal newMarginTop,
                           qreal newMarginRight,
                           qreal newMarginBottom)
    : width(newWidth)
    , height(newHeight)
    , marginLeft(newMarginLeft)
    , marginTop(newMarginTop)
    , marginRight(newMarginRight)
    , marginBottom(newMarginBottom)
{}

MImKey::MImKey(const MImKeyModel &newModel,
               const MImAbstractKeyAreaStyleContainer &style,
               QGraphicsItem &parent,
               const QSharedPointer<StylingCache> &newStylingCache,
               MImFontPool &pool)
    : QGraphicsItem(&parent),
      width(0),
      mModel(newModel),
      shift(false),
      currentLabel(mModel.binding(false) ? mModel.binding(false)->label() : ""),
      currentState(Normal),
      selected(false),
      styleContainer(style),
      currentTouchPointCount(0),
      hasGravity(false),
      rowHasSecondaryLabel(false),
      stylingCache(newStylingCache),
      overrideIcon(0),
      ignoreOverride(false),
      composing(false),
      needsCompactIcon(false),
      fontPool(pool)
{
    if (mModel.binding(false)) {
        loadIcon(false);
    }
    if (mModel.binding(true)) {
        loadIcon(true);
    }

    hide();

    // Initialize keyFontData, in case font() is called before setGeometry() or anything
    // else that triggers updateLabelFont().
    keyFontData = fontPool.font(true);

    //label position should be computed later, when geometry will be known
}

MImKey::~MImKey()
{
}

const QString MImKey::label() const
{
    if (override && !ignoreOverride && !override->label().isEmpty())
        return override->label();

    if (isComposeKey() && !isComposing())
        return QString();

    return currentLabel;
}

const QString MImKey::secondaryLabel() const
{
    return binding().secondaryLabel();
}

const QRectF &MImKey::buttonRect() const
{
    return cachedButtonRect;
}

const QRectF &MImKey::buttonBoundingRect() const
{
    return cachedButtonBoundingRect;
}

void MImKey::handleGeometryChange()
{
    const Geometry &g = currentGeometry;
    cachedButtonBoundingRect = QRectF(pos().x(), pos().y(),
                                      g.width + g.marginLeft + g.marginRight,
                                      g.height + g.marginTop + g.marginBottom);
    cachedButtonRect = cachedButtonBoundingRect.adjusted( g.marginLeft,   g.marginTop,
                                                         -g.marginRight, -g.marginBottom);

    invalidateLabelPos();
    updateNeedsCompactIcon();
    if (override && !override->icon().isEmpty()) {
        loadOverrideIcon(override->icon());
    }
}

void MImKey::setIgnoreOverriding(bool ignore)
{
    ignoreOverride = ignore;

    if (!override) {
        return;
    }

    invalidateLabelPos();
}

void MImKey::overrideBinding(const MImKeyBinding *binding)
{
    const_cast<MImKeyModel&>(mModel).overrideBinding(binding, false);
    const_cast<MImKeyModel&>(mModel).overrideBinding(binding, true);
    currentLabel = this->binding().accented(accent);
    // TODO: new binding could require font change,
    // but we do not have such use case now,
    // so leave it for future
    invalidateLabelPos();
}

void MImKey::invalidateLabelPos()
{
    labelArea = QRectF();
    secondaryLabelArea = QRectF();

    updateLabelFont();
}

void MImKey::updateLabelFont()
{
    // this method does not update stylingCache,
    // because it can not increase font size,
    // so new font will always fit into area prepared
    // for font defined by styling

    // Use a maximum label rectangle that is a bit smaller than the button
    const QRect maximumLabelRect = buttonRect().adjusted(0, 0, -10, -5).toRect();
    const bool shareFont = (model().width() == MImKeyModel::Medium
                           && !keyOverride());
    keyFontData = fontPool.font(shareFont);
    // Skip a non-sense case without real dimensions
    if (buttonRect().width() != 0 && buttonRect().height() != 0) {
        scaleDownFont(keyFontData->font(), label(), maximumLabelRect);
    }
}

void MImKey::updateLabelPos() const
{
    const QRectF paintingArea(currentGeometry.marginLeft,
                              currentGeometry.marginTop,
                              currentGeometry.width,
                              currentGeometry.height);

    if (!rowHasSecondaryLabel) {
        labelArea = paintingArea;
    } else {
        const int labelHeight = stylingCache->primary.height();
        const int secondaryLabelHeight = stylingCache->secondary.height();
        const int topMargin = styleContainer->labelMarginTop();
        const int labelLeftWithSecondary = styleContainer->labelMarginLeftWithSecondary();
        const int secondarySeparation = styleContainer->secondaryLabelSeparation();
        const bool landscape = (MPlainWindow::instance()->orientation() == M::Landscape);

        // In landscape the secondary labels are below the primary ones. In portrait,
        // secondary labels are horizontally next to primary labels.
        if (landscape) {
            // primary label: horizontally centered, top margin defines y
            // secondary: horizontally centered, primary bottom + separation margin defines y
            const int primaryY = paintingArea.top() + topMargin;
            labelArea = QRectF(paintingArea.left(),
                               primaryY,
                               paintingArea.width(),
                               labelHeight);
            if (!secondaryLabel().isEmpty()) {
                secondaryLabelArea = QRectF(paintingArea.left(),
                                            labelArea.bottom() + secondarySeparation,
                                            paintingArea.width(),
                                            secondaryLabelHeight);
            }
        } else {
            // primary label: horizontally according to left margin, vertically centered
            // secondary: horizontally on right of primary + separation margin, vertically centered
            const int primaryX = paintingArea.left() + labelLeftWithSecondary;
            labelArea = QRectF(primaryX,
                               paintingArea.top(),
                               stylingCache->primary.width(label()),
                               paintingArea.height());
            if (!secondaryLabel().isEmpty()) {
                secondaryLabelArea = QRectF(labelArea.right() + secondarySeparation,
                                            paintingArea.top(),
                                            stylingCache->secondary.width(secondaryLabel()),
                                            paintingArea.height());
            }
        }
    }
}

void MImKey::setModifiers(bool shift, QChar accent)
{
    if (this->shift != shift || this->accent != accent) {
        this->shift = shift;
        this->accent = accent;
        currentLabel = binding().accented(accent);

        invalidateLabelPos();
        update();
    }
}

bool MImKey::modifiers(QChar *accent) const
{
    if (accent) {
        *accent = this->accent;
    }
    return shift;
}

void MImKey::setDownState(bool down)
{
    ButtonState newState;

    if (currentState == Disabled) {
        return;
    }

    if (down) {
        // Pressed state is the same for selectable and non-selectable.
        newState = Pressed;
    } else {
        newState = (selected ? Selected : Normal);
    }

    if (newState != currentState) {
        switch (newState) {
        case Pressed:
            // pressed key becomes lastActiveKey regardless to previous state
            activeKeys.removeAll(this);
            activeKeys.append(this);
            break;
        case Normal:
            // inactive key should be removed from list of active keys
            activeKeys.removeAll(this);
            hasGravity = false;
            break;
        case Selected:
            if (currentState == Normal) {
                // selected key is marked as active one,
                // but we should try to keep lastActiveKey unchanged,
                // because key selection happens before next key is pressed
                // but our implementation could trigger this change later
                activeKeys.prepend(this);
            }
            // we have nothing to do if key changes state from Pressed to Selected,
            // because it is already in the list of active keys
            break;
        case Disabled:
            // should never happen
            break;
        }

        currentState = newState;
        setVisible((currentState != Normal) || override);
        update();
    }
}

void MImKey::setSelected(bool select)
{
    if (selected != select && currentState != Disabled) {
        selected = select;

        // refresh state
        setDownState(currentState == Pressed);
    }
}

void MImKey::setComposing(bool compose)
{
    if (!isComposeKey())
        return;
    bool changed = (composing != compose);
    composing = compose;
    if (changed) {
        // Label font should be updated to prevent using old key override label font.
        updateLabelFont();
        update();
    }
}

MImKey::ButtonState MImKey::state() const
{
    return currentState;
}

bool MImKey::isComposing() const
{
    return composing;
}

const MImKeyModel &MImKey::model() const
{
    return mModel;
}

const MImKeyBinding &MImKey::binding() const
{
    if (const MImKeyBinding *b = mModel.binding(shift)) {
        return *b;
    } else {
        qWarning() << __PRETTY_FUNCTION__
                   << "Requested key binding not found!";

        static MImKeyBinding noSuchBinding;
        return noSuchBinding;
    }
}

bool MImKey::isDeadKey() const
{
    return binding().isDead();
}

bool MImKey::isShiftKey() const
{
    return (binding().action() == MImKeyBinding::ActionShift
            && !isDeadKey());
}

bool MImKey::isNormalKey() const
{
    return (binding().action() == MImKeyBinding::ActionInsert
            && !isDeadKey());
}

bool MImKey::isQuickPick() const
{
    return binding().isQuickPick();
}

bool MImKey::isComposeKey() const
{
    const MImKeyBinding::KeyAction action(mModel.binding(shift)->action());
    return (action == MImKeyBinding::ActionCompose);
}

bool MImKey::isBackspaceKey() const
{
    return (binding().action() == MImKeyBinding::ActionBackspace);
}

bool MImKey::increaseTouchPointCount()
{
    if (++currentTouchPointCount <= touchPointLimit()) {
        if (currentTouchPointCount > 0) {
            setDownState(true);
        }

        return true;
    } else {
        --currentTouchPointCount;
        return false;
    }
}

bool MImKey::decreaseTouchPointCount()
{
    if (--currentTouchPointCount >= 0) {
        if (currentTouchPointCount == 0) {
            setDownState(false);
        }

        return true;
    } else {
        ++currentTouchPointCount;
        return false;
    }
}

void MImKey::resetTouchPointCount()
{
    while (decreaseTouchPointCount())
    {}
}

int MImKey::touchPointCount() const
{
    return currentTouchPointCount;
}

int MImKey::touchPointLimit()
{
    return 20;
}

void MImKey::activateGravity()
{
    hasGravity = true;
}

bool MImKey::isGravityActive() const
{
    return hasGravity;
}

const MScalableImage * MImKey::backgroundImage() const
{
    static MScalableImage result;
    static QPixmap bg;

    MImKeyStylingContext ctx(static_cast<MaliitKeyboard::KeyState>(state()),
                             static_cast<MaliitKeyboard::KeyStyle>(model().style()));

    // KeyStateDisabled can only be set through key overrides, but if we are
    // currently ignoring overrides, we need to default to idle state:
    if (ignoreOverride && ctx.state == MaliitKeyboard::KeyStateDisabled) {
        ctx.state = MaliitKeyboard::KeyStateIdle;
    }

    // Check whether the key needs to be highlighted:
    if (not ignoreOverride && override && override->highlighted() && enabled()) {
        ctx.overrides |= MaliitKeyboard::KeyOverrideHighlighted;
    }

    bg = gKeyStyle.background(ctx);
    result.setPixmap(&bg);
    return &result;
}

const MScalableImage *MImKey::normalBackgroundImage() const
{
    static MScalableImage result;
    static QPixmap bg;

    // normalBackgroundImage() always ignores overriden attributes:
    MImKeyStylingContext ctx(MaliitKeyboard::KeyStateIdle,
                             static_cast<MaliitKeyboard::KeyStyle>(model().style()));

    bg = gKeyStyle.background(ctx);
    result.setPixmap(&bg);
    return &result;
}

QRectF MImKey::boundingRect() const
{
    return QRectF(QPointF(), buttonBoundingRect().size());
}

void MImKey::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    mTimestamp("MImKey", "start");

    // We use QGraphicsView::DontSavePainterState, so save/restore state manually
    // Not strictly needed at the moment, but prevents subtle breakage later
    painter->save();
    const MScalableImage *background = backgroundImage();
    const QRectF paintingArea(currentGeometry.marginLeft,
                              currentGeometry.marginTop,
                              currentGeometry.width,
                              currentGeometry.height);
    const QPixmap *iconPixmap(icon());

    if (background) {
        background->draw(paintingArea, painter);
    }

    if (overrideIcon) {
        QPointF iconPos(paintingArea.left() + (paintingArea.width() - overrideIcon->width()) / 2,
                        paintingArea.top() + (paintingArea.height() - overrideIcon->height()) / 2);
        painter->drawPixmap(iconPos, *overrideIcon);

    } else if (iconPixmap && (!override || override->label().isEmpty())) {
        QPointF iconPos(paintingArea.left() + (paintingArea.width() - iconPixmap->width()) / 2,
                        paintingArea.top() + (paintingArea.height() - iconPixmap->height()) / 2);
        painter->drawPixmap(iconPos, *iconPixmap);

    } else {
        painter->setFont(font());
        painter->setPen(fontColor());
        painter->drawText(labelRect(), Qt::AlignCenter, label());
        if (!secondaryLabel().isEmpty()) {
            painter->setFont(styleContainer->secondaryFont());
            painter->drawText(secondaryLabelArea, Qt::AlignCenter, secondaryLabel());
        }
    }
    painter->restore();

    mTimestamp("MImKey", "end");
}

void MImKey::setKeyOverride(const QSharedPointer<MKeyOverride> &newOverride)
{
    if (newOverride == override)
        return;

    MKeyOverride::KeyOverrideAttributes changedAttributes;
    QString label;
    QString icon;
    bool highlighted = false;
    bool enabled = true;

    if (override) {
        label = override->label();
        icon = override->icon();
        highlighted = override->highlighted();
        enabled = override->enabled();
    }

    // check label
    if (label != newOverride->label()) {
        changedAttributes |= MKeyOverride::Label;
    }

    // check icon.
    if (icon != newOverride->icon()) {
        changedAttributes |= MKeyOverride::Icon;
    }

    // check highlighted
    if (highlighted != newOverride->highlighted()) {
        changedAttributes |= MKeyOverride::Highlighted;
    }

    // check enabled
    if (enabled != newOverride->enabled()) {
        changedAttributes |= MKeyOverride::Enabled;
    }

    override = newOverride;

    if (changedAttributes) {
        updateOverrideAttributes(changedAttributes);
    }
}

QSharedPointer<MKeyOverride> MImKey::keyOverride() const
{
    return override;
}

void MImKey::resetKeyOverride()
{
    if (!override)
        return;

    invalidateLabelPos();
    update();
    override.clear();
    delete overrideIcon;
    overrideIcon = 0;
    if (currentState == Disabled) {
        currentState = Normal;
    }
    setVisible(currentState != Normal);
}

const QPixmap *MImKey::icon() const
{
    return iconInfo().pixmap;
}

QString MImKey::iconId() const
{
    return iconInfo().id;
}

void MImKey::drawIcon(QPainter *painter) const
{
    if(isComposeKey() && isComposing())
        return;
    const QPixmap *iconPixmap(icon());
    const QRect rectangle(buttonRect().toRect());

    if (iconPixmap) {
        QPointF iconPos(rectangle.x() + (rectangle.width() - iconPixmap->width()) / 2,
                        rectangle.y() + (rectangle.height() - iconPixmap->height()) / 2);
        painter->drawPixmap(iconPos, *iconPixmap);
    }
}

int MImKey::preferredFixedWidth() const
{
    switch(mModel.width()) {
    case MImKeyModel::Small:
        return styleContainer->keyWidthSmallFixed();

    case MImKeyModel::Medium:
        return styleContainer->keyWidthMediumFixed();

    case MImKeyModel::Large:
        return styleContainer->keyWidthLargeFixed();

    case MImKeyModel::XLarge:
        return styleContainer->keyWidthXLargeFixed();

    case MImKeyModel::XxLarge:
        return styleContainer->keyWidthXxLargeFixed();

    // This one of course makes no real sense:
    case MImKeyModel::Stretched:
        return styleContainer->keyWidthStretchedFixed();
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not find preferred fixed width in style";
    return -1;
}

qreal MImKey::preferredWidth(qreal pixelPerSizeUnit, qreal spacing) const
{
    switch(mModel.width()) {
    case MImKeyModel::Small:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthSmall());

    case MImKeyModel::Medium:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthMedium());

    case MImKeyModel::Large:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthLarge());

    case MImKeyModel::XLarge:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthXLarge());

    case MImKeyModel::XxLarge:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthXxLarge());

    case MImKeyModel::Stretched:
        return computeWidth(pixelPerSizeUnit,
                            spacing,
                            styleContainer->keyWidthStretched());
    }

    qWarning() << __PRETTY_FUNCTION__
               << "Could not find preferred width in style";
    return -1;
}

const MImKey::Geometry &MImKey::geometry() const
{
    return currentGeometry;
}

void MImKey::setGeometry(const MImKey::Geometry &geometry)
{
    currentGeometry = geometry;
    handleGeometryChange();
}

void MImKey::setWidth(qreal width)
{
    currentGeometry.width = width;
    handleGeometryChange();
}

void MImKey::setHeight(qreal height)
{
    currentGeometry.height = height;
    handleGeometryChange();
}

void MImKey::setMargins(qreal left,
                        qreal top,
                        qreal right,
                        qreal bottom)
{
    currentGeometry.marginLeft = left;
    currentGeometry.marginTop = top;
    currentGeometry.marginRight = right;
    currentGeometry.marginBottom = bottom;
    handleGeometryChange();
}

void MImKey::setSecondaryLabelEnabled(bool enable)
{
    if (rowHasSecondaryLabel != enable) {
        rowHasSecondaryLabel = enable;
        invalidateLabelPos();
    }
}

const QRectF & MImKey::labelRect() const
{
    if (labelArea.isNull()) {
        updateLabelPos();
    }

    return labelArea;
}

const QRectF & MImKey::secondaryLabelRect() const
{
    if (labelArea.isNull()) {
        updateLabelPos();
    }

    return secondaryLabelArea;
}

void MImKey::loadIcon(bool shift)
{
    IconInfo &normalIconInfo(shift ? upperCaseIcon : lowerCaseIcon);
    IconInfo &normalIconInfoSelected(shift ? upperCaseIconSelected : lowerCaseIconSelected);
    IconInfo &normalIconInfoHighlighted(shift ? upperCaseIconHighlighted : lowerCaseIconHighlighted);

    IconInfo &compactIconInfo(shift ? upperCaseCompactIcon : lowerCaseCompactIcon);
    IconInfo &compactIconInfoSelected(shift ? upperCaseCompactIconSelected : lowerCaseCompactIconSelected);
    IconInfo &compactIconInfoHighlighted(shift ? upperCaseCompactIconHighlighted : lowerCaseCompactIconHighlighted);

    const MImKeyBinding::KeyAction action(mModel.binding(shift)->action());
    QSize size;
    QString iconProperty;
    QSize compactIconSize;
    bool checkIconForSelectedState = false;
    bool checkIconForHighlightedState = false;

    switch(action) {
        case MImKeyBinding::ActionBackspace:
            iconProperty = "keyBackspaceIconId";
            size = styleContainer->keyBackspaceIconSize();
            compactIconSize = styleContainer->keyBackspaceCompactIconSize();
            break;
        case MImKeyBinding::ActionShift:
            if (shift) {
                iconProperty = "keyShiftUppercaseIconId";
            } else {
                iconProperty = "keyShiftIconId";
            }
            compactIconSize = styleContainer->keyShiftCompactIconSize();
            size = styleContainer->keyShiftIconSize();
            checkIconForSelectedState = true;
            break;
        case MImKeyBinding::ActionReturn:
            if (mModel.binding(shift)->label().isEmpty()) {
                iconProperty = "keyEnterIconId";
                size = styleContainer->keyEnterIconSize();
                compactIconSize = styleContainer->keyEnterCompactIconSize();
            }
            checkIconForHighlightedState = true;
            break;
        case MImKeyBinding::ActionLayoutMenu:
            iconProperty = "keyMenuIconId";
            size = styleContainer->keyMenuIconSize();
            compactIconSize = styleContainer->keyMenuCompactIconSize();
            break;
        case MImKeyBinding::ActionTab:
            if (mModel.binding(shift)->label().isEmpty()) {
                iconProperty = "keyTabIconId";
                size = styleContainer->keyTabIconSize();
                compactIconSize = styleContainer->keyTabCompactIconSize();
            }
            break;
        case MImKeyBinding::ActionCompose:
            iconProperty = "keyEnterIconId";
            size = styleContainer->keyEnterIconSize();
            compactIconSize = styleContainer->keyEnterCompactIconSize();
            break;
        default:
            break;
    }

    normalIconInfo.id = getCSSProperty<QString>(styleContainer, iconProperty, mModel.rtl());

    if (!normalIconInfo.id.isEmpty()) {
        normalIconInfo.pixmap = MTheme::pixmap(normalIconInfo.id, size);
    }

    if (checkIconForSelectedState) {
        normalIconInfoSelected.id = getCSSProperty<QString>(styleContainer,
                                                            QString("%1Selected").arg(iconProperty),
                                                            mModel.rtl());
        if (!normalIconInfoSelected.id.isEmpty()) {
            normalIconInfoSelected.pixmap = MTheme::pixmap(normalIconInfoSelected.id, size);
        }
    }

    if (checkIconForHighlightedState) {
        normalIconInfoHighlighted.id = getCSSProperty<QString>(styleContainer,
                                                               QString("%1Highlighted").arg(iconProperty),
                                                               mModel.rtl());
        if (!normalIconInfoHighlighted.id.isEmpty()) {
            normalIconInfoHighlighted.pixmap = MTheme::pixmap(normalIconInfoHighlighted.id, size);
        }
    }

    const QString compactIconProperty = iconProperty.replace("IconId", "CompactIconId");
    compactIconInfo.id = getCSSProperty<QString>(styleContainer, compactIconProperty, mModel.rtl());

    if (!compactIconInfo.id.isEmpty()) {
        compactIconInfo.pixmap = MTheme::pixmap(compactIconInfo.id, compactIconSize);
    }

    if (checkIconForSelectedState) {
        compactIconInfoSelected.id = getCSSProperty<QString>(styleContainer,
                                                             QString("%1Selected").arg(compactIconProperty),
                                                             mModel.rtl());
        if (!compactIconInfoSelected.id.isEmpty()) {
            compactIconInfoSelected.pixmap = MTheme::pixmap(compactIconInfoSelected.id, compactIconSize);
        }
    }

    if (checkIconForHighlightedState) {
        compactIconInfoHighlighted.id = getCSSProperty<QString>(styleContainer,
                                                                QString("%1Highlighted").arg(iconProperty),
                                                                mModel.rtl());
        if (!compactIconInfoHighlighted.id.isEmpty()) {
            compactIconInfoHighlighted.pixmap = MTheme::pixmap(compactIconInfoHighlighted.id, size);
        }
    }
}

void MImKey::loadOverrideIcon(const QString& icon)
{
    const QSize paintingSize(currentGeometry.width, currentGeometry.height);

    delete overrideIcon;
    overrideIcon = 0;

    if (icon.isEmpty() || !paintingSize.width() || !paintingSize.height()) {
        return;
    }

    QFileInfo fileInfo(icon);

    if (fileInfo.exists() && fileInfo.isAbsolute() && fileInfo.isFile()) {

        overrideIcon = new QPixmap(icon);
        if (overrideIcon->width() > paintingSize.width()
                || overrideIcon->height() > paintingSize.height()) {
            QPixmap *scaled = new QPixmap(overrideIcon->scaled(paintingSize, Qt::KeepAspectRatio));
            delete overrideIcon;
            overrideIcon = scaled;
        }
    }
}


void MImKey::updateNeedsCompactIcon()
{
    const IconInfo &normalIcon = (shift ? upperCaseIcon : lowerCaseIcon);

    if (!normalIcon.pixmap) {
        needsCompactIcon = true;
        return;
    }

    const QSize &iconSize = normalIcon.pixmap->size();
    const QSize &margins = styleContainer->requiredKeyIconMargins();

    // Need to use compact icon if the normal icon does not fit in the key
    needsCompactIcon = buttonRect().width() - iconSize.width() < margins.width()
                       || buttonRect().height() - iconSize.height() < margins.height();
}

const MImKey::IconInfo &MImKey::iconInfo() const
{
    const IconInfo &compactIcon(compactIconInfo());

    return (compactIcon.pixmap && needsCompactIcon) ? compactIcon
                                                    : normalIconInfo();
}

const MImKey::IconInfo &MImKey::normalIconInfo() const
{
    if (override && override->highlighted()) {
        return (shift ? upperCaseIconHighlighted : lowerCaseIconHighlighted);
    }

    if (state() == MImKey::Selected) {
        return (shift ? upperCaseIconSelected : lowerCaseIconSelected);
    }

    return (shift ? upperCaseIcon : lowerCaseIcon);
}

const MImKey::IconInfo &MImKey::compactIconInfo() const
{
    if (override && override->highlighted()) {
        return (shift ? upperCaseCompactIconHighlighted : lowerCaseCompactIconHighlighted);
    }

    if (state() == MImKey::Selected) {
        return (shift ? upperCaseCompactIconSelected : lowerCaseCompactIconSelected);
    }

    return (shift ? upperCaseCompactIcon : lowerCaseCompactIcon);
}

const QFont &MImKey::font() const
{
    return *keyFontData->font();
}

const QColor &MImKey::fontColor() const
{
    // When overridden, keys in Normal, Pressed and Selected state will use
    // overrideColor instead:
    const bool highlighted(override && override->highlighted());
    const QColor &overrideColor =(styleContainer->keyHighlightedFontColor());

    switch (state()) {
    case MImKey::Pressed:
        return (highlighted ? overrideColor : styleContainer->keyPressedFontColor());

    case MImKey::Selected:
        return (highlighted ? overrideColor : styleContainer->keySelectedFontColor());

    // Disabled state ignores override font color:
    case MImKey::Disabled:
        return styleContainer->keyDisabledFontColor();

    default:
        qWarning() << __PRETTY_FUNCTION__ << "Invalid key state:" << state();
    case MImKey::Normal:
        return (highlighted ? overrideColor : styleContainer->fontColor());
    }
}

void MImKey::updateOverrideAttributes(MKeyOverride::KeyOverrideAttributes changedAttributes)
{
    if (!override || !changedAttributes) {
        return;
    }

    if (changedAttributes & MKeyOverride::Label) {
        invalidateLabelPos();
    }

    if (changedAttributes & MKeyOverride::Icon) {
        loadOverrideIcon(override->icon());
    }

    // update() is enough for MKeyOverride::Highlighted
    // and it will be called later in this method

    if (changedAttributes & MKeyOverride::Enabled) {
        if (override->enabled()) {
            currentState = Normal;
        } else {
            currentState = Disabled;
            resetTouchPointCount();
        }
    }

    if (isVisible()) {
        update();
    } else {
        show();
    }
}

