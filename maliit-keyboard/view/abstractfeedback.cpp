/* LICENSE HEADER */

#include "abstractfeedback.h"

namespace MaliitKeyboard
{

AbstractFeedback::AbstractFeedback(QObject *parent)
    : QObject(parent)
{}

AbstractFeedback::~AbstractFeedback()
{}

void AbstractFeedback::onKeyPressed()
{
    playPressFeedback();
}

void AbstractFeedback::onKeyReleased()
{
    playReleaseFeedback();
}

void AbstractFeedback::onLayoutChanged()
{
    playLayoutChangeFeedback();
}

void AbstractFeedback::onKeyboardHidden()
{
    playKeyboardHideFeedback();
}

} // namespace MaliitKeyboard
