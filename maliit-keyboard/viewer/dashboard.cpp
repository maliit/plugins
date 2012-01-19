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

#include "models/key.h"
#include "view/renderer.h"
#include "dashboard.h"

#include <QInputMethodEvent>

namespace MaliitKeyboard {

class DashboardPrivate
{
public:
    Renderer *renderer;
    QTextEdit *text_entry;
    QVBoxLayout *vbox;
    QSpacerItem *top;
    QSpacerItem *bottom;
    QWidget *buttons;

    explicit DashboardPrivate()
        : renderer(0)
        , text_entry(new QTextEdit)
        , vbox(new QVBoxLayout)
        , top(new QSpacerItem(0, 0))
        , bottom(new QSpacerItem(0, 0))
        , buttons(new QWidget)
    {}
};

Dashboard::Dashboard(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new DashboardPrivate)
{
    setWindowTitle("Maliit Keyboard Viewer");
    resize(854, 480);

    QWidget *w = new QWidget;
    setCentralWidget(w);
    w->show();

    QVBoxLayout *vbox = d_ptr->vbox;
    w->setLayout(vbox);

    QSpacerItem *top_spacer = d_ptr->top;
    vbox->addItem(top_spacer);

    QWidget *buttons = d_ptr->buttons;
    buttons->show();
    vbox->addWidget(buttons);
    QHBoxLayout *hbox = new QHBoxLayout;
    buttons->setLayout(hbox);

    QPushButton *show = new QPushButton("Show virtual keyboard");
    connect(show, SIGNAL(clicked()),
            this, SLOT(onShow()));
    hbox->addWidget(show);
    show->show();

    QPushButton *close = new QPushButton("Close application");
    connect(close, SIGNAL(clicked()),
            qApp,  SLOT(quit()));
    hbox->addWidget(close);
    close->show();

    QTextEdit *te = d_ptr->text_entry;
    vbox->addWidget(te);
    te->show();
    te->setFocus();

    QSpacerItem *bottom_spacer = d_ptr->bottom;
    vbox->addItem(bottom_spacer);

    onShow();
}

Dashboard::~Dashboard()
{}

void Dashboard::setRenderer(Renderer *renderer)
{
    Q_D(Dashboard);
    d->renderer = renderer;
}

void Dashboard::onKeyReleased(const Key &key)
{
    Q_D(Dashboard);

    QInputMethodEvent *im_ev = 0;
    QKeyEvent *key_ev = 0;

    switch(key.action()) {
    case Key::ActionInsert:
        im_ev = new QInputMethodEvent;
        im_ev->setCommitString(key.text());
        break;

    case Key::ActionBackspace:
        key_ev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
        break;

    case Key::ActionReturn:
        key_ev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
        break;

    case Key::ActionSpace:
        im_ev = new QInputMethodEvent;
        im_ev->setCommitString(" ");
        break;

    case Key::ActionLeft:
        key_ev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Left, Qt::NoModifier);
        break;

    case Key::ActionRight:
        key_ev = new QKeyEvent(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
        break;

    default:
        break;
    }

    if (im_ev) {
        qApp->postEvent(d->text_entry, im_ev);
    }

    if (key_ev) {
        qApp->postEvent(d->text_entry, key_ev);
    }
}

void Dashboard::onShow()
{
    Q_D(Dashboard);

    if (d->renderer) {
        d->renderer->show();
    }

    d->top->changeSize(0, 50);
    d->bottom->changeSize(0, 250);
    d->vbox->invalidate();
    d->buttons->hide();
}

void Dashboard::onHide()
{
    Q_D(Dashboard);
    d->top->changeSize(0, 0);
    d->bottom->changeSize(0, 0);
    d->vbox->invalidate();
    d->buttons->show();
}

} // namespace MaliitKeyboard
