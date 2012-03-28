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

#ifndef MALIIT_KEYBOARD_LAYOUT_H
#define MALIIT_KEYBOARD_LAYOUT_H

#include "key.h"
#include "keyarea.h"
#include "wordribbon.h"

#include <QtCore>

namespace MaliitKeyboard {

class Layout;
typedef QSharedPointer<Layout> SharedLayout;

// TODO: Implement hit test on Layout, one to check whether key was hit, one to check whether word candidate was hit.
// Should return invalid key/wc, or found key/wc.
// Would be used by Glass.
class Layout
{
public:
    enum Orientation {
        Landscape,
        Portrait
    };

    enum Alignment {
        Left,
        Top,
        Right,
        Bottom
    };

    enum Panel {
        LeftPanel,
        RightPanel,
        CenterPanel,
        ExtendedPanel,
        NumPanels
    };

    Q_ENUMS(Orientation)
    Q_ENUMS(Panel)

private:
    QSize m_screen_size;
    QPoint m_origin;
    QPoint m_extended_panel_offset;
    Orientation m_orientation;
    Alignment m_alignment;
    Panel m_active_panel;
    KeyArea m_left;
    KeyArea m_right;
    KeyArea m_center;
    KeyArea m_extended;
    WordRibbon m_ribbon;

    struct {
        QVector<Key> left;
        QVector<Key> right;
        QVector<Key> center;
        QVector<Key> extended;
    } m_active_keys;

    Key m_magnifier_key;

public:
    explicit Layout();

    QSize screenSize() const;
    void setScreenSize(const QSize &size);

    QPoint extendedPanelOffset() const;
    void setExtendedPanelOffset(const QPoint &offset);

    Orientation orientation() const;
    void setOrientation(Orientation orientation);

    Alignment alignment() const;
    void setAlignment(Alignment alignment);

    Panel activePanel() const;
    void setActivePanel(Panel panel);
    QRect geometry(Panel panel) const;

    KeyArea activeKeyArea() const;
    void setActiveKeyArea(const KeyArea &active);
    QRect activeKeyAreaGeometry() const;

    KeyArea leftPanel() const;
    void setLeftPanel(const KeyArea &left);
    QRect leftPanelGeometry() const;

    KeyArea rightPanel() const;
    void setRightPanel(const KeyArea &right);
    QRect rightPanelGeometry() const;

    KeyArea centerPanel() const;
    void setCenterPanel(const KeyArea &center);
    QRect centerPanelGeometry() const;

    KeyArea extendedPanel() const;
    void setExtendedPanel(const KeyArea &extended);
    QRect extendedPanelGeometry() const;

    WordRibbon wordRibbon() const;
    void setWordRibbon(const WordRibbon &ribbon);
    QRect wordRibbonGeometry() const;

    QVector<Key> activeKeys() const;
    void clearActiveKeys();
    void appendActiveKey(const Key &key);
    void removeActiveKey(const Key &key);

    Key magnifierKey() const;
    void setMagnifierKey(const Key &key);
    void clearMagnifierKey();

private:
    // TODO: Move into .cpp file instead.
    KeyArea lookup(Panel panel) const;
    QPoint origin() const;
    QPoint panelOrigin() const;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_LAYOUT_H
