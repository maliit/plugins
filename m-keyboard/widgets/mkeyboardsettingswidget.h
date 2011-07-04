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

#ifndef MKEYBOARDSETTINGSWIDGET_H
#define MKEYBOARDSETTINGSWIDGET_H

#include "mkeyboardsettings.h"

#include <QObject>
#include <QWeakPointer>

class QGraphicsItem;
class QGraphicsLayoutItem;

#ifdef HAVE_MEEGOTOUCH
class MBasicListItem;
class MButton;
class MDialog;
class MList;
class QModelIndex;
class MGridLayoutPolicy;
class MLinearLayoutPolicy;
class MLabel;
class MContainer;
class MKeyboardSettingsListItem;
#else
#include "mwidget-wrapper.h"
#endif

class MKeyboardSettingsWidget : public MWidget
{
    Q_OBJECT
public:
    MKeyboardSettingsWidget(MKeyboardSettings *, QGraphicsItem *parent = 0);
    virtual ~MKeyboardSettingsWidget();

protected:
    //! reimp
    virtual void retranslateUi();
    //! reimp_end

private slots:
    void showChineseTransliterationOptions();
    void updateTitle();
    void setErrorCorrectionState(bool enabled);
    void syncErrorCorrectionState();
    void setCorrectionSpaceState(bool enabled);
    void syncCorrectionSpaceState();

    void setFuzzyState(bool enabled);
    void syncFuzzyState();
    void setWordPredictionState(bool enabled);
    void syncWordPredictionState();
    void updateChineseSettingPanel();
    void selectChineseTransliteration();
    void updateChineseTransliterationModel();

private:
    void buildUi();
    void addItem(QGraphicsLayoutItem *item, int row, int column);
    void removeItem(QGraphicsLayoutItem *item);
    void connectSlots();
    void createChineseTransliterationModel();

#ifdef HAVE_MEEGOTOUCH
    MKeyboardSettings *settingsObject;
    MGridLayoutPolicy *landscapePolicy;
    MLinearLayoutPolicy *portraitPolicy;
    MButton *errorCorrectionSwitch;
    MLabel *errorCorrectionTitle;
    MLabel *errorCorrectionSubtitle;
    MButton *correctionSpaceSwitch;
    MLabel *correctionSpaceTitle;
    MLabel *correctionSpaceSubtitle;

    QGraphicsWidget *chineseContainer;
    MLabel *chineseSettingHeader;
    MButton *fuzzySwitch;
    MLabel *fuzzyTitle;
    MButton *wordPredictionSwitch;
    MLabel *wordPredictionTitle;
    MKeyboardSettingsListItem *chineseTransliterationItem;
    MDialog *chineseTransliterationDialog;
    MList *chineseTransliterationList;
#endif

    friend class Ut_MKeyboardSettingsWidget;
};

#endif
