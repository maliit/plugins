/* * This file is part of meego-keyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */

#ifndef POPUPBASE_H
#define POPUPBASE_H

class IKeyButton;
class KeyButtonArea;

class QPointF;
class QPoint;
class QFont;
class QSize;
class QString;


//! \brief Base class for popup implementation
class PopupBase
{
public:
    //! Constructor
    //! \param mainArea Allows PopupBase to forward requests to the main area
    explicit PopupBase(const KeyButtonArea  *mainArea);

    //! \brief Destructor
    virtual ~PopupBase();

    //! \brief Sets the accurate position of finger relative to center of popup text
    virtual void setFingerPos(const QPointF &pos) = 0;

    //! Returns main area
    virtual KeyButtonArea *mainArea() const = 0;

    //! \brief Sets popup position at specified key in according to current orientation
    //! \param keyPos key's position
    //! \param screenPos key's position on the screen
    //! \param keySize  key's size
    virtual void updatePos(const QPointF &keyPos,
                           const QPoint &screenPos,
                           const QSize &keySize) = 0;

    //! \brief Allows PopupBase to act upon invalid key-pressed on the main area
    virtual void handleInvalidKeyPressedOnMainArea() = 0;

    //! \brief Allows PopupBase to act upon key-pressed on the main area
    //! \param keyPos key's position
    //! \param screenPos key's position on the screen
    //! \param keySize  key's size
    virtual void handleKeyPressedOnMainArea(const IKeyButton *key,
                                            const QString &accent,
                                            bool upperCase) = 0;

    //! \brief Allows PopupBase to act upon long key-pressed on the main area
    //! \param keyPos key's position
    //! \param screenPos key's position on the screen
    //! \param keySize  key's size
    virtual void handleLongKeyPressedOnMainArea(const IKeyButton *key,
                                                const QString &accent,
                                                bool upperCase) = 0;

    //! Returns whether PopupBase has any visible components
    virtual bool isVisible() const = 0;

    //! Enables/disables PopupBase completely (affects visibility)
    virtual void setEnabled(bool ok) = 0;
};

#endif

