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

#include "flickgesturerecognizer.h"
#include "mkeyboardhost.h"
#include "mkeyboardhost_p.h"
#include "mvirtualkeyboardstyle.h"
#include "mvirtualkeyboard.h"
#include "mhardwarekeyboard.h"
#ifdef M_IM_DISABLE_TRANSLUCENCY
#include "mimcorrectioncandidatewindow.h"
#endif
#include "mimcorrectioncandidatewidget.h"
#include "keyboarddata.h"
#include "layoutsmanager.h"
#include "symbolview.h"
#include "mimtoolbar.h"
#include "sharedhandlearea.h"

#include <mimenginewordsinterfacefactory.h>
#include <minputcontextconnection.h>
#include <mplainwindow.h>
#include <mtoolbardata.h>
#include <mgconfitem.h>
#include <mtheme.h>

#include <QDebug>
#include <QCoreApplication>
#include <QKeyEvent>
#include <QFile>
#include <QRegExp>

#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <mreactionmap.h>
#include <MScene>
#include <MSceneManager>
#include <MSceneWindow>
#include <MBanner>
#include <MLocale>
#include <MBreakIterator>
#include <MLibrary>

M_LIBRARY

namespace
{
    const QString InputMethodList("MInputMethodList");
    const QString DefaultInputLanguage("en_GB");
    // TODO: check that these paths still hold
    const QString InputMethodCorrectionSetting("/meegotouch/inputmethods/virtualkeyboard/correctionenabled");
    bool DefaultInputMethodCorrectionSettingOption = true;
    const QString InputMethodCorrectionEngine("/meegotouch/inputmethods/correctionengine");
    const QString AutoCapsSentenceDelimiters(".?!¡¿"); // used as regexp character set content!
    const int MaximumErrorCorrectionCandidate = 5;
    const int RotationDuration = 750; //! After vkb hidden, how long to wait until shown again
    const int AutoBackspaceDelay = 500;      // in ms
    const int LongTapspaceDelay = 1000;      // in ms
    const int BackspaceRepeatInterval = 100; // in ms
    const int MultitapTime = 1500;           // in ms
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
}

MKeyboardHost::CycleKeyHandler::CycleKeyHandler(MKeyboardHost &parent)
    : QObject(&parent),
    host(parent),
    timer(),
    cycleText(),
    prevEvent()
{
    timer.setSingleShot(true);
    timer.setInterval(MultitapTime);
    connect(&timer, SIGNAL(timeout()), this, SLOT(commitCycleKey()));
}

MKeyboardHost::CycleKeyHandler::~CycleKeyHandler()
{
}

bool MKeyboardHost::CycleKeyHandler::handleTextInputKeyClick(const KeyEvent &event)
{
    // If there's a pending keypress, handle it:
    if (timer.isActive() && !(event == prevEvent)) {
        timer.stop();
        commitCycleKey();
    }

    if (event.specialKey() != KeyEvent::CycleSet) {
        return false;  // Host should handle non-cyclekeys.
    }

    if (event.text().length() == 0) {
        qWarning() << __PRETTY_FUNCTION__ << "Empty cycleset in layout";
        return true;
    }

    if (event == prevEvent) {
        // Cycling through same cycle key
        if (host.preedit.length() > 0) {
            // FIXME: appending the cycle key to the preedit --> pressing cycle key
            // when there already is something in preedit will cause the whole preedit
            // to be commited after timeout
            host.setPreedit(host.preedit.left(host.preedit.length() - 1));
        }
        cycleIndex = (cycleIndex + 1) % cycleText.size();
    }
    else {
        // Different key than last time, reset index and cycle text
        prevEvent = event;
        cycleIndex = 0;
        cycleText = event.text();
    }

    host.preedit += cycleText[cycleIndex];
    host.inputContextConnection()->sendPreeditString(host.preedit,
                                                     MInputMethod::PreeditNoCandidates);

    timer.start();

    return true;
}

void MKeyboardHost::CycleKeyHandler::commitCycleKey()
{
    if (cycleText.length() > 0) {
        host.sendString(host.preedit);
        host.preedit.clear();
        cycleText.clear();
        prevEvent = KeyEvent();
    }
}

MKeyboardHost::MKeyboardHost(MInputContextConnection *icConnection, QObject *parent)
    : MInputMethodBase(icConnection, parent),
      vkbStyleContainer(0),
      correctionCandidateWidget(0),
      vkbWidget(0),
      symbolView(0),
      imCorrectionEngine(0),
      inputMethodCorrectionSettings(new MGConfItem(InputMethodCorrectionSetting)),
      inputMethodCorrectionEngine(new MGConfItem(InputMethodCorrectionEngine)),
      engineReady(false),
      angle(M::Angle0),
      rotationInProgress(false),
      correctionEnabled(false),
      feedbackPlayer(0),
      autoCapsEnabled(true),
      autoCapsTriggered(false),
      cursorPos(-1),
      inputMethodMode(M::InputMethodModeNormal),
      backSpaceTimer(),
      spaceTimer(),
      shiftHeldDown(false),
      activeState(MInputMethod::OnScreen),
      modifierLockOnBanner(0),
      haveFocus(false),
      enableMultiTouch(false),
      cycleKeyHandler(new CycleKeyHandler(*this)),
      currentIndicatorDeadKey(false)
{
    displayHeight = MPlainWindow::instance()->visibleSceneSize(M::Landscape).height();
    displayWidth  = MPlainWindow::instance()->visibleSceneSize(M::Landscape).width();

    enableMultiTouch = MGConfItem(MultitouchSettings).value().toBool();

    sceneWindow = new MSceneWindow;
    sceneWindow->setManagedManually(true); // we want the scene window to remain in origin

    // This will add scene window as child of MSceneManager's root element
    // which is the QGraphicsItem that is rotated when orientation changes.
    // It uses animation to carry out the orientation change transform
    // (e.g. rotation and position animation). We do this because transform
    // happens in the scene, not in the view (MWindow) anymore.
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(sceneWindow);

    // Because we set vkbWidget as a child of sceneWindow the vkb
    // will always be in correct orientation. However the animation will be
    // affected as well. If we want to keep the current hiding/showing animation
    // (up & down) without getting it combined with the rotation animation
    // we have at least two options:
    // 1) Make our own MOrientationAnimation when libmeegotouch begins supporting
    //    setting it, through theme probably.
    // 2) Add widgets directly to scene (detached from MSceneManager) and
    //    update their transformations by hand.

    vkbStyleContainer = new MVirtualKeyboardStyleContainer;
    vkbStyleContainer->initialize("MVirtualKeyboard", "MVirtualKeyboardView", 0);

    LayoutsManager::createInstance();

    FlickGestureRecognizer::registerSharedRecognizer();

    vkbWidget = new MVirtualKeyboard(LayoutsManager::instance(), vkbStyleContainer, sceneWindow);
    vkbWidget->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));

    connect(vkbWidget, SIGNAL(keyClicked(const KeyEvent &)),
            this, SLOT(handleKeyClick(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyPressed(const KeyEvent &)),
            this, SLOT(handleKeyPress(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyReleased(const KeyEvent &)),
            this, SLOT(handleKeyRelease(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(showSymbolViewRequested()),
            this, SLOT(showSymbolView()));

    connect(vkbWidget, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleRegionUpdate(const QRegion &)));
    connect(vkbWidget, SIGNAL(inputMethodAreaUpdated(const QRegion &)),
            this, SLOT(handleInputMethodAreaUpdate(const QRegion &)));

    connect(vkbWidget, SIGNAL(userInitiatedHide()),
            this, SLOT(userHide()));

    connect(vkbWidget, SIGNAL(pluginSwitchRequired(MInputMethod::SwitchDirection)),
            this, SIGNAL(pluginSwitchRequired(MInputMethod::SwitchDirection)));

    // construct hardware keyboard object
    hardwareKeyboard = new MHardwareKeyboard(*icConnection, this);
    connect(hardwareKeyboard, SIGNAL(symbolKeyClicked()),
            this, SLOT(handleSymbolKeyClick()));

    bool ok = connect(vkbWidget, SIGNAL(copyPasteClicked(CopyPasteState)),
                      this, SLOT(sendCopyPaste(CopyPasteState)));
    Q_UNUSED(ok); // if Q_NO_DEBUG is defined then the assert won't be used
    Q_ASSERT(ok);

    imToolbar = new MImToolbar;

    ok = connect(imToolbar, SIGNAL(copyPasteRequest(CopyPasteState)),
                 this, SLOT(sendCopyPaste(CopyPasteState)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(sendKeyEventRequest(const QKeyEvent &)),
                 this, SLOT(sendKeyEvent(const QKeyEvent &)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(sendStringRequest(const QString &)),
                 this, SLOT(sendString(const QString &)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(copyPasteClicked(CopyPasteState)),
                 this, SLOT(sendCopyPaste(CopyPasteState)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(closeKeyboardRequest()),
                 this, SLOT(userHide()));
    Q_ASSERT(ok);

    sharedHandleArea = new SharedHandleArea(*imToolbar, sceneWindow);
    sharedHandleArea->resize(MPlainWindow::instance()->visibleSceneSize().width(),
                             sharedHandleArea->size().height());
    ok = connect(imToolbar, SIGNAL(regionUpdated()),
                 sharedHandleArea, SLOT(updatePositionAndRegion()));
    Q_ASSERT(ok);
    sharedHandleArea->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));

    ok = connect(sharedHandleArea, SIGNAL(regionUpdated()),
                 this, SLOT(handleRegionUpdate()));
    Q_ASSERT(ok);

    ok = connect(sharedHandleArea, SIGNAL(inputMethodAreaUpdated()),
                 this, SLOT(handleInputMethodAreaUpdate()));
    Q_ASSERT(ok);

    // Set z value below default level (0.0) so popup will be on top of shared handle area.
    sharedHandleArea->setZValue(-1.0);

    vkbWidget->setSharedHandleArea(sharedHandleArea);
    sharedHandleArea->watchOnWidget(vkbWidget);


    createCorrectionCandidateWidget();

    // Don't listen to device orientation.  Applications can be in different orientation
    // than the device (especially plain qt apps). See NB#185013 - Locking VKB orientation.
    MPlainWindow::instance()->lockOrientationAngle();

    // Ideally we would adjust the hiding/showing animation of vkb according to
    // animation of the application receiving input. For example, with 3-phase
    // MBasicOrientationAnimation we would probably want to sync like this:
    // 1) Navigation bar hiding (altough it's probably already hidden) -> vkb hiding
    // 2) Rotation in progress -> vkb not visible
    // 3) Navigation bar showing -> vkb showing
    connect(MPlainWindow::instance()->sceneManager(),
            SIGNAL(orientationAngleChanged(M::OrientationAngle)),
            SLOT(prepareOrientationChange()));

    // orientationChangeFinished is emitted on every angle change,
    // not only orientation change.
    connect(MPlainWindow::instance()->sceneManager(),
            SIGNAL(orientationChangeFinished(M::Orientation)),
            SLOT(finalizeOrientationChange()));

    symbolView = new SymbolView(LayoutsManager::instance(), vkbStyleContainer,
                                vkbWidget->selectedLayout(), sceneWindow);
    connect(symbolView, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleRegionUpdate(const QRegion &)));
    connect(symbolView, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleInputMethodAreaUpdate(const QRegion &)));

    connect(symbolView, SIGNAL(updateReactionMap()),
            this, SLOT(updateReactionMaps()));

    connect(symbolView, SIGNAL(keyClicked(const KeyEvent &)),
            this, SLOT(handleKeyClick(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyPressed(const KeyEvent &)),
            this, SLOT(handleKeyPress(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyReleased(const KeyEvent &)),
            this, SLOT(handleKeyRelease(const KeyEvent &)));

    symbolView->setSharedHandleArea(sharedHandleArea);
    sharedHandleArea->watchOnWidget(symbolView);

    connect(vkbWidget, SIGNAL(layoutChanged(const QString &)),
            this, SLOT(handleVirtualKeyboardLayoutChanged(const QString &)));

    connect(vkbWidget, SIGNAL(shiftLevelChanged()),
            this, SLOT(updateSymbolViewLevel()));

    connect(hardwareKeyboard, SIGNAL(shiftStateChanged()),
            this, SLOT(updateSymbolViewLevel()));

    connect(vkbWidget, SIGNAL(copyPasteRequest(CopyPasteState)),
            this, SLOT(sendCopyPaste(CopyPasteState)));
    connect(vkbWidget, SIGNAL(sendKeyEventRequest(const QKeyEvent &)),
            this, SLOT(sendKeyEvent(const QKeyEvent &)));
    connect(vkbWidget, SIGNAL(sendStringRequest(const QString &)),
            this, SLOT(sendString(const QString &)));

    if (!inputMethodCorrectionEngine->value().isNull()) {
        imCorrectionEngine = MImEngineWordsInterfaceFactory::instance()->createEngine(
                                inputMethodCorrectionEngine->value().toString());

        if (imCorrectionEngine) {
            engineReady = true;
            initializeInputEngine();
            connect(inputMethodCorrectionSettings, SIGNAL(valueChanged()),
                    this, SLOT(synchronizeCorrectionSetting()));
        } else {
            qDebug() << __PRETTY_FUNCTION__ << "Failed to load correction engine"
                     << inputMethodCorrectionEngine->value().toString();
        }
    }

    feedbackPlayer = MComponentData::feedbackPlayer();

    backSpaceTimer.setSingleShot(true);
    connect(&backSpaceTimer, SIGNAL(timeout()), this, SLOT(autoBackspace()));

    spaceTimer.setSingleShot(true);
    connect(&spaceTimer, SIGNAL(timeout()), this, SLOT(longTapSpace()));

    // hide main layout when symbol view is shown to improve performance
    connect(symbolView, SIGNAL(opened()), vkbWidget, SLOT(hideMainArea()));
    connect(symbolView, SIGNAL(aboutToHide()), vkbWidget, SLOT(showMainArea()));
}

MKeyboardHost::~MKeyboardHost()
{
    hideLockOnInfoBanner();
    delete hardwareKeyboard;
    hardwareKeyboard = 0;
    delete vkbWidget;
    vkbWidget = 0;
    delete symbolView;
    symbolView = 0;
    delete correctionCandidateWidget;
    correctionCandidateWidget = 0;
    delete sceneWindow;
    sceneWindow = 0;
    delete vkbStyleContainer;
    vkbStyleContainer = 0;
#ifdef M_IM_DISABLE_TRANSLUCENCY
    delete correctionSceneWindow;
    correctionSceneWindow = 0;
    delete correctionWindow;
    correctionWindow = 0;
#endif
    delete inputMethodCorrectionSettings;
    inputMethodCorrectionSettings = 0;
    if (imCorrectionEngine) {
        MImEngineWordsInterfaceFactory::instance()->deleteEngine(imCorrectionEngine);
        imCorrectionEngine = 0;
    }
    backSpaceTimer.stop();
    LayoutsManager::destroyInstance();
}

void MKeyboardHost::createCorrectionCandidateWidget()
{
#ifdef M_IM_DISABLE_TRANSLUCENCY
    // Use a separate translucent window for correction candidate widget
    correctionWindow = new MImCorrectionCandidateWindow();
    MWindow *correctionView = new MWindow(new MSceneManager, correctionWindow);
    // Enable translucent in hardware rendering
    correctionView->setTranslucentBackground(!MApplication::softwareRendering());

    // No auto fill in software rendering
    if (MApplication::softwareRendering())
        correctionView->viewport()->setAutoFillBackground(false);
    QSize sceneSize = correctionView->visibleSceneSize(M::Landscape);
    int w = correctionView->visibleSceneSize().width();
    int h = correctionView->visibleSceneSize().height();
    correctionView->scene()->setSceneRect(0, 0, w, h);
    correctionWindow->resize(sceneSize);
    correctionView->setMinimumSize(1, 1);
    correctionView->setMaximumSize(w, h);

    correctionSceneWindow = new MSceneWindow;
    correctionSceneWindow->setManagedManually(true); // we want the scene window to remain in origin
    correctionView->sceneManager()->appearSceneWindowNow(correctionSceneWindow);

    // construct correction candidate widget
    correctionCandidateWidget = new MImCorrectionCandidateWidget(correctionSceneWindow);
    correctionCandidateWidget->hide();
    connect(correctionCandidateWidget, SIGNAL(regionUpdated(const QRegion &)),
            correctionWindow, SLOT(handleRegionUpdate(const QRegion &)));
#else
    // construct correction candidate widget
    correctionCandidateWidget = new MImCorrectionCandidateWidget();
    correctionCandidateWidget->hide();

    connect(correctionCandidateWidget, SIGNAL(regionUpdated(const QRegion &)),
            this, SLOT(handleRegionUpdate(const QRegion &)));
#endif
    connect(correctionCandidateWidget, SIGNAL(candidateClicked(const QString &)),
            this, SLOT(confirmPreedit(const QString &)));
}


void MKeyboardHost::focusChanged(bool focusIn)
{
    haveFocus = focusIn;
    if (activeState == MInputMethod::OnScreen) {
        if (focusIn) {
            // reset the temporary shift state when focus is changed
            resetVirtualKeyboardShiftState();
        }
    } else {
        if (focusIn) {
            hardwareKeyboard->enable();
        } else {
            hardwareKeyboard->disable();
        }
        if (!focusIn) {
            sendInputModeIndicator(MInputMethodBase::NoIndicator);
        }
        hideLockOnInfoBanner();
    }

    if (focusIn) {
        sharedHandleArea->show();
    } else {
        sharedHandleArea->hide();
    }
}


void MKeyboardHost::show()
{
    QWidget *p = MPlainWindow::instance();

    if (p->nativeParentWidget()) {
        p = p->nativeParentWidget();
    }

    p->raise(); // make sure the window gets displayed

    if (activeState == MInputMethod::Hardware) {
        if (!hardwareKeyboard->symViewAvailable())
            symbolView->hideSymbolView();
    } else {
        //Onscreen state
        if (!vkbWidget->symViewAvailable())
            symbolView->hideSymbolView();
    }

    // FIXME: This is from time when toolbar was attached on top of vkb widget
    // and vbk was needed to be shown, even when the position was under visible
    // screen. This is no longer the case and the following showKeyboard() should
    // not be called on hardware mode. What does host::show() even mean in hw mode?
    vkbWidget->showKeyboard();

    updateCorrectionState();
}


void MKeyboardHost::hide()
{
    correctionCandidateWidget->hideWidget();
    symbolView->hideSymbolView();
    vkbWidget->hideKeyboard();
}


void MKeyboardHost::setPreedit(const QString &preeditString)
{
    preedit = preeditString;
    candidates.clear();
    if (imCorrectionEngine) {
        imCorrectionEngine->clearEngineBuffer();
        imCorrectionEngine->appendString(preeditString, true);
        candidates = imCorrectionEngine->candidates();
    }
}


void MKeyboardHost::update()
{
    bool valid = false;

    const bool hasSelection = inputContextConnection()->hasSelection(valid);
    if (valid) {
        imToolbar->setSelectionStatus(hasSelection);
    }

    const int type = inputContextConnection()->contentType(valid);
    if (valid) {
        hardwareKeyboard->setKeyboardType(static_cast<M::TextContentType>(type));
        vkbWidget->setKeyboardType(type);
    }

    updateAutoCapitalization();
    updateContext();

    const int inputMethodModeValue = inputContextConnection()->inputMethodMode(valid);
    if (valid) {
        inputMethodMode = inputMethodModeValue;
        hardwareKeyboard->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
        vkbWidget->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
        sharedHandleArea->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
    }
}


void MKeyboardHost::resetVirtualKeyboardShiftState()
{
    // reset the temporary shift state (shift on state set by user or auto capitalization,
    // besides capslocked)
    if (activeState == MInputMethod::OnScreen && vkbWidget->shiftStatus() != ModifierLockedState) {
        autoCapsTriggered = false;
        vkbWidget->setShiftState(ModifierClearState);
    }
}

void MKeyboardHost::updateAutoCapitalization()
{
    switch (activeState) {
    case MInputMethod::OnScreen:
        autoCapsEnabled = vkbWidget->autoCapsEnabled();
        break;
    default:
        autoCapsEnabled = hardwareKeyboard->autoCapsEnabled();
        break;
    }
    bool valid = false;
    const int type = inputContextConnection()->contentType(valid);
    autoCapsEnabled = (autoCapsEnabled
                       && valid
                       && (type != M::NumberContentType)
                       && (type != M::PhoneNumberContentType));
    autoCapsEnabled = (autoCapsEnabled
                       && inputContextConnection()->autoCapitalizationEnabled(valid)
                       && valid);
    autoCapsEnabled = (autoCapsEnabled
                       && inputContextConnection()->surroundingText(surroundingText, cursorPos));

    if (!autoCapsEnabled)
        return;

    // Capitalization is determined by preedit and Auto Capitalization.
    // If there are some preedit, it should be lower case.
    // Otherwise Auto Capitalization will turn on shift when (text entry capitalization option is ON):
    //   1. at the beginning of one paragraph
    //   2. after a sentence delimiter and one or more spaces
    static const QRegExp autoCapsTrigger("[" + AutoCapsSentenceDelimiters + "] +$");
    autoCapsTriggered = ((preedit.length() == 0)
                         && ((cursorPos == 0)
                             || ((cursorPos > 0)
                                 && (cursorPos <= surroundingText.length())
                                 && surroundingText.left(cursorPos).contains(autoCapsTrigger))));

    if ((activeState == MInputMethod::OnScreen)
        && (vkbWidget->shiftStatus() != ModifierLockedState)) {
        // FIXME: This will break the behaviour of keeping shift latched when shift+character occured.
        // We would really need a state machine for the shift state handling.
        vkbWidget->setShiftState(autoCapsTriggered ?
                                 ModifierLatchedState : ModifierClearState);
    } else if ((activeState == MInputMethod::Hardware) &&
               (hardwareKeyboard->modifierState(Qt::ShiftModifier) != ModifierLockedState)) {
        hardwareKeyboard->setAutoCapitalization(autoCapsTriggered);
    }
}

void MKeyboardHost::updateContext()
{
    if (!correctionEnabled || !preedit.isEmpty()) {
        return;
    }

    QString context;
    bool valid = false;
    const int type = inputContextConnection()->contentType(valid);
    if (valid && (type != M::NumberContentType)
        && (type != M::PhoneNumberContentType)) {
        if (inputContextConnection()->surroundingText(surroundingText, cursorPos)) {
            MLocale locale;
            context = surroundingText.left(cursorPos);
            if (context.endsWith(" ")) {
                context = context.trimmed();
                MBreakIterator it(locale, context, MBreakIterator::WordIterator);
                it.setIndex(context.length());
                context = context.right(context.length() - it.previous()).trimmed();
            } else {
                context.clear();
            }
        }
    }
    qDebug() << "set context:" << context;
    imCorrectionEngine->setContext(context);
}

void MKeyboardHost::reset()
{
    qDebug() << __PRETTY_FUNCTION__;
    switch (activeState) {
    case MInputMethod::OnScreen:
        resetInternalState();
        break;
    case MInputMethod::Hardware:
        hardwareKeyboard->reset();
        break;
    case MInputMethod::Accessory:
        break;
    }
}


void MKeyboardHost::resetInternalState()
{
    backSpaceTimer.stop();
    preedit.clear();
    candidates.clear();
    correctionCandidateWidget->setPreeditString("");
    correctionCandidateWidget->disappear();
    if (engineReady)
        imCorrectionEngine->clearEngineBuffer();
}

void MKeyboardHost::prepareOrientationChange()
{
    if (rotationInProgress) {
        return;
    }
    rotationInProgress = true;

    symbolView->prepareToOrientationChange();
    vkbWidget->prepareToOrientationChange();
    correctionCandidateWidget->prepareToOrientationChange();
}

void MKeyboardHost::finalizeOrientationChange()
{
    angle = MPlainWindow::instance()->orientationAngle();

    if (imToolbar) {
        // load proper layout
        imToolbar->finalizeOrientationChange();
    }

    vkbWidget->finalizeOrientationChange();
    symbolView->finalizeOrientationChange();
    if (sharedHandleArea) {
        sharedHandleArea->finalizeOrientationChange();
    }

    // Finalize candidate list after so its region will apply.
    correctionCandidateWidget->finalizeOrientationChange();

    // If correction candidate widget was open we need to reposition it.
    if (correctionCandidateWidget->isVisible()) {
        bool success = false;
        const QRect rect = inputContextConnection()->preeditRectangle(success);
        QRect localRect;
        // Note: For Qt applications we don't have means to retrieve
        // the correct coordinates for pre-edit rectangle, so rect here
        // is null.
        if (success && !rect.isNull() && rotateRect(rect, localRect)) {
            correctionCandidateWidget->setPosition(localRect);
        } else {
            correctionCandidateWidget->hideWidget();
        }
    }
    // reload keyboard layout for engine when orientation is changed
    if (vkbWidget->isVisible()) {
        imCorrectionEngine->loadKeyboardLayout(vkbWidget->mainLayoutKeys());
    }

    rotationInProgress = false;
}

bool MKeyboardHost::rotatePoint(const QPoint &screen, QPoint &window)
{
    bool res = true;

    switch (angle) {
    case M::Angle90:
        window.setX(screen.y());
        window.setY(displayWidth - screen.x());
        break;
    case M::Angle270:
        window.setX(displayHeight - screen.y());
        window.setY(screen.x());
        break;
    case M::Angle180:
        window.setX(displayWidth - screen.x());
        window.setY(displayHeight - screen.y());
        break;
    case M::Angle0:
        window.setX(screen.x());
        window.setY(screen.y());
        break;
    default:
        qCritical() << __FILE__ << __LINE__ << "Incorrect orientation" << angle;
        res = false;
        break;
    }
    return res;
}


bool MKeyboardHost::rotateRect(const QRect &screenRect, QRect &windowRect)
{
    bool res = true;

    if (!screenRect.isValid()) {
        windowRect = QRect();
        return false;
    }

    switch (angle) {
    case M::Angle90:
        windowRect.setRect(screenRect.y(),
                           displayWidth - screenRect.x() - screenRect.width(),
                           screenRect.height(), screenRect.width());
        break;
    case M::Angle270:
        windowRect.setRect(displayHeight - screenRect.y() - screenRect.height(),
                           screenRect.x(),
                           screenRect.height(), screenRect.width());
        break;
    case M::Angle180:
        windowRect.setRect(displayWidth - screenRect.x() - screenRect.width(),
                           displayHeight - screenRect.y() - screenRect.height(),
                           screenRect.width(), screenRect.height());
        break;
    case M::Angle0:
        windowRect = screenRect;
        break;
    default:
        qCritical() << __FILE__ << __LINE__ << " Incorrect orientation " << angle;
        windowRect = QRect();
        res = false;
        break;
    }
    return res;
}


void MKeyboardHost::mouseClickedOnPreedit(const QPoint &mousePos, const QRect &preeditRect)
{
    // Shows suggestion list when there are some canidates.
    // Even show suggestion list when there is only original input word in candidates.
    if (candidates.size() <= 0)
        return;

    correctionCandidateWidget->setPreeditString(preedit);
    correctionCandidateWidget->setCandidates(candidates);

    if (!correctionCandidateWidget->isVisible()) {
        QPoint localMousePos;
        QRect localRect;

        // Use preeditRect if one was passed (not null).
        if (!preeditRect.isNull() && rotateRect(preeditRect, localRect)) {
            correctionCandidateWidget->setPosition(localRect);
        } else if (rotatePoint(mousePos, localMousePos)) {
            correctionCandidateWidget->setPosition(localMousePos);
        } else {
            return;
        }
    }

    correctionCandidateWidget->showWidget(MImCorrectionCandidateWidget::SuggestionListMode);
}


void MKeyboardHost::visualizationPriorityChanged(bool priority)
{
    vkbWidget->setTemporarilyHidden(priority);
    symbolView->setTemporarilyHidden(priority);
}


void MKeyboardHost::appOrientationChanged(int angle)
{
    // The application receiving input has changed its orientation. Let's change ours.
    MPlainWindow::instance()->setOrientationAngle((M::OrientationAngle)angle);
}


void MKeyboardHost::confirmPreedit(const QString &updatedString)
{
    qDebug() << __PRETTY_FUNCTION__;

    preedit = updatedString;

    const MInputMethod::PreeditFace face = MInputMethod::PreeditDefault;
    inputContextConnection()->sendPreeditString(updatedString, face);
    correctionCandidateWidget->setPreeditString("");

    if (candidates.count() > 1) {
        int suggestionIndex = candidates.indexOf(correctionCandidateWidget->suggestion());
        if (suggestionIndex >= 0) {
            qDebug() << "save index:" << suggestionIndex;
            imCorrectionEngine->setSuggestedCandidateIndex(suggestionIndex);
        }
    }
    imCorrectionEngine->saveAndClearEngineBuffer();
    inputContextConnection()->sendCommitString(preedit);
    preedit.clear();
}


void MKeyboardHost::doBackspace()
{
    // note: backspace shouldn't start accurate mode
    if (preedit.length() > 0) {
        if (!backSpaceTimer.isActive()) {
            setPreedit(preedit.left(preedit.length() - 1));
            updatePreeditStyle();
        } else {
            resetInternalState();
            inputContextConnection()->sendCommitString("");
        }
    } else {
        const KeyEvent event("\b", QEvent::KeyPress, Qt::Key_Backspace,
                             KeyEvent::NotSpecial,
                             vkbWidget->shiftStatus() != ModifierClearState
                             ? Qt::ShiftModifier : Qt::NoModifier);
        inputContextConnection()->sendKeyEvent(event.toQKeyEvent());
    }
    // Backspace toggles shift off if it's on (not locked)
    // except if autoCaps is on and cursor is at 0 position.
    if (vkbWidget->shiftStatus() == ModifierLatchedState
        && (!autoCapsEnabled || cursorPos != 0)) {
        vkbWidget->setShiftState(ModifierClearState);
    }
}

void MKeyboardHost::autoBackspace()
{
    backSpaceTimer.start(BackspaceRepeatInterval); // Must restart before doBackspace
    doBackspace();
}

void MKeyboardHost::handleKeyPress(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Shift) {
        if (shiftHeldDown) {
            return; //ignore duplicated event
        }

        if (activeState == MInputMethod::OnScreen && enableMultiTouch) {
            shiftHeldDown = true;
        }
    }

    bool signalOnly = true;
    if (((inputMethodMode == M::InputMethodModeDirect)
         && (event.specialKey() == KeyEvent::NotSpecial))
        || (event.qtKey() == Qt::Key_plusminus)) { // plusminus key makes an exception

        signalOnly = false;

    } else if (event.qtKey() == Qt::Key_Backspace) {
        backSpaceTimer.start(AutoBackspaceDelay);
    } else if (event.qtKey() == Qt::Key_Space
               && correctionEnabled
               && correctionCandidateWidget->isVisible()
               && correctionCandidateWidget->candidateMode() == MImCorrectionCandidateWidget::PopupMode) {
        spaceTimer.start(LongTapspaceDelay);
    }

    inputContextConnection()->sendKeyEvent(event.toQKeyEvent(), signalOnly);
}

void MKeyboardHost::handleKeyRelease(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Shift) {
        if (!shiftHeldDown) {
            return; //ignore duplicated event
        }

        if (activeState == MInputMethod::OnScreen && enableMultiTouch) {
            shiftHeldDown = false;
        }
    }

    bool signalOnly = true;
    if (((inputMethodMode == M::InputMethodModeDirect)
         && (event.specialKey() == KeyEvent::NotSpecial))
        || (event.qtKey() == Qt::Key_plusminus)) { // plusminus key makes an exception

        signalOnly = false;

    } else if ((event.qtKey() == Qt::Key_Backspace) && backSpaceTimer.isActive()) {
        backSpaceTimer.stop();
        if (correctionCandidateWidget->isVisible()) {
            correctionCandidateWidget->hideWidget();
        } else {
            doBackspace();
        }
    } else if (event.qtKey() == Qt::Key_Space && correctionEnabled) {
        spaceTimer.stop();
    }

    inputContextConnection()->sendKeyEvent(event.toQKeyEvent(), signalOnly);
}

void MKeyboardHost::updateReactionMaps()
{
    if (rotationInProgress) {
        return;
    }

    // Start by making everything transparent
    clearReactionMaps(MReactionMap::Transparent);

    QList<QGraphicsView *> views = MPlainWindow::instance()->scene()->views();
    foreach(QGraphicsView *view, views) {
        MReactionMap *reactionMap = MReactionMap::instance(view);

        if (!reactionMap) {
            continue;
        }

        // Candidates widget
        if (correctionCandidateWidget && correctionCandidateWidget->isVisible()) {
            correctionCandidateWidget->paintReactionMap(reactionMap, view);

            // Correction candidate widget always occupies whole screen.
            continue;
        }

        // Paint either symview or vkb widget reactive areas.
        if (symbolView && symbolView->isFullyVisible()) {
            symbolView->paintReactionMap(reactionMap, view);
        } else if (vkbWidget && vkbWidget->isFullyVisible()) {
            vkbWidget->paintReactionMap(reactionMap, view);
        }

        // Toolbar
        if (imToolbar && imToolbar->isVisible()) {
            imToolbar->paintReactionMap(reactionMap, view);
        }
    }
}

void MKeyboardHost::clearReactionMaps(const QString &clearValue)
{
    if (!MPlainWindow::instance()->scene()) {
        return;
    }

    foreach (QGraphicsView *view, MPlainWindow::instance()->scene()->views()) {
        MReactionMap *reactionMap = MReactionMap::instance(view);
        if (reactionMap) {
            reactionMap->setDrawingValue(clearValue, clearValue);
            reactionMap->setTransform(QTransform()); // Identity
            reactionMap->fillRectangle(0, 0, reactionMap->width(), reactionMap->height());
        }
    }
}

void MKeyboardHost::handleKeyClick(const KeyEvent &event)
{
    // Don't need send key events for Direct input mode here.
    // already send in handleKeyPress and handleKeyRelease.
    if (activeState == MInputMethod::Hardware && inputMethodMode != M::InputMethodModeDirect) {
        // In hardware keyboard mode symbol view is just another source for
        // events that will be handled by duihardwarekeyboard.  The native
        // modifiers may not be correct (depending on the current hwkbd modifier
        // state) but that doesn't matter.
        processKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(),
                        event.text(), false, 1, 0, 0);
        processKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(),
                        event.text(), false, 1, 0, 0);
    } else if ((inputMethodMode != M::InputMethodModeDirect)) {
        handleTextInputKeyClick(event);
    }

    handleGeneralKeyClick(event);

    lastClickEvent = event;
}

void MKeyboardHost::handleGeneralKeyClick(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Shift) {
        switch (vkbWidget->shiftStatus()) {
        case ModifierLatchedState:
            // If current ShiftOn state is due to autocaps, go back to ShiftOff.
            // Otherwise, lock it.
            if (autoCapsTriggered) {
                vkbWidget->setShiftState(ModifierClearState);
            } else {
                vkbWidget->setShiftState(ModifierLockedState);
            }
            break;
        case ModifierClearState:
            vkbWidget->setShiftState(ModifierLatchedState);
            break;
        case ModifierLockedState:
            vkbWidget->setShiftState(ModifierClearState);
            break;
        }
        autoCapsTriggered = false;
    } else if (vkbWidget->shiftStatus() == ModifierLatchedState
               && (event.qtKey() != Qt::Key_Backspace)
               && (event.specialKey() != KeyEvent::Sym)
               && (event.specialKey() != KeyEvent::Switch)
               && (event.specialKey() != KeyEvent::LayoutMenu)
               && (!shiftHeldDown || autoCapsTriggered)) {
        // Any key except shift toggles shift off if it's on (not locked).
        // Exceptions are:
        // - backspace, toggles shift off is handled in doBackspace()
        // - sym, pressing sym key keeps current shift state
        // - switch, pressing switch key keeps current shift state
        // - menu, pressing menu key keeps current shift state
        // - shift, when held down don't bring level down, except with autocaps!
        //   note: For this we cannot use event.modifiers().testFlag(Qt::ShiftModifier)
        //         because it does not differentiate between latched+char and held down + char.
        vkbWidget->setShiftState(ModifierClearState);
    }

    if (event.specialKey() == KeyEvent::LayoutMenu) {
        showLayoutMenu();
    } else if (event.specialKey() == KeyEvent::Sym) {
        handleSymbolKeyClick();
    } else if (event.specialKey() == KeyEvent::Switch) {
        if (symbolView->isActive()) {
            symbolView->switchToNextPage();
        }
    }
}

void MKeyboardHost::handleTextInputKeyClick(const KeyEvent &event)
{
    // Discard KeyPress & Drop type of events.
    if (event.type() != QEvent::KeyRelease

        // Discard also special keys except cycleset which has text
        // that can be sent to input context.
        || (!(event.specialKey() == KeyEvent::NotSpecial
              || event.specialKey() == KeyEvent::CycleSet))

        // Finally, discard Qt backspace and plusminus key, which are handled in
        // handleKeyPress/Release.
        || (event.qtKey() == Qt::Key_Backspace)
        || (event.qtKey() == Qt::Key_plusminus)
        || (event.qtKey() == Qt::Key_Shift)) {

        return;
    }

    if (cycleKeyHandler->handleTextInputKeyClick(event)) {
        return;
    }
    QString text(event.text());

    if (text.isEmpty()) {
        return;
    }

    if (!correctionEnabled) {
        if (preedit.length() > 0) {
            // we just entered accurate mode. send the previous preedit stuff.
            inputContextConnection()->sendCommitString(preedit);
            if (engineReady)
                imCorrectionEngine->clearEngineBuffer();
            preedit.clear();
        }

        inputContextConnection()->sendCommitString(text);

    } else if ((event.qtKey() == Qt::Key_Space) || (event.qtKey() == Qt::Key_Return) || (event.qtKey() == Qt::Key_Tab)) {
        // commit suggestion if correction candidate widget is visible and with popupMode
        // or ignore it if correction widget is visible and with suggestionlist mode
        // otherwise commit preedit
        if (event.qtKey() == Qt::Key_Space
            && correctionCandidateWidget->isVisible()) {
            if (correctionCandidateWidget->candidateMode() == MImCorrectionCandidateWidget::PopupMode) {
                inputContextConnection()->sendCommitString(correctionCandidateWidget->suggestion());
            } else {
                return;
            }
        } else {
            inputContextConnection()->sendCommitString(preedit);
        }

        if (lastClickEvent.specialKey() != KeyEvent::CycleSet) {
            correctionCandidateWidget->setPreeditString("");
            candidates.clear();
            correctionCandidateWidget->setCandidates(candidates);
            correctionCandidateWidget->hideWidget();
            //save context.
            imCorrectionEngine->setContext(preedit);
        }
        // send trailing space
        inputContextConnection()->sendCommitString(text);

        imCorrectionEngine->clearEngineBuffer();
        preedit.clear();
    } else {
        // common case: just append stuff to current preedit
        preedit += text;

        candidates.clear();
        qDebug() << "event touch point:" << event.touchPoint();
        imCorrectionEngine->tapKeyboard(event.touchPoint(), vkbWidget->shiftStatus() != ModifierClearState);
        candidates = imCorrectionEngine->candidates();
        M::DictionaryType sourceDictionaryType = imCorrectionEngine->candidateSource(0);

        correctionCandidateWidget->setPreeditString(preedit);
        correctionCandidateWidget->setCandidates(candidates);

        updatePreeditStyle();

        // if has candidates and the preedit is not a valid dictionary word,
        // then show word tracker.
        if (candidates.count() > 1
            && sourceDictionaryType == M::DictionaryTypeInvalid) {
            bool success = false;
            const QRect rect = inputContextConnection()->preeditRectangle(success);
            QRect localRect;
            if (success && !rect.isNull() && rotateRect(rect, localRect)) {
                correctionCandidateWidget->setPosition(localRect);
                correctionCandidateWidget->showWidget(MImCorrectionCandidateWidget::PopupMode);
            }
        } else {
            correctionCandidateWidget->hideWidget();
        }
    }
}

void MKeyboardHost::initializeInputEngine()
{
    // init correction engine
    // set language according current displayed language in virtual keyboard
    QString language = vkbWidget->layoutLanguage();

    if (language.isEmpty())
        language = DefaultInputLanguage;

    qDebug() << __PRETTY_FUNCTION__ << "- used language:" << language;

    if (engineReady) {
        // TODO: maybe we should check return values here and in case of failure
        // be always in accurate mode, for example
        imCorrectionEngine->setLanguage(language, M::LanguagePriorityPrimary);
        imCorrectionEngine->loadKeyboardLayout(vkbWidget->mainLayoutKeys());
        synchronizeCorrectionSetting();
        imCorrectionEngine->disablePrediction();
        imCorrectionEngine->enableCompletion();
        imCorrectionEngine->setMaximumCandidates(MaximumErrorCorrectionCandidate);
        imCorrectionEngine->setExactWordPositionInList(M::ExactInListFirst);
    }
}

void MKeyboardHost::synchronizeCorrectionSetting()
{
    bool correction = inputMethodCorrectionSettings->value(DefaultInputMethodCorrectionSettingOption).toBool();

    if (!correction) {
        imCorrectionEngine->disableCorrection();
    } else {
        imCorrectionEngine->enableCorrection();
    }

    updateCorrectionState();
}


void MKeyboardHost::updateCorrectionState()
{
    if (activeState == MInputMethod::Hardware) {
        inputContextConnection()->setGlobalCorrectionEnabled(false);
        correctionEnabled = false;
    } else {
        if (!engineReady) {
            inputContextConnection()->setGlobalCorrectionEnabled(false);
            correctionEnabled = false;
            return;
        }
        bool val = false;
        bool enabled = inputContextConnection()->correctionEnabled(val);
        if (val)
            correctionEnabled = enabled && imCorrectionEngine->correctionEnabled();
        else
            correctionEnabled = imCorrectionEngine->correctionEnabled();

        // info context the global correction option
        // TODO: should not put setGlobalCorrectionEnabled here, it will send correction setting
        // whenever focus changes. But have to at this moment, because im-uiserver start before
        // application, and there is no focus widget, no activateContext, calling
        // setGlobalCorrectionEnabled() at that time, can not record the setting.
        // Only after the application is running, this setGlobalCorrectionEnabled() can take effect
        inputContextConnection()->setGlobalCorrectionEnabled(imCorrectionEngine->correctionEnabled());
    }
}


void MKeyboardHost::userHide()
{
    vkbWidget->hideKeyboard();
    inputContextConnection()->notifyImInitiatedHiding();
}


void MKeyboardHost::sendCopyPaste(CopyPasteState action)
{
    switch (action) {
    case InputMethodCopy:
        inputContextConnection()->copy();
        break;
    case InputMethodPaste:
        inputContextConnection()->paste();
        break;
    default:
        qDebug() << __PRETTY_FUNCTION__ << "invalid action" << action;
        break;
    }
}

void MKeyboardHost::showLayoutMenu()
{
    emit settingsRequested();
}

void MKeyboardHost::setRegionInfo(RegionList &regionStore,
                                  const QRegion &region,
                                  const QPointer<QObject> &widget)
{
    bool found = false;

    for (RegionList::iterator iterator = regionStore.begin();
         iterator != regionStore.end();
         ++iterator) {
        if (iterator->first == widget) {
            iterator->second = region;
            found = true;
            break;
        }
    }

    if (!found) {
        regionStore.append(ObjectRegionPair(widget, region));
    }
}

QRegion MKeyboardHost::combineRegion()
{
    return combineRegionImpl(widgetRegions, true);
}

QRegion MKeyboardHost::combineInputMethodArea()
{
    return combineRegionImpl(inputMethodAreaWidgetRegions, false);
}

QRegion MKeyboardHost::combineRegionImpl(const RegionList &regionStore,
                                         bool includeExtraInteractiveAreas)
{
    QRegion combinedRegion;

    foreach (const ObjectRegionPair &pair, regionStore) {
        if (pair.first) {
            combinedRegion |= pair.second;
        }
    }

    if (sharedHandleArea) {
        //add region occupied by sharedHandleArea
        combinedRegion = sharedHandleArea->addRegion(combinedRegion, includeExtraInteractiveAreas);
    }

    return combinedRegion;
}

void MKeyboardHost::handleRegionUpdate(const QRegion &region)
{
    QPointer<QObject> pointer(QObject::sender());

    setRegionInfo(widgetRegions, region, pointer);
    handleRegionUpdate();
}

void MKeyboardHost::handleRegionUpdate()
{
    emit regionUpdated(combineRegion());
    updateReactionMaps();
}

void MKeyboardHost::handleInputMethodAreaUpdate(const QRegion &region)
{
    QPointer<QObject> pointer(QObject::sender());

    setRegionInfo(inputMethodAreaWidgetRegions, region, pointer);
    handleInputMethodAreaUpdate();
}

void MKeyboardHost::handleInputMethodAreaUpdate()
{
    emit inputMethodAreaUpdated(combineInputMethodArea());
}

void MKeyboardHost::sendKeyEvent(const QKeyEvent &key)
{
    inputContextConnection()->sendKeyEvent(key);
}

void MKeyboardHost::sendString(const QString &text)
{
    inputContextConnection()->sendCommitString(text);
}

void MKeyboardHost::setToolbar(QSharedPointer<const MToolbarData> toolbar)
{
    if (toolbar && toolbar->isVisible()) {
        imToolbar->showToolbarWidget(toolbar);
    } else {
        imToolbar->hideToolbarWidget();
    }
}

void MKeyboardHost::processKeyEvent(QEvent::Type keyType, Qt::Key keyCode,
                                    Qt::KeyboardModifiers modifiers, const QString &text,
                                    bool autoRepeat, int count, quint32 nativeScanCode,
                                    quint32 nativeModifiers)
{
    if ((activeState != MInputMethod::Hardware) ||
        !hardwareKeyboard->filterKeyEvent(keyType, keyCode, modifiers, text,
                                          autoRepeat, count, nativeScanCode,
                                          nativeModifiers)) {
        inputContextConnection()->sendKeyEvent(QKeyEvent(keyType, keyCode, modifiers, text,
                                                         autoRepeat, count));
    }
}

void MKeyboardHost::clientChanged()
{
    hardwareKeyboard->clientChanged();
    resetInternalState();
    hide(); // could do some quick hide also
}

void MKeyboardHost::switchContext(MInputMethod::SwitchDirection direction, bool enableAnimation)
{
    if (activeState == MInputMethod::OnScreen) {
        vkbWidget->switchLayout(direction, enableAnimation);
    }
}

void MKeyboardHost::setState(const QSet<MInputMethod::HandlerState> &state)
{
    if (state.isEmpty()) {
        return;
    }

    const MInputMethod::HandlerState actualState = *state.begin();
    if (activeState == actualState)
        return;

    if ((activeState == MInputMethod::OnScreen) && (preedit.length() > 0)) {
        inputContextConnection()->sendCommitString(preedit);
    }

    // Resets before changing the activeState to make sure clear.
    resetInternalState();
    activeState = actualState;

    // Keeps separate states for symbol view in OnScreen state and Hardware state
    if (activeState == MInputMethod::OnScreen) {
        hideLockOnInfoBanner();
        sendInputModeIndicator(MInputMethodBase::NoIndicator);
        disconnect(hardwareKeyboard, SIGNAL(deadKeyStateChanged(const QChar &)),
                   this, SLOT(handleHwKeyboardStateChanged()));
        disconnect(hardwareKeyboard, SIGNAL(modifiersStateChanged()),
                   this, SLOT(handleHwKeyboardStateChanged()));
        disconnect(hardwareKeyboard, SIGNAL(scriptChanged()),
                   this, SLOT(handleHwKeyboardStateChanged()));
        if (haveFocus) {
            hardwareKeyboard->disable();
        }
    } else {
        currentIndicatorDeadKey = false;
        connect(hardwareKeyboard, SIGNAL(deadKeyStateChanged(const QChar &)),
                this, SLOT(handleHwKeyboardStateChanged()));
        connect(hardwareKeyboard, SIGNAL(modifiersStateChanged()),
                this, SLOT(handleHwKeyboardStateChanged()));
        connect(hardwareKeyboard, SIGNAL(scriptChanged()),
                this, SLOT(handleHwKeyboardStateChanged()));
        if (haveFocus) {
            hardwareKeyboard->enable();
        }
    }

    symbolView->setKeyboardState(actualState);
    vkbWidget->setKeyboardState(actualState);
    updateCorrectionState();
    updateAutoCapitalization();
}

void MKeyboardHost::handleSymbolKeyClick()
{
    if (((activeState == MInputMethod::Hardware) && !hardwareKeyboard->symViewAvailable())
        || !vkbWidget->symViewAvailable()) {
        return;
    }

    // Toggle SymbolView.
    if (!symbolView->isActive()) {
        symbolView->showSymbolView();
        //give the symbolview right shift level(for hardware state)
        updateSymbolViewLevel();
    } else {
        symbolView->hideSymbolView();
    }
}

void MKeyboardHost::updateSymbolViewLevel()
{
    if (!symbolView->isActive())
        return;

    ModifierState shiftState = ModifierClearState;
    if (activeState == MInputMethod::OnScreen) {
        shiftState = vkbWidget->shiftStatus();
    } else {
        shiftState = hardwareKeyboard->modifierState(Qt::ShiftModifier);
    }
    symbolView->setShiftState(shiftState);
}

void MKeyboardHost::showSymbolView()
{
    symbolView->showSymbolView(SymbolView::FollowMouseShowMode);
    //give the symbolview right shift level(for hardware state)
    updateSymbolViewLevel();
}

MInputMethodBase::InputModeIndicator MKeyboardHost::deadKeyToIndicator(const QChar &key)
{
    switch (key.unicode()) {
    case 0x00b4:
        return MInputMethodBase::DeadKeyAcute;
    case 0x02c7:
        return MInputMethodBase::DeadKeyCaron;
    case 0x005e:
        return MInputMethodBase::DeadKeyCircumflex;
    case 0x00a8:
        return MInputMethodBase::DeadKeyDiaeresis;
    case 0x0060:
        return MInputMethodBase::DeadKeyGrave;
    case 0x007e:
        return MInputMethodBase::DeadKeyTilde;
    default:
        return MInputMethodBase::NoIndicator;
    }
}

void MKeyboardHost::handleHwKeyboardStateChanged()
{
    // only change indicator state when there is focus is in a widget and state is Hardware.
    if (!haveFocus || activeState != MInputMethod::Hardware)
        return;

    const ModifierState shiftState = hardwareKeyboard->modifierState(Qt::ShiftModifier);
    const ModifierState fnState = hardwareKeyboard->modifierState(FnLevelModifier);
    const QString xkbLayout = LayoutsManager::instance().xkbLayout();
    const QString xkbVariant = LayoutsManager::instance().xkbVariant();

    const bool previousIndicatorDeadKey(currentIndicatorDeadKey);

    MInputMethodBase::InputModeIndicator indicatorState
        = deadKeyToIndicator(hardwareKeyboard->deadKeyState());

    currentIndicatorDeadKey = false;

    if (indicatorState != MInputMethodBase::NoIndicator) {
        currentIndicatorDeadKey = true;
    } else if (fnState == ModifierLockedState) {
        indicatorState = MInputMethodBase::NumAndSymLocked;

    } else if (fnState == ModifierLatchedState) {
        indicatorState = MInputMethodBase::NumAndSymLatched;

    } else if (xkbLayout == "ara"
        && xkbVariant.isEmpty()) {
        indicatorState = MInputMethodBase::Arabic;

    } else if (xkbVariant.isEmpty() || xkbVariant == "latin") {
        indicatorState = MInputMethodBase::LatinLower;
        if (shiftState == ModifierLockedState) {
            indicatorState = MInputMethodBase::LatinLocked;
        } else if (shiftState == ModifierLatchedState) {
            indicatorState = MInputMethodBase::LatinUpper;
        }

    } else if (xkbVariant == "cyrillic") {
        indicatorState = MInputMethodBase::CyrillicLower;
        if (shiftState == ModifierLockedState) {
            indicatorState = MInputMethodBase::CyrillicLocked;
        } else if (shiftState == ModifierLatchedState) {
            indicatorState = MInputMethodBase::CyrillicUpper;
        }
    }

    sendInputModeIndicator(indicatorState);

    QString lockOnNotificationLabel;

    if (indicatorState == MInputMethodBase::LatinLocked
        || indicatorState == MInputMethodBase::CyrillicLocked) {
        //% "Caps lock on"
        lockOnNotificationLabel = qtTrId("qtn_hwkb_caps_lock");
    } else if (indicatorState == MInputMethodBase::NumAndSymLocked) {
        //% "Symbol lock on"
        lockOnNotificationLabel = qtTrId("qtn_hwkb_fn_lock");
    }
    if (!lockOnNotificationLabel.isEmpty()
        && !previousIndicatorDeadKey
        && hardwareKeyboard->keyboardType() != M::NumberContentType
        && hardwareKeyboard->keyboardType() != M::PhoneNumberContentType) {
        // notify the modifier is changed to locked state
        // number and phone number content type always force FN key to be locked,
        // don't need indicator lock notification.
        showLockOnInfoBanner(lockOnNotificationLabel);
    } else if (modifierLockOnBanner) {
        hideLockOnInfoBanner();
    }
}

void MKeyboardHost::showLockOnInfoBanner(const QString &notification)
{
    if (modifierLockOnBanner) {
        modifierLockOnBanner->setTitle(notification);
    } else {
        //TODO: discuss with UI designer whether we need to specify
        // the disappear time out.
        modifierLockOnBanner = new MBanner;
        modifierLockOnBanner->setTitle(notification);
        modifierLockOnBanner->appear(MSceneWindow::DestroyWhenDone);
    }
}

void MKeyboardHost::hideLockOnInfoBanner()
{
    if (modifierLockOnBanner) {
        modifierLockOnBanner->disappear();
    }
    modifierLockOnBanner = 0;
}

QList<MInputMethodBase::MInputMethodSubView>
MKeyboardHost::subViews(MInputMethod::HandlerState state) const
{
    QList<MInputMethodBase::MInputMethodSubView> sViews;
    if (state == MInputMethod::OnScreen) {
        QMap<QString, QString> selectedLayouts = LayoutsManager::instance().selectedLayouts();
        QMap<QString, QString>::const_iterator i = selectedLayouts.constBegin();
        while (i != selectedLayouts.constEnd()) {
            MInputMethodBase::MInputMethodSubView subView;
            subView.subViewId = i.key();
            subView.subViewTitle = i.value();
            sViews.append(subView);
            ++i;
        }
    }
    return sViews;
}

void MKeyboardHost::setActiveSubView(const QString &subViewId, MInputMethod::HandlerState state)
{
    if (state == MInputMethod::OnScreen) {
        const QStringList layoutFileList = LayoutsManager::instance().layoutFileList();
        int index = layoutFileList.indexOf(subViewId);
        vkbWidget->setLayout(index);
    }
}

QString MKeyboardHost::activeSubView(MInputMethod::HandlerState state) const
{
    if (state == MInputMethod::OnScreen) {
        // return the active vkb layout
        return vkbWidget->selectedLayout();
    } else {
        return QString();
    }
}

void MKeyboardHost::handleVirtualKeyboardLayoutChanged(const QString &layout)
{
    // reset the temporary shift state when layout is changed
    resetVirtualKeyboardShiftState();
    if (symbolView) {
        symbolView->setLayout(layout);
    }

    initializeInputEngine();
    updateAutoCapitalization();
    emit activeSubViewChanged(layout);
}

void MKeyboardHost::updatePreeditStyle()
{
    // preedit style type depends on candidates.count().
    // candidates.count   styleType
    //  0 or 1            PreeditNoCandidates
    //  1 or >1           PreeditDefault
    MInputMethod::PreeditFace face = MInputMethod::PreeditNoCandidates;
    if (candidates.count() > 1 ) {
        face = MInputMethod::PreeditDefault;
    }

    inputContextConnection()->sendPreeditString(preedit, face);
}

void MKeyboardHost::longTapSpace()
{
    if (!correctionCandidateWidget->isVisible()
        || correctionCandidateWidget->candidateMode() != MImCorrectionCandidateWidget::PopupMode
        || candidates.size() <= 0)
        return;

    correctionCandidateWidget->setPreeditString(preedit);
    correctionCandidateWidget->setCandidates(candidates);
    correctionCandidateWidget->showWidget(MImCorrectionCandidateWidget::SuggestionListMode);
}
