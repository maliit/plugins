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

#include "popupfactory.h"
#include "popupbase.h"
#include "popupplugin.h"

#include <QDir>
#include <QPluginLoader>
#include <QDebug>

namespace
{
    const char * const PluginDir = "/usr/lib/meego-im-plugins/meego-keyboard/";
}

// dummy popup to be used if no plugin is found
class DummyPopup: public PopupBase
{
public:
    explicit DummyPopup(KeyButtonArea *mainArea)
        : PopupBase(mainArea)
    {}

    virtual ~DummyPopup()
    {}

    //! \reimp
    virtual void updatePos(const QPointF &,
                           const QPoint &,
                           const QSize &)
    {}

    virtual void setFingerPos(const QPointF &)
    {}

    virtual KeyButtonArea* mainArea() const
    {
        return 0;
    }

    virtual void handleInvalidKeyPressedOnMainArea()
    {}

    virtual void handleKeyPressedOnMainArea(const IKeyButton *,
                                            const QString &,
                                            bool)
    {}

    virtual void handleLongKeyPressedOnMainArea(const IKeyButton *,
                                                const QString &,
                                                bool)
    {}

    virtual bool isVisible() const
    {
        return false;
    }

    virtual void setEnabled(bool)
    {}
    //! \reimp_end
};

PopupFactory *PopupFactory::instance()
{
    static PopupFactory instance;
    return &instance;
}

PopupBase *PopupFactory::createPopup(KeyButtonArea *mainArea) const
{
    return (plugin ? plugin->createPopup(mainArea)
                   : new DummyPopup(mainArea));
}

PopupFactory::PopupFactory()
    : plugin(0)
{
    // setup pluginloader
    QDir pluginsDir(PluginDir);
    QStringList filters;
    filters << "lib*.so";
    pluginsDir.setNameFilters(filters);
    QStringList fileList = pluginsDir.entryList();

    if (!fileList.isEmpty()) {
        // just load the first file
        QString fileName = fileList.at(0);
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *pluginInstance = loader.instance();

        plugin = qobject_cast<PopupPlugin *>(pluginInstance);

        if (!plugin) {
            qDebug() << "Error loading mvkb sub-plugin:" << loader.errorString();
        }
    }
}

PopupFactory::~PopupFactory()
{}
