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
#include "logic/layoutupdater.h"
#include "view/renderer.h"
#include "view/abstractbackgroundbuffer.h"
#include "view/glass.h"
#include "view/setup.h"
#include "models/keyarea.h"
#include "models/layout.h"
#include "models/wordribbon.h"

#include <maliit/plugins/subviewdescription.h>
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
    QWidget *window;
    BackgroundBuffer buffer;
    Renderer renderer;
    Glass glass;
    LayoutUpdater layout_updater;
    Editor editor;

    explicit InputMethodPrivate(MAbstractInputMethodHost *host)
        : buffer(host)
        , renderer()
        , glass()
        , layout_updater()
        , editor()
    {
        renderer.setSurfaceFactory(host->surfaceFactory());
        glass.setWindow(renderer.viewport());
        glass.addExtendedWindow(renderer.extendedViewport());
        editor.setHost(host);

        WordRibbon ribbon;
        Area area;
        area.setBackground(QByteArray("background.png"));
        area.setBackgroundBorders(QMargins(0, 0, 0, 0));
        area.setSize(QSize(854, 40));
        ribbon.setArea(area);

        SharedLayout layout(new Layout);
        layout->setWordRibbon(ribbon);
        renderer.addLayout(layout);
        glass.addLayout(layout);
        layout_updater.setLayout(layout);

        const QRect screen_area(QApplication::desktop() ? QApplication::desktop()->screenGeometry()
                                                        : QRect(0, 0, 480, 854));
        layout->setScreenSize(screen_area.size());
        layout->setAlignment(Layout::Bottom);
    }
};

InputMethod::InputMethod(MAbstractInputMethodHost *host)
    : MAbstractInputMethod(host)
    , d_ptr(new InputMethodPrivate(host))
{
    Q_D(InputMethod);

    Setup::connectGlassToLayoutUpdater(&d->glass, &d->layout_updater);
    Setup::connectGlassToRenderer(&d->glass, &d->renderer);
    Setup::connectLayoutUpdaterToRenderer(&d->layout_updater, &d->renderer);

    connect(&d->glass,  SIGNAL(keyPressed(Key,SharedLayout)),
            &d->editor, SLOT(onKeyPressed(Key)));

    connect(&d->glass,  SIGNAL(keyReleased(Key,SharedLayout)),
            &d->editor, SLOT(onKeyReleased(Key)));

    connect(&d->glass,  SIGNAL(keyEntered(Key,SharedLayout)),
            &d->editor, SLOT(onKeyEntered(Key)));

    connect(&d->glass,  SIGNAL(keyExited(Key,SharedLayout)),
            &d->editor, SLOT(onKeyExited(Key)));

    connect(&d->glass, SIGNAL(keyboardClosed()),
            inputMethodHost(), SLOT(notifyImInitiatedHiding()));

    connect(&d->renderer, SIGNAL(regionChanged(QRegion)),
            host,         SLOT(setInputMethodArea(QRegion)));

    connect(&d->renderer, SIGNAL(regionChanged(QRegion)),
            host,         SLOT(setScreenRegion(QRegion)));

    connect(&d->glass, SIGNAL(switchLeft(SharedLayout)),
            this,      SLOT(onSwitchLeft()));

    connect(&d->glass, SIGNAL(switchRight(SharedLayout)),
            this,      SLOT(onSwitchRight()));

    connect(&d->editor, SIGNAL(keyboardClosed()),
            this,       SLOT(hide()));
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
    d->layout_updater.clearActiveKeysAndMagnifier();
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

} // namespace MaliitKeyboard
