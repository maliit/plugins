/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
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
 *
 */

//! \class MaliitKeyboard::Logic::SpellChecker
//! Checks spelling and suggest words. Currently Spellchecker is
//! implemented by using Hunspell.
#include "spellchecker.h"

#ifdef HAVE_HUNSPELL
#include "hunspell/hunspell.hxx"
#else
class Hunspell
{
public:
    Hunspell(const char *, const char *, const char * = NULL) : encoding("UTF-8") {}
    int add_dic (const char *, const char * = NULL) { return 0; }
    char *get_dic_encoding() { return encoding.data(); }
    int spell(const char *, int * = NULL, char ** = NULL) { return 1; }
    int suggest(char *** lst, const char *) { if (lst) { *lst = NULL; } return 0; }
    void free_list(char ***, int) {}
    int add(const char *) { return 0; }
private:
    // Using QByteArray here instead of just returning "UTF-8" in get_dic_encoding
    // to avoid a following warning:
    // warning: deprecated conversion from string constant to ‘char*’ [-Wwrite-strings]
    QByteArray encoding;
};
#endif

#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QStringList>
#include <QDebug>

namespace MaliitKeyboard {
namespace Logic {

struct SpellCheckerPrivate
{
    Hunspell hunspell;
    QTextCodec *codec;
    bool enabled;
    QSet<QString> ignored_words;

    SpellCheckerPrivate(const QString &dictionary_path,
                        const QString &user_dictionary);
};

SpellCheckerPrivate::SpellCheckerPrivate(const QString &dictionary_path,
                                         const QString &user_dictionary)
    // XXX: toUtf8? toLatin1? toAscii? toLocal8Bit?
    : hunspell((dictionary_path + ".aff").toUtf8().constData(),
               (dictionary_path + ".dic").toUtf8().constData())
    , codec(QTextCodec::codecForName(hunspell.get_dic_encoding()))
    , enabled(false)
    , ignored_words()
{
    if (not user_dictionary.isEmpty() and QFile::exists(user_dictionary)) {
        // Non-zero status means some error.
        if (hunspell.add_dic(user_dictionary.toUtf8().constData())) {
            qWarning() << __PRETTY_FUNCTION__ << ": Failed to add user directory (" << user_directory << ").";
        }
    }
    if (not codec) {
        qWarning () << __PRETTY_FUNCTION__ << ":Could not find codec for" << hunspell.get_dic_encoding() << "- turning off spellchecking and suggesting.";
        return;
    }
    enabled = true;
}

//! Creates a spellchecker using passed dictionary and extensible user dictionary.
//!
//! \param dictionary_path path to dictionary
//! \param user_dictionary path to user dictionary
SpellChecker::SpellChecker(const QString &dictionary_path,
                           const QString &user_dictionary)
    : d_ptr(new SpellCheckerPrivate(dictionary_path,
                                    user_dictionary))
{}

//! \brief Checks spelling of given \a word.
//!
//! Ignored words are treated as having correct spelling.
//!
//! \param word word to check for spelling.
//!
//! \return \c true if the word has correct spelling (or is ignored), otherwise \c false.
bool SpellChecker::spell(const QString &word)
{
    Q_D(SpellChecker);

    if (not d->enabled) {
        return true;
    }

    if (d->ignored_words.contains(word)) {
        return true;
    }

    return d->hunspell.spell(d->codec->fromUnicode(word));
}

//! \brief Gives some suggestions for given \a word.
//!
//! \param word Base for suggestions.
//! \param limit Suggestion count limit (-1 for no limits).
//!
//! \return a list of suggestions.
QStringList SpellChecker::suggest(const QString &word,
                                  int limit)
{
    Q_D(SpellChecker);

    if (not d->enabled) {
        return QStringList();
    }

    char** suggestions = NULL;
    const int suggestions_count = d->hunspell.suggest(&suggestions, d->codec->fromUnicode(word));

    // Less than zero means some error.
    if (suggestions_count < 0) {
        qWarning() << __PRETTY_FUNCTION__ << ": Failed to get suggestions for" << word << ".";
        return QStringList();
    }

    QStringList result;
    const int final_limit((limit < 0) ? suggestions_count : qMin(limit, suggestions_count));

    for (int index(0); index < final_limit; ++index) {
        result << d->codec->toUnicode(suggestions[index]);
    }
    d->hunspell.free_list(&suggestions, suggestions_count);
    return result;
}

SpellChecker::~SpellChecker()
{}

//! Marks given \a word as ignored.
//!
//! Ignored words are not checked for spelling.
//!
//! \param word word to ignore.
void SpellChecker::ignoreWord(const QString &word)
{
    Q_D(SpellChecker);

    if (not d->enabled) {
        return;
    }

    d->ignored_words.insert(word);
}

//! Adds given \a word to user dictionary.
//!
//! Passed word will be used for spellchecking and suggesting.
//!
//! \param word word to be added to user dictionary.
void SpellChecker::addToUserWordlist(const QString &word)
{
    Q_D(SpellChecker);

    if (not d->enabled) {
        return;
    }

    // Non-zero return value means some error.
    if (d->hunspell.add(d->codec->fromUnicode(word))) {
        qWarning() << __PRETTY_FUNCTION__ << ": Failed to add '" << word << "' to user dictionary.";
    }
}

}} // namespace Logic, MaliitKeyboard
