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

#ifndef GETCSSPROPERTY_H
#define GETCSSPROPERTY_H

#include "style-wrapper.h"
#include <QDebug>

/*!
 * \brief Helper method to get styling property.
 * T should be the real type of retrieved property. Class T should have default
 * constructor and assignment operator.
 * \param container Style container.
 * \param propertyName Property name.
 * \param rtl Contains true if we need to get RTL version of specified property.
 * \param defVal This value will be returned if specified property does not exist.
 * Default value for this parameter is T().
 * \return Returns value of requested property or \a defVal if \a propertyName was not found
 * or \a propertyName is empty.
 */
template <class T>
T getCSSProperty(const MWidgetStyleContainer &container,
                    QString propertyName,
                    const bool rtl,
                    const T &defVal = T())
{
    static const QString RtlSuffix = QString::fromLatin1("Rtl");
    QVariant result(QVariant::Invalid);
    T value(defVal);

    if (!propertyName.isEmpty()) {
        if (rtl) {
            propertyName.append(RtlSuffix);
        }
        QByteArray ba = propertyName.toLatin1();
        result = container->property(ba.data());
    }

    if (result.isValid()) {
       value  = result.value<T>();
    } else if (!propertyName.isEmpty()) {
        qWarning() << __PRETTY_FUNCTION__ << "Property" << propertyName
                   << "does not exist in" << container.objectName();
    }

    return value;
}

#endif

