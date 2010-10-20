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



#ifndef VKB_KEYEVENT_H
#define VKB_KEYEVENT_H

#include <QKeyEvent>
#include <QPoint>
#include <Qt>

//! Internal keyboard event class, basically a superset of QKeyEvent
class KeyEvent
{
public:
    //! Special keys defined by virtual keyboard and not covered by Qt::Key
    enum SpecialKey {
        NotSpecial,
        Copy,
        Paste,
        LayoutMenu,
        CycleSet,
        Sym,
        Commit,
        Switch,
        NumSpecialKeys
    };

    //! Constructor that takes all attributes
    KeyEvent(const QString &text = QString(),
             QKeyEvent::Type type = QEvent::KeyRelease,
             Qt::Key qtKey = Qt::Key_unknown,
             SpecialKey specialKey = NotSpecial,
             Qt::KeyboardModifiers modifiers = Qt::NoModifier,
             const QPoint &touchPoint = QPoint());

    //! Constructor that copies another event, except for type
    KeyEvent(const KeyEvent &other, QKeyEvent::Type type);

    /*!
     * \return Qt key code. We always use Qt::Key_unknown in case key code is
     * not used, unlike QKeyEvent::key().  Also, this is not used for normal
     * alphanumeric keys; for those the text is all there is.
     */
    Qt::Key qtKey() const;

    //! \return special key code, which can augment or override Qt key code
    SpecialKey specialKey() const;

    //! \return modifiers
    Qt::KeyboardModifiers modifiers() const;

    /*!
     * \return Unicode text the key generated. Can be empty, in which case
     * specialKey() or qtKey() should return something useful.
     */
    QString text() const;

    /*!
     * \return this event converted into a QKeyEvent.  Results are undefined
     * if specialKey() returns other than NotSpecial.
     */
    QKeyEvent toQKeyEvent() const;

    //! \return event type, either QEvent::KeyPress or QEvent::KeyRelease
    QKeyEvent::Type type() const;

    bool operator==(const KeyEvent &other) const;

    void setTouchPoint(const QPoint &point);

    QPoint touchPoint() const;

protected:
    QKeyEvent::Type m_type;
    Qt::Key m_qtKey;
    SpecialKey m_specialKey;
    QString m_text;
    Qt::KeyboardModifiers m_modifiers;
    QPoint m_touchPoint;
};

#endif
