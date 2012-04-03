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

#ifndef INPUTMETHODHOSTPROBE_H
#define INPUTMETHODHOSTPROBE_H

#include <mabstractinputmethodhost.h>
#include <mimplugindescription.h>

#include <QKeyEvent>

class InputMethodHostProbe
    : public MAbstractInputMethodHost
{
    Q_OBJECT

private:
    QString m_commit_string_history;
    QKeyEvent m_last_key_event;
    int m_key_event_count;

public:
    InputMethodHostProbe();

    QString commitStringHistory() const;

    void sendCommitString(const QString &string,
                          int replace_start,
                          int replace_length,
                          int cursor_pos);

    QKeyEvent lastKeyEvent() const;
    int keyEventCount() const;
    void sendKeyEvent(const QKeyEvent& event, Maliit::EventRequestType);

    // unused reimpl
    int contentType(bool&) {return 0;}
    bool correctionEnabled(bool&) {return false;}
    bool predictionEnabled(bool&) {return false;}
    bool autoCapitalizationEnabled(bool&) {return false;}
    bool surroundingText(QString&, int&) {return false;}
    bool hasSelection(bool&) {return false;}
    int inputMethodMode(bool&) {return 0;}
    QRect preeditRectangle(bool&) {return QRect();}
    QRect cursorRectangle(bool&) {return QRect();}
    int anchorPosition(bool&) {return 0;}
    QString selection(bool&) {return QString();}
    void sendPreeditString(const QString&, const QList<Maliit::PreeditTextFormat>&, int, int, int) {}
    void notifyImInitiatedHiding() {}
    void copy() {}
    void paste() {}
    void setRedirectKeys(bool) {}
    void setDetectableAutoRepeat(bool) {}
    void setGlobalCorrectionEnabled(bool) {}
    void setInputModeIndicator(Maliit::InputModeIndicator) {}
    void switchPlugin(Maliit::SwitchDirection) {}
    void switchPlugin(const QString&) {}
    void setScreenRegion(const QRegion&) {}
    void setInputMethodArea(const QRegion&) {}
    void setSelection(int, int) {}
    void setOrientationAngleLocked(bool) {}
    QList<MImPluginDescription> pluginDescriptions(Maliit::HandlerState) const {return QList<MImPluginDescription>();}
};

#endif // INPUTMETHODHOSTPROBE_H
