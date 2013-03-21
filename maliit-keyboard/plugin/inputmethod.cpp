/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 * Copyright (C) 2012-2013 Canonical Ltd
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

#include "inputmethod.h"
#include "editor.h"
#include "updatenotifier.h"

#include "models/key.h"
#include "models/keyarea.h"
#include "models/wordribbon.h"

#include "logic/layout.h"
#include "logic/layoutupdater.h"
#include "logic/wordengine.h"
#include "logic/style.h"
#include "logic/languagefeatures.h"

#include "view/renderer.h"
#include "view/glass.h"
#include "view/setup.h"

#ifdef HAVE_QT_MOBILITY
#include "view/soundfeedback.h"
typedef MaliitKeyboard::SoundFeedback DefaultFeedback;
#else
#include "view/nullfeedback.h"
typedef MaliitKeyboard::NullFeedback DefaultFeedback;
#endif

#include <maliit/plugins/subviewdescription.h>
#include <maliit/plugins/abstractpluginsetting.h>
#include <maliit/plugins/updateevent.h>

#include <QApplication>
#include <QWidget>
#include <QDesktopWidget>

class MImUpdateEvent;

namespace MaliitKeyboard {

typedef QScopedPointer<Maliit::Plugins::AbstractPluginSetting> ScopedSetting;
typedef QSharedPointer<MKeyOverride> SharedOverride;
typedef QMap<QString, SharedOverride>::const_iterator OverridesIterator;

namespace {

Key overrideToKey(const SharedOverride &override)
{
    Key key;

    key.rLabel().setText(override->label());
    key.setIcon(override->icon().toUtf8());
    // TODO: hightlighted and enabled information are not available in
    // Key. Should we just really create a KeyOverride model?

    return key;
}

} // unnamed namespace

class InputMethodPrivate
{
public:
    Renderer renderer;
    Glass glass;
    Logic::LayoutUpdater layout_updater;
    Editor editor;
    DefaultFeedback feedback;
    Logic::Layout layout;
    SharedStyle style;
    UpdateNotifier notifier;
    QMap<QString, SharedOverride> key_overrides;
    ScopedSetting style_setting;
    ScopedSetting feedback_setting;
    ScopedSetting auto_correct_setting;
    ScopedSetting auto_caps_setting;
    ScopedSetting word_engine_setting;
    ScopedSetting hide_word_ribbon_in_portrait_mode_setting;

    explicit InputMethodPrivate(MAbstractInputMethodHost *host);
    void setLayoutOrientation(Logic::Layout::Orientation orientation);
    void syncWordEngine(Logic::Layout::Orientation orientation);

    void connectToNotifier();
};


InputMethodPrivate::InputMethodPrivate(MAbstractInputMethodHost *host)
    : renderer()
    , glass()
    , layout_updater()
    , editor(EditorOptions(), new Model::Text, new Logic::WordEngine, new Logic::LanguageFeatures)
    , feedback()
    , layout()
    , style(new Style)
    , notifier()
    , key_overrides()
    , style_setting()
    , feedback_setting()
    , auto_correct_setting()
    , auto_caps_setting()
    , word_engine_setting()
    , hide_word_ribbon_in_portrait_mode_setting()
{
    renderer.setHost(host);
    glass.setSurface(renderer.surface());
    glass.setExtendedSurface(renderer.extendedSurface());
    editor.setHost(host);

    glass.addLayout(&layout);
    layout_updater.setLayout(&layout);

    renderer.setStyle(style);
    layout_updater.setStyle(style);
    feedback.setStyle(style);

    const QSize &screen_size(QApplication::desktop()->screenGeometry().size());
    layout.setScreenSize(screen_size);
    layout.setAlignment(Logic::Layout::Bottom);

    connectToNotifier();
}


void InputMethodPrivate::setLayoutOrientation(Logic::Layout::Orientation orientation)
{
    syncWordEngine(orientation);
    layout_updater.setOrientation(orientation);
}


void InputMethodPrivate::syncWordEngine(Logic::Layout::Orientation orientation)
{
    // hide_word_ribbon_in_potrait_mode_setting overrides word_engine_setting:
    const bool override_activation(hide_word_ribbon_in_portrait_mode_setting->value().toBool()
                                   && orientation == Logic::Layout::Portrait);

    editor.wordEngine()->setEnabled(override_activation
                                    ? false
                                    : word_engine_setting->value().toBool());
}

void InputMethodPrivate::connectToNotifier()
{
    QObject::connect(&notifier, SIGNAL(cursorPositionChanged(int, QString)),
                     &editor,   SLOT(onCursorPositionChanged(int, QString)));

    QObject::connect(&notifier, SIGNAL(keysOverriden(Logic::KeyOverrides, bool)),
                     &layout,   SLOT(onKeysOverriden(Logic::KeyOverrides, bool)));
}

InputMethod::InputMethod(MAbstractInputMethodHost *host)
    : MAbstractInputMethod(host)
    , d_ptr(new InputMethodPrivate(host))
{
    Q_D(InputMethod);

    Setup::connectAll(&d->glass, &d->layout, &d->layout_updater,
                      &d->renderer, &d->editor, &d->feedback);

    connect(&d->glass, SIGNAL(keyboardClosed()),
            this,      SLOT(onKeyboardClosed()));

    connect(&d->glass, SIGNAL(switchLeft(Logic::Layout *)),
            this,      SLOT(onLeftLayoutSelected()));

    connect(&d->glass, SIGNAL(switchRight(Logic::Layout *)),
            this,      SLOT(onRightLayoutSelected()));

    connect(&d->editor, SIGNAL(leftLayoutSelected()),
            this,       SLOT(onLeftLayoutSelected()));

    connect(&d->editor, SIGNAL(rightLayoutSelected()),
            this,       SLOT(onRightLayoutSelected()));

    // TODO: handle screen size changes

    registerStyleSetting(host);
    registerFeedbackSetting(host);
    registerAutoCorrectSetting(host);
    registerAutoCapsSetting(host);
    registerWordEngineSetting(host);
    registerHideWordRibbonInPortraitModeSetting(host);

    // Setting layout orientation depends on word engine and hide word ribbon
    // settings to be initialized first:
    const QSize &screen_size(QApplication::desktop()->screenGeometry().size());
    d->setLayoutOrientation(screen_size.width() >= screen_size.height()
                         ? Logic::Layout::Landscape : Logic::Layout::Portrait);
}

InputMethod::~InputMethod()
{}

void InputMethod::show()
{
    Q_D(InputMethod);
    d->renderer.show();
}

void InputMethod::hide()
{
    Q_D(InputMethod);
    d->renderer.hide();
    d->layout_updater.resetOnKeyboardClosed();
    d->editor.clearPreedit();
}

void InputMethod::setPreedit(const QString &preedit,
                             int cursor_position)
{
    Q_UNUSED(cursor_position)
    Q_D(InputMethod);
    d->editor.replacePreedit(preedit);
}

void InputMethod::switchContext(Maliit::SwitchDirection direction,
                                bool animated)
{
    Q_UNUSED(direction)
    Q_UNUSED(animated)
}

QList<MAbstractInputMethod::MInputMethodSubView>
InputMethod::subViews(Maliit::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);

    QList<MInputMethodSubView> views;

    Q_FOREACH (const QString &id, d->layout_updater.keyboardIds()) {
        MInputMethodSubView v;
        v.subViewId = id;
        v.subViewTitle = d->layout_updater.keyboardTitle(id);
        views.append(v);
    }

    return views;
}

void InputMethod::setActiveSubView(const QString &id,
                                   Maliit::HandlerState state)
{
    Q_UNUSED(state)
    Q_D(InputMethod);

    d->layout_updater.setActiveKeyboardId(id);
}

QString InputMethod::activeSubView(Maliit::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);

    return d->layout_updater.activeKeyboardId();
}

void InputMethod::handleAppOrientationChanged(int angle)
{
    Q_D(InputMethod);
    d->setLayoutOrientation((angle == 0 || angle == 180)
                            ? Logic::Layout::Landscape
                            : Logic::Layout::Portrait);
}

bool InputMethod::imExtensionEvent(MImExtensionEvent *event)
{
    Q_D(InputMethod);

    if (not event or event->type() != MImExtensionEvent::Update) {
        return false;
    }

    MImUpdateEvent *update_event(static_cast<MImUpdateEvent *>(event));

    d->notifier.notify(update_event);

    return true;
}


void InputMethod::registerStyleSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    QStringList available_styles = d->style->availableProfiles();
    attributes[Maliit::SettingEntryAttributes::defaultValue] = MALIIT_DEFAULT_PROFILE;
    attributes[Maliit::SettingEntryAttributes::valueDomain] = available_styles;
    attributes[Maliit::SettingEntryAttributes::valueDomainDescriptions] = available_styles;

    d->style_setting.reset(host->registerPluginSetting("current_style",
                                                       QT_TR_NOOP("Keyboard style"),
                                                       Maliit::StringType,
                                                       attributes));

    connect(d->style_setting.data(), SIGNAL(valueChanged()),
            this,                    SLOT(onStyleSettingChanged()));

    d->style->setProfile(d->style_setting->value().toString());
}


void InputMethod::registerFeedbackSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->feedback_setting.reset(host->registerPluginSetting("feedback_enabled",
                                                          QT_TR_NOOP("Feedback enabled"),
                                                          Maliit::BoolType,
                                                          attributes));

    connect(d->feedback_setting.data(), SIGNAL(valueChanged()),
            this,                       SLOT(onFeedbackSettingChanged()));

    d->feedback.setEnabled(d->feedback_setting->value().toBool());
}


void InputMethod::registerAutoCorrectSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->auto_correct_setting.reset(host->registerPluginSetting("auto_correct_enabled",
                                                              QT_TR_NOOP("Auto-correct enabled"),
                                                              Maliit::BoolType,
                                                              attributes));

    connect(d->auto_correct_setting.data(), SIGNAL(valueChanged()),
            this,                           SLOT(onAutoCorrectSettingChanged()));

    d->editor.setAutoCorrectEnabled(d->auto_correct_setting->value().toBool());
}


void InputMethod::registerAutoCapsSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->auto_caps_setting.reset(host->registerPluginSetting("auto_caps_enabled",
                                                           QT_TR_NOOP("Auto-capitalization enabled"),
                                                           Maliit::BoolType,
                                                           attributes));

    connect(d->auto_caps_setting.data(), SIGNAL(valueChanged()),
            this,                        SLOT(onAutoCapsSettingChanged()));

    d->editor.setAutoCapsEnabled(d->auto_caps_setting->value().toBool());
}


void InputMethod::registerWordEngineSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->word_engine_setting.reset(host->registerPluginSetting("word_engine_enabled",
                                                             QT_TR_NOOP("Error correction/word prediction enabled"),
                                                             Maliit::BoolType,
                                                             attributes));

    connect(d->word_engine_setting.data(), SIGNAL(valueChanged()),
            this,                          SLOT(onWordEngineSettingChanged()));

    d->editor.wordEngine()->setEnabled(d->word_engine_setting->value().toBool());
}

void InputMethod::registerHideWordRibbonInPortraitModeSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = false;

    d->hide_word_ribbon_in_portrait_mode_setting.reset(
        host->registerPluginSetting("hide_word_ribbon_in_potrait_mode",
                                    QT_TR_NOOP("Disable word engine in portrait mode"),
                                    Maliit::BoolType,
                                    attributes));

    connect(d->hide_word_ribbon_in_portrait_mode_setting.data(), SIGNAL(valueChanged()),
            this, SLOT(onHideWordRibbonInPortraitModeSettingChanged()));
}


void InputMethod::onLeftLayoutSelected()
{
    // This API smells real bad.
    const QList<MImSubViewDescription> &list =
        inputMethodHost()->surroundingSubViewDescriptions(Maliit::OnScreen);

    if (list.count() > 0) {
        Q_EMIT activeSubViewChanged(list.at(0).id());
    }
}

void InputMethod::onRightLayoutSelected()
{
    // This API smells real bad.
    const QList<MImSubViewDescription> &list =
        inputMethodHost()->surroundingSubViewDescriptions(Maliit::OnScreen);

    if (list.count() > 1) {
        Q_EMIT activeSubViewChanged(list.at(1).id());
    }
}

void InputMethod::onScreenSizeChange(const QSize &size)
{
    Q_D(InputMethod);

    d->layout.setScreenSize(size);
    d->setLayoutOrientation(size.width() >= size.height()
                            ? Logic::Layout::Landscape
                            : Logic::Layout::Portrait);
}

void InputMethod::onStyleSettingChanged()
{
    Q_D(InputMethod);
    d->style->setProfile(d->style_setting->value().toString());
}

void InputMethod::onKeyboardClosed()
{
    hide();
    inputMethodHost()->notifyImInitiatedHiding();
}

void InputMethod::onFeedbackSettingChanged()
{
    Q_D(InputMethod);
    d->feedback.setEnabled(d->feedback_setting->value().toBool());
}

void InputMethod::onAutoCorrectSettingChanged()
{
    Q_D(InputMethod);
    d->editor.setAutoCorrectEnabled(d->auto_correct_setting->value().toBool());
}

void InputMethod::onAutoCapsSettingChanged()
{
    Q_D(InputMethod);
    d->editor.setAutoCapsEnabled(d->auto_caps_setting->value().toBool());
}

void InputMethod::onWordEngineSettingChanged()
{
    // FIXME: Renderer doesn't seem to update graphics properly. Word ribbon
    // is still visible until next VKB show/hide.
    Q_D(InputMethod);
    d->syncWordEngine(d->layout.orientation());
}

void InputMethod::onHideWordRibbonInPortraitModeSettingChanged()
{
    Q_D(InputMethod);
    d->setLayoutOrientation(d->layout.orientation());
}

void InputMethod::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    Q_D(InputMethod);

    for (OverridesIterator i(d->key_overrides.begin()), e(d->key_overrides.end()); i != e; ++i) {
        const SharedOverride &override(i.value());

        if (override) {
            disconnect(override.data(), SIGNAL(keyAttributesChanged(const QString &, const MKeyOverride::KeyOverrideAttributes)),
                       this,            SLOT(updateKey(const QString &, const MKeyOverride::KeyOverrideAttributes)));
        }
    }

    d->key_overrides.clear();
    QMap<QString, Key> overriden_keys;

    for (OverridesIterator i(overrides.begin()), e(overrides.end()); i != e; ++i) {
        const SharedOverride &override(i.value());

        if (override) {
            d->key_overrides.insert(i.key(), override);
            connect(override.data(), SIGNAL(keyAttributesChanged(const QString &, const MKeyOverride::KeyOverrideAttributes)),
                    this,            SLOT(updateKey(const QString &, const MKeyOverride::KeyOverrideAttributes)));
            overriden_keys.insert(i.key(), overrideToKey(override));
        }
    }
    d->notifier.notifyOverride(overriden_keys);
}

void InputMethod::updateKey(const QString &key_id,
                            const MKeyOverride::KeyOverrideAttributes changed_attributes)
{
    Q_D(InputMethod);

    Q_UNUSED(changed_attributes);

    QMap<QString, SharedOverride>::iterator iter(d->key_overrides.find(key_id));

    if (iter != d->key_overrides.end()) {
        const Key &override_key(overrideToKey(iter.value()));
        Logic::KeyOverrides overrides_update;

        overrides_update.insert(key_id, override_key);
        d->notifier.notifyOverride(overrides_update, true);
    }
}

} // namespace MaliitKeyboard
