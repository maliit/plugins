/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2012 Openismus GmbH. All rights reserved.
 *
 * Contact: maliit-discuss@lists.maliit.org
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

#include "abstractfeedback.h"

//! \class MaliitKeyboard::AbstractFeedback
//! An abstract class to play various kind of feedback for certain keyboard
//! interactions, such as key presses or gestures.
//! Derived classes provide the actual feedback functionality. They need to
//! implement (but never call) the private virtual playSomething methods.

//! \fn MaliitKeyboard::AbstractFeedback::setStyle
//! \brief Sets the shared Style instance.
//!
//! Feedback implementations that depend on style attributes should probably
//! refresh all their feedback effects when this method is called.
//! AbstractFeedback does store the Style instance in any way.

//! \fn MaliitKeyboard::AbstractFeedback::playPressFeedback
//! \brief Plays feedback when pressing a key.

//! \fn MaliitKeyboard::AbstractFeedback::playReleaseFeedback
//! \brief Plays feedback when releasing a key.

//! \fn MaliitKeyboard::AbstractFeedback::playLayoutChangeFeedback
//! \brief Plays feedback when changing a keyboard layout.

//! \fn MaliitKeyboard::AbstractFeedback::playKeyboardHideFeedback
//! \brief Plays feedback when hiding the keyboard.

namespace MaliitKeyboard
{

//! @param parent The owner of this instance. Can be 0, in case QObject
//!               ownership is not required.
AbstractFeedback::AbstractFeedback(QObject *parent)
    : QObject(parent)
{}

AbstractFeedback::~AbstractFeedback()
{}

//! \brief Triggers feedback by calling playPressFeedback().
void AbstractFeedback::onKeyPressed()
{
    playPressFeedback();
}

//! \brief Triggers feedback by calling playReleaseFeedback().
void AbstractFeedback::onKeyReleased()
{
    playReleaseFeedback();
}

//! \brief Triggers feedback by calling playLayoutChangeFeedback().
void AbstractFeedback::onLayoutChanged()
{
    playLayoutChangeFeedback();
}

//! \brief Triggers feedback by calling playKeyboardHideFeedback().
void AbstractFeedback::onKeyboardHidden()
{
    playKeyboardHideFeedback();
}

} // namespace MaliitKeyboard
