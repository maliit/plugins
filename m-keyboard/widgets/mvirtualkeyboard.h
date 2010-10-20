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



#ifndef MVIRTUALKEYBOARD_H
#define MVIRTUALKEYBOARD_H

#include "keyeventhandler.h"
#include "mkeyboardcommon.h"
#include "singlewidgetbuttonarea.h"
#include "layoutdata.h"

#include <minputmethodnamespace.h>
#include <MWidget>
#include <MNamespace>
#include <mimenginetypes.h>
#include <QPixmap>
#include <QSharedPointer>
#include <QTimeLine>
#include <QPointer>

class QGraphicsGridLayout;
class QGraphicsLinearLayout;
class QGraphicsWidget;
class MButton;
class MScalableImage;
class MSceneManager;
class MVirtualKeyboardStyleContainer;
class HorizontalSwitcher;
class KeyEvent;
class LayoutsManager;
class Notification;
class VKBDataKey;
class VkbToolbar;
class MImToolbar;
class MReactionMap;
class MToolbarData;
class Handle;
class Grip;
class FlickGesture;
class SharedHandleArea;

/*!
  \class MVirtualKeyboard

  \brief The MVirtualKeyboard class provides interfaces for the usage of the
   virtual keyboard. The interfaces include hide/show/orientation of the keyboard.
   It also provides interfaces to get keystatus

*/
class MVirtualKeyboard : public MWidget
{
    Q_OBJECT
    Q_PROPERTY(QString layoutLanguage READ layoutLanguage)
    Q_PROPERTY(QString layoutTitle READ layoutTitle)

    friend class Ut_MVirtualKeyboard;
    friend class Ut_MKeyboardHost;

public:
    /*!
     * \brief Constructor for creating an virtual keyboard object.
     * \param parent Parent object.
     */
    MVirtualKeyboard(const LayoutsManager &layoutsManager,
                     const MVirtualKeyboardStyleContainer *styleContainer,
                     QGraphicsWidget *parent = 0);

    //! Destructor
    ~MVirtualKeyboard();

    //! \reimp
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    //! \reimp_end

    //! \brief Tells whether keyboard is opened and not in the middle of show or hide animation.
    bool isFullyVisible() const;

    /*!
     * \brief Method to get the language for the currently displayed layout
     *
     * Note that this is the real language found in the XML loaded based on a language
     * list entry (which may be different).
     * \return the language
     */
    QString layoutLanguage() const;

    /*!
     * \brief Method to get the title for the currently displayed layout
     *
     * Note that this is the real title found in the XML loaded.
     * \return the title
     */
    QString layoutTitle() const;

    /*!
     * \brief Get the layout currently selected (from layout list)
     */
    QString selectedLayout() const;

    //! Sets keyboard type according text entry type, type matches M::TextContentType
    void setKeyboardType(const int type);

    // for unit tests
    //! Returns shift key status
    ModifierState shiftStatus() const;

    //! Characters defines word boundaries
    static const QString WordSeparators;

    //! Prepare virtual keyboard for orientation change
    void prepareToOrientationChange();

    //! Finalize orientation change
    void finalizeOrientationChange();

    /*!
     * \brief Shows or hides some keyboard's widgets depending on keyboard state.
     * OnScreen requires to show all keyboard components.
     * Any other value requires to hide everything except for toolbar.
     * \param newState actual state
     */
    void setKeyboardState(MInputMethod::HandlerState newState);

    /*!
     * \brief Returns current active state of the virtual keyboard.
     * The virtual keyboard has three kinds of modes, OnScreen, Hardware, and Accessory, \sa MInputMethod::HandlerState.
     * \sa setKeyboardState().
     */
    MInputMethod::HandlerState keyboardState() const;

    /*!
     * \brief Returns whether the symbol view is available for current layout.
     */
    bool symViewAvailable() const;

    /*!
     * Switch layout to given direction, \sa MInputMethodBase::swipe
     */
    void switchLayout(MInputMethod::SwitchDirection direction, bool enableAnimation);

    //! Set input method mode
    void setInputMethodMode(M::InputMethodMode mode);

    //! Return bounding rectangle of main keyboard area in scene coordinates
    QRect mainAreaSceneRect() const;

    //! Set pointer to shared handle area
    void setSharedHandleArea(const QPointer<SharedHandleArea> &newSharedHandleArea);

    //! Returns whether autocaps is enabled.
    bool autoCapsEnabled() const;

    /*!
     * \brief If \a hidden is true, hides active keyboard. Otherwise, shows temporarily inactive keyboard.
     */
    void setTemporarilyHidden(bool hidden);

    /*!
     * \brief Returns the keys in the main layout.
     */
    QList<M::KeyboardLayoutKey> mainLayoutKeys() const;

public slots:
    /*!
     * Method to switch level. Changes into next possible level.
     * Back to zero if maximum reached
     */
    void switchLevel();

    /*!
     * Method to set shift state
     */
    void setShiftState(ModifierState level);

    /*!
     * Method to Show the keyboard
     * \param fadeOnly just fade in if true, fade and slide otherwise
     */
    void showKeyboard(bool fadeOnly = false);

    /*!
     * Method to hide the keyboard
     * \param fadeOnly just fade out if true, fade and slide otherwise
     * \param temporary hide temporarily during screen rotation
     */
    void hideKeyboard(bool fadeOnly = false, bool temporary = false);

    //! Add vkb widget portions to the reaction map.
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * Method to change the orientation
     * \param orientation M::Orientation
     */
    void organizeContent(M::Orientation orientation);

    void setLayout(int layoutIndex);

    //! Hide main keyboard layout
    void hideMainArea();

    //! Show main kayboard layout
    void showMainArea();

private slots:
    /*!
     * \brief Handler for shift pressed state change (separate from shift state).
     */
    void handleShiftPressed(bool shiftPressed);

    /*!
     * Handler for right flick operation
     */
    void flickRightHandler();

    /*!
     * Handler for left flick operation
     */
    void flickLeftHandler();

    /*!
     * \brief Handler for upward flick operation
     * \param binding Key binding
     */
    void flickUpHandler(const KeyBinding &binding);

    /*!
     * Method to fade the vkb during transition
     */
    void fade(int);

    /*!
     * This function gets called when fading is finished
     */
    void showHideFinished();

    void keyboardsReset();

    void numberKeyboardReset();

    void onSectionSwitchStarting(int current, int next);

    void onSectionSwitched(QGraphicsWidget *previous, QGraphicsWidget *current);

    /*!
     * Send \a regionUpdated and \a inputMethodAreaUpdated signals with the
     * current region of this widget and its children combined, unless \a
     * suppressRegionUpdate has been used to suppress updates.
     */
    void sendVKBRegion();

    //! \brief Call organizeContent() and sendVKBRegion() if the vkb is visible
    void organizeContentAndSendRegion();

    //! Handle flick down gesture signals from handle areas
    void handleHandleFlickDown(const FlickGesture &gesture);

signals:
    /*!
     * \brief Emitted when key is pressed
     * Note that this happens also when user keeps finger down/mouse
     * button pressed and moves over another key (event is about the new key)
     * \param event key event
     */
    void keyPressed(const KeyEvent &event);

    /*!
     * \brief Emitted when key is released
     * Note that this happens also when user keeps finger down/mouse
     * button pressed and moves over another key (event is about the old key)
     * \param event key event
     */
    void keyReleased(const KeyEvent &event);

    /*!
     * \brief Emitted when user releases mouse button/lifts finger
     * Except when done on a dead key
     * \param event key event
     */
    void keyClicked(const KeyEvent &event);

    //! \see MInputMethodBase::regionUpdated()
    void regionUpdated(const QRegion &);

    //! \see MInputMethodBase::inputMethodAreaUpdated()
    void inputMethodAreaUpdated(const QRegion &);

    //! This signal is emitted when input layout is changed
    //! \param layout this is always the layout from XML file in unmodified form
    void layoutChanged(const QString &layout);

    //! Emitted when user hides the keyboard, e.g. by pressing the close button
    void userInitiatedHide();

    /*!
     * \brief This signal is emitted when copy/paste button is clicked
     * \param state CopyPasteState button action (copy or paste)
     */
    void copyPasteClicked(CopyPasteState action);

    //! Emitted when require a copy/paste action
    void copyPasteRequest(CopyPasteState);

    //! Emitted when require sending a keyevent
    void sendKeyEventRequest(const QKeyEvent &);

    //! Emitted when require sending a string
    void sendStringRequest(const QString &);

    //! Emitted when shift state is changed
    void shiftLevelChanged();

    //! Emitted after the widget has finished hiding.
    void hidden();

    //! Emitted when fully visible.
    void opened();

    //! Emitted when symbol view should be shown
    void showSymbolViewRequested();

    /*!
     * Inform that plugin should be switched in according to \a direction.
     */
    void pluginSwitchRequired(MInputMethod::SwitchDirection direction);

    /*!
     * Inform that active plugin should be replaced with specified one.
     * \param pluginName Name orf plugin which will be activated
     */
    void pluginSwitchRequired(const QString &pluginName);


private:
    void updateMainLayoutAtKeyboardIndex();

    //! Getter for style container
    const MVirtualKeyboardStyleContainer &style() const;

    /*!
     * \brief Calculate region occupied by keyboard.
     * \param notJustMainKeyboardArea Set this param to true if you want to include toolbar into region.
     * \param includeExtraInteractiveAreas Set this param to true if you want to include transparent
     * interactive area into region
     */
    QRegion region(bool notJustMainKeyboardArea = true) const;

    /*!
     * \brief Maps \a offset to scene coordinate.
     */
    QPoint mapOffsetToScene(QPointF offset);

    const LayoutData *currentLayoutModel() const;

    KeyButtonArea *keyboardWidget(int layoutIndex = -1) const;

    /*!
     * Method to setup timeline
     */
    void setupTimeLine();

    /*!
     * Paint the reactive areas of the buttons
     *
     * This does not include the layout keys.
     * It paints the reactive areas for the buttons
     * in the bottom row of the keyboard as well as
     * the close/minimize/hide button.
     */
    void drawButtonsReactionMaps(MReactionMap *reactionMap, QGraphicsView *view);

    /*! This can be set true if it is known that we're going
     * to have multiple region updates simultaneously.
     * It prevents vkb to send its update region. sendVKBRegion()
     * is called automatically when updates are enabled again.
     */
    void suppressRegionUpdate(bool suppress);

    //! creates a switcher for qwerty layouts
    void createSwitcher();

    //! Reloads sections to switcher with current layout type and orientation
    void reloadSwitcherContent();

    //! Creates a new section widget of given layout/layout type and orientation.
    KeyButtonArea *createMainSectionView(const QString &layout,
                                         LayoutData::LayoutType,
                                         M::Orientation orientation,
                                         QGraphicsWidget *parent = 0);

    // creates a new section widget
    KeyButtonArea *createSectionView(const QString &layout,
                                     LayoutData::LayoutType layoutType,
                                     M::Orientation orientation,
                                     const QString &section,
                                     bool usePopup,
                                     QGraphicsWidget *parent = 0);

    // recreates keyboard widgets
    void recreateKeyboards();

    // makes new keybutton areas for number and phone number
    void recreateSpecialKeyboards();

    //! \brief Resets different components of vkb to their initial states.
    void resetState();

    //! \brief Returns height depending on current mode. It includes
    // toolbar only for hardware keyboard or whole widget for virtual
    // keyboard
    int actualHeight() const;

    //! Connect signals from a \a handle widget or whatever provides identical flick signals
    template <class T>
    void connectHandle(const T &handleLike);

private:
    //! Main layout indices
    enum LayoutIndex {
        KeyboardHandleIndex,
        KeyboardIndex
    };

    //! Keyboard state wrt. \a showKeyboard / \a hideKeyboard calls.
    enum Activity {
        Active,                 // After showKeyboard call
        Inactive,               // After hidekeyboard(..., false)
        TemporarilyInactive     // After hideKeyboard(..., true)
    };

    //! Slow/hide animation constants
    enum {
        ShowHideFrames = 100,
#ifdef SLOW_TRANSITIONS         // for easier debugging
        ShowHideTime = 750,
        ShowHideInterval = 50
#else
        ShowHideTime = 250,
        ShowHideInterval = 20
#endif
    };

    //! Current Style being used
    const MVirtualKeyboardStyleContainer *styleContainer;

    QGraphicsLinearLayout *mainLayout;

    //! Current layout level
    int currentLevel;

    //! Max number of layout levels
    int numLevels;

    //! \see Activity.  Note that e.g. call to hideKeyboard() marks keyboard
    //! immediately Inactive but that doesn't mean the keyboard is immediately
    //! hidden, so activity does not strictly match visibility.
    Activity activity;

    //! Scene manager to get the device width and height
    MSceneManager *sceneManager;

    //! Shift key status
    ModifierState shiftState;

    // Vkb show hide time line
    QTimeLine showHideTimeline;

    LayoutData::LayoutType currentLayoutType;

    M::Orientation currentOrientation;

    QString currentLayout;

    bool hideShowByFadingOnly;

    //! Used by suppressRegionUpdate & sendVKBRegion
    bool sendRegionUpdates;
    bool regionUpdateRequested;

    const LayoutsManager &layoutsMgr;

    //! Switcher for showing the main qwerty section.
    HorizontalSwitcher *mainKeyboardSwitcher;

    Notification *notification;

    QGraphicsWidget *numberKeyboard;
    QGraphicsWidget *phoneNumberKeyboard;

    QSharedPointer<QPixmap> backgroundPixmap;

    MInputMethod::HandlerState activeState;

    KeyEventHandler eventHandler;

    //! Contains true if multi-touch is enabled
    bool enableMultiTouch;

    //! Handle area containing toolbar widget.
    // MVirtualKeyboard is not owner of this object.
    QPointer<SharedHandleArea> sharedHandleArea;
};

#endif
