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



#include "keyevent.h"

KeyEvent::KeyEvent(const QString &text, QKeyEvent::Type type, Qt::Key qtKey, SpecialKey specialKey,
                   Qt::KeyboardModifiers modifiers, const QPoint &touchPoint)
    : m_type(type),
      m_qtKey(qtKey),
      m_specialKey(specialKey),
      m_text(text),
      m_modifiers(modifiers),
      m_touchPoint(touchPoint)
{
}

KeyEvent::KeyEvent(const KeyEvent &other, QKeyEvent::Type type)
    : m_type(type),
      m_qtKey(other.m_qtKey),
      m_specialKey(other.m_specialKey),
      m_text(other.m_text),
      m_modifiers(other.m_modifiers),
      m_touchPoint(other.m_touchPoint)
{
}

Qt::Key KeyEvent::qtKey() const
{
    return m_qtKey;
}

KeyEvent::SpecialKey KeyEvent::specialKey() const
{
    return m_specialKey;
}

Qt::KeyboardModifiers KeyEvent::modifiers() const
{
    return m_modifiers;
}

QKeyEvent KeyEvent::toQKeyEvent() const
{
    return QKeyEvent(m_type, m_qtKey, m_modifiers, m_text, false, qMax(m_text.size(), 1));
}

QKeyEvent::Type KeyEvent::type() const
{
    return m_type;
}

QString KeyEvent::text() const
{
    return m_text;
}

bool KeyEvent::operator==(const KeyEvent &other) const
{
    return (m_qtKey == other.m_qtKey)
           && (m_specialKey == other.m_specialKey)
           && (m_modifiers == other.m_modifiers)
           && (m_type == other.m_type)
           && (m_text == other.m_text);
}

void KeyEvent::setTouchPoint(const QPoint &point)
{
    m_touchPoint = point;
}

QPoint KeyEvent::touchPoint() const
{
    return m_touchPoint;
}
