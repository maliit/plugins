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

#include "keyareaconverter.h"

#include "style.h"
#include "keyboardloader.h"
#include "models/keyarea.h"
#include "models/key.h"

#include <QtCore>

namespace MaliitKeyboard {
namespace {

KeyArea createFromKeyboard(Style *style,
                           const Keyboard &source,
                           const QPoint &anchor,
                           Layout::Orientation orientation,
                           Layout::Alignment alignment,
                           bool is_extended_keyarea = false)
{
    // An ad-hoc geometry updater that also uses styling information.
    KeyArea ka;
    Keyboard kb(source);

    if (not style) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No style given, aborting.";
        return ka;
    }

    style->setStyleName(kb.style_name);

    KeyFont font;
    font.setName(style->fontName());
    font.setSize(style->fontSize());
    font.setColor(QByteArray("#ffffff"));

    KeyFont small_font(font);
    small_font.setSize(12);

    static const QMargins bg_margins(style->keyBackgroundBorders());

    const qreal max_width(style->keyAreaWidth(orientation));
    const qreal key_height(style->keyHeight(orientation));
    const qreal margin = style->keyMargin(orientation);
    const qreal padding = style->keyAreaPadding(orientation);

    QPoint pos(0, 0);
    QVector<int> row_indices;
    int spacer_count = 0;
    qreal consumed_width = 0;


    for (int index = 0; index < kb.keys.count(); ++index) {
        row_indices.append(index);
        Key &key(kb.keys[index]);
        const KeyDescription &desc(kb.key_descriptions.at(index));
        int width = 0;
        pos.setY(key_height * desc.row);

        bool at_row_start((index == 0)
                          || (kb.key_descriptions.at(index - 1).row < desc.row));
        bool at_row_end((index + 1 == kb.keys.count())
                        || (index + 1 < kb.keys.count()
                            && kb.key_descriptions.at(index + 1).row > desc.row));

        if (desc.left_spacer || desc.right_spacer) {
            ++spacer_count;
        }

        key.setBackground(style->keyBackground(desc.style, KeyDescription::NormalState));
        key.setBackgroundBorders(bg_margins);

        width = style->keyWidth(orientation, desc.width);

        const qreal key_margin((at_row_start || at_row_end) ? margin + padding : margin * 2);
        key.setRect(QRect(pos.x(), pos.y(), width + key_margin, key_height));
        key.setMargins(QMargins(at_row_start ? padding : margin, margin,
                                at_row_end   ? padding : margin, margin));

        key.setFont(key.text().count() > 1 ? small_font : font);

        // FIXME: Read from KeyDescription instead.
        if (key.text().isEmpty()) {
            switch (key.action()) {
            case Key::ActionShift:
                key.setIcon(style->icon(KeyDescription::ShiftIcon,
                                        KeyDescription::NormalState));
                break;

            case Key::ActionBackspace:
                key.setIcon(style->icon(KeyDescription::BackspaceIcon,
                                        KeyDescription::NormalState));
                break;

            case Key::ActionReturn:
                key.setIcon(style->icon(KeyDescription::ReturnIcon,
                                        KeyDescription::NormalState));
                break;

            default:
                break;
            }
        }

        pos.rx() += key.rect().width();

        if (at_row_end) {
            if (not is_extended_keyarea
                && spacer_count > 0 && pos.x() < max_width + 1) {
                const int spacer_width = qMax<int>(0, max_width - pos.x()) / spacer_count;
                pos.setX(0);
                int right_x = 0;

                Q_FOREACH (int row_index, row_indices) {
                    Key &k(kb.keys[row_index]);
                    const KeyDescription &d(kb.key_descriptions.at(row_index));

                    QRect r(k.rect());
                    QMargins m(k.margins());
                    int extra_width = 0;

                    if (d.left_spacer) {
                        m.setLeft(m.left() + spacer_width);
                        extra_width += spacer_width;
                    }

                    if (d.right_spacer) {
                        m.setRight(m.right() + spacer_width);
                        extra_width += spacer_width;
                    }

                    k.setMargins(m);

                    r.translate(right_x - r.left(), 0);
                    r.setWidth(r.width() + extra_width);
                    k.setRect(r);

                    right_x = r.right();
                }
            }

            consumed_width = qMax<qreal>(consumed_width, key.rect().right() + padding);
            row_indices.clear();
            pos.setX(0);
            spacer_count = 0;
        }
    }

    const int height = pos.y() + key_height;
    ka.background = style->keyAreaBackground();
    ka.background_borders = style->keyAreaBackgroundBorders();
    ka.keys = kb.keys;
    // FIXME: left, right aligment does not work as expected (treated as top alignment currently).
    ka.rect =  QRectF(anchor.x() - (is_extended_keyarea ? consumed_width : max_width) / 2,
                      (alignment == Layout::Bottom ? anchor.y() - height : 0),
                      (is_extended_keyarea ? consumed_width : max_width),
                      height);

    return ka;
}
}

KeyAreaConverter::KeyAreaConverter(Style *style,
                                   KeyboardLoader *loader,
                                   const QPoint &anchor)
    : m_style(style)
    , m_loader(loader)
    , m_anchor(anchor)
{
    if (not style || not loader) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Neither style nor loader can be null.";
    }
}

KeyAreaConverter::~KeyAreaConverter()
{}

KeyArea KeyAreaConverter::keyArea(Layout::Orientation orientation,
                                  Layout::Alignment alignment) const
{
    return createFromKeyboard(m_style, m_loader->keyboard(),
                              m_anchor, orientation, alignment);
}

KeyArea KeyAreaConverter::nextKeyArea(Layout::Orientation orientation,
                                      Layout::Alignment alignment) const
{
    return createFromKeyboard(m_style, m_loader->nextKeyboard(),
                              m_anchor, orientation, alignment);
}

KeyArea KeyAreaConverter::previousKeyArea(Layout::Orientation orientation,
                                          Layout::Alignment alignment) const
{
    return createFromKeyboard(m_style, m_loader->previousKeyboard(),
                              m_anchor, orientation, alignment);
}

KeyArea KeyAreaConverter::shiftedKeyArea(Layout::Orientation orientation,
                                         Layout::Alignment alignment) const
{
    return createFromKeyboard(m_style, m_loader->shiftedKeyboard(),
                              m_anchor, orientation, alignment);
}

KeyArea KeyAreaConverter::symbolsKeyArea(Layout::Orientation orientation,
                                         Layout::Alignment alignment,
                                         int page) const
{
    return createFromKeyboard(m_style, m_loader->symbolsKeyboard(page),
                              m_anchor, orientation, alignment);
}

KeyArea KeyAreaConverter::deadKeyArea(Layout::Orientation orientation,
                                      Layout::Alignment alignment,
                                      const Key &dead) const
{
    return createFromKeyboard(m_style, m_loader->deadKeyboard(dead),
                              m_anchor, orientation, alignment);
}

KeyArea KeyAreaConverter::shiftedDeadKeyArea(Layout::Orientation orientation,
                                             Layout::Alignment alignment,
                                             const Key &dead) const
{
    return createFromKeyboard(m_style, m_loader->shiftedDeadKeyboard(dead),
                              m_anchor, orientation, alignment);
}

KeyArea KeyAreaConverter::extendedKeyArea(Layout::Orientation orientation,
                                          Layout::Alignment alignment,
                                          const Key &key) const
{
    return createFromKeyboard(m_style, m_loader->extendedKeyboard(key),
                              m_anchor, orientation, alignment, true);
}

KeyArea KeyAreaConverter::numberKeyArea(Layout::Orientation orientation,
                                        Layout::Alignment alignment) const
{
    return createFromKeyboard(m_style, m_loader->numberKeyboard(),
                              m_anchor, orientation, alignment);
}

KeyArea KeyAreaConverter::phoneNumberKeyArea(Layout::Orientation orientation,
                                             Layout::Alignment alignment) const
{
    return createFromKeyboard(m_style, m_loader->phoneNumberKeyboard(),
                              m_anchor, orientation, alignment);
}

} // namespace MaliitKeyboard
