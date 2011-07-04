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



#ifndef MHARDWAREKEYBOARD_H
#define MHARDWAREKEYBOARD_H

#include <Qt>
#include <QObject>
#include <QString>
#include <QEvent>
#include <QTimer>
#include <QTime>
#include <QRegExp>
#include "mxkb.h"
#include "hwkbcharloopsmanager.h"
#include "hwkbdeadkeymapper.h"
#include "mkeyboardcommon.h"
#include <minputmethodnamespace.h>

class MAbstractInputMethodHost;

/*!
  \brief MHardwareKeyboard implements the hardware keyboard for inputmethod plugin.

  Class MHardwareKeyboard provides enhanced functionality for hardware keyboard, such as
  latch/unlatch and lock/unlock Shift/Fn modifiers; Symbol key functionality; auto-lock Fn
  key when focus is in number/phone number content type etc.  See
  meego-im-framework/doc/src/internals.dox section KeyEventFiltering for more detail.
*/
class MHardwareKeyboard : public QObject
{
    Q_OBJECT

public:
    /*!
     * \brief Constructor for creating an MHardwareKeyboard object.
     * \param imHost input method host to use for sending events and
     * pre-edit and commit strings
     * \param parent Parent object.
     */
    MHardwareKeyboard(MAbstractInputMethodHost& imHost, QObject *parent = 0);

    //! Destructor
    ~MHardwareKeyboard();

    //! \brief Set keyboard type according text entry type.
    void setKeyboardType(MInputMethod::TextContentType type);

    //! \brief Return current keyboard type.
    MInputMethod::TextContentType keyboardType() const;

    //! \return current state for \a modifier key in hardware keyboard.
    ModifierState modifierState(Qt::KeyboardModifier modifier) const;

    //! \return current dead key composing state in form of the active dead key character
    //! or null character
    QChar deadKeyState() const;

    //! Set auto capitalization state.
    void setAutoCapitalization(bool state);

    /*! \brief Enable MHardwareKeyboard functionality
     *
     * This implies that autorepeat is configured, modifier and other state is reset and
     * key event redirection from input context is enabled.
     * Note: direct input mode won't support autorepeat and modifier state, only handle
     * symbol key.
     *
     * You can call enable() again after enable() without calling disable() in between.
     * This could be done e.g. when focus is changed from one widget to another.
     */
    void enable();

    /*! \brief Disable MHardwareKeyboard functionality
     *
     * This implies that autorepeat is enabled, modifier state is reset and key event
     * redirection from input context is disabled.
     */
    void disable();

    /*! \brief Reset internal state of the hardware keyboard code.
     * \post modifiers in clear state
     */
    void reset();

    //! \brief MAbstractInputMethod::clientChanged implementation
    void clientChanged();

    /*!
     * \brief Filter input key events that come from the hardware keyboard.
     *
     * Exception: the key event may also come from the symbol view.  In that case
     * nativeModifiers is allowed not to be correct (always 0).  nativeScanCode is
     * 0 as well.
     *
     * See meego-im-framework/doc/src/internals.dox section KeyEventFiltering for more detail.
     *
     * \return true if the event was handled, false otherwise
     */
    bool filterKeyEvent(QEvent::Type eventType, Qt::Key keyCode,
                        Qt::KeyboardModifiers modifiers, const QString &text,
                        bool autoRepeat, int count, quint32 nativeScanCode,
                        quint32 nativeModifiers, unsigned long time);

    //! \return whether the symbol view is available for the current layout.
    bool symViewAvailable() const;

    //! \return whether the autocaps is enabled for the current layout.
    bool autoCapsEnabled() const;

    //! Set input method mode
    void setInputMethodMode(MInputMethod::InputMethodMode mode);

    //! \return current input method mode
    MInputMethod::InputMethodMode inputMethodMode() const;

signals:

    //! \bried Emitted when symbol key is clicked (pressed and released consecutively).
    void symbolKeyClicked();

    //! \brief Emitted when shift state is changed.
    void shiftStateChanged() const;

    /*! \brief Emitted when the state of modifiers is changed between normal/latched/locked.
     *
     * Can be emitted also when the modifier state has not changed.
     */
    void modifiersStateChanged() const;

    //! \brief Emitted when dead key composing state changes
    void deadKeyStateChanged(const QChar &deadKey);

    //! \brief Emitted when the script is changed.
    void scriptChanged() const;

    //! \brief Emitted when the hardware keyboard is enabled.
    void enabled();

private slots:
    //! Called when long press timer started on key press timeouts.
    void handleLongPressTimeout();

    //! Display notification for Ctrl+C
    void handleClipboardDataChange();

private:
    //! \brief If character is not accepted by the client, try to find an acceptable character
    //! on another shift level
    //!
    //! Only supports MTextEdit's number and phone number content type at the moment and
    //! even for those we simply have a hardwired set of accepted characters, instead of
    //! considering the current decimal separator.
    //!
    //! \post text and fnPressState possibly modified for the found character
    void correctToAcceptedCharacter(QString &text, quint32 nativeScanCode,
                                    quint32 nativeModifiers, bool &fnPressState) const;

    //! \brief Helper for \a filterKeyPress that handles script switching shortcuts
    bool handleScriptSwitchOnPress(Qt::Key keyCode, Qt::KeyboardModifiers modifiers);

    //! \brief Helper for \a filterKeyRelease that handles script switching shortcuts
    bool handleScriptSwitchOnRelease(Qt::Key keyCode, Qt::KeyboardModifiers modifiers);

    /*! \brief Process key release event when symbol modifier is pressed.
     *
     * \return true if the event was processed/consumed, false otherwise
     */
    bool handleReleaseWithSymModifier(Qt::Key keyCode);

    //! \brief Just like \a handleReleaseWithSymModifier but for key press events.
    //! \post text and nativeModifiers are changed in case ongoing loop is terminated when
    //! shift or fn is latched and the latched modifier is not additionally pressed
    bool handlePressWithSymModifier(QString &text, quint32 nativeScanCode,
                                    quint32 &nativeModifiers);

    //! \brief Helper for \a handlePressWithSymModifier and \a handleReleaseWithSymModifier
    //! to stop the ongoing looping.
    void commitSymPlusCharacterCycle();

    //! \return true if the key has known action on press event, false otherwise
    bool actionOnPress(Qt::Key keyCode) const;

    //! \return true if the key press event is such that it should be passed to the application
    bool passKeyOnPress(Qt::Key keyCode, const QString &text, quint32 nativeScanCode,
                        quint32 nativeModifiers) const;

    /*! When X11 modifier bits indicated by \a affect mask in \a value are changed
     * compared to their state in \a previousModifiers, emit modifierStatesChanged signal
     * with a modifier corresponding to the changed bit and state that is either
     * ModifierClearState (modifier off) or \a onState (modifier on).  \a shiftMask can be
     * either ShiftMask or LockMask, corresponding to different shift modifier masks used
     * for latching and locking, respectively.
     *
     * Only Fn and Shift modifiers are checked.
     *
     * Also emit shiftStateChanged if shift modifier state changes.
     *
     * \sa modifierStatesChanged
     * \sa shiftLevelChanged
     */
    void notifyModifierChange(unsigned char previousModifiers, unsigned int shiftMask,
                              unsigned int affect, unsigned int value) const;

    /*! \brief Set latch state of modifiers indicated by \a affect to that indicated by \a value.
     *
     * \param affect X modifier mask
     * \param value modifier states indicated with bits corresponding to \a affect mask
     *
     * \post X11 latch state updated using Xkb.
     * \post changes notified using notifyModifierChange
     * \post autoCaps cleared if needed
     * \post currentLatchedMods updated
     */
    void latchModifiers(unsigned int affect, unsigned int value);

    //! Just like \a latchModifiers but set lock state instead.
    void lockModifiers(unsigned int affect, unsigned int value);

    /*! \brief Cycle Shift/Fn clear/latched/locked states as specified for modifier click.
     *
     * \param keyCode key code of released modifier
     * \param lockMask X11 modifier mask indicating what to lock in latched->locked transition
     * and what to unlock in locked->clear transition.
     * \param latchMask just like lockMask but for clear->latched, latched->locked and
     * latched->clear transitions
     * \param unlockMask mask that indicates what to unlock in clear->latched transition.
     * \param unlatchMask Just like unlockMask but indicates what to unlatch.
     */
    void cycleModifierState(Qt::Key keyCode, unsigned int lockMask,
                            unsigned int latchMask, unsigned int unlockMask,
                            unsigned int unlatchMask);

    /*! \brief Handle Fn/Shift release, cycling state using cycleModifierState if applicable.
     *
     * Parameters are as described for cycleModifierState.
     */
    void handleCyclableModifierRelease(Qt::Key keyCode, unsigned int lockMask,
                                       unsigned int latchMask, unsigned int unlockMask,
                                       unsigned int unlatchMask);

    /*! \brief Convert X keycode to Unicode string
     *
     * \param keycode X keycode
     * \param shiftLevel desired X shift level
     * (0 = no shift, 1 = shift, 2 = fn, 3 = shift+fn)
     * \return conversion result.  Valid empty result cannot be distinguished
     * from a conversion error, which also results to an empty string.
     */
    QString keycodeToString(unsigned int keycode, unsigned int shiftLevel) const;

    //! Helper for filterKeyEvent, handles press events
    bool filterKeyPress(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                        QString text, bool autoRepeat, int count,
                        quint32 nativeScanCode, quint32 nativeModifiers, unsigned long time);

    //! Helper for filterKeyEvent, handles release events
    bool filterKeyRelease(Qt::Key keyCode, Qt::KeyboardModifiers modifiers,
                          QString text,
                          quint32 nativeScanCode, quint32 nativeModifiers, unsigned long time);

    //! Helper for filterKeyRelease and filterKeyPress, handle arrow keys with Fn
    bool filterArrowKeys(QEvent::Type eventType, Qt::Key keyCode,
                         Qt::KeyboardModifiers modifiers,
                         QString text, bool autoRepeat, int count,
                         quint32 nativeModifiers) const;

    //! Recognize cases where we need to pretend Fn modifier was not on and change
    //! \a keyCode and \a text accordingly.
    void filterMaybeIgnoreFn(Qt::Key &keyCode, QString &text,
                             quint32 nativeScanCode, quint32 nativeModifiers) const;

    //! Used to track latching inside MHardwareKeyboard instead of latching keys using X.
    //! Changes \a keyCode, \a text and \a nativeModifiers accordingly. Returns true
    //! if some of the above were changed, false otherwise.
    bool handleLatching(Qt::Key &keyCode, QString &text, quint32 latchedModifiers,
                        quint32 nativeScanCode, quint32 &nativeModifiers) const;

    //! \brief Enable custom autorepeat
    //!
    //! Custom autorepeat means that only backspace and arrow keys have autorepeat
    //! functionality.  This also sets hardwired key repeat rate and delay and enables
    //! detectable autorepeat (see \a MAbstractInputMethodHost::setDetectableAutoRepeat).
    void enableCustomAutoRepeat();

    //! \brief Switch hardware keyboard layout.
    //!
    //! Some hardware keyboard cover maximum 2 scripts. For example, Chinese Zhuyin
    //! keyboard has Zhuyin and Latin printings. If current keyboard supports 2 scripts,
    //! calling this method can switch the script.
    void switchKeyMap();

    MInputMethod::TextContentType currentKeyboardType;
    MXkb mXkb;
    bool autoCaps;
    HwKbCharLoopsManager hwkbCharLoopsManager;
    MAbstractInputMethodHost& inputMethodHost;

    //! An attribute of the last key event passed to filterKeyEvent.
    QEvent::Type lastEventType;
    //! An attribute of the last key event passed to filterKeyEvent.
    Qt::Key lastKeyCode;

    // key is native scan code / X keycode, value is native modifiers at the time of the
    // press event
    typedef QHash<quint32, quint32> PressedKeyMap;
    PressedKeyMap pressedKeys;

    //! X modifier mask of currently latched modifiers. Unlike locked modifiers,
    //! latched modifiers are tracked internally by MHardwareKeyboard.
    unsigned char currentLatchedMods;

    //! Stores scan code of key for which there were latched modifier(s) on press.
    quint32 scanCodeWithLatchedModifiers;

    //! X modifier mask of currently locked modifiers.  This approximates X modifier state and
    //! is updated in reset and lockModifiers.  Note that if another application (un)locks
    //! a modifier, we don't know about that and end up out of sync with X.
    unsigned char currentLockedMods;

    //! When Sym+<character>... is in progress, index to the character loop currently
    //! being cycled, if any.  -1 otherwise.
    int characterLoopIndex;
    //! Text attribute of the last release event with sym modifier and for which a
    //! character loop was found.
    QString lastSymText;

    bool stateTransitionsDisabled;
    //! Number of shift keys in pressed state at the same time.
    unsigned char shiftsPressed;
    //! If true, user has activated caps lock by pressing two shift keys at once.  Until
    //! both of them have been released, no state transitions are done on shift release.
    //! TODO: what about on Fn release?
    bool shiftShiftCapsLock;

    QTimer longPressTimer;
    quint32 longPressKey;       // native scan code / X keycode
    quint32 longPressModifiers; // X modifier mask
    bool longPressFnPressState; // Status of Fn key for accepted character

    MInputMethod::InputMethodMode imMode;

    //! \brief Is Fn key pressed at the moment?
    bool fnPressed;

    //! What we've last sent as the preedit string
    QString preedit;
    //! Preedit not touched by long press handling
    QString preeditBeforeLongPress;
    //! Time of press event that updated \a preedit (not changed by long press processing or similar)
    unsigned long preeditTime;
    //! Native X keycode of the event that caused \a preedit to be set
    quint32 preeditScanCode;

    const QRegExp numberContentCharacterMatcher;
    const QRegExp phoneNumberContentCharacterMatcher;

    HwKbDeadKeyMapper deadKeyMapper;

    QTime lastCtrlCTime;

    friend class Ut_MHardwareKeyboard;
    friend class Ft_MHardwareKeyboard;
};

#endif
