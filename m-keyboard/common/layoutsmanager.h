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


#ifndef LAYOUTSMANAGER_H
#define LAYOUTSMANAGER_H

#include "keyboarddata.h"
#include "keyboardmapping.h"
#include "layoutdata.h"

#include <mimsettings.h>
#include <QMap>
#include <QObject>
#include <memory>
#include <QFileSystemWatcher>

class LayoutsManager : public QObject
{
    Q_OBJECT

public:
    virtual ~LayoutsManager();

    //! \brief Get singleton instance
    //! \return singleton instance
    static LayoutsManager &instance();

    //! \brief Create singleton
    static void createInstance();

    //! \brief Destroy singleton
    static void destroyInstance();

    //! \brief Returns the count of keyboard layouts.
    int layoutCount() const;

    //! \brief Returns the keyboard layout file name list.
    QStringList layoutFileList() const;

    //! Returns title of a given \a layoutFile as stated in xml.
    QString keyboardTitle(const QString &layoutFile) const;

    //! Returns whether autocaps is enabled for given \a layoutFile.
    bool autoCapsEnabled(const QString &layoutFile) const;

    //! \return keyboard language (as specified in XML) of a given \a layoutFile.
    QString keyboardLanguage(const QString &layoutFile) const;

    //! \brief Get layout model by layoutFile, type, and orientation
    const LayoutData *layout(const QString &layoutFile, LayoutData::LayoutType type,
                             MInputMethod::Orientation orientation) const;

    //! \brief Get layout model specific to current hardware keyboard layout.
    const LayoutData *hardwareLayout(LayoutData::LayoutType type,
                                     MInputMethod::Orientation orientation) const;

    //! \brief Returns currently set default layout file name.
    QString defaultLayoutFile() const;

    //! \brief Returns currently system display language
    QString systemDisplayLanguage() const;

    //! \brief Returns the model for hardware keyboard
    QString xkbModel() const;

    //! \brief Returns current layout for hardware keyboard
    QString xkbLayout() const;

    //! \brief Returns current variant for hardware keyboard
    QString xkbVariant() const;

    //! \brief Returns the primary layout for hardware keyboard
    QString xkbPrimaryLayout() const;

    //! \brief Returns the primary variant for hardware keyboard
    QString xkbPrimaryVariant() const;

    //! \brief Returns the secondary layout for hardware keyboard
    QString xkbSecondaryLayout() const;

    //! \brief Returns the secondary variant for hardware keyboard
    QString xkbSecondaryVariant() const;

    //! \brief Sets \a layout and \a variant as the xkb map for hardware keyboard.
    void setXkbMap(const QString &layout, const QString &variant);

    //! \brief Returns whether autocaps is enabled for hardware keyboard.
    bool hardwareKeyboardAutoCapsEnabled() const;

    /*!
     *\brief Returns the layout file names and their titles for selected keyboards.
     */
    QMap<QString, QString> availableLayouts() const;

    /*!
     *\brief Ensures that at least 1 English keyboard is available.
     *\note If current layouts have covered some "English" keyboard(s), nothing would be
     *      done here. Otherwise "English (UK)" keyboard would be inserted temporarily.
     */
    void ensureEnglishKeyboardAvailable();

    /*!
     *\brief Releases temporary "English (UK)" keyboard if it exists.
     */
    void releaseTemporaryEnglishKeyboard();

signals:
    //! Signals that layouts have been reset and keyboard data can
    //! be reloaded using new layouts returned by layoutList().
    void layoutsChanged();

    //! Signals that number format have been reset and number/phonenumber
    //! keyboard data can be reloaded.
    void numberFormatChanged();

    //! Signals that selected layouts have been changed.
    void selectedLayoutsChanged();

    //! The xkb layout of hardware keyboard has been changed.
    void hardwareLayoutChanged();

private:
    //! Default constructor
    explicit LayoutsManager();

    /*!
     * Returns the layout of the given layout name
     * \param layout layout name such as fi_FI.xml or fi.xml
     * \return true if layout was loaded, false if failure or layout already existed
     */
    bool loadLayout(const QString &layout);

    //! \return keyboard that matches layout list entry /a layoutFile
    const KeyboardData *keyboardByName(const QString &layoutFile) const;

    //! Maps HW Sym variant type to xml file which contains the symbols.
    QString symbolVariantFileName(HardwareSymbolVariant symVariant);

    //! Initialize the xkb keyboard map.
    void initXkbMap();

private slots:
    void syncLayouts();
    void syncHardwareKeyboard();
    void syncNumberKeyboards();
    void layoutsDirectoryChanged();

private:

    //! MImSettings for selected layouts available for vkb's use.
    //! The settings are set by control panel applet.
    MImSettings configLayouts;

    //! Setting that tells the xkb model.
    MImSettings xkbModelSetting;

    //! Current xkb layout.
    QString xkbCurrentLayout;

    //! Current xkb variant.
    QString xkbCurrentVariant;

    //! All keyboards arranged by layout name.
    //! The map key is layout name as read from settings.
    QMap<QString, KeyboardData *> keyboards;

    //! Iterator that points to current layout and keyboard.
    //! Current keyboard is used when no layout name is specified
    //! in calls to public methods of this class.
    QMap<QString, KeyboardData *>::const_iterator current;

    //! Current hardware layout (xkb) specific keyboard.
    KeyboardData hwKeyboard;

    //! Current number keyboard
    KeyboardData numberKeyboard;

    //! Current phone number keyboard
    KeyboardData phoneNumberKeyboard;

    //! System number format in gconf
    MImSettings numberFormatSetting;

    HardwareKeyboardLayout currentHwkbLayoutType;

    // Layouts hacking indicator.
    bool temporaryEnglishKeyboardInserted;

    //! All available layouts as (filename, title)
    mutable QMap<QString, QString> mAvailableLayouts;

    //! Watcher for available layouts
    QFileSystemWatcher layoutsDirectoryWatcher;

    //! Singleton instance
    static LayoutsManager *Instance;

    friend class Ut_LayoutsManager;
};


inline LayoutsManager &LayoutsManager::instance()
{
    Q_ASSERT(Instance);
    return *Instance;
}


#endif
