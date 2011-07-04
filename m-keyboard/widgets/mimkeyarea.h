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

#ifndef MIMKEYAREA_H
#define MIMKEYAREA_H

#include "mimabstractkeyarea.h"
#include "mimkey.h"

#include <QSize>
#include <QFontMetrics>
#include <QSharedPointer>

class MImKeyAreaPrivate;
class MImAbstractPopup;

//! \brief MImKeyArea reimplements MImAbstractKeyArea and is optimized for drawing.
class MImKeyArea
    : public MImAbstractKeyArea
{
    Q_OBJECT
    Q_DISABLE_COPY(MImKeyArea)

public:
    //! \brief Constructor, see \a MImAbstractKeyArea
    static MImKeyArea *create(const LayoutData::SharedLayoutSection &newSection,
                              bool usePopup,
                              QGraphicsWidget *parent);

    //! \brief Destructor
    virtual ~MImKeyArea();

    //! \reimp
    virtual void modifiersChanged(bool shift,
                                  const QChar &accent = QChar());
    virtual void paint(QPainter *painter,
                       const QStyleOptionGraphicsItem *option,
                       QWidget *view);
    virtual QRectF boundingRect() const;
    virtual void setShiftState(ModifierState newShiftState);
    virtual QList<const MImAbstractKey *> keys() const;
    virtual MImAbstractKey * findKey(const QString &id);
    virtual void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides);
    virtual void setContentType(MInputMethod::TextContentType type);
    virtual void setToggleKeyState(bool on);
    virtual void setComposeKeyState(bool isComposing);
    virtual void resetActiveKeys();
    //! \reimp_end

    //! \brief Locks down vertical movement.
    //!
    //! If enabled, only horizontal movement is used for key lookups, which
    //! will only return keys from first row.
    void lockVerticalMovement(bool enabled);

private slots:
    void updateKeyAttributes(const QString &, MKeyOverride::KeyOverrideAttributes);

protected:
    //! \brief Constructor, see \a MImAbstractKeyArea
    explicit MImKeyArea(const LayoutData::SharedLayoutSection &section,
                        QGraphicsWidget *parent = 0);

    //! \reimp
    virtual void init();
    virtual QSizeF sizeHint(Qt::SizeHint which,
                            const QSizeF &constraint) const;
    virtual void drawReactiveAreas(MReactionMap *reactionMap,
                                   QGraphicsView *view);
    virtual void updateKeyGeometries(int availableWidth);
    virtual MImAbstractKey *keyAt(const QPoint &pos) const;
    virtual void onThemeChangeCompleted();
    virtual void applyStyle();
    //! \reimp_end

    //! \brief Release given \a key if it is required by current attribure override
    void releaseKey(MImKey *key);

private:
    Q_DECLARE_PRIVATE(MImKeyArea);
    MImKeyAreaPrivate * const d_ptr;

#ifdef UNIT_TEST
    friend class Ut_MImAbstractKeyArea;
    friend class Ut_KeyEventHandler;
    friend class Ut_SymbolView;
    friend class Bm_MImAbstractKeyArea; //benchmarks
    friend class Bm_Painting;
#endif

    friend class FixtureVirtualKeyboard; //!< needed for testing
};

#endif // MIMKEYAREA_H
