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


#ifndef MIMWORDLIST_H
#define MIMWORDLIST_H

#ifdef HAVE_MEEGOTOUCH
#include <MDialog>
#else
#include <QGraphicsWidget>
#include <QGraphicsView>
#endif

class QGraphicsLinearLayout;
class MSeparator;
class MImWordListItem;
class MReactionMap;

/*!
 * \brief MIMWordList is used for word list dialog.
 *
 * MImWordList shows a dialog which list the suggested candidates.
 */
class MImWordList
#ifdef HAVE_MEEGOTOUCH
    : public MDialog
#else
    : public QGraphicsWidget
#endif
{
    Q_OBJECT
    friend class Ut_MImWordList;
    friend class Ut_MImCorrectionCandidateWidget;

public:
    enum {
        MaxCandidateCount = 5
    };

    //! Constructor
    explicit MImWordList();

    //! Destructor
    virtual ~MImWordList();

    /*!
     * \brief Sets suggestion candidates.
     *
     * The first candidate in the list must be the word the user typed in.
     * typedWordIsInDictionary indicates if this word is already present in a dictionary.
     */
    void setCandidates(const QStringList &candidates, bool typedWordIsInDictionary);

    /*!
     * \brief Returns the suggestion candidates shown in the widget.
     *
     * The list is limited to MaxCandidateCount items.
     */
    QStringList candidates() const;

    /*!
     * \brief Draw its reactive areas onto the reaction map
     */
    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

#ifndef HAVE_MEEGOTOUCH
    void appear() {show();}
    void disappear() {disappear();}
#endif

signals:
    /*!
     * \brief This signal is emitted when clicking on a candidate.
     */
    void candidateClicked(const QString &);

private slots:

    void select();

private:
    QStringList mCandidates;
    QGraphicsLinearLayout *mainLayout;
    MImWordListItem *candidateItems[MaxCandidateCount];
    MImWordListItem *addToDictionaryItem;
    MSeparator *addToDictionarySeparator;
};

#endif
