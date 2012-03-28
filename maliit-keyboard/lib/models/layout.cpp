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

#include "layout.h"

namespace MaliitKeyboard {

Layout::Layout()
    : m_screen_size()
    , m_extended_panel_offset()
    , m_orientation(Landscape)
    , m_alignment(Bottom)
    , m_active_panel(CenterPanel)
    , m_left()
    , m_right()
    , m_center()
    , m_extended()
    , m_ribbon()
    , m_active_keys()
    , m_magnifier_key()
{}

QSize Layout::screenSize() const
{
    return m_screen_size;
}

void Layout::setScreenSize(const QSize &size)
{
    m_screen_size = size;
}

QPoint Layout::extendedPanelOffset() const
{
    return m_extended_panel_offset;
}

void Layout::setExtendedPanelOffset(const QPoint &offset)
{
    m_extended_panel_offset = offset;
}

Layout::Orientation Layout::orientation() const
{
    return m_orientation;
}

void Layout::setOrientation(Orientation orientation)
{
    m_orientation = orientation;
}

Layout::Alignment Layout::alignment() const
{
    return m_alignment;
}

void Layout::setAlignment(Alignment alignment)
{
    m_alignment = alignment;
}

Layout::Panel Layout::activePanel() const
{
    return m_active_panel;
}

void Layout::setActivePanel(Panel panel)
{
    if (panel != NumPanels) {
        m_active_panel = panel;
    }
}

QRect Layout::geometry(Panel panel) const
{
    Q_UNUSED(panel)
    return QRect();
}

KeyArea Layout::activeKeyArea() const
{
    return lookup(activePanel());
}

void Layout::setActiveKeyArea(const KeyArea &active)
{
    switch (activePanel()) {
    case LeftPanel: setLeftPanel(active); break;
    case RightPanel: setRightPanel(active); break;
    case CenterPanel: setCenterPanel(active); break;
    case ExtendedPanel: setExtendedPanel(active); break;

    default:
        qCritical() << __PRETTY_FUNCTION__
                    << "Should not be reached, invalid panel:" << activePanel();
        break;
    }
}

QRect Layout::activeKeyAreaGeometry() const
{
    switch(m_active_panel) {
    case LeftPanel: return leftPanelGeometry();
    case RightPanel: return rightPanelGeometry();
    case CenterPanel: return centerPanelGeometry();
    case ExtendedPanel: return extendedPanelGeometry();

    default:
        break;
    }

    qCritical() << __PRETTY_FUNCTION__
                << "Should not be reached, invalid panel:" << m_active_panel;
    return QRect();
}

KeyArea Layout::leftPanel() const
{
    return m_left;
}

void Layout::setLeftPanel(const KeyArea &left)
{
    if (m_left != left) {
        m_left = left;
    }
}

QRect Layout::leftPanelGeometry() const
{
    return QRect();
}

KeyArea Layout::rightPanel() const
{
    return m_right;
}

void Layout::setRightPanel(const KeyArea &right)
{
    if (m_right != right) {
        m_right = right;
    }
}

QRect Layout::rightPanelGeometry() const
{
    return QRect();
}

KeyArea Layout::centerPanel() const
{
    return m_center;
}

void Layout::setCenterPanel(const KeyArea &center)
{
    if (m_center != center) {
        m_center = center;
    }
}

QRect Layout::centerPanelGeometry() const
{
    return QRect(panelOrigin(), m_center.area().size());
}

KeyArea Layout::extendedPanel() const
{
    return m_extended;
}

void Layout::setExtendedPanel(const KeyArea &extended)
{
    if (m_extended != extended) {
        m_extended = extended;
    }
}

QRect Layout::extendedPanelGeometry() const
{
    return QRect(panelOrigin() + m_extended_panel_offset, m_extended.area().size());
}

WordRibbon Layout::wordRibbon() const
{
    return m_ribbon;
}

void Layout::setWordRibbon(const WordRibbon &ribbon)
{
    m_ribbon = ribbon;
}

QRect Layout::wordRibbonGeometry() const
{
    return QRect(origin(), m_ribbon.area().size());
}

QVector<Key> Layout::activeKeys() const
{
    switch (m_active_panel) {
    case LeftPanel: return m_active_keys.left;
    case RightPanel: return m_active_keys.right;
    case CenterPanel: return m_active_keys.center;
    case ExtendedPanel: return m_active_keys.extended;
    case NumPanels: break;
    }

    return QVector<Key>();
}

void Layout::clearActiveKeys()
{
    m_active_keys.left.clear();
    m_active_keys.right.clear();
    m_active_keys.center.clear();
    m_active_keys.extended.clear();
}

void Layout::appendActiveKey(const Key &key)
{
    switch (m_active_panel) {
    case LeftPanel: m_active_keys.left.append(key); break;
    case RightPanel: m_active_keys.right.append(key); break;
    case CenterPanel: m_active_keys.center.append(key); break;
    case ExtendedPanel: m_active_keys.extended.append(key); break;
    case NumPanels: break;
    }
}

void Layout::removeActiveKey(const Key &key)
{
    QVector<Key> *active_keys = 0;
    switch (m_active_panel) {
    case LeftPanel: active_keys = &m_active_keys.left; break;
    case RightPanel: active_keys = &m_active_keys.right; break;
    case CenterPanel: active_keys = &m_active_keys.center; break;
    case ExtendedPanel: active_keys = &m_active_keys.extended; break;
    case NumPanels: break;
    }

    if (active_keys) {
        for (int index = 0; index < active_keys->count(); ++index) {
            const Key &current(active_keys->at(index));
            if (current.origin() == key.origin()
                && current.label() == key.label()) {
                active_keys->remove(index);
                break;
            }
        }
    }
}

Key Layout::magnifierKey() const
{
    return m_magnifier_key;
}

void Layout::setMagnifierKey(const Key &key)
{
    m_magnifier_key = key;
}

void Layout::clearMagnifierKey()
{
    setMagnifierKey(Key());
}

KeyArea Layout::lookup(Panel panel) const
{
    switch(panel) {
    case LeftPanel: return m_left;
    case RightPanel: return m_right;
    case CenterPanel: return m_center;
    case ExtendedPanel: return m_extended;
    case NumPanels: break;
    }

    qCritical() << __PRETTY_FUNCTION__
                << "Should not be reached, invalid panel:" << panel;
    return KeyArea();
}

QPoint Layout::origin() const
{
    const QSize size(m_orientation == Landscape ? m_screen_size
                                                : QSize(m_screen_size.height(), m_screen_size.width()));
    const QSize &center_size(m_center.area().size());


    switch(m_alignment) {
    case Top:
        return QPoint(size.width() / 2 - center_size.width() / 2, 0);

    case Bottom:
        return QPoint(size.width() / 2 - center_size.width() / 2,
                      size.height() - center_size.height() - m_ribbon.area().size().height());
    default:
        break;
    }

    qCritical() << __PRETTY_FUNCTION__
                << "Should not be reached, invalid alignment:" << m_alignment;
    return QPoint();
}

QPoint Layout::panelOrigin() const
{
    return (origin() + QPoint(0, m_ribbon.area().size().height()));
}

} // namespace MaliitKeyboard
