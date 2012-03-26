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
#include "logic/keyareaconverter.h"
#include "logic/state-machines/shiftmachine.h"
#include "logic/state-machines/viewmachine.h"
#include "logic/state-machines/deadkeymachine.h"
#include "models/area.h"
#include "models/keyboard.h"
#include "models/keydescription.h"
#include "models/wordribbon.h"
#include "models/wordcandidate.h"

#ifdef HAVE_PRESAGE
#include <presage.h>
#endif

namespace MaliitKeyboard {

#ifdef HAVE_PRESAGE
class CandidatesCallback
    : public PresageCallback
{
private:
    const std::string& m_past_context;
    const std::string m_empty;

public:
    explicit CandidatesCallback(const std::string& past_context);

    std::string get_past_stream() const;
    std::string get_future_stream() const;
};

CandidatesCallback::CandidatesCallback(const std::string &past_context)
    : m_past_context(past_context)
    , m_empty()
{}

std::string CandidatesCallback::get_past_stream() const
{
    return m_past_context;
}

std::string CandidatesCallback::get_future_stream() const
{
    return m_empty;
}
#endif

namespace {

Key makeActive(const Key &key,
               const Style *style)
{
    Key k(key);

    // FIXME: Use correct key style, but where to look it up?
    k.rArea().setBackground(style->keyBackground(KeyDescription::NormalStyle, KeyDescription::PressedState));
    k.rArea().setBackgroundBorders(style->keyBackgroundBorders());

    return k;
}

WordCandidate makeActive(const WordCandidate &candidate,
                         const Style *style)
{
    Q_UNUSED(style)

    WordCandidate c(candidate);
    Font f(c.label().font());
    f.setSize(14);
    f.setColor("#fff");
    c.rLabel().setFont(f);

    return c;
}

WordCandidate makeInactive(const WordCandidate &candidate,
                           const Style *style)
{
    Q_UNUSED(style)

    WordCandidate c(candidate);
    Font f(c.label().font());
    f.setSize(14);
    f.setColor("#ddd");
    c.rLabel().setFont(f);

    return c;
}

Key magnifyKey(const Key &key,
               const Style *style,
               const QRectF &key_area_rect)
{
    // FIXME: Remove test code
    // TEST CODE STARTS
    Font magnifier_font;
    magnifier_font.setName(style->fontName());
    magnifier_font.setSize(50);
    magnifier_font.setColor(QByteArray("#ffffff"));

    static const QMargins bg_margins(6, 6, 6, 6);
    // TEST CODE ENDS

    if (key.action() != Key::ActionInsert) {
        return Key();
    }

    Key magnifier(key);
    QRect magnifier_rect(key.rect().translated(0, -120).adjusted(-20, -20, 20, 20));
    const QRect &mapped(magnifier_rect.translated(key_area_rect.topLeft().toPoint()));

    const int delta_left(mapped.left() - (key_area_rect.left() + 10));
    const int delta_right((key_area_rect.right() - 10) - mapped.right());

    if (delta_left < 0) {
        magnifier_rect.translate(qAbs<int>(delta_left), 0);
    } else if (delta_right < 0) {
        magnifier_rect.translate(delta_right, 0);
    }

    magnifier.setOrigin(magnifier_rect.topLeft());
    magnifier.rArea().setBackground(style->keyBackground(KeyDescription::NormalStyle,
                                                         KeyDescription::PressedState));
    magnifier.rArea().setSize(magnifier_rect.size());
    magnifier.rArea().setBackgroundBorders(bg_margins);
    magnifier.rLabel().setFont(magnifier_font);

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
    Style style;
    Style extended_keys_style;
#ifdef HAVE_PRESAGE
    std::string candidates_context;
    CandidatesCallback candidates;
    Presage presage;
#endif

    explicit LayoutUpdaterPrivate()
        : initialized(false)
        , layout()
        , loader()
        , shift_machine()
        , view_machine()
        , deadkey_machine()
        , anchor()
        , style()
        , extended_keys_style()
#ifdef HAVE_PRESAGE
        , candidates_context()
        , candidates(CandidatesCallback(candidates_context))
        , presage(&candidates)
#endif
    {
        style.setProfile("nokia-n9");
        extended_keys_style.setProfile("nokia-n9-extended-keys");
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

    const Style * activeStyle() const
    {
        return (layout->activePanel() == Layout::ExtendedPanel
                ? &extended_keys_style : &style);
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

    if (d->layout && d->layout->orientation() != orientation) {
        d->layout->setOrientation(orientation);

        const KeyAreaConverter converter(&d->style, &d->loader, d->anchor);
        d->layout->setCenterPanel(d->inShiftedState() ? converter.shiftedKeyArea(orientation)
                                                      : converter.keyArea(orientation));

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

    layout->appendActiveKey(makeActive(key, d->activeStyle()));

    if (d->layout->activePanel() == Layout::CenterPanel) {
        layout->setMagnifierKey(magnifyKey(key, d->activeStyle(),
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

    if (d->layout != layout || d->layout.isNull()) {
        return;
    }

    clearActiveKeysAndMagnifier();

    const Layout::Orientation orientation(d->layout->orientation());
    const KeyAreaConverter converter(&d->extended_keys_style, &d->loader, d->anchor);
    KeyArea ext_ka(converter.extendedKeyArea(orientation, key));

    if (not ext_ka.hasKeys()) {
        return;
    }

    const QSize &ext_panel_size(ext_ka.area().size());
    const QSize &center_panel_size(d->layout->centerPanel().area().size());
    const QPointF &key_center(key.rect().center());
    const qreal safety_margin(d->extended_keys_style.safetyMargin(orientation));

    QPoint offset(qMax<int>(safety_margin, key_center.x() - ext_panel_size.width() / 2),
                  key_center.y() - d->extended_keys_style.verticalOffset(orientation));

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

void LayoutUpdater::onKeyEntered(const Key &key,
                                 const SharedLayout &layout)
{
    Q_D(const LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    layout->appendActiveKey(makeActive(key, d->activeStyle()));

    if (d->layout->activePanel() == Layout::CenterPanel) {
        layout->setMagnifierKey(magnifyKey(key, d->activeStyle(), d->layout->centerPanelGeometry()));
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

void LayoutUpdater::onSurroundingTextChanged(const QString &surround, int offset)
{

#ifndef HAVE_PRESAGE
    Q_UNUSED(surround)
    Q_UNUSED(offset)
#else
    Q_D(LayoutUpdater);

    if (d->layout.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No layout specified.";
        return;
    }

    d->candidates_context = surround.left(offset).toStdString();

    // request prediction
    const std::vector<std::string> predictions = d->presage.predict();

    WordRibbon ribbon(d->layout->wordRibbon());
    ribbon.clearCandidates();

    // TODO: Fine-tune presage behaviour to also perform error correction, not just word prediction.
    if (not surround.isEmpty()) {
        // FIXME: max_candidates should come from style, too:
        const static unsigned int max_candidates = 7;
        for (unsigned int index = 0; index < predictions.size() && index < max_candidates; ++index) {
            WordCandidate candidate;
            candidate.rLabel().setText(QString::fromStdString(predictions.at(index)));
            // FIXME: compute based on VKB width?
            candidate.rArea().setSize(QSize(96, 40));
            candidate.setOrigin(QPoint(index * 96, 0));
            // FIXME: should be void applyStyle, or such
            candidate = makeInactive(candidate, d->activeStyle());
            ribbon.appendCandidate(candidate);
        }
    }

    d->layout->setWordRibbon(ribbon);
    Q_EMIT wordCandidatesChanged(d->layout);
#endif
}

void LayoutUpdater::onWordCandidatePressed(const WordCandidate &candidate,
                                           const SharedLayout &layout)
{
    Q_D(LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    WordRibbon ribbon(layout->wordRibbon());
    QVector<WordCandidate> &candidates(ribbon.rCandidates());

    for (int index = 0; index < candidates.count(); ++index) {
        const WordCandidate &current(candidates.at(index));

        if (current.label().text() == candidate.label().text()
            && current.rect() == candidate.rect()) {
            candidates.replace(index, makeActive(candidate, d->activeStyle()));
            layout->setWordRibbon(ribbon);

            Q_EMIT wordCandidatesChanged(layout);
            break;
        }
    }
}

void LayoutUpdater::onWordCandidateReleased(const WordCandidate &candidate,
                                            const SharedLayout &layout)
{
    Q_D(LayoutUpdater);

    if (d->layout != layout) {
        return;
    }

    WordRibbon ribbon(layout->wordRibbon());
    QVector<WordCandidate> &candidates(ribbon.rCandidates());

    for (int index = 0; index < candidates.count(); ++index) {
        const WordCandidate &current(candidates.at(index));

        if (current.label().text() == candidate.label().text()
            && current.rect() == candidate.rect()) {
            candidates.replace(index, makeInactive(candidate, d->activeStyle()));
            layout->setWordRibbon(ribbon);

            Q_EMIT wordCandidatesChanged(layout);
            Q_EMIT wordCandidateSelected(candidate.label().text());

            break;
        }
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

    if (d->layout.isNull()) {
        return;
    }

    d->layout->clearActiveKeys();
    d->layout->clearMagnifierKey();

    const KeyAreaConverter converter(&d->style, &d->loader, d->anchor);
    const Layout::Orientation orientation(d->layout->orientation());
    d->layout->setCenterPanel(d->inShiftedState() ? converter.shiftedKeyArea(orientation)
                                                  : converter.keyArea(orientation));

    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::switchToPrimarySymView()
{
    Q_D(LayoutUpdater);

    if (d->layout.isNull()) {
        return;
    }

    const KeyAreaConverter converter(&d->style, &d->loader, d->anchor);
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

    if (d->layout.isNull()) {
        return;
    }

    const KeyAreaConverter converter(&d->style, &d->loader, d->anchor);
    const Layout::Orientation orientation(d->layout->orientation());
    d->layout->setCenterPanel(converter.symbolsKeyArea(orientation, 1));

    Q_EMIT layoutChanged(d->layout);
}

void LayoutUpdater::switchToAccentedView()
{
    Q_D(LayoutUpdater);

    if (d->layout.isNull()) {
        return;
    }

    const KeyAreaConverter converter(&d->style, &d->loader, d->anchor);
    const Layout::Orientation orientation(d->layout->orientation());
    const Key accent(d->deadkey_machine.accentKey());
    d->layout->setCenterPanel(d->inShiftedState() ? converter.shiftedDeadKeyArea(orientation, accent)
                                                  : converter.deadKeyArea(orientation, accent));

    Q_EMIT layoutChanged(d->layout);
}

} // namespace MaliitKeyboard
