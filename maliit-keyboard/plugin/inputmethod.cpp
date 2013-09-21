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
#include "maliitcontext.h"

#include "models/key.h"
#include "models/keyarea.h"
#include "models/wordribbon.h"
#include "models/layout.h"

#include "logic/layouthelper.h"
#include "logic/layoutupdater.h"
#include "logic/wordengine.h"
#include "logic/style.h"
#include "logic/languagefeatures.h"
#include "logic/eventhandler.h"

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
#include <QtQuick>

class MImUpdateEvent;

namespace MaliitKeyboard {

typedef QScopedPointer<Maliit::Plugins::AbstractPluginSetting> ScopedSetting;
typedef QSharedPointer<MKeyOverride> SharedOverride;
typedef QMap<QString, SharedOverride>::const_iterator OverridesIterator;

namespace {

void makeQuickViewTransparent(QQuickView *view)
{
    QSurfaceFormat format;
    format.setAlphaBufferSize(8);
    view->setFormat(format);
    view->setColor(QColor(Qt::transparent));
}

QQuickView *getSurface (MAbstractInputMethodHost *host)
{
    QScopedPointer<QQuickView> view(new QQuickView (0));

    host->registerWindow (view.data(), Maliit::PositionCenterBottom);

    makeQuickViewTransparent(view.data());

    return view.take ();
}

QQuickView *getOverlaySurface (MAbstractInputMethodHost *host, QQuickView *parent)
{
    QScopedPointer<QQuickView> view(new QQuickView (0));

    view->setTransientParent(parent);

    host->registerWindow (view.data(), Maliit::PositionOverlay);

    makeQuickViewTransparent(view.data());

    return view.take ();
}

const QString g_maliit_keyboard_qml(MALIIT_KEYBOARD_DATA_DIR "/maliit-keyboard.qml");
const QString g_maliit_keyboard_extended_qml(MALIIT_KEYBOARD_DATA_DIR "/maliit-keyboard-extended.qml");
const QString g_maliit_magnifier_qml(MALIIT_KEYBOARD_DATA_DIR "/maliit-magnifier.qml");

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

class Settings
{
public:
    ScopedSetting style;
    ScopedSetting feedback;
    ScopedSetting auto_correct;
    ScopedSetting auto_caps;
    ScopedSetting word_engine;
    ScopedSetting hide_word_ribbon_in_portrait_mode;
};

class LayoutGroup
{
public:
    Logic::LayoutHelper helper;
    Logic::LayoutUpdater updater;
    Model::Layout model;
    Logic::EventHandler event_handler;

    explicit LayoutGroup();
};

LayoutGroup::LayoutGroup()
    : helper()
    , updater()
    , model()
    , event_handler(&model, &updater)
{}


class InputMethodPrivate
{
public:
    QScopedPointer<QQuickView> surface;
    QScopedPointer<QQuickView> extended_surface;
    QScopedPointer<QQuickView> magnifier_surface;
    Editor editor;
    DefaultFeedback feedback;
    SharedStyle style;
    UpdateNotifier notifier;
    QMap<QString, SharedOverride> key_overrides;
    Settings settings;
    LayoutGroup layout;
    LayoutGroup extended_layout;
    Model::Layout magnifier_layout;
    MaliitContext context;

    explicit InputMethodPrivate(InputMethod * const q,
                                MAbstractInputMethodHost *host);
    void setLayoutOrientation(Logic::LayoutHelper::Orientation orientation);
    void syncWordEngine(Logic::LayoutHelper::Orientation orientation);

    void connectToNotifier();
    void setContextProperties(QQmlContext *qml_context);
};


InputMethodPrivate::InputMethodPrivate(InputMethod *const q,
                                       MAbstractInputMethodHost *host)
    : surface(getSurface(host))
    , extended_surface(getOverlaySurface(host, surface.data()))
    , magnifier_surface(getOverlaySurface(host, surface.data()))
    , editor(Logic::EditorOptions(), new Model::Text, new Logic::WordEngine, new Logic::LanguageFeatures)
    , feedback()
    , style(new Style)
    , notifier()
    , key_overrides()
    , settings()
    , layout()
    , extended_layout()
    , magnifier_layout()
    , context(q, style)
{
    editor.setHost(host);

    layout.updater.setLayout(&layout.helper);
    extended_layout.updater.setLayout(&extended_layout.helper);

    layout.updater.setStyle(style);
    extended_layout.updater.setStyle(style);
    feedback.setStyle(style);

    const QSize &screen_size(QGuiApplication::primaryScreen()->availableSize());
    layout.helper.setScreenSize(screen_size);
    layout.helper.setAlignment(Logic::LayoutHelper::Bottom);
    extended_layout.helper.setScreenSize(screen_size);
    extended_layout.helper.setAlignment(Logic::LayoutHelper::Floating);

    QObject::connect(&layout.event_handler,          SIGNAL(extendedKeysShown(Key)),
                     &extended_layout.event_handler, SLOT(onExtendedKeysShown(Key)));

    connectToNotifier();

    // TODO: Figure out whether two views can share one engine.
    QQmlEngine *const engine(surface->engine());
    engine->addImportPath(MALIIT_KEYBOARD_DATA_DIR);
    setContextProperties(engine->rootContext());

    surface->setSource(QUrl::fromLocalFile(g_maliit_keyboard_qml));

    QQmlEngine *const extended_engine(extended_surface->engine());
    extended_engine->addImportPath(MALIIT_KEYBOARD_DATA_DIR);
    setContextProperties(extended_engine->rootContext());

    extended_surface->setSource(QUrl::fromLocalFile(g_maliit_keyboard_extended_qml));

    QQmlEngine *const magnifier_engine(magnifier_surface->engine());
    magnifier_engine->addImportPath(MALIIT_KEYBOARD_DATA_DIR);
    setContextProperties(magnifier_engine->rootContext());

    magnifier_surface->setSource(QUrl::fromLocalFile(g_maliit_magnifier_qml));
}


void InputMethodPrivate::setLayoutOrientation(Logic::LayoutHelper::Orientation orientation)
{
    syncWordEngine(orientation);
    layout.updater.setOrientation(orientation);
    extended_layout.updater.setOrientation(orientation);
}


void InputMethodPrivate::syncWordEngine(Logic::LayoutHelper::Orientation orientation)
{
    // hide_word_ribbon_in_potrait_mode_setting overrides word_engine_setting:
#ifndef DISABLE_PREEDIT
    const bool override_activation(settings.hide_word_ribbon_in_portrait_mode->value().toBool()
                                   && orientation == Logic::LayoutHelper::Portrait);
#else
    Q_UNUSED(orientation)
    const bool override_activation = true;
#endif

    editor.wordEngine()->setEnabled(override_activation
                                    ? false
                                    : settings.word_engine->value().toBool());
}

void InputMethodPrivate::connectToNotifier()
{
    QObject::connect(&notifier, SIGNAL(cursorPositionChanged(int, QString)),
                     &editor,   SLOT(onCursorPositionChanged(int, QString)));

    QObject::connect(&notifier,      SIGNAL(keysOverriden(Logic::KeyOverrides, bool)),
                     &layout.helper, SLOT(onKeysOverriden(Logic::KeyOverrides, bool)));
}

void InputMethodPrivate::setContextProperties(QQmlContext *qml_context)
{
    qml_context->setContextProperty("maliit", &context);
    qml_context->setContextProperty("maliit_layout", &layout.model);
    qml_context->setContextProperty("maliit_event_handler", &layout.event_handler);
    qml_context->setContextProperty("maliit_extended_layout", &extended_layout.model);
    qml_context->setContextProperty("maliit_extended_event_handler", &extended_layout.event_handler);
    qml_context->setContextProperty("maliit_magnifier_layout", &magnifier_layout);
}

InputMethod::InputMethod(MAbstractInputMethodHost *host)
    : MAbstractInputMethod(host)
    , d_ptr(new InputMethodPrivate(this, host))
{
    Q_D(InputMethod);

    // FIXME: Reconnect feedback instance.
    Setup::connectAll(&d->layout.event_handler, &d->layout.updater, &d->editor);
    Setup::connectAll(&d->extended_layout.event_handler, &d->extended_layout.updater, &d->editor);

    connect(&d->layout.helper, SIGNAL(centerPanelChanged(KeyArea,Logic::KeyOverrides)),
            &d->layout.model, SLOT(setKeyArea(KeyArea)));

    connect(&d->extended_layout.helper, SIGNAL(extendedPanelChanged(KeyArea,Logic::KeyOverrides)),
            &d->extended_layout.model, SLOT(setKeyArea(KeyArea)));

    connect(&d->layout.helper,    SIGNAL(magnifierChanged(KeyArea)),
            &d->magnifier_layout, SLOT(setKeyArea(KeyArea)));

    connect(&d->layout.model, SIGNAL(widthChanged(int)),
            this,             SLOT(onLayoutWidthChanged(int)));

    connect(&d->layout.model, SIGNAL(heightChanged(int)),
            this,             SLOT(onLayoutHeightChanged(int)));

    connect(&d->layout.updater, SIGNAL(keyboardTitleChanged(QString)),
            &d->layout.model,   SLOT(setTitle(QString)));

    connect(&d->extended_layout.model, SIGNAL(widthChanged(int)),
            this,                      SLOT(onExtendedLayoutWidthChanged(int)));

    connect(&d->extended_layout.model, SIGNAL(heightChanged(int)),
            this,                      SLOT(onExtendedLayoutHeightChanged(int)));

    connect(&d->extended_layout.model, SIGNAL(originChanged(QPoint)),
            this,                      SLOT(onExtendedLayoutOriginChanged(QPoint)));

    connect(&d->magnifier_layout, SIGNAL(widthChanged(int)),
            this,                 SLOT(onMagnifierLayoutWidthChanged(int)));

    connect(&d->magnifier_layout, SIGNAL(heightChanged(int)),
            this,                 SLOT(onMagnifierLayoutHeightChanged(int)));

    connect(&d->magnifier_layout, SIGNAL(originChanged(QPoint)),
            this,                 SLOT(onMagnifierLayoutOriginChanged(QPoint)));

    // FIXME: Reimplement keyboardClosed, switchLeft and switchRight
    // (triggered by glass).

    connect(&d->editor, SIGNAL(rightLayoutSelected()),
            this,       SLOT(onRightLayoutSelected()));

    connect(QGuiApplication::primaryScreen(), SIGNAL(geometryChanged(QRect)),
            this,               SLOT(onScreenSizeChange(QRect)));

    registerStyleSetting(host);
    registerFeedbackSetting(host);
    registerAutoCorrectSetting(host);
    registerAutoCapsSetting(host);
    registerWordEngineSetting(host);
    registerHideWordRibbonInPortraitModeSetting(host);

    // Setting layout orientation depends on word engine and hide word ribbon
    // settings to be initialized first:
    const QSize &screen_size(QGuiApplication::primaryScreen()->availableSize());
    d->setLayoutOrientation(screen_size.width() >= screen_size.height()
                            ? Logic::LayoutHelper::Landscape : Logic::LayoutHelper::Portrait);
}

InputMethod::~InputMethod()
{}

void InputMethod::show()
{
    Q_D(InputMethod);

    const QRect &rect = d->surface->screen()->availableGeometry();

    d->surface->setGeometry(QRect(QPoint(rect.x() + (rect.width() - d->layout.model.width()) / 2,
                                         rect.y() + rect.height() - d->layout.model.height()),
                                  QSize(d->layout.model.width(),
                                        d->layout.model.height())));

    d->surface->show();
    d->extended_surface->show();
    d->magnifier_surface->show();
}

void InputMethod::hide()
{
    Q_D(InputMethod);
    d->layout.updater.resetOnKeyboardClosed();
    d->editor.clearPreedit();
    d->surface->hide();
    d->extended_surface->hide();
    d->magnifier_surface->hide();
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

    Q_FOREACH (const QString &id, d->layout.updater.keyboardIds()) {
        MInputMethodSubView v;
        v.subViewId = id;
        v.subViewTitle = d->layout.updater.keyboardTitle(id);
        views.append(v);
    }

    return views;
}

void InputMethod::setActiveSubView(const QString &id,
                                   Maliit::HandlerState state)
{
    Q_UNUSED(state)
    Q_D(InputMethod);

    // FIXME: Perhaps better to let both LayoutUpdater share the same KeyboardLoader instance?
    d->layout.updater.setActiveKeyboardId(id);
    d->extended_layout.updater.setActiveKeyboardId(id);
}

QString InputMethod::activeSubView(Maliit::HandlerState state) const
{
    Q_UNUSED(state)
    Q_D(const InputMethod);

    return d->layout.updater.activeKeyboardId();
}

void InputMethod::handleAppOrientationChanged(int angle)
{
    Q_D(InputMethod);
    d->setLayoutOrientation((angle == 0 || angle == 180)
                            ? Logic::LayoutHelper::Landscape
                            : Logic::LayoutHelper::Portrait);
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

    d->settings.style.reset(host->registerPluginSetting("current_style",
                                                        QT_TR_NOOP("Keyboard style"),
                                                        Maliit::StringType,
                                                        attributes));

    connect(d->settings.style.data(), SIGNAL(valueChanged()),
            this,                     SLOT(onStyleSettingChanged()));

    // Call manually for the first time to initialize dependent values:
    onStyleSettingChanged();
}


void InputMethod::registerFeedbackSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->settings.feedback.reset(host->registerPluginSetting("feedback_enabled",
                                                           QT_TR_NOOP("Feedback enabled"),
                                                           Maliit::BoolType,
                                                           attributes));

    connect(d->settings.feedback.data(), SIGNAL(valueChanged()),
            this,                        SLOT(onFeedbackSettingChanged()));

    d->feedback.setEnabled(d->settings.feedback->value().toBool());
}


void InputMethod::registerAutoCorrectSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->settings.auto_correct.reset(host->registerPluginSetting("auto_correct_enabled",
                                                               QT_TR_NOOP("Auto-correct enabled"),
                                                               Maliit::BoolType,
                                                               attributes));

    connect(d->settings.auto_correct.data(), SIGNAL(valueChanged()),
            this,                            SLOT(onAutoCorrectSettingChanged()));

    d->editor.setAutoCorrectEnabled(d->settings.auto_correct->value().toBool());
}


void InputMethod::registerAutoCapsSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->settings.auto_caps.reset(host->registerPluginSetting("auto_caps_enabled",
                                                            QT_TR_NOOP("Auto-capitalization enabled"),
                                                            Maliit::BoolType,
                                                            attributes));

    connect(d->settings.auto_caps.data(), SIGNAL(valueChanged()),
            this,                         SLOT(onAutoCapsSettingChanged()));

    d->editor.setAutoCapsEnabled(d->settings.auto_caps->value().toBool());
}


void InputMethod::registerWordEngineSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = true;

    d->settings.word_engine.reset(host->registerPluginSetting("word_engine_enabled",
                                                              QT_TR_NOOP("Error correction/word prediction enabled"),
                                                              Maliit::BoolType,
                                                              attributes));

    connect(d->settings.word_engine.data(), SIGNAL(valueChanged()),
            this,                           SLOT(onWordEngineSettingChanged()));

#ifndef DISABLE_PREEDIT
    d->editor.wordEngine()->setEnabled(d->settings.word_engine->value().toBool());
#else
    d->editor.wordEngine()->setEnabled(false);
#endif
}

void InputMethod::registerHideWordRibbonInPortraitModeSetting(MAbstractInputMethodHost *host)
{
    Q_D(InputMethod);

    QVariantMap attributes;
    attributes[Maliit::SettingEntryAttributes::defaultValue] = false;

    d->settings.hide_word_ribbon_in_portrait_mode.reset(
        host->registerPluginSetting("hide_word_ribbon_in_potrait_mode",
                                    QT_TR_NOOP("Disable word engine in portrait mode"),
                                    Maliit::BoolType,
                                    attributes));

    connect(d->settings.hide_word_ribbon_in_portrait_mode.data(), SIGNAL(valueChanged()),
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

void InputMethod::onScreenSizeChange(const QRect &)
{
    Q_D(InputMethod);

    const QSize &size(QGuiApplication::primaryScreen()->availableSize());

    d->layout.helper.setScreenSize(size);
    d->extended_layout.helper.setScreenSize(d->layout.helper.screenSize());

    d->setLayoutOrientation(size.width() >= size.height()
                            ? Logic::LayoutHelper::Landscape : Logic::LayoutHelper::Portrait);
}

void InputMethod::onStyleSettingChanged()
{
    Q_D(InputMethod);
    d->style->setProfile(d->settings.style->value().toString());
    d->layout.model.setImageDirectory(d->style->directory(Style::Images));
    d->extended_layout.model.setImageDirectory(d->style->directory(Style::Images));
    d->magnifier_layout.setImageDirectory(d->style->directory(Style::Images));
}

void InputMethod::onKeyboardClosed()
{
    hide();
    inputMethodHost()->notifyImInitiatedHiding();
}

void InputMethod::onFeedbackSettingChanged()
{
    Q_D(InputMethod);
    d->feedback.setEnabled(d->settings.feedback->value().toBool());
}

void InputMethod::onAutoCorrectSettingChanged()
{
    Q_D(InputMethod);
    d->editor.setAutoCorrectEnabled(d->settings.auto_correct->value().toBool());
}

void InputMethod::onAutoCapsSettingChanged()
{
    Q_D(InputMethod);
    d->editor.setAutoCapsEnabled(d->settings.auto_caps->value().toBool());
}

void InputMethod::onWordEngineSettingChanged()
{
    // FIXME: Renderer doesn't seem to update graphics properly. Word ribbon
    // is still visible until next VKB show/hide.
    Q_D(InputMethod);
    d->syncWordEngine(d->layout.helper.orientation());
}

void InputMethod::onHideWordRibbonInPortraitModeSettingChanged()
{
    Q_D(InputMethod);
    d->setLayoutOrientation(d->layout.helper.orientation());
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

void InputMethod::onLayoutWidthChanged(int width)
{
    Q_D(InputMethod);
    d->surface->setWidth(width);
}

void InputMethod::onLayoutHeightChanged(int height)
{
    Q_D(InputMethod);
    d->surface->setHeight(height);
}

void InputMethod::onExtendedLayoutWidthChanged(int width)
{
    Q_D(InputMethod);
    d->extended_surface->setWidth(width);
}

void InputMethod::onExtendedLayoutHeightChanged(int height)
{
    Q_D(InputMethod);
    d->extended_surface->setHeight(height);
}

void InputMethod::onExtendedLayoutOriginChanged(const QPoint &origin)
{
    Q_D(InputMethod);
    d->extended_surface->setPosition(d->surface->position() + origin);
}

void InputMethod::onMagnifierLayoutWidthChanged(int width)
{
    Q_D(InputMethod);
    d->magnifier_surface->setWidth(width);
}

void InputMethod::onMagnifierLayoutHeightChanged(int height)
{
    Q_D(InputMethod);
    d->magnifier_surface->setHeight(height);
}

void InputMethod::onMagnifierLayoutOriginChanged(const QPoint &origin)
{
    Q_D(InputMethod);
    d->magnifier_surface->setPosition(d->surface->position() + origin);
}

} // namespace MaliitKeyboard
