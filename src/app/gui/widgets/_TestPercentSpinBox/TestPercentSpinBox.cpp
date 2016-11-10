/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../PercentSpinBox.h"

class TestPercentSpinBox : public QObject
{
    Q_OBJECT

private slots:
    void decimalPercent();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPercentSpinBox::decimalPercent()
{
    PercentSpinBox x;

    x.setMinimum(0.);
    x.setMaximum(1000.);
    x.setDecimals(2);
    x.setValue(1000.);      // So that the signal spy will catch the first signal for sure

    QSignalSpy spyDecimalPercentChanged(&x, SIGNAL(decimalPercentChanged(double)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: decimalPercent when setting the value normally
    for (double i=0.; i< x.maximum(); i += (x.maximum() - x.minimum()) / 50.)
    {
        x.setValue(i);
        QCOMPARE(x.decimalPercent(), i / 100.);

        QCOMPARE(spyDecimalPercentChanged.count(), 1);
        spyArguments = spyDecimalPercentChanged.takeFirst();
        QCOMPARE(spyArguments.at(0).toDouble(), i / 100.);
    }

    // ------------------------------------------------------------------------
    // Test: setting the value via the setDecimalPercent method
    for (double i=0.; i< x.maximum() / 100.; i += (x.maximum() - x.minimum()) / 100. / 50.)
    {
        x.setDecimalPercent(i);
        QCOMPARE(x.value(), i * 100.);
        QCOMPARE(x.decimalPercent(), i);

        QCOMPARE(spyDecimalPercentChanged.count(), 1);
        spyArguments = spyDecimalPercentChanged.takeFirst();
        QCOMPARE(spyArguments.at(0).toDouble(), i);
    }
}


QTEST_MAIN(TestPercentSpinBox)
#include "TestPercentSpinBox.moc"
