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

#include "mkeyboardhost.h"
#include "mkeyboardhost_p.h"

#include "flickgesturerecognizer.h"
#include "mvirtualkeyboardstyle.h"
#include "mvirtualkeyboard.h"
#include "mhardwarekeyboard.h"
#include "keyboarddata.h"
#include "layoutsmanager.h"
#include "mimtoolbar.h"
#include "sharedhandlearea.h"
#include "reactionmappainter.h"
#include "regiontracker.h"
#include "simplefilelog.h"
#include "reactionmapwrapper.h"
#include "enginemanager.h"
#include "abstractenginewidgethost.h"
#include "mimrootwidget.h"

#include <mimenginefactory.h>
#include <mabstractinputmethodhost.h>
#include <mplainwindow.h>
#include <mtoolbardata.h>
#include <mkeyoverride.h>
#include <mgconfitem.h>
#include <mimplugindescription.h>

#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QKeyEvent>
#include <QRegExp>
#include <QEasingCurve>
#include <QTextBoundaryFinder>

#include <MCancelEvent>
#include <MComponentData>
#include <MDeviceProfile>
#include <MScene>
#include <MSceneManager>
#include <MSceneWindow>
#include <MBanner>
#include <MLibrary>

M_LIBRARY

namespace
{
    const QString InputMethodList("MInputMethodList");
    // TODO: check that these paths still hold
    const QRegExp AutoCapsTrigger("[.?!¡¿] +$");
    const QString AutoPunctuationTriggers(".,?!");
    const int AutoBackspaceDelay = 500;      // in ms
    const int LongPressTime = 600;           // in ms
    const int BackspaceRepeatInterval = 100; // in ms
    const int MultitapTime = 1500;           // in ms
    const Qt::KeyboardModifier FnLevelModifier = Qt::GroupSwitchModifier;
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
    const char * const NotificationObjectName = "ModifierLockNotification";
    MKeyboardHost *currentInstance = 0;
    const char *const MImTouchPointsLogfile = "touchpoints.csv";
    const char PlusSign('+');
    const char MinusSign('-');
    bool gOwnsComponentData = false;

    QSize defaultScreenSize(QWidget *w)
    {
        if (not QApplication::desktop() || not w) {
            return QSize();
        }

        return QApplication::desktop()->screenGeometry(w).size();
    }
}

MKeyboardHost::SlideUpAnimation::SlideUpAnimation(QObject *parent)
    : QPropertyAnimation(parent)
{
    setPropertyName("pos");
    setEndValue(QPointF(0, 0));
}

void MKeyboardHost::SlideUpAnimation::updateCurrentTime(int currentTime)
{
    QGraphicsWidget &widget(*dynamic_cast<QGraphicsWidget *>(targetObject()));
    const qreal wantedEndY(MPlainWindow::instance()->visibleSceneSize().height()
                           - widget.size().height());

    if (endValue().toPointF().y() != wantedEndY) {
        setEndValue(QPointF(0, wantedEndY));
    }

    QPropertyAnimation::updateCurrentTime(currentTime);
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
            host.preedit = host.preedit.left(host.preedit.length() - 1);
        }
        cycleIndex = (cycleIndex + 1) % cycleText.size();
    }
    else {
        // Different key than last time, reset index and cycle text
        prevEvent = event;
        cycleIndex = 0;
        cycleText = event.text();
    }

    // Note: If the cycle key is used together with error correction preedit,
    // to insert cycle character in the middle of preedit, then we can not
    // simply set preeditCursorPos to -1.
    host.preedit += cycleText[cycleIndex];
    host.preeditCursorPos = -1;
    QList<MInputMethod::PreeditTextFormat> preeditFormats;
    MInputMethod::PreeditTextFormat preeditFormat(0, host.preedit.length(),
                                                  MInputMethod::PreeditNoCandidates);
    preeditFormats << preeditFormat;
    host.inputMethodHost()->sendPreeditString(host.preedit,
                                              preeditFormats);

    timer.start();

    return true;
}

void MKeyboardHost::CycleKeyHandler::reset()
{
    timer.stop();
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

MKeyboardHost::MKeyboardHost(MAbstractInputMethodHost *host,
                             QWidget *mainWindow)
    : MAbstractInputMethod(host, mainWindow),
      vkbStyleContainer(0),
      vkbWidget(0),
      symbolView(0),
      correctionEnabled(false),
      autoCapsEnabled(true),
      autoCapsTriggered(false),
      cursorPos(-1),
      preeditCursorPos(-1),
      hasSelection(false),
      preeditHasBeenEdited(false),
      inputMethodMode(M::InputMethodModeNormal),
      backspaceTimer(),
      shiftHeldDown(false),
      activeState(MInputMethod::OnScreen),
      modifierLockOnBanner(0),
      haveFocus(false),
      sipRequested(false),
      visualizationPriority(false),
      enableMultiTouch(MGConfItem(MultitouchSettings).value().toBool()),
      cycleKeyHandler(new CycleKeyHandler(*this)),
      currentIndicatorDeadKey(false),
      engineLayoutDirty(false),
      backspaceMode(NormalBackspaceMode),
      spaceInsertedAfterCommitString(false),
      touchPointLogHandle(0),
      view(0),
      mRootWidget(new MImRootWidget),
      toolbarHidePending(false),
      keyOverrideClearPending(false),
      regionUpdatesEnabledBeforeOrientationChange(true),
      appOrientationAngle(M::Angle90) // shouldn't matter, see handleAppOrientationChanged comment
{
    Q_ASSERT(host != 0);
    Q_ASSERT(mainWindow != 0);

    if (!MComponentData::instance()) {
        static int argc = qApp->argc();
        static char **argv = qApp->argv();
        MComponentData::createInstance(argc, argv, qApp->applicationName());
        gOwnsComponentData = true;
    }

    qRegisterMetaType<KeyContext>("KeyContext");

    connect(host, SIGNAL(pluginsChanged()),
            this, SLOT(onPluginsChange()));

    view = new MPlainWindow(host, mainWindow);
    // MSceneManager's of MWindow's are lazy-initialized. However, their
    //implict creation does resize the scene rect of our view, so we trigger
    // the lazy-initialization right here, to stay in control of things:
    (void *) view->sceneManager();

    // Once MComponentData is alive, it keeps resizing our QWidgets to wrong
    // dimensions, so we fix it right back here:
    const QSize screenSize(defaultScreenSize(view));
    view->resize(screenSize);
    view->setMinimumSize(1, 1);
    view->setMaximumSize(screenSize);
    view->setSceneRect(QRect(QPoint(), screenSize));

#ifdef HAVE_REACTIONMAP
    MReactionMap::createInstance(*mainWindow, qAppName(), this);
#endif

    RegionTracker::createInstance();
    connect(&RegionTracker::instance(), SIGNAL(regionChanged(const QRegion &)),
            host, SLOT(setScreenRegion(const QRegion &)));
    connect(&RegionTracker::instance(), SIGNAL(inputMethodAreaChanged(const QRegion &)),
            host, SLOT(setInputMethodArea(const QRegion &)));

    // Create the reaction map painter
    ReactionMapPainter::createInstance();

    displayHeight = MPlainWindow::instance()->visibleSceneSize(M::Landscape).height();
    displayWidth  = MPlainWindow::instance()->visibleSceneSize(M::Landscape).width();

    sceneWindow = new MSceneWindow;
    sceneWindow->setManagedManually(true); // we want the scene window to remain in origin

    MPlainWindow::instance()->setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);

    // our paint methods are accurate enough, so we can disable painter saving
    MPlainWindow::instance()->setOptimizationFlags(QGraphicsView::DontSavePainterState);

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

    connect(vkbWidget, SIGNAL(geometryChanged()),
            this, SLOT(handleVirtualKeyboardGeometryChange()));

    connect(vkbWidget, SIGNAL(keyClicked(const KeyEvent &)),
            this, SLOT(handleKeyClick(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyPressed(const KeyEvent &)),
            this, SLOT(handleKeyPress(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyReleased(const KeyEvent &)),
            this, SLOT(handleKeyRelease(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(longKeyPressed(const KeyEvent &)),
            this, SLOT(handleLongKeyPress(const KeyEvent &)));
    connect(vkbWidget, SIGNAL(keyCancelled(const KeyEvent &)),
            this, SLOT(handleKeyCancel(const KeyEvent &)));


    connect(vkbWidget, SIGNAL(userInitiatedHide()),
            this, SLOT(userHide()));

    connect(vkbWidget, SIGNAL(pluginSwitchRequired(MInputMethod::SwitchDirection)),
            this, SLOT(switchPlugin(MInputMethod::SwitchDirection)));

    // construct hardware keyboard object
    hardwareKeyboard = new MHardwareKeyboard(*host, this);
    connect(hardwareKeyboard, SIGNAL(symbolKeyClicked()),
            this, SLOT(handleSymbolKeyClick()));
    // Trigger a reaction map when the hardware keyboard is opened
    connect(hardwareKeyboard, SIGNAL(enabled()), &ReactionMapPainter::instance(), SLOT(repaint()));

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
                 this, SLOT(sendStringFromToolbar(const QString &)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(copyPasteClicked(CopyPasteState)),
                 this, SLOT(sendCopyPaste(CopyPasteState)));
    Q_ASSERT(ok);
    ok = connect(imToolbar, SIGNAL(closeKeyboardRequest()),
                 this, SLOT(userHide()));
    Q_ASSERT(ok);

    sharedHandleArea = new SharedHandleArea(*imToolbar, sceneWindow);
    sharedHandleArea->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));

    // Set z value below default level (0.0) so popup will be on top of shared handle area.
    sharedHandleArea->setZValue(-1.0);
    sharedHandleArea->watchOnWidget(vkbWidget);

    // Don't listen to device orientation.  Applications can be in different orientation
    // than the device (especially plain qt apps). See NB#185013 - Locking VKB orientation.
    MPlainWindow::instance()->lockOrientationAngle();

    symbolView = new SymbolView(LayoutsManager::instance(), vkbStyleContainer,
                                vkbWidget->selectedLayout(), sceneWindow);
    connect(symbolView, SIGNAL(geometryChanged()),
            this, SLOT(handleSymbolViewGeometryChange()));
    connect(symbolView, SIGNAL(visibleChanged()), this, SLOT(handleSymbolViewVisibleChanged()));

    connect(symbolView, SIGNAL(keyClicked(const KeyEvent &)),
            this, SLOT(handleKeyClick(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyPressed(const KeyEvent &)),
            this, SLOT(handleKeyPress(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyReleased(const KeyEvent &)),
            this, SLOT(handleKeyRelease(const KeyEvent &)));
    connect(symbolView, SIGNAL(longKeyPressed(const KeyEvent &)),
            this, SLOT(handleLongKeyPress(const KeyEvent &)));
    connect(symbolView, SIGNAL(keyCancelled(const KeyEvent &)),
            this, SLOT(handleKeyCancel(const KeyEvent &)));

    connect(symbolView, SIGNAL(userInitiatedHide()),
            this, SLOT(userHide()));

    sharedHandleArea->watchOnWidget(symbolView);

    connect(MPlainWindow::instance()->sceneManager(), SIGNAL(orientationChangeFinished(M::Orientation)),
            this, SLOT(finalizeOrientationChange()));

    connect(vkbWidget, SIGNAL(layoutChanged(const QString &)),
            this, SLOT(handleVirtualKeyboardLayoutChanged(const QString &)));

    connect(vkbWidget, SIGNAL(shiftLevelChanged()),
            this, SLOT(updateSymbolViewLevel()));

    connect(hardwareKeyboard, SIGNAL(shiftStateChanged()),
            this, SLOT(updateSymbolViewLevel()));

    EngineManager::createInstance(*this);
    QString language = vkbWidget->layoutLanguage();
    EngineManager::instance().updateLanguage(language);
    connect(&EngineManager::instance(), SIGNAL(correctionSettingChanged()),
            this,                                   SLOT(updateCorrectionState()));

    engineLayoutDirty = true;
    if (vkbWidget->isVisible())
        updateEngineKeyboardLayout();

    backspaceTimer.setSingleShot(true);
    connect(&backspaceTimer, SIGNAL(timeout()), this, SLOT(autoBackspace()));

    // Set up animations
    slideUpAnimation.setTargetObject(vkbWidget);
    slideUpAnimation.setEasingCurve(QEasingCurve::InOutQuint);
    slideUpAnimation.setDuration(OnScreenAnimationTime);

    connect(&slideUpAnimation, SIGNAL(finished()), this, SLOT(handleAnimationFinished()));
    // Trigger a reaction map update
    connect(&slideUpAnimation, SIGNAL(finished()), &ReactionMapPainter::instance(), SLOT(repaint()));

    Q_ASSERT(currentInstance == 0); // Several instances of this class is invalid.
    currentInstance = this;
}

MKeyboardHost::~MKeyboardHost()
{
    slideUpAnimation.stop();
    EngineManager::destroyInstance();
    hideLockOnInfoBanner();
    delete hardwareKeyboard;
    hardwareKeyboard = 0;
    delete vkbWidget;
    vkbWidget = 0;
    delete symbolView;
    symbolView = 0;
    delete sceneWindow;
    sceneWindow = 0;
    delete vkbStyleContainer;
    vkbStyleContainer = 0;
    delete touchPointLogHandle;
    touchPointLogHandle = 0;
    backspaceMode = NormalBackspaceMode;
    backspaceTimer.stop();
    LayoutsManager::destroyInstance();
    ReactionMapPainter::destroyInstance();
    RegionTracker::destroyInstance();
    currentInstance = 0;
    // TODO: let scene handle deletion of this widget:
    delete mRootWidget;
    mRootWidget = 0;

    if (gOwnsComponentData) {
        delete MComponentData::instance();
        gOwnsComponentData = false;
    }
}

MKeyboardHost* MKeyboardHost::instance()
{
    return currentInstance;
}

QTextStream &MKeyboardHost::touchPointLog()
{
    if (!touchPointLogHandle) {
        touchPointLogHandle = new SimpleFileLog(MImTouchPointsLogfile);
    }
    return touchPointLogHandle->stream();
}

// TODO: it would seem that application focus state is passed to all plugins by
// MInputContextGlibDBusConnection::updateWidgetInformation, including nonactive ones.  If
// that really is the case, then what we're doing here is wrong.  Yet another reason to have
// plugin (de)activation methods?  Also see comments in MIMPluginManagerPrivate::replacePlugin.
void MKeyboardHost::handleFocusChange(bool focusIn)
{
    haveFocus = focusIn;
    if (activeState == MInputMethod::OnScreen) {
        if (focusIn) {
            symbolView->hideSymbolView();
            // reset latched shift state when focus is changed
            resetVirtualKeyboardLatchedShiftState();
        }
    } else {
        if (focusIn) {
            hardwareKeyboard->enable();
        } else {
            hardwareKeyboard->disable();
            inputMethodHost()->setInputModeIndicator(MInputMethod::NoIndicator);
        }
        hideLockOnInfoBanner();
    }
}

void MKeyboardHost::sendRegionEstimate()
{
    // We need to send one initial region update to have passthruwindow make the
    // window visible.  We also want the scene manager to move the underlying window
    // so that the focused widget will be visible after the show animation and we want
    // it to do that right away, so the region we send now is (usually) the final
    // region (i.e. what it will be after animation is finished).  Region will be sent
    // additionally after animation is finished just in case copy button appears or
    // something else changes during the animation.  But normally the region should be
    // the same as the one we send now.

    // Region is calculated from widget geometries so this method assumes relevant layouts
    // to have been activated beforehand. A QGraphicsItem::show() will do, for example.

    QRectF stackedRects(0.0f, MPlainWindow::instance()->visibleSceneSize().height(),
                        0.0f, 0.0f);

    // Add vkb rect if vkb is visible.
    if (vkbWidget->isVisible()) {
        QRectF vkbRect(vkbWidget->rect());
        vkbRect.moveBottom(stackedRects.top());
        stackedRects |= vkbRect;
    }

    // Add toolbar rect (toolbar is always visible, even when it is empty)
    QRectF toolbarRect(imToolbar->rect());
    toolbarRect.moveBottom(stackedRects.top());
    stackedRects |= toolbarRect;

    const QRegion region(sceneWindow->mapRectToScene(stackedRects).toRect());

    RegionTracker::instance().sendInputMethodAreaEstimate(region);
    RegionTracker::instance().sendRegionEstimate(region);
}

void MKeyboardHost::show()
{
    if (((activeState == MInputMethod::Hardware) && !hardwareKeyboard->symViewAvailable())
        || ((activeState == MInputMethod::OnScreen) && !vkbWidget->symViewAvailable())) {
        symbolView->hideSymbolView();
    }

    if (sipRequested && (slideUpAnimation.state() == QAbstractAnimation::Stopped)) {
        return;
    }
    sipRequested = true;
    if (visualizationPriority) {
        return;
    }
    RegionTracker::instance().enableSignals(false);

    // Enable the widgets back
    // Note: The widgets have been disabled when VKB was going to hide.
    MPlainWindow::instance()->setEnabled(true);

    handleAppOrientationChanged(appOrientationAngle);

    // This will add scene window as child of MSceneManager's root element
    // which is the QGraphicsItem that is rotated when orientation changes.
    // It uses animation to carry out the orientation change transform
    // (e.g. rotation and position animation). We do this because transform
    // happens in the scene, not in the view (MWindow) anymore.
    MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(sceneWindow);

    sharedHandleArea->show();

    prepareHideShowAnimation();
    if (activeState == MInputMethod::OnScreen) {
        vkbWidget->show();
    }

    // Ensure the vkb layout language is the same as actual engine language.
    // This is done because currently IM engine interface shares correction engine
    // instances, and some other input method plugin might change engine state.
    EngineManager::instance().ensureLanguageInUse(vkbWidget->layoutLanguage());

    // Update input engine keyboard layout.
    if (vkbWidget->isVisible())
        updateEngineKeyboardLayout();
    if (EngineManager::instance().handler()
        && EngineManager::instance().handler()->hasErrorCorrection())
        updateCorrectionState();

    // If view's scene rect is larger than scene window, and fully contains it,
    // then dock scene window (and therefore, VKB) to the bottom center of scene:
    if (not view->sceneRect().intersected(sceneWindow->mapRectToScene(sceneWindow->rect())).contains(view->rect())) {
        QSizeF sceneSize(view->sceneRect().size());
        if (view->orientation() == M::Portrait) {
            sceneSize = QSizeF(sceneSize.height(), sceneSize.width());
        }

        sceneWindow->setPos(sceneSize.width() * 0.5 - sceneWindow->size().width() * 0.5,
                            sceneSize.height() - (view->visibleSceneSize().height()));
    } else {
        sceneWindow->setPos(0, 0);
    }

    sendRegionEstimate();
    slideUpAnimation.setDirection(QAbstractAnimation::Forward);
    slideUpAnimation.start();

    if (EngineManager::instance().handler()) {
        AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler()->engineWidgetHost();
        if (engineWidgetHost && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::DockedMode) {
            engineWidgetHost->showEngineWidget(AbstractEngineWidgetHost::DockedMode);
        }
    }
}


void MKeyboardHost::prepareHideShowAnimation()
{
    if (activeState == MInputMethod::Hardware) {
        slideUpAnimation.setDuration(HardwareAnimationTime);
        slideUpAnimation.setTargetObject(sharedHandleArea);
        slideUpAnimation.setStartValue(QPointF(0, MPlainWindow::instance()->visibleSceneSize().height()));
    } else {
        slideUpAnimation.setDuration(OnScreenAnimationTime);

        if (symbolView->isActive()) {
            slideUpAnimation.setTargetObject(symbolView);
        } else {
            slideUpAnimation.setTargetObject(vkbWidget);
        }
        slideUpAnimation.setStartValue(QPointF(0, MPlainWindow::instance()->visibleSceneSize().height()
                                               + sharedHandleArea->size().height()));
    }
}


void MKeyboardHost::hide()
{
    RegionTracker::instance().enableSignals(false);
    RegionTracker::instance().sendInputMethodAreaEstimate(QRegion());

    if (EngineManager::instance().handler()) {
        AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler()->engineWidgetHost();
        if (engineWidgetHost && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::FloatingMode) {
            engineWidgetHost->hideEngineWidget();
        }
    }

    // Avoid receiving input events when sliding away
    MPlainWindow::instance()->setEnabled(false);

    prepareHideShowAnimation();
    slideUpAnimation.setDirection(QAbstractAnimation::Backward);
    slideUpAnimation.start();

    sipRequested = false;

    if (touchPointLogHandle) {
        touchPointLogHandle->flush();
    }
}


void MKeyboardHost::handleAnimationFinished()
{
    if (slideUpAnimation.direction() == QAbstractAnimation::Backward) {
        if (toolbarHidePending) {
            imToolbar->hideToolbarWidget();
            toolbarHidePending = false;
        }

        if (keyOverrideClearPending) {
            QMap<QString, QSharedPointer<MKeyOverride> > emptyOverrides;
            vkbWidget->setKeyOverrides(emptyOverrides);
            symbolView->setKeyOverrides(emptyOverrides);
            keyOverrideClearPending = false;
            overrides = emptyOverrides;
        }

        sharedHandleArea->hide();
        vkbWidget->hide();
        vkbWidget->resetState();
        symbolView->hideSymbolView();

        if (EngineManager::instance().handler()) {
            AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler()->engineWidgetHost();
            if (engineWidgetHost && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::DockedMode) {
                engineWidgetHost->hideEngineWidget();
            }
        }
        // TODO: the following line which was added to improve plugin switching (see the
        // commit comment) would cause animation not to be seen if it was in ::hide() but
        // just having it here without any two-phase show/hide protocol that considers
        // plugin switching might result to odd situations as well.
        MPlainWindow::instance()->sceneManager()->disappearSceneWindowNow(sceneWindow);
    }

    RegionTracker::instance().enableSignals(true);
}


void MKeyboardHost::handleVirtualKeyboardGeometryChange()
{
    if (slideUpAnimation.state() == QAbstractAnimation::Stopped
         && !vkbWidget->isPlayingAnimation()) {
        vkbWidget->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - vkbWidget->size().height());
    }
}


void MKeyboardHost::handleSymbolViewGeometryChange()
{
    if (symbolView->isVisible()) {
        symbolView->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - symbolView->size().height());
    }
}

void MKeyboardHost::handleSymbolViewVisibleChanged()
{
    if (symbolView->isVisible()) {
        vkbWidget->hide();
    } else if (!visualizationPriority && sipRequested) {
        if (activeState == MInputMethod::OnScreen) {
            vkbWidget->show();
        } else {
            // SharedHandleArea no longer tracks anything, reposition it to the bottom of the screen
            sharedHandleArea->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - sharedHandleArea->size().height());
        }
    }
}

void MKeyboardHost::setPreedit(const QString &preeditString, int cursor)
{
    if (EngineManager::instance().handler()
        && EngineManager::instance().handler()->acceptPreeditInjection()) {
        const int preeditLength = preeditString.length();
        // Application calls this method to tell the preedit and cursor information.
        // If the length of preedit is less or equal to 4, then ignore the input cursor,
        // put the cursor at the end of preedit. Otherwise using the input cursor for
        // preedit.
        preeditCursorPos = ((preeditLength > 4 && cursor <= preeditLength) || cursor < 0)
            ? cursor : preeditLength;

        preedit = preeditString;
        preeditHasBeenEdited = false;
        QStringList candidates;
        bool preeditInDict = false;
        if (EngineManager::instance().engine()) {
            EngineManager::instance().engine()->clearEngineBuffer();
            EngineManager::instance().engine()->reselectString(preeditString);
            candidates = EngineManager::instance().engine()->candidates();
            preeditInDict = (EngineManager::instance().engine()->candidateSource(0) != MImEngine::DictionaryTypeInvalid);
            if (EngineManager::instance().handler() && EngineManager::instance().handler()->engineWidgetHost())
                EngineManager::instance().handler()->engineWidgetHost()->setCandidates(candidates);
        }

        // updatePreedit() will send back the preedit with updated preedit style.
        // TODO: But this sending back preedit could cause problem. Because the
        // application is calling this setPreedit by sending an asynchronized
        // event. Maybe we need to find out the other way to update the preedit style.
        updatePreedit(preedit, candidates.count(), preeditInDict, 0, 0, preeditCursorPos);

        // If the above updatePreedit does not actually cause any change (typically format and
        // cursor position change) on the application side, there is no reason for update() to
        // be called and therefore shift state might stay latched unless we explicitly clear
        // the state here.
        if ((activeState == MInputMethod::OnScreen)
                && (vkbWidget->shiftStatus() != ModifierLockedState)) {
            vkbWidget->setShiftState(ModifierClearState);
        }
    }
}

void MKeyboardHost::localSetPreedit(const QString &preeditString, int replaceStart,
                                    int replaceLength, int cursor, bool preeditWordEdited)
{
    preedit = preeditString;
    preeditCursorPos = cursor;
    preeditHasBeenEdited = preeditWordEdited;
    QStringList candidates;
    bool preeditInDict = false;
    AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
        EngineManager::instance().handler()->engineWidgetHost() : 0;
    if (EngineManager::instance().engine()) {
        EngineManager::instance().engine()->clearEngineBuffer();
        EngineManager::instance().engine()->reselectString(preeditString);
        candidates = EngineManager::instance().engine()->candidates();
        preeditInDict = (EngineManager::instance().engine()->candidateSource(0) != MImEngine::DictionaryTypeInvalid);
        if (engineWidgetHost) {
            engineWidgetHost->setCandidates(candidates);
        }
    }
    updatePreedit(preedit, candidates.count(), preeditInDict, replaceStart, replaceLength, cursor);

    if (preedit.isEmpty() && engineWidgetHost && engineWidgetHost->isActive()
        && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::FloatingMode) {
        engineWidgetHost->hideEngineWidget();
    }
}

void MKeyboardHost::update()
{
    bool valid = false;

    hasSelection = inputMethodHost()->hasSelection(valid);
    if (valid) {
        imToolbar->setSelectionStatus(hasSelection);
    }

    const int type = inputMethodHost()->contentType(valid);
    if (valid) {
        hardwareKeyboard->setKeyboardType(static_cast<M::TextContentType>(type));
        vkbWidget->setKeyboardType(type);
        if (EngineManager::instance().handler()
            && EngineManager::instance().handler()->hasErrorCorrection()) {
            // update correction option when content type is changed.
            updateCorrectionState();
        }
    }

    if (EngineManager::instance().handler()) {
        if (EngineManager::instance().handler()->hasAutoCaps())
            updateAutoCapitalization();
        if (EngineManager::instance().handler()->hasContext())
            updateContext();
    }

    updateCorrectionWidgetPosition();

    const int inputMethodModeValue = inputMethodHost()->inputMethodMode(valid);
    if (valid) {
        inputMethodMode = inputMethodModeValue;
        hardwareKeyboard->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
        vkbWidget->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
        sharedHandleArea->setInputMethodMode(static_cast<M::InputMethodMode>(inputMethodMode));
    }
}


void MKeyboardHost::resetVirtualKeyboardShiftState()
{
    // reset shift state
    if (activeState == MInputMethod::OnScreen) {
        autoCapsTriggered = false;
        vkbWidget->setShiftState(ModifierClearState);
    }
}

void MKeyboardHost::resetVirtualKeyboardLatchedShiftState()
{
    // reset latched shift state (shift on state set by user or auto capitalization,
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
    const int type = inputMethodHost()->contentType(valid);
    autoCapsEnabled = (autoCapsEnabled
                       && valid
                       && (type != M::NumberContentType)
                       && (type != M::PhoneNumberContentType));
    autoCapsEnabled = (autoCapsEnabled
                       && inputMethodHost()->autoCapitalizationEnabled(valid)
                       && valid);
    autoCapsEnabled = (autoCapsEnabled
                       && inputMethodHost()->surroundingText(surroundingText, cursorPos));

    if (!autoCapsEnabled)
        return;

    const bool hasSelection = (inputMethodHost()->hasSelection(valid) && valid);
    int anchorPosition = inputMethodHost()->anchorPosition(valid);
    if (!valid) {
        anchorPosition = cursorPos;
    }

    const int inputPosition = hasSelection ? qMin(anchorPosition, cursorPos) : cursorPos;

    // Capitalization is determined by preedit and Auto Capitalization.
    // If there are some preedit, it should be lower case.
    // Otherwise Auto Capitalization will turn on shift when (text entry capitalization option is ON):
    //   1. at the beginning of one paragraph
    //   2. after a sentence delimiter and one or more spaces
    autoCapsTriggered = ((preedit.length() == 0)
                         && ((inputPosition == 0)
                             || ((inputPosition > 0)
                                 && (inputPosition <= surroundingText.length())
                                 && surroundingText.left(inputPosition).contains(AutoCapsTrigger))));

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

    bool valid = false;
    const int type = inputMethodHost()->contentType(valid);
    if (!valid || (type == M::NumberContentType)
        || (type == M::PhoneNumberContentType)) {
        return;
    }

    if (inputMethodHost()->surroundingText(surroundingText, cursorPos)
        && EngineManager::instance().engine()) {
        EngineManager::instance().engine()->setContext(surroundingText, cursorPos);
    }
}

void MKeyboardHost::reset()
{
    qDebug() << __PRETTY_FUNCTION__;
    switch (activeState) {
    case MInputMethod::OnScreen:
        if (EngineManager::instance().handler())
            EngineManager::instance().handler()->resetHandler();
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
    spaceInsertedAfterCommitString = false;
    backspaceMode = NormalBackspaceMode;
    backspaceTimer.stop();
    preedit.clear();
    preeditCursorPos = -1;
    preeditHasBeenEdited = false;
    if (EngineManager::instance().handler()) {
        AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler()->engineWidgetHost();
        if (engineWidgetHost && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::FloatingMode) {
            engineWidgetHost->reset();
            engineWidgetHost->hideEngineWidget();
        }
    }
    if (EngineManager::instance().engine())
        EngineManager::instance().engine()->clearEngineBuffer();
    cycleKeyHandler->reset();
}

void MKeyboardHost::prepareOrientationChange()
{
    // Suppress layout change noise during orientation change.
    regionUpdatesEnabledBeforeOrientationChange = RegionTracker::instance().enableSignals(false);

    symbolView->prepareToOrientationChange();
    vkbWidget->prepareToOrientationChange();
    AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
        EngineManager::instance().handler()->engineWidgetHost() : 0;
    if (engineWidgetHost) {
        engineWidgetHost->prepareToOrientationChange();
    }
}

void MKeyboardHost::finalizeOrientationChange()
{
    if (imToolbar) {
        // load proper layout
        imToolbar->finalizeOrientationChange();
    }

    vkbWidget->finalizeOrientationChange();
    symbolView->finalizeOrientationChange();
    if (sharedHandleArea) {
        sharedHandleArea->finalizeOrientationChange();
        if (activeState == MInputMethod::Hardware) {
            sharedHandleArea->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - sharedHandleArea->size().height());
        }
    }

    AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
        EngineManager::instance().handler()->engineWidgetHost() : 0;
    if (engineWidgetHost)
        engineWidgetHost->finalizeOrientationChange();

    // reload keyboard layout for engine when orientation is changed
    engineLayoutDirty = true;
    if (vkbWidget->isVisible()) {
        updateEngineKeyboardLayout();
    }
    // Reactivate tracker after rotation and layout has settled.
    RegionTracker::instance().enableSignals(regionUpdatesEnabledBeforeOrientationChange, sipRequested);
    regionUpdatesEnabledBeforeOrientationChange = true;
}

void MKeyboardHost::handleMouseClickOnPreedit(const QPoint &mousePos, const QRect &preeditRect)
{
    Q_UNUSED(mousePos);
    Q_UNUSED(preeditRect);
    if (!EngineManager::instance().handler())
        return;

    AbstractEngineWidgetHost *engineWidgetHost =
        EngineManager::instance().handler()->engineWidgetHost();
    // Shows suggestion list when there are some candidates.
    // Even show suggestion list when there is only original input word in candidates.
    if (!EngineManager::instance().handler()->hasErrorCorrection()
        || !engineWidgetHost
        || engineWidgetHost->candidates().size() <= 0)
        return;

    engineWidgetHost->showEngineWidget(AbstractEngineWidgetHost::DialogMode);
}


void MKeyboardHost::handleVisualizationPriorityChange(bool priority)
{
    if (visualizationPriority == priority) {
        return;
    }

    visualizationPriority = priority;

    // TODO: hide/show by fading?
    if (sipRequested) {
        if (priority) {
            MPlainWindow::instance()->sceneManager()->disappearSceneWindowNow(sceneWindow);
        } else {
            handleAppOrientationAboutToChange(appOrientationAngle);
            MPlainWindow::instance()->sceneManager()->appearSceneWindowNow(sceneWindow);
        }
    }
}

void MKeyboardHost::handleAppOrientationAboutToChange(int angle)
{
    appOrientationAngle = static_cast<M::OrientationAngle>(angle);
    if ((MPlainWindow::instance()->sceneManager()->orientationAngle() == appOrientationAngle)
        || !sipRequested || visualizationPriority) {
        return;
    }

    prepareOrientationChange();

    // The application receiving input has changed its orientation. Let's change ours.
    // Disable the transition animation for rotation so that we can rotate fast and get
    // the right snapshot for the rotation animation.
    MPlainWindow::instance()->sceneManager()->setOrientationAngle(appOrientationAngle,
                                                                  MSceneManager::ImmediateTransition);
}

void MKeyboardHost::handleAppOrientationChanged(int angle)
{
    // We'll get this call on first display of the vkb without a previous
    // handleAppOrientationAboutToChange. So, make sure our internal orientation is in sync
    // with the application's by going through the rotation explicitly (without animation).
    handleAppOrientationAboutToChange(angle);

    AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
        EngineManager::instance().handler()->engineWidgetHost() : 0;

    engineWidgetHost->handleAppOrientationChanged();
}

void MKeyboardHost::handleCandidateClicked(const QString &clickedCandidate, int index)
{
    Q_UNUSED(index);
    if (EngineManager::instance().handler()
            ->commitWhenCandidateClicked()) {
        commitString(clickedCandidate);
    }
}

void MKeyboardHost::commitString(const QString &updatedString)
{    
    AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
        EngineManager::instance().handler()->engineWidgetHost() : 0;

    if (EngineManager::instance().engine() && engineWidgetHost) {
        if (engineWidgetHost->candidates().count() > 1) {
            const int suggestionIndex = engineWidgetHost->suggestedWordIndex();
            if (suggestionIndex >= 0) {
                EngineManager::instance().engine()->setSuggestedCandidateIndex(suggestionIndex);
            }
        }
        EngineManager::instance().engine()->saveAndClearEngineBuffer();
    }

    // Add space after preedit if word tracker was clicked OR
    // if word was selected from word list when preedit was edited AND
    // cursor was at the end of preedit.
    if (engineWidgetHost
        && (engineWidgetHost->displayMode() == AbstractEngineWidgetHost::FloatingMode
            || (engineWidgetHost->displayMode() == AbstractEngineWidgetHost::DialogMode
                && preeditHasBeenEdited
                && (preedit.size() == preeditCursorPos || preeditCursorPos == -1)))) {
        inputMethodHost()->sendCommitString(updatedString + " ");
        resetInternalState();
        spaceInsertedAfterCommitString = true;
    } else {
        inputMethodHost()->sendCommitString(updatedString);
        resetInternalState();
    }
}


void MKeyboardHost::doBackspace()
{
    // note: backspace shouldn't start accurate mode
    if (EngineManager::instance().handler()
        && EngineManager::instance().handler()->cursorCanMoveInsidePreedit()
        && preedit.length() > 0) {
        if (backspaceMode != AutoBackspaceMode) {
            if ((preeditCursorPos < 0) || (preeditCursorPos == preedit.length())) {
                const int cursor = (preeditCursorPos > 0) ? (preeditCursorPos - 1) : preeditCursorPos;
                localSetPreedit(preedit.left(preedit.length() - 1), 0, 0, cursor, true);
            } else {
                if (preeditCursorPos == 0) {
                    // If preedit cursor is at the beginning of preedit, and there is a
                    // previous word before the space, then backspace will recompose the
                    // preedit, mark the word arround cursor be preedit. Otherwise,
                    // backspace just commit preedit and delete the space before the preedit.
                    QString previousWord;
                    if (needRecomposePreedit(previousWord)) {
                        preedit = previousWord + preedit;
                        preeditCursorPos = previousWord.length();
                        localSetPreedit(preedit, -previousWord.length() - 1,
                                        previousWord.length() + 1, preeditCursorPos, true);
                    } else {
                        inputMethodHost()->sendCommitString(preedit, 0, 0, 0);
                        sendBackSpaceKeyEvent();
                        resetInternalState();
                    }
                } else {
                    --preeditCursorPos;
                    localSetPreedit(preedit.remove(preeditCursorPos, 1), 0, 0, preeditCursorPos, true);
                }
            }
        } else {
            resetInternalState();
            inputMethodHost()->sendCommitString("");
            startBackspace(AutoBackspaceMode);
        }
    } else {
        QString previousWord;
        QChar removedSymbol;
        bool valid = false;
        // If error correction is enabled and there is a word before cursor,
        // backspace will recompose the preedit.
        if (backspaceMode != AutoBackspaceMode
            && correctionEnabled
            && !inputMethodHost()->hasSelection(valid)
            && valid
            && needRecomposePreedit(previousWord, &removedSymbol)) {

            // If removed symbol is a letter then it was removed from the preedit word
            // and thus preedit was edited.
            const bool preeditWordEdited = removedSymbol.isLetter();
            preedit = previousWord;
            preeditCursorPos = previousWord.length();
            localSetPreedit(preedit, -previousWord.length() - 1, previousWord.length() + 1,
                            preeditCursorPos, preeditWordEdited);
        } else {
            sendBackSpaceKeyEvent();
        }
    }
    // Backspace toggles shift off if it's latched except if:
    // - autoCaps is on and cursor is at 0 position
    // - text is selected (since we don't know what is
    //   selected we can't predict the next state)
    // - previous cursor position will have autoCaps
    if (EngineManager::instance().handler()
        && EngineManager::instance().handler()->hasAutoCaps()
        && vkbWidget->shiftStatus() == ModifierLatchedState
        && (!autoCapsEnabled || cursorPos != 0)
        && !hasSelection
        && (cursorPos == 0
            || !autoCapsEnabled
            || (cursorPos > 0
                && cursorPos <= surroundingText.length()
                && !surroundingText.left(cursorPos-1).contains(AutoCapsTrigger)))) {
        vkbWidget->setShiftState(ModifierClearState);
    }
}

void MKeyboardHost::autoBackspace()
{
    backspaceMode = AutoBackspaceMode;
    backspaceTimer.start(BackspaceRepeatInterval); // Must restart before doBackspace
    doBackspace();
}

void MKeyboardHost::handleKeyPress(const KeyEvent &event)
{

    if (EngineManager::instance().handler()
        && EngineManager::instance().handler()->handleKeyPress(event))
        return;

    if (event.qtKey() == Qt::Key_Shift) {
        if (shiftHeldDown) {
            return; //ignore duplicated event
        }

        if (activeState == MInputMethod::OnScreen && enableMultiTouch) {
            shiftHeldDown = true;
        }
    } else if (event.specialKey() == KeyEvent::Sym) {

        if (activeState == MInputMethod::OnScreen
            && event.isFromPrimaryTouchPoint()
            && vkbWidget == static_cast<MVirtualKeyboard *>(sender())) {
            showSymbolView(SymbolView::FollowMouseShowMode,
                           event.scenePosition());
            return;
        }
    }

    MInputMethod::EventRequestType requestType = MInputMethod::EventRequestSignalOnly;
    if ((inputMethodMode == M::InputMethodModeDirect)
         && (event.specialKey() == KeyEvent::NotSpecial)) {

        requestType = MInputMethod::EventRequestBoth;
        inputMethodHost()->sendKeyEvent(event.toQKeyEvent(), requestType);

    } else if (event.qtKey() == Qt::Key_Backspace) {
        AbstractEngineWidgetHost *engineWidgetHost =
            EngineManager::instance().handler() ?
            EngineManager::instance().handler()->engineWidgetHost() : 0;
        if ( engineWidgetHost
            && engineWidgetHost->isActive()
            && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::FloatingMode) {
            // hide word tracker when backspace key press
            if (EngineManager::instance().handler()->correctionAcceptedWithSpaceEnabled()) {
                // WordTrackerBackspaceMode mode: hide word tracker when backspace key press.
                // And remove preedit if holding backspace long enough. But does nothing
                // for backspace key release.
                engineWidgetHost->hideEngineWidget();
                startBackspace(WordTrackerBackspaceMode);
            } else {
                startBackspace(NormalBackspaceMode);
            }
        } else {
            startBackspace(NormalBackspaceMode);
        }
    }
}

void MKeyboardHost::handleKeyRelease(const KeyEvent &event)
{
    if (EngineManager::instance().handler()
        && EngineManager::instance().handler()->handleKeyRelease(event))
        return;

    if (event.qtKey() == Qt::Key_Shift) {
        if (!shiftHeldDown) {
            return; //ignore duplicated event
        }

        if (activeState == MInputMethod::OnScreen && enableMultiTouch) {
            shiftHeldDown = false;
        }
    }

    if ((inputMethodMode == M::InputMethodModeDirect)
         && (event.specialKey() == KeyEvent::NotSpecial)) {

        inputMethodHost()->sendKeyEvent(event.toQKeyEvent(), MInputMethod::EventRequestBoth);

    } else if (event.qtKey() == Qt::Key_Backspace) {
        if (backspaceTimer.isActive()) {
            backspaceTimer.stop();
            // If the backspace Mode is WordTrackerBackspaceMode or
            // AutoBackspaceMode, don't need to do backspace.
            if (backspaceMode != WordTrackerBackspaceMode
                && backspaceMode != AutoBackspaceMode) {
                doBackspace();
            }
            backspaceMode = NormalBackspaceMode;
        }
    }
}

void MKeyboardHost::handleKeyClick(const KeyEvent &event)
{
    if (EngineManager::instance().handler()
        && EngineManager::instance().handler()->handleKeyClick(event)) {
        // After the key event is consumed by the proper engine handler, the "shift" state
        // should be updated accordingly right here because the engine handler can not
        // do it.
        if ((vkbWidget->shiftStatus() == ModifierLatchedState)
             && (!shiftHeldDown))
            vkbWidget->setShiftState(ModifierClearState);
        return;
    }

    // Don't need send key events for Direct input mode here.
    // already send in handleKeyPress and handleKeyRelease.
    if (activeState == MInputMethod::Hardware && inputMethodMode != M::InputMethodModeDirect) {
        // In hardware keyboard mode symbol view is just another source for
        // events that will be handled by duihardwarekeyboard.  The native
        // modifiers may not be correct (depending on the current hwkbd modifier
        // state) but that doesn't matter.
        processKeyEvent(QEvent::KeyPress, event.qtKey(), event.modifiers(),
                        event.text(), false, 1, 0, 0, 0);
        processKeyEvent(QEvent::KeyRelease, event.qtKey(), event.modifiers(),
                        event.text(), false, 1, 0, 0, 0);
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
               && (event.qtKey() == Qt::Key_Return)
               && (!shiftHeldDown || autoCapsTriggered)) {
        // Enter will toggle latched shift off only when
        // autoCaps is not enabled.
        if (!autoCapsEnabled) {
            vkbWidget->setShiftState(ModifierClearState);
        }
    } else if (vkbWidget->shiftStatus() == ModifierLatchedState
               && (event.qtKey() == Qt::Key_Space)
               && (!shiftHeldDown || autoCapsTriggered)) {
        // Space will toggle latched shift off except if:
        // - text is selected (since we don't know what is
        //   selected we can't predict the next state)
        // - next cursor position will have autoCaps
        if (!autoCapsEnabled
            || (!hasSelection
                && cursorPos <= surroundingText.length()
                && !surroundingText.left(cursorPos).contains(AutoCapsTrigger))) {
            vkbWidget->setShiftState(ModifierClearState);
        }
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
        qWarning() << __PRETTY_FUNCTION__
                   << "KeyEvent::LayoutMenu is unsupported";
    } else if (event.specialKey() == KeyEvent::Sym) {
        handleSymbolKeyClick();
    } else if (event.specialKey() == KeyEvent::Switch) {
        if (symbolView->isActive()) {
            symbolView->switchToNextPage();
        }
    } else if (event.specialKey() == KeyEvent::ChangeSign) {
        togglePlusMinus();
    }
}

void MKeyboardHost::handleLongKeyPress(const KeyEvent &event)
{
    AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
        EngineManager::instance().handler()->engineWidgetHost() : 0;
    // long tap space key when word tracker is visible will switch to word list.
    if (event.qtKey() == Qt::Key_Space
        && correctionEnabled
        && engineWidgetHost
        && engineWidgetHost->isActive()
        && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::FloatingMode
        && engineWidgetHost->candidates().size() > 0) {
        // This press event is done. Current touch events are still being
        // delivered to vkb/symbol view (actually, to one of its children) so
        // we send cancel event to it.
        MCancelEvent cancel;

        if (symbolView->isActive()) {
            symbolView->scene()->sendEvent(symbolView, &cancel);
        } else {
            vkbWidget->scene()->sendEvent(vkbWidget, &cancel);
        }
        engineWidgetHost->showEngineWidget(AbstractEngineWidgetHost::DialogMode);
    }
}

void MKeyboardHost::handleKeyCancel(const KeyEvent &event)
{
    if (event.qtKey() == Qt::Key_Backspace) {
        backspaceMode = NormalBackspaceMode;
        backspaceTimer.stop();
    } else if (event.qtKey() == Qt::Key_Shift) {
        shiftHeldDown = false;
    }
}

void MKeyboardHost::sendCommitStringOrReturnEvent(const KeyEvent &event) const
{
    // We send return as a normal key event instead of an input method event so
    // that it properly triggers returnPressed signal emission in MTextEdit and
    // QLineEdit (and possibly in other similar targets).
    if (event.qtKey() == Qt::Key_Return) {
        inputMethodHost()->sendKeyEvent(KeyEvent(event, QEvent::KeyPress).toQKeyEvent(), MInputMethod::EventRequestEventOnly);
        inputMethodHost()->sendKeyEvent(event.toQKeyEvent(), MInputMethod::EventRequestEventOnly);
    } else {
        inputMethodHost()->sendCommitString(event.text());
    }
}

void MKeyboardHost::handleTextInputKeyClick(const KeyEvent &event)
{
    const bool spaceInsertedAfterCommitStringPrev(spaceInsertedAfterCommitString);
    if (!((event.specialKey() == KeyEvent::Sym)
          || (event.specialKey() == KeyEvent::Switch)
          || (event.specialKey() == KeyEvent::Sym)
          || (event.qtKey() == Qt::Key_Shift))) {
        spaceInsertedAfterCommitString = false;
    }

    // Discard KeyPress & Drop type of events.
    if (event.type() != QEvent::KeyRelease

        // Discard also special keys except cycleset which has text
        // that can be sent to input context.
        || (!(event.specialKey() == KeyEvent::NotSpecial
              || event.specialKey() == KeyEvent::CycleSet))

        // Finally, discard Qt backspace, which is handled in
        // handleKeyPress/Release.
        || (event.qtKey() == Qt::Key_Backspace)
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
            inputMethodHost()->sendCommitString(preedit, 0, 0, preeditCursorPos);
            if (EngineManager::instance().engine())
                EngineManager::instance().engine()->clearEngineBuffer();
            preedit.clear();
            preeditCursorPos = -1;
        }

        sendCommitStringOrReturnEvent(event);

    } else if ((event.qtKey() == Qt::Key_Space) || (event.qtKey() == Qt::Key_Return) || (event.qtKey() == Qt::Key_Tab)) {
        // commit suggestion if correction candidate widget is visible and with popupMode
        // or ignore it if correction widget is visible and with suggestionlist mode
        // otherwise commit preedit
        bool eventSent(false);
        AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
            EngineManager::instance().handler()->engineWidgetHost() : 0;
        if (event.qtKey() == Qt::Key_Space
            && engineWidgetHost
            && engineWidgetHost->isActive()
            && EngineManager::instance().handler()->correctionAcceptedWithSpaceEnabled()) {
            if (engineWidgetHost->displayMode() == AbstractEngineWidgetHost::FloatingMode) {
                spaceInsertedAfterCommitString = true;
                const QString suggestion = engineWidgetHost->candidates().at(engineWidgetHost->suggestedWordIndex());
                inputMethodHost()->sendCommitString(suggestion + " ");
                eventSent = true;
            } else {
                // ignore space click when word list is visible.
                return;
            }
        } else {
            if (!preedit.isEmpty()) {
                // if cursor is inside preedit. need commit preedit
                // and reposition cursor before it, then insert space/return
                // also need check whether widget supports the surrounding text query.
                const bool needRepositionCursor = preeditCursorPos != -1
                                                  && preeditCursorPos != preedit.length()
                                                  && inputMethodHost()->surroundingText(surroundingText, cursorPos)
                                                  && (cursorPos >= 0);
                int eventCharactersInserted(0);
                eventSent = event.qtKey() != Qt::Key_Return;
                if (eventSent) {
                    preedit.insert(needRepositionCursor ? preeditCursorPos : preedit.size(), event.text());
                    eventCharactersInserted = 1;
                }

                inputMethodHost()->sendCommitString(
                    preedit, 0, 0, needRepositionCursor ? (preeditCursorPos + eventCharactersInserted) : -1);
            }
        }


        if (lastClickEvent.specialKey() != KeyEvent::CycleSet) {
            if (engineWidgetHost) {
                engineWidgetHost->reset();
            }
        }
        // Send trailing space.
        if (!eventSent) {
            sendCommitStringOrReturnEvent(event); // it's always return here
        }

        if (EngineManager::instance().engine())
            EngineManager::instance().engine()->clearEngineBuffer();
        preedit.clear();
        preeditCursorPos = -1;
    } else if (spaceInsertedAfterCommitStringPrev && (text.length() == 1)
               && AutoPunctuationTriggers.contains(text[0])) {
        sendBackSpaceKeyEvent();
        resetInternalState();
        inputMethodHost()->sendCommitString(text + " ");
    } else {
        // append text to the end of preedit if cursor is at the end of
        // preedit (or cursor is -1, invisible). Or if cursor is inside
        // preedit, insert text after cursor.
        if (preeditCursorPos < 0 || preeditCursorPos == preedit.length()) {
            if (preeditCursorPos < 0) {
                if (preedit.isEmpty()) {
                    preeditCursorPos = text.length();
                }
            } else {
                preeditCursorPos += text.length();
            }
            preedit += text;
            // send touch point to engine if not in symbol view
            // otherwise send character to engine.
            if (EngineManager::instance().engine()) {
                if (!symbolView->isActive())
                    EngineManager::instance().engine()->tapKeyboard(event.correctionPosition(),
                            event.modifiers() & Qt::ShiftModifier, text.at(0));
                else
                    EngineManager::instance().engine()->appendCharacter(text.at(0));
            }
        } else {
            preedit.insert(preeditCursorPos, text);
            preeditCursorPos += text.length();
            if (EngineManager::instance().engine()) {
                EngineManager::instance().engine()->clearEngineBuffer();
                EngineManager::instance().engine()->reselectString(preedit);
            }
        }

        QStringList candidates;
        bool preeditInDict = false;
        if (EngineManager::instance().engine()) {
            candidates = EngineManager::instance().engine()->candidates();
            preeditInDict = (EngineManager::instance().engine()->candidateSource(0) != MImEngine::DictionaryTypeInvalid);
        }

        AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
            EngineManager::instance().handler()->engineWidgetHost() : 0;
        if (engineWidgetHost) {
            engineWidgetHost->setCandidates(candidates);
        }
        preeditHasBeenEdited = true;

        // update preedit before showing word tracker
        updatePreedit(preedit, candidates.count(), preeditInDict, 0, 0, preeditCursorPos);

        if ((preeditCursorPos < 0 || preeditCursorPos == preedit.length())
             && EngineManager::instance().engine()
             && engineWidgetHost) {
            MImEngine::DictionaryType sourceDictionaryType
                = EngineManager::instance().engine()->candidateSource(0);
            // because the written word (preedit) should be on the top of candidate list.
            // So when there are some other candidates and the preedit is not a valid
            // dictionary word, show word tracker.
            if (candidates.count() > 1
                && sourceDictionaryType == MImEngine::DictionaryTypeInvalid) {
                setEngineWidgetHostPosition(engineWidgetHost);
                engineWidgetHost->showEngineWidget(AbstractEngineWidgetHost::FloatingMode);
            } else {
                engineWidgetHost->hideEngineWidget();
            }
        } else if (engineWidgetHost && engineWidgetHost->isActive()) {
            engineWidgetHost->hideEngineWidget();
        }
    }
}

void MKeyboardHost::updateCorrectionState()
{
    if (activeState == MInputMethod::Hardware) {
        inputMethodHost()->setGlobalCorrectionEnabled(false);
        correctionEnabled = false;
    } else {
        if (!EngineManager::instance().engine()) {
            inputMethodHost()->setGlobalCorrectionEnabled(false);
            correctionEnabled = false;
            return;
        }

        // Don't use correction for certain types.
        bool enabledByContent = true, ctValid;
        int contentType = inputMethodHost()->contentType(ctValid);
        if (ctValid && (contentType == M::NumberContentType
                        || contentType == M::PhoneNumberContentType
                        || contentType == M::EmailContentType
                        || contentType == M::UrlContentType)) {
            enabledByContent = false;
        }

        // Enable correction if correction is enabled from MTextEdit and prediction
        // is not disabled (Qt::ImhNoPredictiveText hint not set). Ignore either
        // value if value is not set.
        bool ecValid = false, pValid = false;
        bool ecEnabled = inputMethodHost()->correctionEnabled(ecValid);
        bool pEnabled = inputMethodHost()->predictionEnabled(pValid);
        correctionEnabled = enabledByContent
                            && (!ecValid || ecEnabled)
                            && (!pValid || pEnabled)
                            && EngineManager::instance().engine()
                            && EngineManager::instance().engine()->correctionEnabled()
                            && EngineManager::instance().engine()->completionEnabled();

        // info context the global correction option
        // TODO: should not put setGlobalCorrectionEnabled here, it will send correction setting
        // whenever focus changes. But have to at this moment, because im-uiserver start before
        // application, and there is no focus widget, no activateContext, calling
        // setGlobalCorrectionEnabled() at that time, can not record the setting.
        // Only after the application is running, this setGlobalCorrectionEnabled() can take effect
        if (EngineManager::instance().engine())
            inputMethodHost()->setGlobalCorrectionEnabled(
                    EngineManager::instance().engine()->correctionEnabled());
    }
}

void MKeyboardHost::onPluginsChange()
{
    // we should call vkbWidget->enableSinglePageHorizontalFlick() every time when
    // other plugin is loaded or unloaded
    QList<MImPluginDescription> pluginDescriptions = inputMethodHost()->pluginDescriptions(MInputMethod::OnScreen);
    int enabledPluginsCount = 0;

    foreach (const MImPluginDescription &description, pluginDescriptions) {
        if (description.enabled()) {
            ++enabledPluginsCount;
        }
    }
    vkbWidget->enableSinglePageHorizontalFlick(enabledPluginsCount > 1);
}

void MKeyboardHost::userHide()
{
    if (!preedit.isEmpty()) {
        sendString(preedit);
    }
    hide();
    inputMethodHost()->notifyImInitiatedHiding();
    resetInternalState();
}


void MKeyboardHost::sendCopyPaste(CopyPasteState action)
{
    switch (action) {
    case InputMethodCopy:
        inputMethodHost()->copy();
        break;
    case InputMethodPaste:
        inputMethodHost()->paste();
        break;
    default:
        qDebug() << __PRETTY_FUNCTION__ << "invalid action" << action;
        break;
    }
}

void MKeyboardHost::switchPlugin(MInputMethod::SwitchDirection direction)
{
    if (EngineManager::instance().handler()) {
        EngineManager::instance().handler()->editingInterrupted();
        EngineManager::instance().handler()->preparePluginSwitching();
    }
    inputMethodHost()->switchPlugin(direction);
}

void MKeyboardHost::sendKeyEvent(const QKeyEvent &key)
{
    inputMethodHost()->sendKeyEvent(key);
}

void MKeyboardHost::sendString(const QString &text)
{
    inputMethodHost()->sendCommitString(text);
}

void MKeyboardHost::sendStringFromToolbar(const QString &text)
{
    if (EngineManager::instance().handler())
        EngineManager::instance().handler()->clearPreedit(true);
    reset();
    sendString(text);
}

void MKeyboardHost::setToolbar(QSharedPointer<const MToolbarData> toolbar)
{
    toolbarHidePending = false;

    if (toolbar && toolbar->isVisible()) {
        const MToolbarData *oldToolbar = imToolbar->currentToolbarData();
        imToolbar->showToolbarWidget(toolbar);
        // if current is in Hardware state, and no toolbar being visible before,
        // start animation to show new toolbar.
        if (!oldToolbar && activeState == MInputMethod::Hardware) {
            prepareHideShowAnimation();
            slideUpAnimation.start();
        }
    } else if (haveFocus) {
        imToolbar->hideToolbarWidget();
    } else {
        toolbarHidePending = true;
    }
}

void MKeyboardHost::processKeyEvent(QEvent::Type keyType, Qt::Key keyCode,
                                    Qt::KeyboardModifiers modifiers, const QString &text,
                                    bool autoRepeat, int count, quint32 nativeScanCode,
                                    quint32 nativeModifiers, unsigned long time)
{
    if ((activeState != MInputMethod::Hardware) ||
        !hardwareKeyboard->filterKeyEvent(keyType, keyCode, modifiers, text,
                                          autoRepeat, count, nativeScanCode,
                                          nativeModifiers, time)) {
        inputMethodHost()->sendKeyEvent(QKeyEvent(keyType, keyCode, modifiers, text,
                                                  autoRepeat, count),
                                        MInputMethod::EventRequestEventOnly);
    }
    //XXX
    //to logicstatemachine
}

void MKeyboardHost::handleClientChange()
{
    // There are some cases when the keyboard is not hidden with animation and the key
    // overrides must be reset in this situations as well (e.g. changing between applications)
    if (keyOverrideClearPending) {
        QMap<QString, QSharedPointer<MKeyOverride> > emptyOverrides;
        vkbWidget->setKeyOverrides(emptyOverrides);
        symbolView->setKeyOverrides(emptyOverrides);
        keyOverrideClearPending = false;
        overrides = emptyOverrides;
    }

    hardwareKeyboard->clientChanged();
    resetInternalState();
    if (sipRequested) {
        hide();
    }
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

    if (activeState == MInputMethod::OnScreen
        && EngineManager::instance().handler())
        EngineManager::instance().handler()->editingInterrupted();

    // Resets before changing the activeState to make sure clear.
    resetInternalState();
    activeState = actualState;

    //TODO: update engine language if hardwarekeyboard supports
    // error correction/prediction

    AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
        EngineManager::instance().handler()->engineWidgetHost() : 0;
    if ( engineWidgetHost
         && engineWidgetHost->isActive()
         && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::DockedMode) {
        engineWidgetHost->setPageIndex(0);
    }

    // Keeps separate states for symbol view in OnScreen state and Hardware state
    if (activeState == MInputMethod::OnScreen) {
        hideLockOnInfoBanner();
        inputMethodHost()->setInputModeIndicator(MInputMethod::NoIndicator);

        disconnect(hardwareKeyboard, SIGNAL(deadKeyStateChanged(const QChar &)),
                   this, SLOT(handleHwKeyboardStateChanged()));
        disconnect(hardwareKeyboard, SIGNAL(modifiersStateChanged()),
                   this, SLOT(handleHwKeyboardStateChanged()));
        disconnect(hardwareKeyboard, SIGNAL(scriptChanged()),
                   this, SLOT(handleHwKeyboardStateChanged()));
        if (haveFocus) {
            hardwareKeyboard->disable();
        }
        if (sipRequested) {
            slideUpAnimation.stop();
            vkbWidget->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - vkbWidget->size().height());
            vkbWidget->show();
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
        vkbWidget->hide();
        vkbWidget->resetState();
        if (sipRequested) {
            slideUpAnimation.stop();
            sharedHandleArea->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - sharedHandleArea->size().height());
        }
    }

    // Hide symbol view before changing the state.
    // TODO: Wait until transition animation has been finished
    // before changing state when re-enabling animation for
    // symbolview hiding.
    symbolView->hideSymbolView();

    symbolView->setKeyboardState(actualState);
    if (EngineManager::instance().handler()) {
        if (EngineManager::instance().handler()->hasErrorCorrection())
            updateCorrectionState();
        if (EngineManager::instance().handler()->hasAutoCaps())
            updateAutoCapitalization();
    }
}

void MKeyboardHost::handleSymbolKeyClick()
{
    if (!vkbWidget->symViewAvailable()
        || symbolView->isTemporarilyActive()
        || ((activeState == MInputMethod::Hardware)
            && !hardwareKeyboard->symViewAvailable())) {
        return;
    }

    // Toggle SymbolView.
    if (!symbolView->isVisible()) {
        showSymbolView(SymbolView::NormalShowMode);
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

void MKeyboardHost::showSymbolView(SymbolView::ShowMode showMode,
                                   const QPointF &initialScenePress)
{
    // We always send a cancel event when showing symbol view, even when showSymbolView is
    // not called from handleKeyPress which is the case where cancel event is the obvious
    // thing to do.  This is because user may hold shift down and click symbol key, which
    // in that case is handled by handleSymbolKeyClick, because sym is not the primary
    // touchpoint.  Also in this case we want to reset the active key area so that when
    // the symbol view is exited, keys won't be in level 1 even when shift key state is
    // normal.
    MCancelEvent cancel;
    vkbWidget->scene()->sendEvent(vkbWidget, &cancel);

    symbolView->setPos(0, MPlainWindow::instance()->visibleSceneSize().height() - symbolView->size().height());
    symbolView->showSymbolView(showMode, initialScenePress);
    //give the symbolview right shift level(for hardware state)
    updateSymbolViewLevel();
}

MInputMethod::InputModeIndicator MKeyboardHost::deadKeyToIndicator(const QChar &key)
{
    switch (key.unicode()) {
    case 0x00b4:
        return MInputMethod::DeadKeyAcuteIndicator;
    case 0x02c7:
        return MInputMethod::DeadKeyCaronIndicator;
    case 0x005e:
        return MInputMethod::DeadKeyCircumflexIndicator;
    case 0x00a8:
        return MInputMethod::DeadKeyDiaeresisIndicator;
    case 0x0060:
        return MInputMethod::DeadKeyGraveIndicator;
    case 0x007e:
        return MInputMethod::DeadKeyTildeIndicator;
    default:
        return MInputMethod::NoIndicator;
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

    MInputMethod::InputModeIndicator indicatorState
        = deadKeyToIndicator(hardwareKeyboard->deadKeyState());

    currentIndicatorDeadKey = false;

    if (indicatorState != MInputMethod::NoIndicator) {
        currentIndicatorDeadKey = true;
    } else if (fnState == ModifierLockedState) {
        indicatorState = MInputMethod::NumAndSymLockedIndicator;

    } else if (fnState == ModifierLatchedState) {
        indicatorState = MInputMethod::NumAndSymLatchedIndicator;

    } else if (xkbLayout == "ara"
        && xkbVariant.isEmpty()) {
        indicatorState = MInputMethod::ArabicIndicator;

    } else if (xkbVariant.isEmpty() || xkbVariant == "latin") {
        indicatorState = MInputMethod::LatinLowerIndicator;
        if (shiftState == ModifierLockedState) {
            indicatorState = MInputMethod::LatinLockedIndicator;
        } else if (shiftState == ModifierLatchedState) {
            indicatorState = MInputMethod::LatinUpperIndicator;
        }

    } else if (xkbVariant == "cyrillic") {
        indicatorState = MInputMethod::CyrillicLowerIndicator;
        if (shiftState == ModifierLockedState) {
            indicatorState = MInputMethod::CyrillicLockedIndicator;
        } else if (shiftState == ModifierLatchedState) {
            indicatorState = MInputMethod::CyrillicUpperIndicator;
        }
    }

    inputMethodHost()->setInputModeIndicator(indicatorState);

    QString lockOnNotificationLabel;

    if (indicatorState == MInputMethod::LatinLockedIndicator
        || indicatorState == MInputMethod::CyrillicLockedIndicator) {
        //% "Caps lock on"
        lockOnNotificationLabel = qtTrId("qtn_hwkb_caps_lock");
    } else if (indicatorState == MInputMethod::NumAndSymLockedIndicator) {
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
        // It is needed to set the proper style name to have properly wrapped, multiple lines
        // with too much content. The MBanner documentation also emphasises to specify the
        // style name for the banners explicitly in the code.
        modifierLockOnBanner->setStyleName("InformationBanner");
        modifierLockOnBanner->setObjectName(NotificationObjectName);
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

QList<MAbstractInputMethod::MInputMethodSubView>
MKeyboardHost::subViews(MInputMethod::HandlerState state) const
{
    QList<MAbstractInputMethod::MInputMethodSubView> sViews;
    if (state == MInputMethod::OnScreen) {
        QMap<QString, QString> availableLayouts = LayoutsManager::instance().availableLayouts();
        QMap<QString, QString>::const_iterator i = availableLayouts.constBegin();
        while (i != availableLayouts.constEnd()) {
            MAbstractInputMethod::MInputMethodSubView subView;
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

void MKeyboardHost::showLanguageNotification()
{
    if (activeState == MInputMethod::OnScreen && vkbWidget) {
        vkbWidget->showLanguageNotification();
    }
}

void MKeyboardHost::handleVirtualKeyboardLayoutChanged(const QString &layout)
{
    // reset shift state when layout is changed
    resetVirtualKeyboardShiftState();
    if (symbolView) {
        symbolView->setLayout(layout);
    }

    // update language properties
    EngineManager::instance().updateLanguage(vkbWidget->layoutLanguage());

    resetInternalState();

    engineLayoutDirty = true;
    if (vkbWidget->isVisible())
        updateEngineKeyboardLayout();

    if (EngineManager::instance().handler()
        && EngineManager::instance().handler()->hasAutoCaps())
        updateAutoCapitalization();
    emit activeSubViewChanged(layout);
}

void MKeyboardHost::updateEngineKeyboardLayout()
{
    // NOTE: This method should not be called before vkbWidget is visible, i.e.
    //       show() has been called to it. This is because vkbWidget->show(),
    //       recreates the keyboard widgets for the current setup and therefore
    //       correct keyboard layout cannot be retreived before that.
    Q_ASSERT(vkbWidget->isVisible());

    if (!EngineManager::instance().engine() || !engineLayoutDirty)
        return;

    if (activeState == MInputMethod::OnScreen
        && EngineManager::instance().handler()
        && EngineManager::instance().handler()->supportTouchPointAccuracy()) {
        const bool status(EngineManager::instance().engine()->setKeyboardLayoutKeys(
                                vkbWidget->mainLayoutKeys()));
        Q_UNUSED(status);
        Q_ASSERT(status);
    }
    engineLayoutDirty = false;
}

void MKeyboardHost::updatePreedit(const QString &string, int candidateCount, bool preeditInDictionary,
                                  int replaceStart, int replaceLength, int cursor)
{
    // Preedit style is MInputMethod::PreeditNoCandidates only if there are no
    // other candidate suggestions from engine AND preedit cannot be found
    // from dictionary.
    MInputMethod::PreeditFace face = MInputMethod::PreeditDefault;
    if (candidateCount <= 1 && !preeditInDictionary) {
        face = MInputMethod::PreeditNoCandidates;
    }

    QList<MInputMethod::PreeditTextFormat> preeditFormats;
    MInputMethod::PreeditTextFormat preeditFormat(0, string.length(), face);
    preeditFormats << preeditFormat;
    inputMethodHost()->sendPreeditString(string, preeditFormats, replaceStart, replaceLength, cursor);
}

void MKeyboardHost::startBackspace(MKeyboardHost::BackspaceMode mode)
{
    backspaceMode = mode;
    backspaceTimer.start(AutoBackspaceDelay);
}

void MKeyboardHost::updateCorrectionWidgetPosition()
{
    AbstractEngineWidgetHost *engineWidgetHost = EngineManager::instance().handler() ?
        EngineManager::instance().handler()->engineWidgetHost() : 0;
    if (correctionEnabled && engineWidgetHost && engineWidgetHost->isActive()
        && engineWidgetHost->displayMode() == AbstractEngineWidgetHost::FloatingMode) {
        setEngineWidgetHostPosition(engineWidgetHost);
    }
}

void MKeyboardHost::setEngineWidgetHostPosition(AbstractEngineWidgetHost *engineWidgetHost)
{
    bool success = false;
    const QRect rectVsScene = inputMethodHost()->cursorRectangle(success);
    // invalid cursorRectangle should not be ignored, wordTracker must hide itself in that case
    QRect rect;
    if (success && rectVsScene.isValid()) {
        rect = sceneWindow->mapRectFromScene(rectVsScene).toRect();
        // hide wordtracker if cursor is covered by keyboard
        if (rect.top() > MPlainWindow::instance()->sceneManager()->visibleSceneSize().height() - keyboardHeight()) {
            rect = QRect();
        }
    }
    engineWidgetHost->setPosition(rect);
}

void MKeyboardHost::sendBackSpaceKeyEvent() const
{
    const KeyEvent pressEvent("\b", QEvent::KeyPress, Qt::Key_Backspace,
                              KeyEvent::NotSpecial,
                              vkbWidget->shiftStatus() != ModifierClearState
                              ? Qt::ShiftModifier : Qt::NoModifier);

    const KeyEvent releaseEvent("\b", QEvent::KeyRelease, Qt::Key_Backspace,
                                KeyEvent::NotSpecial,
                                vkbWidget->shiftStatus() != ModifierClearState
                                ? Qt::ShiftModifier : Qt::NoModifier);

    inputMethodHost()->sendKeyEvent(pressEvent.toQKeyEvent(),
                                    MInputMethod::EventRequestEventOnly);
    inputMethodHost()->sendKeyEvent(releaseEvent.toQKeyEvent(),
                                    MInputMethod::EventRequestEventOnly);
}

void MKeyboardHost::togglePlusMinus()
{
    QString text;
    int cursorPos;

    // Get current text
    if (!inputMethodHost()->surroundingText(text, cursorPos))
        return;

    // Sanity check for cursor pos
    if (cursorPos > text.length() || cursorPos < 0)
        return;

    // Following code will send (replace or insert) only the sign mark
    // because it is dangerous to resend the whole word. When
    // result is disallowed by validator sending the word back
    // will erase the word instead of just the sign mark.

    int wordBegin = cursorPos; // Beginning of word to prepend the sign.

    // Check whether text[cursorPos] is the character to work on (has or will have sign).
    bool cursorIsAtFirstChar = cursorPos == 0
                               || (text[cursorPos-1].isSpace()
                                   && (text[cursorPos] == PlusSign
                                       || text[cursorPos] == MinusSign));

    // Check if need to seek backward for the current word's beginning.
    if (!cursorIsAtFirstChar) {
        // Find start of "number" under cursor & track word
        for (QString::const_iterator i = text.begin() + cursorPos - 1;
            i >= text.begin(); --i) {

            // Space is a start of number
            if (i->isSpace())
                break;

            --wordBegin;

            // Plus & minus signs are a start of number
            if (i < text.begin() + cursorPos && // If cursor is on plus/minus look for previous word
                (*i == PlusSign || *i == MinusSign))
                break;
        }
    }

    // Not exactly the whole word length but from beginning
    // of word to old cursor.
    int wordLength = cursorPos - wordBegin;

    char sign = (wordLength >= 0)
                ? text[wordBegin].toAscii() : 0;

    // Change/add sign
    int replaceCount;
    if (sign && sign == MinusSign) { // Change minus to plus
        sign = PlusSign;
        replaceCount = 1;
    } else if (sign && sign == PlusSign) { // Change plus to minus
        sign = MinusSign;
        replaceCount = 1;
    } else { // Add minus
        sign = MinusSign;
        replaceCount = 0;
    }

    // If replacement is done add one to new cursor position.
    // Position is relative to start of committed string.
    // Note: There is currently no way of knowing if adding a sign did not succeed,
    // due to validator, for example. In that case we incorrectly move cursor one step forward.
    int newCursorPos = wordLength + (1 - replaceCount);

    // Update host
    inputMethodHost()->sendCommitString(QString(sign),
                                        -wordLength,
                                        replaceCount,
                                        newCursorPos);
}

void MKeyboardHost::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &newOverrides)
{
    if (!haveFocus && newOverrides.size() == 0) {
        keyOverrideClearPending = true; // not changing overrides while hiding
    } else {
        keyOverrideClearPending = false;
        overrides = newOverrides;
        vkbWidget->setKeyOverrides(overrides);
        symbolView->setKeyOverrides(overrides);
    }
}

bool MKeyboardHost::needRecomposePreedit(QString &previousWord, QChar *removedSymbol)
{
    // check whether there is a previous word + space before cursor.
    bool needRecomposePreedit = false;
    if (inputMethodHost()->surroundingText(surroundingText, cursorPos)
        && !surroundingText.isEmpty()
        && cursorPos > 0) {
        previousWord = surroundingText.left(cursorPos - 1);
        if (removedSymbol)
            *removedSymbol = surroundingText[cursorPos-1];
        if (!previousWord.isEmpty()) {
            // ignore space, punct and symbol.
            const QChar lastChar = previousWord.at(previousWord.length() - 1);
            if (!lastChar.isSpace() && !lastChar.isPunct() && !lastChar.isSymbol()) {
                QTextBoundaryFinder finder(QTextBoundaryFinder::Word, previousWord);
                finder.setPosition(previousWord.length());
                const int lastWordBreak = finder.toPreviousBoundary();
                if (lastWordBreak > 0)
                    previousWord = previousWord.right(previousWord.length() - lastWordBreak);
                needRecomposePreedit = true;
            }
        }
    }
    return needRecomposePreedit;
}

void MKeyboardHost::setKeyOverridesActive(bool active)
{
    if (active) {
        vkbWidget->setKeyOverrides(overrides);
        symbolView->setKeyOverrides(overrides);
    } else {
        QMap<QString, QSharedPointer<MKeyOverride> > emptyOverrides;
        vkbWidget->setKeyOverrides(emptyOverrides);
        symbolView->setKeyOverrides(emptyOverrides);
    }
}

void MKeyboardHost::handleToggleKeyStateChanged(bool onOff)
{
    this->vkbWidget->setToggleKeyState(onOff);
}

void MKeyboardHost::handleComposeKeyStateChanged(bool composing)
{
    this->vkbWidget->setComposeKeyState(composing);
}

int MKeyboardHost::keyboardHeight() const
{
    int height = 0;
    if (symbolView->isActive()) {
        height = symbolView->size().height();
    } else if (vkbWidget->isVisible()) {
        height = vkbWidget->size().height();
    }

    if (sharedHandleArea->isVisible()) {
        height += sharedHandleArea->size().height() - sharedHandleArea->shadowHeight();
    }
    return height;
}
