/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Copyright (C) 2012 Openismus GmbH
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

#include "models/area.h"
#include "models/keyboard.h"
#include "models/keydescription.h"
#include "models/wordribbon.h"
#include "models/wordcandidate.h"
#include "models/text.h"
#include "models/styleattributes.h"

#include "logic/keyareaconverter.h"
#include "logic/state-machines/shiftmachine.h"
#include "logic/state-machines/viewmachine.h"
#include "logic/state-machines/deadkeymachine.h"

namespace MaliitKeyboard {
namespace {

enum ActivationPolicy {
    ActivateElement,
    DeactivateElement
};

Key makeActive(const Key &key,
               const StyleAttributes *attributes)
{
    if (not attributes) {
        return key;
    }

    Key k(key);

    // FIXME: Use correct key style, but where to look it up?
    k.rArea().setBackground(attributes->keyBackground(KeyDescription::NormalStyle, KeyDescription::PressedState));
    k.rArea().setBackgroundBorders(attributes->keyBackgroundBorders());

    return k;
}

void applyStyleToCandidate(WordCandidate *candidate,
                           const StyleAttributes *attributes,
                           Layout::Orientation orientation,
                           ActivationPolicy policy)
{
    if (not candidate || not attributes) {
        return;
    }

    Label &label(candidate->rLabel());
    Font f(label.font());
    f.setSize(attributes->candidateFontSize(orientation));
    f.setStretch(attributes->candidateFontStretch(orientation));

    QByteArray color;
    switch(policy) {
    case ActivateElement:
        color = QByteArray("#fff");
        break;

    case DeactivateElement:
        color = QByteArray("#ddd");
        break;
    }

    f.setColor(color);
    label.setFont(f);
}

// FIXME: Make word candidates fit word ribbon also after orientation change.
void applyStyleToWordRibbon(WordRibbon *ribbon,
                            const SharedStyle &style,
                            Layout::Orientation orientation)
{
    if (not ribbon || style.isNull()) {
        return;
    }

    Area area;
    StyleAttributes *const a(style->attributes());

    area.setBackground(a->wordRibbonBackground());
    area.setBackgroundBorders(a->wordRibbonBackgroundBorders());
    area.setSize(QSize(a->keyAreaWidth(orientation), a->wordRibbonHeight(orientation)));
    ribbon->setArea(area);
}

bool updateWordRibbon(const SharedLayout &layout,
                      const WordCandidate &candidate,
                      const StyleAttributes *attributes,
                      ActivationPolicy policy)
{
    if (layout.isNull() || not attributes) {
        return false;
    }

    WordRibbon ribbon(layout->wordRibbon());
    QVector<WordCandidate> &candidates(ribbon.rCandidates());

    for (int index = 0; index < candidates.count(); ++index) {
        WordCandidate &current(candidates[index]);

        if (current.label().text() == candidate.label().text()
            && current.rect() == candidate.rect()) {
            applyStyleToCandidate(&current, attributes, layout->orientation(), policy);
            layout->setWordRibbon(ribbon);

            return true;
        }
    }

    return false;
}

Key magnifyKey(const Key &key,
               const StyleAttributes *attributes,
               Layout::Orientation orientation,
               const QRectF &key_area_rect)
{
    Font magnifier_font;
    magnifier_font.setName(attributes->fontName(orientation));
    magnifier_font.setColor(attributes->fontColor(orientation));
    magnifier_font.setSize(attributes->magnifierFontSize(orientation));

    if (key.action() != Key::ActionInsert) {
        return Key();
    }

    QRect magnifier_rect(key.origin(), QSize(attributes->magnifierKeyWidth(orientation),
                                             attributes->magnifierKeyHeight(orientation)));
    magnifier_rect.translate(0, -1 * attributes->verticalOffset(orientation));

    const QRect &mapped(magnifier_rect.translated(key_area_rect.topLeft().toPoint()));

    const int delta_left(mapped.left()
                         - (key_area_rect.left()
                         + attributes->safetyMargin(orientation)));
    const int delta_right((key_area_rect.right()
                           - attributes->safetyMargin(orientation))
                          - mapped.right());

    if (delta_left < 0) {
        magnifier_rect.translate(qAbs<int>(delta_left), 0);
    } else if (delta_right < 0) {
        magnifier_rect.translate(delta_right, 0);
    }

    Key magnifier(key);
    magnifier.setOrigin(magnifier_rect.topLeft());
    magnifier.rArea().setBackground(attributes->magnifierKeyBackground());
    magnifier.rArea().setSize(magnifier_rect.size());
    magnifier.rArea().setBackgroundBorders(attributes->magnifierKeyBackgroundBorders());
    magnifier.rLabel().setFont(magnifier_font);

    // Compute label rectangle, contains the text:
    const qreal label_offset(attributes->magnifierKeyLabelOffset(orientation));
    const QSize &magnifier_size(magnifier.area().size());
    const QRect label_rect(0, 0,
                           magnifier_size.width(),
                           magnifier_size.height() - label_offset);
    magnifier.rLabel().setRect(label_rect);
    magnifier.setMargins(QMargins());

    return magnifier;
}

} // anonymous namespace

class LayoutUpdaterPrivate
{
public:
    bool initialized;
    SharedLayout layout;
    KeyboardLoader loader;
    ShiftMachine shift_machine;
    ViewMachine view_machine;
    DeadkeyMachine deadkey_machine;
    QPoint anchor;
    SharedStyle style;
    Layout::Panel close_extended_on_release;

    explicit LayoutUpdaterPrivate()
        : initialized(false)
        , layout()
        , loader()
        , shift_machine()
        , view_machine()
        , deadkey_machine()
        , anchor()
        , style()
        , close_extended_on_release(Layout::NumPanels) // NumPanels counts as invalid panel.
    {}

    bool inShiftedState() const
    {
        return (shift_machine.inState(ShiftMachine::shift_state) or
                shift_machine.inState(ShiftMachine::caps_lock_state) or
                shift_machine.inState(ShiftMachine::latched_shift_state));
    }

    bool arePrimarySymbolsShown() const
    {
        return view_machine.inState(ViewMachine::symbols0_state);
    }

    bool areSecondarySymbolsShown() const
    {
        return view_machine.inState(ViewMachine::symbols1_state);
    }

    bool areSymbolsShown() const
    {
        return arePrimarySymbolsShown() or areSecondarySymbolsShown();
    }

    bool inDeadkeyState() const
    {
        return (deadkey_machine.inState(DeadkeyMachine::deadkey_state) or
                deadkey_machine.inState(DeadkeyMachine::latched_deadkey_state));
    }

    const StyleAttributes * activeStyleAttributes() const
    {
        return (layout->activePanel() == Layout::ExtendedPanel
                ? style->extendedKeysAttributes() : style->attributes());
    }
};

LayoutUpdater::LayoutUpdater(QObject *parent)
    : QObject(parent)
    , d_ptr(new LayoutUpdaterPrivate)
{
    connect(&d_ptr->loader, SIGNAL(keyboardsChanged()),
            this,           SLOT(onKeyboardsChanged()),
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
    return d->loader.ids();
}

QString LayoutUpdater::activeKeyboardId() const
{
    Q_D(const LayoutUpdater);
    return d->loader.activeId();
}

void LayoutUpdater::setActiveKeyboardId(const QString &id)
{
    Q_D(LayoutUpdater);
    d->loader.setActiveId(id);
}

QString LayoutUpdater::keyboardTitle(const QString &id) const
{
    Q_D(const LayoutUpdater);
    return d->loader.title(id);
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

void LayoutUpdater::setOrientation(Layout::Orientation orientation)
{
    Q_D(LayoutUpdater);

    if (d->layout && d->style && d->layout->orientation() != orientation) {
        d->layout->setOrientation(orientation);

        const KeyAreaConverter converter(d->style->attributes(), &d->loader, d->anchor);
        d->layout->setCenterPanel(d->inShiftedState() ? converter.shiftedKeyArea(orientation)
                                                      : converter.keyArea(orientation));

        WordRibbon ribbon(d->layout->wordRibbon());
        applyStyleToWordRibbon(&ribbon, d->style, orientation);
        d->layout->setWordRibbon(ribbon);

        clearActiveKeysAndMagnifier();
        Q_EMIT layoutChanged(d->layout);
    }
}

void LayoutUpdater::setStyle(const SharedStyle &style)
{
    Q_D(LayoutUpdater);
    if (d->style != style) {
        if (d->style) {
            disconnect(d->style.data(), SIGNAL(profileChanged()),
                       this,            SLOT(applyProfile()));
        }
        d->style = style;
        connect(d->style.data(), SIGNAL(profileChanged()),
                this,            SLOT(applyProfile()));
    }
}

void LayoutUpdater::onKeyPressed(const Key &key,
                                 const SharedLayout &layout)
{
    Q_D(LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->appendActiveKey(makeActive(key, d->activeStyleAttributes()));

    if (d->layout->activePanel() == Layout::CenterPanel) {
        layout->setMagnifierKey(magnifyKey(key, d->activeStyleAttributes(), d->layout->orientation(),
                                           d->layout->centerPanelGeometry()));
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

    if (d->layout != layout || d->layout.isNull() || d->style.isNull()) {
        return;
    }

    clearActiveKeysAndMagnifier();

    const Layout::Orientation orientation(d->layout->orientation());
    StyleAttributes * const extended_attributes(d->style->extendedKeysAttributes());
    const qreal vertical_offset(d->style->attributes()->verticalOffset(orientation));
    const KeyAreaConverter converter(extended_attributes, &d->loader, d->anchor);
    KeyArea ext_ka(converter.extendedKeyArea(orientation, key));

    if (not ext_ka.hasKeys()) {
        return;
    }

    const QSize &ext_panel_size(ext_ka.area().size());
    const QSize &center_panel_size(d->layout->centerPanel().area().size());
    const QPointF &key_center(key.rect().center());
    const qreal safety_margin(extended_attributes->safetyMargin(orientation));

    QPoint offset(qMax<int>(safety_margin, key_center.x() - ext_panel_size.width() / 2),
                  key.rect().top() - vertical_offset);

    if (offset.x() + ext_panel_size.width() > center_panel_size.width()) {
        offset.rx() = center_panel_size.width() - ext_panel_size.width() - safety_margin;
    }

    d->layout->setExtendedPanelOffset(offset);
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

void LayoutUpdater::onKeyAreaPressed(Layout::Panel panel, const SharedLayout &layout)
{
    Q_D(LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    if (d->layout->activePanel() == Layout::ExtendedPanel && panel != Layout::ExtendedPanel) {
        d->close_extended_on_release = panel;
    }
}

void LayoutUpdater::onKeyAreaReleased(Layout::Panel panel, const SharedLayout &layout)
{
    Q_D(LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    if (d->close_extended_on_release == panel) {
        d->layout->setExtendedPanel(KeyArea());
        d->layout->setActivePanel(Layout::CenterPanel);
        Q_EMIT layoutChanged(d->layout);
    }

    d->close_extended_on_release = Layout::NumPanels;
}

void LayoutUpdater::onKeyEntered(const Key &key,
                                 const SharedLayout &layout)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->appendActiveKey(makeActive(key, d->activeStyleAttributes()));

    if (d->layout->activePanel() == Layout::CenterPanel) {
        layout->setMagnifierKey(magnifyKey(key, d->activeStyleAttributes(), d->layout->orientation(),
                                           d->layout->centerPanelGeometry()));
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

void LayoutUpdater::resetOnKeyboardClosed()
{
    Q_D(const LayoutUpdater);

    clearActiveKeysAndMagnifier();
    d->layout->setExtendedPanel(KeyArea());
    d->layout->setActivePanel(Layout::CenterPanel);
    // we do not emit layoutChanged(): there is no need to do it at close and
    // if we do, rendered re-shows the keyboard
}

void LayoutUpdater::onCandidatesUpdated(const QStringList &candidates)
{
    Q_D(LayoutUpdater);

    if (d->layout.isNull()) {
        qWarning() << __PRETTY_FUNCTION__
                   << "No layout specified.";
        return;
    }

    // Copy WordRibbon instance in order to preserve geometry and styling:
    WordRibbon ribbon(d->layout->wordRibbon());
    ribbon.clearCandidates();

    const StyleAttributes * const attributes(d->activeStyleAttributes());
    const Layout::Orientation orientation(d->layout->orientation());
    const int candidate_width(attributes->keyAreaWidth(orientation) / (orientation == Layout::Landscape ? 6 : 4));

    for (int index = 0; index < candidates.count(); ++index) {
        WordCandidate word_candidate;
        word_candidate.rLabel().setText(candidates.at(index));
        word_candidate.rArea().setSize(QSize(candidate_width, 56));
        word_candidate.setOrigin(QPoint(index * candidate_width, 0));
        applyStyleToCandidate(&word_candidate, d->activeStyleAttributes(), orientation, DeactivateElement);
        ribbon.appendCandidate(word_candidate);
    }

    d->layout->setWordRibbon(ribbon);
    Q_EMIT wordCandidatesChanged(d->layout);
}

void LayoutUpdater::onWordCandidatePressed(const WordCandidate &candidate,
                                           const SharedLayout &layout)
{
    Q_D(LayoutUpdater);

    if (layout == d->layout
        && updateWordRibbon(d->layout, candidate, d->activeStyleAttributes(), ActivateElement)) {
        Q_EMIT wordCandidatesChanged(d->layout);
    }
}

void LayoutUpdater::onWordCandidateReleased(const WordCandidate &candidate,
                                            const SharedLayout &layout)
{
    Q_D(LayoutUpdater);

    if (layout == d->layout
        && updateWordRibbon(d->layout, candidate, d->activeStyleAttributes(), DeactivateElement)) {
        Q_EMIT wordCandidatesChanged(d->layout);
        Q_EMIT wordCandidateSelected(candidate.label().text());
    }
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

void LayoutUpdater::applyProfile()
{
    Q_D(const LayoutUpdater);

    if (not d->layout) {
        return;
    }

    if (d->arePrimarySymbolsShown()) {
        switchToPrimarySymView();
    } else if (d->areSecondarySymbolsShown()) {
        switchToSecondarySymView();
    } else if (d->inDeadkeyState()) {
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

    if (not d->layout.isNull() || not d->style.isNull()) {
        WordRibbon ribbon(d->layout->wordRibbon());
        applyStyleToWordRibbon(&ribbon, d->style, d->layout->orientation());
        d->layout->setWordRibbon(ribbon);
    }
}

void LayoutUpdater::switchToMainView()
{
    Q_D(LayoutUpdater);

    if (d->layout.isNull() || d->style.isNull()) {
        return;
    }

    d->layout->clearActiveKeys();
    d->layout->clearMagnifierKey();

    const KeyAreaConverter converter(d->style->attributes(), &d->loader, d->anchor);
    const Layout::Orientation orientation(d->layout->orientation());
    d->layout->setCenterPanel(d->inShiftedState() ? converter.shiftedKeyArea(orientation)
                                                  : converter.keyArea(orientation));

    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::switchToPrimarySymView()
{
    Q_D(LayoutUpdater);

    if (d->layout.isNull() || d->style.isNull()) {
        return;
    }

    const KeyAreaConverter converter(d->style->attributes(), &d->loader, d->anchor);
    const Layout::Orientation orientation(d->layout->orientation());
    d->layout->setCenterPanel(converter.symbolsKeyArea(orientation, 0));

    // Reset shift state machine, also see switchToMainView.
    d->shift_machine.restart();

    //d->shift_machine->start();
    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::switchToSecondarySymView()
{
    Q_D(LayoutUpdater);

    if (d->layout.isNull() || d->style.isNull()) {
        return;
    }

    const KeyAreaConverter converter(d->style->attributes(), &d->loader, d->anchor);
    const Layout::Orientation orientation(d->layout->orientation());
    d->layout->setCenterPanel(converter.symbolsKeyArea(orientation, 1));

    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::switchToAccentedView()
{
    Q_D(LayoutUpdater);

    if (d->layout.isNull() || d->style.isNull()) {
        return;
    }

    const KeyAreaConverter converter(d->style->attributes(), &d->loader, d->anchor);
    const Layout::Orientation orientation(d->layout->orientation());
    const Key accent(d->deadkey_machine.accentKey());
    d->layout->setCenterPanel(d->inShiftedState() ? converter.shiftedDeadKeyArea(orientation, accent)
                                                  : converter.deadKeyArea(orientation, accent));

    Q_EMIT layoutChanged(d->layout);
}

} // namespace MaliitKeyboard
