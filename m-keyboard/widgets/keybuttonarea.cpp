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
}

M::InputMethodMode KeyButtonArea::InputMethodMode;

KeyButtonArea::KeyButtonArea(const LayoutData::SharedLayoutSection &sectionModel,
                             bool usePopup,
                             QGraphicsWidget *parent)
    : MStylableWidget(parent),
      currentLevel(0),
      popup(PopupFactory::instance()->createPopup(this)),
      newestTouchPointId(-1),
      wasGestureTriggered(false),
      enableMultiTouch(MGConfItem(MultitouchSettings).value().toBool()),
      activeDeadkey(0),
      feedbackPlayer(0),
      section(sectionModel),
      usePopup(usePopup)
{
    // By default multi-touch is disabled
    if (enableMultiTouch) {
        setAcceptTouchEvents(true);
    }

    grabGesture(FlickGestureRecognizer::sharedGestureType());

    popup->setKeyboardFont(style()->font());
    popup->hidePopup();

    feedbackPlayer = MComponentData::feedbackPlayer();

    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);

    longPressTimer.setSingleShot(true);
    longPressTimer.setInterval(LongPressTimeOut);
    connect(&longPressTimer, SIGNAL(timeout()),
            this, SLOT(handleLongKeyPressed()));
}

KeyButtonArea::~KeyButtonArea()
{
    delete popup;
}

void KeyButtonArea::setInputMethodMode(M::InputMethodMode inputMethodMode)
{
    InputMethodMode = inputMethodMode;
}

const LayoutData::SharedLayoutSection &KeyButtonArea::sectionModel() const
{
    return section;
}

void KeyButtonArea::updatePopup(const QPoint &pointerPosition, const IKeyButton *key)
{
    // Use prefetched key if given.
    if (!key) {
        key = keyAt(pointerPosition);
    }

    if (!key || key->label().isEmpty() || key->key().style() != VKBDataKey::NormalStyle) {
        popup->hidePopup();
        longPressTimer.stop();
        return;
    }

    const QRectF &buttonRect = key->buttonRect();
    // mimframework guarantees that scene positions matches with
    // screen position, so we can use mapToScene to calculate screen position
    const QPoint pos = mapToScene(buttonRect.topLeft()).toPoint();

    popup->updatePos(buttonRect.topLeft(), pos, buttonRect.toRect().size());

    // Get direction for finger position from key center and normalize components.
    QPointF direction(pointerPosition - buttonRect.center());
    direction.rx() /= buttonRect.width();
    direction.ry() /= buttonRect.height();
    popup->setFingerPos(direction);
    popup->setTargetButton(key);

    popup->showPopup();

    // start timer if finger is moved to other key
    if (touchPoints[newestTouchPointId].activeKey != key) {
        longPressTimer.start();
    }
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
KeyButtonArea::onHide()
{
    clearActiveKeys();
    unlockDeadkeys();
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
    if (change == QGraphicsItem::ItemVisibleChange && !value.toBool()) {
        onHide();
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

void KeyButtonArea::ungrabMouseEvent(QEvent */*event*/)
{
    // Make sure popup is hidden even if mouse grab
    // is lost without mouse release event.
    popup->hidePopup();
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

        if (e->type() == QEvent::TouchBegin) {
            touchPoints.clear();
        }

        foreach (const QTouchEvent::TouchPoint &tp, touch->touchPoints()) {

            switch (tp.state()) {
            case Qt::TouchPointPressed:
                touchPointPressed(tp.pos().toPoint(), tp.id());
                break;
            case Qt::TouchPointMoved:
                touchPointMoved(tp.pos().toPoint(), tp.id());
                break;
            case Qt::TouchPointReleased:
                touchPointReleased(tp.pos().toPoint(), tp.id());
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
        popup->hidePopup();
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
    newestTouchPointId = id;

    // Create new TouchPointInfo structure and overwrite any previous one.
    touchPoints[id] = TouchPointInfo();
    TouchPointInfo &tpi = touchPoints[id];
    tpi.pos = pos;
    tpi.initialPos = pos;

    // Reset gesture checks.
    wasGestureTriggered = false;

    IKeyButton *key = keyAt(pos);
    if (!key) {
        return;
    }

    mTimestamp("KeyButtonArea", key->label());

    tpi.initialKey = key;
    tpi.fingerInsideArea = true;

    if (usePopup && (id == newestTouchPointId)) {
        updatePopup(pos, key);
    }

    setActiveKey(key, tpi);
}

void KeyButtonArea::touchPointMoved(const QPoint &pos, int id)
{
    TouchPointInfo &tpi = touchPoints[id];

    if (!isObservableMove(tpi.pos, pos))
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
        if (usePopup
            && (id == newestTouchPointId)) {
            updatePopup(pos, key);
        }
    } else {
        if (tpi.fingerInsideArea && feedbackPlayer) {
            feedbackPlayer->play(MFeedbackPlayer::Cancel);
        }
        // Finger has slid off the keys
        if (tpi.fingerInsideArea && (id == newestTouchPointId)) {
            popup->hidePopup();
            longPressTimer.stop();
        }

        tpi.fingerInsideArea = false;
    }

    setActiveKey(key, tpi);
}

void KeyButtonArea::touchPointReleased(const QPoint &pos, int id)
{
    TouchPointInfo &tpi = touchPoints[id];

    tpi.fingerInsideArea = false;

    if (wasGestureTriggered) {
        return;
    }

    // Hide popup otherwise it could be shown on another touch point.
    if (id == newestTouchPointId) {
        popup->hidePopup();
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

bool
KeyButtonArea::isPopupActive() const
{
    return popup->isPopupVisible();
}

void KeyButtonArea::drawReactiveAreas(MReactionMap */*reactionMap*/, QGraphicsView */*view*/)
{
    // Empty default implementation. Geometries of buttons are known by derived classes.
}

const PopupBase &KeyButtonArea::popupWidget() const
{
    return *popup;
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
    popup->setKeyboardFont(style()->font());
}

const KeyButtonAreaStyleContainer &KeyButtonArea::baseStyle() const
{
    return style();
}

void KeyButtonArea::handleLongKeyPressed()
{
    QString accent;

    if (activeDeadkey) {
        accent = activeDeadkey->label();
    }

    TouchPointInfo &tpi = touchPoints[newestTouchPointId];
    if (tpi.activeKey) {
        emit longKeyPressed(tpi.activeKey, accent, level() % 2);
    }
}

KeyButtonArea::TouchPointInfo::TouchPointInfo()
    : fingerInsideArea(false),
      activeKey(0),
      initialKey(0),
      initialPos(),
      pos(),
      checkGravity(true)
{
}
