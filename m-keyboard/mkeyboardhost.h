/* * This file is part of m-keyboard *
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



#ifndef MKEYBOARDHOST_H
#define MKEYBOARDHOST_H

#include "mkeyboardcommon.h"
#include "minputmethodbase.h"
#include "keyevent.h"
#include <MNamespace>
#include <QStringList>
#include <QTimer>
#include <QPointer>

class MFeedbackPlayer;
class MGConfItem;
class MImCorrectionCandidateWidget;
class MSceneWindow;
class MVirtualKeyboard;
class MVirtualKeyboardStyleContainer;
class MHardwareKeyboard;
class SymbolView;
class MImEngineWordsInterface;
class QWidget;
class MBanner;
class SharedHandleArea;
class MImToolbar;


//! Logic class for virtual keyboard
class MKeyboardHost: public MInputMethodBase
{
    Q_OBJECT

public:
    MKeyboardHost(MInputContextConnection *icConnection, QObject *parent = 0);
    virtual ~MKeyboardHost();

    //! reimp
    virtual void focusChanged(bool focusIn);
    virtual void show();
    virtual void hide();
    virtual void setPreedit(const QString &preeditString);
    virtual void update();
    virtual void reset();
    virtual void mouseClickedOnPreedit(const QPoint &mousePos, const QRect &preeditRect);
    virtual void visualizationPriorityChanged(bool priority);
    virtual void appOrientationChanged(int angle);
    virtual void setToolbar(QSharedPointer<const MToolbarData> toolbar);
    virtual void setState(const QSet<MInputMethod::HandlerState> &state);
    virtual void processKeyEvent(QEvent::Type keyType, Qt::Key keyCode,
                                 Qt::KeyboardModifiers modifiers,
                                 const QString &text, bool autoRepeat, int count,
                                 quint32 nativeScanCode, quint32 nativeModifiers);
    virtual void clientChanged();
    virtual void switchContext(MInputMethod::SwitchDirection direction, bool enableAnimation);
    virtual QList<MInputMethodBase::MInputMethodSubView> subViews(MInputMethod::HandlerState state
                                                                  = MInputMethod::OnScreen) const;
    virtual void setActiveSubView(const QString &,
                                  MInputMethod::HandlerState state = MInputMethod::OnScreen);
    virtual QString activeSubView(MInputMethod::HandlerState state = MInputMethod::OnScreen) const;
    //! reimp_end

private slots:
    /*!
     * Handle key clicks from widgets
     * \param event internal key event
     */
    void handleKeyClick(const KeyEvent &event);

    /*!
     * Send QKeyEvent when key is pressed in direct input mode
     * \param event internal key event
     */
    void handleKeyPress(const KeyEvent &event);

    /*!
     * Send QKeyEvent when key is released in direct input mode
     * \param event internal key event
     */
    void handleKeyRelease(const KeyEvent &event);

    //! \brief Draws reaction maps for the topmost widget.
    void updateReactionMaps();

    /*!
     * Confirm \a string to replace preedit
     */
    void confirmPreedit(const QString &string);

    /*! \brief Prepares vkb for orientation change when application is about to rotate.
     *
     * This should hide vkb.
     */
    void prepareOrientationChange();

    /*! \brief Finalizes orientation change after application has rotated.
     *
     * This should popup vkb if it was visible before orientation change.
     */
    void finalizeOrientationChange();

    //! Synchronize correction setting
    void synchronizeCorrectionSetting();

    //! handles user initiated hiding of the keyboard
    void userHide();

    //! does one backspace and schedules the next if it is holding backspace.
    void autoBackspace();

    /*! \brief Sends request to copy or paste text
     *  \param action ImCopyPasteState Required action (copy or paste)
     */
    virtual void sendCopyPaste(CopyPasteState action);

    /*! Receive region updates from widgets, combine them and signal as input method's region
     * using \a MInputMethodBase::regionUpdated.
     *
     * \param region updated region
     */
    void handleRegionUpdate(const QRegion &region);

    //! This overloaded function handles region updates from sharedHandleArea.
    void handleRegionUpdate();

    /*!
     * Receive region updates from widgets, combine them and signal as input
     * method area using \a MInputMethodBase::inputMethodAreaUpdated.
     *
     * \param region updated region
     */
    void handleInputMethodAreaUpdate(const QRegion &region);

    //! This overloaded function handles region updates from sharedHandleArea.
    void handleInputMethodAreaUpdate();

    //! Sends key event
    void sendKeyEvent(const QKeyEvent &);

    //! Sends string
    void sendString(const QString &);

    //! Handle symbol key click.
    void handleSymbolKeyClick();

    //! Updates the shift level for Symbol view.
    void updateSymbolViewLevel();

    //! Shows symbol view
    void showSymbolView();

    /*!
     * Receives modifier state changed signal or script changed signal  from hardware
     * keyboard, sends input mode indicator state notification to Application Framework
     * (Home screen status bar).
     */
    void handleHwKeyboardStateChanged();

    //! show FN/Caps Lock infobanner
    void showLockOnInfoBanner(const QString &notification);

    //! hide FN/Caps Lock infobanner
    void hideLockOnInfoBanner();

    //! Handle active layout is changed to \a layout for virtual keyboard.
    void handleVirtualKeyboardLayoutChanged(const QString &layout);

private:
    //! \brief Reset internal state, used by reset() and others
    void resetInternalState();

    void createCorrectionCandidateWidget();

    //! Rotates coodinates from screen to window
    bool rotatePoint(const QPoint &screen, QPoint &window);
    bool rotateRect(const QRect &screenRect, QRect &windowRect);

    //! Update error correction flag
    void updateCorrectionState();

    //! update autocapitalization state
    void updateAutoCapitalization();

    //! update context
    void updateContext();

    /*!
     * Reset temporary shift state (shift on state set by user or auto capitalization,
     * besides capslocked) for virtual keyboard.
     */
    void resetVirtualKeyboardShiftState();

    //! Actual backspace operation
    void doBackspace();

    // shows the layout menu
    void showLayoutMenu();

    /*! \brief Handle key click event that changes the state of the keyboard.
     *
     *  This method should contain functionality that is common to
     *  both directmode and non-directmode.
     *  \sa handleTextInputKeyClick
     */
    void handleGeneralKeyClick(const KeyEvent &event);

    /*! \brief Handle key click event that relates to sending text.
     *
     *  This key click handling is not called in direct mode since
     *  it causes extra text to be sent to input context.
     *  \sa handleGeneralKeyClick
     */
    void handleTextInputKeyClick(const KeyEvent &event);

    /*! \brief Clears reaction maps with given MReactionMap color value.
     *  \param clearValue A MReactionMap color value such as MReactionMap::Inactive.
     */
    void clearReactionMaps(const QString &clearValue);

    /*! Mapping from QObject to region.
     */
    typedef QPair<QPointer<QObject>, QRegion> ObjectRegionPair;

    /*! Container of region mapping information.
     */
    typedef QList<ObjectRegionPair> RegionList;

    /*!
     * \brief Save \a region occupied by \a widget into \a regionStore.
     */
    void setRegionInfo(RegionList &regionStore,
                       const QRegion &region,
                       const QPointer<QObject> &widget);
    /*!
     * \brief Return union of all regions in \a regionStore after adding sharedHandleArea region.
     */
    QRegion combineRegion();

    /*!
     * \brief Return union of all regions in \a regionStore after adding sharedHandleArea region.
     */
    QRegion combineInputMethodArea();

    /*!
     * \brief Common implementation for combineRegion() and combineInputMethodArea().
     * \param includeExtraInteractiveAreas Result includes extra interactive area
     * if this parameter is true.
     */
    QRegion combineRegionImpl(const RegionList &regionStore,
                              bool includeExtraInteractiveAreas);


    //! initialize input engine
    void initializeInputEngine();

    //! \return input mode indicator corresponding to a dead \a key character or
    //! MInputMethodBase::NoIndicator if not a (supported) dead key character
    static MInputMethodBase::InputModeIndicator deadKeyToIndicator(const QChar &key);

    //! update preedit style according error correction candidates.
    void updatePreeditStyle();

private:
    class CycleKeyHandler; //! Reacts to cycle key press events.
    friend class CycleKeyHandler;
    QString preedit;

    MVirtualKeyboardStyleContainer *vkbStyleContainer;

    MImCorrectionCandidateWidget *correctionCandidateWidget;
    MVirtualKeyboard *vkbWidget;
    MHardwareKeyboard *hardwareKeyboard;
    SymbolView *symbolView;

    MImEngineWordsInterface *imCorrectionEngine;
    //! default input method error correction setting
    MGConfItem *inputMethodCorrectionSettings;
    MGConfItem *inputMethodCorrectionEngine;

    QStringList candidates;
    bool engineReady;

    M::OrientationAngle angle;
    int displayWidth;
    int displayHeight;

    bool rotationInProgress;

    //! error correction flag
    bool correctionEnabled;

    //! Feedback player instance
    MFeedbackPlayer *feedbackPlayer;

    //! FIXME: should we provide such a flag to on/off auto caps
    bool autoCapsEnabled;
    //! Contains true if autocapitalization decides to switch keyboard to upper case
    bool autoCapsTriggered;
    QString surroundingText;
    int cursorPos;

    int inputMethodMode;

    QTimer backSpaceTimer;

    KeyEvent lastClickEvent;

    //! Keeps track of shift up/down status.
    bool shiftHeldDown;

    MSceneWindow *sceneWindow;
#ifdef M_IM_DISABLE_TRANSLUCENCY
    QWidget *correctionWindow;
    MSceneWindow *correctionSceneWindow;
#endif

    //! Regions of widgets created by MKeyboardHost
    RegionList widgetRegions;

    //! Regions of widgets that affect the input method area
    RegionList inputMethodAreaWidgetRegions;

    //! current active state
    MInputMethod::HandlerState activeState;

    QPointer<MBanner> modifierLockOnBanner; //! widget to notify modifier is in locked state

    //! Indicates whether focus is in a widget according to focusChanged calls
    //! \sa focusChanged
    bool haveFocus;

    //! Contains true if multi-touch is enabled
    bool enableMultiTouch;

    //! Handle area containing toolbar widget.
    QPointer<SharedHandleArea> sharedHandleArea;

    //! Toolbar widget containing copy/paste and custom buttons.
    QPointer<MImToolbar> imToolbar;

    CycleKeyHandler *cycleKeyHandler;

    bool currentIndicatorDeadKey;

#ifdef UNIT_TEST
    friend class Ut_MKeyboardHost;
#endif
};

#endif
