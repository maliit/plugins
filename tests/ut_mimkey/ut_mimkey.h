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



#ifndef UT_MIMKEY_H
#define UT_MIMKEY_H

#include <mimkey.h>

#include <QtTest/QTest>
#include <QObject>
#include <QList>

class MApplication;
class MImKeyModel;
class MImKey;
class MImAbstractKeyAreaStyleContainer;
class QGraphicsItem;
class KeyboardData;
class MImFontPool;
class MImKeyStyle;

class Ut_MImKey: public QObject
{
    Q_OBJECT

public:
    enum Direction {
        Down,
        Up
    };

    struct KeyTriple {
        int index;
        MImAbstractKey::ButtonState state;
        int lastActiveIndex;

        // Needed for metatype registration:
        KeyTriple()
            : index(-1)
            , state(MImAbstractKey::Normal)
            , lastActiveIndex(-1)
        {}

        KeyTriple(int newIndex,
                  MImAbstractKey::ButtonState newState,
                  int newLastActiveIndex)
            : index(newIndex)
            , state(newState)
            , lastActiveIndex(newLastActiveIndex)
        {}

    };

    typedef QPair<Direction, bool> DirectionPair;
    typedef QList<MImKey *> KeyList;
    typedef QList<MImKeyModel *> ModelList;

private:

    MApplication *app;
    MImKey *subject;
    QGraphicsItem *parent;
    MImKeyModel *dataKey;
    MImAbstractKeyAreaStyleContainer *style;
    MImFontPool *fontPool;

private slots:
    void init();
    void cleanup();
    void initTestCase();
    void cleanupTestCase();

    void testSetModifier_data();
    void testSetModifier();
    void testKey();
    void testBinding();
    void testIsDead();

    void testTouchPointCount_data();
    void testTouchPointCount();
    void testResetTouchPointCount();

    void testActiveKeys_data();
    void testActiveKeys();
    void testResetActiveKeys();
    void testVisitActiveKeys();

    void testKeyRects();
    void testGravity();

    void testLabelOverride();
    void testKeyDisabling();

    void testOverrideBinding();

    void testIconInfo_data();
    void testIconInfo();

private:
    MImKey *createKey(bool state = false);
    MImKeyModel *createKeyModel();
    MImKey *createDeadKey(MImKeyModel *model, bool state = false);
    MImKeyModel *createDeadKeyModel(const QString &label);
    QSharedPointer<MImKey::StylingCache> stylingCache;
    MImKeyStyle *keyStyle;
};

Q_DECLARE_METATYPE(Ut_MImKey::DirectionPair)
Q_DECLARE_METATYPE(Ut_MImKey::KeyTriple)
Q_DECLARE_METATYPE(MImAbstractKey::ButtonState)

#endif

