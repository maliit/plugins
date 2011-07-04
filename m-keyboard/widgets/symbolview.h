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

#ifndef SYMBOLVIEW_H
#define SYMBOLVIEW_H

#include "keyeventhandler.h"
#include "mkeyboardcommon.h"
#include "layoutdata.h"
#include "reactionmappaintable.h"
#include "mwidget-wrapper.h"

#include <minputmethodnamespace.h>
#include <QPointer>

class MReactionMap;
#ifdef HAVE_MEEGOTOUCH
class MSceneManager;
#endif
class MVirtualKeyboardStyleContainer;
class HorizontalSwitcher;
class LayoutsManager;
class QGraphicsLinearLayout;
class QGraphicsSceneMouseEvent;
class KeyEvent;
class Handle;
class MKeyOverride;

/*!
 * \brief SymbolView is used to show different layouts symbols/upper case/lower case
 */
class SymbolView : public MWidget, public ReactionMapPaintable
{
    Q_OBJECT

public:
    enum ShowMode {
        NormalShowMode,         // Shows page normally
        FollowMouseShowMode     // Shows page temporarily, grabs the mouse, and generates mouse press
    };

    enum HideMode {
        NormalHideMode,         // Hides page normally
        TemporaryHideMode       // Hides page temporarily, page is hidden temporarily during screen rotation.
    };

    /*!
     * \brief Constructor for creating an SymbolView object.
     * \param baseSize QSize
     * \param parent Parent object.
     */
    SymbolView(const LayoutsManager &layoutsManager, const MVirtualKeyboardStyleContainer *,
               const QString &layout, QGraphicsWidget *parent = 0);

    /*!
     * Destructor
     */
    virtual ~SymbolView();

    void prepareToOrientationChange();

    void finalizeOrientationChange();

    void setShiftState(ModifierState newShiftState);

    //! Returns current level.
    int currentLevel() const;

    //! Return TRUE if widget is activated
    bool isActive() const;

    //! Tells whether symbol view is currently only temporarily opened,
    //! i.e. will close automatically on key release.
    bool isTemporarilyActive() const;

    /*! Returns interactive region currently occupied by SymbolView,
     *  in scene coordinates.
     */
    QRegion interactiveRegion() const;

    //! Returns the page count.
    int pageCount() const;

    //! Returns the index of current page.
    int currentPage() const;

    /*!
     * \brief If \a hidden is true, hides active symbol view. Otherwise, shows temporarily inactive symbol view.
     */
    void setTemporarilyHidden(bool hidden);

    /*! \reimp */
    bool isPaintable() const;
    /*! \reimp_end */

public slots:
    /*!
     * Handler to show the page.
     * If the page is already visible, then just return.
     * \param mode Value is NormalShowMode or FollowMouseShowMode. Default is NormalShowMode. \sa ShowMode.
     * \param initialScenePress Position in scene coordinates where mouse should initially get pressed.
     *        If mode is NormalShowMode this parameter is ignored. The location should be location of primary
     *        touch point because that's the point symbol view will follow.
     */
    void showSymbolView(ShowMode mode = NormalShowMode,
                        const QPointF &initialScenePress = QPointF());

    /*!
     * Handler to hide the page
     * \param mode Value is NormalHideMode or TemporaryHideMode. Default is NormalHideMode. \sa HideMode.
     */
    void hideSymbolView(HideMode mode = NormalHideMode);

    /*!
     * Changes between onscreen and hardware keyboard modes. Symbol view's contents depend on which one is used.
     * When hardware keyboard is used, symbol view has hw layout specific layout variant, and function row is hidden.
     * \param state New state to be applied.
     */
    void setKeyboardState(MInputMethod::HandlerState state);

    /*!
     * Method to load svc corresponding to given layout file name
     * \param layoutFile const QString&, new layout file name
     */
    void setLayout(const QString &layoutFile);

    /*!
     * \brief Selects the next page if current is not the rightmost.
     */
    void switchToNextPage();

    /*!
     * \brief Selects the previous page if current is not the leftmost.
     */
    void switchToPrevPage();

    //! \brief Paint symview's area to global reaction map.
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

    /*!
     * \brief Uses custom key overrides which is defined by \a overrides.
     */
    void setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides);

signals:
    //! Used to broadcast shift state to all pages/MImAbstractKeyAreas.
    void levelSwitched(int);

    /*!
     * Emitted on clicked on layout
     */
    void keyClicked(const KeyEvent &event);

    /*!
     * Emitted when key is pressed
     */
    void keyPressed(const KeyEvent &event);

    /*!
     * Emitted when key is released
     */
    void keyReleased(const KeyEvent &event);

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

    //! Emitted when user hides the keyboard, e.g. by pressing the close button
    void userInitiatedHide();

protected:
    /*! \reimp */
#ifdef HAVE_MEEGOTOUCH
    virtual void cancelEvent(MCancelEvent *event);
#endif
    virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *);
    virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
    /*! \reimp_end */

private slots:
    void onSwitchDone();

    //! Handler for shift pressed state change (separate from shift state).
    void handleShiftPressed(bool shiftPressed);

    //! Special handling for pressed keys, not handled through KeyEventHandler.
    void handleKeyPressed(const MImAbstractKey *key);

    //! Special handling for released keys, not handled through KeyEventHandler.
    void handleKeyReleased(const MImAbstractKey *key);

    //! Special handling for clicked keys, not handled through KeyEventHandler.
    void handleKeyClicked(const MImAbstractKey *);

    //! When hardware keyboard layout has changed, reload contents if currently in Hardware state.
    void handleHwLayoutChange();

private:
    //! Main layout indices
    enum LayoutIndex {
        GripIndex,
        KeyboardIndex
    };

    //! symbol view state wrt. \a showSymbolView / \a hideSymbolView calls.
    enum Activity {
        Active,                 // After showSymbolView(NormalShowMode) call
        TemporarilyActive,      // After showSymbolView(FollowMouseShowMode) call
        Inactive,               // After hideSymbolView(NormalHideMode) calls
        TemporarilyInactive     // After hideSymbolView(TemporaryHideMode)
    };

    //! Ensure that content, position and size are correct for the current orientation
    void organizeContent();

    //! \brief Creates necessary widget layouts where keys will be added.
    void setupLayout();

    //! \brief Reloads keys/buttons based on current language and orientation.
    void reloadContent();

    //! \brief Reloads switcher with pages from given \a layout, selecting the page \a selectPage.
    void loadSwitcherPages(const LayoutData *layout, unsigned int selectPage = 0);

    //! \brief Helper method to create and add a new page.
    void addPage(const LayoutData::SharedLayoutSection &symbolSection);

    //! \brief Creates MImAbstractKeyArea from given section model and connects appropriate signals.
    MImAbstractKeyArea *createMImAbstractKeyArea(const LayoutData::SharedLayoutSection &section,
                                                 bool enablePopup = true);

    //! Getter for style container
    const MVirtualKeyboardStyleContainer &style() const;

    //! Retrieves title of a symbol section from given page.
    QString pageTitle(int pageIndex) const;

    //! Connect signals from a \a handle widget
    void connectHandle(Handle *handle);

    void setActivity(Activity newActivity);

    void grabAndPressActiveKeyArea(const QPointF &initialPressLocation);

    //! Current style being used.
    const MVirtualKeyboardStyleContainer *styleContainer;

#ifdef HAVE_MEEGOTOUCH
    //! scene manager
    const MSceneManager &sceneManager;
#endif

    //! To check if symbol view is opened
    Activity activity;

    //! Zero-based index to currently active page
    int activePage;

    //! Case selector: 0 for lower case, 1 for upper case
    ModifierState shiftState;

    const LayoutsManager &layoutsMgr;

    QPointer<HorizontalSwitcher> pageSwitcher;

    M::Orientation currentOrientation;

    QString currentLayout;

    //! The main layout holds toolbar, handles and key button areas.
    QGraphicsLinearLayout *mainLayout;

    //! Handler for button events
    KeyEventHandler eventHandler;

    //! Contains true if multi-touch is enabled
    bool enableMultiTouch;

    MInputMethod::HandlerState activeState;

    bool hideOnQuickPick;
    bool hideOnSpaceKey;
    bool symKeyHeldDown;

    //! Contains current keyboard overrides
    QMap<QString, QSharedPointer<MKeyOverride> > overrides;

#ifdef UNIT_TEST
    friend class Ut_SymbolView;
#endif
};

#endif

