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

#include "view/renderer.h"
#include "view/glass.h"
#include "logic/layoutupdater.h"

#include "models/keyarea.h"
#include "models/key.h"
#include "models/keylabel.h"
#include "models/layout.h"

#include <QApplication>
#include <QWidget>

namespace {
MaliitKeyboard::Key createKey(const QByteArray &id,
                              const MaliitKeyboard::SharedFont &f,
                              const QRect &kr,
                              const QByteArray &t,
                              const QColor &c,
                              MaliitKeyboard::Key::Action a = MaliitKeyboard::Key::ActionInsert)
{
    MaliitKeyboard::KeyLabel l;
    l.setText(t);
    l.setColor(c);
    l.setFont(f);

    MaliitKeyboard::Key k;
    k.setRect(kr);
    k.setBackground(id);
    k.setLabel(l);
    k.setAction(a);

    return k;
}

MaliitKeyboard::KeyArea createPrimaryKeyArea()
{
    typedef QByteArray QBA;
    const QBA bg("key-background.png");

    MaliitKeyboard::SharedFont font(new QFont);
    font->setBold(true);
    font->setPointSize(16);

    MaliitKeyboard::KeyArea ka;
    ka.rect = QRectF(0, 554, 480, 300);
    ka.keys.append(createKey(bg, font, QRect(10, 10, 40, 60), QBA("Q"), Qt::darkBlue));
    ka.keys.append(createKey(bg, font, QRect(60, 10, 80, 120), QBA("W"), Qt::darkMagenta));
    ka.keys.append(createKey(bg, font, QRect(10, 80, 40, 50), QBA("A"), Qt::black));
    ka.keys.append(createKey(bg, font, QRect(10, 140, 130, 60), QBA("shift"), Qt::darkCyan,
                             MaliitKeyboard::Key::ActionShift));
    ka.keys.append(createKey(bg, font, QRect(160, 10, 120, 120), QBA("switch"), Qt::darkCyan,
                             MaliitKeyboard::Key::ActionSwitch));

    return ka;
}

MaliitKeyboard::KeyArea createSecondaryKeyArea()
{
    typedef QByteArray QBA;
    const QBA bg("key-background.png");

    MaliitKeyboard::SharedFont font(new QFont);
    font->setBold(true);
    font->setPointSize(16);

    MaliitKeyboard::KeyArea ka;
    ka.rect = QRectF(0, 0, 480, 100);
    ka.keys.append(createKey(bg, font, QRect(10, 10, 40, 60), QBA("T"), Qt::darkBlue));
    ka.keys.append(createKey(bg, font, QRect(60, 10, 80, 80), QBA("O"), Qt::darkMagenta));

    return ka;
}

}

int main(int argc,
         char ** argv)
{
    QApplication app(argc, argv);

    QWidget *window = new QWidget;
    window->resize(480, 854);
    window->showFullScreen();

    MaliitKeyboard::SharedLayout l0(new MaliitKeyboard::Layout);
    l0->setCenterPanel(createPrimaryKeyArea());

    MaliitKeyboard::SharedLayout l1(new MaliitKeyboard::Layout);
    l1->setCenterPanel(createSecondaryKeyArea());

    MaliitKeyboard::Renderer renderer;
    renderer.setWindow(window);
    renderer.addLayout(l0);
    renderer.addLayout(l1);
    renderer.show();

    MaliitKeyboard::Glass glass;
    glass.setWindow(renderer.viewport());
    glass.addLayout(l0);
    glass.addLayout(l1);

    // One layout updater can only manage one layout. If more layouts need to
    // be managed, then more layout updaters are required.
    MaliitKeyboard::LayoutUpdater updater;
    updater.init();
    updater.setLayout(l0);

    QObject::connect(&glass,   SIGNAL(keyPressed(Key, SharedLayout)),
                     &updater, SLOT(onKeyPressed(Key, SharedLayout)));

    QObject::connect(&glass,   SIGNAL(keyReleased(Key, SharedLayout)),
                     &updater, SLOT(onKeyReleased(Key, SharedLayout)));

    QObject::connect(&updater,  SIGNAL(layoutChanged(SharedLayout)),
                     &renderer, SLOT(onLayoutChanged(SharedLayout)));

    QObject::connect(&updater,  SIGNAL(keysChanged(SharedLayout)),
                     &renderer, SLOT(onKeysChanged(SharedLayout)));

    return app.exec();
}
