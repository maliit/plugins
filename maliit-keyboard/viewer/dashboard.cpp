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
#include "models/layout.h"
#include "view/renderer.h"
#include "view/abstracttexteditor.h"
#include "logic/keyareaconverter.h"
#include "dashboard.h"

#include <QInputMethodEvent>

namespace MaliitKeyboard {

class DashboardEditor
    : public AbstractTextEditor
{
private:
    QTextEdit *const m_target;

public:
    explicit DashboardEditor(QTextEdit *target,
                             const EditorOptions &options,
                             QObject *parent = 0);

private:
    //! \reimp
    virtual void sendPreeditString(const QString &preedit);
    virtual void sendCommitString(const QString &commit);
    virtual void sendKeyEvent(const QKeyEvent &ev);
    //! \reimp_end
};

DashboardEditor::DashboardEditor(QTextEdit *target,
                                 const EditorOptions &options,
                                 QObject *parent)
    : AbstractTextEditor(options, Model::SharedText(new Model::Text), parent)
    , m_target(target)
{}

void DashboardEditor::sendPreeditString(const QString &preedit)
{
    QInputMethodEvent *ev = new QInputMethodEvent(preedit, QList<QInputMethodEvent::Attribute>());
    qApp->postEvent(m_target, ev);
}

void DashboardEditor::sendCommitString(const QString &commit)
{
    QInputMethodEvent *ev = new QInputMethodEvent;
    ev->setCommitString(commit);
    qApp->postEvent(m_target, ev);
}

void DashboardEditor::sendKeyEvent(const QKeyEvent &ev)
{
    QKeyEvent *new_ev = new QKeyEvent(ev.type(), ev.key(), ev.modifiers(), ev.text(), ev.isAutoRepeat(), ev.count());
    qApp->postEvent(m_target, new_ev);
}

class DashboardPrivate
{
public:
    Renderer *renderer;
    QTextEdit *text_entry;
    DashboardEditor *editor;
    QGraphicsProxyWidget *proxy_widget;
    QVBoxLayout *vbox;
    QSpacerItem *top;
    QSpacerItem *bottom;
    QWidget *buttons;
    Layout::Orientation orientation;

    explicit DashboardPrivate(Dashboard *q)
        : renderer(0)
        , text_entry(new QTextEdit)
        , editor(new DashboardEditor(text_entry, EditorOptions(), q))
        , proxy_widget(0)
        , vbox(new QVBoxLayout)
        , top(new QSpacerItem(0, 0))
        , bottom(new QSpacerItem(0, 0))
        , buttons(new QWidget)
        , orientation(Layout::Landscape)
    {}
};

Dashboard::Dashboard(QWidget *parent)
    : QMainWindow(parent)
    , d_ptr(new DashboardPrivate(this))
{
    setWindowTitle("Maliit Keyboard Viewer");
    resize(854, 480);

    QGraphicsView *v = new QGraphicsView;
    v->setScene(new QGraphicsScene(v));
    v->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    v->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setCentralWidget(v);
    v->show();
    QWidget *w = 0;
    d_ptr->proxy_widget = v->scene()->addWidget(w = new QWidget);
    d_ptr->proxy_widget->setTransformOriginPoint(d_ptr->proxy_widget->geometry().center());
    w->resize(size());

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

    QPushButton *orientation_changed = new QPushButton("Change orientation");
    connect(orientation_changed, SIGNAL(clicked()),
            this, SLOT(onOrientationChangeClicked()));
    hbox->addWidget(orientation_changed);
    orientation_changed->show();

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

AbstractTextEditor * Dashboard::editor() const
{
    Q_D(const Dashboard);
    return d->editor;
}

void Dashboard::onShow()
{
    Q_D(Dashboard);

    if (d->renderer) {
        d->renderer->show();
    }

    d->top->changeSize(0, d->orientation == Layout::Landscape ? 50 : 80);
    d->bottom->changeSize(0, d->orientation == Layout::Landscape ? 250 : 350);
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

void Dashboard::onOrientationChangeClicked()
{
    Q_D(Dashboard);

    d->orientation = (d->orientation == Layout::Landscape ? Layout::Portrait : Layout::Landscape);
    static_cast<QGraphicsView *>(centralWidget())->rotate(d->orientation == Layout::Landscape ? 90 : 270);
    const QSize &s(centralWidget()->size());
    d->proxy_widget->resize(d->orientation == Layout::Landscape ? s : QSize(s.height(), s.width()));
    onShow();
    Q_EMIT orientationChanged(d->orientation);
}

} // namespace MaliitKeyboard
