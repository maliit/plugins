// -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; c-file-offsets: ((innamespace . 0)); -*-
/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 *
 */

#include "layoutupdater.h"
#include "style.h"
#include "logic/state-machines/shiftmachine.h"
#include "logic/state-machines/viewmachine.h"
#include "logic/state-machines/deadkeymachine.h"
#include "models/keyboard.h"
#include "models/keydescription.h"

namespace MaliitKeyboard {

namespace {

bool verify(const QScopedPointer<KeyboardLoader> &loader,
            const SharedLayout &layout)
{
    return (not loader.isNull() && not layout.isNull());
}

// FIXME: only access settings *once*
QPoint computeAnchor(const QSize &size,
                     Layout::Orientation orientation)
{
    // A workaround for virtual desktops where QDesktopWidget reports
    // screenGeometry incorrectly: Allow user to override via settings.
    QSettings settings("maliit.org", "vkb");
    QPoint anchor(settings.value("anchor").toPoint());

    if (anchor.isNull()) {
        anchor = (orientation == Layout::Landscape)
                  ? QPoint(size.width() / 2, size.height())
                  : QPoint(size.height() / 2, size.width());

        // Enforce creation of settings file, otherwise it's too hard to find
        // the override (and get the syntax ride). I know, it's weird.
        settings.setValue("_anchor", anchor);
    }

    return anchor;
}

KeyArea createFromKeyboard(Style *style,
                           const Keyboard &source,
                           const QPoint &anchor,
                           Layout::Orientation orientation,
                           bool is_extended_keyarea = false)
{
    // An ad-hoc geometry updater that also uses styling information.
    // Will only work for portrait mode (lots of hardcoded stuff).
    KeyArea ka;
    Keyboard kb(source);

    if (not style) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No style given, aborting.";
        return ka;
    }

    style->setStyleName(kb.style_name);

    KeyFont font;
    font.setName(style->fontName());
    font.setSize(style->fontSize());
    font.setColor(QByteArray("#ffffff"));

    KeyFont small_font(font);
    small_font.setSize(12);

    static const QMargins bg_margins(6, 6, 6, 6);

    const qreal max_width(style->keyAreaWidth(orientation));
    const qreal key_height(style->keyHeight(orientation));
    const qreal margin = style->keyMargin(orientation);
    const qreal padding = style->keyAreaPadding(orientation);

    QPoint pos(0, 0);
    QVector<int> row_indices;
    int spacer_count = 0;
    qreal consumed_width = 0;


    for (int index = 0; index < kb.keys.count(); ++index) {
        row_indices.append(index);
        Key &key(kb.keys[index]);
        const KeyDescription &desc(kb.key_descriptions.at(index));
        int width = 0;
        pos.setY(key_height * desc.row);

        bool at_row_end((index + 1 == kb.keys.count())
                        || (index + 1 < kb.keys.count()
                            && kb.key_descriptions.at(index + 1).row > desc.row));

        if (desc.left_spacer || desc.right_spacer) {
            ++spacer_count;
        }

        key.setBackground(style->keyBackground(desc.style, KeyDescription::NormalState));
        key.setBackgroundBorders(bg_margins);

        width = style->keyWidth(orientation, desc.width);
        const bool use_left_padding(pos.x() == 0);
        const bool use_right_padding(pos.x() + width + margin + padding == max_width);

        const qreal key_margin((use_left_padding || use_right_padding) ? margin + padding : margin * 2);
        key.setRect(QRect(pos.x(), pos.y(), width + key_margin, key_height));
        key.setMargins(QMargins(use_left_padding ? padding : margin, margin,
                                use_right_padding ? padding : margin, margin));

        key.setFont(key.text().count() > 1 ? small_font : font);

        // FIXME: Read from KeyDescription instead.
        if (key.text().isEmpty()) {
            switch (key.action()) {
            case Key::ActionShift:
                key.setIcon(style->icon(KeyDescription::ShiftIcon,
                                        KeyDescription::NormalState));
                break;

            case Key::ActionBackspace:
                key.setIcon(style->icon(KeyDescription::BackspaceIcon,
                                        KeyDescription::NormalState));
                break;

            case Key::ActionReturn:
                key.setIcon(style->icon(KeyDescription::ReturnIcon,
                                        KeyDescription::NormalState));
                break;

            default:
                break;
            }
        }

        pos.rx() += key.rect().width();

        if (at_row_end) {
            if (not is_extended_keyarea
                && spacer_count > 0 && pos.x() < max_width + 1) {
                const int spacer_width = qMax<int>(0, max_width - pos.x()) / spacer_count;
                pos.setX(0);
                int right_x = 0;

                Q_FOREACH (int row_index, row_indices) {
                    Key &k(kb.keys[row_index]);
                    const KeyDescription &d(kb.key_descriptions.at(row_index));

                    QRect r(k.rect());
                    QMargins m(k.margins());
                    int extra_width = 0;

                    if (d.left_spacer) {
                        m.setLeft(m.left() + spacer_width);
                        extra_width += spacer_width;
                    }

                    if (d.right_spacer) {
                        m.setRight(m.right() + spacer_width);
                        extra_width += spacer_width;
                    }

                    k.setMargins(m);

                    r.translate(right_x - r.left(), 0);
                    r.setWidth(r.width() + extra_width);
                    k.setRect(r);

                    right_x = r.right();
                }
            }

            consumed_width = qMax<qreal>(consumed_width, key.rect().right() + padding);
            row_indices.clear();
            pos.setX(0);
            spacer_count = 0;
        }
    }

    const int height = pos.y() + key_height;
    ka.keys = kb.keys;
    ka.rect =  QRectF(anchor.x() - (is_extended_keyarea ? consumed_width : max_width) / 2,
                      anchor.y() - height,
                      (is_extended_keyarea ? consumed_width : max_width),
                      height);

    return ka;
}

Key makeActive(const Key &key,
               const Style &style)
{
    // FIXME: Remove test code
    // TEST CODE STARTS
    static const QMargins bg_margins(6, 6, 6, 6);
    // TEST CODE ENDS

    Key k(key);
    // FIXME: Use correct key style, but where to look it up?
    k.setBackground(style.keyBackground(KeyDescription::NormalStyle,
                                        KeyDescription::PressedState));
    k.setBackgroundBorders(bg_margins);

    return k;
}

Key magnifyKey(const Key &key,
               const Style &style,
               const QRectF &key_area_rect)
{
    // FIXME: Remove test code
    // TEST CODE STARTS
    KeyFont magnifier_font;
    magnifier_font.setName(style.fontName());
    magnifier_font.setSize(50);
    magnifier_font.setColor(QByteArray("#ffffff"));

    static const QMargins bg_margins(6, 6, 6, 6);
    // TEST CODE ENDS

    if (key.action() != Key::ActionInsert) {
        return Key();
    }

    Key magnifier(key);
    magnifier.setBackground(style.keyBackground(KeyDescription::NormalStyle,
                                                KeyDescription::PressedState));

    QRect magnifier_rect(key.rect().translated(0, -120).adjusted(-20, -20, 20, 20));
    const QRect &mapped(magnifier_rect.translated(key_area_rect.topLeft().toPoint()));

    const int delta_left(mapped.left() - (key_area_rect.left() + 10));
    const int delta_right((key_area_rect.right() - 10) - mapped.right());

    if (delta_left < 0) {
        magnifier_rect.translate(qAbs<int>(delta_left), 0);
    } else if (delta_right < 0) {
        magnifier_rect.translate(delta_right, 0);
    }

    magnifier.setRect(magnifier_rect);
    magnifier.setBackgroundBorders(bg_margins);
    magnifier.setFont(magnifier_font);

    return magnifier;
}

} // anonymous namespace

class LayoutUpdaterPrivate
{
public:
    bool initialized;
    SharedLayout layout;
    QScopedPointer<KeyboardLoader> loader;
    ShiftMachine shift_machine;
    ViewMachine view_machine;
    DeadkeyMachine deadkey_machine;
    QSize screen_size;
    QPoint anchor;
    Style style;

    explicit LayoutUpdaterPrivate()
        : initialized(false)
        , layout()
        , loader(new KeyboardLoader)
        , shift_machine()
        , view_machine()
        , deadkey_machine()
        , screen_size()
        , anchor()
        , style()
    {
        style.setProfile("nokia-n9");
    }

    bool inShiftedState() const
    {
        return (shift_machine.inState(ShiftMachine::shift_state) or
                shift_machine.inState(ShiftMachine::caps_lock_state) or
                shift_machine.inState(ShiftMachine::latched_shift_state));
    }

    bool areSymbolsShown() const
    {
        return (view_machine.inState(ViewMachine::symbols0_state) or
                view_machine.inState(ViewMachine::symbols1_state));
    }

    bool inDeadkeyState() const
    {
        return (deadkey_machine.inState(DeadkeyMachine::deadkey_state) or
                deadkey_machine.inState(DeadkeyMachine::latched_deadkey_state));
    }
};

LayoutUpdater::LayoutUpdater(QObject *parent)
    : QObject(parent)
    , d_ptr(new LayoutUpdaterPrivate)
{
    connect(d_ptr->loader.data(), SIGNAL(keyboardsChanged()),
            this,                 SLOT(onKeyboardsChanged()),
            Qt::UniqueConnection);
}

LayoutUpdater::~LayoutUpdater()
{}

void LayoutUpdater::init()
{
    Q_D(LayoutUpdater);

    d->shift_machine.setup(this);
    d->view_machine.setup(this);
    d->deadkey_machine.setup(this);
}

QStringList LayoutUpdater::keyboardIds() const
{
    Q_D(const LayoutUpdater);
    return d->loader->ids();
}

QString LayoutUpdater::activeKeyboardId() const
{
    Q_D(const LayoutUpdater);
    return d->loader->activeId();
}

void LayoutUpdater::setActiveKeyboardId(const QString &id)
{
    Q_D(LayoutUpdater);
    d->loader->setActiveId(id);
    d->anchor = computeAnchor(d->screen_size, Layout::Landscape);
}

QString LayoutUpdater::keyboardTitle(const QString &id) const
{
    Q_D(const LayoutUpdater);
    return d->loader->title(id);
}

void LayoutUpdater::setScreenSize(const QSize &size)
{
    Q_D(LayoutUpdater);
    d->screen_size = size;
}

void LayoutUpdater::setLayout(const SharedLayout &layout)
{
    Q_D(LayoutUpdater);
    d->layout = layout;

    if (not d->initialized) {
        init();
        d->initialized = true;
    }
}

//! Replace internal KeyboardLoader with another instance or completely
//! disable it (when set to 0). LayoutUpdater takes ownership.
void LayoutUpdater::resetKeyboardLoader(KeyboardLoader *loader)
{
    Q_D(LayoutUpdater);
    d->loader.reset(loader);

    if (d->loader.isNull()) {
        return;
    }

    connect(loader, SIGNAL(keyboardsChanged()),
            this,   SLOT(onKeyboardsChanged()),
            Qt::UniqueConnection);
}

void LayoutUpdater::setOrientation(Layout::Orientation orientation)
{
    Q_D(LayoutUpdater);

    if (d->layout && d->layout->orientation() != orientation) {
        d->layout->setOrientation(orientation);

        // FIXME: reposition extended keys, too (and left/right?).
        d->anchor = computeAnchor(d->screen_size, orientation);
        d->layout->setCenterPanel(createFromKeyboard(&d->style,
                                                     d->inShiftedState() ? d->loader->shiftedKeyboard()
                                                                         : d->loader->keyboard(),
                                                     d->anchor,
                                                     orientation));
        clearActiveKeysAndMagnifier();
        Q_EMIT layoutChanged(d->layout);
    }
}

void LayoutUpdater::onKeyPressed(const Key &key,
                                 const SharedLayout &layout)
{
    Q_D(LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->appendActiveKey(makeActive(key, d->style));

    if (d->layout->activePanel() == Layout::CenterPanel) {
        layout->setMagnifierKey(magnifyKey(key, d->style, d->layout->activeKeyArea().rect));
    }

    switch (key.action()) {
    case Key::ActionShift:
        Q_EMIT shiftPressed();
        break;

    case Key::ActionDead:
        d->deadkey_machine.setAccentKey(key);
        Q_EMIT deadkeyPressed();
        break;

    default:
        break;
    }

    Q_EMIT keysChanged(layout);
}

void LayoutUpdater::onKeyLongPressed(const Key &key,
                                     const SharedLayout &layout)
{
    Q_UNUSED(key);
    Q_D(LayoutUpdater);

    if (d->layout != layout || not verify(d->loader, d->layout)) {
        return;
    }

    clearActiveKeysAndMagnifier();
    const Keyboard &ext_kb(d->loader->extendedKeyboard(key));

    if (ext_kb.keys.isEmpty()) {
        return;
    }

    const Layout::Orientation orientation(d->layout->orientation());
    KeyArea ext_ka(createFromKeyboard(&d->style, ext_kb, d->anchor, orientation, true));
    const QRectF &center_rect(d->layout->centerPanel().rect);
    const QPointF offset(center_rect.topLeft() + key.rect().center());
    ext_ka.rect.moveTo(QPointF(offset.x() - (ext_ka.rect.width() / 2),
                               offset.y() - d->style.verticalExtendedKeysOffset(orientation)));

    const qreal safety_margin(d->style.extendedKeysSafetyMargin(orientation));
    if (ext_ka.rect.left() < center_rect.left() + safety_margin) {
        ext_ka.rect.moveTo(center_rect.left() + safety_margin, ext_ka.rect.top());
    } else if (ext_ka.rect.right() > center_rect.right() - safety_margin) {
        ext_ka.rect.moveTo(center_rect.right() - ext_ka.rect.width() - safety_margin, ext_ka.rect.top());
    }

    d->layout->setExtendedPanel(ext_ka);
    d->layout->setActivePanel(Layout::ExtendedPanel);
    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::onKeyReleased(const Key &key,
                                  const SharedLayout &layout)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->removeActiveKey(key);
    layout->clearMagnifierKey();

    if (d->layout->activePanel() == Layout::ExtendedPanel) {
        d->layout->clearActiveKeys();
        d->layout->setExtendedPanel(KeyArea());
        d->layout->setActivePanel(Layout::CenterPanel);
        Q_EMIT layoutChanged(d->layout);
    }

    switch (key.action()) {
    case Key::ActionShift:
        Q_EMIT shiftReleased();
        break;

    case Key::ActionInsert:
        if (d->shift_machine.inState(ShiftMachine::latched_shift_state)) {
            Q_EMIT shiftCancelled();
        }

        if (d->deadkey_machine.inState(DeadkeyMachine::latched_deadkey_state)) {
            Q_EMIT deadkeyCancelled();
        }

        break;

    case Key::ActionSym:
        Q_EMIT symKeyReleased();
        break;

    case Key::ActionSwitch:
        Q_EMIT symSwitcherReleased();
        break;

    case Key::ActionDead:
        Q_EMIT deadkeyReleased();
        break;

    default:
        break;
    }

    Q_EMIT keysChanged(layout);
}

void LayoutUpdater::onKeyEntered(const Key &key,
                                 const SharedLayout &layout)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->appendActiveKey(makeActive(key, d->style));

    if (d->layout->activePanel() == Layout::CenterPanel) {
        layout->setMagnifierKey(magnifyKey(key, d->style, d->layout->activeKeyArea().rect));
    }

    Q_EMIT keysChanged(layout);
}

void LayoutUpdater::onKeyExited(const Key &key, const SharedLayout &layout)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->removeActiveKey(key);
    layout->clearMagnifierKey(); // FIXME: This is in a race with onKeyEntered.
    Q_EMIT keysChanged(layout);
}

void LayoutUpdater::clearActiveKeysAndMagnifier()
{
    Q_D(const LayoutUpdater);

    if (d->layout.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No layout specified.";
        return;
    }

    d->layout->clearActiveKeys();
    d->layout->clearMagnifierKey();
}

void LayoutUpdater::syncLayoutToView()
{
    Q_D(const LayoutUpdater);

    if (not d->layout) {
        return;
    }

    // Symbols do not care about shift state.
    if (d->areSymbolsShown()) {
        return;
    }

    if (d->inDeadkeyState()) {
        switchToAccentedView();
    } else {
        switchToMainView();
    }
}

void LayoutUpdater::onKeyboardsChanged()
{
    Q_D(LayoutUpdater);

    // Resetting state machines should reset layout also.
    // FIXME: Most probably reloading will happen three
    // times, which is not what we want.
    d->shift_machine.restart();
    d->deadkey_machine.restart();
    d->view_machine.restart();
}

void LayoutUpdater::switchToMainView()
{
    Q_D(LayoutUpdater);

    if (not verify(d->loader, d->layout)) {
        return;
    }

    d->layout->clearActiveKeys();
    d->layout->clearMagnifierKey();
    d->layout->setCenterPanel(createFromKeyboard(&d->style,
                                                 d->inShiftedState() ? d->loader->shiftedKeyboard()
                                                                     : d->loader->keyboard(),
                                                 d->anchor,
                                                 d->layout->orientation()));
    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::switchToPrimarySymView()
{
    Q_D(LayoutUpdater);

    if (not verify(d->loader, d->layout)) {
        return;
    }

    d->layout->setCenterPanel(createFromKeyboard(&d->style,
                                                 d->loader->symbolsKeyboard(0),
                                                 d->anchor,
                                                 d->layout->orientation()));
    // Reset shift state machine, also see switchToMainView.
    d->shift_machine.restart();

    //d->shift_machine->start();
    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::switchToSecondarySymView()
{
    Q_D(LayoutUpdater);

    if (not verify(d->loader, d->layout)) {
        return;
    }

    d->layout->setCenterPanel(createFromKeyboard(&d->style,
                                                 d->loader->symbolsKeyboard(1),
                                                 d->anchor,
                                                 d->layout->orientation()));
    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::switchToAccentedView()
{
    Q_D(LayoutUpdater);

    if (not verify(d->loader, d->layout)) {
        return;
    }

    d->layout->setCenterPanel(createFromKeyboard(&d->style,
                                                 d->inShiftedState() ? d->loader->shiftedDeadKeyboard(d->deadkey_machine.accentKey())
                                                                     : d->loader->deadKeyboard(d->deadkey_machine.accentKey()),
                                                 d->anchor,
                                                 d->layout->orientation()));
    Q_EMIT layoutChanged(d->layout);
}

} // namespace MaliitKeyboard
