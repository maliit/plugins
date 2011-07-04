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


#ifndef __FAKEGCONF_H__
#define __FAKEGCONF_H__

#include <QObject>
#include <QStringList>
#include <QVariant>

/**
 * This file defines FakeGConf class that can be used to store settings
 * in treelike structure, like MImSettings does. All settings are
 * stored in-memory only and are valid during the lifetime of FakeGConf
 * instance.
 */

/*!
 * \class FakeGConfItem
 * \brief Stores one settings item that has name, and optionally
 *        value and children.
 *
 * This class is for the internal use of FakeGConf class.
 */
class FakeGConfItem : public QObject
{
    Q_OBJECT
public:
    FakeGConfItem(FakeGConfItem *parent, const QString &name);
    ~FakeGConfItem();

    FakeGConfItem *findChild(const QString &childName);
    QList<FakeGConfItem *> children();

    bool hasChildren();
    bool hasValueSet();

    QString name();
    QVariant value();
    void setValue(const QVariant &value);

signals:
    void valueChanged();

private:
    FakeGConfItem *parent;
    QList<FakeGConfItem *> childList;

    QString keyName;
    QVariant keyValue;
};

/*!
 * \class FakeGConf
 * \brief Simple in-memory version of gconf settings database.
 */
class FakeGConf
{
public:
    FakeGConf();
    ~FakeGConf();

    FakeGConfItem *initKey(const QString &key);

    void setValue(const QString &key, const QVariant &value);
    QVariant value(const QString &key);

    QStringList listDirs(const QString &key);
    QStringList listEntries(const QString &key);

private:
    FakeGConfItem root;
};


#endif // __FAKEGCONF_H__
