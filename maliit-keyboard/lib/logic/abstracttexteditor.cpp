/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2012-2013 Canonical Ltd
 *
 * Contact: maliit-discuss@lists.maliit.org
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
 *
 */

#include "abstracttexteditor.h"
#include "logic/eventhandler.h" // For signal/slot connection setup.
#include "logic/layoutupdater.h" // For signal/slot connection setup.
#include "models/wordribbon.h"
#include "models/styleattributes.h"

namespace MaliitKeyboard {
namespace Logic {

//! \class AbstractTextEditor
//! \brief Class implementing preedit edition.
//!
//! It owns a text model (which can be gotten by text() method) and a
//! word engine (word_engine()). The class has to be subclassed and
//! subclass has to provide sendPreeditString(), sendCommitString(),
//! sendKeyEvent(), invokeAction() and destructor implementations.

// The function declaration has to be in one line, because \fn is a
// single line parameter.
//! \fn void AbstractTextEditor::sendPreeditString(const QString &preedit, Model::Text::PreeditFace face, const Replacement &replacement)
//! \brief Sends preedit to application.
//! \param preedit Preedit to send.
//! \param face Face of the preedit.
//! \param replacement Struct describing replacement of the text.
//!
//! Implementations of this pure virtual method have to convert \a
//! face into specific attributes used by backend they
//! support. Preedit text has to be sent as-is. Replacement members
//! should be understood as follows: if either start or length are
//! lesser than zero or both of them are zeros then those parameters
//! should be ignored. Otherwise they mark the beginning and length of
//! a surrounding text's substring that is going to be replaced by
//! given \a preedit. Cursor position member of replacement should be
//! ignored if its value is lesser than zero. Otherwise it describes a
//! position of cursor relatively to the beginning of preedit.

//! \fn void AbstractTextEditor::sendCommitString(const QString &commit)
//! \brief Commits a string to application.
//! \param commit String to be commited in place of preedit.
//!
//! Implementations of this method should discard current preedit and
//! commit given \a commit in its place.

//! \fn void AbstractTextEditor::sendKeyEvent(const QKeyEvent &ev)
//! \brief Sends a key event to application.
//! \param ev Key event to send.
//!
//! The implementation should translate passed \a ev to values their
//! backend understands and pass this to application.

//! \fn void AbstractTextEditor::invokeAction(const QString &action, const QKeySequence &sequence)
//! \brief Invokes an action in the application
//! \param action Action to invoke
//! \param sequence Key sequence to emit when action cannot be called directly
//!
//! Application first tries to invoke a signal/slot \a action and when not available
//! it will emit the key sequence \a sequence. One would call this method for
//! example with "copy", "CTRL+C" arguments.

//! \property AbstractTextEditor::preeditEnabled
//! \brief Describes whether preedit is enabled.
//!
//! When it is \c false then everything typed by virtual keyboard is
//! immediately commited.

//! \property AbstractTextEditor::autoCorrectEnabled
//! \brief Describes whether auto correction on space is enabled.
//!
//! When it is \c true then pressing space will commit corrected word
//! if it was misspelled. Otherwise it will just commit what was in
//! preedit.

//! \fn void AbstractTextEditor::autoCapsActivated()
//! \brief Emitted when auto capitalization mode is enabled for
//! following input.

//! \fn void AbstractTextEditor::autoCorrectEnabledChanged(bool enabled)
//! \brief Emitted when auto correction setting changes.
//! \param enabled New setting.

//! \fn void AbstractTextEditor::preeditEnabledChanged(bool enabled)
//! \brief Emitted when preedit setting changes.
//! \param enabled New setting.

//! \fn void AbstractTextEditor::wordCandidatesChanged(const WordCandidateList &word_candidates)
//! \brief Emitted when new word candidates are generated.
//! \param word_candidates New word candidates.
//!
//! Note that the list might be empty as well to indicate that there
//! should be no word candidates.

//! \fn void AbstractTextEditor::keyboardClosed()
//! \brief Emitted when keyboard close is requested.

//! \class AbstractTextEditor::Replacement
//! \brief Plain struct containing beginning and length of replacement and
//! desired cursor position.
//!
//! Mostly used by sendPreeditString() implementations.

//! \fn AbstractTextEditor::Replacement::Replacement()
//! \brief Constructor.
//!
//! Constructs an instance with no replacement and no cursor position
//! change.

//! \fn AbstractTextEditor::Replacement::Replacement(int position)
//! \brief Constructor.
//! \param position New cursor position.
//!
//! Constructs an instance with no replacement and new cursor position.

//! \fn AbstractTextEditor::Replacement::Replacement(int r_start, int r_length, int position)
//! \brief Constructor.
//! \param r_start Replacement start.
//! \param r_length Replacement length.
//! \param position New cursor position.
//!
//! Constructs an instance with a replacement and new cursor position.

//! \var AbstractTextEditor::Replacement::start
//! \brief Beginning of replacement.

//! \var AbstractTextEditor::Replacement::length
//! \brief Length of replacement.

//! \var AbstractTextEditor::Replacement::cursor_position
//! \brief New cursor position relative to the beginning of preedit.

namespace {

//! \brief Checks whether given \a c is a word separator.
//! \param c Char to test.
//!
//! Other way to do checks would be using isLetterOrNumber() + some
//! other methods. But UTF is so crazy that I am not sure whether
//! other strange categories are parts of the word or not. It is
//! easier to specify punctuations and whitespaces.
inline bool isSeparator(const QChar &c)
{
    return (c.isPunct() || c.isSpace());
}

//! \brief Extracts a word boundaries at cursor position.
//! \param surrounding_text Text from which extraction will happen.
//! \param cursor_position Position of cursor within \a surrounding_text.
//! \param replacement Place where replacement data will be stored.
//!
//! \return whether surrounding text was valid (not empty).
//!
//! If cursor is placed right after the word, boundaries of this word
//! are extracted.  Otherwise if cursor is placed right before the
//! word, then no word boundaries are stored - instead invalid
//! replacement is stored. It might happen that cursor position is
//! outside the string, so \a replacement will have fixed position.
bool extractWordBoundariesAtCursor(const QString& surrounding_text,
                                   int cursor_position,
                                   AbstractTextEditor::Replacement *replacement)
{
    const int text_length(surrounding_text.length());

    if (text_length == 0) {
        return false;
    }

    // just in case - if cursor is far after last char in surrounding
    // text we place it right after last char.
    cursor_position = qBound(0, cursor_position, text_length);

    // cursor might be placed in after last char (that is to say - its
    // index might be the one of string terminator) - for simplifying
    // the algorithm below we fake it as cursor is put on delimiter:
    // "abc" - surrounding text
    //     | - cursor placement
    // "abc " - fake surrounding text
    const QString fake_surrounding_text(surrounding_text + " ");
    const QChar *const fake_data(fake_surrounding_text.constData());
    // begin is index of first char in a word
    int begin(-1);
    // end is index of a char after last char in a word.
    // -2, because -2 - (-1) = -1 and we would like to
    // have -1 as invalid length.
    int end(-2);

    for (int iter(cursor_position); iter >= 0; --iter) {
        const QChar &c(fake_data[iter]);

        if (isSeparator(c)) {
            if (iter != cursor_position) {
                break;
            }
        } else {
            begin = iter;
        }
    }

    if (begin >= 0) {
        // take note that fake_data's last QChar is always a space.
        for (int iter(cursor_position); iter <= text_length; ++iter) {
            const QChar &c(fake_data[iter]);

            end = iter;
            if (isSeparator(c)) {
                break;
            }
        }
    }

    if (replacement) {
        replacement->start = begin;
        replacement->length = end - begin;
        replacement->cursor_position = cursor_position;
    }

    return true;
}

Qt::Key toRepeatableQtKey(Key::Action action)
{
    switch(action) {
    default: return Qt::Key_unknown;
    case Key::ActionBackspace: return Qt::Key_Backspace;
    case Key::ActionLeft: return Qt::Key_Left;
    case Key::ActionUp: return Qt::Key_Up;
    case Key::ActionRight: return Qt::Key_Right;
    case Key::ActionDown: return Qt::Key_Down;
    case Key::ActionSpace: return Qt::Key_Space;
    }
}

} // unnamed namespace

//! \brief Connects event handler to editor.
//! @param event_handler The event handler, provides e.g. key events.
//! @param editor The editor, responding to (key) events.
void connectEventHandlerToTextEditor(EventHandler *event_handler,
                                     AbstractTextEditor *editor)
{
    QObject::connect(event_handler, SIGNAL(keyPressed(Key)),
                     editor,        SLOT(onKeyPressed(Key)));

    QObject::connect(event_handler, SIGNAL(keyReleased(Key)),
                     editor,        SLOT(onKeyReleased(Key)));

    QObject::connect(event_handler, SIGNAL(keyEntered(Key)),
                     editor,        SLOT(onKeyEntered(Key)));

    QObject::connect(event_handler, SIGNAL(keyExited(Key)),
                     editor,        SLOT(onKeyExited(Key)));
}

//! \brief Connects layout updater to editor.
//! @param updater The layout updater.
//! @param editor The editor.
void connectLayoutUpdaterToTextEditor(LayoutUpdater *updater,
                                      AbstractTextEditor *editor)
{
    QObject::connect(updater, SIGNAL(wordCandidateSelected(QString)),
                     editor,  SLOT(replaceAndCommitPreedit(QString)));

    QObject::connect(updater, SIGNAL(addToUserDictionary()),
                     editor,  SLOT(showUserCandidate()));

    QObject::connect(updater, SIGNAL(userCandidateSelected(QString)),
                     editor,  SLOT(addToUserDictionary(QString)));

    QObject::connect(editor,  SIGNAL(wordCandidatesChanged(WordCandidateList)),
                     updater, SLOT(onWordCandidatesChanged(WordCandidateList)));

    QObject::connect(editor,  SIGNAL(autoCapsActivated()),
                     updater, SIGNAL(autoCapsActivated()));

    QObject::connect(editor->wordEngine(), SIGNAL(enabledChanged(bool)),
                     updater,              SLOT(setWordRibbonVisible(bool)));
    }


class AbstractTextEditorPrivate
{
public:
    struct AutoRepeat {
        QTimer timer;
        Qt::Key key;
        bool key_sent;
        int delay;
        int interval;

        AutoRepeat()
            : timer()
            , key(Qt::Key_unknown)
            , key_sent(false)
            , delay(500)
            , interval(50)
        {
            timer.setSingleShot(true);
        }
    } auto_repeat;

    QScopedPointer<Model::Text> text;
    QScopedPointer<Logic::AbstractWordEngine> word_engine;
    QScopedPointer<Logic::AbstractLanguageFeatures> language_features;
    bool preedit_enabled;
    bool auto_correct_enabled;
    bool auto_caps_enabled;
    int ignore_next_cursor_position;
    QString ignore_next_surrounding_text;

    explicit AbstractTextEditorPrivate(Model::Text *new_text,
                                       Logic::AbstractWordEngine *new_word_engine,
                                       Logic::AbstractLanguageFeatures *new_language_features);
    bool valid() const;
};

AbstractTextEditorPrivate::AbstractTextEditorPrivate(Model::Text *new_text,
                                                     Logic::AbstractWordEngine *new_word_engine,
                                                     Logic::AbstractLanguageFeatures *new_language_features)
    : auto_repeat()
    , text(new_text)
    , word_engine(new_word_engine)
    , language_features(new_language_features)
    , preedit_enabled(false)
    , auto_correct_enabled(false)
    , auto_caps_enabled(false)
    , ignore_next_cursor_position(-1)
    , ignore_next_surrounding_text()
{
    (void) valid();
}

bool AbstractTextEditorPrivate::valid() const
{
    const bool is_invalid(text.isNull() || word_engine.isNull() || language_features.isNull());

    if (is_invalid) {
        qCritical() << __PRETTY_FUNCTION__
                    << "Invalid text model, or no word engine given! The text editor will not function properly.";
    }

    return (!is_invalid);
}

//! \brief Constructor.
//! \param text Text model.
//! \param word_engine Word engine.
//! \param language_features Language features.
//! \param parent Parent of this instance or \c NULL if none is needed.
//!
//! Takes ownership of \a text, \a word_engine and \a language_features.
AbstractTextEditor::AbstractTextEditor(Model::Text *text,
                                       Logic::AbstractWordEngine *word_engine,
                                       Logic::AbstractLanguageFeatures *language_features,
                                       QObject *parent)
    : QObject(parent)
    , d_ptr(new AbstractTextEditorPrivate(text, word_engine, language_features))
{
    connect(&d_ptr->auto_repeat.timer, SIGNAL(timeout()),
            this,                      SLOT(autoRepeatKey()));

    connect(word_engine, SIGNAL(candidatesChanged(WordCandidateList)),
            this,        SIGNAL(wordCandidatesChanged(WordCandidateList)));
}

//! \brief Destructor.
AbstractTextEditor::~AbstractTextEditor()
{}

//! \brief Gets editor's text model.
Model::Text * AbstractTextEditor::text() const
{
    Q_D(const AbstractTextEditor);
    return d->text.data();
}

//! \brief Gets editor's word engine.
Logic::AbstractWordEngine * AbstractTextEditor::wordEngine() const
{
    Q_D(const AbstractTextEditor);
    return d->word_engine.data();
}

//! \brief Sets auto-repeat behavior for pressed keys.
void AbstractTextEditor::setAutoRepeatBehaviour(int auto_repeat_delay,
                                                int auto_repeat_interval)
{
    Q_D(AbstractTextEditor);

    d->auto_repeat.delay = auto_repeat_delay;
    d->auto_repeat.interval = auto_repeat_interval;
}

//! \brief Reacts to key press.
//! \param key Pressed key.
//!
//! Checks whether auto-repeatable key was was pressed, in which case preedit
//! gets committed. Backspace cancels primary candidate, too (relevant for
//! auto-correct).
void AbstractTextEditor::onKeyPressed(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (!d->valid()) {
        return;
    }

    d->auto_repeat.key = toRepeatableQtKey(key.action());
    if (d->auto_repeat.key != Qt::Key_unknown) {
        commitPreedit();
        d->auto_repeat.timer.start(d->auto_repeat.delay);
        d->auto_repeat.key_sent = true;
    }

    if (key.action() == Key::ActionBackspace) {
        if (d->auto_correct_enabled && !d->text->primaryCandidate().isEmpty()) {
            d->text->setPrimaryCandidate(QString());
            d->auto_repeat.key_sent = true;
        } else {
            d->auto_repeat.key_sent = false;
        }
    }
}

//! \brief Reacts to key release.
//! \param key Released key.
//!
//! If common key is pressed then it is appended to preedit.  If
//! backspace was pressed then preedit is commited and a character
//! before cursor is removed. If space is pressed then primary
//! candidate is applied if enabled. In other cases standard behaviour
//! applies.
void AbstractTextEditor::onKeyReleased(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (!d->valid()) {
        return;
    }

    const QString &text(key.label().text());
    Qt::Key event_key = Qt::Key_unknown;

    switch(key.action()) {
    case Key::ActionInsert:
        d->text->appendToPreedit(text);
#ifdef DISABLE_PREEDIT
        commitPreedit();
#else
        sendPreeditString(d->text->preedit(), d->text->preeditFace());
#endif

        // computeCandidates can change preedit face, so needs to happen
        // before sending preedit:
        if (d->preedit_enabled) {
            d->word_engine->computeCandidates(d->text.data());
        }

        sendPreeditString(d->text->preedit(), d->text->preeditFace(),
                          Replacement(d->text->cursorPosition()));

        if (!d->preedit_enabled) {
            commitPreedit();
        }

        break;

    case Key::ActionBackspace: {
        commitPreedit();

        if (!d->auto_repeat.key_sent) {
            event_key = Qt::Key_Backspace;
        }

        d->auto_repeat.timer.stop();
     } break;

    case Key::ActionSpace: {
        const bool auto_caps_activated = d->language_features->activateAutoCaps(d->text->preedit());
        const bool replace_preedit = d->auto_correct_enabled && !d->text->primaryCandidate().isEmpty();

        if (replace_preedit) {
            const QString &appendix = d->language_features->appendixForReplacedPreedit(d->text->preedit());
            d->text->setPreedit(d->text->primaryCandidate());
            d->text->appendToPreedit(appendix);
        } else {
            d->text->appendToPreedit(" ");
        }
        commitPreedit();

        if (auto_caps_activated && d->auto_caps_enabled) {
            Q_EMIT autoCapsActivated();
        }

        d->auto_repeat.timer.stop();
    } break;

    case Key::ActionReturn:
        event_key = Qt::Key_Return;
        break;

    case Key::ActionClose:
        Q_EMIT keyboardClosed();
        break;

    case Key::ActionCancel:
        replacePreedit("");
        Q_EMIT keyboardClosed();
        break;

    case Key::ActionLeft:
        event_key = Qt::Key_Left;
        commitPreedit();
        d->auto_repeat.timer.stop();
        break;

    case Key::ActionUp:
        event_key = Qt::Key_Up;
        commitPreedit();
        d->auto_repeat.timer.stop();
        break;

    case Key::ActionRight:
        event_key = Qt::Key_Right;
        commitPreedit();
        d->auto_repeat.timer.stop();
        break;

    case Key::ActionDown:
        event_key = Qt::Key_Down;
        commitPreedit();
        d->auto_repeat.timer.stop();
        break;

    case Key::ActionLeftLayout:
        Q_EMIT leftLayoutSelected();
        break;

    case Key::ActionRightLayout:
        Q_EMIT rightLayoutSelected();
        break;

    case Key::ActionCommand:
        invokeAction(text, key.commandSequence());
        break;

    default:
        break;
    }

    if (event_key != Qt::Key_unknown) {
        commitPreedit();
        sendKeyEvent(KeyStatePressed, event_key, Qt::NoModifier);
    }
}

//! \brief Reacts to sliding into a key.
//! \param key Slid in key.
//!
//! For now it only set backspace repeat timer if we slide into
//! backspace.
void AbstractTextEditor::onKeyEntered(const Key &key)
{
    Q_D(AbstractTextEditor);

    d->auto_repeat.key = toRepeatableQtKey(key.action());
    if (d->auto_repeat.key != Qt::Key_unknown) {
        d->auto_repeat.key_sent = false;
        d->auto_repeat.timer.start(d->auto_repeat.delay);
    }
}

//! \brief Reacts to sliding out of a key.
//! \param key Slid out key.
//!
//! Stops auto-repeat when sliding out from repeatable keys.
void AbstractTextEditor::onKeyExited(const Key &key)
{
    Q_D(AbstractTextEditor);

    if (toRepeatableQtKey(key.action()) != Qt::Key_unknown) {
        d->auto_repeat.timer.stop();
    }
}

//! \brief Replaces current preedit with given replacement
//! \param replacement New preedit.
void AbstractTextEditor::replacePreedit(const QString &replacement)
{
    Q_D(AbstractTextEditor);

    if (!d->valid()) {
        return;
    }

    d->text->setPreedit(replacement);
    // computeCandidates can change preedit face, so needs to happen
    // before sending preedit:
    d->word_engine->computeCandidates(d->text.data());
    sendPreeditString(d->text->preedit(), d->text->preeditFace());
}

//! \brief Replaces current preedit with given replacement and then
//! commits it.
//! \param replacement New preedit string to commit.
void AbstractTextEditor::replaceAndCommitPreedit(const QString &replacement)
{
    Q_D(AbstractTextEditor);

    if (!d->valid()) {
        return;
    }

    const bool auto_caps_activated = d->language_features->activateAutoCaps(d->text->preedit());
    const QString &appendix(d->language_features->appendixForReplacedPreedit(d->text->preedit()));
    d->text->setPreedit(replacement);
    d->text->appendToPreedit(appendix);
    commitPreedit();

    if (auto_caps_activated && d->auto_caps_enabled) {
        Q_EMIT autoCapsActivated();
    }
}

//! \brief Clears preedit. Does *not* update preedit in application, use replacePreedit instead.
void AbstractTextEditor::clearPreedit()
{
    Q_D(AbstractTextEditor);

    if (!d->valid()) {
        return;
    }

    d->text->setPreedit("");
    d->word_engine->computeCandidates(d->text.data());
}

//! \brief Returns whether preedit functionality is enabled.
//! \sa preeditEnabled
bool AbstractTextEditor::isPreeditEnabled() const
{
    Q_D(const AbstractTextEditor);
    return d->preedit_enabled;
}

//! \brief Sets whether enable preedit functionality.
//! \param enabled \c true to enable preedit functionality.
//! \sa preeditEnabled
void AbstractTextEditor::setPreeditEnabled(bool enabled)
{
    Q_D(AbstractTextEditor);

    if (d->preedit_enabled != enabled) {
        d->preedit_enabled = enabled;
        Q_EMIT preeditEnabledChanged(d->preedit_enabled);
    }
}

//! \brief Returns whether auto-correct functionality is enabled.
//! \sa autoCorrectEnabled
bool AbstractTextEditor::isAutoCorrectEnabled() const
{
    Q_D(const AbstractTextEditor);
    return d->auto_correct_enabled;
}

//! \brief Sets whether enable the auto-correct functionality.
//! \param enabled \c true to enable auto-correct functionality.
//! \sa autoCorrectEnabled
void AbstractTextEditor::setAutoCorrectEnabled(bool enabled)
{
    Q_D(AbstractTextEditor);

    if (d->auto_correct_enabled != enabled) {
        d->auto_correct_enabled = enabled;
        Q_EMIT autoCorrectEnabledChanged(d->auto_correct_enabled);
    }
}

bool AbstractTextEditor::isAutoCapsEnabled() const
{
    Q_D(const AbstractTextEditor);
    return d->auto_caps_enabled;
}

void AbstractTextEditor::setAutoCapsEnabled(bool enabled)
{
    Q_D(AbstractTextEditor);

    if (d->auto_caps_enabled != enabled) {
        d->auto_caps_enabled = enabled;
        Q_EMIT autoCapsEnabledChanged(d->auto_caps_enabled);
    }
}

//! \brief Commits current preedit.
void AbstractTextEditor::commitPreedit()
{
    Q_D(AbstractTextEditor);

    if (!d->valid() || d->text->preedit().isEmpty()) {
        return;
    }

    sendCommitString(d->text->preedit());
    d->text->commitPreedit();
    d->word_engine->clearCandidates();
}

// TODO: this implementation does not take into account following features:
// 1) preedit string
//      if there is preedit then first call to autoRepeatKey should clean it completely
//      and following calls should remove remaining text character by character
// 2) multitouch
//      it is not completely clean how to handle multitouch for backspace,
//      but we can follow the strategy from meego-keyboard - release pressed
//      key when user press another one at the same time. Then we do not need to
//      change anything in this method
//! \brief Sends key event and sets auto repeat timer.
void AbstractTextEditor::autoRepeatKey()
{
    Q_D(AbstractTextEditor);

    commitPreedit();

    if (d->auto_repeat.key == Qt::Key_Space) {
        sendCommitString(" ");
    } else {
        sendKeyEvent(KeyStatePressed, d->auto_repeat.key, Qt::NoModifier);
    }

    d->auto_repeat.key_sent = true;
    d->auto_repeat.timer.start(d->auto_repeat.interval);
}

//! \brief Emits wordCandidatesChanged() signal with current preedit
//! as a candidate.
void AbstractTextEditor::showUserCandidate()
{
    Q_D(AbstractTextEditor);

    if (d->text->preedit().isEmpty()) {
        return;
    }

    WordCandidateList candidates;
    WordCandidate candidate(WordCandidate::SourceUser, d->text->preedit());

    candidates << candidate;

    Q_EMIT wordCandidatesChanged(candidates);
}

//! \brief Adds \a word to user dictionary.
//! \param word Word to be added.
void AbstractTextEditor::addToUserDictionary(const QString &word)
{
    Q_D(AbstractTextEditor);

    d->word_engine->addToUserDictionary(word);
    d->text->setPrimaryCandidate(word);

    Q_EMIT wordCandidatesChanged(WordCandidateList());
}

//! \brief Sends preedit string to application with no replacement.
//! \param preedit Preedit to send.
//! \param face Face of the preedit.
void AbstractTextEditor::sendPreeditString(const QString &preedit,
                                           Model::Text::PreeditFace face)
{
    sendPreeditString(preedit, face, Replacement());
}

//! \brief Reacts to cursor position change in application's text
//! field.
//! \param cursor_position new cursor position
//! \param surrounding_text surrounding text of a preedit
//!
//! Extract words with the cursor inside and replaces it with a preedit.
//! This is called preedit activation.
void AbstractTextEditor::onCursorPositionChanged(int cursor_position,
                                                 const QString &surrounding_text)
{
    Q_D(AbstractTextEditor);
    Replacement r;

    if (!extractWordBoundariesAtCursor(surrounding_text, cursor_position, &r)) {
        return;
    }

    if (r.start < 0 || r.length < 0) {
        if (d->ignore_next_surrounding_text == surrounding_text &&
            d->ignore_next_cursor_position == cursor_position) {
            d->ignore_next_surrounding_text.clear();
            d->ignore_next_cursor_position = -1;
        } else {
            d->text->setPreedit("");
            d->text->setCursorPosition(0);
        }
    } else {
        const int cursor_pos_relative_word_begin(r.start - r.cursor_position);
        const int word_begin_relative_cursor_pos(r.cursor_position - r.start);
        const QString word(surrounding_text.mid(r.start, r.length));
        Replacement word_r(cursor_pos_relative_word_begin, r.length,
                           word_begin_relative_cursor_pos);

        d->text->setPreedit(word, word_begin_relative_cursor_pos);
        // computeCandidates can change preedit face, so needs to happen
        // before sending preedit:
        d->word_engine->computeCandidates(d->text.data());
        sendPreeditString(d->text->preedit(), d->text->preeditFace(), word_r);
        // Qt is going to send us an event with cursor position places
        // at the beginning of replaced word and surrounding text
        // without the replaced word. We want to ignore it.
        d->ignore_next_cursor_position = r.start;
        d->ignore_next_surrounding_text = QString(surrounding_text).remove(r.start, r.length);
    }
}

}} // namespace Logic, MaliitKeyboard
