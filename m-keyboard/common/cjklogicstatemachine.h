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

#ifndef CJKLOGICSTATEMACHINE_H
#define CJKLOGICSTATEMACHINE_H

#include "keyboarddata.h"
#include <QList>
#include <QObject>
#include <QTimer>

#include <mimsettings.h>

class QString;
class KeyEvent;
class InputStateAbstract;
class AbstractEngineWidgetHost;
class MAbstractInputMethodHost;
class MImEngineWordsInterface;


class CJKLogicStateMachine : public QObject
{
    Q_OBJECT
    friend class StandbyState;
    friend class MatchState;
    friend class MatchNotStartedState;
    friend class MatchStartedState;
    friend class PredictionState;
public:
    CJKLogicStateMachine(AbstractEngineWidgetHost &engineWidgetHost,
                               MAbstractInputMethodHost &host,
                               MImEngineWordsInterface &engine);

    virtual ~CJKLogicStateMachine();

public slots:
    bool handleKeyPress(const KeyEvent &event);
    bool handleKeyRelease(const KeyEvent &event);
    bool handleKeyClick(const KeyEvent &event);

    void handleOrientationChange(M::Orientation orientation);

    void resetWithCommitStringToApp();

    void resetWithoutCommitStringToApp();

    void setSyllableDivideEnabled(bool enabled);
    bool IsSyllableDivideEnabled() const { return syllableDivideIsEnabled;}

signals:
    void layoutMenuKeyClicked();
    void symbolKeyClicked();
    void symbolSwitchKeyClicked();
    void composeStateChanged(bool);
    void toggleKeyStateChanged(bool);
    void keyOverrideActiveRequest(bool);

private slots:
    bool handleKeyEvent(const KeyEvent &event);

    //All these function are split from function handleKeyEvent(const KeyEvent &event).
    void handleLayoutMenuKey(const KeyEvent &event);
    void handleSymbolKey(const KeyEvent &event);
    void handleSymbolSwitchKey(const KeyEvent &event);
    void handleToggleKeyClicked(bool bReset = false);

    void handleDigitKey(const KeyEvent &event);
    void handleLetterKey(const KeyEvent &event);
    void handleQuotationMarkKey(const KeyEvent &event);
    void handleSpaceKey(const KeyEvent &event);
    void handleBackspaceKey(const KeyEvent &event);
    void handleLongPressBackspaceKey();
    void handleEnterKey(const KeyEvent &event);
    void handleOtherKey(const KeyEvent &event);

    void handleArrowKey(const KeyEvent &event);
    void handleCandidateClicked(const QString &candStr, int wordIndex);

    //common handle function can be used in part of the sub states.
    //Place these function here is to avoid duplicate code in several places.

    //! specified on handling no-Standby mode
    void handleArrowKeyEx(const KeyEvent &event);

    /*!
     * \brief Send preedit that will be showed in the text entry. The preedit is the combination of
              the two parameter string.
     * \param matchedPart The prefix part of the preedit, showing directly. If match didn't start,
              then this string is empty.
     * \param unmatchedPart The suffix part of the preedit, which is showing based on
     *        syllableDivideIsEnabled flag.
     */
    void sendPreedit(const QString &matchedPart, const QString &unmatchedPart);

    void syncChineseTransliteration();

private:
    void emitLayoutMenuKeyClickSignal();
    void emitKeyOverrideActiveRequest(bool flag);
    void changeState(const QString &state);
    bool isValidInputLetter(QChar ch);
    QString transliterateString(unsigned int candidateIndex, const QString &defaultText);


    InputStateAbstract *currentState;

    InputStateAbstract *standbyState;
    InputStateAbstract *matchState;
    InputStateAbstract *predictionState;

    //Passed to State Machine.
    AbstractEngineWidgetHost &engineWidgetHost;
    MAbstractInputMethodHost &inputMethodHost;
    MImEngineWordsInterface &inputMethodEngine;

    QTimer *backspaceTimer;
    bool backspaceLongPressTriggered;
    QString userChoseCandidateString;
    bool syllableDivideIsEnabled;

    bool currentOnOffState;

    MImSettings chineseTransliterationConf;
    QString chineseTransliteration;
};


class InputStateAbstract
{
public:
    virtual ~InputStateAbstract() {}

    /*!  Called when change from current state to another state by inside.
     *   Do some initial work.
     */
    virtual void initState() = 0;

    /*!  Called when change from current state to another state.
     *   Do some clear work.
     */
    virtual void clearState() = 0;

    /*!  Called when state machine is reset to standby mode by outside.
     *   Differences between shutDown() and clearState() is that:
     *   shutDown() will destroy some shared information between states.
     *   clearState() will only destroy it's owned information.
     */
    virtual void shutDown(bool needCommitString) = 0;

    virtual void handleOrientationChange(M::Orientation orientation) = 0;
    virtual void handleCandidateClicked(const QString &candStr, int wordIndex) = 0;

    virtual void handleLayoutMenuKey(const KeyEvent &event) = 0;
    virtual void handleSymbolKey(const KeyEvent &event) = 0;

    virtual void handleDigitKey(const KeyEvent &event) = 0;
    virtual void handleLetterKey(const KeyEvent &event) = 0;
    virtual void handleQuotationMarkKey(const KeyEvent &event) = 0;
    virtual void handleSpaceKey(const KeyEvent &event) = 0;
    virtual void handleBackspaceKey(const KeyEvent &event) = 0;
    virtual void handleLongPressBackspaceKey() = 0;
    virtual void handleEnterKey(const KeyEvent &event) = 0;
    virtual void handleOtherKey(const KeyEvent &event) = 0;
    virtual void handleArrowKey(const KeyEvent &event) =0;
    virtual void handleToggleKeyClicked() = 0;

protected:
    InputStateAbstract(CJKLogicStateMachine * machine) : stateMachine(machine) { }

    CJKLogicStateMachine * stateMachine;
};


class StandbyState : public InputStateAbstract
{
public:
    StandbyState(CJKLogicStateMachine * machine);
    virtual ~StandbyState();

    virtual void initState();
    virtual void clearState();
    virtual void shutDown(bool needCommitString);
    virtual void handleOrientationChange(M::Orientation orientation);
    virtual void handleCandidateClicked(const QString &candStr, int wordIndex);

    virtual void handleLayoutMenuKey(const KeyEvent &event);
    virtual void handleSymbolKey(const KeyEvent &event);

    virtual void handleDigitKey(const KeyEvent &event);
    virtual void handleLetterKey(const KeyEvent &event);
    virtual void handleQuotationMarkKey(const KeyEvent &event);
    virtual void handleSpaceKey(const KeyEvent &event);
    virtual void handleBackspaceKey(const KeyEvent &event);
    virtual void handleLongPressBackspaceKey();
    virtual void handleEnterKey(const KeyEvent &event);
    virtual void handleOtherKey(const KeyEvent &event);
    virtual void handleArrowKey(const KeyEvent &event);
    virtual void handleToggleKeyClicked();
};

class MatchState : public InputStateAbstract
{
    friend class MatchNotStartedState;
    friend class MatchStartedState;
public:
    MatchState(CJKLogicStateMachine * machine);
    virtual ~MatchState();

    virtual void initState();
    virtual void clearState();
    virtual void shutDown(bool needCommitString);
    virtual void handleOrientationChange(M::Orientation orientation);
    virtual void handleCandidateClicked(const QString &candStr, int wordIndex);

    virtual void handleLayoutMenuKey(const KeyEvent &event);
    virtual void handleSymbolKey(const KeyEvent &event);

    virtual void handleDigitKey(const KeyEvent &event);
    virtual void handleLetterKey(const KeyEvent &event);
    virtual void handleQuotationMarkKey(const KeyEvent &event);
    virtual void handleSpaceKey(const KeyEvent &event);
    virtual void handleBackspaceKey(const KeyEvent &event);
    virtual void handleLongPressBackspaceKey();
    virtual void handleEnterKey(const KeyEvent &event);
    virtual void handleOtherKey(const KeyEvent &event);
    virtual void handleArrowKey(const KeyEvent &event);
    virtual void handleToggleKeyClicked();

protected:
    void changeMatchState(QString newMatchState);
    InputStateAbstract * currentMatchState;
    InputStateAbstract * matchNotStartState;
    InputStateAbstract * matchStartState;

    //preedit used to record user input.
    QString inputPreedit;

    //record the length of the string recognized by engine.
    int recognizedStringLength;

    //Flag is true if there is no candidates for pinyin string.
    bool matchIsTerminated;
};

class MatchNotStartedState: public InputStateAbstract
{
public:
    MatchNotStartedState(MatchState * matchMachine, CJKLogicStateMachine * machine);
    virtual ~MatchNotStartedState();

    virtual void initState();
    virtual void clearState();
    virtual void shutDown(bool needCommitString);
    virtual void handleOrientationChange(M::Orientation orientation);
    virtual void handleCandidateClicked(const QString &candStr, int wordIndex);

    virtual void handleLayoutMenuKey(const KeyEvent &event);
    virtual void handleSymbolKey(const KeyEvent &event);

    virtual void handleDigitKey(const KeyEvent &event);
    virtual void handleLetterKey(const KeyEvent &event);
    virtual void handleQuotationMarkKey(const KeyEvent &event);
    virtual void handleSpaceKey(const KeyEvent &event);
    virtual void handleBackspaceKey(const KeyEvent &event);
    virtual void handleLongPressBackspaceKey();
    virtual void handleEnterKey(const KeyEvent &event);
    virtual void handleOtherKey(const KeyEvent &event);
    virtual void handleArrowKey(const KeyEvent &event);
    virtual void handleToggleKeyClicked();

protected:
    MatchState * matchStateMachine;
};

class MatchStartedState: public InputStateAbstract
{
public:
    MatchStartedState(MatchState * matchMachine, CJKLogicStateMachine * machine);
    virtual ~MatchStartedState();

    virtual void initState();
    virtual void clearState();
    virtual void shutDown(bool needCommitString);
    virtual void handleOrientationChange(M::Orientation orientation);
    virtual void handleCandidateClicked(const QString &candStr, int wordIndex);

    virtual void handleLayoutMenuKey(const KeyEvent &event);
    virtual void handleSymbolKey(const KeyEvent &event);

    virtual void handleDigitKey(const KeyEvent &event);
    virtual void handleLetterKey(const KeyEvent &event);
    virtual void handleQuotationMarkKey(const KeyEvent &event);
    virtual void handleSpaceKey(const KeyEvent &event);
    virtual void handleBackspaceKey(const KeyEvent &event);
    virtual void handleLongPressBackspaceKey();
    virtual void handleEnterKey(const KeyEvent &event);
    virtual void handleOtherKey(const KeyEvent &event);
    virtual void handleArrowKey(const KeyEvent &event);
    virtual void handleToggleKeyClicked();

private:
    // Record string list history for learning new words.
    QStringList matchedPreeditHistory;
    QStringList selectedCandidatesHistory;

protected:
    MatchState * matchStateMachine;
};

class PredictionState : public InputStateAbstract
{
public:
    PredictionState(CJKLogicStateMachine * machine);
    virtual ~PredictionState();

    virtual void initState();
    virtual void clearState();
    virtual void shutDown(bool needCommitString);
    virtual void handleOrientationChange(M::Orientation orientation);
    virtual void handleCandidateClicked(const QString &candStr, int wordIndex);

    virtual void handleLayoutMenuKey(const KeyEvent &event);
    virtual void handleSymbolKey(const KeyEvent &event);

    virtual void handleDigitKey(const KeyEvent &event);
    virtual void handleLetterKey(const KeyEvent &event);
    virtual void handleQuotationMarkKey(const KeyEvent &event);
    virtual void handleSpaceKey(const KeyEvent &event);
    virtual void handleBackspaceKey(const KeyEvent &event);
    virtual void handleLongPressBackspaceKey();
    virtual void handleEnterKey(const KeyEvent &event);
    virtual void handleOtherKey(const KeyEvent &event);
    virtual void handleArrowKey(const KeyEvent &event);
    virtual void handleToggleKeyClicked();

private:

};

#endif // MKEYBOARDLOGICSTATEMACHINE_H
