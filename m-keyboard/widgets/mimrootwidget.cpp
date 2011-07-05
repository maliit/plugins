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

#include "mimrootwidget.h"

class MImRootWidgetPrivate
{
public:
    Q_DECLARE_PUBLIC(MImRootWidget)
    MImRootWidget * const q_ptr;
    MInputMethod::OrientationAngle angle;

    explicit MImRootWidgetPrivate(MImRootWidget *q)
        : q_ptr(q)
        , angle(MInputMethod::Angle0)
    {}
};

MImRootWidget::MImRootWidget(QGraphicsItem *parent)
    : QGraphicsWidget(parent)
    , d_ptr(new MImRootWidgetPrivate(this))
{
    if (QDesktopWidget *w = QApplication::desktop()) {
        resize(w->screenGeometry().size());
    } else {
        qWarning() << __PRETTY_FUNCTION__
                   << "Couldn't find QDesktopWidget - unable to resize to default geometry.";
    }
}

MImRootWidget::~MImRootWidget()
{
    delete d_ptr;
}

MInputMethod::OrientationAngle MImRootWidget::orientationAngle() const
{
    Q_D(const MImRootWidget);
    return d->angle;
}

void MImRootWidget::setOrientationAngle(MInputMethod::OrientationAngle angle)
{
    Q_D(MImRootWidget);
    if (d->angle != angle) {
        d->angle = angle;

        // TODO: check transformation center of QGraphicsWidget's. Do we need
        // to adjust widget after rotation + resize?
        setRotation(angle);
        if (QDesktopWidget *w = QApplication::desktop()) {
            switch(angle) {
            case MInputMethod::Angle0:
            case MInputMethod::Angle180:
                resize(w->screenGeometry().size());
                break;

            case MInputMethod::Angle90:
            case MInputMethod::Angle270:
                resize(w->screenGeometry().height(), w->screenGeometry().width());
                break;
            }
        } else {
            qWarning() << __PRETTY_FUNCTION__
                       << "Couldn't find QDesktopWidget - unable to resize after changing orientation angle to"
                       << angle;
        }

        emit orientationAngleChanged(d->angle);
    }
}

MInputMethod::Orientation MImRootWidget::orientation() const
{
    Q_D(const MImRootWidget);

    switch(d->angle) {
    case MInputMethod::Angle0:
    case MInputMethod::Angle180:
        return MInputMethod::Landscape;

    case MInputMethod::Angle90:
    case MInputMethod::Angle270:
        return MInputMethod::Portrait;
    }

    // Should never reach:
    return MInputMethod::Landscape;
}
