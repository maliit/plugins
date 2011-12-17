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

Key findActiveKey(QVector<Key> *active_keys,
                  const QPoint &origin,
                  const QPoint &pos)
{
    if (not active_keys) {
        return Key();
    }

    for (int index = 0; index < active_keys->count(); ++index) {
        const Key &k(active_keys->at(index));
        if (k.rect().translated(origin).contains(pos)) {
            return k;
        }
    }

    return Key();
}
}

class GlassPrivate
{
public:
    QWidget *window;
    QVector<SharedLayout> layouts;
    QVector<Key> active_keys;
    QPoint last_pos;
    QPoint press_pos;
    QElapsedTimer gesture_timer;
    bool gesture_triggered;

    explicit GlassPrivate()
        : window(0)
        , layouts()
        , active_keys()
        , last_pos()
        , press_pos()
        , gesture_timer()
        , gesture_triggered(false)
    {}
};

Glass::Glass(QObject *parent)
    : QObject(parent)
    , d_ptr(new GlassPrivate)
{}

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

        // Intended fallthru:
    case QKeyEvent::MouseButtonRelease: {
        if (d->gesture_triggered) {
            return false;
        }

        QMouseEvent *qme = static_cast<QMouseEvent *>(ev);
        d->last_pos = qme->pos();
        d->press_pos = qme->pos(); // FIXME: dont update on mouse release, clear instead.
        ev->accept();

        Q_FOREACH (const SharedLayout &layout, d->layouts) {
            const QPoint &pos(layout->orientation() == Layout::Landscape
                              ? qme->pos() : QPoint(d->window->height() - qme->pos().y(), qme->pos().x()));
            const QVector<Key> &keys(layout->activeKeyArea().keys);

            // FIXME: use binary range search
            const QRect &rect(layout->activeKeyArea().rect.toRect());
            if (rect.contains(pos)) {

                for (int index = 0; index < keys.count(); ++index) {
                    Key k(keys.at(index));

                    // Pressed state is not stored in key itself, so list of pressed keys (overridden keys) must be maintained elsewhere.
                    const QRect &key_rect = k.rect().translated(rect.topLeft());
                    if (key_rect.contains(pos)) {

                        if (qme->type() == QKeyEvent::MouseButtonPress) {
                            d->active_keys.append(k);
                            Q_EMIT keyPressed(k, layout);
                        } else if (qme->type() == QKeyEvent::MouseButtonRelease) {
                            removeActiveKey(&d->active_keys, k);
                            Q_EMIT keyReleased(k, layout);
                        }

                        return true;
                    }
                }
            }
        }
    } break;

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

            const QRect &rect(layout->activeKeyArea().rect.toRect());

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



            const QVector<Key> &keys(layout->activeKeyArea().keys);

            // FIXME: use binary range search
            const QPoint &origin(rect.topLeft());

            const Key &last_key(findActiveKey(&d->active_keys, origin, last_pos));
            if (not last_key.rect().isEmpty()
                && not last_key.rect().translated(origin).contains(pos)) {
                removeActiveKey(&d->active_keys, last_key);
                Q_EMIT keyExited(last_key, layout);
            }

            if (rect.contains(pos)) {
                for (int index = 0; index < keys.count(); ++index) {
                    Key k(keys.at(index));

                    // Pressed state is not stored in key itself, so list of pressed keys (overridden keys) must be maintained elsewhere.
                    const QRect &key_rect = k.rect().translated(origin);
                    if (key_rect.contains(pos)
                        && k != findActiveKey(&d->active_keys, origin, pos)) {
                        d->active_keys.append(k);
                        Q_EMIT keyEntered(k, layout);

                        return true;
                    }
                }
            }
        }
    } break;

    default:
        break;
    }

    return false;
}

} // namespace MaliitKeyboard
