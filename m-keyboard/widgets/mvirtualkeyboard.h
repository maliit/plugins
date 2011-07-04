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



#ifndef MVIRTUALKEYBOARD_H
#define MVIRTUALKEYBOARD_H

#include "keyeventhandler.h"
#include "mkeyboardcommon.h"
#include "mimkeyarea.h"
#include "layoutdata.h"
#include "reactionmappaintable.h"
#include "mwidget-wrapper.h"

#include <minputmethodnamespace.h>
#include <MWidget>
#include <MNamespace>
#include <mimenginetypes.h>
#include <QSharedPointer>
#include <QPointer>

class QPixmap;
class MCancelEvent;
class QGraphicsGridLayout;
class QGraphicsLinearLayout;
class QGraphicsWidget;
class QPropertyAnimation;
class QRectF;
class MButton;
class MScalableImage;
class MSceneManager;
class MVirtualKeyboardStyleContainer;
class HorizontalSwitcher;
class KeyEvent;
class LayoutsManager;
class Notification;
class MImKeyModel;
class VkbToolbar;
class MImToolbar;
class MReactionMap;
class MToolbarData;
class Handle;
class Grip;
class FlickGesture;
class SharedHandleArea;
class MKeyOverride;

/*!
  \class MVirtualKeyboard

  \brief The MVirtualKeyboard class provides interfaces for the usage of the
   virtual keyboard. The interfaces include hide/show/orientation of the keyboard.
   It also provides interfaces to get keystatus

*/
class MVirtualKeyboard : public MWidget, public ReactionMapPaintable
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
     * \brief Returns whether the symbol view is available for current layout.
     */
    bool symViewAvailable() const;

    /*!
     * Switch layout to given direction, \sa MAbstractInputMethod::swipe
     */
    void switchLayout(MInputMethod::SwitchDirection direction, bool enableAnimation);

    //! Set input method mode
    void setInputMethodMode(M::InputMethodMode mode);

    //! Returns whether autocaps is enabled.
    bool autoCapsEnabled() const;

    /*!
     * \brief Returns the keys in the main layout.
     */
    QList<MImEngine::KeyboardLayoutKey> mainLayoutKeys() const;

    //! reimp
    bool isPaintable() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    //! reimp_end

    //! \brief Resets different components of vkb to their initial states.
    void resetState();

    //! \brief Returns the amount of main keyboard layouts in use.
    int mainKeyboardCount() const;

    //! Show notification informing about current language
    void showLanguageNotification();

    //! Indicates whether an animation is playing.
    bool isPlayingAnimation();

    //! \brief Enable or disable horizontal gesture recognition.
    //! \sa HorizontalSwitcher::enableSinglePageHorizontalFlick(bool)
    void enableSinglePageHorizontalFlick(bool enable);

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

    //! Add vkb widget portions to the reaction map.
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * Method to change the orientation
     * \param orientation M::Orientation
     * \param force reorganize even when not visible
     */
    void organizeContent(M::Orientation orientation, bool force = false);

    void setLayout(int layoutIndex);

    /*!
     * \brief Uses custom key overrides which is defined by \a overrides.
     */
    void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides);

    /*!
     * \brief Sets the state of on off toggle key.
     */
    void setToggleKeyState(bool onOff);

    /*!
     * \brief Sets the state of compose key.
     */
    void setComposeKeyState(bool isComposing);

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

    void keyboardsReset();

    void numberKeyboardReset();

    void onSectionSwitchStarting(int current, int next);

    void onSectionSwitched(QGraphicsWidget *previous, QGraphicsWidget *current);

    void onVerticalAnimationFinished();

    /*!
     * \brief Creates new instance of Notification if previous one being deleted.
     *
     * Notification could be deleted, because it is a child of current active layout
     * and user could disable it through settings applet. In this case we have to
     * create new instance of Notification to be able to show it later when user
     * will change layout again.
     */
    void resurrectNotification();

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

    /*!
     * \brief Emitted when key is long pressed
     */
    void longKeyPressed(const KeyEvent &event);

    /*!
     * \brief Emitted when key is released due to internal event,
     * but not user activity.
     *
     * \param event key event
     *
     * \sa keyReleased
     */

    void keyCancelled(const KeyEvent &event);
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

    //! Emitted when shift state is changed
    void shiftLevelChanged();

    /*!
     * Inform that plugin should be switched in according to \a direction.
     */
    void pluginSwitchRequired(MInputMethod::SwitchDirection direction);

    /*!
     * Inform that active plugin should be replaced with specified one.
     * \param pluginName Name orf plugin which will be activated
     */
    void pluginSwitchRequired(const QString &pluginName);

protected:
    //! \reimp
#ifdef HAVE_MEEGOTOUCH
    virtual void cancelEvent(MCancelEvent *event);
#endif
    virtual void resizeEvent(QGraphicsSceneResizeEvent *);
    //! \reimp_end

private:
    void updateMainLayoutAtKeyboardIndex();

    //! Getter for style container
    const MVirtualKeyboardStyleContainer &style() const;

    /*!
     * \brief Maps \a offset to scene coordinate.
     */
    QPoint mapOffsetToScene(QPointF offset);

    const LayoutData *currentLayoutModel() const;

    MImAbstractKeyArea *keyboardWidget(int layoutIndex = -1) const;

    /*!
     * Paint the reactive areas of the buttons
     *
     * This does not include the layout keys.
     * It paints the reactive areas for the buttons
     * in the bottom row of the keyboard as well as
     * the close/minimize/hide button.
     */
    void drawButtonsReactionMaps(MReactionMap *reactionMap, QGraphicsView *view);

    //! creates a switcher for qwerty layouts
    void createSwitcher();

    //! Reloads sections to switcher with current layout type and orientation
    void reloadSwitcherContent();

    //! Creates a new section widget of given layout/layout type and orientation.
    MImAbstractKeyArea *createMainSectionView(const QString &layout,
                                         LayoutData::LayoutType,
                                         M::Orientation orientation,
                                         QGraphicsWidget *parent = 0);

    // creates a new section widget
    MImAbstractKeyArea *createSectionView(const QString &layout,
                                     LayoutData::LayoutType layoutType,
                                     M::Orientation orientation,
                                     const QString &section,
                                     bool usePopup,
                                     QGraphicsWidget *parent = 0);

    // recreates keyboard widgets
    void recreateKeyboards();

    // makes new keybutton areas for number and phone number
    void recreateSpecialKeyboards();

    //! Connect signals from a \a handle widget or whatever provides identical flick signals
    template <class T>
    void connectHandle(const T &handleLike);

    //! Sets the current content type (handles email/url overrides):
    void setContentType(M::TextContentType type);

    //! Play the vertical animation when VKB height is changed.
    void playVerticalAnimation(int animLine);

private:
    //! Main layout indices
    enum LayoutIndex {
        KeyboardHandleIndex,
        KeyboardIndex
    };

    //! Current Style being used
    const MVirtualKeyboardStyleContainer *styleContainer;

    QGraphicsLinearLayout *mainLayout;

    //! Current layout level
    int currentLevel;

    //! Max number of layout levels
    int numLevels;

#ifdef HAVE_MEEGOTOUCH
    //! Scene manager to get the device width and height
    MSceneManager *sceneManager;
#endif

    //! Shift key status
    ModifierState shiftState;

    LayoutData::LayoutType currentLayoutType;

    M::Orientation currentOrientation;

    QString currentLayout;

    const LayoutsManager &layoutsMgr;

    //! Switcher for showing the main qwerty section.
    HorizontalSwitcher *mainKeyboardSwitcher;

    QPointer<Notification> notification;

    MImAbstractKeyArea *numberKeyboard;
    MImAbstractKeyArea *phoneNumberKeyboard;

    QSharedPointer<QPixmap> backgroundPixmap;

    KeyEventHandler eventHandler;

    //! Contains true if multi-touch is enabled
    bool enableMultiTouch;

    //! Handle area containing toolbar widget.
    // MVirtualKeyboard is not owner of this object.
    QPointer<SharedHandleArea> sharedHandleArea;

    //! Contains current keyboard overrides
    QMap<QString, QSharedPointer<MKeyOverride> > overrides;

    //! Currently active content type for general layout:
    M::TextContentType generalContentType;

    bool toggleKeyState;
    bool composeKeyState;

    //! A vertical animation played only when switching to another VKB layout with different height.
    QPropertyAnimation *verticalAnimation;
    QRectF animHiddingArea;
    bool switchStarted;
};

#endif
