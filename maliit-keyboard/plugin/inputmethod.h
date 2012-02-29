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

#ifndef MALIIT_KEYBOARD_INPUTMETHOD_H
#define MALIIT_KEYBOARD_INPUTMETHOD_H

#include <mabstractinputmethod.h>
#include <mabstractinputmethodhost.h>
#include <QtGui>

namespace MaliitKeyboard {

class InputMethodPrivate;

class InputMethod
    : public MAbstractInputMethod
{
    Q_OBJECT
    Q_DISABLE_COPY(InputMethod)
    Q_DECLARE_PRIVATE(InputMethod)

public:
    explicit InputMethod(MAbstractInputMethodHost *host,
                         QWidget *window);

    virtual ~InputMethod();

    //! \reimp
    virtual void show();
    Q_SLOT virtual void hide();
    virtual void switchContext(MInputMethod::SwitchDirection direction,
                               bool animated);
    virtual QList<MAbstractInputMethod::MInputMethodSubView>
    subViews(MInputMethod::HandlerState state = MInputMethod::OnScreen) const;
    virtual void setActiveSubView(const QString &id,
                                  MInputMethod::HandlerState state = MInputMethod::OnScreen);
    virtual QString activeSubView(MInputMethod::HandlerState state = MInputMethod::OnScreen) const;
    virtual void handleAppOrientationChanged(int angle);
    //! \reimp_end

private:
    Q_SLOT void onSwitchLeft();
    Q_SLOT void onSwitchRight();

    const QScopedPointer<InputMethodPrivate> d_ptr;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_INPUTMETHOD_H
