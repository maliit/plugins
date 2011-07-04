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


#ifndef WORDRIBBONDIALOG_H
#define WORDRIBBONDIALOG_H

#ifdef HAVE_MEEGOTOUCH
#include <MDialog>
#include <MList>
#include <MAbstractCellCreator>

#include "wordribbon.h"
#include "reactionmappaintable.h"

class WordRibbonDialogSwitcher;
class WordRibbonDialogModel;

class WordRibbonCreator : public QObject, public MCellCreator
{
    Q_OBJECT
private:
    QSizeF size;
public:
    static int totalCellcount;
    WordRibbonCreator(QObject *parent=0);
    virtual QSizeF cellSize() const;
    virtual MWidget *createCell(const QModelIndex &index, MWidgetRecycler &recycler) const;
    virtual void updateCell(const QModelIndex &index, MWidget *cell) const;

signals:
    void candidateClicked(const QString & selectedWord);
};

class WordRibbonDialog : public MDialog
{
    Q_OBJECT
    static const int DefaultCandidateTextbuttonCount = 20;
public:
    WordRibbonDialog();
    virtual ~WordRibbonDialog();

    void paintReactionMap(MReactionMap *reactionMap, QGraphicsView *view);

public slots:
    void setCandidates(QStringList candidates, QString titleStr = "");
    void finalizeOrientationChange(); 

signals:
    void candidateClicked(const QString & selectedWord, int wordIndex);

private:
    WordRibbonDialogModel *dataModel;
    //M::Orientation orientation;
    QString titleString;
    MList   *listWidget;
    QStringList candidatesList;

private slots:
    void onRibbonClick(const QString & selectedWord);

private:
    friend class Ut_WordRibbonDialog;
};

#endif // HAVE_MEEGOTOUCH
#endif // WORDRIBBONDIALOG_H
