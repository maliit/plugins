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

#include "glass.h"
#include "logic/hitlogic.h"
#include "models/keyarea.h"
#include "models/wordribbon.h"

#include <QWidget>

namespace MaliitKeyboard {

namespace {
void removeActiveKey(QVector<Key> *active_keys,
                     const Key &key)
{
    if (not active_keys) {
        return;
    }

    for (int index = 0; index < active_keys->count(); ++index) {
        if (active_keys->at(index) == key) {
            active_keys->remove(index);
            break;
        }
    }
}

}

class GlassPrivate
{
public:
    QWidget *window;
    QVector<SharedLayout> layouts;
    QVector<Key> active_keys;
    WordCandidate active_candidate;
    QPoint last_pos;
    QPoint press_pos;
    QElapsedTimer gesture_timer;
    bool gesture_triggered;
    QTimer long_press_timer;
    SharedLayout long_press_layout;

    explicit GlassPrivate()
        : window(0)
        , layouts()
        , active_keys()
        , active_candidate()
        , last_pos()
        , press_pos()
        , gesture_timer()
        , gesture_triggered(false)
        , long_press_timer()
        , long_press_layout()
    {
        long_press_timer.setInterval(300);
        long_press_timer.setSingleShot(true);
    }
};

Glass::Glass(QObject *parent)
    : QObject(parent)
    , d_ptr(new GlassPrivate)
{
    connect(&d_ptr->long_press_timer, SIGNAL(timeout()),
            this,                     SLOT(onLongPressTriggered()),
            Qt::UniqueConnection);
}

Glass::~Glass()
{}

void Glass::setWindow(QWidget *window)
{
    Q_D(Glass);

    d->window = window;
    clearLayouts();

    d->window->installEventFilter(this);
}

void Glass::addLayout(const SharedLayout &layout)
{
    Q_D(Glass);
    d->layouts.append(layout);
}

void Glass::clearLayouts()
{
    Q_D(Glass);
    d->layouts.clear();
}

bool Glass::eventFilter(QObject *obj,
                        QEvent *ev)
{
    Q_D(Glass);
    static bool measure_fps(QCoreApplication::arguments().contains("-measure-fps"));

    if (not obj || not ev) {
        return false;
    }

    switch(ev->type()) {
    case QEvent::Paint: {
        if (measure_fps) {
            static int count = 0;
            static QElapsedTimer fps_timer;

            if (0 == count % 120) {
                qDebug() << "FPS:" << count / ((0.01 + fps_timer.elapsed()) / 1000) << count;
                fps_timer.restart();
                count = 0;
            }

            d->window->update();
            ++count;
        }
    } break;

    case QKeyEvent::MouseButtonPress:
        d->gesture_timer.restart();
        d->gesture_triggered = false;

        return handlePressReleaseEvent(ev);

    case QKeyEvent::MouseButtonRelease:
        d->long_press_timer.stop();

        if (d->gesture_triggered) {
            return false;
        }

        return handlePressReleaseEvent(ev);

    case QKeyEvent::MouseMove: {
        if (d->gesture_triggered) {
            return false;
        }

        QMouseEvent *qme = static_cast<QMouseEvent *>(ev);
        ev->accept();

        Q_FOREACH (const SharedLayout &layout, d->layouts) {
            const QPoint &pos(layout->orientation() == Layout::Landscape
                              ? qme->pos() : QPoint(d->window->height() - qme->pos().y(), qme->pos().x()));
            const QPoint &last_pos(layout->orientation() == Layout::Landscape
                                   ? d->last_pos : QPoint(d->window->height() - d->last_pos.y(), d->last_pos.x()));
            d->last_pos = qme->pos();

            const QPoint &press_pos(layout->orientation() == Layout::Landscape
                                    ? d->press_pos : QPoint(d->window->height() - d->press_pos.y(), d->press_pos.x()));

            const QRect &rect(layout->activeKeyAreaGeometry());

            if (d->gesture_timer.elapsed() < 250) {
                if (pos.y() > (press_pos.y() - rect.height() * 0.33)
                    && pos.y() < (press_pos.y() + rect.height() * 0.33)) {
                    if (pos.x() < (press_pos.x() - rect.width() * 0.33)) {
                        d->gesture_triggered = true;
                        Q_EMIT switchRight(layout);
                    } else if (pos.x() > (press_pos.x() + rect.width() * 0.33)) {
                        d->gesture_triggered = true;
                        Q_EMIT switchLeft(layout);
                    }
                } else if (pos.x() > (press_pos.x() - rect.width() * 0.33)
                           && pos.x() < (press_pos.x() + rect.width() * 0.33)) {
                    if (pos.y() > (press_pos.y() + rect.height() * 0.50)) {
                        d->gesture_triggered = true;
                        Q_EMIT keyboardClosed();
                    }
                }
            }

            if (d->gesture_triggered) {
                Q_FOREACH (const Key &k, d->active_keys) {
                    Q_EMIT keyExited(k, layout);
                }

                d->active_keys.clear();

                return true;
            }

            const Key &last_key(Logic::keyHit(d->active_keys, rect, last_pos));

            if (last_key.valid()) {
                removeActiveKey(&d->active_keys, last_key);
                d->long_press_timer.stop();
                Q_EMIT keyExited(last_key, layout);
            }

            const Key &key(Logic::keyHit(layout->activeKeyArea().keys(),
                                         layout->activeKeyAreaGeometry(),
                                         pos, d->active_keys));

            if (key.valid()) {
                d->active_keys.append(key);

                if (key.hasExtendedKeys()) {
                    d->long_press_timer.start();
                    d->long_press_layout = layout;
                }

                Q_EMIT keyEntered(key, layout);
                return true;
            }
        }
    } break;

    default:
        break;
    }

    return false;
}

void Glass::onLongPressTriggered()
{
    Q_D(Glass);

    if (d->gesture_triggered || d->active_keys.isEmpty()
        || d->long_press_layout.isNull()
        || d->long_press_layout->activePanel() == Layout::ExtendedPanel) {
        return;
    }

    Q_FOREACH (const Key &k, d->active_keys) {
        Q_EMIT keyExited(k, d->long_press_layout); // Not necessarily correct layout for the key ...
    }

    Q_EMIT keyLongPressed(d->active_keys.last(), d->long_press_layout);
    d->active_keys.clear();
}

bool Glass::handlePressReleaseEvent(QEvent *ev)
{
    if (not ev) {
        return false;
    }

    Q_D(Glass);

    bool consumed = false;
    QMouseEvent *qme = static_cast<QMouseEvent *>(ev);
    d->last_pos = qme->pos();
    d->press_pos = qme->pos(); // FIXME: dont update on mouse release, clear instead.
    ev->accept();

    Q_FOREACH (const SharedLayout &layout, d->layouts) {
        const QPoint &pos(layout->orientation() == Layout::Landscape
                          ? qme->pos() : QPoint(d->window->height() - qme->pos().y(), qme->pos().x()));

        switch (qme->type()) {
        case QKeyEvent::MouseButtonPress: {
            const Key &key(Logic::keyHit(layout->activeKeyArea().keys(),
                                         layout->activeKeyAreaGeometry(),
                                         pos, d->active_keys));

            if (key.valid()) {
                d->active_keys.append(key);
                Q_EMIT keyPressed(key, layout);

                if (key.hasExtendedKeys()) {
                    d->long_press_timer.start();
                    d->long_press_layout = layout;
                }

                consumed = true;
            } else {
                const WordCandidate &candidate(Logic::wordCandidateHit(layout->wordRibbon().candidates(),
                                                                       layout->wordRibbonGeometry(),
                                                                       pos));

                if (candidate.valid()) {
                    d->active_candidate = candidate;
                    Q_EMIT wordCandidatePressed(candidate, layout);
                    consumed = true;
                }
            }
        } break;

        case QKeyEvent::MouseButtonRelease: {
            const Key &key(Logic::keyHit(layout->activeKeyArea().keys(),
                                         layout->activeKeyAreaGeometry(),
                                         pos, d->active_keys, Logic::AcceptIfInFilter));

            if (key.valid()) {
                removeActiveKey(&d->active_keys, key);
                Q_EMIT keyReleased(key, layout);
                consumed = true;
            } else {
                const WordCandidate &candidate(Logic::wordCandidateHit(layout->wordRibbon().candidates(),
                                                                       layout->wordRibbonGeometry(),
                                                                       pos));

                if (candidate.valid() && candidate == d->active_candidate) {
                    d->active_candidate = WordCandidate();
                    Q_EMIT wordCandidateReleased(candidate, layout);
                    consumed = true;
                }
            }

        } break;

        default:
            break;
        }
    }

    return consumed;
}

} // namespace MaliitKeyboard
