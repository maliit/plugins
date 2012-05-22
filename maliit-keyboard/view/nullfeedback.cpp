/* LICENSE HEADER */

#include "nullfeedback.h"

namespace MaliitKeyboard {

NullFeedback::NullFeedback(QObject *parent)
    : AbstractFeedback(parent)
{}

NullFeedback::~NullFeedback()
{}

void NullFeedback::playPressFeedback()
{}

void NullFeedback::playReleaseFeedback()
{}

void NullFeedback::playKeyboardHideFeedback()
{}

void NullFeedback::playLayoutChangeFeedback()
{}

} // namespace MaliitKeyboard
