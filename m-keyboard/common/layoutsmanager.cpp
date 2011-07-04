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



#include "layoutsmanager.h"
#include <algorithm>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "mimlayouttitleparser.h"

namespace
{
    const QString EnabledLayouts("/meegotouch/inputmethods/onscreen/enabled");
    const QString InputMethodDefaultLayout("/meegotouch/inputmethods/virtualkeyboard/layouts/default");
    const QString XkbLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/layout");
    const QString XkbVariantSettingName("/meegotouch/inputmethods/hwkeyboard/variant");
    const QString XkbSecondaryLayoutSettingName("/meegotouch/inputmethods/hwkeyboard/secondarylayout");
    const QString XkbModelSettingName("/meegotouch/inputmethods/hwkeyboard/model");
    const QString XkbSecondaryVariantSettingName("/meegotouch/inputmethods/hwkeyboard/secondaryvariant");
    const QString HardwareKeyboardAutoCapsDisabledLayouts("/meegotouch/inputmethods/hwkeyboard/autocapsdisabledlayouts");
    const QString DefaultHardwareKeyboardAutoCapsDisabledLayout("ara"); // Uses xkb layout name. Arabic is "ara".
    const QString SystemDisplayLanguage("/meegotouch/i18n/language");
    const QString FallbackLayout("en_gb.xml");
    const QString FallbackXkbLayout("us");
    const QString NumberFormatSettingName("/meegotouch/i18n/lc_numeric");
    const QString NumberKeyboardFileArabic("number_ar.xml");
    const QString NumberKeyboardFileLatin("number.xml");
    const QString PhoneNumberKeyboardFileArabic("phonenumber_ar.xml");
    const QString PhoneNumberKeyboardFileLatin("phonenumber.xml");
    const QString PhoneNumberKeyboardFileRussian("phonenumber_ru.xml");
    const QString PhoneNumberKeyboardFileHebrew("phonenumber_he.xml");
    const QString SymbolKeyboardFileCommon("hwsymbols_common.xml");
    const QString SymbolKeyboardFileChinese("hwsymbols_chinese.xml");
    const QString FallbackXkbModel("evdev");
    const QString VKBConfigurationPath("/usr/share/meegotouch/virtual-keyboard/layouts/");
    const QLatin1String VKBUserLayoutPath(".config/meego-keyboard/layouts/");
    const QString VKBLayoutsFilterRule("*.xml");
    const QString VKBLayoutsIgnoreRules("number|test|customer|default"); // use as regexp to ignore number, test, customer and default layouts
    const QLatin1String KeyboardId("libmeego-keyboard.so");

    QStringList fromEnabledLayoutsSettings(const QStringList& list)
    {
        QStringList layouts;

        QString first;
        unsigned int i = 0;
        foreach (const QString &value, list) {
            if (i % 2 == 0)
                first = value;
            else if (first == KeyboardId)
                layouts.push_back(value);
            i++;
        }

        return layouts;
    }

    const QString EnglishLanguagePrefix("en");
}

LayoutsManager *LayoutsManager::Instance = 0;


// FIXME: Style of MImAbstractKeyAreas should not be needed in classes that deal with data models.
LayoutsManager::LayoutsManager()
    : configLayouts(EnabledLayouts),
      xkbModelSetting(XkbModelSettingName),
      numberFormatSetting(NumberFormatSettingName),
      currentHwkbLayoutType(InvalidHardwareKeyboard),
      temporaryEnglishKeyboardInserted(false),
      mAvailableLayouts(),
      layoutsDirectoryWatcher()
{
    // Read settings for the first time and load keyboard layouts.
    syncLayouts();
    initXkbMap();
    syncHardwareKeyboard();
    syncNumberKeyboards();

    layoutsDirectoryWatcher.addPath(VKBConfigurationPath);
    layoutsDirectoryWatcher.addPath(QFileInfo(QDir::home(), VKBUserLayoutPath).filePath());

    // Synchronize with settings when someone changes them (e.g. via control panel).
    connect(&configLayouts, SIGNAL(valueChanged()), this, SLOT(syncLayouts()));
    connect(&configLayouts, SIGNAL(valueChanged()), this, SIGNAL(selectedLayoutsChanged()));
    connect(&numberFormatSetting, SIGNAL(valueChanged()), SLOT(syncNumberKeyboards()));
    connect(&layoutsDirectoryWatcher, SIGNAL(directoryChanged(QString)), this, SLOT(layoutsDirectoryChanged()));
}

LayoutsManager::~LayoutsManager()
{
    qDeleteAll(keyboards);
    keyboards.clear();
}

void LayoutsManager::createInstance()
{
    Q_ASSERT(!Instance);
    if (!Instance) {
        Instance = new LayoutsManager;
    }
}

void LayoutsManager::destroyInstance()
{
    Q_ASSERT(Instance);
    delete Instance;
    Instance = 0;
}

int LayoutsManager::layoutCount() const
{
    return keyboards.count();
}

QStringList LayoutsManager::layoutFileList() const
{
    // This will return layout files in alphabetical ascending order.
    // This means that order in gconf is ignored.
    QStringList layoutFiles = keyboards.keys();
    layoutFiles.sort();
    return layoutFiles;
}

const KeyboardData *LayoutsManager::keyboardByName(const QString &layoutFile) const
{
    QMap<QString, KeyboardData *>::const_iterator kbIter = keyboards.find(layoutFile);

    const KeyboardData *keyboard = NULL;
    if (kbIter != keyboards.end())
        keyboard = *kbIter;

    return keyboard;
}

QString LayoutsManager::keyboardTitle(const QString &layoutFile) const
{
    const KeyboardData *const keyboard = keyboardByName(layoutFile);
    return (keyboard ? keyboard->title() : "");
}

bool LayoutsManager::autoCapsEnabled(const QString &layoutFile) const
{
    const KeyboardData *const keyboard = keyboardByName(layoutFile);
    return (keyboard ? keyboard->autoCapsEnabled() : false);
}

QString LayoutsManager::keyboardLanguage(const QString &layoutFile) const
{
    const KeyboardData *const keyboard = keyboardByName(layoutFile);
    return (keyboard ? keyboard->language() : "");
}

const LayoutData *LayoutsManager::layout(const QString &layoutFile,
        LayoutData::LayoutType type,
        M::Orientation orientation) const
{
    const LayoutData *lm = 0;

    if (type == LayoutData::Number) {
        lm = numberKeyboard.layout(type, orientation);
    } else if (type == LayoutData::PhoneNumber) {
        lm = phoneNumberKeyboard.layout(type, orientation);
    } else {
        QMap<QString, KeyboardData *>::const_iterator kbIter = keyboards.find(layoutFile);

        if (kbIter != keyboards.end() && *kbIter) {
            lm = (*kbIter)->layout(type, orientation);
        }
    }

    if (!lm) {
        static const LayoutData empty;
        lm = &empty;
    }

    return lm;
}

const LayoutData *LayoutsManager::hardwareLayout(LayoutData::LayoutType type,
                                                 M::Orientation orientation) const
{
    return hwKeyboard.layout(type, orientation);
}

QString LayoutsManager::defaultLayoutFile() const
{
    return MImSettings(InputMethodDefaultLayout).value(FallbackLayout).toString();
}

QString LayoutsManager::systemDisplayLanguage() const
{
    return MImSettings(SystemDisplayLanguage).value().toString();
}

void LayoutsManager::initXkbMap()
{
    //init current xkb layout and variant.
    setXkbMap(xkbPrimaryLayout(), xkbPrimaryVariant());
}

QString LayoutsManager::xkbModel() const
{
    return xkbModelSetting.value(FallbackXkbModel).toString();
}

QString LayoutsManager::xkbLayout() const
{
    return xkbCurrentLayout;
}

QString LayoutsManager::xkbVariant() const
{
    return xkbCurrentVariant;
}

QString LayoutsManager::xkbPrimaryLayout() const
{
    return MImSettings(XkbLayoutSettingName).value(FallbackXkbLayout).toString();
}

QString LayoutsManager::xkbPrimaryVariant() const
{
    return MImSettings(XkbVariantSettingName).value().toString();
}

QString LayoutsManager::xkbSecondaryLayout() const
{
    return MImSettings(XkbSecondaryLayoutSettingName).value().toString();
}

QString LayoutsManager::xkbSecondaryVariant() const
{
    return MImSettings(XkbSecondaryVariantSettingName).value().toString();
}

void LayoutsManager::setXkbMap(const QString &layout, const QString &variant)
{
    bool changed = false;
    if (layout != xkbCurrentLayout) {
        changed = true;
        xkbCurrentLayout = layout;
    }

    if (variant != xkbCurrentVariant) {
        changed = true;
        xkbCurrentVariant = variant;
    }

    if (changed) {
        syncHardwareKeyboard();
    }
}

bool LayoutsManager::hardwareKeyboardAutoCapsEnabled() const
{
    // Arabic hwkb layout default disable autocaps.
    QStringList autoCapsDisabledLayouts = MImSettings(HardwareKeyboardAutoCapsDisabledLayouts)
                                            .value(QStringList(DefaultHardwareKeyboardAutoCapsDisabledLayout))
                                            .toStringList();
    return !(autoCapsDisabledLayouts.contains(xkbLayout()));
}

bool LayoutsManager::loadLayout(const QString &layout)
{
    QMap<QString, KeyboardData *>::iterator kbIterator;

    // sanity tests
    if (layout.isEmpty())
        return false;

    KeyboardData *keyboard = new KeyboardData;

    bool loaded = keyboard->loadNokiaKeyboard(layout);

    if (!loaded) {
        loaded = keyboard->loadNokiaKeyboard(layout);
    }

    if (!loaded) {
        delete keyboard;
        keyboard = NULL;
        return false;
    }

    // Make sure entry for layout exists. Create if it doesn't.
    kbIterator = keyboards.find(layout);
    if (kbIterator == keyboards.end()) {
        kbIterator = keyboards.insert(layout, 0);
    }

    if (*kbIterator) {
        // What to do?
        qWarning() << "LayoutsManager: Layouts have already been loaded for layout "
                   << keyboard->layoutFile();
        delete keyboard;
        keyboard = NULL;
        return false;
    }

    *kbIterator = keyboard;

    return true;
}

void LayoutsManager::syncNumberKeyboards()
{
    QString numberFormat = numberFormatSetting.value().toString().section("_", 0, 0);
    bool loaded = false;

    // Load the proper number layout
    if (numberFormat == "ar") {
        loaded = numberKeyboard.loadNokiaKeyboard(NumberKeyboardFileArabic);
    }
    // In other cases and fallback
    if (!loaded)
    {
        numberKeyboard.loadNokiaKeyboard(NumberKeyboardFileLatin);
    }

    // Load the proper phone number layout
    loaded = false;
    if (numberFormat == "ar") {
        loaded = phoneNumberKeyboard.loadNokiaKeyboard(PhoneNumberKeyboardFileArabic);
    } else if (numberFormat == "ru") {
        loaded = phoneNumberKeyboard.loadNokiaKeyboard(PhoneNumberKeyboardFileRussian);
    } else if (numberFormat == "he") {
        loaded = phoneNumberKeyboard.loadNokiaKeyboard(PhoneNumberKeyboardFileHebrew);
    }

    if (!loaded) {
        phoneNumberKeyboard.loadNokiaKeyboard(PhoneNumberKeyboardFileLatin);
    }
    emit numberFormatChanged();
}

void LayoutsManager::syncLayouts()
{
    bool changed = false;
    QStringList newLayouts;
    const QStringList oldLayouts = layoutFileList();

    // Add new ones
    if (!configLayouts.value().isNull()) {
        newLayouts = fromEnabledLayoutsSettings(configLayouts.value().toStringList());

        foreach(QString layoutFile, newLayouts) {
            // Existing layouts are not reloaded.
            if (!oldLayouts.contains(layoutFile, Qt::CaseInsensitive)) {
                // Add new layout
                if (!loadLayout(layoutFile)) {
                    qWarning() << __PRETTY_FUNCTION__
                               << "New layout file " << layoutFile << " could not be loaded.";
                } else {
                    changed = true;
                }
            }
        }
    }

    // Remove old ones
    foreach(QString old, oldLayouts) {
        if (!newLayouts.contains(old, Qt::CaseInsensitive)) {
            keyboards.remove(old);
            changed = true;
        }
    }

    // Try FallbackLayout if no layouts loaded.
    // Don't try to load again if we already tried.
    if (keyboards.isEmpty() && !newLayouts.contains(FallbackLayout)) {
        if (loadLayout(FallbackLayout)) {
            changed = true;
        }
    }

    // Cancel "Containing temporary English (UK) layout" indicator
    // when "English (UK)" layout is synchronizedinto current layouts.
    if (layoutFileList().contains(FallbackLayout))
        temporaryEnglishKeyboardInserted = false;

    if (changed) {
        emit layoutsChanged();
    }
}

void LayoutsManager::syncHardwareKeyboard()
{
    const QString xkbName = xkbLayout();
    const HardwareKeyboardLayout hwkbLayoutType = xkbLayoutType(xkbName);

    if (hwkbLayoutType == currentHwkbLayoutType) {
        return;
    }

    currentHwkbLayoutType = hwkbLayoutType;

    // What we could do here is to load a generic hw layout xml file
    // that would import the correct symbol layout variant but since
    // symbol sections are the only things we currently use, let's just
    // load the hw symbols xml directly.
    const HardwareSymbolVariant symVariant = HwkbLayoutToSymVariant[hwkbLayoutType];
    const QString filename = symbolVariantFileName(symVariant);

    if (hwKeyboard.loadNokiaKeyboard(filename)) {
        emit hardwareLayoutChanged();
    } else {
        qWarning() << "LayoutsManager: loading of hardware layout specific keyboard "
                   << filename << " failed";
    }
}

QString LayoutsManager::symbolVariantFileName(HardwareSymbolVariant symVariant)
{
    QString symFileName;

    switch (symVariant) {
    case HwSymbolVariantChinese:
        symFileName = SymbolKeyboardFileChinese;
        break;
    case HwSymbolVariantCommon:
    default:
        symFileName = SymbolKeyboardFileCommon;
        break;
    }

    return symFileName;
}

QMap<QString, QString> LayoutsManager::availableLayouts() const
{
    if (mAvailableLayouts.empty()) {
        QList<QDir> dirs;
        dirs << QDir(VKBConfigurationPath, VKBLayoutsFilterRule);
        QFileInfo userLayoutsDirectory(QDir::home(), VKBUserLayoutPath);
        dirs << QDir(userLayoutsDirectory.filePath(), VKBLayoutsFilterRule);
        QRegExp ignoreExp(VKBLayoutsIgnoreRules, Qt::CaseInsensitive);

        foreach (const QDir &dir, dirs) {
            // available keyboard layouts are determined by xml layouts that can be found
            foreach (const QFileInfo &keyboardFileInfo, dir.entryInfoList()) {
                if (keyboardFileInfo.fileName().contains(ignoreExp))
                    continue;

                QFile layoutFile(keyboardFileInfo.absoluteFilePath());
                if (!layoutFile.open(QIODevice::ReadOnly | QIODevice::Text))
                    continue;
                MImLayoutTitleParser titleParser(&layoutFile);
                if (!titleParser.parse()) {
                    qDebug() << "Error parsing file: " << layoutFile.fileName() << "Error: " << titleParser.errorString();
                    continue;
                }

                if (titleParser.keyboardTitle().isEmpty())
                    continue;

                if (mAvailableLayouts.contains(keyboardFileInfo.fileName())) {
                    continue;
                }

                mAvailableLayouts.insert(keyboardFileInfo.fileName(), titleParser.keyboardTitle());
            }
        }
    }
    return mAvailableLayouts;
}

void LayoutsManager::layoutsDirectoryChanged()
{
    mAvailableLayouts.clear();
}

void LayoutsManager::ensureEnglishKeyboardAvailable()
{
    // If temporary "English (UK)" keyboard has been insterted, do nothing here.
    if (temporaryEnglishKeyboardInserted)
        return;

    // Check whether current layouts have included some English layout.
    foreach (const KeyboardData * const kbData, keyboards.values()) {
        if (kbData->language().startsWith(EnglishLanguagePrefix)) {
            // Here means that some English layout exists in current layouts.
            // It might be "English (UK)" or "English (US)", or whatever other English layout.
            // Anyway, we needn't add temporary "English (UK)" layout here.
            return;
        }
    }

    // Here means that no any English layout is available in current layouts.
    // So insert "English (UK)" layout temporarily into current layouts.
    if (loadLayout(FallbackLayout)) {
        temporaryEnglishKeyboardInserted = true;
        emit layoutsChanged();
    }
}

void LayoutsManager::releaseTemporaryEnglishKeyboard()
{
    // If no temporary "English (UK)" layout exists, do nothing here.
    if (!temporaryEnglishKeyboardInserted)
        return;

    // Remove temporary "English (UK)" layout and declare that layouts have been changed.
    keyboards.remove(FallbackLayout);
    temporaryEnglishKeyboardInserted = false;
    emit layoutsChanged();
}
