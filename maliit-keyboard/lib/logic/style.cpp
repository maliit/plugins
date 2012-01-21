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

#include "style.h"

namespace MaliitKeyboard {

namespace {
const char *const styles_dir(MALIIT_KEYBOARD_STYLES_DIR);
const QString profile_filename_format("%1/%2.ini");
const QString key_with_format("key-width%2");

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

QByteArray fromKeyIcon(KeyDescription::Icon icon)
{
    switch (icon) {
    case KeyDescription::NoIcon: return QByteArray();
    case KeyDescription::ReturnIcon: return QByteArray("return");
    case KeyDescription::BackspaceIcon: return QByteArray("backspace");
    case KeyDescription::ShiftIcon: return QByteArray("shift");
    case KeyDescription::ShiftLatchedIcon: return QByteArray("shift-latched");
    case KeyDescription::CapsLockIcon: return QByteArray("caps-lock");
    }

    return QByteArray();
}

QByteArray fromKeyStyle(KeyDescription::Style style)
{
    switch (style) {
    case KeyDescription::NormalStyle: return QByteArray("normal");
    case KeyDescription::DeadkeyStyle: return QByteArray("dead");
    case KeyDescription::SpecialStyle: return QByteArray("special");
    }

    return QByteArray();
}

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

QByteArray buildBackgroundId(KeyDescription::Style style,
                             KeyDescription::State state)
{
    QByteArray result("background/");
    result.append(fromKeyStyle(style));
    result.append(fromKeyState(state));

    return result;
}

QByteArray buildIconId(KeyDescription::Icon icon,
                       KeyDescription::State state)
{
    QByteArray result("icon/");
    result.append(fromKeyIcon(icon));
    result.append(fromKeyState(state));

    return result;
}

QByteArray buildId(Layout::Orientation orientation,
                   const QByteArray &style_name,
                   const QByteArray &id)
{
    QByteArray result;
    result.append(style_name);
    result.append('/');
    result.append(orientation == Layout::Landscape ? "landscape" : "portrait");
    result.append('/');
    result.append(id);

    return result;
}

QVariant lookup(const QScopedPointer<QSettings> &store,
                Layout::Orientation orientation,
                const QByteArray &style_name,
                const QByteArray &id)
{
    if (store.isNull()) {
        qCritical() << __PRETTY_FUNCTION__
                    << "No store found, aborting.";
        return QVariant();
    }

    const QVariant &result(store->value(buildId(orientation, style_name, id)));

    if (not result.isValid()) {
        return store->value(buildId(orientation, QByteArray("default"), id));
    }

    return result;
}

}

class StylePrivate
{
public:
    QString name;
    QScopedPointer<QSettings> store;

    explicit StylePrivate()
        : name()
        , store()
    {}
};

Style::Style(QObject *parent)
    : QObject(parent)
    , d_ptr(new StylePrivate)
{}

Style::~Style()
{}

void Style::setProfile(const QString &profile)
{
    Q_D(Style);

    d->store.reset(new QSettings(profile_filename_format
                                 .arg(styles_dir).arg(profile),
                                 QSettings::IniFormat));
}

void Style::setStyleName(const QString &name)
{
    Q_D(Style);
    d->name = name;
}

QByteArray Style::keyAreaBackground() const
{
    Q_D(const Style);
    return (d->store.isNull() ? QByteArray()
                              : d->store->value("background/key-area").toByteArray());
}

QByteArray Style::keyBackground(KeyDescription::Style style,
                                KeyDescription::State state) const
{
    Q_D(const Style);
    return (d->store.isNull() ? QByteArray()
                              : d->store->value(buildBackgroundId(style, state)).toByteArray());
}

QMargins Style::keyAreaBackgroundBorders() const
{
    Q_D(const Style);
    return (d->store.isNull() ? QMargins()
                              : fromByteArray(d->store->value("background/key-area-borders").toByteArray()));
}

QMargins Style::keyBackgroundBorders() const
{
    Q_D(const Style);
    return (d->store.isNull() ? QMargins()
                              : fromByteArray(d->store->value("background/key-borders").toByteArray()));
}

QByteArray Style::icon(KeyDescription::Icon icon,
                       KeyDescription::State state) const
{
    Q_D(const Style);
    return (d->store.isNull() ? QByteArray()
                              : d->store->value(buildIconId(icon, state)).toByteArray());
}

QByteArray Style::fontName(const QByteArray &group_id) const
{
    Q_UNUSED(group_id)
    return QByteArray("Nokia Pure");
}

qreal Style::fontSize(const QByteArray &group_id) const
{
    Q_UNUSED(group_id)
    return 20;
}

qreal Style::keyHeight(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation,
                  d->name.toLocal8Bit(),
                  QByteArray("key-height")).toReal();
}

qreal Style::keyWidth(Layout::Orientation orientation,
                      KeyDescription::Width width) const
{
    Q_D(const Style);
    return lookup(d->store, orientation,
                  d->name.toLocal8Bit(),
                  QByteArray("key-width").append(fromKeyWidth(width))).toReal();
}

qreal Style::keyAreaWidth(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation,
                  d->name.toLocal8Bit(),
                  QByteArray("key-area-width")).toReal();
}

qreal Style::keyMargin(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation,
                  d->name.toLocal8Bit(),
                  QByteArray("key-margins")).toReal();
}

qreal Style::keyAreaPadding(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation,
                  d->name.toLocal8Bit(),
                  QByteArray("key-area-paddings")).toReal();
}

qreal Style::verticalExtendedKeysOffset(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation,
                  d->name.toLocal8Bit(),
                  QByteArray("vertical-extended-keys-offset")).toReal();
}

qreal Style::extendedKeysSafetyMargin(Layout::Orientation orientation) const
{
    Q_D(const Style);
    return lookup(d->store, orientation,
                  d->name.toLocal8Bit(),
                  QByteArray("extended-keys-safety-margin")).toReal();
}

} // namespace MaliitKeyboard
