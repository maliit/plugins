/* This file is part of meegotouch-inputmethodkeyboard *
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

#include "dummydriver_mkh.h"

DummyDriverMkh::DummyDriverMkh()
    : predictionEnabledFlag(false),
      correctionEnabledFlag(false),
      completionEnabledFlag(false),
      maximumErrorsCount(0),
      maximumCandidatesCount(0),
      suggestedCandidateIndexValue(0)
{
}

DummyDriverMkh::~DummyDriverMkh()
{
}

bool DummyDriverMkh::init()
{
    return true;
}

bool DummyDriverMkh::addDictionaryWord(const QString &w, M::DictionaryType type)
{
    Q_UNUSED(w);
    Q_UNUSED(type);
    return false;
}

bool DummyDriverMkh::removeDictionaryWord(const QString &w, M::DictionaryType type)
{
    Q_UNUSED(w);
    Q_UNUSED(type);
    return false;
}

bool DummyDriverMkh::disableDictionary(M::DictionaryType type)
{
    Q_UNUSED(type);
    return false;
}
bool DummyDriverMkh::enableDictionary(M::DictionaryType type)
{
    Q_UNUSED(type);
    return false;
}
bool DummyDriverMkh::removeDictionary(M::DictionaryType type)
{
    Q_UNUSED(type);
    return false;
}

void DummyDriverMkh::appendString(const QString &s, bool reselection)
{
    Q_UNUSED(s);
    Q_UNUSED(reselection);
}
void DummyDriverMkh::appendCharacter(const QChar &c)
{
    Q_UNUSED(c);
}

void DummyDriverMkh::tapKeyboard(const QPoint &position, bool shift)
{
    Q_UNUSED(position);
    Q_UNUSED(shift);
}

void DummyDriverMkh::setContext(const QString &s)
{
    Q_UNUSED(s);
}

void DummyDriverMkh::disablePrediction()
{
    predictionEnabledFlag = false;
}

void DummyDriverMkh::enablePrediction()
{
    predictionEnabledFlag = true;
}

bool DummyDriverMkh::predictionEnabled()
{
    return predictionEnabledFlag;
}

void DummyDriverMkh::disableCorrection()
{
    correctionEnabledFlag = false;
}

void DummyDriverMkh::enableCorrection()
{
    correctionEnabledFlag = true;
}

bool DummyDriverMkh::correctionEnabled()
{
    return correctionEnabledFlag;
}

void DummyDriverMkh::disableCompletion()
{
    completionEnabledFlag = false;
}

void DummyDriverMkh::enableCompletion()
{
    completionEnabledFlag = true;
}

bool DummyDriverMkh::completionEnabled()
{
    return completionEnabledFlag;
}

QStringList DummyDriverMkh::candidates(unsigned int uStartIndex, unsigned int uNum)
{
    Q_UNUSED(uStartIndex);
    Q_UNUSED(uNum);
    return candidateList;
}

int DummyDriverMkh::totalCandidates()
{
    return 0;
}

int DummyDriverMkh::matchedLength()
{
    return 0;
}

int DummyDriverMkh::suggestedCandidateIndex()
{
    return suggestedCandidateIndexValue;
}

bool  DummyDriverMkh::setSuggestedCandidateIndex(int index)
{
    Q_UNUSED(index);
    return true;
}

void DummyDriverMkh::setExactWordPositionInList(M::ExactInListType setting)
{
    Q_UNUSED(setting);
}

M::DictionaryType DummyDriverMkh::candidateSource(int index)
{
    Q_UNUSED(index);
    return M::DictionaryTypeInvalid;
}

bool DummyDriverMkh::setKeyboardLayoutKeys(const QList<M::KeyboardLayoutKey> &keys)
{
    Q_UNUSED(keys);
    return true;
}

QList<M::KeyboardLayoutKey> DummyDriverMkh::keyboardLayoutKeys()
{
    QList<M::KeyboardLayoutKey> keys;
    return keys;
}

void DummyDriverMkh::clearEngineBuffer()
{
}

void DummyDriverMkh::saveAndClearEngineBuffer()
{
}

QString  DummyDriverMkh::language()
{
    return driverLanguage;
}

bool DummyDriverMkh::setLanguage(const QString  &l, M::LanguagePriority p)
{
    Q_UNUSED(p);
    driverLanguage = l;
    return true;
}

bool DummyDriverMkh::setKeyboardLayout(const QString &l)
{
    kdbLanguage = l;
    return true;
}

QString DummyDriverMkh::keyboardLayout()
{
    return kdbLanguage;
}

bool DummyDriverMkh::exportAsNokiaDictionary(const QString &fileName)
{
    Q_UNUSED(fileName);
    return false;
}

bool DummyDriverMkh::importNokiaUserDictionary(const QString &fileName)
{
    Q_UNUSED(fileName);
    return false;
}

void DummyDriverMkh::setMaximumErrors(int maxErrors)
{
    maximumErrorsCount = maxErrors;
}

int DummyDriverMkh::maximumErrors()
{
    return maximumErrorsCount;
}

QString DummyDriverMkh::driver()
{
    return "dummyimdriver_mkh";
}

void DummyDriverMkh::setMaximumCandidates(int maxCandidates)
{
    maximumCandidatesCount = maxCandidates;
}

int DummyDriverMkh::maximumCandidates()
{
    return maximumCandidatesCount;
}

QString DummyDriverMkh::error()
{
    return QString();
}

void DummyDriverMkh::setCandidates(const QStringList &candidates)
{
    candidateList = candidates;
}

void DummyDriverMkh::setSuggestedCandidateIndexReturnValue(int index)
{
    suggestedCandidateIndexValue = index;
}
