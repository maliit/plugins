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

#ifndef MIMABSTRACTKEYAREA_H
#define MIMABSTRACTKEYAREA_H

#include "mkeyboardcommon.h"
#include "mimabstractkey.h"
#include "mimabstractkeyareastyle.h"
#include "layoutdata.h"
#include "style-wrapper.h"

#ifdef HAVE_MEEGOTOUCH
#include <MFeedback>
#endif

#include <QList>
#include <QStringList>
#include <QTouchEvent>
#include <QTimer>
#include <QTime>
#include <QGraphicsView>

struct KeyContext;
class FlickGesture;
class MReactionMap;
class MImAbstractPopup;
class MKeyOverride;
class MImAbstractKeyAreaPrivate;

//! \brief MImAbstractKeyArea is a view for virtual keyboard layout represented by LayoutModel
class MImAbstractKeyArea
    : public MStylableWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(MImAbstractKeyArea)

public:
    //! \brief Destructor
    virtual ~MImAbstractKeyArea();

    //! \brief Returns section shown by this key area.
    const LayoutData::SharedLayoutSection &sectionModel() const;

    //! \brief Returns current level of this layout.
    int level() const;

    //! \brief Exposes style used by this key area.
    const MImAbstractKeyAreaStyleContainer &baseStyle() const;

    //! \brief Sets input method mode for all MImAbstractKeyArea instances.
    //! \param inputMethodMode the new input method mode
    static void setInputMethodMode(MInputMethod::InputMethodMode inputMethodMode);

    //! \brief Returns relative button base width
    qreal relativeKeyBaseWidth() const;

    //! \brief Returns all keys from this key area.
    virtual QList<const MImAbstractKey *> keys() const = 0;

    //! \brief Returns key with given \a id
    virtual MImAbstractKey * findKey(const QString &id) = 0;

    //! \brief Notification for derived classes about button modifier change.
    //!
    //! Derived classes should not change the level of selected dead keys. This is to
    //! ensure all dead keys can be used with all characters in every level.
    //! \param shift whether shift modifier is enabled
    //! \param accent which accented version should be used, for a key
    virtual void modifiersChanged(bool shift,
                                  const QChar &accent = QChar());

    //! \brief Resets active keys belonging to this key area to normal state.
    virtual void resetActiveKeys() = 0;

    //! \brief Enable or disable horizontal gesture recognition.
    virtual void enableHorizontalFlick(bool enable);

    //! \brief Returns true if \key belongs to this key area.
    virtual bool contains(const MImAbstractKey *key) const;

public slots:
    //! \brief Tell key area to switch levels for all keys.
    //! \param level the new level
    void switchLevel(int level);

    //! \brief Set shift state.
    //! \param shiftState the new shift state
    virtual void setShiftState(ModifierState shiftState);

    //! \brief Draw reactive areas for all keys.
    //! \param reactionMap the reaction map to draw onto
    //! \param view the view to be used
    virtual void drawReactiveAreas(MReactionMap *reactionMap,
                                   QGraphicsView *view);

    //! \brief Unlock all locked dead keys.
    //! \param deadKey the corresponding dead key
    void unlockDeadKeys(MImAbstractKey *deadKey);

    //! \brief Hide popup
    void hidePopup();

    /*!
     * \brief Uses custom key overrides which is defined by \a overrides.
     */
    virtual void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides) = 0;

    /*!
     * \brief Sets the current content type (handles email/url overrides).
     */
    virtual void setContentType(MInputMethod::TextContentType type) = 0;

    /*!
     * \brief Sets the state of on off toggle key.
     */
    virtual void setToggleKeyState(bool on) = 0;

    /*!
     * \brief Sets the state of compose key.
     */
    virtual void setComposeKeyState(bool isComposing) = 0;

    //! \brief Sets popup.
    //! \param popup the popup instance. MImAbstractKeyArea takes ownership.
    //! \sa MImAbstractPopup::setMainArea
    void setPopup(MImAbstractPopup *popup);

    //! \brief Returns popup
    const MImAbstractPopup *popup() const;

signals:
    //! \brief Emitted when key is pressed
    //!
    //! Note that this happens also when user keeps finger down/mouse
    //! button pressed and moves over another key (event is about the new key)
    //! \param key describes pressed button
    //! \param keyContext Context information at the time key was pressed
    void keyPressed(const MImAbstractKey *key,
                    const KeyContext &keyContext);

    //! \brief Emitted when key is released.
    //!
    //! Note that this happens also when user keeps finger down/mouse
    //! button pressed and moves over another key (event is about the old key)
    //! \param key describes released button
    //! \param keyContext Context information at the time key was released
    void keyReleased(const MImAbstractKey *key,
                     const KeyContext &keyContext);

    //! \brief Emitted when user releases mouse button/lifts finger.
    //!
    //! Except when done on a dead key
    //! \param key describes clicked button
    //! \param keyContext Context information at the time key was clicked
    void keyClicked(const MImAbstractKey *key,
                    const KeyContext &keyContext);

    //! \brief Emitted when long press is detected.
    //!
    //! Long press detection is:
    //! - cancelled when latest pressed key is released;
    //! - restarted when finger is moved to other key;
    //! - restarted when new touch point is recognized by MImAbstractKeyArea.
    //! \param key describes pressed button
    //! \param keyContext Context information at the time key was longpressed
    void longKeyPressed(const MImAbstractKey *key,
                        const KeyContext &keyContext);

    //!
    //! \brief Emitted when key is released due to internal event,
    //! but not user activity.
    //!
    //! \param key describes cancelled button
    //! \param keyContext Context information at the time key was cancelled
    //!
    //! \sa keyReleased
    void keyCancelled(const MImAbstractKey *key,
                      const KeyContext &keyContext);

    //! \brief Emitted when key area is flicked right.
    void flickRight();

    //! \brief Emitted when key area is flicked left.
    void flickLeft();

    //! \brief Emitted when key area is flicked down.
    void flickDown();

    //! \brief Emitted if button width has changed
    //! \param baseWidth base width used for relative button widths
    void relativeKeyBaseWidthChanged(qreal baseWidth);

protected:
    //! \brief Constructor
    //! \post Object is not fully initialized.  \a init must be called to finalize initialization.
    //! \a init must not be called in the constructor of a derived class but by the eventual
    //! instantiator of the derived class.
    //! \param privateData Pointer to private class.
    //! MImAbstractKeyArea takes ownership on private data class, so derived classes
    //! should NOT delete private objects.
    //! \param parent Key area's parent.
    explicit MImAbstractKeyArea(MImAbstractKeyAreaPrivate *privateData,
                                QGraphicsWidget *parent = 0);

    //! \brief Completes initialization.
    virtual void init();

    //! \reimp
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual QVariant itemChange(GraphicsItemChange change,
                                const QVariant &value);

    virtual void grabMouseEvent(QEvent *event);
    virtual void ungrabMouseEvent(QEvent *event);
// TODO: We need a new way to handle cancel events from e.g., gestures.
#ifdef HAVE_MEEGOTOUCH
    virtual void cancelEvent(MCancelEvent *);
#endif
    virtual bool event(QEvent *event);
    //! \reimp_end

    /*!
     * \brief Resets keyboard and release active keys.
     *
     * Doesn't reset latched/locked shift state.  Cancels shift press (that has not been
     * released).
     */
    void reset();

    //! \brief Called when key area's visibility changed.
    //! \param visible the new visbility status
    virtual void handleVisibilityChanged(bool visible);

    //! Shows popup and updates its content and position.
    //! \param key current key
    void updatePopup(MImAbstractKey *key = 0);

    //! \brief Get maximum number of columns in this key area.
    int maxColumns() const;

    //! \brief Get number of rows in this key area.
    int rowCount() const;

    //! \brief Updates button labels and/or icons according to current level
    //!        and deadkey.
    //! \param accent the accent version of a dead key.
    void updateKeyModifiers(const QChar &accent = QChar());

    //! \brief Returns key at given \a pos.
    //!
    //! Accepts positions outside widget geometry because
    //! of reactive margins.
    //! \param pos the position (in key area space) to look up key
    virtual MImAbstractKey *keyAt(const QPoint &pos) const = 0;

    //! \brief Updates key (and row) geometry based on given \a availableWidth.
    //! \param availableWidth with of the key area
    virtual void updateKeyGeometries(int availableWidth) = 0;

    //! \brief Log touch point information to
    //!        $HOME/.meego-im/vkb-touchpoints.log, for debugging purposes.
    //! \param tp the touch point
    //! \param key key underneath touch point
    //! \param lastKey last key that was associated to the touch point
    void logTouchPoint(const QTouchEvent::TouchPoint &tp,
                       const MImAbstractKey *key,
                       const MImAbstractKey *lastKey = 0) const;

    qreal mRelativeKeyBaseWidth; //!< Relative key base width in currently active layout
    bool debugTouchPoints; //!< Whether touch point debugging is enabled

    //! Correct the vertical offset of a touchpoint
    //! \param scenePos Input position in scene coordinates.
    //! \returns the corrected position in item coordinates.
    QPoint correctedTouchPoint(const QPointF &scenePos) const;

    //! Correct the reaction rects for the vertical offset of a touchpoint
    //! \param originalRect Original rectangle in item coordinates.
    //! \returns the corrected rectangle in item coordinates.
    QRectF correctedReactionRect(const QRectF &originalRect) const;

protected slots:
    //! Update background images, text layouts, etc. when the theme changed.
    virtual void onThemeChangeCompleted();

    //! Handle long press on the key
    virtual void handleLongKeyPressed();

    //! Handle idle VKB
    virtual void handleIdleVkb();

protected:
    Q_DECLARE_PRIVATE(MImAbstractKeyArea);
    MImAbstractKeyAreaPrivate * const d_ptr;

private:
#ifdef HAVE_MEEGOTOUCH
    M_STYLABLE_WIDGET(MImAbstractKeyAreaStyle)
#endif

#ifdef UNIT_TEST
    friend class MReactionMapTester;
    friend class Ut_MImAbstractKeyArea;
    friend class Ut_SymbolView;
    friend class Ut_MVirtualKeyboard;
    friend class Bm_MImAbstractKeyArea; //benchmarks
    friend class Bm_Painting;
#endif
};

/*! \brief This structure packs contextual information about
 *         a key that was pressed/released/clicked.
 */
struct KeyContext
{
    KeyContext()
        : upperCase(false),
          isFromPrimaryTouchPoint(false)
    {
    }

    KeyContext(bool upperCase, const QString &accent = QString(),
               const QPointF &scenePos = QPointF(),
               const QPoint correctionPos = QPoint(),
               bool primaryTouchPoint = false)
       : upperCase(upperCase),
         accent(accent),
         scenePos(scenePos),
         errorCorrectionPos(correctionPos),
         isFromPrimaryTouchPoint(primaryTouchPoint)
    {
    }

    bool upperCase;            //!< Whether key area was considered to be in upper case level.
    QString accent;            //!< Active accent, if any.
    QPointF scenePos;          //!< Accurate scene position of the key's hit point.
    QPoint errorCorrectionPos; //!< Hit point of the key in layout coordinates,
                               //!  tweaked suitable for error correction engine.
    bool isFromPrimaryTouchPoint; //!< Whether key was invoked with primary touch point.
};

Q_DECLARE_METATYPE(KeyContext)

#endif
