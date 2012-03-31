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

#ifndef MALIIT_KEYBOARD_LAYOUTUPDATER_H
#define MALIIT_KEYBOARD_LAYOUTUPDATER_H

#include "keyboardloader.h"

#include "models/key.h"
#include "models/wordcandidate.h"
#include "models/layout.h"

#include <QtCore>

namespace MaliitKeyboard {

class LayoutUpdaterPrivate;

class LayoutUpdater
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(LayoutUpdater)
    Q_DECLARE_PRIVATE(LayoutUpdater)

public:
    explicit LayoutUpdater(QObject *parent = 0);
    virtual ~LayoutUpdater();

    void init();

    QStringList keyboardIds() const;
    QString activeKeyboardId() const;
    void setActiveKeyboardId(const QString &id);
    QString keyboardTitle(const QString &id) const;

    void setLayout(const SharedLayout &layout);
    Q_SLOT void setOrientation(Layout::Orientation orientation);

    // Key signal handlers:
    Q_SLOT void onKeyPressed(const Key &key,
                             const SharedLayout &layout);
    Q_SLOT void onKeyLongPressed(const Key &key,
                                 const SharedLayout &layout);
    Q_SLOT void onKeyReleased(const Key &key,
                              const SharedLayout &layout);
    Q_SLOT void onKeyEntered(const Key &key,
                             const SharedLayout &layout);
    Q_SLOT void onKeyExited(const Key &key,
                            const SharedLayout &layout);
    Q_SLOT void clearActiveKeysAndMagnifier();
    Q_SLOT void onCandidatesUpdated(const QStringList &candidates);

    // WordCandidate signal handlers:
    Q_SLOT void onWordCandidatePressed(const WordCandidate &candidate,
                                       const SharedLayout &layout);
    Q_SLOT void onWordCandidateReleased(const WordCandidate &candidate,
                                        const SharedLayout &layout);

    Q_SIGNAL void layoutChanged(const SharedLayout &layout);
    Q_SIGNAL void keysChanged(const SharedLayout &layout);
    Q_SIGNAL void wordCandidatesChanged(const SharedLayout &layout);
    Q_SIGNAL void wordCandidateSelected(const QString &candidate);

private:
    Q_SIGNAL void shiftPressed();
    Q_SIGNAL void shiftReleased();
    Q_SIGNAL void autoCapsActivated();
    Q_SIGNAL void shiftCancelled();

    Q_SLOT void syncLayoutToView();
    Q_SLOT void onKeyboardsChanged();

    Q_SIGNAL void symKeyReleased();
    Q_SIGNAL void symSwitcherReleased();

    Q_SLOT void switchToMainView();
    Q_SLOT void switchToPrimarySymView();
    Q_SLOT void switchToSecondarySymView();

    Q_SIGNAL void deadkeyPressed();
    Q_SIGNAL void deadkeyReleased();
    Q_SIGNAL void deadkeyCancelled();

    Q_SLOT void switchToAccentedView();

    const QScopedPointer<LayoutUpdaterPrivate> d_ptr;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_LAYOUTUPDATER_H
