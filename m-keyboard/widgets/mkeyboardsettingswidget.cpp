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

#include "mkeyboardsettingswidget.h"
#include "layoutsmanager.h"

#ifdef HAVE_MEEGOTOUCH
#include "mkeyboardsettingslistitem.h"

#include <MButton>
#include <MLabel>
#include <MLayout>
#include <MLocale>
#include <MGridLayoutPolicy>
#include <MLinearLayoutPolicy>
#include <MContentItem>
#include <MAbstractCellCreator>
#include <MList>
#include <MDialog>
#include <MBanner>
#include <MBasicListItem>
#include <MContainer>
#include <MAbstractCellCreator>
#include <MImageWidget>
#endif

#include <QObject>
#include <QGraphicsLinearLayout>
#include <QStandardItemModel>
#include <QItemSelectionModel>
#include <QTimer>
#include <QDebug>

namespace
{
    //!object name for settings' widgets
    const QString ObjectNameSelectedKeyboardsItem("SelectedKeyboardsItem");
    const QString ObjectNameErrorCorrectionButton("KeyboardErrorCorrectionButton");
    const QString ObjectNameCorrectionSpaceButton("KeyboardCorrectionSpaceButton");
    const QString ObjectNameFuzzyPinyinButton("KeyboardFuzzyPinyinButton");
    const QString ObjectNameWordPredictionButton("KeyboardWordPredictionButton");
    const QString ObjectNameChineseTransliterationItem("ChineseTransliterationItem");
    const int MChineseTransliterationLayoutRole = Qt::UserRole + 1;

    const QString ChineseInputLanguage("zh_cn_*.xml");
};

#ifdef HAVE_MEEGOTOUCH
class MChineseTransliterationCellCreator: public MAbstractCellCreator<MContentItem>
{
public:
    /*! \reimp */
    virtual MWidget *createCell(const QModelIndex &index,
                                MWidgetRecycler &recycler) const;
    virtual void updateCell(const QModelIndex &index, MWidget *cell) const;
    /*! \reimp_end */
};

MWidget *MChineseTransliterationCellCreator::createCell(const QModelIndex &index,
                                         MWidgetRecycler &recycler) const
{
    MContentItem *cell = qobject_cast<MContentItem *>(recycler.take("MContentItem"));
    if (!cell) {
        cell = new MContentItem(MContentItem::SingleTextLabel);
    }
    updateCell(index, cell);
    return cell;
}

void MChineseTransliterationCellCreator::updateCell(const QModelIndex &index, MWidget *cell) const
{
    MContentItem *contentItem = qobject_cast<MContentItem *>(cell);

    contentItem->setTitle(index.data(Qt::DisplayRole).toString());
}
#endif

MKeyboardSettingsWidget::MKeyboardSettingsWidget(MKeyboardSettings *settings, QGraphicsItem *parent)
    : MWidget(parent)
#ifdef HAVE_MEEGOTOUCH
    , settingsObject(settings)
    , chineseTransliterationDialog(NULL)
    , chineseTransliterationList(NULL)
#endif
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(settings)
    Q_UNUSED(parent)
#else
    MLayout *layout = new MLayout(this);

    landscapePolicy = new MGridLayoutPolicy(layout);
    landscapePolicy->setContentsMargins(0, 0, 0, 0);
    landscapePolicy->setSpacing(0);
    //To make sure that both columns have the same width, give them the same preferred width.
    landscapePolicy->setColumnPreferredWidth(0, 800);
    landscapePolicy->setColumnPreferredWidth(1, 800);
    portraitPolicy = new MLinearLayoutPolicy(layout, Qt::Vertical);
    portraitPolicy->setContentsMargins(0, 0, 0, 0);
    portraitPolicy->setSpacing(0);

    layout->setLandscapePolicy(landscapePolicy);
    layout->setPortraitPolicy(portraitPolicy);

    buildUi();
    syncErrorCorrectionState();
    syncCorrectionSpaceState();
    syncFuzzyState();
    syncWordPredictionState();
    retranslateUi();
    updateChineseSettingPanel();
    connectSlots();
#endif
}

MKeyboardSettingsWidget::~MKeyboardSettingsWidget()
{
#ifdef HAVE_MEEGOTOUCH
    delete chineseTransliterationDialog;
    chineseTransliterationDialog = NULL;
#endif
}

void MKeyboardSettingsWidget::buildUi()
{
#ifdef HAVE_MEEGOTOUCH
    // Error correction settings
    MContainer *eCContainer = new MContainer(this);
    eCContainer->setContentsMargins(0, 0, 0, 0);
    eCContainer->setStyleName("CommonLargePanelInverted");
    eCContainer->setHeaderVisible(false);

    errorCorrectionSwitch = new MButton(this);
    errorCorrectionSwitch->setObjectName(ObjectNameErrorCorrectionButton);
    errorCorrectionSwitch->setStyleName("CommonRightSwitchInverted");
    errorCorrectionSwitch->setViewType(MButton::switchType);
    errorCorrectionSwitch->setCheckable(true);

    errorCorrectionTitle = new MLabel(this);
    errorCorrectionTitle->setStyleName("CommonTitleInverted");

    errorCorrectionSubtitle = new MLabel(this);
    errorCorrectionSubtitle->setStyleName("CommonSubTitleInverted");

    QGraphicsLinearLayout *eCLabelLayout = new QGraphicsLinearLayout(Qt::Vertical);
    eCLabelLayout->setContentsMargins(0, 0, 0, 0);
    eCLabelLayout->setSpacing(0);
    eCLabelLayout->addItem(errorCorrectionTitle);
    eCLabelLayout->addItem(errorCorrectionSubtitle);
    eCLabelLayout->addStretch();

    QGraphicsLinearLayout *eCLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    eCLayout->setContentsMargins(0, 0, 0, 0);
    eCLayout->setSpacing(0);
    eCLayout->addItem(eCLabelLayout);
    eCLayout->addItem(errorCorrectionSwitch);
    eCLayout->setAlignment(errorCorrectionSwitch, Qt::AlignCenter);
    // Put to first row, second column on the grid
    eCContainer->centralWidget()->setLayout(eCLayout);
    addItem(eCContainer, 0, 1);

    // "Space selects the correction candidate" settings
    MContainer *wCContainer = new MContainer(this);
    wCContainer->setContentsMargins(0, 0, 0, 0);
    wCContainer->setStyleName("CommonLargePanelInverted");
    wCContainer->setHeaderVisible(false);

    correctionSpaceSwitch = new MButton(this);
    correctionSpaceSwitch->setObjectName(ObjectNameCorrectionSpaceButton);
    correctionSpaceSwitch->setStyleName("CommonRightSwitchInverted");
    correctionSpaceSwitch->setViewType(MButton::switchType);
    correctionSpaceSwitch->setCheckable(true);

    correctionSpaceTitle = new MLabel(this);
    correctionSpaceTitle->setStyleName("CommonTitleInverted");

    correctionSpaceSubtitle = new MLabel(this);
    correctionSpaceSubtitle->setStyleName("CommonSubTitleInverted");

    QGraphicsLinearLayout *wCLabelLayout = new QGraphicsLinearLayout(Qt::Vertical);
    wCLabelLayout->setContentsMargins(0, 0, 0, 0);
    wCLabelLayout->setSpacing(0);
    wCLabelLayout->addItem(correctionSpaceTitle);
    wCLabelLayout->addItem(correctionSpaceSubtitle);
    wCLabelLayout->addStretch();

    QGraphicsLinearLayout *wCLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    wCLayout->setContentsMargins(0, 0, 0, 0);
    wCLayout->setSpacing(0);
    wCLayout->addItem(wCLabelLayout);
    wCLayout->addItem(correctionSpaceSwitch);
    wCLayout->setAlignment(correctionSpaceSwitch, Qt::AlignCenter);
    // Put to second row, second column on the grid
    wCContainer->centralWidget()->setLayout(wCLayout);
    addItem(wCContainer, 1, 1);

    // Initial Chinese setting

    // Chinese setting panel container
    QGraphicsLinearLayout *containerLayout = new QGraphicsLinearLayout(Qt::Vertical);
    // Ensure that there is no offset compared to widgets in layout above
    containerLayout->setContentsMargins(0, 0, 0, 0);
    chineseContainer = new QGraphicsWidget(this); // Used to be able to show/hide all children
    chineseContainer->setLayout(containerLayout);
    chineseSettingHeader = new MLabel(this);
    chineseSettingHeader->setStyleName("CommonGroupHeaderInverted");
    containerLayout->addItem(chineseSettingHeader);

    // Error correction setting
    MContainer *fuzzyContainer = new MContainer(this);
    fuzzyContainer->setContentsMargins(0, 0, 0, 0);
    fuzzyContainer->setStyleName("CommonLargePanelInverted");
    fuzzyContainer->setHeaderVisible(false);

    fuzzySwitch = new MButton(this);
    fuzzySwitch->setObjectName(ObjectNameFuzzyPinyinButton);
    fuzzySwitch->setStyleName("CommonRightSwitchInverted");
    fuzzySwitch->setViewType(MButton::switchType);
    fuzzySwitch->setCheckable(true);

    fuzzyTitle = new MLabel(this);
    fuzzyTitle->setStyleName("CommonSingleTitleInverted");

    QGraphicsLinearLayout *fuzzyLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    fuzzyLayout->setContentsMargins(0, 0, 0, 0);
    fuzzyLayout->setSpacing(0);
    fuzzyLayout->addItem(fuzzyTitle);
    fuzzyLayout->addItem(fuzzySwitch);
    fuzzyLayout->setAlignment(fuzzySwitch, Qt::AlignCenter);
    fuzzyContainer->centralWidget()->setLayout(fuzzyLayout);
    containerLayout->addItem(fuzzyContainer);

    // Word prediction setting
    MContainer *wordPredictionContainer = new MContainer(this);
    wordPredictionContainer->setContentsMargins(0, 0, 0, 0);
    wordPredictionContainer->setStyleName("CommonLargePanelInverted");
    wordPredictionContainer->setHeaderVisible(false);

    wordPredictionSwitch = new MButton(this);
    wordPredictionSwitch->setObjectName(ObjectNameWordPredictionButton);
    wordPredictionSwitch->setStyleName("CommonRightSwitchInverted");
    wordPredictionSwitch->setViewType(MButton::switchType);
    wordPredictionSwitch->setCheckable(true);

    wordPredictionTitle = new MLabel(this);
    wordPredictionTitle->setStyleName("CommonSingleTitleInverted");

    QGraphicsLinearLayout *wordPredictionLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    wordPredictionLayout->setContentsMargins(0, 0, 0, 0);
    wordPredictionLayout->setSpacing(0);
    wordPredictionLayout->addItem(wordPredictionTitle);
    wordPredictionLayout->addItem(wordPredictionSwitch);
    wordPredictionLayout->setAlignment(wordPredictionSwitch, Qt::AlignCenter);
    wordPredictionLayout->setAlignment(wordPredictionTitle, Qt::AlignCenter);
    wordPredictionContainer->centralWidget()->setLayout(wordPredictionLayout);
    containerLayout->addItem(wordPredictionContainer);

    // Chinese transliteration setting.
    chineseTransliterationItem = new MKeyboardSettingsListItem();
    MImageWidget *chineseTransliterationArrow = new MImageWidget();
    chineseTransliterationArrow->setStyleName("CommonComboBoxIconInverted");
    chineseTransliterationItem->setImageWidget(chineseTransliterationArrow);
    chineseTransliterationItem->setObjectName(ObjectNameChineseTransliterationItem);
    connect(chineseTransliterationItem, SIGNAL(clicked()), this, SLOT(showChineseTransliterationOptions()));
    chineseTransliterationItem->setStyleName("CommonBasicListItemInverted");
    containerLayout->addItem(chineseTransliterationItem);

    chineseContainer->setVisible(false);

    retranslateUi();
#endif
}

void MKeyboardSettingsWidget::addItem(QGraphicsLayoutItem *item, int row, int column)
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(item)
    Q_UNUSED(row)
    Q_UNUSED(column)
#else
    landscapePolicy->addItem(item, row, column);
    portraitPolicy->addItem(item);
#endif
}

void MKeyboardSettingsWidget::removeItem(QGraphicsLayoutItem *item)
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(item)
#else
    landscapePolicy->removeItem(item);
    portraitPolicy->removeItem(item);
#endif
}

void MKeyboardSettingsWidget::retranslateUi()
{
#ifdef HAVE_MEEGOTOUCH
    updateTitle();
    MWidget::retranslateUi();
#endif
}

void MKeyboardSettingsWidget::updateTitle()
{
#ifdef HAVE_MEEGOTOUCH
    if (!errorCorrectionTitle    || !errorCorrectionSubtitle
        || !correctionSpaceTitle || !correctionSpaceSubtitle
        || !chineseSettingHeader || !fuzzyTitle
        || !wordPredictionTitle  || !chineseTransliterationItem
        || !settingsObject)
        return;

    //% "Error correction"
    errorCorrectionTitle->setText(qtTrId("qtn_txts_error_correction"));
    //% "Error correction description"
    errorCorrectionSubtitle->setText(qtTrId("qtn_txts_error_correction_description"));
    //% "Insert with space"
    correctionSpaceTitle->setText(qtTrId("qtn_txts_insert_with_space"));
    //% "Space key inserts the suggested word"
    correctionSpaceSubtitle->setText(qtTrId("qtn_txts_insert_with_space_description"));

    //% "Chinese virtual keyboards"
    chineseSettingHeader->setText(qtTrId("qtn_ckb_chinese_keyboards"));
    //% "Fuzzy Pinyin input"
    fuzzyTitle->setText(qtTrId("qtn_ckb_fuzzy_pinyin"));
    //% "Next word prediction"
    wordPredictionTitle->setText(qtTrId("qtn_ckb_next_prediction"));
    chineseTransliterationItem->setTitle(qtTrId("qtn_ckb_convert_chinese"));

    // Sets subtitle of Chinese transliteration.
    chineseTransliterationItem->setSubtitle(
                settingsObject->chineseTransliterationOptions().value(
                    settingsObject->chineseTransliteration()));
#endif
}

void MKeyboardSettingsWidget::connectSlots()
{
#ifdef HAVE_MEEGOTOUCH
    if (!settingsObject || !errorCorrectionSwitch || !correctionSpaceSwitch)
        return;

    connect(errorCorrectionSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setErrorCorrectionState(bool)));
    connect(settingsObject, SIGNAL(errorCorrectionChanged()),
            this, SLOT(syncErrorCorrectionState()));
    connect(correctionSpaceSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setCorrectionSpaceState(bool)));
    connect(settingsObject, SIGNAL(correctionSpaceChanged()),
            this, SLOT(syncCorrectionSpaceState()));
    connect(settingsObject, SIGNAL(enabledKeyboardsChanged()),
            this, SLOT(updateChineseSettingPanel()));

    connect(fuzzySwitch, SIGNAL(toggled(bool)),
            this, SLOT(setFuzzyState(bool)));
    connect(settingsObject, SIGNAL(fuzzyStateChanged()),
            this, SLOT(syncFuzzyState()));

    connect(wordPredictionSwitch, SIGNAL(toggled(bool)),
            this, SLOT(setWordPredictionState(bool)));
    connect(settingsObject, SIGNAL(wordPredictionStateChanged()),
            this, SLOT(syncWordPredictionState()));
#endif
}

void MKeyboardSettingsWidget::updateChineseSettingPanel()
{
#ifdef HAVE_MEEGOTOUCH
    QStringList allKeyboardLayoutFiles = settingsObject->selectedKeyboards();

    QRegExp chineseLayoutExp(ChineseInputLanguage, Qt::CaseInsensitive);
    chineseLayoutExp.setPatternSyntax(QRegExp::Wildcard);

    if (allKeyboardLayoutFiles.indexOf(chineseLayoutExp) != -1) {
        // Chinese keyboard installed, then show Chinese setting panel.
        if (!chineseContainer->isVisible()) {
            addItem(chineseContainer, 2, 1);
            chineseContainer->setVisible(true);
        }
    } else {
        // No Chinese keyboard installed, then hide Chinese setting panel.
        if (chineseContainer->isVisible()) {
            removeItem(chineseContainer);
            chineseContainer->setVisible(false);
        }
    }
#endif
}

void MKeyboardSettingsWidget::setErrorCorrectionState(bool enabled)
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(enabled)
#else
    if (!settingsObject)
        return;

    if (settingsObject->errorCorrection() != enabled) {
        settingsObject->setErrorCorrection(enabled);
        if (!enabled) {
            // Disable the "Select with Space" option if the error correction is disabled
            setCorrectionSpaceState(false);
            correctionSpaceSwitch->setEnabled(false);
        } else {
            // Enable the "Select with Space" switch again
            correctionSpaceSwitch->setEnabled(true);
        }
    }
#endif
}

void MKeyboardSettingsWidget::syncErrorCorrectionState()
{
#ifdef HAVE_MEEGOTOUCH
    if (!settingsObject || !errorCorrectionSwitch)
        return;

    const bool errorCorrectionState = settingsObject->errorCorrection();
    if (errorCorrectionSwitch->isChecked() != errorCorrectionState) {
        errorCorrectionSwitch->setChecked(errorCorrectionState);
    }

    if (!errorCorrectionState) {
        // Disable the "Select with Space" option if the error correction is disabled
        setCorrectionSpaceState(false);
        correctionSpaceSwitch->setEnabled(false);
    } else {
        // Enable the "Select with Space" switch again
        correctionSpaceSwitch->setEnabled(true);
    }
#endif
}

void MKeyboardSettingsWidget::setCorrectionSpaceState(bool enabled)
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(enabled)
#else
    if (!settingsObject)
        return;

    if (settingsObject->correctionSpace() != enabled)
        settingsObject->setCorrectionSpace(enabled);
#endif
}

void MKeyboardSettingsWidget::syncCorrectionSpaceState()
{
#ifdef HAVE_MEEGOTOUCH
    if (!settingsObject)
        return;

    const bool correctionSpaceState = settingsObject->correctionSpace();
    if (correctionSpaceSwitch
        && correctionSpaceSwitch->isChecked() != correctionSpaceState) {
        correctionSpaceSwitch->setChecked(correctionSpaceState);
    }
#endif
}

void MKeyboardSettingsWidget::setFuzzyState(bool enabled)
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(enabled)
#else
    if (!settingsObject)
        return;
    settingsObject->setFuzzyPinyin(enabled);
#endif
}

void MKeyboardSettingsWidget::syncFuzzyState()
{
#ifdef HAVE_MEEGOTOUCH
    if (!settingsObject)
        return;

    const bool fuzzyState = settingsObject->fuzzyPinyin();
    if (fuzzySwitch &&
        (fuzzySwitch->isChecked() != fuzzyState)) {
        fuzzySwitch->setChecked(fuzzyState);
    }
#endif
}

void MKeyboardSettingsWidget::setWordPredictionState(bool enabled)
{
#ifndef HAVE_MEEGOTOUCH
    Q_UNUSED(enabled)
#else
    if (!settingsObject)
          return;
    settingsObject->setWordPrediction(enabled);
#endif
}

void MKeyboardSettingsWidget::syncWordPredictionState()
{
#ifdef HAVE_MEEGOTOUCH
    if (!settingsObject)
        return;

    const bool wordPredictionState = settingsObject->wordPrediction();
    if (wordPredictionSwitch &&
        (wordPredictionSwitch->isChecked() != wordPredictionState)) {
        wordPredictionSwitch->setChecked(wordPredictionState);
    }
#endif
}

void MKeyboardSettingsWidget::showChineseTransliterationOptions()
{
#ifdef HAVE_MEEGOTOUCH
    // Create the dialog when necessary.
    if (chineseTransliterationDialog == NULL) {
        chineseTransliterationDialog = new MDialog(qtTrId("qtn_ckb_convert_chinese"), M::OkButton);
        chineseTransliterationList = new MList(chineseTransliterationDialog);
        chineseTransliterationList->setCellCreator(new MChineseTransliterationCellCreator);
        chineseTransliterationList->setSelectionMode(MList::SingleSelection);
        createChineseTransliterationModel();
        chineseTransliterationDialog->setCentralWidget(chineseTransliterationList);

        connect(chineseTransliterationDialog, SIGNAL(accepted()),
                this, SLOT(selectChineseTransliteration()));
    }
    // Update the model before displaying the dialog.
    updateChineseTransliterationModel();
    // Display the dialog.
    chineseTransliterationDialog->exec();
#endif
}

void MKeyboardSettingsWidget::createChineseTransliterationModel()
{
#ifdef HAVE_MEEGOTOUCH
    if ((settingsObject == NULL) || (chineseTransliterationList == NULL))
        return;

    // Construct the model of Chinese transliteration.
    QMap<QString, QString> totalOptions = settingsObject->chineseTransliterationOptions();
    QStandardItemModel *model = new QStandardItemModel(totalOptions.size(),
                                                       1,
                                                       chineseTransliterationList);
    QMap<QString, QString>::const_iterator it = totalOptions.constBegin();
    for (int j = 0; it != totalOptions.constEnd(); ++it, ++j) {
        QStandardItem *item = new QStandardItem(it.value());
        item->setData(it.value(), Qt::DisplayRole);
        item->setData(it.key(), MChineseTransliterationLayoutRole);
        model->setItem(j, item);
    }

    // Set the model.
    chineseTransliterationList->setItemModel(model);
    chineseTransliterationList->setSelectionModel(new QItemSelectionModel(model,
                                                                          chineseTransliterationList));
#endif
}

void MKeyboardSettingsWidget::updateChineseTransliterationModel()
{
#ifdef HAVE_MEEGOTOUCH
    if ((settingsObject == NULL) || (chineseTransliterationList == NULL))
        return;

    // Clear current selection.
    chineseTransliterationList->selectionModel()->clearSelection();
    // Query the model item of current Chinese transliteration setting value.
    QStandardItemModel *model = static_cast<QStandardItemModel *>(chineseTransliterationList->itemModel());
    QString hightlightString = settingsObject->chineseTransliterationOptions().value(
                                                         settingsObject->chineseTransliteration());
    QList<QStandardItem *> items = model->findItems(hightlightString);
    // Set the selected item as highlight.
    chineseTransliterationList->selectionModel()->select(items.at(0)->index(),
                                                         QItemSelectionModel::Select);
#endif
}

void MKeyboardSettingsWidget::selectChineseTransliteration()
{
#ifdef HAVE_MEEGOTOUCH
    if ((settingsObject == NULL) || (chineseTransliterationDialog == NULL))
        return;

    // Save current option.
    QModelIndexList indexList = chineseTransliterationList->selectionModel()->selectedIndexes();
    QString value = indexList.at(0).data(MChineseTransliterationLayoutRole).toString();
    settingsObject->setChineseTransliteration(value);
    // Update related title.
    chineseTransliterationItem->setSubtitle(
                settingsObject->chineseTransliterationOptions().value(
                    settingsObject->chineseTransliteration()));
#endif
}
