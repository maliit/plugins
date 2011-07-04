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

#ifdef HAVE_MEEGOTOUCH

#include "wordribbondialogmodel.h"
#include "wordribbon.h"

#include <QDebug>
#include <QtCore>

WordRibbonDialogModel::WordRibbonDialogModel(QObject *parent) :
    QAbstractListModel(parent),
    calcCandidatesBar(0),
    currentRowWidth(0)
{
    setObjectName("WordRibbonDialogModelObj");
    calcCandidatesBar = new WordRibbon(WordRibbon::DialogStyleMode);
}

WordRibbonDialogModel::~WordRibbonDialogModel()
{
    if(calcCandidatesBar)
        delete calcCandidatesBar;
}

int WordRibbonDialogModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    if (cache.contains(this->currentRowWidth))
        return cache.object(this->currentRowWidth)->count();
    else
        return 0;
}

QVariant WordRibbonDialogModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole) {
        if (!cache.contains(this->currentRowWidth) ||
            cache.object(this->currentRowWidth)->count() <= index.row()) {
            return QVariant();
        }
        QStringList rowData = cache.object(this->currentRowWidth)->at(index.row());
        return QVariant(rowData);
    }
    return QVariant();
}

void WordRibbonDialogModel::fillCandidates(int rowWidth)
{
    ListSeparate *listSeparate = new ListSeparate;

    int index = 0;
    int consume = 0;
    int i = 0;
    int tmpConsume = 0;

    do {
       tmpConsume = calcCandidatesBar->capacity(rowWidth, listOrginal, index);
        if(tmpConsume <= 0){
            delete listSeparate;
            return;
        }
        consume += tmpConsume;
        listSeparate->append(QStringList(listOrginal.mid(index, tmpConsume)));
        index = consume;
        ++i;
    } while (consume < listOrginal.count());
    cache.insert(rowWidth, listSeparate);
    currentRowWidth = rowWidth;
    reset();
}

void WordRibbonDialogModel::setCandidates(const QStringList &list, int rowWidth)
{
    if(rowWidth <= 0 || list.empty()) {
        if (rowWidth <= 0)
            qWarning() << Q_FUNC_INFO << "rowWidth <= 0";
        if (list.empty())
            qWarning() << Q_FUNC_INFO << "list.empty()";
        listOrginal.clear();
        cache.clear();
        currentRowWidth = 0;
        return;
    }

    if (listOrginal != list) {
        listOrginal = list;
        cache.clear();
        currentRowWidth = 0;
        fillCandidates(rowWidth);
        return;
    }
    else {
        if (cache.contains(rowWidth)) {
            if(currentRowWidth != rowWidth) {
                currentRowWidth = rowWidth;
                reset();
            }
            return;
        }
        else {
            fillCandidates(rowWidth);
            return;
        }
    }
}

QStringList WordRibbonDialogModel::candidates()
{
    return listOrginal;
}

#endif // HAVE_MEEGOTOUCH
