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

#ifndef MTOOLBARLABEL_H
#define MTOOLBARLABEL_H

#ifdef HAVE_MEEGOTOUCH
#include <MLabel>
#else
#include <QLabel>
#endif

#include <QGraphicsItem>
#include <QSharedPointer>

/*!
 * \class MToolbarLabel
 * \brief MToolbarLabel is provided for the labelss in the input method toolbar.
 *
 * MToolbarLabel is inherit from MLabel. It is used to show text only.
 */
class MToolbarItem;

class MToolbarLabel
#ifdef HAVE_MEEGOTOUCH
    : public MLabel
#else
    : public QLabel
#endif
{
    Q_OBJECT
    Q_DISABLE_COPY(MToolbarLabel)

public:
    /*!
     * \Brief Constructor
     */
    explicit MToolbarLabel(QSharedPointer<MToolbarItem> item, QGraphicsItem *parent = 0);

    //! Destructor
    virtual ~MToolbarLabel();

    //! Return pointer to corresponding toolbar item.
    QSharedPointer<MToolbarItem> item();

signals:
    //! \brief Emitted when visibility (in a sense of the label being available
    //! or not) changes.
    void availabilityChanged();

private slots:
    //! Update label's properties when properties of toolbar item are updated.
    void updateData(const QString &attribute);

private:
    //! MToolbarItem is used as model for this label.
    QSharedPointer<MToolbarItem> itemPtr;
};

#endif

