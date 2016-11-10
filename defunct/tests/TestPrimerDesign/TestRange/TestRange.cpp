/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScopedPointer>
#include "PrimerDesign/range.h"

using namespace PrimerDesign;

class TestRange : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void constructor();
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
void TestRange::constructor()
{
    Range range1(1, 20);
    Range range2(30, 10);

    QVERIFY2(range1.min() == 1, "Min should have been 1.");
    QVERIFY2(range1.max() == 20, "Max should have been 20.");

    QVERIFY2(range2.min() == 10, "Min should have been 10.");
    QVERIFY2(range2.max() == 30, "Max should have been 30.");
}

QTEST_MAIN(TestRange)
#include "TestRange.moc"
