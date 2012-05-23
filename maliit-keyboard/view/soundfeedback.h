/* LICENSE HEADER */

#ifndef MALIIT_KEYBOARD_SOUNDFEEDBACK_H
#define MALIIT_KEYBOARD_SOUNDFEEDBACK_H

#include "abstractfeedback.h"

namespace MaliitKeyboard {

class SoundFeedbackPrivate;

class SoundFeedback
    : public AbstractFeedback
{
    Q_OBJECT
    Q_DISABLE_COPY(SoundFeedback)
    Q_DECLARE_PRIVATE(SoundFeedback)

public:
    explicit SoundFeedback(QObject *parent = 0);
    virtual ~SoundFeedback();

private:
    const QScopedPointer<SoundFeedbackPrivate> d_ptr;

    virtual void playPressFeedback();
    virtual void playReleaseFeedback();
    virtual void playLayoutChangeFeedback();
    virtual void playKeyboardHideFeedback();
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_SOUNDFEEDBACK_H
