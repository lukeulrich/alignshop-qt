/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../AgDoubleSpinBox.h"

class TestAgDoubleSpinBox : public QObject
{
    Q_OBJECT

private slots:
    void visibleDecimals();     // Also tests setVisibleDecimals
    void textFromValue();
    void value();
    void stepFactor();
    void stepBy();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAgDoubleSpinBox::visibleDecimals()
{
    AgDoubleSpinBox x;

    QCOMPARE(x.visibleDecimals(), 0);
    x.setVisibleDecimals(1);
    QCOMPARE(x.visibleDecimals(), 1);
    x.setVisibleDecimals(-1);
    QCOMPARE(x.visibleDecimals(), 1);
    x.setVisibleDecimals(4);
    QCOMPARE(x.visibleDecimals(), 4);
    x.setVisibleDecimals(-3);
    QCOMPARE(x.visibleDecimals(), 4);
    x.setVisibleDecimals(0);
    QCOMPARE(x.visibleDecimals(), 0);
}

void TestAgDoubleSpinBox::textFromValue()
{
    AgDoubleSpinBox x;

    x.setDecimals(0);
    QCOMPARE(x.visibleDecimals(), 0);

    QCOMPARE(x.textFromValue(123.), QString("123"));
    QCOMPARE(x.textFromValue(123.4), QString("123"));
    QCOMPARE(x.textFromValue(123.5), QString("124"));
    QCOMPARE(x.textFromValue(123.329483), QString("123"));

    x.setDecimals(1);
    QCOMPARE(x.textFromValue(123.), QString("123"));
    QCOMPARE(x.textFromValue(123.4), QString("123"));
    QCOMPARE(x.textFromValue(123.5), QString("124"));
    QCOMPARE(x.textFromValue(123.9929483), QString("124"));

    QList<int> ilist;
    ilist << 1 << 2 << 5;
    foreach (int i, ilist)
    {
        x.setVisibleDecimals(i);
        QCOMPARE(x.textFromValue(123.), QString("123.0"));
        QCOMPARE(x.textFromValue(123.4), QString("123.4"));
        QCOMPARE(x.textFromValue(123.5), QString("123.5"));
        QCOMPARE(x.textFromValue(123.9929483), QString("124.0"));
    }

    x.setDecimals(2);
    x.setVisibleDecimals(1);
    QCOMPARE(x.textFromValue(123.), QString("123.0"));
    QCOMPARE(x.textFromValue(123.456), QString("123.5"));
    QCOMPARE(x.textFromValue(123.53), QString("123.5"));
    QCOMPARE(x.textFromValue(123.9929483), QString("124.0"));
}

void TestAgDoubleSpinBox::value()
{
    AgDoubleSpinBox x;

    x.setMaximum(1000.);

    // ------------------------------------------------------------------------
    // Test: equal number of visible decimals and decimals
    x.setDecimals(0);
    x.setVisibleDecimals(0);

    x.setValue(.3);
    QCOMPARE(x.value(), 0.);
    x.setValue(12.928);
    QCOMPARE(x.value(), 13.);

    // ------------------------------------------------------------------------
    // Test: more visible decimals than decimals
    x.setVisibleDecimals(5);

    x.setValue(.3);
    QCOMPARE(x.value(), 0.);
    x.setValue(12.928);
    QCOMPARE(x.value(), 13.);

    // ------------------------------------------------------------------------
    // Test: more decimals than visible decimals
    x.setVisibleDecimals(1);
    x.setDecimals(2);
    x.setValue(123.);
    QCOMPARE(x.value(), 123.);
    x.setValue(123.456);
    QCOMPARE(x.value(), 123.46);
    x.setValue(123.539);
    QCOMPARE(x.value(), 123.54);
    x.setValue(123.9929483);
    QCOMPARE(x.value(), 123.99);
}

void TestAgDoubleSpinBox::stepFactor()
{
    AgDoubleSpinBox x;

    QCOMPARE(x.stepFactor(), 0.);
    x.setValue(50.);

    QList<double> doubles;
    doubles << .5 << .23 << 124.45;
    foreach (double d, doubles)
    {
        x.setStepFactor(d);
        QCOMPARE(x.stepFactor(), d);
    }

    double lastStepAmount = x.singleStep();
    x.setStepFactor(-83.934);
    QCOMPARE(x.stepFactor(), doubles.last());

    x.setStepFactor(0.);
    QCOMPARE(x.stepFactor(), 0.);
    QCOMPARE(x.singleStep(), lastStepAmount);

    x.setValue(30.);
    QCOMPARE(x.singleStep(), lastStepAmount);

    x.setValue(75.);
    x.setStepFactor(.2);
}

void TestAgDoubleSpinBox::stepBy()
{
    AgDoubleSpinBox x;
    x.setDecimals(5);

    // Setup:
    x.setMinimum(0.);
    x.setMaximum(10000.);
    x.setValue(1.);
    x.setStepFactor(0.);
    x.setSingleStep(5);

    // ------------------------------------------------------------------------
    // Test: default stepping when step factor is zero
    x.stepBy(1);
    QCOMPARE(x.value(), 6.);
    x.stepBy(3);
    QCOMPARE(x.value(), 21.);
    x.stepBy(0);
    QCOMPARE(x.value(), 21.);
    x.stepBy(-1);
    QCOMPARE(x.value(), 16.);
    x.stepBy(-3);
    QCOMPARE(x.value(), 1.);

    // ------------------------------------------------------------------------
    // Test: single step with step factor
    x.setStepFactor(1.);        // Double it each time
    x.stepBy(1);
    QCOMPARE(x.value(), 2.);
    x.stepBy(1);
    QCOMPARE(x.value(), 4.);
    x.stepBy(1);
    QCOMPARE(x.value(), 8.);
    x.stepBy(-1);
    QCOMPARE(x.value(), 4.);
    x.stepBy(-1);
    QCOMPARE(x.value(), 2.);
    x.stepBy(-1);
    QCOMPARE(x.value(), 1.);

    x.setStepFactor(.5);        // Step up by half the value
    x.stepBy(1);
    QCOMPARE(x.value(), 1.5);
    x.stepBy(1);
    QCOMPARE(x.value(), 2.25);
    x.stepBy(1);
    QCOMPARE(x.value(), 3.375);
    x.stepBy(-1);
    QCOMPARE(x.value(), 2.25);
    x.stepBy(-1);
    QCOMPARE(x.value(), 1.5);
    x.stepBy(-1);
    QCOMPARE(x.value(), 1.);

    // ------------------------------------------------------------------------
    // Test: multi step
    x.setStepFactor(1.);
    x.stepBy(2);
    QCOMPARE(x.value(), 4.);
    x.stepBy(3);
    QCOMPARE(x.value(), 32.);
    x.stepBy(-5);
    QCOMPARE(x.value(), 1.);

    // ------------------------------------------------------------------------
    // Test: 10% of each value
    x.setStepFactor(.1);
    x.setValue(100.);
    x.stepBy(-1);
    if (fabs(x.value() - (100. / 1.1)) > .0001)
        QVERIFY(0);
    x.stepBy(1);
    QCOMPARE(x.value(), 100.);
    x.stepBy(-2);
    if (fabs(x.value() - (100. / (1.1 * 1.1))) > .0001)
        QVERIFY(0);
    x.stepBy(2);
    QCOMPARE(x.value(), 100.);
    x.stepBy(1);
    QCOMPARE(x.value(), 110.);
}

QTEST_MAIN(TestAgDoubleSpinBox)
#include "TestAgDoubleSpinBox.moc"
