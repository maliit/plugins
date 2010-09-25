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



#ifndef KEYBUTTONAREA_H
#define KEYBUTTONAREA_H

#include "ikeybutton.h"
#include "keyevent.h"
#include "layoutdata.h"
#include "vkbdatakey.h"
#include "mkeyboardcommon.h"

#include <MWidget>
#include <QColor>
#include <QHash>
#include <QList>
#include <QStringList>
#include <QTouchEvent>

class FlickGesture;
class MFeedbackPlayer;
class MReactionMap;
class MScalableImage;
class MVirtualKeyboardStyleContainer;
class LayoutData;
class QGraphicsLinearLayout;
class QTextLayout;
class PopupBase;

/*!
 * \class KeyButtonArea
 * \brief KeyButtonArea is a view for virtual keyboard layout represented by LayoutModel
 */
class KeyButtonArea : public MWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(KeyButtonArea)

public:
    /*!
    * \brief Constructor
    * \param style The shared VKB style container.
    * \param sectionModel A section model that this KeyButtonArea visualizes.
    * \param usePopup Sets whether popup should be used when long press occurs.
    * \param parent The widget's parent.
    */
    KeyButtonArea(const MVirtualKeyboardStyleContainer *style,
                  const LayoutData::SharedLayoutSection &sectionModel,
                  bool usePopup = false,
                  QGraphicsWidget *parent = 0);

    //! \brief Destructor
    virtual ~KeyButtonArea();

    /*!
     * \brief Checks whether popup is currently turned on.
     */
    bool isPopupActive() const;

    //! \return layout model
    const LayoutData::SharedLayoutSection &sectionModel() const;

    //! Returns current level of this layout.
    int level() const;

    //! Set input method mode for all KeyButtonArea instances
    static void setInputMethodMode(M::InputMethodMode inputMethodMode);

public slots:
    /*!
     * This slot is used to switch levels
     */
    void switchLevel(int level);

    virtual void setShiftState(ModifierState newShiftState);

    virtual void drawReactiveAreas(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * \brief unlock all locked deadkeys
     */
    void unlockDeadkeys();

signals:

    /*!
     * \brief Emitted when key is pressed
     * Note that this happens also when user keeps finger down/mouse
     * button pressed and moves over another key (event is about the new key)
     * \param key describes pressed button
     * \param accent label of pressed dead key if any
     * \param upperCase contains true if key is in uppercase state
     */
    void keyPressed(const IKeyButton *key, const QString &accent, bool upperCase);

    /*!
     * \brief Emitted when key is released
     * Note that this happens also when user keeps finger down/mouse
     * button pressed and moves over another key (event is about the old key)
     * \param key describes released button
     * \param accent label of pressed dead key if any
     * \param upperCase contains true if key is in uppercase state
     */
    void keyReleased(const IKeyButton *key, const QString &accent, bool upperCase);

    /*!
     * \brief Emitted when user releases mouse button/lifts finger
     * Except when done on a dead key
     * \param key describes clicked button
     * \param accent label of pressed dead key if any
     * \param upperCase contains true if key is in uppercase state
     */
    void keyClicked(const IKeyButton *key, const QString &accent, bool upperCase);

    //! Emitted when flicked right
    void flickRight();

    //! Emitted when flicked left
    void flickLeft();

    //! Emitted when flicked down
    void flickDown();

    /*!
     * \brief Emitted when flicked up
     * \param binding Information about the key where mouse button was pressed
     */
    void flickUp(const KeyBinding &binding);

protected:
    //! Stores touch point specific information.
    struct TouchPointInfo {
        TouchPointInfo();

        //! True if finger is held down and is inside the layout's area.
        bool fingerInsideArea;

        //! Currently held down key
        IKeyButton *activeKey;

        //! Key that this touch point was first pressed on, if any.
        IKeyButton *initialKey;

        //! Initial position of the touch point when it was first pressed.
        QPoint initialPos;

        //! Last known position of the touch point
        QPoint pos;
    };

    /*! \reimp */
    virtual void resizeEvent(QGraphicsSceneResizeEvent *event);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    virtual QVariant itemChange(GraphicsItemChange, const QVariant &);
    virtual void grabMouseEvent(QEvent *e);
    virtual void ungrabMouseEvent(QEvent *e);
    virtual bool event(QEvent *event);
    /*! \reimp_end */

    //! Called when widget is about to lose visibility.
    virtual void onHide();


    //! Shows popup and updates its content and position.
    void updatePopup(const QPoint &pos, const IKeyButton *key = 0);

    /*!
    * \brief Verifies should we process cursor movement or not
    * \param pos QPointF new cursor position
    * \return bool
    */
    bool isObservableMove(const QPointF &prevPos, const QPointF &pos);

    /*!
    * \brief Get level count of the virtual keyboard.
    * \return int. The level count.
    */
    int maxColumns() const;

    /*!
    * \brief Get row count of the virtual keyboard, in current level.
    * \return int. The row count.
    */
    int rowCount() const;

    //! \brief Updates button labels and/or icons according to current level and deadkey.
    void updateButtonModifiers();

    /*! \brief Notification for derived classes about button modifier change.
     *
     *  Derived classes should not change the level of selected dead keys. This is to
     *  ensure all dead keys can be used with all characters in every level.
     */
    virtual void modifiersChanged(bool shift, QChar accent = QChar());

    //! \brief Returns key at given \a pos.
    //!
    //! Accepts positions outside widget geometry because
    //! of reactive margins.
    virtual IKeyButton *keyAt(const QPoint &pos) const = 0;

    /*! \brief Calculates button and row geometry based on given \a availableWidth.
     *  \post Button rectangle cache and row width cache are up to date.
     */
    virtual void updateButtonGeometriesForWidth(int availableWidth) = 0;

    //! \brief Getter for style container
    const MVirtualKeyboardStyleContainer &style() const;

    const PopupBase &popupWidget() const;

    //! Sets button state and sends release & press events.
    void setActiveKey(IKeyButton *key, TouchPointInfo &tpi);

    void clearActiveKeys();

protected slots:
    //! Update background images, text layouts, etc. when the theme changed.
    virtual void onThemeChangeCompleted();

private:
    //! \brief Handler for flick gestures from Qt gesture framework.
    void handleFlickGesture(FlickGesture *gesture);

    //! \brief Touch point press handler.
    //! \brief pos Current touchpoint position.
    //! \param id Touch point identifier defined by the system.
    void touchPointPressed(const QPoint &pos, int id);

    //! \brief Touch point move handler.
    //! \brief pos Current touchpoint position.
    //! \param id Touch point identifier defined by the system.
    void touchPointMoved(const QPoint &pos, int id);

    //! \brief Touch point release handler.
    //! \brief pos Current touchpoint position.
    //! \param id Touch point identifier defined by the system.
    void touchPointReleased(const QPoint &pos, int id);

    //! \brief Gravitational key lookup
    IKeyButton *gravitationalKeyAt(const QPoint &pos, IKeyButton *activeKey) const;

    void click(IKeyButton *key);

    //! Current level
    int currentLevel;

    //! Popup to show additional information for a button
    PopupBase *popup;

    //! style
    const MVirtualKeyboardStyleContainer *styleContainer;

    //! Touch point id of the most recent press event.
    int newestTouchPointId;

    //! List of punctuation labels
    QList<QStringList> punctuationsLabels;

    //! List of accent labels
    QList<QStringList> accentLabels;

    //! Whether a gesture was already triggered for any active touch point.
    bool wasGestureTriggered;

    //! Keys are QTouchEvent::TouchPoint id
    QMap<int, TouchPointInfo> touchPoints;

    bool enableMultiTouch;

    //! Activated dead key
    IKeyButton *activeDeadkey;

    /*!
     * Feedback player instance
     */
    MFeedbackPlayer *feedbackPlayer;

    //! layout section viewed by this class
    const LayoutData::SharedLayoutSection section;

    const bool usePopup;

    static M::InputMethodMode InputMethodMode;

#ifdef UNIT_TEST
    friend class MReactionMapTester;
    friend class Ut_KeyButtonArea;
    friend class Ut_SymbolView;
    friend class Bm_KeyButtonArea; //benchmarks
#endif
};

#endif
