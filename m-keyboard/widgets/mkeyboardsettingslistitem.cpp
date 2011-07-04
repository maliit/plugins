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

#ifdef HAVE_MEEGOTOUCH
#include "mkeyboardsettingslistitem.h"

#include <MLabel>
#include <MImageWidget>
#include <QGraphicsGridLayout>
#include <QDebug>

MKeyboardSettingsListItem::MKeyboardSettingsListItem(MBasicListItem::ItemStyle style,
                                                     QGraphicsItem *parent)
    : MBasicListItem(style, parent), layout(NULL), stretchItem(NULL)
{
}

QGraphicsLayout *MKeyboardSettingsListItem::createLayout()
{
    if (!layout) {
        layout = new QGraphicsGridLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);
        stretchItem = new QGraphicsWidget(this);
    } else {
        clearLayout();
    }

    switch (itemStyle()) {
        case IconWithTitleAndSubtitle:
            layout->addItem(titleLabelWidget(), 0, 0);
            layout->addItem(subtitleLabelWidget(), 1, 0);
            layout->addItem(stretchItem, 2, 0);
            layout->addItem(imageWidget(), 0, 1, 3, 1, Qt::AlignCenter);
            break;

        default:
            // Not supported
            qWarning() << __PRETTY_FUNCTION__ << "Unsupported item style";
            break;
    }

    return layout;
}

void MKeyboardSettingsListItem::clearLayout()
{
    for (int i = layout->count() - 1; i >= 0; i--) {
        layout->removeAt(i);
    }
}

#endif // HAVE_MEEGOTOUCH
