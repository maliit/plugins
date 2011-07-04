
#ifndef UT_WORDRIBBONITEM_H
#define UT_WORDRIBBONITEM_H

#include "wordribbonitem.h"

#include <MApplication>
#include <minputmethodnamespace.h>
#include <QtTest/QTest>
#include <QObject>

class WordRibbonItemStyleContainer;
class MPlainWindow;

class Ut_WordRibbonItem : public QObject
{
    Q_OBJECT

private slots:
    //! initialize application and class
    void initTestCase();
    void init();
    void cleanup();
    void cleanupTestCase();

    void checkSetText_data();
    void checkSetText();

    void checkSetHighlighted();

    void checkSetPositionIndex_data();
    void checkSetPositionIndex();

    void checkSignalEmission_data();
    void checkSignalEmission();

    void checkSize_data();
    void checkSize();

private:
    void rotateToAngle(MInputMethod::OrientationAngle angle);

private:
    MApplication *app;
    MPlainWindow * view;
    WordRibbonItem *subject;
    WordRibbonItemStyleContainer *candidateItemStyleContainer;
};

#endif
