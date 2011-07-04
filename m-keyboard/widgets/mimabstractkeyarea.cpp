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
#include "flickgesture.h"
#include "flickgesturerecognizer.h"
#include "mimabstractkeyarea.h"
#include "mimabstractkeyarea_p.h"
#include "mimkeyvisitor.h"
#include "mimreactionmap.h"
#include "mkeyboardhost.h"
#include "mimabstractpopup.h"

#include <MCancelEvent>
#include <MFeedback>
#include <mimsettings.h>
#include <QEvent>
#include <QGraphicsSceneMouseEvent>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QCoreApplication>
#include <mtimestamp.h>

#ifdef unix
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

namespace
{
    // This GConf item defines whether multitouch is enabled or disabled
    const char *const MultitouchSettings = "/meegotouch/inputmethods/multitouch/enabled";

    // Minimal distance (in px) for touch point from key button edge.
    const int CorrectionDistanceThreshold = 2;

    // Used for error correction:
    QPoint gAdjustedPositionForCorrection = QPoint();

    QPointF adjustedByGravity(const MImAbstractKey *const key,
                              const QPoint &pos,
                              int horizontalGravity,
                              int verticalGravity)
    {
        if (key
            && key->isGravityActive()
            && key->buttonBoundingRect().adjusted(-horizontalGravity, -verticalGravity,
                                                   horizontalGravity,  verticalGravity).contains(pos)) {
            return key->buttonBoundingRect().center();
        }

        return pos;
    }

    QString toString(const QPointF &p, const QString &separator = ", ")
    {
        return QString("%1%2%3").arg(p.x())
                                .arg(separator)
                                .arg(p.y());
    }

    QString toString(const QRectF &r, const QString separator = ", ")
    {
        return QString("%1%2%3%4%5").arg(toString(r.topLeft(), separator))
                                    .arg(separator)
                                    .arg(r.width())
                                    .arg(separator)
                                    .arg(r.height());
    }

    QString toString(Qt::TouchPointState state)
    {
        switch(state) {
        default:
            return "n/a";

        case Qt::TouchPointPressed:
            return "pressed";

        case Qt::TouchPointMoved:
            return "moved";

        case Qt::TouchPointStationary:
            return "stationary";

        case Qt::TouchPointReleased:
            return "released";
        }
    }

    QString timeStamp()
    {
#ifdef unix
        struct timeval tv;
        gettimeofday(&tv, 0);
        return  QString("%1.%2").arg(tv.tv_sec).arg(tv.tv_usec);
#else
        return QTime::currentTime().toString();
#endif
    }
}

M::InputMethodMode MImAbstractKeyAreaPrivate::InputMethodMode;

MImAbstractKeyAreaPrivate::MImAbstractKeyAreaPrivate(const LayoutData::SharedLayoutSection &newSection,
                                                     MImAbstractKeyArea *owner)
    : q_ptr(owner),
      currentLevel(0),
      popup(0),
      wasGestureTriggered(false),
      feedbackSliding(MImReactionMap::Sliding),
      section(newSection),
      primaryPressArrived(false),
      primaryReleaseArrived(false),
      mouseMoveInTransition(false),
      allowedHorizontalFlick(true),
      ignoreTouchEventsUntilNewBegin(false)
{
}

MImAbstractKeyAreaPrivate::~MImAbstractKeyAreaPrivate()
{
}

void MImAbstractKeyAreaPrivate::click(MImAbstractKey *key, const KeyContext &keyContext)
{
    Q_Q(MImAbstractKeyArea);

    if (!key || !key->enabled()) {
        return;
    }

    MImKeyVisitor::SpecialKeyFinder deadFinder(MImKeyVisitor::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&deadFinder);
    MImAbstractKey *deadKey = deadFinder.deadKey();

    if (!key->isDeadKey()) {
        emit q->keyClicked(key, keyContext);
        if (!key->isShiftKey()) {
            q->unlockDeadKeys(deadKey);
        }
    } else if (key == deadKey) {
        q->unlockDeadKeys(deadKey);
    } else {
        // Deselect previous dead key, if any:
        if (deadKey) {
            deadKey->setSelected(false);
        }

        // key is the new deadkey:
        key->setSelected(true);
        q->updateKeyModifiers(key->label().at(0));
    }
}

void MImAbstractKeyAreaPrivate::handleFlickGesture(int direction,
                                                   Qt::GestureState state)
{
    Q_Q(MImAbstractKeyArea);

    if (InputMethodMode == M::InputMethodModeDirect) {
        return;
    }

    if (!allowedHorizontalFlick) {
        if ((direction == FlickGesture::Left)
            || (direction == FlickGesture::Right)) {
            return;
        }
    }

    // Any non-upward flick gesture, complete or not, resets active keys etc.
    if (!wasGestureTriggered
        && (state != Qt::NoGesture)
        && direction != FlickGesture::Up) {

        if (popup) {
            popup->cancel();
        }

        MImAbstractKey *const lastActiveKey = MImAbstractKey::lastActiveKey();
        if (lastActiveKey && lastActiveKey->state() == MImAbstractKey::Pressed) {
            MImKeyVisitor::SpecialKeyFinder finder;
            MImAbstractKey::visitActiveKeys(&finder);
            const bool hasActiveShiftKeys = (finder.shiftKey() != 0);
            const KeyContext context(hasActiveShiftKeys || isUpperCase());
            emit q->keyCancelled(lastActiveKey, context);
        }

        MImKeyVisitor::KeyAreaReset reset;
        MImAbstractKey::visitActiveKeys(&reset);

        longPressTimer.stop();
        wasGestureTriggered = true;
    }

    if (state == Qt::GestureFinished) {

        switch (direction) {
        case FlickGesture::Left:
            emit q->flickLeft();
            break;

        case FlickGesture::Right:
            emit q->flickRight();
            break;

        case FlickGesture::Down:
            emit q->flickDown();
            break;

        case FlickGesture::Up:
            // Flick up not used.
            break;

        default:
            return;
        }
    }
}

void MImAbstractKeyAreaPrivate::handleTouchEvent(QTouchEvent *event)
{
    Q_Q(MImAbstractKeyArea);

    if (event->type() == QEvent::TouchBegin) {
        ignoreTouchEventsUntilNewBegin = false;
    }

    if (!q->isVisible() ||
        ignoreTouchEventsUntilNewBegin) {
        return;
    }

    foreach (const QTouchEvent::TouchPoint &tp, event->touchPoints()) {

        switch (tp.state()) {
        case Qt::TouchPointPressed:
            if (tp.isPrimary()) {
                primaryTouchPointPressed(tp);

                // Primary touch point pressed and there are other active touch points.
                if (event->touchPoints().count() > 1) {
                    // Next mouse move will have a last position which is not suitable
                    // for touch event conversion.
                    mouseMoveInTransition = true;
                }
            } else {
                touchPointPressed(tp);
            }
            break;

        case Qt::TouchPointMoved:
            // Primary touch point moves are always generated
            // from mouseMoveEvent() handler.
            if (!tp.isPrimary()) {
                touchPointMoved(tp);
            }
            break;

        case Qt::TouchPointReleased:
            if (tp.isPrimary()) {
                primaryTouchPointReleased(tp);
            } else {
                touchPointReleased(tp);
            }
            break;
        default:
            break;
        }
    }
}

void MImAbstractKeyAreaPrivate::primaryTouchPointPressed(const QTouchEvent::TouchPoint &tp)
{
    if (!primaryPressArrived) {
        primaryPressArrived = true;
        primaryReleaseArrived = false;
        touchPointPressed(tp);
    }
}

void MImAbstractKeyAreaPrivate::primaryTouchPointMoved(const QTouchEvent::TouchPoint &tp)
{
    // The check for both press and release serve the role of a sanity check,
    // and in addition the release is checked for the purpose of discarding
    // further mouse move events after primary touch point is lifted.
    // This is because mouse move events will continue to be delivered for
    // another, non-primary, touch point.
    if (!primaryPressArrived || primaryReleaseArrived) {
        return;
    }

    touchPointMoved(tp);
}

void MImAbstractKeyAreaPrivate::primaryTouchPointReleased(const QTouchEvent::TouchPoint &tp)
{
    // Just return in case press has not arrived. This can also be due to mouse being
    // grabbed to somewhere else.
    if (!primaryPressArrived) {
        return;
    }

    if (!primaryReleaseArrived) {
        touchPointReleased(tp);
        primaryReleaseArrived = true;
        primaryPressArrived = false;
    }
}

void MImAbstractKeyAreaPrivate::touchPointPressed(const QTouchEvent::TouchPoint &tp)
{
    Q_Q(MImAbstractKeyArea);

    mTimestamp("MImAbstractKeyArea", "start");
    wasGestureTriggered = false;
    mostRecentTouchPosition = tp.pos();

    // Gestures only slow down in speed typing mode:
    if (isInSpeedTypingMode(true)) {
        idleVkbTimer.stop();
        // TODO: check how expensive gesture (un)grabbing is:
        q->ungrabGesture(FlickGestureRecognizer::sharedGestureType());
    }

    const QPoint pos = q->correctedTouchPoint(tp.scenePos());
    MImAbstractKey *key = q->keyAt(pos);


    if (q->debugTouchPoints) {
        q->logTouchPoint(tp, key);
    }

    if (!key || !key->enabled()) {
        longPressTimer.stop();
        mTimestamp("MImAbstractKeyArea", "end");
        return;
    }


    // Try to commit currently active key before activating new key:
    MImAbstractKey *const lastActiveKey = MImAbstractKey::lastActiveKey();
    MImKeyVisitor::SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    if (q->style()->commitPreviousKeyOnPress()
        && lastActiveKey
        && lastActiveKey != key
        && lastActiveKey->enabled()
        && lastActiveKey->isNormalKey()
        && lastActiveKey->touchPointCount() > 0) {
        // TODO: play release sound? Potentially confusing to user, who
        // might still press this key.

        emit q->keyClicked(lastActiveKey,
                           KeyContext(hasActiveShiftKeys || isUpperCase(),
                                      QString(),
                                      tp.scenePos(),
                                      gAdjustedPositionForCorrection));

        lastActiveKey->resetTouchPointCount();
    }

    if (key->increaseTouchPointCount()
        && key->touchPointCount() == 1) {
        q->updatePopup(key);
        longPressTimer.start(q->style()->longPressTimeout());

        // We activate the key's gravity here because a key cannot
        // differentiate between initially-pressed-key or
        // activated-by-moving-onto-it.
        // However, the key deactivates the gravity itself again
        // (touchpoint count goes from 1 to 0).
        key->activateGravity();

        emit q->keyPressed(key,
                           KeyContext(hasActiveShiftKeys || isUpperCase(),
                                      finder.deadKey() ? finder.deadKey()->label() : QString(),
                                      tp.scenePos(), QPoint(), tp.isPrimary()));
    }
    mTimestamp("MImAbstractKeyArea", "end");
}

void MImAbstractKeyAreaPrivate::touchPointMoved(const QTouchEvent::TouchPoint &tp)
{
    Q_Q(MImAbstractKeyArea);

    mostRecentTouchPosition = tp.pos();

    if (wasGestureTriggered) {
        longPressTimer.stop();
        return;
    }

    if (tp.scenePos() == tp.lastScenePos()) {
        return;
    }

    mTimestamp("MImAbstractKeyArea", "start");

    const QPoint pos = q->correctedTouchPoint(tp.scenePos());
    const QPoint lastPos = q->correctedTouchPoint(tp.lastScenePos());

    const GravitationalLookupResult lookup = gravitationalKeyAt(pos, lastPos);
    MImKeyVisitor::SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);
    const bool hasActiveShiftKeys = (finder.shiftKey() != 0);

    // For a moving touchpoint, we only need to consider enter-key or leave-key events:
    if (lookup.key != lookup.lastKey) {
        q->updatePopup(lookup.key);

        if (lookup.key) {
            if (lookup.key->enabled()
                && lookup.key->increaseTouchPointCount()
                && lookup.key->touchPointCount() == 1) {
                // Reaction map cannot discover when we move from one key
                // (= reactive area) to another
                // slot is called asynchronously to get screen update as fast as possible
                QMetaObject::invokeMethod(&feedbackSliding, "play", Qt::QueuedConnection);

                longPressTimer.start(q->style()->longPressTimeout());
                emit q->keyPressed(lookup.key,
                                   KeyContext(hasActiveShiftKeys || isUpperCase(),
                                              finder.deadKey() ? finder.deadKey()->label() : QString(),
                                              tp.scenePos(), QPoint(), tp.isPrimary()));
            }
        }

        if (lookup.lastKey
            && lookup.lastKey->enabled()
            && lookup.lastKey->decreaseTouchPointCount()
            && lookup.lastKey->touchPointCount() == 0) {
            emit q->keyReleased(lookup.lastKey,
                                KeyContext(hasActiveShiftKeys || isUpperCase(),
                                           finder.deadKey() ? finder.deadKey()->label() : QString()));
        }
    }

    if (!lookup.key || !lookup.key->enabled()) {
        longPressTimer.stop();
    }

    if (q->debugTouchPoints) {
        q->logTouchPoint(tp, lookup.key, lookup.lastKey);
    }
    mTimestamp("MImAbstractKeyArea", "end");
}

void MImAbstractKeyAreaPrivate::touchPointReleased(const QTouchEvent::TouchPoint &tp)
{
    Q_Q(MImAbstractKeyArea);

    mostRecentTouchPosition = tp.pos();

    if (wasGestureTriggered) {
        return;
    }
    mTimestamp("MImAbstractKeyArea", "start");

    idleVkbTimer.start(q->style()->idleVkbTimeout());

    const QPoint pos = q->correctedTouchPoint(tp.scenePos());
    const QPoint lastPos = q->correctedTouchPoint(tp.lastScenePos());

    const GravitationalLookupResult lookup = gravitationalKeyAt(pos, lastPos);

    MImKeyVisitor::SpecialKeyFinder finder;
    MImAbstractKey::visitActiveKeys(&finder);

    // Key context before release
    KeyContext keyContext((finder.shiftKey() != 0) || isUpperCase(),
                          finder.deadKey() ? finder.deadKey()->label() : QString(),
                          tp.scenePos(),
                          gAdjustedPositionForCorrection);

    if (lookup.key
        && lookup.key->enabled()
        && lookup.key->decreaseTouchPointCount()
        && lookup.key->touchPointCount() == 0) {
        longPressTimer.stop();

        emit q->keyReleased(lookup.key, keyContext);

        // Update key context after release.
        MImAbstractKey::visitActiveKeys(&finder);
        KeyContext clickContext = keyContext;
        clickContext.upperCase = (finder.shiftKey() != 0) || isUpperCase();
        clickContext.accent = finder.deadKey() ? finder.deadKey()->label() : QString();

        click(lookup.key, clickContext);
    }

    if (lookup.lastKey
        && lookup.lastKey != lookup.key
        && lookup.lastKey->enabled()
        && lookup.lastKey->decreaseTouchPointCount()
        && lookup.lastKey->touchPointCount() == 0) {
        // Use the same key context as before release.
        emit q->keyReleased(lookup.lastKey, keyContext);
    }

    // We're finished with this touch point, inform popup:
    if (popup) {
        popup->cancel();
    }

    longPressTimer.stop();

    if (q->debugTouchPoints) {
        q->logTouchPoint(tp, lookup.key, lookup.lastKey);
    }
    mTimestamp("MImAbstractKeyArea", "end");
}

QTouchEvent::TouchPoint
MImAbstractKeyAreaPrivate::fromMouseEvent(const QGraphicsSceneMouseEvent *event,
                                          MouseEventToTouchPointOption option)
{
    Qt::TouchPointState state;
    switch (event->type()) {
    case QEvent::GraphicsSceneMousePress:
    case QEvent::GraphicsSceneMouseDoubleClick: // arrives on press
        state = Qt::TouchPointPressed;
        break;
    case QEvent::GraphicsSceneMouseMove:
        state = Qt::TouchPointMoved;
        break;
    case QEvent::GraphicsSceneMouseRelease:
        state = Qt::TouchPointReleased;
        break;
    default:
        qWarning("Trying to convert mouse event of invalid type to a touch point.");
        state = Qt::TouchPointStationary;
        break;
    }

    QTouchEvent::TouchPoint tp(0);
    tp.setState(state | Qt::TouchPointPrimary);
    tp.setPos(event->pos());
    tp.setScenePos(event->scenePos());

    if (option == CopyAllMembers) {
        tp.setLastPos(event->lastPos());
        tp.setLastScenePos(event->lastScenePos());
    } else {
        tp.setLastPos(event->pos());
        tp.setLastScenePos(event->scenePos());
    }

    return tp;
}

bool
MImAbstractKeyAreaPrivate::multiTouchEnabled()
{
    static bool gConfRead = false;
    static bool touchEventsAccepted = false;
    if (!gConfRead) {
        touchEventsAccepted = MImSettings(MultitouchSettings).value().toBool();
        gConfRead = true;
    }
    return touchEventsAccepted;
}

MImAbstractKeyAreaPrivate::GravitationalLookupResult
MImAbstractKeyAreaPrivate::gravitationalKeyAt(const QPoint &pos,
                                              const QPoint &lastPos) const
{
    Q_Q(const MImAbstractKeyArea);

    // TODO: Needs explicit test coverage, maybe.
    MImAbstractKey *key = 0;
    MImAbstractKey *lastKey = 0;

    const qreal hGravity = q->style()->touchpointHorizontalGravity();
    const qreal vGravity = q->style()->touchpointVerticalGravity();

    key = q->keyAt(adjustedByGravity(MImAbstractKey::lastActiveKey(),
                                     pos, hGravity, vGravity).toPoint());
    lastKey = q->keyAt(adjustedByGravity(MImAbstractKey::lastActiveKey(),
                                         lastPos, hGravity, vGravity).toPoint());

    // Check whether error correction needs updated position:
    if (key) {
        QPoint &ec = gAdjustedPositionForCorrection;
        const QRectF &br = key->buttonRect();

        ec.setX(qBound<int>(br.left() + CorrectionDistanceThreshold,
                            pos.x(),
                            br.right() - CorrectionDistanceThreshold));

        ec.setY(qBound<int>(br.top() + CorrectionDistanceThreshold,
                            pos.y(),
                            br.bottom() - CorrectionDistanceThreshold));
    }

    return GravitationalLookupResult(key, lastKey);
}

bool MImAbstractKeyAreaPrivate::isInSpeedTypingMode(bool restartTimers)
{
    Q_Q(MImAbstractKeyArea);

    return ((restartTimers ? lastTouchPointPressEvent.restart()
                           : lastTouchPointPressEvent.elapsed()) < q->style()->idleVkbTimeout());
}

void MImAbstractKeyAreaPrivate::switchStyleMode()
{
    Q_Q(MImAbstractKeyArea);

    if (!q->style()->syncStyleModeWithKeyCount()) {
        return;
    }

    switch(section->keyCount()) {

    case 10:
        q->style().setModeKeys10();
        break;

    case 11:
        q->style().setModeKeys11();
        break;

    case 12:
        q->style().setModeKeys12();
        break;

    case 13:
        q->style().setModeKeys13();
        break;

    case 14:
        q->style().setModeKeys14();
        break;

    case 15:
        q->style().setModeKeys15();
        break;

    case 30:
        q->style().setModeKeys30();
        break;

    case 31:
        q->style().setModeKeys31();
        break;

    case 32:
        q->style().setModeKeys32();
        break;

    case 33:
        q->style().setModeKeys33();
        break;

    case 34:
        q->style().setModeKeys34();
        break;

    case 35:
        q->style().setModeKeys35();
        break;

    case 36:
        q->style().setModeKeys36();
        break;

    case 37:
        q->style().setModeKeys37();
        break;

    case 38:
        q->style().setModeKeys38();
        break;

    case 39:
        q->style().setModeKeys39();
        break;

    case 40:
        q->style().setModeKeys40();
        break;

    case 41:
        q->style().setModeKeys41();
        break;

    case 42:
        q->style().setModeKeys42();
        break;

    case 43:
        q->style().setModeKeys43();
        break;

    case 44:
        q->style().setModeKeys44();
        break;

    case 45:
        q->style().setModeKeys45();
        break;

    default:
        break;
    }
}

// actual class implementation
MImAbstractKeyArea::MImAbstractKeyArea(MImAbstractKeyAreaPrivate *privateData,
                                       QGraphicsWidget *parent)
    : MStylableWidget(parent),
      mRelativeKeyBaseWidth(0),
      debugTouchPoints(false),
      d_ptr(privateData)
{
    Q_D(MImAbstractKeyArea);

    // By default multi-touch is disabled
    if (d->multiTouchEnabled()) {
        setAcceptTouchEvents(true);
    }

    d->lastTouchPointPressEvent.restart();
    grabGesture(FlickGestureRecognizer::sharedGestureType());

    d->longPressTimer.setSingleShot(true);
    d->idleVkbTimer.setSingleShot(true);

    connect(&d->longPressTimer, SIGNAL(timeout()),
            this, SLOT(handleLongKeyPressed()));

    connect(&d->idleVkbTimer, SIGNAL(timeout()),
            this, SLOT(handleIdleVkb()));

    connect(MTheme::instance(), SIGNAL(themeChangeCompleted()),
            this, SLOT(onThemeChangeCompleted()),
            Qt::UniqueConnection);
}

MImAbstractKeyArea::~MImAbstractKeyArea()
{
    delete d_ptr;
}

void MImAbstractKeyArea::init()
{
    Q_D(MImAbstractKeyArea);

    d->switchStyleMode();
    debugTouchPoints = style()->debugTouchPoints();
}

void MImAbstractKeyArea::setInputMethodMode(M::InputMethodMode inputMethodMode)
{
    MImAbstractKeyAreaPrivate::InputMethodMode = inputMethodMode;
}

qreal MImAbstractKeyArea::relativeKeyBaseWidth() const
{
    return mRelativeKeyBaseWidth;
}

const LayoutData::SharedLayoutSection &MImAbstractKeyArea::sectionModel() const
{
    Q_D(const MImAbstractKeyArea);

    return d->section;
}

void MImAbstractKeyArea::updatePopup(MImAbstractKey *key)
{
    Q_D(MImAbstractKeyArea);

    if (!d->popup) {
        return;
    }

    if (!key || !isVisible()) {
        d->popup->cancel();
        return;
    }

    const QRectF &buttonRect = key->buttonRect();
    // IM FW guarantees that a scene position matches with
    // screen position, so we can use mapToScene to calculate screen position:
    const QPoint pos = mapToScene(buttonRect.topLeft()).toPoint();

    MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&finder);

    d->popup->updatePos(buttonRect.topLeft(), pos, buttonRect.toRect().size());
    d->popup->handleKeyPressedOnMainArea(key,
                                          KeyContext(d->isUpperCase(),
                                                     finder.deadKey() ? finder.deadKey()->label() : QString()));
}

int MImAbstractKeyArea::maxColumns() const
{
    Q_D(const MImAbstractKeyArea);

    return d->section->maxColumns();
}

int MImAbstractKeyArea::rowCount() const
{
    Q_D(const MImAbstractKeyArea);

    return d->rowCount();
}

void
MImAbstractKeyArea::handleVisibilityChanged(bool visible)
{
    Q_D(MImAbstractKeyArea);

    if (!visible) {
        if (d->popup) {
            d->popup->setVisible(false);
        }

        MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::FindDeadKey);
        MImAbstractKey::visitActiveKeys(&finder);

        unlockDeadKeys(finder.deadKey());

        foreach (const MImAbstractKey *key, keys()) {
            // We know there is a special case for backspace
            // need keyCancelled signal
            if (key->isBackspaceKey()
                && key->state() == MImAbstractKey::Pressed) {
                emit keyCancelled(key, KeyContext());
            }
        }

        MImKeyVisitor::KeyAreaReset keyAreaReset;
        keyAreaReset.setKeyParentItem(this);
        MImAbstractKey::visitActiveKeys(&keyAreaReset);
    }
}

void
MImAbstractKeyArea::switchLevel(int level)
{
    Q_D(MImAbstractKeyArea);

    if (level != d->currentLevel) {
        d->currentLevel = level;

        // Update uppercase / lowercase
        MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::FindDeadKey);
        MImAbstractKey::visitActiveKeys(&finder);

        updateKeyModifiers(finder.deadKey() ? finder.deadKey()->label().at(0) : '\0');

        update();
    }
}

int MImAbstractKeyArea::level() const
{
    Q_D(const MImAbstractKeyArea);

    return d->currentLevel;
}

void MImAbstractKeyArea::setShiftState(ModifierState /*newShiftState*/)
{
    // Empty default implementation
}

void MImAbstractKeyArea::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    const int newWidth = static_cast<int>(event->newSize().width());

    if (newWidth != static_cast<int>(event->oldSize().width())) {
        updateKeyGeometries(newWidth);
    }
}

void MImAbstractKeyArea::mousePressEvent(QGraphicsSceneMouseEvent *ev)
{
    Q_D(MImAbstractKeyArea);
    d->primaryTouchPointPressed(d->fromMouseEvent(ev));
    d->mouseMoveInTransition = false;
}

void MImAbstractKeyArea::mouseMoveEvent(QGraphicsSceneMouseEvent *ev)
{
    Q_D(MImAbstractKeyArea);
    d->primaryTouchPointMoved(d->fromMouseEvent(ev, d->mouseMoveInTransition ?
                                                    MImAbstractKeyAreaPrivate::ResetLastPosMember :
                                                    MImAbstractKeyAreaPrivate::CopyAllMembers));
    d->mouseMoveInTransition = false;
}

void MImAbstractKeyArea::mouseReleaseEvent(QGraphicsSceneMouseEvent *ev)
{
    Q_D(MImAbstractKeyArea);
    d->primaryTouchPointReleased(d->fromMouseEvent(ev));
}

QVariant MImAbstractKeyArea::itemChange(GraphicsItemChange change, const QVariant &value)
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

void MImAbstractKeyArea::grabMouseEvent(QEvent */*event*/)
{
    Q_D(MImAbstractKeyArea);

    // If mimabstractkeyarea is hidden without mouseReleaseEvent
    // the enabled <flicked> would stay true if mouse
    // grab is obtained again without mousePressEvent.
    // This would ignore mouseReleaseEvent and would not cause keyClicked.
    d->wasGestureTriggered = false;

    const qreal ScalingFactor = style()->flickGestureThresholdRatio();
    const int HorizontalThreshold = static_cast<int>(boundingRect().width() * ScalingFactor);
    const int VerticalThreshold = static_cast<int>(boundingRect().height() * ScalingFactor);
    const int Timeout = style()->flickGestureTimeout();

    FlickGestureRecognizer::instance()->setFinishThreshold(HorizontalThreshold, VerticalThreshold);
    FlickGestureRecognizer::instance()->setStartThreshold(HorizontalThreshold / 2, VerticalThreshold / 2);
    FlickGestureRecognizer::instance()->setTimeout(Timeout);
}

void MImAbstractKeyArea::ungrabMouseEvent(QEvent *)
{
    Q_D(MImAbstractKeyArea);

    // Make sure popup can respond to mouse grab removal:
    if (d->popup) {
        d->popup->cancel();
    }

    d->primaryPressArrived = false;
    d->primaryReleaseArrived = false;

    d->longPressTimer.stop();
}

void MImAbstractKeyArea::cancelEvent(MCancelEvent *)
{
    Q_D(MImAbstractKeyArea);
    reset();

    // Workaround for NB#248227. This is not proper way to handle cancel event
    // but popup interaction requires it at the moment.
    d->ignoreTouchEventsUntilNewBegin = true;
}

bool MImAbstractKeyArea::event(QEvent *ev)
{
    Q_D(MImAbstractKeyArea);

    bool eaten = false;
    QString start, end;
    start = QString("%1|start").arg(ev->type());
    end = QString("%1|end").arg(ev->type());
    mTimestamp("MImAbstractKeyArea", start);

    if (ev->type() == QEvent::Gesture) {
        const Qt::GestureType flickGestureType = FlickGestureRecognizer::sharedGestureType();
        FlickGesture *flickGesture = static_cast<FlickGesture *>(static_cast<QGestureEvent *>(ev)->gesture(flickGestureType));

        if (flickGesture) {
            d->handleFlickGesture(flickGesture->direction(), flickGesture->state());
            eaten = true;
        }
    } else if (ev->type() == QEvent::TouchBegin
               || ev->type() == QEvent::TouchUpdate
               || ev->type() == QEvent::TouchEnd) {
        d->handleTouchEvent(static_cast<QTouchEvent*>(ev));
        eaten = true;
    }

    const bool result = eaten || MWidget::event(ev);

    mTimestamp("MImAbstractKeyArea", end);
    return result;
}

void MImAbstractKeyArea::unlockDeadKeys(MImAbstractKey *deadKey)
{
    if (!deadKey || !deadKey->isDeadKey()) {
        return;
    }

    deadKey->setSelected(false);
    deadKey->resetTouchPointCount();
    updateKeyModifiers('\0');
}

void MImAbstractKeyArea::hidePopup()
{
    Q_D(MImAbstractKeyArea);

    if (!d->popup) {
        return;
    }

    d->popup->setVisible(false);
}

void MImAbstractKeyArea::drawReactiveAreas(MReactionMap *,
                                           QGraphicsView *)
{
    // Empty default implementation. Geometries of buttons are known by derived classes.
}

void MImAbstractKeyArea::setPopup(MImAbstractPopup *popup)
{
    Q_D(MImAbstractKeyArea);

    if (popup != d->popup) {
        delete d->popup;
        d->popup = popup;

        if (d->popup) {
            d->popup->setMainArea(this);
        }
    }
}

const MImAbstractPopup *MImAbstractKeyArea::popup() const
{
    Q_D(const MImAbstractKeyArea);

    return d->popup;
}

void MImAbstractKeyArea::logTouchPoint(const QTouchEvent::TouchPoint &tp,
                                       const MImAbstractKey *key,
                                       const MImAbstractKey *lastKey) const
{
    static bool headerWritten = false;
    QTextStream &out = MKeyboardHost::instance()->touchPointLog();

    if (!headerWritten) {
        out << "time (sec.msec)\t"
            << "tp_id\t"
            << "tp_state\t"
            << "start_x\t" << "start_y\t"
            << "last_x\t" << "last_y\t"
            << "current_x\t" << "current_y\t"
            << "center_x\t" << "center_y\t"
            << "delta_x\t" << "delta_y\t"
            << "label\t"
            << "label_last\t"
            << "br_x\t" << "br_y\t" << "br_w\t" << "br_h\n";
        headerWritten = true;
    }

    out << timeStamp() << "\t"
        << tp.id() << "\t"
        << toString(tp.state()) << "\t"
        << toString(tp.startScenePos(), "\t") << "\t"
        << toString(tp.lastScenePos(), "\t") << "\t"
        << toString(tp.scenePos(), "\t") << "\t"
        << (key ? toString(mapRectToScene(key->buttonBoundingRect()).center(), "\t")
                : "n/a\t") << "\t"
        << (key ? toString(tp.scenePos() - mapRectToScene(key->buttonBoundingRect()).center(), "\t")
                : "n/a\t") << "\t"
        << (key ? key->label() : "n/a") << "\t"
        << (lastKey ? lastKey->label() : "n/a") << "\t"
        << (key ? toString(mapRectToScene(key->buttonBoundingRect()), "\t")
                : "n/a\t\t\t") << "\n";
}

void MImAbstractKeyArea::updateKeyModifiers(const QChar &accent)
{
    Q_D(MImAbstractKeyArea);

    // We currently don't allow active dead key level changing. If we did,
    // we should update activeDeadKey level before delivering its accent to
    // other keys.
    bool shift = (d->currentLevel == 1);
    modifiersChanged(shift, accent);
}

void MImAbstractKeyArea::modifiersChanged(bool,
                                          const QChar &)
{
    // Empty default implementation
}

void MImAbstractKeyArea::enableHorizontalFlick(bool enable)
{
    Q_D(MImAbstractKeyArea);

    d->allowedHorizontalFlick = enable;
}

void MImAbstractKeyArea::onThemeChangeCompleted()
{
    Q_D(MImAbstractKeyArea);

    // TODO: update all other CSS attributes that are mapped to members.
    d->switchStyleMode();
    updateKeyGeometries(size().width());
}

const MImAbstractKeyAreaStyleContainer &MImAbstractKeyArea::baseStyle() const
{
    return style();
}

void MImAbstractKeyArea::handleLongKeyPressed()
{
    Q_D(MImAbstractKeyArea);
    MImAbstractKey *const lastActiveKey = MImAbstractKey::lastActiveKey();

    if (!lastActiveKey) {
        return;
    }

    MImKeyVisitor::SpecialKeyFinder finder(MImKeyVisitor::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&finder);

    const QString accent = (finder.deadKey() ? finder.deadKey()->label()
                                           : QString());

    KeyContext keyContext(d->isUpperCase(), accent,
                          mapToScene(d->mostRecentTouchPosition));

    if (d->popup) {
        d->popup->handleLongKeyPressedOnMainArea(lastActiveKey,
                                                  keyContext);
    }

    emit longKeyPressed(lastActiveKey, keyContext);
}

void MImAbstractKeyArea::handleIdleVkb()
{
    grabGesture(FlickGestureRecognizer::sharedGestureType());
}

void MImAbstractKeyArea::reset()
{
    Q_D(MImAbstractKeyArea);

    d->primaryPressArrived = false;
    d->primaryReleaseArrived = false;

    // Handle shift before ungrab because that changes key state
    bool shiftLatchedOrLocked(false);

    foreach (const MImAbstractKey *key, keys()) {
        if (key->isShiftKey()) {
            shiftLatchedOrLocked = key->modifiers();
        }

        if ((key->isShiftKey() || key->isBackspaceKey())
            && key->state() == MImAbstractKey::Pressed) {
            emit keyCancelled(key, KeyContext());
        }
    }

    if (scene()->mouseGrabberItem() == this) {
        // Ungrab mouse explicitly since we probably used grabMouse() to get it.
        ungrabMouse();
    }

    if (d->popup) {
        d->popup->cancel();
    }

    MImKeyVisitor::SpecialKeyFinder deadFinder(MImKeyVisitor::FindDeadKey);
    MImAbstractKey::visitActiveKeys(&deadFinder);
    unlockDeadKeys(deadFinder.deadKey());

    MImKeyVisitor::KeyAreaReset keyAreaReset;
    keyAreaReset.setKeyParentItem(this);
    MImAbstractKey::visitActiveKeys(&keyAreaReset);
    modifiersChanged(shiftLatchedOrLocked);
    update();
}

QPoint MImAbstractKeyArea::correctedTouchPoint(const QPointF &scenePos) const
{
    QPointF pos = mapFromScene(scenePos);

    if (pos.y() >= baseStyle()->touchpointVerticalOffset()) {
        pos.ry() -= baseStyle()->touchpointVerticalOffset();
    }

    return pos.toPoint();
}

QRectF MImAbstractKeyArea::correctedReactionRect(const QRectF &originalRect) const
{
    QRectF rect = originalRect;
    const qreal offset = baseStyle()->touchpointVerticalOffset();

    if (rect.top() >= offset) {
        rect.setTop(rect.top() + offset);
    }
    const qreal newBottom(rect.bottom() + offset);
    if (newBottom <= size().height()) {
        rect.setBottom(newBottom);
    }

    return rect;
}

bool MImAbstractKeyArea::contains(const MImAbstractKey *key) const
{
    return (key && keys().contains(key));
}
