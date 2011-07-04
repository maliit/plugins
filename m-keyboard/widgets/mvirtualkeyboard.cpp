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



#include "mvirtualkeyboard.h"
#include "mvirtualkeyboardstyle.h"
#include "horizontalswitcher.h"
#include "keyboarddata.h"
#include "layoutdata.h"
#include "layoutsmanager.h"
#include "notification.h"
#include "mimkeymodel.h"
#include "mimtoolbar.h"
#include "mimabstractkey.h"
#include "keyevent.h"
#include "grip.h"
#include "reactionmappainter.h"
#include "regiontracker.h"
#include "reactionmapwrapper.h"

#include <mtoolbardata.h>
#include <mkeyoverride.h>

#include <QDebug>
#include <QPainter>
#include <QGraphicsLinearLayout>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsWidget>
#include <QPropertyAnimation>
#include <QPixmap>

#include <MCancelEvent>
#include <MButton>
#include <MScalableImage>
#include <MSceneManager>
#include <mtimestamp.h>
#include <mplainwindow.h>
#include <MApplication>


namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
    const int VerticalAnimatinDuration = 250;

    const QString ChineseLanguagePrefix("zh");
}

const QString MVirtualKeyboard::WordSeparators("-.,!? \n");


MVirtualKeyboard::MVirtualKeyboard(const LayoutsManager &layoutsManager,
                                   const MVirtualKeyboardStyleContainer *styleContainer,
                                   QGraphicsWidget *parent)
    : MWidget(parent),
      ReactionMapPaintable(),
      styleContainer(styleContainer),
      mainLayout(new QGraphicsLinearLayout(Qt::Vertical, this)),
      currentLevel(0),
      numLevels(2),
      sceneManager(MPlainWindow::instance()->sceneManager()),
      shiftState(ModifierClearState),
      currentLayoutType(LayoutData::General),
      currentOrientation(sceneManager->orientation()),
      layoutsMgr(layoutsManager),
      mainKeyboardSwitcher(0),
      notification(0),
      numberKeyboard(0),
      phoneNumberKeyboard(0),
      eventHandler(this),
      generalContentType(M::FreeTextContentType),
      toggleKeyState(false),
      composeKeyState(false),
      verticalAnimation(NULL),
      switchStarted(false)
{
    setFlag(QGraphicsItem::ItemHasNoContents);
    setObjectName("MVirtualKeyboard");
    hide();

    RegionTracker::instance().addRegion(*this);
    RegionTracker::instance().addInputMethodArea(*this);

    notification = new Notification(this);
    connect(notification, SIGNAL(destroyed(QObject *)),
            this, SLOT(resurrectNotification()));

    connect(&eventHandler, SIGNAL(keyPressed(const KeyEvent &)),
            this, SIGNAL(keyPressed(const KeyEvent &)));
    connect(&eventHandler, SIGNAL(keyReleased(const KeyEvent &)),
            this, SIGNAL(keyReleased(const KeyEvent &)));
    connect(&eventHandler, SIGNAL(keyClicked(const KeyEvent &)),
            this, SIGNAL(keyClicked(const KeyEvent &)));
    connect(&eventHandler, SIGNAL(longKeyPressed(const KeyEvent &)),
            this, SIGNAL(longKeyPressed(const KeyEvent &)));
    connect(&eventHandler, SIGNAL(shiftPressed(bool)),
            this, SLOT(handleShiftPressed(bool)));
    connect(&eventHandler, SIGNAL(keyCancelled(const KeyEvent &)),
            this, SIGNAL(keyCancelled(const KeyEvent &)));

    enableMultiTouch = MImSettings(MultitouchSettings).value().toBool();

    createSwitcher();

    // setting maximum width explicitly. otherwise max width of children will override. (bug?)
    setMaximumWidth(QWIDGETSIZE_MAX);
    setMinimumWidth(0);

    // add stuff to the layout

    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    Grip &keyboardGrip = *new Grip(this);
    keyboardGrip.setZValue(-1); // popup should be on top of keyboardGrip
    keyboardGrip.setObjectName("KeyboardHandle");
    mainLayout->addItem(&keyboardGrip);
    connectHandle(keyboardGrip);

    mainLayout->addItem(mainKeyboardSwitcher); // start in qwerty

    connect(&layoutsMgr, SIGNAL(layoutsChanged()), this, SLOT(keyboardsReset()));
    connect(&layoutsMgr, SIGNAL(numberFormatChanged()), this, SLOT(numberKeyboardReset()));
    keyboardsReset(); // creates keyboard widgets

    organizeContent(currentOrientation);

    // Request a reaction map painting if it appears
    connect(this, SIGNAL(displayEntered()), &signalForwarder, SIGNAL(requestRepaint()));

    // Initialize the vertical animation.
    verticalAnimation = new QPropertyAnimation(this, "pos");
    verticalAnimation->setDuration(VerticalAnimatinDuration);
    connect(verticalAnimation, SIGNAL(finished()), this, SLOT(onVerticalAnimationFinished()));
}


MVirtualKeyboard::~MVirtualKeyboard()
{
    delete mainKeyboardSwitcher;
    mainKeyboardSwitcher = 0;

    // we do not want to resurrect destroyed notification anymore,
    // therefore we disconnect all signals from notification explicitly
    disconnect(notification, 0, this, 0);
    delete notification;
    notification = 0;

    delete phoneNumberKeyboard;
    delete numberKeyboard;

    delete verticalAnimation;
    verticalAnimation = NULL;
}


template <class T>
void MVirtualKeyboard::connectHandle(const T &handle)
{
    connect(&handle, SIGNAL(flickLeft(const FlickGesture &)), this, SLOT(flickLeftHandler()));
    connect(&handle, SIGNAL(flickRight(const FlickGesture &)), this, SLOT(flickRightHandler()));
    connect(&handle, SIGNAL(flickDown(const FlickGesture &)), this, SIGNAL(userInitiatedHide()));
}


void
MVirtualKeyboard::prepareToOrientationChange()
{
}


void
MVirtualKeyboard::finalizeOrientationChange()
{
    M::Orientation newOrientation = sceneManager->orientation();
    // Force-mode required to update orientation even when we're not shown
    // and e.g. symbol view is active.
    // This makes sure keyboard is correctly layed out after rotation and
    // avoids any flicker when shown first time after orientation was changed.
    organizeContent(newOrientation, true);
    ReactionMapPainter::instance().repaint();
    // restore toggle, compose key state
    setToggleKeyState(toggleKeyState);
    setComposeKeyState(composeKeyState);
}

void
MVirtualKeyboard::switchLevel()
{
    switch (shiftState) {
    case ModifierClearState:
        currentLevel = 0;
        break;
    case ModifierLatchedState:
        currentLevel = 1;
        break;
    case ModifierLockedState:
        currentLevel = 1;
        break;
    }

    for (int i = 0; i < mainKeyboardSwitcher->count(); ++i) {
        // handling main section:
        MImAbstractKeyArea *mainKba = keyboardWidget(i);
        if (mainKba) {
            mainKba->switchLevel(currentLevel);
            mainKba->setShiftState(shiftState);
        }
    }
}

void
MVirtualKeyboard::setShiftState(ModifierState state)
{
    if (shiftState != state) {
        shiftState = state;

        switchLevel();
        emit shiftLevelChanged();
    }
}


void
MVirtualKeyboard::flickLeftHandler()
{
    if (!mainKeyboardSwitcher->isRunning()) {
        if (mainKeyboardSwitcher->isAtBoundary(HorizontalSwitcher::Right)) {
            emit pluginSwitchRequired(MInputMethod::SwitchForward);
            return;
        }

        mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Right);
        setLayout(mainKeyboardSwitcher->current());
    }
}


void
MVirtualKeyboard::flickRightHandler()
{
    if (!mainKeyboardSwitcher->isRunning()) {
        if (mainKeyboardSwitcher->isAtBoundary(HorizontalSwitcher::Left)) {
            emit pluginSwitchRequired(MInputMethod::SwitchBackward);
            return;
        }

        mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Left);
        setLayout(mainKeyboardSwitcher->current());
    }
}

bool MVirtualKeyboard::isPaintable() const
{
    return isVisible();
}

int MVirtualKeyboard::mainKeyboardCount() const
{
    return mainKeyboardSwitcher->count();
}

void MVirtualKeyboard::resetState()
{
    // Default state for shift is ShiftOff.
    setShiftState(ModifierClearState);

    setToggleKeyState(false);
    setComposeKeyState(false);

    // Dead keys are unlocked in MImAbstractKeyArea::onHide().
    // As long as this method is private, and only called from
    // hideKeyboard(), we don't have to explicitly unlock them.
}

void MVirtualKeyboard::showLanguageNotification()
{
    if ((mainKeyboardSwitcher->current() != -1) && (currentLayoutType == LayoutData::General)) {
        QGraphicsWidget *const widget = mainKeyboardSwitcher->currentWidget();
        const QRectF br = widget ? mainKeyboardSwitcher->mapRectToItem(this, widget->boundingRect())
                          : QRectF(QPointF(), MPlainWindow::instance()->visibleSceneSize());
        const QString layoutFile(layoutsMgr.layoutFileList()[mainKeyboardSwitcher->current()]);

        notification->displayText(layoutsMgr.keyboardTitle(layoutFile), br);
        notification->setParentItem(widget ? widget : this);
    }
}

void MVirtualKeyboard::enableSinglePageHorizontalFlick(bool enable)
{
    mainKeyboardSwitcher->enableSinglePageHorizontalFlick(enable);
}

void MVirtualKeyboard::organizeContent(M::Orientation orientation, const bool force)
{
    if (!isVisible() && !force) {
        return;
    }
    if (currentOrientation != orientation) {
        currentOrientation = orientation;
        // Reload for portrait/landscape
        int index = mainKeyboardSwitcher->current();
        // RecreateKeyboards() will lead to all slides being removed from switcher and a call to updateGeometry().
        // So we need to set the new preferredWidth first.
        setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());
        mainKeyboardSwitcher->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());
        recreateKeyboards();
        // Since all keyboards were reset, we reactive the current keyboard and adjust its size.
        mainKeyboardSwitcher->setCurrent(index);

        // After orientation change, our size doesn't get updated properly
        // this leads to only left part of the vkb to be displayed when rotating
        // from portrait to landscape. Adjusting size here explicitly, after setting
        // preferredWidth. If preferredWidth is not set, adjustSize() leads to
        // mainKeyboardSwitcher becoming as wide as landscape on first start in portrait.
        setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());
        adjustSize();
    }
}

void MVirtualKeyboard::drawButtonsReactionMaps(MReactionMap *reactionMap, QGraphicsView *view)
{
    // Depending on which keyboard type is currently shown
    // we must pick the correct MImAbstractKeyArea(s).
    QGraphicsLayoutItem *item = mainLayout->itemAt(KeyboardIndex);

    if (item) {
        const bool useWidgetFromSwitcher = (item == mainKeyboardSwitcher
                                            && mainKeyboardSwitcher->currentWidget());

        MImAbstractKeyArea *kba = static_cast<MImAbstractKeyArea *>(useWidgetFromSwitcher ? mainKeyboardSwitcher->currentWidget()
                                                                                : item);

        kba->drawReactiveAreas(reactionMap, view);
    }
}


QString MVirtualKeyboard::layoutLanguage() const
{
    return layoutsMgr.keyboardLanguage(currentLayout);
}

QString MVirtualKeyboard::layoutTitle() const
{
    return layoutsMgr.keyboardTitle(currentLayout);
}

QString MVirtualKeyboard::selectedLayout() const
{
    return currentLayout;
}

void MVirtualKeyboard::paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view)
{
#ifndef HAVE_REACTIONMAP
    Q_UNUSED(reactionMap);
    Q_UNUSED(view);
    return;
#else
    if (!isVisible()) {
        return;
    }

    // Draw keyboard area with inactive color to prevent transparent holes.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(this, view);
    reactionMap->fillRectangle(layout()->itemAt(KeyboardIndex)->geometry());
    reactionMap->fillRectangle(layout()->itemAt(KeyboardHandleIndex)->geometry());

    drawButtonsReactionMaps(reactionMap, view);
#endif // HAVE_REACTIONMAP
}


const MVirtualKeyboardStyleContainer &MVirtualKeyboard::style() const
{
    return *styleContainer;
}


void MVirtualKeyboard::setKeyboardType(const int type)
{
    // map m content type to layout model type
    LayoutData::LayoutType newLayoutType = LayoutData::General;

    switch (type) {
    case M::NumberContentType:
        newLayoutType = LayoutData::Number;
        break;

    case M::PhoneNumberContentType:
        newLayoutType = LayoutData::PhoneNumber;
        break;
    }

    if (newLayoutType != currentLayoutType) {
        currentLayoutType = newLayoutType;
        updateMainLayoutAtKeyboardIndex();
    }

    setContentType(static_cast<M::TextContentType>(type));

    // (1) If current content type is Email/Url and current active VKB is a Chinese VKB,
    //     ensure an English keyboard is available.
    //     ("English (UK)" keyboard would be inserted temporarily when necessary.)
    // (2) If current content type is not Email/Url, release the temporary "English (UK)" keyboard
    //     if it exists.
    if ((type == M::EmailContentType) || (type == M::UrlContentType)) {
        if (layoutLanguage().startsWith(ChineseLanguagePrefix, Qt::CaseSensitive))
            LayoutsManager::instance().ensureEnglishKeyboardAvailable();
    } else {
        LayoutsManager::instance().releaseTemporaryEnglishKeyboard();
    }
}

ModifierState MVirtualKeyboard::shiftStatus() const
{
    return shiftState;
}


const LayoutData *MVirtualKeyboard::currentLayoutModel() const
{
    return layoutsMgr.layout(currentLayout, currentLayoutType, currentOrientation);
}


void MVirtualKeyboard::setLayout(int layoutIndex)
{
    qDebug() << __PRETTY_FUNCTION__;
    if ((layoutIndex < 0) || (layoutIndex >= layoutsMgr.layoutCount())) {
        return;
    }

    const QStringList layoutList = layoutsMgr.layoutFileList();
    const QString nextLayout = layoutList.at(layoutIndex);

    qDebug() << "\t" << currentLayout << " -> " << nextLayout << " index=" << layoutIndex;

    if (nextLayout != currentLayout) {
        currentLayout = nextLayout;

        // Switcher has the layout loaded, just switchTo() it.
        // NOTE: Switcher already has correct index if layout change was
        // initiated by a flick gesture.
        if (mainKeyboardSwitcher->count() >= layoutIndex) {
            mainKeyboardSwitcher->setCurrent(layoutIndex);
        }
        emit layoutChanged(currentLayout);
    }
}


void MVirtualKeyboard::keyboardsReset()
{
    int layoutIndex = -1; // Layout to apply after reload.

    if (layoutsMgr.layoutCount() > 0) {
        const QStringList layoutList = layoutsMgr.layoutFileList();

        // If new layout set does not contain previous current layout
        // we need to set it to something else.
        layoutIndex = layoutList.indexOf(currentLayout);

        if (layoutIndex == -1) {
            // Current layout not in new layoutlist.
            // Set current layout to default layout
            layoutIndex = layoutList.indexOf(layoutsMgr.defaultLayoutFile());
            if (layoutIndex == -1) {
                // Last resort, take first
                layoutIndex = 0;
            }
        }

        // TODO: we should simplify the whole keyboardsReset(). Now
        // MVirtualKeyboard manages currentLayout and HorizontalSwitcher also
        // manages its current widget index.  We always have to synchronize
        // both of them if one is changed (in the right order, too).
        currentLayout.clear();
    }

    recreateKeyboards();

    if (layoutIndex >= 0) {
        setLayout(layoutIndex);
    }
}

void MVirtualKeyboard::numberKeyboardReset()
{
    recreateSpecialKeyboards(); // number and phone number keyboard
}

void MVirtualKeyboard::cancelEvent(MCancelEvent *event)
{
    QGraphicsWidget *keyArea(mainKeyboardSwitcher->currentWidget());
    if (keyArea) {
        scene()->sendEvent(keyArea, event);
    }
}

void MVirtualKeyboard::resizeEvent(QGraphicsSceneResizeEvent *)
{
    const QGraphicsWidget *const widget = mainKeyboardSwitcher->currentWidget();

    if (widget) {
        notification->resize(widget->size());
        notification->update();
    }
}

void MVirtualKeyboard::onSectionSwitchStarting(int current, int next)
{
    switchStarted = true;

    if (mainKeyboardSwitcher->currentWidget()) {
        // Current widget is animated off the screen but if mouse is not moved
        // relative to screen it appears to MImAbstractKeyArea as if mouse was held
        // still. We explicitly call ungrabMouse() to prevent long press event
        // in MImAbstractKeyArea. Visible effect of not doing this is would be
        // appearing popup, for example.
        mainKeyboardSwitcher->currentWidget()->ungrabMouse();
    }

    if ((current != -1) && (currentLayoutType == LayoutData::General)) {
        QGraphicsWidget *const nextWidget = mainKeyboardSwitcher->widget(next);
        QRectF br = nextWidget ? mainKeyboardSwitcher->mapRectToItem(this, nextWidget->boundingRect())
                               : QRectF(QPointF(), MPlainWindow::instance()->visibleSceneSize());

        notification->displayText(layoutsMgr.keyboardTitle(layoutsMgr.layoutFileList()[next]), br);
        notification->setParentItem(nextWidget ? nextWidget : this);
    }
}


void MVirtualKeyboard::resurrectNotification()
{
    notification = new Notification(this);
    connect(notification, SIGNAL(destroyed(QObject *)),
            this, SLOT(resurrectNotification()));
}


void MVirtualKeyboard::onSectionSwitched(QGraphicsWidget *previous, QGraphicsWidget *current)
{
    // Play the vertical animation if necessary.
    if (switchStarted) {
        int heightDelta = ((previous == NULL) || (current == NULL))
                          ? 0 : (previous->size().height() - current->size().height());
        playVerticalAnimation(heightDelta);
    }

    switchStarted = false;
    organizeContent(currentOrientation);
}


void MVirtualKeyboard::createSwitcher()
{
    delete mainKeyboardSwitcher; // Delete previous views
    mainKeyboardSwitcher = new HorizontalSwitcher(this);
    mainKeyboardSwitcher->setLooping(true);
    mainKeyboardSwitcher->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());
    mainKeyboardSwitcher->setDuration(style()->layoutChangeDuration());
    mainKeyboardSwitcher->setEasingCurve(style()->layoutChangeEasingCurve());

    // In addition to animating sections (MImAbstractKeyAreas), we also
    // use switcher to provide us the moving direction logic.
    // These are the signals we track to update our state.
    connect(mainKeyboardSwitcher, SIGNAL(switchStarting(int, int)),
            this, SLOT(onSectionSwitchStarting(int, int)));
    connect(mainKeyboardSwitcher, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)),
            this, SLOT(onSectionSwitched(QGraphicsWidget *, QGraphicsWidget *)));
    // Repaint the reaction maps if the keyboard is changed
    connect(mainKeyboardSwitcher, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)),
            &signalForwarder, SIGNAL(requestRepaint()));
}


void MVirtualKeyboard::reloadSwitcherContent()
{
    // delete previous pages
    mainKeyboardSwitcher->deleteAll();

    // Load certain type and orientation from all layouts.
    foreach (const QString &layoutFile, layoutsMgr.layoutFileList()) {
        MImAbstractKeyArea *mainSection = createMainSectionView(layoutFile, LayoutData::General,
                                                                currentOrientation);
        mainSection->setObjectName("VirtualKeyboardMainRow");
        mainSection->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());
        mainKeyboardSwitcher->addWidget(mainSection);
    }
}


MImAbstractKeyArea *MVirtualKeyboard::createMainSectionView(const QString &layout,
                                                            LayoutData::LayoutType layoutType,
                                                            M::Orientation orientation,
                                                            QGraphicsWidget *parent)
{
    MImAbstractKeyArea *keyArea = createSectionView(layout, layoutType, orientation,
                                                    LayoutData::mainSection,
                                                    true, parent);

    // horizontal flick handling only on main section of qwerty
    connect(keyArea, SIGNAL(flickLeft()), this, SLOT(flickLeftHandler()));
    connect(keyArea, SIGNAL(flickRight()), this, SLOT(flickRightHandler()));
    connect(this, SIGNAL(displayExited()), keyArea, SLOT(hidePopup()));

    return keyArea;
}

MImAbstractKeyArea * MVirtualKeyboard::createSectionView(const QString &layout,
                                                         LayoutData::LayoutType layoutType,
                                                         M::Orientation orientation,
                                                         const QString &section,
                                                         bool usePopup,
                                                         QGraphicsWidget *parent)
{
    const LayoutData *model = layoutsMgr.layout(layout, layoutType, orientation);
    MImAbstractKeyArea *view = MImKeyArea::create(model->section(section), usePopup, parent);

    eventHandler.addEventSource(view);
    connect(view, SIGNAL(flickDown()), this, SIGNAL(userInitiatedHide()));

    return view;
}

void MVirtualKeyboard::handleShiftPressed(bool shiftPressed)
{
    if (enableMultiTouch) {
        // When shift pressed, always use the second level. If not pressed, use whatever the current level is.
        const int level = shiftPressed ? 1 : currentLevel;

        MImAbstractKeyArea *mainKb = keyboardWidget();
        if (mainKb) {
            mainKb->switchLevel(level);
        }
    }
}

MImAbstractKeyArea *MVirtualKeyboard::keyboardWidget(int layoutIndex) const
{
    if (!mainKeyboardSwitcher) {
        return 0;
    }

    return static_cast<MImAbstractKeyArea *>((layoutIndex == -1)
                                        ? mainKeyboardSwitcher->currentWidget()
                                        : mainKeyboardSwitcher->widget(layoutIndex));
}

void MVirtualKeyboard::recreateKeyboards()
{
    reloadSwitcherContent(); // main keyboards
    recreateSpecialKeyboards(); // number and phone number keyboard
    switchLevel(); // update shift level in recreated keyboards
    mainKeyboardSwitcher->setKeyOverrides(overrides);
    numberKeyboard->setKeyOverrides(overrides);
    phoneNumberKeyboard->setKeyOverrides(overrides);
    mainKeyboardSwitcher->setContentType(generalContentType);
}


void MVirtualKeyboard::recreateSpecialKeyboards()
{
    // If mainLayout contains any of those, mainLayout will be updated by the
    // QGraphicsLayoutItem dtor, and the layout items will be automatically
    // removed from the mainLayout:
    delete numberKeyboard;
    numberKeyboard = 0;
    delete phoneNumberKeyboard;
    phoneNumberKeyboard = 0;

    // number:
    const QString defaultLayout = layoutsMgr.defaultLayoutFile();

    numberKeyboard = createSectionView(defaultLayout, LayoutData::Number,
                                       currentOrientation, LayoutData::mainSection,
                                       false, numberKeyboard);

    // phone:
    phoneNumberKeyboard = createSectionView(defaultLayout, LayoutData::PhoneNumber,
                                            currentOrientation, LayoutData::mainSection,
                                            false, phoneNumberKeyboard);


    // sanity check. Don't allow load failure for these
    if (!numberKeyboard || !phoneNumberKeyboard) {
        qFatal("Error loading number keyboard");
    }

    numberKeyboard->setObjectName("VirtualKeyboardNumberMainRow");
    phoneNumberKeyboard->setObjectName("VirtualKeyboardPhoneMainRow");

    updateMainLayoutAtKeyboardIndex();
}

bool MVirtualKeyboard::symViewAvailable() const
{
    bool available = true;
    switch (currentLayoutType) {
    case LayoutData::Number:
    case LayoutData::PhoneNumber:
        available = false;
        break;
    default:
        break;
    }
    return available;
}

void MVirtualKeyboard::switchLayout(MInputMethod::SwitchDirection direction, bool enableAnimation)
{
    qDebug() << __PRETTY_FUNCTION__ << direction << enableAnimation;
    if (direction == MInputMethod::SwitchUndefined) {
        return;
    }

    if (enableAnimation) {
        if (direction == MInputMethod::SwitchForward) {
            mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Right);
        } else {
            mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Left);
        }
    } else {
        int current = mainKeyboardSwitcher->current();
        if (direction == MInputMethod::SwitchForward) {
            current = (current + 1) % mainKeyboardSwitcher->count();
        } else {
            --current;
            if (current < 0) {
                current = mainKeyboardSwitcher->count() - 1;
            }
        }

        mainKeyboardSwitcher->setCurrent(current);
    }
    setLayout(mainKeyboardSwitcher->current());
}


void MVirtualKeyboard::setInputMethodMode(M::InputMethodMode mode)
{
    MImAbstractKeyArea::setInputMethodMode(mode);
}

bool MVirtualKeyboard::autoCapsEnabled() const
{
    return layoutsMgr.autoCapsEnabled(currentLayout);
}

void MVirtualKeyboard::updateMainLayoutAtKeyboardIndex()
{
    // remove what currently is in the keyboard position in the main layout
    QGraphicsWidget *previousWidget = dynamic_cast<QGraphicsWidget *>(mainLayout->itemAt(KeyboardIndex));

    if (previousWidget) {
        if ((previousWidget == mainKeyboardSwitcher)
            || (previousWidget == numberKeyboard)
            || (previousWidget == phoneNumberKeyboard)) {
            mainLayout->removeItem(previousWidget);
            previousWidget->hide();
        } else {
            qWarning() << __PRETTY_FUNCTION__
                       << "Unexpected widget found in main layout.";
        }
    }

    // show appropriate keyboard widget
    QGraphicsWidget *newWidget = 0;

    switch (currentLayoutType) {
    case LayoutData::Number:
        newWidget = numberKeyboard;
        break;

    case LayoutData::PhoneNumber:
        newWidget = phoneNumberKeyboard;
        break;
    default:
        newWidget = mainKeyboardSwitcher;
        break;
    }

    mainLayout->insertItem(KeyboardIndex, newWidget);
    newWidget->setVisible(true);

    // resize and update keyboards if needed
    organizeContent(currentOrientation);

    // Request a reaction map repainting when the first keyboard is loaded or
    // the keyboard is changed between normal, phone and number layouts.
    if (previousWidget != newWidget) {
        signalForwarder.emitRequestRepaint();
    }

}

QList<MImEngine::KeyboardLayoutKey> MVirtualKeyboard::mainLayoutKeys() const
{
    mainLayout->activate();

    QList<MImEngine::KeyboardLayoutKey> keys;
    MImAbstractKeyArea *mainKb = keyboardWidget();
    if (!mainKb) {
        return keys;
    }

    foreach (const MImAbstractKey *ikey, mainKb->keys()) {
        // only care about the keys which insert characters.
        if (ikey->model().binding()->action() == MImKeyBinding::ActionInsert) {
            bool isPunctuation = false;
            bool isSymbol = false;
            QList<QChar> symbols;
            foreach (const QChar &c, ikey->model().binding()->label()) {
                symbols << c;
                if (c.isPunct())
                    isPunctuation = true;
                if (c.isSymbol())
                    isSymbol = true;
            }
            // Ignore symbols
            if (isSymbol)
                continue;

            foreach (const QChar &c, ikey->model().binding()->accentedLabels()) {
                symbols << c;
            }

            MImEngine::KeyboardLayoutKey key;
            if (isPunctuation) {
                key.type = MImEngine::KeyboardLayoutKeyNonRegional;
            } else {
                key.type = MImEngine::KeyboardLayoutKeyRegional;
            }
            key.rect = ikey->buttonRect().toRect();
            key.symbols = symbols;
            keys << key;
        }
    }
    return keys;
}

void MVirtualKeyboard::setKeyOverrides(const QMap<QString, QSharedPointer<MKeyOverride> > &overrides)
{
    mainKeyboardSwitcher->setKeyOverrides(overrides);
    numberKeyboard->setKeyOverrides(overrides);
    phoneNumberKeyboard->setKeyOverrides(overrides);
    this->overrides = overrides;
}

void MVirtualKeyboard::setContentType(M::TextContentType type)
{
    if( currentLayoutType != LayoutData::General
        || generalContentType == type )
    {
        return;
    }
    mainKeyboardSwitcher->setContentType(type);
    generalContentType = type;
}

void MVirtualKeyboard::setToggleKeyState(bool onOff)
{
    toggleKeyState = onOff;
    // toggle key only stays on main keybard.
    MImAbstractKeyArea *mainKb = keyboardWidget();
    if (mainKb) {
        mainKb->setToggleKeyState(onOff);
    }
}

void MVirtualKeyboard::setComposeKeyState(bool isComposing)
{
    composeKeyState = isComposing;
    // compose key only stays on main keybard.
    MImAbstractKeyArea *mainKb = keyboardWidget();
    if (mainKb) {
        mainKb->setComposeKeyState(isComposing);
    }
}

void MVirtualKeyboard::playVerticalAnimation(int animLine)
{
    if (animLine == 0)
        return;

    // Calculate the backgroud area for the vertical animation,
    // and this area would be painted in BLACK to hide views which just underlie VKB.
    animHiddingArea.setTopLeft(QPointF(0.0, 0.0));
    animHiddingArea.setSize(QSize(size().width(), size().height() + animLine));

    // Disable VKB temporarily during playing the animation.
    setEnabled(false);
    // Enable painting background in black during playing the animation.
    setFlag(QGraphicsItem::ItemHasNoContents, false);

    // Start the vertical animation.
    QPointF startPos(pos());
    QPointF endPos(pos().x(), pos().y() + animLine);
    verticalAnimation->setStartValue(startPos);
    verticalAnimation->setEndValue(endPos);
    verticalAnimation->start();
}

bool MVirtualKeyboard::isPlayingAnimation()
{
    return (verticalAnimation->state() == QPropertyAnimation::Running);
}

void MVirtualKeyboard::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    // Make VKB background black during playing the vertical animation.
    painter->fillRect(animHiddingArea, Qt::black);
}

void MVirtualKeyboard::onVerticalAnimationFinished()
{
    // Recover original painting flag.
    setFlag(QGraphicsItem::ItemHasNoContents);
    // Enable VKB.
    setEnabled(true);
    // Repaint the reaction map.
    signalForwarder.emitRequestRepaint();
}
