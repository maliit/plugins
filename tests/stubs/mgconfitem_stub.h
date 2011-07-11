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


#ifndef __MGCONFITEM_STUB_H__
#define __MGCONFITEM_STUB_H__

#include "fakegconf.h"

#include <mimsettings.h>
#include <QDebug>

/**
 * MImSettings stub class.
 * To fake MImSettings operations, derive from this class
 * and implement the methods you want to fake. Instantiate your
 * derived stub class and assign it to gMImSettingsStub
 * global variable.
 */
class MImSettingsStub
{
public:
    virtual void mImSettingsConstructor(const MImSettings *instance, const QString &key, QObject *parent = 0);
    virtual void mImSettingsDestructor(const MImSettings *instance);
    virtual QString key(const MImSettings *instance);
    virtual QVariant value(const MImSettings *instance);
    virtual QVariant value(const MImSettings *instance, const QVariant &def);
    virtual void set(const MImSettings *instance, const QVariant &val);
    virtual QList<QString> listDirs(const MImSettings *instance);
    virtual QList<QString> listEntries(const MImSettings *instance);

protected:
    QMap<const MImSettings *, QString> instanceKeys; // This map links MImSettings instance to its present key.
    FakeGConf fakeGConf; // This is the in-memory storage for settings.
};

void MImSettingsStub::mImSettingsConstructor(const MImSettings *instance, const QString &key, QObject *)
{
    if (!key.isEmpty()) {
        FakeGConfItem *fakeItem = fakeGConf.initKey(key);
        QObject::connect(fakeItem, SIGNAL(valueChanged()), instance, SIGNAL(valueChanged()));

        instanceKeys[instance] = key;
    }
}

void MImSettingsStub::mImSettingsDestructor(const MImSettings *instance)
{
    instanceKeys.remove(instance);
}

QString MImSettingsStub::key(const MImSettings *instance)
{
    return instanceKeys[instance];
}

QVariant MImSettingsStub::value(const MImSettings *instance)
{
    return fakeGConf.value(instanceKeys[instance]);
}

QVariant MImSettingsStub::value(const MImSettings *instance, const QVariant &def)
{
    QVariant val = fakeGConf.value(instanceKeys[instance]);
    if (val.isNull())
        val = def;
    return val;
}

void MImSettingsStub::set(const MImSettings *instance, const QVariant &value)
{
    fakeGConf.setValue(instanceKeys[instance], value);
}

QList<QString> MImSettingsStub::listDirs(const MImSettings *instance)
{
    return fakeGConf.listDirs(instanceKeys[instance]);
}

QList<QString> MImSettingsStub::listEntries(const MImSettings *instance)
{
    return fakeGConf.listEntries(instanceKeys[instance]);
}


/**
 * This is the stub class instance used by the system. If you want to alter behaviour,
 * derive your stub class from MImSettingsStub, implement the methods you want to
 * fake, create an instance of your stub class and assign the instance into this global variable.
 */
// this dynamic alloc for gMImSettingsStub will cause memory leak. But it is accaptable for
// unit test, because it is just a small memory leak. And this can avoid core dump if there are
// some static MImSettings object declared by application.
MImSettingsStub *gMImSettingsStub = new MImSettingsStub;

/**
 * These are the proxy method implementations of MImSettings. They will
 * call the stub object methods of the gMImSettingsStub.
 */

MImSettings::MImSettings(const QString &key, QObject *parent)
{
    gMImSettingsStub->mImSettingsConstructor(this, key, parent);
}

MImSettings::~MImSettings()
{
    gMImSettingsStub->mImSettingsDestructor(this);
}

QString MImSettings::key() const
{
    return gMImSettingsStub->key(this);
}

QVariant MImSettings::value() const
{
    return gMImSettingsStub->value(this);
}

QVariant MImSettings::value(const QVariant &def) const
{
    return gMImSettingsStub->value(this, def);
}

void MImSettings::set(const QVariant &val)
{
    gMImSettingsStub->set(this, val);
}

QList<QString> MImSettings::listDirs() const
{
    return gMImSettingsStub->listDirs(this);
}

QList<QString> MImSettings::listEntries() const
{
    return gMImSettingsStub->listEntries(this);
}

#endif //__MGCONFITEM_STUB_H__
