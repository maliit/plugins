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

#include "mtoolbarbuttonview.h"
#include "mtoolbarbutton.h"

MToolbarButtonView::MToolbarButtonView(MToolbarButton *controller)
#ifdef HAVE_MEEGOTOUCH
    : MButtonView(controller)
#endif
{
}


QSizeF MToolbarButtonView::optimalSize(const QSizeF& maxSize)
{
#ifndef HAVE_MEEGOTOUCH
    return maxSize;
#else
    // Let the MButtonView implementation to calculate the right preferred size
    QSizeF preferredSize = sizeHint(Qt::PreferredSize);

    // The calculation of the MButtonView::sizeHint() does not take into account
    // the left and right margins (only left and right text margins are used). Let's
    // add it here.
    preferredSize.setWidth(preferredSize.width()+marginLeft()+marginRight());
    // Use the height from the maximal size, because it is correct to get from the style
    preferredSize.setHeight(maxSize.height());
    // We can not be bigger than the maximum size
    if (preferredSize.width() > maxSize.width()){
        return maxSize;
    }
    return preferredSize;
#endif
}
