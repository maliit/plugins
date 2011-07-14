/*
 * This file is part of meego-keyboard
 *
 * Copyright (C) 2010-2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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

 */

#ifndef NAMESPACE_H
#define NAMESPACE_H

#include <QtCore>
#include <QtGui>

namespace MaliitKeyboard {

    //! Defines whether key is idle, pressed or selected.
    enum KeyState {
        KeyStateIdle,     //!< Key is idle (default state)
        KeyStatePressed,  //!< Key is pressed
        KeyStateSelected, //!< Key was selected (e.g., caps lock selects shift key)
        KeyStateDisabled, //!< Key allows no user interaction
    };

    //! Defines actions that can be bound to a key.
    enum KeyAction {
        KeyActionInsert,
        KeyActionShift,
        KeyActionSpace,
        KeyActionBackspace,
        KeyActionCycle,
        KeyActionLayoutMenu,
        KeyActionSym,
        KeyActionReturn,
        KeyActionDecimalSeparator,
        KeyActionPlusMinusToggle,
        KeyActionTab,
        KeyActionCommit,
        KeyActionSwitch,
        KeyActionOnOffToggle,
        KeyActionCompose
    };

    //! Defines style type of keys.
    enum KeyStyle {
        KeyStyleNormal,    //!< Key uses normal style (character key)
        KeyStyleSpecial,   //!< Key uses special style (return, shift, etc.)
        KeyStyleDeadkey    //!< Key uses deadkey style
    };

    //! Defines width of keys.
    enum KeyWidth {
        KeyWidthSmall,     //!< Key uses small width
        KeyWidthMedium,    //!< Key uses medium width
        KeyWidthLarge,     //!< Key uses large width
        KeyWidthXLarge,    //!< Key uses extra large width
        KeyWidthXxLarge,   //!< Key uses extra-extra large width
        KeyWidthStretched  //!< Key consumes remaining width
    };

    //! Defines height of keys.
    enum KeyHeight {
        KeyHeightSmall,    //!< Key uses small height
        KeyHeightMedium,   //!< Key uses medium height
        KeyHeightLarge,    //!< Key uses large height
        KeyHeightXLarge,   //!< Key uses extra large height
        KeyHeightXxLarge,  //!< Key uses extra-extra large height
        KeyHeightStretched //!< Key consumes remaining height
    };

    //! Defines all the attributes of an key override.
    enum KeyOverride {
        KeyOverrideNone = 0x0,
        KeyOverrideLabel = 0x1,
        KeyOverrideIcon  = 0x2,
        KeyOverrideHighlighted = 0x4,
        KeyOverrideEnabled = 0x8
    };
    Q_DECLARE_FLAGS(KeyOverrides, KeyOverride)
}

#endif // NAMESPACE_H
