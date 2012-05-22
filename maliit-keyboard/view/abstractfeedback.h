/* LICENSE HEADER */

#ifndef MALIIT_KEYBOARD_ABSTRACTFEEDBACK_H
#define MALIIT_KEYBOARD_ABSTRACTFEEDBACK_H

#include <QtCore>

namespace MaliitKeyboard {

class AbstractFeedback
    : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(AbstractFeedback)

public:
    explicit AbstractFeedback(QObject *parent = 0);
    virtual ~AbstractFeedback() = 0;

    Q_SLOT void onKeyPressed();
    Q_SLOT void onKeyReleased();
    Q_SLOT void onLayoutChanged();
    Q_SLOT void onKeyboardHidden();
    // TODO: add slot for key-to-key slide feedback
    // TODO: add slot for keyboard show feedback

private:
    virtual void playPressFeedback() = 0;
    virtual void playReleaseFeedback() = 0;
    virtual void playLayoutChangeFeedback() = 0;
    virtual void playKeyboardHideFeedback() = 0;
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_ABSTRACTFEEDBACK_H
