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

#include "inputmethod.h"
#include "editor.h"

#include "models/keyarea.h"
#include "models/layout.h"
#include "models/wordribbon.h"

#include "logic/layoutupdater.h"
#include "logic/wordengine.h"
#include "logic/style.h"

#include "view/renderer.h"
#include "view/abstractbackgroundbuffer.h"
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
#include <maliit/plugins/abstractsurfacefactory.h>

#include <QApplication>
#include <QWidget>
#include <QDesktopWidget>

namespace MaliitKeyboard {

class BackgroundBuffer
    : public AbstractBackgroundBuffer
{
private:
    MAbstractInputMethodHost *m_host;

public:
    explicit BackgroundBuffer(MAbstractInputMethodHost *host)
        : AbstractBackgroundBuffer()
    {
        m_host = host;
    }

    virtual ~BackgroundBuffer()
    {}

    QPixmap background() const
    {
        if (not m_host) {
            static QPixmap empty;
            return empty;
        }

        return m_host->background();
    }
};

class InputMethodPrivate
{
public:
    Maliit::Plugins::AbstractSurfaceFactory *surfaceFactory;
    BackgroundBuffer buffer;
    Renderer renderer;
    Glass glass;
    LayoutUpdater layout_updater;
    Editor editor;
    Logic::WordEngine word_engine;
    DefaultFeedback feedback;
    SharedLayout layout;
    SharedStyle style;

    explicit InputMethodPrivate(MAbstractInputMethodHost *host)
        : surfaceFactory(host->surfaceFactory())
        , buffer(host)
        , renderer()
        , glass()
        , layout_updater()
        , editor(EditorOptions())
        , word_engine()
        , feedback()
        , layout(new Layout)
        , style(new Style)
    {
        // FIXME: Get style profile via settings API. Also need to show available profiles.
        style->setProfile("nokia-n9");

        renderer.setSurfaceFactory(surfaceFactory);
        glass.setSurface(renderer.surface());
        glass.setExtendedSurface(renderer.extendedSurface());
        editor.setHost(host);

        renderer.addLayout(layout);
        glass.addLayout(layout);
        layout_updater.setLayout(layout);

        const QSize &screen_size(surfaceFactory->screenSize());
        layout->setScreenSize(screen_size);
        layout->setAlignment(Layout::Bottom);
        layout_updater.setOrientation(screen_size.width() >= screen_size.height() ? Layout::Landscape : Layout::Portrait);
    }
};

InputMethod::InputMethod(MAbstractInputMethodHost *host)
    : MAbstractInputMethod(host)
    , d_ptr(new InputMethodPrivate(host))
{
    Q_D(InputMethod);

    Setup::connectAll(&d->glass, &d->layout_updater, &d->renderer, &d->editor, &d->word_engine, &d->feedback);

    connect(&d->glass, SIGNAL(keyboardClosed()),
            this,      SLOT(hide()));

    connect(&d->glass, SIGNAL(switchLeft(SharedLayout)),
            this,      SLOT(onSwitchLeft()));

    connect(&d->glass, SIGNAL(switchRight(SharedLayout)),
            this,      SLOT(onSwitchRight()));
    connect(d->surfaceFactory, SIGNAL(screenSizeChanged(QSize)),
            this,              SLOT(onScreenSizeChange(QSize)));
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
    inputMethodHost()->notifyImInitiatedHiding();
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
    d->layout_updater.setOrientation((angle == 0 || angle == 180) ? Layout::Landscape
                                                                  : Layout::Portrait);
}

void InputMethod::onSwitchLeft()
{
    // This API smells real bad.
    const QList<MImSubViewDescription> &list =
        inputMethodHost()->surroundingSubViewDescriptions(Maliit::OnScreen);

    if (list.count() > 0) {
        Q_EMIT activeSubViewChanged(list.at(0).id());
    }
}

void InputMethod::onSwitchRight()
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

    d->layout->setScreenSize(size);
    d->layout_updater.setOrientation(size.width() >= size.height() ? Layout::Landscape : Layout::Portrait);
}

} // namespace MaliitKeyboard
