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

#ifndef MTOOLBARBUTTON_H
#define MTOOLBARBUTTON_H

#ifdef HAVE_MEEGOTOUCH
#include <MButton>
#else
#include <QGraphicsWidget>
#endif

#include <QSharedPointer>

/*!
 * \class MToolbarWidget
 * \brief MToolbarWidget is provide for the buttons in the input method toolbar.
 *
 * MToolbarWidget inherits from MButton. It can not only use the iconID, but also use the icon
 * which is not in current theme, by setIcon() with the absolute file name of the icon, and the
 * icon will be scaled according setIconPercent() and button size.
 */
class QPixmap;
class MToolbarItem;

class MToolbarButton
#ifdef HAVE_MEEGOTOUCH
    : public MButton
#else
    : public QGraphicsWidget
#endif
{
    Q_OBJECT
    Q_DISABLE_COPY(MToolbarButton)

public:
    /*!
     * \Brief Constructor
     */
    explicit MToolbarButton(QSharedPointer<MToolbarItem> item, QGraphicsItem *parent = 0);

    //! Destructor
    virtual ~MToolbarButton();

    /*!
     * \Brief Sets the icon which is to be displayed on the button.
     * \param iconFile The absolute file name of the icon.
     */
    void setIconFile(const QString &iconFile);

    /*!
     * \Brief Sets the percentage of the icon size.
     * \param percent It is used to scale the icon, limit it inside button.
     *
     * This method scales the icon to limit it inside container(button). The scale is
     * relative to the container. After scaling, the icon is scaled to occupy \a percent
     * percentage of container's size. but its origin ratio is still kept.
     */
    void setIconPercent(int percent);

    //! \reimp
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                       QWidget *widget = 0);
    //! \reimp_end

    //! Return pointer to corresponding toolbar item.
    QSharedPointer<MToolbarItem> item();

    //! Select style name depending on item attributes.
    void updateStyleName();

    //! Set the text of the button and updates the preferred size internally
    void setText(const QString &text);

signals:
    /*!
     * \brief Emitted when button is clicked.
     * \param item Pointer to corresponding toolbar item.
     *
     * Warning: do not store pointer which is used as parameter for this signal,
     * call MToolbarItem::item() if you need to get pointer to toolbar item.
     */
    void clicked(MToolbarItem *item);

    //! \brief Emitted when visibility (in a sense of the button being available
    //! or not) changes.
    void availabilityChanged();

private slots:
    //! Update button's properties when properties of toolbar item are updated.
    void updateData(const QString &attribute);

    //! Emits clicked(MToolbarItem *) when base class emits clicked(bool)
    void onClick();

private:
    QPixmap *icon;
    QString iconFile;
    int sizePercent;
    QSharedPointer<MToolbarItem> itemPtr;
    QSizeF originalMinSize;

    friend class Ut_MImToolbar;
};

#endif
