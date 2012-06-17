/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2012 One Laptop per Child Association
 *
 * Contact: maliit-discuss@lists.maliit.org
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

#include "styleattributes.h"

//! \class StyleAttributes
//! This class allows to query style attributes, such as image names and font
//! sizes. Styling attributes are read from INI files.

namespace MaliitKeyboard {
namespace {

//! \brief Converts KeyDescription::Width enum values into string representations.
//! @param width The enum value to convert.
//! @returns The string representation.
QByteArray fromKeyWidth(KeyDescription::Width width)
{
    switch(width) {
    case KeyDescription::Medium: return QByteArray();
    case KeyDescription::Small: return QByteArray("-small");
    case KeyDescription::Large: return QByteArray("-large");
    case KeyDescription::XLarge: return QByteArray("-xlarge");
    case KeyDescription::XXLarge: return QByteArray("-xxlarge");
    case KeyDescription::Stretched: return QByteArray("-stretched"); // makes no sense actually.
    }

    return QByteArray();
}


//! \brief Converts KeyDescription::Icon enum values into string representations.
//! @param icon The enum value to convert.
//! @returns The string representation.
QByteArray fromKeyIcon(KeyDescription::Icon icon)
{
    switch (icon) {
    case KeyDescription::NoIcon: return QByteArray();
    case KeyDescription::ReturnIcon: return QByteArray("return");
    case KeyDescription::BackspaceIcon: return QByteArray("backspace");
    case KeyDescription::ShiftIcon: return QByteArray("shift");
    case KeyDescription::ShiftLatchedIcon: return QByteArray("shift-latched");
    case KeyDescription::CapsLockIcon: return QByteArray("caps-lock");
    case KeyDescription::CloseIcon: return QByteArray("close");
    }

    return QByteArray();
}


//! \brief Converts KeyDescription::Style enum values into string representations.
//! @param style The enum value to convert.
//! @returns The string representation.
QByteArray fromKeyStyle(KeyDescription::Style style)
{
    switch (style) {
    case KeyDescription::NormalStyle: return QByteArray("normal");
    case KeyDescription::DeadkeyStyle: return QByteArray("dead");
    case KeyDescription::SpecialStyle: return QByteArray("special");
    }

    return QByteArray();
}


//! \brief Converts KeyDescription::State enum values into string representations.
//! @param state The enum value to convert.
//! @returns The string representation.
QByteArray fromKeyState(KeyDescription::State state)
{
    switch (state) {
    case KeyDescription::NormalState: return QByteArray();
    case KeyDescription::PressedState: return QByteArray("-pressed");
    case KeyDescription::DisabledState: return QByteArray("-disabled");
    case KeyDescription::HighlightedState: return QByteArray("-highlighted");
    }

    return QByteArray();
}


//! \brief Converts string representations into QMargins.
//! @param data The string representation to convert. Splits into four
//!             integers, using spaces as separator.
//! @returns The converted QMargins.
QMargins fromByteArray(const QByteArray &data)
{
    QMargins result;
    const QList<QByteArray> &tokens(data.split(' '));

    if (tokens.count() != 4) {
        return result;
    }

    result.setLeft(tokens.at(0).toInt());
    result.setTop(tokens.at(1).toInt());
    result.setRight(tokens.at(2).toInt());
    result.setBottom(tokens.at(3).toInt());

    return result;
}


//! \brief Builds keys to look up values from INI files.
//! @param orientation The layout orientation (landscape or portrait).
//! @param style_name The style name, maps to INI file sections.
//! @param attribute_name The attribute name that we want to look up.
//! @returns A key. String won't be empty, even if key is invalid.
QByteArray buildKey(Layout::Orientation orientation,
                    const QByteArray &style_name,
                    const QByteArray &attribute_name)
{
    QByteArray result;
    result.append(style_name);
    result.append('/');
    result.append(orientation == Layout::Landscape ? "landscape" : "portrait");
    result.append('/');
    result.append(attribute_name);

    return result;
}


//! \brief Looks up values in a QSettings store.
//! @param orientation The layout orientation (landscape or portrait).
//! @param style_name Style name, maps to INI file sections. If section does
//!                   not exist, lookup will be repeated in 'default' section.
//! @param attribute_name The attribute name we want to look up.
//! @returns A value. QVariant can be invalid.
QVariant lookup(const QScopedPointer<const QSettings> &store,
                Layout::Orientation orientation,
                const QByteArray &style_name,
                const QByteArray &attribute_name)
{
    const QVariant &result(store->value(buildKey(orientation, style_name, attribute_name)));

    if (not result.isValid()) {
        return store->value(buildKey(orientation, QByteArray("default"), attribute_name));
    }

    return result;
}

} // namespace


//! @param store The settings store which is used to look up all attributes.
//!              Must not be null. StyleAttribute instance takes ownership.
StyleAttributes::StyleAttributes(const QSettings *store)
    : m_store(store)
    , m_style_name()
{
    if (m_store.isNull()) {
        qFatal("QSettings store cannot be null!");
    }

    if (m_store->status() != QSettings::NoError) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Could not read INI file:"
                   << m_store->fileName();
    }
}


//! \brief Sets the active style name.
//!
//! Consider HTML and CSS, where HTML provides the input and CSS specifies
//! the style for the input. In our case, XML layout files provide the data
//! and this Style class specifies the style for the input.
//! This function works similar to &lt;div class="some_class_name"/&gt;, with a
//! some_class_name {} section within the CSS file. The class names, in our
//! case, are the style attributes from a section element within a XML layout
//! file.
//!
//! \param name The style name. Works similar to HTML/CSS class and maps it to
//!             a section within the INI file. There is no check whether such
//!             a section exists!
void StyleAttributes::setStyleName(const QString &name)
{
    m_style_name = name;
}

//! \brief Looks up the background image name for word ribbons.
//! @returns Value of "background\word-ribbon".
QByteArray StyleAttributes::wordRibbonBackground() const
{
    return m_store->value("background/word-ribbon").toByteArray();
}


//! \brief Looks up the background image name for key areas.
//! @returns Value of "background\key-area".
QByteArray StyleAttributes::keyAreaBackground() const
{
    return m_store->value("background/key-area").toByteArray();
}


//! \brief Looks up the background image name for the key magnifier.
//! @returns Value of "background\magnifier-key"
QByteArray StyleAttributes::magnifierKeyBackground() const
{
    return m_store->value("background/magnifier-key").toByteArray();
}


//! \brief Looks up the background image name for keys, depending on style
//!        and state.
//! @param style The key style (normal, special, deadkey).
//! @param state The key state (normal, pressed, disabled, highlighted).
//! @returns Value of "background\${style}[-${state}]"
QByteArray StyleAttributes::keyBackground(KeyDescription::Style style,
                                          KeyDescription::State state) const
{
    QByteArray key("background/");
    key.append(fromKeyStyle(style));
    key.append(fromKeyState(state));

    return m_store->value(key).toByteArray();
}


//! \brief Looks up the borders for 9-tiling word ribbon background.
//! @returns Value of "background\word-ribbon-borders".
QMargins StyleAttributes::wordRibbonBackgroundBorders() const
{
    return fromByteArray(m_store->value("background/word-ribbon-borders").toByteArray());
}


//! \brief Looks up the borders for 9-tiling key area background.
//! @returns Value of "background\key-area-borders".
QMargins StyleAttributes::keyAreaBackgroundBorders() const
{
    return fromByteArray(m_store->value("background/key-area-borders").toByteArray());
}


//! \brief Looks up the borders for 9-tiling magnifier key background.
//! @returns Value of "background\magnifier-key-borders".
QMargins StyleAttributes::magnifierKeyBackgroundBorders() const
{
    return fromByteArray(m_store->value("background/magnifier-key-borders").toByteArray());
}


//! \brief Looks up the borders for 9-tiling key background.
//! @returns Value of "background\key-borders".
QMargins StyleAttributes::keyBackgroundBorders() const
{
    return fromByteArray(m_store->value("background/key-borders").toByteArray());
}


//! \brief Looks up the icon name for keys with icons instead of labels.
//! @param icon The key icon (return, backspace, shift, ...).
//! @param state The key state (normal, pressed, disabled, highlighted).
//! @returns Value of "icon\${icon}[-${state}]".
QByteArray StyleAttributes::icon(KeyDescription::Icon icon,
                                 KeyDescription::State state) const
{
    QByteArray key("icon/");
    key.append(fromKeyIcon(icon));
    key.append(fromKeyState(state));

    return m_store->value(key).toByteArray();
}


//! \brief Looks up the font name used for key labels.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value currently hard-coded (FIXME).
QByteArray StyleAttributes::fontName(Layout::Orientation orientation) const
{
    Q_UNUSED(orientation)
    return QByteArray("Nokia Pure");
}


//! \brief Looks up the font color used for key labels.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\font-color".
QByteArray StyleAttributes::fontColor(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("font-color")).toByteArray();
}


//! \brief Looks up the font size used for key labels.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\font-size".
qreal StyleAttributes::fontSize(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("font-size")).toReal();
}


//! \brief Looks up the small font size used for key labels.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\small-font-size".
qreal StyleAttributes::smallFontSize(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("small-font-size")).toReal();
}


//! \brief Looks up the font size used for word candidates.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\candidates-font-size".
qreal StyleAttributes::candidateFontSize(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("candidate-font-size")).toReal();
}


//! \brief Looks up the font size used for the key magnifier.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\magnifier-font-size".
qreal StyleAttributes::magnifierFontSize(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("magnifier-font-size")).toReal();
}


//! \brief Looks up the font stretch used for word candidates.
//!
//! Non-stretched fonts have a value of 100. Smaller values lead to condensed
//! fonts, higher values stretch the fonts. Smaller values allow to show
//! longer (or more) word candidates that would otherwise have to be cut off.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\candidate-font-stretch".
qreal StyleAttributes::candidateFontStretch(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("candidate-font-stretch")).toReal();
}


//! \brief Looks up the word ribbon height.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\word-ribbon-height".
qreal StyleAttributes::wordRibbonHeight(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("word-ribbon-height")).toReal();
}


//! \brief Looks up the key height.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\key-height".
qreal StyleAttributes::keyHeight(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("key-height")).toReal();
}


//! \brief Looks up the key width.
//! @param orientation The layout orientation (landscape or portrait).
//! @param width The key width type.
//! @returns Value of "${style}\${orientation}\key-width-${width}".
qreal StyleAttributes::keyWidth(Layout::Orientation orientation,
                                KeyDescription::Width width) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("key-width").append(fromKeyWidth(width))).toReal();
}


//! \brief Looks up the key area width.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\key-area-width".
qreal StyleAttributes::keyAreaWidth(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("key-area-width")).toReal();
}


//! \brief Looks up the key margins.
//!
//! The right margin of key A together with the left margin of key B defines
//! the spacing between the two keys. However, the margins count as reactive
//! areas for the keys.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\key-margins".
qreal StyleAttributes::keyMargin(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("key-margins")).toReal();
}

//! \brief Looks up the key area paddings.
//!
//! The left-most key will have its left key margin be overriden by the
//! padding, meaning that the padding defines the spacing between left key
//! area border and first left key (unless a spacer element is involved). The
//! same rules apply to the right-most key and the right key area border.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\key-area-paddings".
qreal StyleAttributes::keyAreaPadding(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("key-area-paddings")).toReal();
}


//! \brief Looks up the vertical offset.
//!
//! Used to position magnifier and extended key area, relative to the pressed
//! key.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\vertical-offset".
qreal StyleAttributes::verticalOffset(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("vertical-offset")).toReal();
}


//! \brief Looks up the safety margin.
//!
//! Try to never put magnifier or extended key area within safety margin,
//! which are aded (substracted) from the left and right key area borders.
//! @param orientation The layout orientation (landscape or portrait).
//! @returns Value of "${style}\${orientation}\safety-margin".
qreal StyleAttributes::safetyMargin(Layout::Orientation orientation) const
{
    return lookup(m_store, orientation,
                  m_style_name.toLocal8Bit(),
                  QByteArray("safety-margin")).toReal();
}


//! \brief Looks up the sound file name that is played when a key was pressed.
//! @returns Value of "sound/key-press".
QByteArray StyleAttributes::keyPressSound() const
{
    return m_store->value("sound/key-press").toByteArray();
}


//! \brief Looks up the sound file name that is played when a key was released.
//! @returns Value of "sound/key-release".
QByteArray StyleAttributes::keyReleaseSound() const
{
    return m_store->value("sound/key-release").toByteArray();
}


//! \brief Looks up the sound file name that is played when the layout changed.
//! @returns Value of "sound/layout-change".
QByteArray StyleAttributes::layoutChangeSound() const
{
    return m_store->value("sound/layout-change").toByteArray();
}


//! \brief Looks up the sound file name that is played when keyboard is hidden.
//! @returns Value of "sound/keyboard-hide".
QByteArray StyleAttributes::keyboardHideSound() const
{
    return m_store->value("sound/keyboard-hide").toByteArray();
}

} // namespace MaliitKeyboard
