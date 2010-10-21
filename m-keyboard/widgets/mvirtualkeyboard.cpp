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



#include "mvirtualkeyboard.h"
#include "mvirtualkeyboardstyle.h"
#include "horizontalswitcher.h"
#include "keyboarddata.h"
#include "layoutdata.h"
#include "layoutsmanager.h"
#include "notification.h"
#include "vkbdatakey.h"
#include "mimtoolbar.h"
#include "ikeybutton.h"
#include "keyevent.h"
#include "grip.h"
#include "sharedhandlearea.h"

#include <mtoolbardata.h>

#include <QDebug>
#include <QPainter>
#include <QGraphicsLinearLayout>
#include <QStyleOptionGraphicsItem>
#include <QGraphicsWidget>

#include <MButton>
#include <MScalableImage>
#include <MSceneManager>
#include <mreactionmap.h>
#include <mtimestamp.h>
#include <mplainwindow.h>
#include <MApplication>


namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";
}

const QString MVirtualKeyboard::WordSeparators("-.,!? \n");


MVirtualKeyboard::MVirtualKeyboard(const LayoutsManager &layoutsManager,
                                   const MVirtualKeyboardStyleContainer *styleContainer,
                                   QGraphicsWidget *parent)
    : MWidget(parent),
      styleContainer(styleContainer),
      mainLayout(new QGraphicsLinearLayout(Qt::Vertical, this)),
      currentLevel(0),
      numLevels(2),
      activity(Inactive),
      sceneManager(MPlainWindow::instance()->sceneManager()),
      shiftState(ModifierClearState),
      currentLayoutType(LayoutData::General),
      currentOrientation(sceneManager->orientation()),
      hideShowByFadingOnly(false),
      sendRegionUpdates(true),
      regionUpdateRequested(false),
      layoutsMgr(layoutsManager),
      mainKeyboardSwitcher(0),
      notification(0),
      numberKeyboard(0),
      phoneNumberKeyboard(0),
      activeState(MInputMethod::OnScreen),
      eventHandler(this)
{
    setFlag(QGraphicsItem::ItemHasNoContents);
    setObjectName("MVirtualKeyboard");
    hide();

    notification = new Notification(styleContainer, this);

    connect(&eventHandler, SIGNAL(keyPressed(const KeyEvent &)),
            this, SIGNAL(keyPressed(const KeyEvent &)));
    connect(&eventHandler, SIGNAL(keyReleased(const KeyEvent &)),
            this, SIGNAL(keyReleased(const KeyEvent &)));
    connect(&eventHandler, SIGNAL(keyClicked(const KeyEvent &)),
            this, SIGNAL(keyClicked(const KeyEvent &)));
    connect(&eventHandler, SIGNAL(shiftPressed(bool)),
            this, SLOT(handleShiftPressed(bool)));

    enableMultiTouch = MGConfItem(MultitouchSettings).value().toBool();

    createSwitcher();

    setupTimeLine();

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
}


MVirtualKeyboard::~MVirtualKeyboard()
{
    delete mainKeyboardSwitcher;
    mainKeyboardSwitcher = 0;

    delete notification;
    notification = 0;

    delete phoneNumberKeyboard;
    delete numberKeyboard;
}


template <class T>
void MVirtualKeyboard::connectHandle(const T &handle)
{
    connect(&handle, SIGNAL(flickLeft(const FlickGesture &)), this, SLOT(flickLeftHandler()));
    connect(&handle, SIGNAL(flickRight(const FlickGesture &)), this, SLOT(flickRightHandler()));
    connect(&handle, SIGNAL(flickDown(const FlickGesture &)), this, SLOT(handleHandleFlickDown(const FlickGesture &)));
}


void MVirtualKeyboard::handleHandleFlickDown(const FlickGesture &/* gesture */)
{
    if (activeState == MInputMethod::OnScreen) {
        hideKeyboard();
        emit userInitiatedHide();
    }
}


void
MVirtualKeyboard::prepareToOrientationChange()
{
    // if inactive, just ignore
    if (activity != Active) {
        return;
    }
    hideKeyboard(true, true);
}


void
MVirtualKeyboard::finalizeOrientationChange()
{
    // if inactive, just ignore
    if (activity != TemporarilyInactive) {
        return;
    }

    showKeyboard(true);
}

bool MVirtualKeyboard::isFullyVisible() const
{
    return ((activity == Active)
            && isVisible()
            && (showHideTimeline.state() != QTimeLine::Running));
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
        KeyButtonArea *mainKba = keyboardWidget(i);
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
MVirtualKeyboard::setupTimeLine()
{
    showHideTimeline.setCurveShape(QTimeLine::EaseInCurve);
    showHideTimeline.setFrameRange(0, ShowHideFrames);
    showHideTimeline.setDuration(ShowHideTime);
    showHideTimeline.setUpdateInterval(ShowHideInterval);
    connect(&showHideTimeline, SIGNAL(frameChanged(int)), this, SLOT(fade(int)));
    connect(&showHideTimeline, SIGNAL(finished()), this, SLOT(showHideFinished()));
}


void
MVirtualKeyboard::flickLeftHandler()
{
    if ((activeState == MInputMethod::OnScreen) && !mainKeyboardSwitcher->isRunning()) {
        if (mainKeyboardSwitcher->isAtBoundary(HorizontalSwitcher::Right)) {
            emit pluginSwitchRequired(MInputMethod::SwitchForward);
            return;
        }

        mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Right);
        setLayout(mainKeyboardSwitcher->current());
    }
}


void
MVirtualKeyboard::flickUpHandler(const KeyBinding &binding)
{
    if (binding.action() == KeyBinding::ActionSym) {
        emit showSymbolViewRequested();
    }
}


void
MVirtualKeyboard::flickRightHandler()
{
    if ((activeState == MInputMethod::OnScreen) && !mainKeyboardSwitcher->isRunning()) {
        if (mainKeyboardSwitcher->isAtBoundary(HorizontalSwitcher::Left)) {
            emit pluginSwitchRequired(MInputMethod::SwitchBackward);
            return;
        }

        mainKeyboardSwitcher->switchTo(HorizontalSwitcher::Left);
        setLayout(mainKeyboardSwitcher->current());
    }
}


void MVirtualKeyboard::showKeyboard(bool fadeOnly)
{
    qDebug() << __PRETTY_FUNCTION__ << fadeOnly;
    organizeContent(sceneManager->orientation());

    if (activity == Active) {
        // already showing up
        return;
    }

    activity = Active;

    showHideTimeline.setDirection(QTimeLine::Forward);

    if (showHideTimeline.state() != QTimeLine::Running) {
        hideShowByFadingOnly = fadeOnly;

        QPoint regionOffset(0, 0);
        if (!fadeOnly) {
            // sharedHandleArea is positioned right below the visible area
            // vkb is positioned below sharedHandleArea
            int shift = 0;

            if (sharedHandleArea) {
                shift = sharedHandleArea->size().height();
            }
            setPos(0, sceneManager->visibleSceneSize().height() + shift);
            regionOffset.setY(-actualHeight() - shift);
        } else {
            setPos(0, sceneManager->visibleSceneSize().height() - actualHeight());
        }

        suppressRegionUpdate(true); // Don't send separate region update for imtoolbar.
        showHideTimeline.start();
        show();

        // We need to send one initial region update to have passthruwindow make the
        // window visible.  We also want the scene manager to move the underlying window
        // so that the focused widget will be visible after the show animation and we want
        // it to do that right away, so the region we send now is (usually) the final
        // region (i.e. what it will be after animation is finished).  Region will be sent
        // additionally after animation is finished just in case copy button appears or
        // something else changes during the animation.  But normally the region should be
        // the same as the one we send now.  We do this initial region sending after
        // show() so that we can use region().  We cannot use sendVKBRegion() since region
        // updates are suppressed and because we want to apply the offset.
        regionOffset = mapOffsetToScene(regionOffset);
        emit regionUpdated(region(true).translated(regionOffset));
        emit inputMethodAreaUpdated(region(true).translated(regionOffset));
    } else if (hideShowByFadingOnly) {
        // fade() doesn't alter the position when we're just fading
        setPos(0, sceneManager->visibleSceneSize().height() - actualHeight());
        show();
    }
}


void MVirtualKeyboard::hideKeyboard(bool fadeOnly, bool temporary)
{
    qDebug() << __PRETTY_FUNCTION__ << "HIDE";

    if (activity == Active) {
        showHideTimeline.setDirection(QTimeLine::Backward);

        if (showHideTimeline.state() != QTimeLine::Running) {
            hideShowByFadingOnly = fadeOnly;
            suppressRegionUpdate(true);
            showHideTimeline.start();
        }

        activity = temporary ? TemporarilyInactive : Inactive;
    }
}

void MVirtualKeyboard::resetState()
{
    // Default state for shift is ShiftOff.
    setShiftState(ModifierClearState);

    // Dead keys are unlocked in KeyButtonArea::onHide().
    // As long as this method is private, and only called from
    // hideKeyboard(), we don't have to explicitly unlock them.
}

int MVirtualKeyboard::actualHeight() const
{
    int result = size().height();

    if ((activeState != MInputMethod::OnScreen)) {
        result = 0;
    }

    return result;
}

void MVirtualKeyboard::organizeContent(M::Orientation orientation)
{
    if (currentOrientation != orientation) {
        currentOrientation = orientation;

        // Reload for portrait/landscape
        int index = mainKeyboardSwitcher->current();
        recreateKeyboards();
        mainKeyboardSwitcher->setCurrent(index);
        mainKeyboardSwitcher->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());
    }

    mainLayout->invalidate();
    resize(MPlainWindow::instance()->visibleSceneSize().width(), mainLayout->preferredHeight());
    if ((activity == Active) && (showHideTimeline.state() != QTimeLine::Running)) {
        setPos(0, sceneManager->visibleSceneSize().height() - actualHeight());
    }
}


void
MVirtualKeyboard::fade(int frame)
{
    const float opacity = float(frame) / ShowHideFrames;
    const QSize sceneSize = sceneManager->visibleSceneSize();

    if (!hideShowByFadingOnly) {
        int shift = 0;

        if (sharedHandleArea) {
            shift = sharedHandleArea->size().height();
        }
        //vkb should be under sharedHandleArea
        setPos(0, sceneSize.height() + shift - (opacity * (actualHeight() + shift)));
    }

    this->setOpacity(opacity);
    if (sharedHandleArea) {
        // fade sharedHandleArea together
        sharedHandleArea->setOpacity(opacity);
    }
    update();
}


void
MVirtualKeyboard::showHideFinished()
{
    const bool hiding = (showHideTimeline.direction() == QTimeLine::Backward);

    if (hiding) {
        // Assuming here we don't want to reset vkb state if temporarily hiding.
        // For example, don't reset state when hiding for screen rotation.
        if (activity == Inactive) {
            resetState();
        }

        hide();
        emit hidden();
    } else {
        emit opened();
    }

    sendVKBRegion();
    suppressRegionUpdate(false);
}


void MVirtualKeyboard::organizeContentAndSendRegion()
{
    if (isVisible()) {
        organizeContent(currentOrientation);
        sendVKBRegion();
    }
}


void MVirtualKeyboard::sendVKBRegion(const QRegion &extraRegion)
{
    regionUpdateRequested = true;

    if (!sendRegionUpdates)
        return;

    emit regionUpdated(region(true) |= extraRegion);
    emit inputMethodAreaUpdated(region(true));
}

QRegion MVirtualKeyboard::region(const bool notJustMainKeyboardArea) const
{
    QRegion region;
    QRectF rect;

    if (isVisible()) {
        mainLayout->activate();

        // Main keyboard area (qwerty/number/etc.)
        if (activeState == MInputMethod::OnScreen) {
            mainLayout->activate();
            rect = mainLayout->itemAt(KeyboardIndex)->geometry();
            region |= mapRectToScene(rect).toRect();
            if (notJustMainKeyboardArea) {
                rect = mainLayout->itemAt(KeyboardHandleIndex)->geometry();
                region |= mapRectToScene(rect).toRect();
            }
        } else {
            // set offscreen rectangle to provide proper positioning for toolbar
            rect = QRectF(0, 0, size().width(), 1);
            region |= mapRectToScene(rect).toRect();
        }
    }

    return region;
}

QPoint MVirtualKeyboard::mapOffsetToScene(QPointF offset)
{
    QPointF startingPoint(mapToScene(QPointF(0, 0)));

    offset = mapToScene(offset);

    return QPoint(offset.x() - startingPoint.x(),
                  offset.y() - startingPoint.y());
}


QRect MVirtualKeyboard::mainAreaSceneRect() const
{
    QRect result;

    if (activeState == MInputMethod::OnScreen) {
        result = region(false).boundingRect();
    }

    return result;
}

void MVirtualKeyboard::setKeyboardState(MInputMethod::HandlerState newState)
{
    if (activeState == newState) {
        return;
    }

    bool savedSendRegionUpdates = sendRegionUpdates;
    sendRegionUpdates = true; // our region is changed, so we must sent it now

    activeState = newState;
    resetState();

    static_cast<QGraphicsWidget *>(mainLayout->itemAt(KeyboardIndex))->setVisible(newState == MInputMethod::OnScreen);
    showHideTimeline.stop(); // position must be updated by organizeContentAndSendRegion()
    organizeContentAndSendRegion();
    sendRegionUpdates = savedSendRegionUpdates;
}

MInputMethod::HandlerState MVirtualKeyboard::keyboardState() const
{
    return activeState;
}

void MVirtualKeyboard::drawButtonsReactionMaps(MReactionMap *reactionMap, QGraphicsView *view)
{
    // Depending on which keyboard type is currently shown
    // we must pick the correct KeyButtonArea(s).
    QGraphicsLayoutItem *item = mainLayout->itemAt(KeyboardIndex);

    if (item) {
        const bool useWidgetFromSwitcher = (item == mainKeyboardSwitcher
                                            && mainKeyboardSwitcher->currentWidget());

        KeyButtonArea *kba = static_cast<KeyButtonArea *>(useWidgetFromSwitcher ? mainKeyboardSwitcher->currentWidget()
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
    if ((activity != Active)) {
        return;
    }

    // Draw keyboard area with inactive color to prevent transparent holes.
    reactionMap->setInactiveDrawingValue();
    reactionMap->setTransform(this, view);
    reactionMap->fillRectangle(layout()->itemAt(KeyboardIndex)->geometry());
    reactionMap->fillRectangle(layout()->itemAt(KeyboardHandleIndex)->geometry());

    if (activeState == MInputMethod::OnScreen) {
        drawButtonsReactionMaps(reactionMap, view);
    }
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

    if (newLayoutType == currentLayoutType) {
        return;
    }

    currentLayoutType = newLayoutType;
    updateMainLayoutAtKeyboardIndex();
}

void MVirtualKeyboard::suppressRegionUpdate(bool suppress)
{
    // Call sendVKBRegion if suppress is now released and was previously
    // set on, and at least one region update has been requested.
    if (!suppress && !sendRegionUpdates && regionUpdateRequested) {

        // Allow sendVKBRegion to send region
        sendRegionUpdates = true;

        // and send the region.
        sendVKBRegion();
    }

    sendRegionUpdates = !suppress;
    regionUpdateRequested = false;
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

        emit layoutChanged(currentLayout);

        // Switcher has the layout loaded, just switchTo() it.
        // NOTE: Switcher already has correct index if layout change was
        // initiated by a flick gesture.
        if (mainKeyboardSwitcher->count() >= layoutIndex) {
            mainKeyboardSwitcher->setCurrent(layoutIndex);
        }
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

void MVirtualKeyboard::onSectionSwitchStarting(int current, int next)
{
    if (mainKeyboardSwitcher->currentWidget()) {
        // Current widget is animated off the screen but if mouse is not moved
        // relative to screen it appears to KeyButtonArea as if mouse was held
        // still. We explicitly call ungrabMouse() to prevent long press event
        // in KeyButtonArea. Visible effect of not doing this is would be
        // appearing popup, for example.
        mainKeyboardSwitcher->currentWidget()->ungrabMouse();
    }

    if ((current != -1) && (currentLayoutType == LayoutData::General)) {
        const QGraphicsWidget *const nextWidget = mainKeyboardSwitcher->widget(next);
        QRectF br = nextWidget ? mainKeyboardSwitcher->mapRectToItem(this, nextWidget->boundingRect())
                               : QRectF(QPointF(), MPlainWindow::instance()->visibleSceneSize());

        notification->displayText(layoutsMgr.keyboardTitle(layoutsMgr.layoutFileList()[next]), br);
    }
}


void MVirtualKeyboard::onSectionSwitched(QGraphicsWidget *previous, QGraphicsWidget *current)
{
    Q_UNUSED(previous);
    Q_UNUSED(current);
    // All sections may not be of the same height so we send a region just in
    // case.  That also causes reaction maps to be redrawn.
    organizeContentAndSendRegion();
}


void MVirtualKeyboard::createSwitcher()
{
    delete mainKeyboardSwitcher; // Delete previous views
    mainKeyboardSwitcher = new HorizontalSwitcher(this);
    mainKeyboardSwitcher->setLooping(true);
    mainKeyboardSwitcher->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());

    // In addition to animating sections (KeyButtonAreas), we also
    // use switcher to provide us the moving direction logic.
    // These are the signals we track to update our state.
    connect(mainKeyboardSwitcher, SIGNAL(switchStarting(int, int)),
            this, SLOT(onSectionSwitchStarting(int, int)));
    connect(mainKeyboardSwitcher, SIGNAL(switchDone(QGraphicsWidget *, QGraphicsWidget *)),
            this, SLOT(onSectionSwitched(QGraphicsWidget *, QGraphicsWidget *)));
}


void MVirtualKeyboard::reloadSwitcherContent()
{
    // delete previous pages
    mainKeyboardSwitcher->deleteAll();

    // Load certain type and orientation from all layouts.
    foreach (const QString &layoutFile, layoutsMgr.layoutFileList()) {
        KeyButtonArea *mainSection = createMainSectionView(layoutFile, LayoutData::General,
                                                           currentOrientation);
        mainSection->setObjectName("VirtualKeyboardMainRow");
        mainSection->setPreferredWidth(MPlainWindow::instance()->visibleSceneSize().width());
        mainKeyboardSwitcher->addWidget(mainSection);
    }
}


KeyButtonArea *MVirtualKeyboard::createMainSectionView(const QString &layout,
                                                       LayoutData::LayoutType layoutType,
                                                       M::Orientation orientation,
                                                       QGraphicsWidget *parent)
{
    KeyButtonArea *buttonArea = createSectionView(layout, layoutType, orientation,
                                                  LayoutData::mainSection,
                                                  true, parent);

    // horizontal flick handling only on main section of qwerty
    connect(buttonArea, SIGNAL(flickLeft()), this, SLOT(flickLeftHandler()));
    connect(buttonArea, SIGNAL(flickRight()), this, SLOT(flickRightHandler()));

    return buttonArea;
}

KeyButtonArea * MVirtualKeyboard::createSectionView(const QString &layout,
                                                    LayoutData::LayoutType layoutType,
                                                    M::Orientation orientation,
                                                    const QString &section,
                                                    bool usePopup,
                                                    QGraphicsWidget *parent)
{
    const LayoutData *model = layoutsMgr.layout(layout, layoutType, orientation);
    KeyButtonArea *view = new SingleWidgetButtonArea(model->section(section),
                                                     usePopup, parent);

    eventHandler.addEventSource(view);

    connect(view, SIGNAL(flickDown()), this, SIGNAL(userInitiatedHide()));
    connect(view, SIGNAL(flickUp(KeyBinding)),
            this, SLOT(flickUpHandler(KeyBinding)));
    connect(view, SIGNAL(regionUpdated(QRegion)),
            this, SLOT(sendVKBRegion(QRegion)));

    return view;
}

void MVirtualKeyboard::handleShiftPressed(bool shiftPressed)
{
    if (enableMultiTouch) {
        // When shift pressed, always use the second level. If not pressed, use whatever the current level is.
        const int level = shiftPressed ? 1 : currentLevel;

        KeyButtonArea *mainKb = keyboardWidget();
        if (mainKb) {
            mainKb->switchLevel(level);
        }
    }
}

KeyButtonArea *MVirtualKeyboard::keyboardWidget(int layoutIndex) const
{
    if (!mainKeyboardSwitcher) {
        return 0;
    }

    return static_cast<KeyButtonArea *>((layoutIndex == -1)
                                        ? mainKeyboardSwitcher->currentWidget()
                                        : mainKeyboardSwitcher->widget(layoutIndex));
}

void MVirtualKeyboard::recreateKeyboards()
{
    reloadSwitcherContent(); // main keyboards
    recreateSpecialKeyboards(); // number and phone number keyboard
    switchLevel(); // update shift level in recreated keyboards
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

void MVirtualKeyboard::setSharedHandleArea(const QPointer<SharedHandleArea> &newSharedHandleArea)
{
    sharedHandleArea = newSharedHandleArea;
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


void MVirtualKeyboard::hideMainArea()
{
    QGraphicsItem *item = dynamic_cast<QGraphicsItem*>(mainLayout->itemAt(KeyboardIndex));
    if (item) {
        item->hide();
    }
}


void MVirtualKeyboard::showMainArea()
{
    QGraphicsItem *item = dynamic_cast<QGraphicsItem*>(mainLayout->itemAt(KeyboardIndex));
    if (item) {
        item->show();
    }
}

void MVirtualKeyboard::setInputMethodMode(M::InputMethodMode mode)
{
    KeyButtonArea::setInputMethodMode(mode);
}

bool MVirtualKeyboard::autoCapsEnabled() const
{
    return layoutsMgr.autoCapsEnabled(currentLayout);
}

void MVirtualKeyboard::setTemporarilyHidden(bool hidden)
{
    if (hidden && activity == Active) {
        hideKeyboard(true, true);
    } else if (!hidden && activity == TemporarilyInactive) {
        showKeyboard(true);
    }
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
    newWidget->setVisible(activeState == MInputMethod::OnScreen);

    // resize and update keyboards if needed
    organizeContentAndSendRegion();
}

QList<M::KeyboardLayoutKey> MVirtualKeyboard::mainLayoutKeys() const
{
    QList<M::KeyboardLayoutKey> keys;
    KeyButtonArea *mainKb = keyboardWidget();
    foreach (const IKeyButton *ikey, mainKb->keys()) {
        // only care about the keys which insert characters.
        if (ikey->key().binding()->action() == KeyBinding::ActionInsert) {
            bool isPunctuation = false;
            bool isSymbol = false;
            QList<QChar> symbols;
            foreach (const QChar &c, ikey->key().binding()->label()) {
                symbols << c;
                if (c.isPunct())
                    isPunctuation = true;
                if (c.isSymbol())
                    isSymbol = true;
            }
            // Ignore symbols
            if (isSymbol)
                continue;

            foreach (const QChar &c, ikey->key().binding()->accentedLabels()) {
                symbols << c;
            }

            M::KeyboardLayoutKey key;
            if (isPunctuation) {
                key.type = M::KeyboardLayoutKey_NonRegional;
            } else {
                key.type = M::KeyboardLayoutKey_Regional;
            }
            key.rect = ikey->buttonRect().toRect();
            key.symbols = symbols;
            keys << key;
        }
    }
    return keys;
}
