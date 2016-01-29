/*
 * This file is part of Maliit Plugins
 *
 * Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies). All rights reserved.
 *
 * Contact: Mohammad Anwari <Mohammad.Anwari@nokia.com>
 * Copyright (C) 2012-2013 Canonical Ltd
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

#include "wordengine.h"
#include "spellchecker.h"

#ifdef HAVE_PRESAGE
#include <presage.h>
#endif

namespace MaliitKeyboard {
namespace Logic {

namespace {

void appendToCandidates(WordCandidateList *candidates,
                        WordCandidate::Source source,
                        const QString &candidate,
                        bool is_preedit_capitalized)
{
    if (!candidates) {
        return;
    }

    QString changed_candidate(candidate);

    if (!changed_candidate.isEmpty() && is_preedit_capitalized) {
        changed_candidate[0] = changed_candidate.at(0).toUpper();
    }

    WordCandidate word_candidate(source, changed_candidate);

    if (!candidates->contains(word_candidate)) {
        candidates->append(word_candidate);
    }
}

} // namespace

//! \class WordEngine
//! \brief Provides error correction (based on Hunspell) and word
//! prediction (based on Presage).

//! \internal
#ifdef HAVE_PRESAGE
class CandidatesCallback
    : public PresageCallback
{
private:
    const std::string& m_past_context;
    const std::string m_empty;

public:
    explicit CandidatesCallback(const std::string& past_context);

    std::string get_past_stream() const;
    std::string get_future_stream() const;
};

CandidatesCallback::CandidatesCallback(const std::string &past_context)
    : m_past_context(past_context)
    , m_empty()
{}

std::string CandidatesCallback::get_past_stream() const
{
    return m_past_context;
}

std::string CandidatesCallback::get_future_stream() const
{
    return m_empty;
}
#endif
//! \internal_end

class WordEnginePrivate
{
public:
    SpellChecker spell_checker;
#ifdef HAVE_PRESAGE
    std::string candidates_context;
    CandidatesCallback presage_candidates;
    Presage presage;
#endif

    explicit WordEnginePrivate();
};

WordEnginePrivate::WordEnginePrivate()
    : spell_checker()
#ifdef HAVE_PRESAGE
    , candidates_context()
    , presage_candidates(CandidatesCallback(candidates_context))
    , presage(&presage_candidates)
#endif
{
    // FIXME: Check whether spellchecker is enabled, and update enabled flag!
#ifdef HAVE_PRESAGE
    presage.config("Presage.Selector.SUGGESTIONS", "6");
    presage.config("Presage.Selector.REPEAT_SUGGESTIONS", "yes");
#endif
}


//! \brief Constructor.
//! \param parent The owner of this instance. Can be 0, in case QObject
//!               ownership is not required.
WordEngine::WordEngine(QObject *parent)
    : AbstractWordEngine(parent)
    , d_ptr(new WordEnginePrivate)
{}

//! \brief Destructor.
WordEngine::~WordEngine()
{}


void WordEngine::setEnabled(bool enabled)
{
 // Don't allow to enable word engine if no backends are available:
#if defined(HAVE_PRESAGE) || defined(HAVE_HUNSPELL)
#else
    if (enabled) {
        qWarning() << __PRETTY_FUNCTION__
                   << "No backend available, cannot enable word engine!";
    }

    enabled = false;
#endif
    AbstractWordEngine::setEnabled(enabled);
}


WordCandidateList WordEngine::fetchCandidates(Model::Text *text)
{
    WordCandidateList candidates;
 
#ifdef DISABLE_PREEDIT
    Q_UNUSED(text)
    return candidates;
#else
    Q_D(WordEngine);

    const QString &preedit(text->preedit());
    const bool is_preedit_capitalized(!preedit.isEmpty() && preedit.at(0).isUpper());

#ifdef HAVE_PRESAGE
    const QString &context = (text->surroundingLeft() + preedit);
    d->candidates_context = context.toStdString();
    const std::vector<std::string> predictions = d->presage.predict();

    // TODO: Fine-tune presage behaviour to also perform error correction, not just word prediction.
    if (!context.isEmpty()) {
        // FIXME: max_candidates should come from style, too:
        const static unsigned int max_candidates = 7;
        const int count(qMin<int>(predictions.size(), max_candidates));
        for (int index = 0; index < count; ++index) {
            appendToCandidates(&candidates, WordCandidate::SourcePrediction, QString::fromStdString(predictions.at(index)),
                               is_preedit_capitalized);
        }
    }
#endif

    const bool correct_spelling(d->spell_checker.spell(preedit));

    if (candidates.isEmpty() and !correct_spelling) {
        Q_FOREACH(const QString &correction, d->spell_checker.suggest(preedit, 5)) {
            appendToCandidates(&candidates, WordCandidate::SourceSpellChecking, correction, is_preedit_capitalized);
        }
    }

    text->setPreeditFace(candidates.isEmpty() ? (correct_spelling ? Model::Text::PreeditDefault
                                                                  : Model::Text::PreeditNoCandidates)
                                              : Model::Text::PreeditActive);

    text->setPrimaryCandidate(candidates.isEmpty() ? QString()
                                                   : candidates.first().label().text());


    return candidates;
#endif
}

void WordEngine::addToUserDictionary(const QString &word)
{
    Q_D(WordEngine);

    d->spell_checker.addToUserWordlist(word);
}

}} // namespace Logic, MaliitKeyboard
