/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */


#include "keyeventhandler.h"
#include "keyevent.h"
#include "ikeybutton.h"
#include "keybuttonarea.h"

#include <QDebug>

KeyEventHandler::KeyEventHandler(QObject *parent)
    : QObject(parent),
      shiftHeldDown(false),
      ignoreShiftClick(false)
{
}

void KeyEventHandler::addEventSource(KeyButtonArea *eventSource)
{
    bool ok = false;

    ok = connect(eventSource, SIGNAL(keyPressed(const IKeyButton *, const QString &, bool)),
                 this, SLOT(handleKeyPress(const IKeyButton *, const QString &, bool)));
    Q_ASSERT(ok);

    ok = connect(eventSource, SIGNAL(keyReleased(const IKeyButton *, const QString &, bool)),
                 this, SLOT(handleKeyRelease(const IKeyButton *, const QString &, bool)));
    Q_ASSERT(ok);

    ok = connect(eventSource, SIGNAL(keyClicked(const IKeyButton *, const QString &, bool, const QPoint &)),
                 this, SLOT(handleKeyClick(const IKeyButton *, const QString &, bool, const QPoint &)));
    Q_ASSERT(ok);
}

void KeyEventHandler::handleKeyPress(const IKeyButton *key, const QString &accent, bool upperCase)
{
    const KeyEvent event = keyToKeyEvent(*key, QEvent::KeyPress, accent, upperCase);
    emit keyPressed(event);

    if (event.qtKey() == Qt::Key_Shift) {
        shiftHeldDown = true;
        emit shiftPressed(true);
    } else if (shiftHeldDown) {
        ignoreShiftClick = true;
    }
}

void KeyEventHandler::handleKeyRelease(const IKeyButton *key, const QString &accent, bool upperCase)
{
    const KeyEvent event = keyToKeyEvent(*key, QEvent::KeyRelease, accent, upperCase);

    emit keyReleased(event);

    if (event.qtKey() == Qt::Key_Shift && shiftHeldDown) {
        shiftHeldDown = false;
        emit shiftPressed(false);
    }
}

void KeyEventHandler::handleKeyClick(const IKeyButton *key, const QString &accent, bool upperCase,
                                     const QPoint &touchPoint)
{
    const KeyEvent event = keyToKeyEvent(*key, QEvent::KeyRelease, accent, upperCase, touchPoint);

    if (event.qtKey() == Qt::Key_Shift && ignoreShiftClick) {
        ignoreShiftClick = false; // ignore this event
    } else {
        emit keyClicked(event);
    }
}

KeyEvent KeyEventHandler::keyToKeyEvent(const IKeyButton &key, QKeyEvent::Type eventType,
                                         const QString &accent, bool upperCase, const QPoint &touchPoint) const
{
    KeyEvent event;

    // Send always upper case letter if shift held down.
    upperCase |= shiftHeldDown;

    if (accent.isEmpty()) {
        event = key.key().toKeyEvent(eventType, upperCase);
    } else {
        event = key.key().toKeyEvent(eventType, accent.at(0), upperCase);
    }
    event.setTouchPoint(touchPoint);

    return event;
}

