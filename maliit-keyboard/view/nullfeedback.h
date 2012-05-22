/* LICENSE HEADER */

#ifndef MALIIT_KEYBOARD_NULLFEEDBACK_H
#define MALIIT_KEYBOARD_NULLFEEDBACK_H

#include "abstractfeedback.h"

namespace MaliitKeyboard {

class NullFeedback
    : public AbstractFeedback
{
    Q_OBJECT
    Q_DISABLE_COPY(NullFeedback)

public:
    explicit NullFeedback(QObject *parent = 0);
    virtual ~NullFeedback();

private:
    virtual void playPressFeedback();
    virtual void playReleaseFeedback();
    virtual void playLayoutChangeFeedback();
    virtual void playKeyboardHideFeedback();
};

} // namespace MaliitKeyboard

#endif // MALIIT_KEYBOARD_NULLFEEDBACK_H
