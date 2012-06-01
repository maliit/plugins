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
#include "coreutils.h"

//! \class Style
//! Style is a container for StyleAttributes, which can read style attributes
//! from INI files. Style makes sure that the proper style directories are
//! used, depending on the given profile. It also maps the known INI files to
//! the StyleAttributes instances.
//!
//! This class is uncopyable; use SharedStyle instead.

namespace MaliitKeyboard {

namespace {
const QString g_styles_dir_path(MaliitKeyboard::CoreUtils::maliitKeyboardDataDirectory() + "/styles");
const QString g_main_fn_format("%1/%2/main.ini");
const QString g_extended_keys_fn_format("%1/%2/extended-keys.ini");
const QString g_profile_image_directory_path_format("%1/%2/images");
const QString g_profile_sounds_directory_path_format("%1/%2/sounds");
}


//! \class StylePrivate
//! \brief The private Style data.
class StylePrivate
{
public:
    QString profile; //!< The profile name.
    QString style_name; //!< The active style name.
    QScopedPointer<StyleAttributes> attributes; //!< The main style attributes.
    QScopedPointer<StyleAttributes> extended_keys_attributes; //!< The extended keys style attributes.

    explicit StylePrivate()
        : profile()
        , style_name()
        , attributes()
        , extended_keys_attributes()
    {}
};


Style::Style()
    : d_ptr(new StylePrivate)
{}


Style::~Style()
{}


//! \brief Sets the style profile.
//!
//! Invalidates previous StyleAttributes instances and creates new ones.
//! \param profile The name of the profile, must be a valid sub directory in
//!                data/styles and contain at least a main.ini file.
void Style::setProfile(const QString &profile)
{
    Q_D(Style);
    d->profile = profile;

    StyleAttributes *attributes = 0;
    StyleAttributes *extended_keys_attributes = 0;

    if (not d->profile.isEmpty()) {
        const QString main_file_name(g_main_fn_format
                                     .arg(g_styles_dir_path).arg(profile));
        const QString extended_keys_file_name(g_extended_keys_fn_format
                                              .arg(g_styles_dir_path).arg(profile));

        attributes =  new StyleAttributes(
            new QSettings(main_file_name, QSettings::IniFormat));
        extended_keys_attributes = new StyleAttributes(
            new QSettings(extended_keys_file_name, QSettings::IniFormat));
    }

    d->attributes.reset(attributes);
    d->extended_keys_attributes.reset(extended_keys_attributes);
}


//! \brief Returns the active style profile.
QString Style::profile() const
{
    Q_D(const Style);
    return d->profile;
}


//! \brief Returns a list of available profiles.
QStringList Style::availableProfiles() const
{
    static const QDir styles_dir(g_styles_dir_path);
    return styles_dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot, QDir::Name);
}


//! \brief Query the profile-dependent directory path for images, sounds, etc.
//! @param directory The directory enum value for which we want to know the
//!                  directory path.
//! @returns The directory path. Will be empty if for example profile is empty.
QString Style::directoryPath(Directory directory) const
{
    Q_D(const Style);

    if (d->profile.isEmpty()) {
        return QString();
    }

    switch (directory) {
    case Images:
        return g_profile_image_directory_path_format.arg(g_styles_dir_path).arg(d->profile);

    case Sounds:
        return g_profile_sounds_directory_path_format.arg(g_styles_dir_path).arg(d->profile);
    }

    return QString();
}


//! \brief Query the main style attributes.
//! @returns The style attributes used for the main key area.
StyleAttributes * Style::attributes() const
{
    Q_D(const Style);
    return d->attributes.data();
}


//! \brief Query the extened keys style attributes.
//! @returns The style attributes used for the extended key area.
StyleAttributes * Style::extendedKeysAttributes() const
{
    Q_D(const Style);
    return d->extended_keys_attributes.data();
}

} // namespace MaliitKeyboard
