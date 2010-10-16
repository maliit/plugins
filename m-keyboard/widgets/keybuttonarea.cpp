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



#include "flickgesture.h"
#include "flickgesturerecognizer.h"
#include "mvirtualkeyboardstyle.h"
#include "keybuttonarea.h"
#include "popupbase.h"
#include "popupfactory.h"

#include <MApplication>
#include <MComponentData>
#include <MFeedbackPlayer>
#include <MSceneManager>
#include <MGConfItem>
#include <QDebug>
#include <QEvent>
#include <QGraphicsLinearLayout>
#include <QGraphicsSceneMouseEvent>
#include <QHash>
#include <QKeyEvent>
#include <mtimestamp.h>

namespace
{
    const int GestureTimeOut = 1000;
    const qreal ZValueButtons = 0.0;

    // Minimal distinguishable cursor/finger movement
    const qreal MovementThreshold = 5.0;

    // For gesture thresholds: How many pixels translate to one counted move event.
    const qreal PixelsToMoveEventsFactor = 0.02;

    // This GConf item defines whether multitouch is enabled or disabled
    const char * const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";

    // Timeout for long press
    const int LongPressTimeOut = 500;

    // Minimal distance for touch point from key button edge.
    const int CorrectionDistanceThreshold = 2;

    // Refuse to process more than X touchpoints at the same time:
    const int TouchPointLimit = 20;
}

M::InputMethodMode KeyButtonArea::InputMethodMode;

KeyButtonArea::KeyButtonArea(const LayoutData::SharedLayoutSection &sectionModel,
                             bool usePopup,
                             QGraphicsWidget *parent)
    : MStylableWidget(parent),
      mRelativeButtonBaseWidth(0),
      currentLevel(0),
      mPopup(usePopup ? PopupFactory::instance()->createPopup(this) : 0),
      newestTouchPointId(-1),
      wasGestureTriggered(false),
      enableMultiTouch(MGConfItem(MultitouchSettings).value().toBool()),
      activeDeadkey(0),
      feedbackPlayer(0),
      section(sectionModel),
      activelyPressedTouchPointId(-1)
{
    // By default multi-touch is disabled
    if (enableMultiTouch) {
        setAcceptTouchEvents(true);
    }

    grabGesture(FlickGestureRecognizer::sharedGestureType());
    feedbackPlayer = MComponentData::feedbackPlayer();

    longPressTimer.setSingleShot(true);
    longPressTimer.setInterval(LongPressTimeOut);

    connect(&longPressTimer, SIGNAL(timeout()),
            this, SLOT(handleLongKeyPressed()));

    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()));
}

KeyButtonArea::~KeyButtonArea()
{
    delete mPopup;
}

void KeyButtonArea::setInputMethodMode(M::InputMethodMode inputMethodMode)
{
    InputMethodMode = inputMethodMode;
}

qreal KeyButtonArea::relativeButtonBaseWidth() const
{
    return mRelativeButtonBaseWidth;
}

const LayoutData::SharedLayoutSection &KeyButtonArea::sectionModel() const
{
    return section;
}

void KeyButtonArea::updatePopup(const QPoint &pointerPosition, const IKeyButton *key)
{
    if (!mPopup) {
        return;
    }

    // Use prefetched key if given.
    if (!key) {
        // TODO: use gravitationalKeyAt() instead?
        key = keyAt(pointerPosition);
    }

    if (!key) {
        mPopup->handleInvalidKeyPressedOnMainArea();
        longPressTimer.stop();
        return;
    }

    const QRectF &buttonRect = key->buttonRect();
    // mimframework guarantees that scene positions matches with
    // screen position, so we can use mapToScene to calculate screen position
    const QPoint pos = mapToScene(buttonRect.topLeft()).toPoint();

    mPopup->updatePos(buttonRect.topLeft(), pos, buttonRect.toRect().size());

    // Get direction for finger position from key center and normalize components.
    QPointF direction(pointerPosition - buttonRect.center());
    direction.rx() /= buttonRect.width();
    direction.ry() /= buttonRect.height();

    longPressTimer.start();
    mPopup->setFingerPos(direction);
    mPopup->handleKeyPressedOnMainArea(key,
                                       activeDeadkey ? activeDeadkey->label() : QString(),
                                       level() % 2);
}

int KeyButtonArea::maxColumns() const
{
    return section->maxColumns();
}

int KeyButtonArea::rowCount() const
{
    return section->rowCount();
}

void
KeyButtonArea::handleVisibilityChanged(bool visible)
{
    if (mPopup) {
        mPopup->setEnabled(visible);
    }

    if (!visible) {
        clearActiveKeys();
        unlockDeadkeys();
    }
}

void
KeyButtonArea::switchLevel(int level)
{
    if (level != currentLevel) {
        currentLevel = level;

        // Update uppercase / lowercase
        updateButtonModifiers();

        update();
    }
}

int KeyButtonArea::level() const
{
    return currentLevel;
}

void KeyButtonArea::setShiftState(ModifierState /*newShiftState*/)
{
    // Empty default implementation
}

bool
KeyButtonArea::isObservableMove(const QPointF &prevPos, const QPointF &pos)
{
    qreal movement = qAbs(prevPos.x() - pos.x()) + qAbs(prevPos.y() - pos.y());

    return movement >= MovementThreshold;
}

void KeyButtonArea::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    const int newWidth = static_cast<int>(event->newSize().width());

    if (newWidth != static_cast<int>(event->oldSize().width())) {
        updateButtonGeometriesForWidth(newWidth);
    }
}

void
KeyButtonArea::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!enableMultiTouch) {
        // Qt always assigns zero to the first touch point, so pass id = 0.
        touchPointPressed(event->pos().toPoint(), 0);
    }
}


void
KeyButtonArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (scene()->mouseGrabberItem() == this) {
        // Ungrab mouse explicitly since we probably used grabMouse() to get it.
        ungrabMouse();
    }

    if (!enableMultiTouch) {
        touchPointReleased(event->pos().toPoint(), 0);
    }
}


void KeyButtonArea::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!enableMultiTouch) {
        touchPointMoved(event->pos().toPoint(), 0);
    }
}

void KeyButtonArea::setActiveKey(IKeyButton *key, TouchPointInfo &tpi)
{
    if (tpi.invalid) {
        return;
    }

    // Selected buttons are currently skipped.
    QString accent;

    if (activeDeadkey) {
        accent = activeDeadkey->label();
    }

    if (tpi.activeKey && (tpi.activeKey != key)) {
        // Release key
        tpi.activeKey->setDownState(false);
        // odd level numbers are upper case,
        // even level numbers are lower case
        emit keyReleased(tpi.activeKey, accent, level() % 2);
        tpi.activeKey = 0;
    }

    if (key && (tpi.activeKey != key)) {
        // Press key
        tpi.activeKey = key;
        tpi.activeKey->setDownState(true);
        emit keyPressed(tpi.activeKey, accent, level() % 2);
    }
}

void KeyButtonArea::clearActiveKeys()
{
    for (int i = 0; i < touchPoints.count(); ++i) {
        setActiveKey(0, touchPoints[i]);
    }
}

void KeyButtonArea::click(IKeyButton *key, const QPoint &touchPoint)
{
    if (!key) {
        return;
    }

    if (!key->isDeadKey()) {
        QString accent;

        if (activeDeadkey) {
            accent = activeDeadkey->label();
        }

        unlockDeadkeys();

        emit keyClicked(key, accent, level() % 2, touchPoint);
    } else if (key == activeDeadkey) {
        unlockDeadkeys();
    } else {
        // Deselect previous dead key, if any
        if (activeDeadkey) {
            activeDeadkey->setSelected(false);
        }

        activeDeadkey = key;
        activeDeadkey->setSelected(true);

        updateButtonModifiers();
    }
}

QVariant KeyButtonArea::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case QGraphicsItem::ItemVisibleChange:
        handleVisibilityChanged(value.toBool());
        break;

    default:
        break;
    }

    return QGraphicsItem::itemChange(change, value);
}

void KeyButtonArea::grabMouseEvent(QEvent */*event*/)
{
    // If keybuttonarea is hidden without mouseReleaseEvent
    // the enabled <flicked> would stay true if mouse
    // grab is obtained again without mousePressEvent.
    // This would ignore mouseReleaseEvent and would not cause keyClicked.
    wasGestureTriggered = false;

    const qreal ScalingFactor = style()->flickGestureThresholdRatio();
    const int HorizontalThreshold = static_cast<int>(boundingRect().width() * ScalingFactor);
    const int VerticalThreshold = static_cast<int>(boundingRect().height() * ScalingFactor);
    const int Timeout = style()->flickGestureTimeout();

    FlickGestureRecognizer::instance()->setFinishThreshold(HorizontalThreshold, VerticalThreshold);
    FlickGestureRecognizer::instance()->setStartThreshold(HorizontalThreshold / 2, VerticalThreshold / 2);
    FlickGestureRecognizer::instance()->setTimeout(Timeout);
}

void KeyButtonArea::ungrabMouseEvent(QEvent *)
{
    // Make sure popup can respond to mouse grab removal:
    if (mPopup) {
        mPopup->handleInvalidKeyPressedOnMainArea();
    }

    longPressTimer.stop();
}

bool KeyButtonArea::event(QEvent *e)
{
    bool eaten = false;

    if (e->type() == QEvent::Gesture) {
        const Qt::GestureType flickGestureType = FlickGestureRecognizer::sharedGestureType();
        FlickGesture *flickGesture = static_cast<FlickGesture *>(static_cast<QGestureEvent *>(e)->gesture(flickGestureType));

        if (flickGesture) {
            handleFlickGesture(flickGesture);
            eaten = true;
        }
    } else if (e->type() == QEvent::TouchBegin
               || e->type() == QEvent::TouchUpdate
               || e->type() == QEvent::TouchEnd) {
        QTouchEvent *touch = static_cast<QTouchEvent*>(e);

        foreach (const QTouchEvent::TouchPoint &tp, touch->touchPoints()) {

            switch (tp.state()) {
            case Qt::TouchPointPressed:
                touchPointPressed(mapFromScene(tp.screenPos()).toPoint(), tp.id());
                break;
            case Qt::TouchPointMoved:
                touchPointMoved(mapFromScene(tp.screenPos()).toPoint(), tp.id());
                break;
            case Qt::TouchPointReleased:
                touchPointReleased(mapFromScene(tp.screenPos()).toPoint(), tp.id());
                break;
            default:
                break;
            }
        }

        eaten = true;
    }

    return eaten || MWidget::event(e);
}

void KeyButtonArea::handleFlickGesture(FlickGesture *gesture)
{
    if (InputMethodMode == M::InputMethodModeDirect) {
        return;
    }

    // Any flick gesture, complete or not, resets active keys etc.
    if (!wasGestureTriggered && (gesture->state() != Qt::NoGesture)) {
        if (mPopup) {
            mPopup->handleInvalidKeyPressedOnMainArea();
        }

        longPressTimer.stop();
        clearActiveKeys();

        wasGestureTriggered = true;
    }

    if (gesture->state() == Qt::GestureFinished) {
        switch (gesture->direction()) {
        case FlickGesture::Left:
            emit flickLeft();
            break;

        case FlickGesture::Right:
            emit flickRight();
            break;

        case FlickGesture::Down:
            emit flickDown();
            break;

        case FlickGesture::Up: {
                const IKeyButton *flickedKey = keyAt(gesture->startPosition());
                if (flickedKey) {
                    emit flickUp(flickedKey->binding());
                }
                break;
            }

        default:
            return;
        }
    }
}

void KeyButtonArea::touchPointPressed(const QPoint &pos, int id)
{
    if (id < 0 || id >= TouchPointLimit) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Too many touchpoints - giving up.";
        return;
    }

    // Reset gesture checks.
    wasGestureTriggered = false;

    // Create new TouchPointInfo structure and overwrite any previous one.
    if (id >= touchPoints.size()) {
        touchPoints.append(TouchPointInfo());
    }

    if (activelyPressedTouchPointId > -1) {
        TouchPointInfo &pressedKeyTpi = touchPoints[activelyPressedTouchPointId];

        if (pressedKeyTpi.initialKey) {
            pressedKeyTpi.initialKey->setDownState(false);
            click(pressedKeyTpi.initialKey);
            pressedKeyTpi.initialKey = 0;
            pressedKeyTpi.invalid = true;
            activelyPressedTouchPointId = -1;
        }
    }

    newestTouchPointId = id;
    TouchPointInfo &tpi = touchPoints[id];
    tpi.reset();

    tpi.pos = pos;
    tpi.initialPos = pos;

    IKeyButton *key = keyAt(pos);
    if (!key) {
        return;
    }

    mTimestamp("KeyButtonArea", key->label());

    tpi.fingerInsideArea = true;
    tpi.initialKey = key;

    updatePopup(pos, key);
    setActiveKey(key, tpi);

    activelyPressedTouchPointId = id;
}

void KeyButtonArea::touchPointMoved(const QPoint &pos, int id)
{
    if (id < 0 || id >= TouchPointLimit) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Too many touchpoints - giving up.";
        return;
    }

    if (id >= touchPoints.size()) {
        touchPoints.append(TouchPointInfo());
    }

    TouchPointInfo &tpi = touchPoints[id];

    if (!isObservableMove(tpi.pos, pos) || tpi.invalid)
        return;

    tpi.pos = pos;

    if (wasGestureTriggered) {
        return;
    }

    // Check if finger is on a key.
    IKeyButton *key = gravitationalKeyAt(pos, id);

    if (key) {

        tpi.fingerInsideArea = true;

        if ((tpi.activeKey != key) && feedbackPlayer) {
            // Finger has slid from a key to an adjacent one.
            feedbackPlayer->play(MFeedbackPlayer::Press);
        }

        // If popup is visible, always update the position.
        if (id == newestTouchPointId) {
            updatePopup(pos, key);
        }
    } else {
        if (tpi.fingerInsideArea && feedbackPlayer) {
            feedbackPlayer->play(MFeedbackPlayer::Cancel);
        }
        // Finger has slid off the keys
        if (mPopup && tpi.fingerInsideArea && (id == newestTouchPointId)) {
            mPopup->handleInvalidKeyPressedOnMainArea();
            longPressTimer.stop();
        }

        tpi.fingerInsideArea = false;
    }

    setActiveKey(key, tpi);
}

void KeyButtonArea::touchPointReleased(const QPoint &pos, int id)
{
    if (id < 0 || id >= TouchPointLimit) {
        qWarning() << __PRETTY_FUNCTION__
                   << "Too many touchpoints - giving up.";
        return;
    }

    if (id >= touchPoints.size()) {
        touchPoints.append(TouchPointInfo());
    }

    if (activelyPressedTouchPointId == id) {
        activelyPressedTouchPointId = -1;
    }

    TouchPointInfo &tpi = touchPoints[id];

    tpi.fingerInsideArea = false;

    if (wasGestureTriggered || tpi.invalid) {
        return;
    }

    // We're finished with this touch point, inform popup:
    if (mPopup && (id == newestTouchPointId)) {
        mPopup->handleInvalidKeyPressedOnMainArea();
        longPressTimer.stop();
    }

    IKeyButton *key = gravitationalKeyAt(pos, id);

    if (key) {
        mTimestamp("KeyButtonArea", key->label());

        // It's presumably possible that we're getting this release event on top
        // of another after press event (of another key) without first getting a
        // move event (or at least such move event that we handle).  Which means
        // that we must send release event for the previous key and press event
        // for this key before sending release and clicked events for this key.
        setActiveKey(key, tpi); // in most cases, does nothing
        setActiveKey(0, tpi); // release key

        click(key, tpi.correctedTouchPoint);
    } else {
        setActiveKey(0, tpi);
    }
}

IKeyButton *
KeyButtonArea::gravitationalKeyAt(const QPoint &pos, int id)
{
    TouchPointInfo &tpi = touchPoints[id];

    if (!tpi.initialKey || !tpi.checkGravity) {
        return keyAt(pos);
    }

    const qreal hGravity = style()->touchpointHorizontalGravity();
    const qreal vGravity = style()->touchpointVerticalGravity();
    const QRectF &br = tpi.initialKey->buttonRect();

    if ((pos.x() > br.left() - hGravity)
        && (pos.x() < br.right() + hGravity)
        && (pos.y() > br.top() - vGravity)
        && (pos.y() < br.bottom() + vGravity)) {

        QPoint correctedPos = pos;
        if (pos.x() < br.left()) {
            correctedPos.setX(br.left() + CorrectionDistanceThreshold);
        } else if (pos.x() > br.right()) {
            correctedPos.setX(br.right() - CorrectionDistanceThreshold);
        }
        if (pos.y() < br.top()) {
            correctedPos.setY(br.top() + CorrectionDistanceThreshold);
        } else if (pos.y() > br.bottom()) {
            correctedPos.setY(br.bottom() - CorrectionDistanceThreshold);
        }
        tpi.correctedTouchPoint = correctedPos;

        return tpi.initialKey;
    } else {
        tpi.checkGravity = false;
        tpi.correctedTouchPoint = pos;
        return keyAt(pos);
    }
}

void
KeyButtonArea::unlockDeadkeys()
{
    if (activeDeadkey) {
        activeDeadkey->setSelected(false);
        activeDeadkey = 0;
        updateButtonModifiers();
    }
}

void KeyButtonArea::drawReactiveAreas(MReactionMap */*reactionMap*/, QGraphicsView */*view*/)
{
    // Empty default implementation. Geometries of buttons are known by derived classes.
}

const PopupBase &KeyButtonArea::popup() const
{
    return *mPopup;
}

void KeyButtonArea::updateButtonModifiers()
{
    bool shift = (currentLevel == 1);

    // We currently don't allow active dead key level changing. If we did,
    // we should update activeDeadkey level before delivering its accent to
    // other keys.
    const QChar accent(activeDeadkey ? activeDeadkey->label().at(0) : '\0');

    modifiersChanged(shift, accent);
}

void KeyButtonArea::modifiersChanged(bool /*shift*/, const QChar /*accent*/)
{
    // Empty default implementation
}

void KeyButtonArea::onThemeChangeCompleted()
{
    updateButtonGeometriesForWidth(size().width());
}

const KeyButtonAreaStyleContainer &KeyButtonArea::baseStyle() const
{
    return style();
}

void KeyButtonArea::handleLongKeyPressed()
{
    activelyPressedTouchPointId = -1;

    QString accent;

    if (activeDeadkey) {
        accent = activeDeadkey->label();
    }

    TouchPointInfo &tpi = touchPoints[newestTouchPointId];
    if (tpi.activeKey) {
        if (mPopup) {
            mPopup->handleLongKeyPressedOnMainArea(tpi.activeKey, accent, level() % 2);
        }
        emit longKeyPressed(tpi.activeKey, accent, level() % 2);
    }
}

KeyButtonArea::TouchPointInfo::TouchPointInfo()
    : fingerInsideArea(false),
      activeKey(0),
      initialKey(0),
      initialPos(),
      pos(),
      checkGravity(true),
      invalid(false)
{
}

void KeyButtonArea::TouchPointInfo::reset()
{
    fingerInsideArea = false;
    activeKey = 0;
    initialKey = 0;
    initialPos = QPoint();
    pos = QPoint();
    checkGravity = true;
    invalid = false;
}
