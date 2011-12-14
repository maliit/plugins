// -*- mode: C++; c-basic-offset: 4; indent-tabs-mode: nil; c-file-offsets: ((innamespace . 0)); -*-
/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 *
 */

#include "layoutparser.h"

#include <QDebug>

namespace MaliitKeyboard {

LayoutParser::LayoutParser(QIODevice *device)
    : m_xml(device)
    , m_keyboard()
    , m_imports()
    , m_last_layout()
    , m_last_section()
    , m_last_row()
    , m_last_key()
{}

bool LayoutParser::parse()
{
    goToRootElement();

    if (not m_xml.isStartElement() || m_xml.name() != QLatin1String("keyboard")) {
        error(QString::fromLatin1("Expected '<keyboard>', but got '<%1>'.").arg(m_xml.name().toString()));
    } else if (not m_xml.hasError()) {
        parseKeyboard();
    }

    //readToEnd();

    return not m_xml.hasError();
}

bool LayoutParser::isLanguageFile()
{
    goToRootElement();

    if (not m_xml.isStartElement() || m_xml.name() != QLatin1String("keyboard")) {
        return false;
    } else if (not m_xml.hasError()) {
        const QXmlStreamAttributes attributes(m_xml.attributes());
        const QStringRef language(attributes.value(QLatin1String("language")));

        return (not language.isEmpty());
    }
    return false;
}

void LayoutParser::goToRootElement()
{
    while (not m_xml.atEnd()) {
        QXmlStreamReader::TokenType type(m_xml.readNext());

        if (type == QXmlStreamReader::StartElement) {
            return;
        }
    }
}

void LayoutParser::error(const QString &message)
{
    if (not m_xml.hasError()) {
        m_xml.raiseError(message);
    }
}

void LayoutParser::parseKeyboard()
{
    const QXmlStreamAttributes attributes(m_xml.attributes());
    const QString version(attributes.value(QLatin1String("version")).toString());
    const QString actual_version(version.isEmpty() ? "1.0" : version);
    const QString title(attributes.value(QLatin1String("title")).toString());
    const QString language(attributes.value(QLatin1String("language")).toString());
    const QString catalog(attributes.value(QLatin1String("catalog")).toString());
    const bool autocapitalization(boolValue(attributes.value(QLatin1String("autocapitalization")), true));
    m_keyboard = TagKeyboardPtr(new TagKeyboard(actual_version,
                                                title,
                                                language,
                                                catalog,
                                                autocapitalization));

    while (m_xml.readNextStartElement()) {
        const QStringRef name(m_xml.name());

        if (name == QLatin1String("import")) {
            parseImport();
        } else if (name == QLatin1String("layout")) {
            parseLayout();
        } else {
            error(QString::fromLatin1("Expected '<layout>' or '<import>', but got '<%1>'.").arg(name.toString()));
        }
    }
}

bool LayoutParser::boolValue(const QStringRef &value, bool defaultValue) {
    if (value.isEmpty()) {
        return defaultValue;
    }

    if (value == QLatin1String("true") ||
        value == QLatin1String("1"))
        return true;

    if (value == QLatin1String("false") ||
        value == QLatin1String("0"))
        return false;

    error(QString::fromLatin1("Expected 'true', 'false', '1' or '0', but got '%1'.").arg(value.toString()));

    return defaultValue;
}

void LayoutParser::parseImport()
{
    const QXmlStreamAttributes attributes(m_xml.attributes());
    const QString file(attributes.value(QLatin1String("file")).toString());

    if (file.isEmpty()) {
        bool found_anything(false);

        while (m_xml.readNextStartElement()) {
            const QStringRef name(m_xml.name());

            if (name == QLatin1String("symview")) {
                found_anything = true;
                parseImportChild(&m_symviews);
            } else if (name == QLatin1String("number")) {
                found_anything = true;
                parseImportChild(&m_numbers);
            } else if (name == QLatin1String("phonenumber")) {
                found_anything = true;
                parseImportChild(&m_phonenumbers);
            } else {
                error(QString::fromLatin1("Expected '<symview>' or '<number>' or '<phonenumber>', but got '<%1>'.").arg(name.toString()));
            }
        }
        if (not found_anything) {
            error(QString::fromLatin1("Expected '<symview>' or '<number>' or '<phonenumber>'."));
        }
    } else {
        if (m_xml.readNextStartElement()) {
            error(QString::fromLatin1("Expected no child tags, because 'file' attribute exists, but got '<%1>'.").arg(m_xml.name().toString()));
        } else {
            m_imports.append(file);
            m_xml.skipCurrentElement();
        }
    }

}

void LayoutParser::parseImportChild(QStringList *target_list)
{
    const QXmlStreamAttributes attributes(m_xml.attributes());
    const QString src(attributes.value(QLatin1String("src")).toString());

    if (src.isEmpty()) {
        error(QString::fromLatin1("Expected non-empty 'src' attribute in '<%1>'.").arg(m_xml.name().toString()));
    } else if (target_list) {
        target_list->append(src);
    }

    m_xml.skipCurrentElement();
}

void LayoutParser::parseLayout()
{
    static const QStringList typeValues(QString::fromLatin1("general,url,email,number,phonenumber,common").split(','));
    static const QStringList orientationValues(QString::fromLatin1("landscape,portrait").split(','));

    const QXmlStreamAttributes attributes(m_xml.attributes());
    const TagLayout::LayoutType type(enumValue("type", typeValues, TagLayout::General));
    const TagLayout::LayoutOrientation orientation(enumValue("orientation", orientationValues, TagLayout::Landscape));
    const bool uniform_font_size(boolValue(attributes.value(QLatin1String("uniform-font-size")), false));

    m_last_layout = TagLayoutPtr(new TagLayout(type,
                                               orientation,
                                               uniform_font_size));
    m_keyboard->appendLayout(m_last_layout);

    bool found_section(false);

    while (m_xml.readNextStartElement()) {
        const QStringRef name(m_xml.name());

        if (name == QLatin1String("section")) {
            found_section = true;
            parseSection();
        } else {
            error(QString::fromLatin1("Expected '<section>', but got '<%1>'.").arg(name.toString()));
        }
    }

    if (not found_section) {
        error(QString::fromLatin1("Expected '<section>'."));
    }
}

template <class E>
E LayoutParser::enumValue(const char * const attribute, const QStringList &values, E defaultValue)
{
    if (m_xml.hasError())
        return defaultValue;

    const QXmlStreamAttributes& attributes(m_xml.attributes());
    const QStringRef& value(attributes.value(QLatin1String(attribute)));

    if (value.isEmpty()) {
        return defaultValue;
    }

    const int index(values.indexOf(value.toString()));

    if (index == -1) {
        error(QString::fromLatin1("Expected one of '%1', but got '%2'.").arg(values.join("', '"), value.toString()));

        return defaultValue;
    }

    return static_cast<E>(index);
}

void LayoutParser::parseSection()
{
    static const QStringList typeValues(QString::fromLatin1("sloppy,non-sloppy").split(','));

    const QXmlStreamAttributes attributes(m_xml.attributes());
    const QString id(attributes.value(QLatin1String("id")).toString());
    const bool movable(boolValue(attributes.value(QLatin1String("movable")), true));
    const TagSection::SectionType type(enumValue("type", typeValues, TagSection::Sloppy));
    const QString style(attributes.value(QLatin1String("style")).toString());

    if (id.isEmpty()) {
        error("Expected non-empty 'id' attribute in '<section>'.");
        return;
    }

    m_last_section = TagSectionPtr(new TagSection(id,
                                                  movable,
                                                  type,
                                                  style));
    m_last_layout->appendSection(m_last_section);

    bool found_row(false);

    while (m_xml.readNextStartElement()) {
        const QStringRef name(m_xml.name());

        if (name == QLatin1String("row")) {
            parseRow();
            found_row = true;
        } else {
            error(QString::fromLatin1("Expected '<row>', but got '<%1>'.").arg(name.toString()));
        }
    }

    if (not found_row) {
        error(QString::fromLatin1("Expected '<row>'."));
    }

}

void LayoutParser::parseRow()
{
    static const QStringList heightValues(QString::fromLatin1("small,medium,large,x-large,xx-large").split(','));

    const TagRow::Height height(enumValue("height", heightValues, TagRow::Medium));

    m_last_row = TagRowPtr(new TagRow(height));
    m_last_section->appendRow(m_last_row);

    while (m_xml.readNextStartElement()) {
        QStringRef name(m_xml.name());

        if (name == QLatin1String("key")) {
            parseKey();
        } else if (name == QLatin1String("spacer")) {
            parseSpacer();
        } else {
            error(QString::fromLatin1("Expected '<key>' or '<spacer>', but got '<%1>'.").arg(name.toString()));
        }
    }
}

void LayoutParser::parseKey()
{
    static const QStringList styleValues(QString::fromLatin1("normal,special,deadkey").split(','));
    static const QStringList widthValues(QString::fromLatin1("small,medium,large,x-large,xx-large,stretched").split(','));

    const QXmlStreamAttributes attributes(m_xml.attributes());
    const TagKey::Style style(enumValue("style", styleValues, TagKey::Normal));
    const TagKey::Width width(enumValue("width", widthValues, TagKey::Medium));
    const bool rtl(boolValue(attributes.value(QLatin1String("rtl")), false));
    const QString id(attributes.value(QLatin1String("id")).toString());

    m_last_key = TagKeyPtr(new TagKey(style,
                                      width,
                                      rtl,
                                      id));
    m_last_row->appendElement(m_last_key);

    bool found_binding(false);

    while (m_xml.readNextStartElement()) {
        QStringRef name(m_xml.name());

        if (name == QLatin1String("binding")) {
            parseBinding();
            found_binding = true;
        } else {
            error(QString::fromLatin1("Expected '<binding>', but got '<%1>'.").arg(name.toString()));
        }
    }

    if (not found_binding) {
        error(QString::fromLatin1("Expected '<binding>'."));
    }
}

void LayoutParser::parseBinding()
{
    static const QStringList actionValues(QString::fromLatin1("insert,shift,backspace,space,cycle,layout_menu,sym,return,commit,decimal_separator,plus_minus_toggle,switch,on_off_toggle,compose,left,up,right,down").split(','));

    const QXmlStreamAttributes attributes(m_xml.attributes());
    const TagBinding::Action action(enumValue("action", actionValues, TagBinding::Insert));
    const bool shift(boolValue(attributes.value(QLatin1String("shift")), false));
    const bool alt(boolValue(attributes.value(QLatin1String("alt")), false));
    const QString label(attributes.value(QLatin1String("label")).toString());
    const QString secondary_label(attributes.value(QLatin1String("secondary_label")).toString());
    const QString accents(attributes.value(QLatin1String("accents")).toString());
    const QString accented_labels(attributes.value(QLatin1String("accented_labels")).toString());
    const QString extended_labels(attributes.value(QLatin1String("extended_labels")).toString());
    const QString cycleset(attributes.value(QLatin1String("cycleset")).toString());
    const bool dead(boolValue(attributes.value(QLatin1String("dead")), false));
    const bool quick_pick(boolValue(attributes.value(QLatin1String("quick_pick")), false));
    const bool rtl(boolValue(attributes.value(QLatin1String("rtl")), false));
    const bool enlarge(boolValue(attributes.value(QLatin1String("enlarge")), false));

    m_last_key->appendBinding(TagBindingPtr(new TagBinding(action,
                                                           shift,
                                                           alt,
                                                           label,
                                                           secondary_label,
                                                           accents,
                                                           accented_labels,
                                                           extended_labels,
                                                           cycleset,
                                                           dead,
                                                           quick_pick,
                                                           rtl,
                                                           enlarge)));
    m_xml.skipCurrentElement();
}

void LayoutParser::parseSpacer()
{
    m_last_row->appendElement(TagSpacerPtr(new TagSpacer));
    m_xml.skipCurrentElement();
}

void LayoutParser::readToEnd()
{
    while (not m_xml.atEnd()) {
        m_xml.readNext();
    }
}

const QString LayoutParser::errorString() const
{
    return m_xml.errorString();
}

const TagKeyboardPtr LayoutParser::keyboard() const
{
    return m_keyboard;
}

const QStringList LayoutParser::imports() const
{
    return m_imports;
}

const QStringList LayoutParser::symviews() const
{
    return m_symviews;
}

const QStringList LayoutParser::numbers() const
{
    return m_numbers;
}

const QStringList LayoutParser::phonenumbers() const
{
    return m_phonenumbers;
}

} // namespace MaliitKeyboard
