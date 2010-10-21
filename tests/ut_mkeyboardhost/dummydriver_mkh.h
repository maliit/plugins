/* This file is part of meego-inputmethodkeyboard *
 *
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * Contact: Nokia Corporation (directui@nokia.com)
 *
 * If you have questions regarding the use of this file, please contact
 * Nokia at directui@nokia.com.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * and appearing in the file LICENSE.LGPL included in the packaging
 * of this file.
 */

#ifndef DUMMYDRIVER_MKH_H
#define DUMMYDRIVER_MKH_H

#include <mimenginetypes.h>
#include <mimenginewordsinterface.h>
#include <QString>
#include <QStringList>
#include <QObject>

class MImEngineWords;

//! Just like the dummy im engine driver from meegotouch-inputmethodengine
//! except that this has functionality specific to the ut_mkeyboardhost
class DummyDriverMkh: public QObject, public MImEngineWordsInterface
{
    Q_OBJECT
    Q_DISABLE_COPY(DummyDriverMkh)
    Q_INTERFACES(MImEngineWordsInterface)

public:
    /*!
     * DummyDriverMkh construtor
     */
    DummyDriverMkh();

    /*!
     * DummyDriverMkh destructor
     */
    ~DummyDriverMkh();

    /*!
     *\reimp
    */

    virtual bool init();

    virtual bool addDictionaryWord(const QString &word, M::DictionaryType);

    virtual bool removeDictionaryWord(const QString &word, M::DictionaryType);

    virtual bool disableDictionary(M::DictionaryType);

    virtual bool enableDictionary(M::DictionaryType);

    virtual bool removeDictionary(M::DictionaryType);

    virtual void appendString(const QString &s, bool reselection = false);

    virtual void appendCharacter(const QChar &c);

    virtual void tapKeyboard(const QPoint &position, bool shift);

    virtual void setContext(const QString &s);

    virtual void disablePrediction();

    virtual void enablePrediction();

    virtual bool predictionEnabled();

    virtual void disableCorrection();

    virtual void enableCorrection();

    virtual bool correctionEnabled();

    virtual void disableCompletion();

    virtual void enableCompletion();

    virtual bool completionEnabled();

    virtual QStringList candidates(unsigned int uStartIndex = 0, unsigned int uNum = 0);

    virtual int totalCandidates();

    virtual int matchedLength();

    virtual int suggestedCandidateIndex();

    virtual bool setSuggestedCandidateIndex(int index);

    virtual void setExactWordPositionInList(M::ExactInListType setting);

    virtual M::DictionaryType candidateSource(int index);

    virtual bool setKeyboardLayoutKeys(const QList<M::KeyboardLayoutKey> &keys);

    virtual QList<M::KeyboardLayoutKey> keyboardLayoutKeys();

    virtual void clearEngineBuffer();

    virtual void saveAndClearEngineBuffer();

    virtual QString  language();

    virtual bool setLanguage(const QString &language, M::LanguagePriority);

    virtual bool setKeyboardLayout(const QString &);

    virtual QString keyboardLayout();

    virtual bool exportAsNokiaDictionary(const QString &);

    virtual bool importNokiaUserDictionary(const QString &);

    virtual void setMaximumErrors(int);

    virtual int maximumErrors();

    virtual void setMaximumCandidates(int maxCandidates);

    virtual int maximumCandidates();

    virtual QString driver();

    virtual QString error();

    /*!
     *\reimp_end
    */

    // Special setters for ut_mkeyboardhost..................................

    void setCandidates(const QStringList &candidates);
    //! \brief Note that this is different from setSuggestedCandidateIndex
    void setSuggestedCandidateIndexReturnValue(int index);

private:
    bool predictionEnabledFlag;
    bool correctionEnabledFlag;
    bool completionEnabledFlag;

    QString driverLanguage;
    QString kdbLanguage;

    int maximumErrorsCount;
    int maximumCandidatesCount;

    QStringList candidateList;
    int suggestedCandidateIndexValue;
};

#endif //DUMMYDRIVER_H
