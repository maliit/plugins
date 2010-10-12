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



#include "keyboarddata.h"

#include <QDebug>
#include <QDomDocument>
#include <QFile>


namespace
{
    const QString VKBConfigurationPath = "/usr/share/meegotouch/virtual-keyboard/layouts/";

    const QString VKBTagKeyboard             = QString("keyboard");
    const QString VKBTagVersion              = QString("version");
    const QString VKBTagCatalog              = QString("catalog");
    const QString VKBTagAutoCapitalization   = QString("autocapitalization");
    const QString VKBTagLayout               = QString("layout");
    const QString VKBTagTitle                = QString("title");
    const QString VKBTagLanguage             = QString("language");
    const QString VKBTagBoolTrue             = QString("true");
    const QString VKBTagBoolFalse            = QString("false");
    const QString VKBTagType                 = QString("type");
    const QString VKBTagTypeGeneral          = QString("general");
    const QString VKBTagTypeUrl              = QString("url");
    const QString VKBTagTypeEmail            = QString("email");
    const QString VKBTagTypeNumber           = QString("number");
    const QString VKBTagTypePhoneNumber      = QString("phonenumber");
    const QString VKBTagTypeCommon           = QString("common");
    const QString VKBTagOrientation          = QString("orientation");
    const QString VKBTagOrientationLandscape = QString("landscape");
    const QString VKBTagOrientationPortrait  = QString("portrait");

    const QString VKBTagID                   = QString("id");
    const QString VKBTagWidth                = QString("width");
    const QString VKBTagHeight               = QString("height");

    const QString VKBTagShift                = QString("shift");
    const QString VKBTagLabel                = QString("label");
    const QString VKBTagTypeNonsloppy        = QString("non-sloppy");
    const QString VKBTagHorizontalAlignment  = QString("horizontal_alignment");
    const QString VKBTagVerticalAlignment    = QString("vertical_alignment");
    const QString VKBTagAlignFull            = QString("full");
    const QString VKBTagAlignLeft            = QString("left");
    const QString VKBTagAlignRight           = QString("right");
    const QString VKBTagAlignBottom          = QString("bottom");
    const QString VKBTagAlignTop             = QString("top");
    const QString VKBTagAlignCenter          = QString("center");

    const QString VKBTagRow                  = QString("row");
    const QString VKBTagSection              = QString("section");
    const QString VKBTagMovable              = QString("movable");

    const QString VKBTagBinding              = QString("binding");
    const QString VKBTagKey                  = QString("key");
    const QString VKBTagDead                 = QString("dead");
    const QString VKBTagSpacer               = QString("spacer");

    const QString VKBTagAccents              = QString("accents");
    const QString VKBTagAccentedLabels       = QString("accented_labels");

    const QString VKBTagExtendedLabels       = QString("extended_labels");

    const QString VKBTagKeyAction            = QString("action");
    const QString VKBTagCycleSet             = QString("cycleset");
    const QString VKBTagSecondaryLabel       = QString("secondary_label");

    const QString ActionStrInsert            = QString("insert");
    const QString ActionStrShift             = QString("shift");
    const QString ActionStrBackspace         = QString("backspace");
    const QString ActionStrSpace             = QString("space");
    const QString ActionStrCycle             = QString("cycle");
    const QString ActionStrLayoutMenu        = QString("layout_menu");
    const QString ActionStrSym               = QString("sym");
    const QString ActionStrReturn            = QString("return");
    const QString ActionStrDecimalSeparator  = QString("decimal_separator");
    const QString ActionStrPlusMinusToggle   = QString("plus_minus_toggle");
    const QString ActionStrTab               = QString("tab");
    const QString ActionStrCommit            = QString("commit");
    const QString ActionStrSwitch            = QString("switch");

    const QString VKBTagImport               = QString("import");
    const QString VKBTagFile                 = QString("file");

    const QString RtlString                  = QString("rtl");
    const QString RtlStringDefValue          = QString("false");

    const QString StyleString                = QString("style");
    const QString StyleStringDefValue        = QString("normal");
    const QString WidthTypeString            = QString("width");
    const QString WidthTypeStringDefValue    = QString("medium");
    const QString FixedString                = QString("fixed");
    const QString FixedStringDefValue        = QString("false");
    const QString HeightTypeString           = QString("height");
    const QString HeightTypeStringDefValue   = QString("medium");

}

struct ParseParameters {
    VKBDataKey *currentKey;

    //! New rows will be added to currentSection
    LayoutData::SharedLayoutSection currentSection;

    //! New keys will be added to currentRow
    LayoutSection::Row *currentRow;

    //! Contains true if current XML tag was successfully parsed
    bool validTag;

    const QString *fileName;

    ParseParameters();
};

ParseParameters::ParseParameters():
    currentKey(0),
    currentSection(0),
    currentRow(0),
    validTag(true),
    fileName(0)
{
}

KeyboardData::KeyboardData()
    : currentLayout(0),
      keyboardVersion(""),
      keyboardTitle(""),
      keyboardLanguage(""),
      keyboardCatalog(""),
      keyboardAutoCapsEnabled(true)
{
    layoutTypeMap[VKBTagTypeGeneral] = LayoutData::General;
    layoutTypeMap[VKBTagTypeUrl] = LayoutData::Url;
    layoutTypeMap[VKBTagTypeEmail] = LayoutData::Email;
    layoutTypeMap[VKBTagTypeNumber] = LayoutData::Number;
    layoutTypeMap[VKBTagTypePhoneNumber] = LayoutData::PhoneNumber;
    layoutTypeMap[VKBTagTypeCommon] = LayoutData::Common;
}

KeyboardData::~KeyboardData()
{
    qDeleteAll(layouts);
    layouts.clear();
}

inline VKBDataKey::StyleType KeyboardData::toStyleType(const QString &attributeValue)
{
    VKBDataKey::StyleType type = VKBDataKey::NormalStyle;

    if (attributeValue == "special") {
        type = VKBDataKey::SpecialStyle;
    } else if (attributeValue == "deadkey") {
        type = VKBDataKey::DeadkeyStyle;
    }

    return type;
}

inline VKBDataKey::WidthType KeyboardData::toWidthType(const QString &attributeValue)
{
    VKBDataKey::WidthType widthType = VKBDataKey::Medium;

    if (attributeValue == "small") {
        widthType = VKBDataKey::Small;
    } else  if (attributeValue == "large") {
        widthType = VKBDataKey::Large;
    } else  if (attributeValue == "x-large") {
        widthType = VKBDataKey::XLarge;
    } else  if (attributeValue == "xx-large") {
        widthType = VKBDataKey::XxLarge;
    } else  if (attributeValue == "stretched") {
        widthType = VKBDataKey::Stretched;
    }

    return widthType;
}

inline LayoutSection::RowHeightType KeyboardData::toHeightType(const QString &attributeValue)
{
    LayoutSection::RowHeightType heightType = LayoutSection::Medium;

    if (attributeValue == "small") {
        heightType = LayoutSection::Small;
    } else  if (attributeValue == "large") {
        heightType = LayoutSection::Large;
    } else  if (attributeValue == "x-large") {
        heightType = LayoutSection::XLarge;
    } else  if (attributeValue == "xx-large") {
        heightType = LayoutSection::XxLarge;
    }
    return heightType;
}

inline bool KeyboardData::toBoolean(const QString &attributeValue)
{
    return attributeValue == "true" || attributeValue == "1";
}

const LayoutData *KeyboardData::layout(LayoutData::LayoutType type,
                                       M::Orientation orientation,
                                       bool portaitFallback) const
{
    return layoutPrivate(type, orientation, portaitFallback);
}

LayoutData *KeyboardData::layoutPrivate(LayoutData::LayoutType type,
                                        M::Orientation orientation,
                                        bool portraitFallback) const
{
    LayoutData *bestMatch = NULL;
    foreach(LayoutData * layoutModel, layouts) {
        if (layoutModel->type() == type) {
            if (layoutModel->orientation() == orientation) {
                return layoutModel;
            } else if (portraitFallback && (orientation == M::Portrait)) {
                // According to requirements, when portrait layout is needed,
                // landscape is used as a fallback
                bestMatch = layoutModel;
            }
        }
    }
    return bestMatch;
}

QString KeyboardData::language() const
{
    return keyboardLanguage;
}

QString KeyboardData::title() const
{
    return keyboardTitle;
}

QString KeyboardData::layoutFile() const
{
    return layoutFileName;
}

bool KeyboardData::autoCapsEnabled() const
{
    return keyboardAutoCapsEnabled;
}

bool KeyboardData::loadNokiaKeyboard(const QString &fileName)
{
    ParseParameters params;

    // allow reloading
    qDeleteAll(layouts);
    layouts.clear();

    layoutFileName = fileName;

    const bool success = loadNokiaKeyboardImpl(fileName, params);
    if (!success) {
        qDeleteAll(layouts);
        layouts.clear();
    }
    return success;
}

bool KeyboardData::loadNokiaKeyboardImpl(const QString &fileName, ParseParameters &params,
        bool importedLayout)
{
    const QString absoluteFileName = VKBConfigurationPath + fileName;

    params.fileName = &absoluteFileName;
    if (!QFile::exists(absoluteFileName)) {
        qWarning() << "Virtual keyboard layout file" << absoluteFileName << "does not exist.";
        return false;
    }

    QFile infile(absoluteFileName);
    QString errorStr;
    int errorLine;
    int errorColumn;
    QDomDocument doc;

    if (!infile.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open virtual keyboard layout file" << absoluteFileName;
        return false;
    }
    if (!doc.setContent((QIODevice *)(&infile), true, &errorStr, &errorLine,
                        &errorColumn)) {
        qWarning() << "Invalid virtual keyboard layout file" << absoluteFileName;
        qWarning("Parse error on line %d column %d: %s", errorLine, errorColumn,
                 errorStr.toAscii().constData());
        return false;
    }

    bool valid = true;
    const QDomElement root = doc.documentElement();

    //check the root tag
    if (!root.isNull() && root.tagName() != VKBTagKeyboard) {
        qWarning() << "Invalid virtual keyboard layout file" << absoluteFileName;
        valid = false;
    } else {
        // Don't overwrite keyboard layout information when using <import>
        if (!importedLayout) {
            keyboardVersion = root.attribute(VKBTagVersion);
            keyboardTitle = root.attribute(VKBTagTitle);
            keyboardLanguage = root.attribute(VKBTagLanguage);
            keyboardCatalog = root.attribute(VKBTagCatalog);
            keyboardAutoCapsEnabled = toBoolean(root.attribute(VKBTagAutoCapitalization, "true"));
        }

        parseChildren(root, params, &VKBTagImport, &KeyboardData::parseTagImport,
                      &VKBTagLayout, &KeyboardData::parseTagLayout);
        valid = params.validTag;
    }

    return valid;
}

/*
 * parseChildren has been hardwired to support max. two different tags at each level,
 * which is sufficient for now and makes this easier to use.  If more will be needed,
 * change this to accept an array of (tagname, parser) structures.
 */
void KeyboardData::parseChildren(const QDomElement &element, ParseParameters &params,
                                 const QString *tag1, TagParser parser1, const QString *tag2,
                                 TagParser parser2)
{
    Q_ASSERT(tag1);
    Q_ASSERT(parser1);

    for (QDomNode child = element.firstChild(); !child.isNull() && params.validTag;
            child = child.nextSibling()) {
        if (child.isElement()) {
            const QDomElement childElement = child.toElement();
            if (childElement.tagName() == *tag1) {
                (this->*parser1)(childElement, params);
            } else if ((tag2 != NULL) && (childElement.tagName() == *tag2)) {
                Q_ASSERT(parser2 != 0);
                (this->*parser2)(childElement, params);
            } else {
                qWarning() << "Unexpected tag" << childElement.tagName() << "on line"
                           << childElement.lineNumber() << "column" << childElement.columnNumber()
                           << "in layout file" << *params.fileName;
                if (tag2) {
                    qWarning() << "Allowed tags are" << *tag1 << "and" << *tag2;
                } else {
                    qWarning() << "The only allowed tag is" << *tag1;
                }
                params.validTag = false;
            }
        }
    }
}

void KeyboardData::parseTagImport(const QDomElement &element, ParseParameters &params)
{
    const QString redirectFileName = element.attribute(VKBTagFile);
    const QString absoluteFileName = VKBConfigurationPath + redirectFileName;
    qDebug() << "Importing file" << absoluteFileName;
    if (QFile::exists(absoluteFileName)) {
        const QString *savedFileName = params.fileName;
        if (!loadNokiaKeyboardImpl(redirectFileName, params, true)) {
            qWarning() << __PRETTY_FUNCTION__
                       << "wrong format xml" << absoluteFileName << "for virtual keyboard";
            params.validTag = false;
        }
        params.fileName = savedFileName;
    }
}

M::Orientation KeyboardData::orientation(const QString &orientationString)
{
    M::Orientation orient = M::Portrait;
    if (orientationString == VKBTagOrientationLandscape)
        orient = M::Landscape;
    return orient;
}

void KeyboardData::parseTagLayout(const QDomElement &element, ParseParameters &params)
{
    const QString typeString = element.attribute(VKBTagType);
    LayoutData::LayoutType type = LayoutData::General;

    // Convert type string to internal representation
    if (layoutTypeMap.contains(typeString)) {
        type = layoutTypeMap.value(typeString);
    } else {
        params.validTag = false;
        qWarning() << __PRETTY_FUNCTION__ << "invalid layout type";
        return;
    }

    // Inherit sections from a layout of same type and same orientation
    // (->just use that layout model), if available, or from a layout of
    // same type and different orientation (copy sections to a new
    // layout model), if available.

    const M::Orientation orient = orientation(element.attribute(VKBTagOrientation));
    LayoutData *layoutModel = layoutPrivate(type, orient, false);

    if (layoutModel == NULL) {
        layoutModel = new LayoutData;

        layoutModel->layoutType = type;
        layoutModel->layoutOrientation = orient;

        M::Orientation otherOrientation;
        if (orient == M::Portrait) {
            otherOrientation = M::Landscape;
        } else {
            otherOrientation = M::Portrait;
        }
        const LayoutData *const parentLayout = layoutPrivate(type, otherOrientation, false);
        if (parentLayout) {
            layoutModel->sectionMap = parentLayout->sectionMap;
        }

        layouts.append(layoutModel);
    }

    currentLayout = layoutModel;

    parseChildren(element, params, &VKBTagSection, &KeyboardData::parseTagSection);

    currentLayout->sections = currentLayout->sectionMap.values();
}

Qt::Alignment KeyboardData::alignment(const QString &alignmentString, bool vertical)
{
    Qt::Alignment align = Qt::AlignJustify;
    if (alignmentString == VKBTagAlignFull)
        align = Qt::AlignJustify;
    else if (alignmentString == VKBTagAlignTop)
        align = Qt::AlignTop;
    else if (alignmentString == VKBTagAlignBottom)
        align = Qt::AlignBottom;
    else if (alignmentString == VKBTagAlignCenter)
        align = vertical ? Qt::AlignVCenter : Qt::AlignHCenter;
    else if (alignmentString == VKBTagAlignLeft)
        align = Qt::AlignLeft;
    else if (alignmentString == VKBTagAlignRight)
        align = Qt::AlignRight;
    return align;
}

void KeyboardData::parseTagSection(const QDomElement &element, ParseParameters &params)
{
    LayoutData::SharedLayoutSection section(new LayoutSection);
    section->movable = toBoolean(element.attribute(VKBTagMovable));
    section->sectionName = element.attribute(VKBTagID);
    section->sectionType = (element.attribute(VKBTagType) == VKBTagTypeNonsloppy) ? LayoutSection::NonSloppy : LayoutSection::Sloppy;
    params.currentSection = section;
    currentLayout->sectionMap.insert(section->sectionName, section);
    parseChildren(element, params, &VKBTagRow, &KeyboardData::parseTagRow);
}

void KeyboardData::parseTagRow(const QDomElement &element, ParseParameters &params)
{
    LayoutSection::Row *row = new LayoutSection::Row;
    row->heightType = toHeightType(element.attribute(HeightTypeString, HeightTypeStringDefValue)); 
    params.currentSection->rows.append(row);
    params.currentRow = row;

    parseChildren(element, params,
                  &VKBTagKey, &KeyboardData::parseTagKey,
                  &VKBTagSpacer, &KeyboardData::parseTagSpacer);

    params.currentSection->mMaxColumns = qMax(params.currentSection->maxColumns(),
                                              row->keys.size());
}

void KeyboardData::parseTagBinding(const QDomElement &element, ParseParameters &params)
{
    const bool shift = toBoolean(element.attribute(VKBTagShift, "false"));

    if (params.currentKey->bindings[shift ? VKBDataKey::Shift : VKBDataKey::NoShift]) {
        qWarning() << "Ignoring duplicate binding with same shift attribute on line"
                   << element.lineNumber() << "column" << element.columnNumber()
                   << "in layout file" << *params.fileName;
        return;
    }

    KeyBinding *binding = new KeyBinding;

    binding->keyLabel = element.attribute(VKBTagLabel);
    binding->keyAction = keyActionFromString(element.attribute(VKBTagKeyAction));
    if (binding->keyAction == KeyBinding::ActionCycle) {
        binding->cycleSet = element.attribute(VKBTagCycleSet);
    }
    binding->secondary_label = element.attribute(VKBTagSecondaryLabel);
    binding->dead = toBoolean(element.attribute(VKBTagDead));

    binding->accents = element.attribute(VKBTagAccents);
    binding->accented_labels = element.attribute(VKBTagAccentedLabels);

    binding->extended_labels = element.attribute(VKBTagExtendedLabels);

    params.currentKey->bindings[shift ? VKBDataKey::Shift : VKBDataKey::NoShift] = binding;
}

void KeyboardData::parseTagKey(const QDomElement &element, ParseParameters &params)
{
    VKBDataKey::StyleType type = toStyleType(element.attribute(StyleString, StyleStringDefValue));
    VKBDataKey::WidthType widthType = toWidthType(element.attribute(WidthTypeString, WidthTypeStringDefValue));
    const bool isRtl = toBoolean(element.attribute(RtlString, RtlStringDefValue));
    const bool isFixed = toBoolean(element.attribute(FixedString, FixedStringDefValue));

    VKBDataKey *key = new VKBDataKey(type, widthType, isFixed, isRtl);
    params.currentKey = key;
    params.currentRow->keys.append(key);

    parseChildren(element, params, &VKBTagBinding, &KeyboardData::parseTagBinding);

    if (key->bindings[1] == NULL) {
        key->bindings[1] = key->bindings[0];
    }

    if (key->bindings[0] == NULL) {
        key->bindings[0] = key->bindings[1];
    }
}

void KeyboardData::parseTagSpacer(const QDomElement &, ParseParameters &params)
{
    params.currentRow->spacerIndices.append(params.currentRow->keys.count() - 1);
}

KeyBinding::KeyAction KeyboardData::keyActionFromString(const QString &typeStr)
{
    KeyBinding::KeyAction result;

    if (typeStr == ActionStrShift)
        result = KeyBinding::ActionShift;
    else if (typeStr == ActionStrInsert)
        result = KeyBinding::ActionInsert;
    else if (typeStr == ActionStrBackspace)
        result = KeyBinding::ActionBackspace;
    else if (typeStr == ActionStrSpace)
        result = KeyBinding::ActionSpace;
    else if (typeStr == ActionStrCycle)
        result = KeyBinding::ActionCycle;
    else if (typeStr == ActionStrLayoutMenu)
        result = KeyBinding::ActionLayoutMenu;
    else if (typeStr == ActionStrSym)
        result = KeyBinding::ActionSym;
    else if (typeStr == ActionStrReturn)
        result = KeyBinding::ActionReturn;
    else if (typeStr == ActionStrDecimalSeparator)
        result = KeyBinding::ActionDecimalSeparator;
    else if (typeStr == ActionStrPlusMinusToggle)
        result = KeyBinding::ActionPlusMinusToggle;
    else if (typeStr == ActionStrTab)
        result = KeyBinding::ActionTab;
    else if (typeStr == ActionStrCommit)
        result = KeyBinding::ActionCommit;
    else if (typeStr == ActionStrSwitch)
        result = KeyBinding::ActionSwitch;
    else
        result = KeyBinding::ActionInsert;
    return result;
}
