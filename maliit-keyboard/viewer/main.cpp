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

#include "dashboard.h"

#include "models/keyarea.h"
#include "models/key.h"
#include "models/keyfont.h"
#include "models/layout.h"

#include "logic/layoutupdater.h"

#include "view/renderer.h"
#include "view/glass.h"
#include "view/setup.h"

#include <QApplication>
#include <QWidget>

namespace {
MaliitKeyboard::Key createKey(const QByteArray &id,
                              const MaliitKeyboard::KeyFont &f,
                              const QRect &kr,
                              const QByteArray &t,
                              const QMargins &m,
                              MaliitKeyboard::Key::Action a = MaliitKeyboard::Key::ActionInsert)
{
    MaliitKeyboard::Key k;
    k.setText(t);
    k.setRect(kr);
    k.setBackground(id);
    k.setFont(f);
    k.setMargins(m);
    k.setAction(a);
    k.setBackgroundBorders(QMargins(6, 6, 6, 6));

    return k;
}

MaliitKeyboard::KeyArea createToolbar(int key_width,
                                      int key_height)
{
    typedef QByteArray QBA;
    const QBA bg("key-background.png");

    MaliitKeyboard::KeyFont font;
    font.setSize(24);
    font.setColor("#fffbed");

    MaliitKeyboard::KeyArea ka;
    ka.background = QByteArray("background.png");
    QMargins margins(4, 4, 4, 4);

    ka.keys.append(createKey(bg, font, QRect(0, 0, key_width, key_height),
                             QBA("<"), margins, MaliitKeyboard::Key::ActionLeft));
    ka.keys.append(createKey(bg, font, QRect(key_width + 8, 0, key_width, key_height),
                             QBA(">"), margins, MaliitKeyboard::Key::ActionRight));

    return ka;
}

}

int main(int argc,
         char ** argv)
{
    QApplication app(argc, argv);

    MaliitKeyboard::Dashboard *dashboard = new MaliitKeyboard::Dashboard;
    dashboard->show();

    MaliitKeyboard::Renderer renderer;
    renderer.setWindow(dashboard);
    dashboard->setRenderer(&renderer);

    MaliitKeyboard::Glass glass;
    glass.setWindow(renderer.viewport());

    // One layout updater can only manage one layout. If more layouts need to
    // be managed, then more layout updaters are required.
    MaliitKeyboard::LayoutUpdater updater;

    MaliitKeyboard::SharedLayout l0(new MaliitKeyboard::Layout);
    renderer.addLayout(l0);
    glass.addLayout(l0);
    updater.setLayout(l0);

    MaliitKeyboard::SharedLayout l1(new MaliitKeyboard::Layout);
    renderer.addLayout(l1);
    glass.addLayout(l1);

    MaliitKeyboard::KeyArea ka(createToolbar(80, 48));
    ka.rect = QRect(0, 0, dashboard->width(), 48);
    l1->setCenterPanel(ka);

    MaliitKeyboard::LayoutUpdater lu1;
    lu1.setLayout(l1);
    lu1.setScreenSize(dashboard->size());
    lu1.resetKeyboardLoader();

    MaliitKeyboard::Setup::connectGlassToLayoutUpdater(&glass, &lu1);
    MaliitKeyboard::Setup::connectLayoutUpdaterToRenderer(&lu1, &renderer);

    updater.setScreenSize(dashboard->size());

    MaliitKeyboard::Setup::connectGlassToLayoutUpdater(&glass, &updater);
    MaliitKeyboard::Setup::connectGlassToRenderer(&glass, &renderer);
    MaliitKeyboard::Setup::connectLayoutUpdaterToRenderer(&updater, &renderer);

    QObject::connect(&glass,    SIGNAL(keyReleased(Key,SharedLayout)),
                     dashboard, SLOT(onKeyReleased(Key)));

    QObject::connect(&glass,    SIGNAL(keyboardClosed()),
                     dashboard, SLOT(onHide()));

    // Allow to specify keyboard id via command line:
    QString keyboard_id("en_gb");
    bool found_keyboard_id = false;

    Q_FOREACH (const QString &arg, QApplication::arguments()) {
        if (found_keyboard_id && not arg.isEmpty()) {
            keyboard_id = arg;
        }

        if (arg == "--id" || arg == "-id") {
            found_keyboard_id = true;
        }
    }

    updater.setActiveKeyboardId(keyboard_id);
    renderer.show();

    return app.exec();
}
