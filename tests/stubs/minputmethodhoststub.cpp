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

#include "minputmethodhoststub.h"

#include <minputmethodnamespace.h>
#include <mimplugindescription.h>
#include <QKeyEvent>
#include <QDebug>

MInputMethodHostStub::MInputMethodHostStub()
{
    clear();
    predictionValid_ = true;
    predictionEnabled_ = true;
    correctionValid_ = true;
    correctionEnabled_ = true;
    autoCapitalizationEnabled_ = true;
    contentType_ = 0;
    inputmethodMode_ = MInputMethod::InputMethodModeNormal;
    keyRedirectionEnabled = false;
    indicator = MInputMethod::NoIndicator;
}

MInputMethodHostStub::~MInputMethodHostStub()
{
    qDeleteAll(keyEvents);
    keyEvents.clear();
}

void MInputMethodHostStub::clear()
{
    preeditRectangleReturnValue = QRect();
    cursorRectangleReturnValue = QRect();
    preedit.clear();
    preeditFormats_.clear();
    commit.clear();
    qDeleteAll(keyEvents);
    keyEvents.clear();

    surroundingString.clear();
    cursorPos = 0;

    sendPreeditCalls = 0;
    sendCommitStringCalls = 0;
    sendKeyEventCalls = 0;
    notifyImInitiatedHidingCalls = 0;
    setGlobalCorrectionEnabledCalls = 0;
    predictionEnabledCalls = 0;
    correctionEnabledCalls = 0;
    contentTypeCalls = 0;
    copyCalls = 0;
    pasteCalls = 0;
    textSelected = false;
    keyRedirectionEnabled = false;
    indicator = MInputMethod::NoIndicator;
    setScreenRegionCalls = 0;
    screenRegions.clear();
    setInputMethodAreaCalls = 0;
    inputMethodAreas.clear();
    orientationAngleLocked = false;
    currentSelection.clear();
}

void MInputMethodHostStub::sendPreeditString(const QString &string,
                                             const QList<MInputMethod::PreeditTextFormat> &preeditFormats,
                                             int replaceStart,
                                             int replaceLength,
                                             int pos)
{
    Q_UNUSED(replaceStart);
    Q_UNUSED(replaceLength);
    Q_UNUSED(pos);

    preedit = string;
    preeditFormats_ = preeditFormats;
    ++sendPreeditCalls;
}

void MInputMethodHostStub::sendCommitString(const QString &string, int replaceStart,
                                            int replaceLength, int pos)
{
    // Parameter replaceStart is relative to cursorPos -> change to absolute
    replaceStart = cursorPos + replaceStart;

    // Remove
    if (replaceLength > 0 &&
        replaceStart >= 0 &&
        replaceStart < commit.length() &&
        replaceStart + replaceLength <= commit.length())
        commit.remove(replaceStart, replaceLength);

    // Append
    if (replaceStart >= 0 &&
        replaceStart <= commit.length())
        commit.insert(replaceStart, string);

    // Set cursor pos
    if (pos >= 0)
      cursorPos = replaceStart + pos;
    else if (pos < 0)
      cursorPos = replaceStart + string.length();

    ++sendCommitStringCalls;
}


void MInputMethodHostStub::sendKeyEvent(const QKeyEvent &keyEvent_,
                                        MInputMethod::EventRequestType requestType)
{
    Q_UNUSED(requestType);
    keyEvents.append(new QKeyEvent(keyEvent_));
    ++sendKeyEventCalls;
}

void MInputMethodHostStub::notifyImInitiatedHiding()
{
    ++notifyImInitiatedHidingCalls;
}

int MInputMethodHostStub::contentType(bool &val)
{
    val = true;
    return contentType_;
}

bool MInputMethodHostStub::correctionEnabled(bool &val)
{
    val = correctionValid_;
    return correctionEnabled_;
}

bool MInputMethodHostStub::predictionEnabled(bool &val)
{
    val = predictionValid_;
    return predictionEnabled_;
}

bool MInputMethodHostStub::surroundingText(QString &text, int &cursorPosition)
{
    text = surroundingString;
    cursorPosition = cursorPos;
    return true;
}

int MInputMethodHostStub::anchorPosition(bool &valid)
{
    valid = false;
    return -1;
}

int MInputMethodHostStub::inputMethodMode(bool &valid)
{
    valid = true;
    return inputmethodMode_;
}

bool MInputMethodHostStub::hasSelection(bool &valid)
{
    valid = true;
    return textSelected;
}

QRect MInputMethodHostStub::preeditRectangle(bool &valid)
{
    valid = true;
    return preeditRectangleReturnValue;
}

QRect MInputMethodHostStub::cursorRectangle(bool &valid)
{
    valid = true;
    return cursorRectangleReturnValue;
}

bool MInputMethodHostStub::autoCapitalizationEnabled(bool &val)
{
    val = true;
    return autoCapitalizationEnabled_;
}

void MInputMethodHostStub::copy()
{
    ++copyCalls;
}

void MInputMethodHostStub::paste()
{
    ++pasteCalls;
}

void MInputMethodHostStub::setGlobalCorrectionEnabled(bool enabled)
{
    globalCorrectionEnabled = enabled;
    ++setGlobalCorrectionEnabledCalls;
}

void MInputMethodHostStub::setRedirectKeys(bool enabled)
{
    keyRedirectionEnabled = enabled;
}

void MInputMethodHostStub::setDetectableAutoRepeat(bool /*enabled*/)
{
}

void MInputMethodHostStub::setInputModeIndicator(MInputMethod::InputModeIndicator newIndicator)
{
    indicator = newIndicator;
}

void MInputMethodHostStub::switchPlugin(MInputMethod::SwitchDirection direction)
{
    Q_UNUSED(direction);
}

void MInputMethodHostStub::switchPlugin(const QString &pluginName)
{
    Q_UNUSED(pluginName);
}

void MInputMethodHostStub::setScreenRegion(const QRegion &region)
{
    qDebug() << __PRETTY_FUNCTION__ << region;
    setScreenRegionCalls++;
    screenRegions << region;
}

void MInputMethodHostStub::setInputMethodArea(const QRegion &region)
{
    qDebug() << __PRETTY_FUNCTION__ << region;
    setInputMethodAreaCalls++;
    inputMethodAreas << region;
}

void MInputMethodHostStub::showSettings()
{
}

void MInputMethodHostStub::setSelection(int start, int length)
{
    Q_UNUSED(start);
    Q_UNUSED(length);
}

void MInputMethodHostStub::setOrientationAngleLocked(bool lock)
{
    orientationAngleLocked = lock;
}

QString MInputMethodHostStub::selection(bool &valid)
{
    valid = true;
    return currentSelection;
}

QList<MImPluginDescription> MInputMethodHostStub::pluginDescriptions(MInputMethod::HandlerState) const
{
    return QList<MImPluginDescription>();
}

