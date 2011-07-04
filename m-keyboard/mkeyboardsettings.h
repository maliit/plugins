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

#ifndef MKEYBOARDSETTINGS_H
#define MKEYBOARDSETTINGS_H

#include <QObject>
#include <MWidget>
#include <mimsettings.h>
#include <mabstractinputmethodsettings.h>

class QGraphicsWidget;

/*!
 * \brief MKeyboardSettings is the implemetation of meego-keyboard setting.
 * MKeyboardSettings implement MAbstractInputMethodSettings and create the meego-keyboard
 * setting. It provides below functionalities: get/set error corretion, get/set word
 * completion, get/set installed (selected) keyboards.
 */
class MKeyboardSettings: public QObject, public MAbstractInputMethodSettings
{
    Q_OBJECT
    Q_DISABLE_COPY(MKeyboardSettings)
    friend class Ut_MKeyboardSettings;

public:
    MKeyboardSettings();

    ~MKeyboardSettings();

    //!\reimp
    virtual QString title();
    virtual QString icon();
    virtual QGraphicsWidget *createContentWidget(QGraphicsWidget *parent = 0);
    //! \reimp_end

    //! Returns a map with layouts and titles for all selected keyboards.
    QStringList selectedKeyboards() const;

    //! Returns the boolean value of error correction option.
    bool errorCorrection() const;

    //! Sets error correction option.
    void setErrorCorrection(bool enabled);

    //! Returns the boolean value of "Space selects the correction candidate" option.
    bool correctionSpace() const;

    //! Sets "Space selects the correction candidate" option.
    void setCorrectionSpace(bool enabled);

    //! Returns fuzzy option of Chinese keyboard.
    bool fuzzyPinyin() const;

    //! Sets fuzzy option of Chinese keyboard.
    void setFuzzyPinyin(bool enabled);

    //! Returns word prediction option of Chinese keyboard.
    bool wordPrediction() const;

    //! Sets word prediction option of Chinese keyboard.
    void setWordPrediction(bool enabled);

    //! Returns a map with keys and descriptions for all Chinese transliteration options.
    QMap<QString, QString> chineseTransliterationOptions();

    //! Returns current value of Chinese transliteration option.
    QString chineseTransliteration();

    //! Sets the value of Chiense transliteration option.
    void setChineseTransliteration(QString value);

Q_SIGNALS:
    //! Emitted when selected keyboards are changed.
    void enabledKeyboardsChanged();

    //! Emitted when error correction option is changed.
    void errorCorrectionChanged();

    //! Emitted when "Space selects the correction candidate" option is changed.
    void correctionSpaceChanged();

    //! Emitted when Chinese fuzzy pinyin is changed.
    void fuzzyChanged();

    //! Emitted when Chinese word prediction is changed.
    void wordPredictionChanged();

private:
    QString keyboardTitle(const QString &layoutFile) const;
    QString keyboardLayoutFile(const QString &title) const;

    struct KeyboardInfo {
        QString layoutFile;
        QString title;
    };

    //! all available keyboards
    QList<KeyboardInfo> availableKeyboardInfos;
    MImSettings keyboardErrorCorrectionConf;
    MImSettings keyboardCorrectionSpaceConf;
    MImSettings enabledKeyboardsConf;

    MImSettings chineseKeyboardFuzzyConf;
    MImSettings chineseKeyboardWordPredictionConf;
    MImSettings chineseTransliterationConf;
};

#endif
