/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScopedPointer>
#include "PrimerDesign/primer.h"
#include "PrimerDesign/parameterset.h"

using namespace PrimerDesign;

class TestParameterSet : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Getters/setters
    void testAmpliconRange();
    void testAmpliconWindowRange();
    void testPrimerRange();
    void testTmRange();
    void testForwardPrefix();
    void testForwardSuffix();
    void testReversePrefix();
    void testReverseSuffix();
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
void TestParameterSet::constructor()
{
    ParameterSet p = ParameterSet();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Getters/setters
void TestParameterSet::testAmpliconRange()
{
    ParameterSet p;
    Range window(constants::kAmpliconLengthMin + 2, constants::kAmpliconLengthMax - 2);
    Range wide(window.min() - 1, window.max() + 1);
    Range left(window.min() - 1, window.max() - 1);
    Range right(window.min() + 1, window.max() + 1);
    Range valid(window.min(), window.max());
    QVERIFY(p.setAmpliconWindowRange(window));
    QVERIFY(p.setAmpliconRange(valid));
    QVERIFY(!p.setAmpliconRange(wide));
    QVERIFY(!p.setAmpliconRange(left));
    QVERIFY(!p.setAmpliconRange(right));
    QVERIFY(p.ampliconRange().min() == valid.min());
    QVERIFY(p.ampliconRange().max() == valid.max());
}

void TestParameterSet::testAmpliconWindowRange()
{
    ParameterSet p;
    Range wide(constants::kAmpliconLengthMin - 1, constants::kAmpliconLengthMax + 1);
    Range valid(constants::kAmpliconLengthMin, constants::kAmpliconLengthMax);
    QVERIFY(p.setAmpliconWindowRange(valid));
    QVERIFY(!p.setAmpliconWindowRange(wide));
    QVERIFY(p.ampliconWindowRange().min() == constants::kAmpliconLengthMin);
    QVERIFY(p.ampliconWindowRange().max() == constants::kAmpliconLengthMax);
}

void TestParameterSet::testPrimerRange()
{
    ParameterSet p;
    Range window(constants::kAmpliconLengthMin + 2, constants::kAmpliconLengthMax - 2);
    Range left(window.min() - 1, window.max() - 1);
    Range right(window.min() + 1, window.max() + 1);
    Range tooLong(window.min(), window.max());
    Range tooShort(window.min(), window.min() + constants::kPrimerLengthMin - 1);
    Range min(window.min(), window.min() + constants::kPrimerLengthMin);
    Range max(window.max() - constants::kPrimerLengthMax, window.max());
    QVERIFY(p.setAmpliconWindowRange(window));

    QVERIFY(p.setPrimerRange(min));
    QVERIFY(!p.setPrimerRange(tooLong));
    QVERIFY(!p.setPrimerRange(tooShort));
    QVERIFY(p.primerRange().min() == min.min());
    QVERIFY(p.primerRange().max() == min.max());

    QVERIFY(p.setPrimerRange(max));
    QVERIFY(!p.setPrimerRange(left));
    QVERIFY(!p.setPrimerRange(right));
    QVERIFY(p.primerRange().min() == max.min());
    QVERIFY(p.primerRange().max() == max.max());
}

void TestParameterSet::testTmRange()
{
    ParameterSet p;
    RangeF tooLow(constants::kTmMin - 1, constants::kTmDefaultMax);
    RangeF tooHigh(constants::kTmMax - 10, constants::kTmMax + 1);
    RangeF valid(constants::kTmMin, constants::kTmMax);

    QVERIFY(p.setTmRange(valid));
    QVERIFY(!p.setTmRange(tooLow));
    QVERIFY(!p.setTmRange(tooHigh));
    QVERIFY(p.tmRange().min() == valid.min());
    QVERIFY(p.tmRange().max() == valid.max());
}

void TestParameterSet::testForwardPrefix()
{
    ParameterSet p;
    QVERIFY(p.setForwardPrefix(constants::kDnaCharacters));
    QVERIFY(!p.setForwardPrefix("hello"));
    QVERIFY(p.forwardPrefix() == constants::kDnaCharacters);
}

void TestParameterSet::testForwardSuffix()
{
    ParameterSet p;
    const char *SUFFIX = "AG";
    QVERIFY2(p.setForwardSuffix("ACG"), "Three characters max.");
    QVERIFY2(p.setForwardSuffix(SUFFIX), "Two characters allowed.");
    QVERIFY2(!p.setForwardSuffix("ACGT"), "Four chars are one too many.");
    QVERIFY2(!p.setForwardSuffix(""), "Blanks not allowed");
    QVERIFY2(p.forwardSuffix() == SUFFIX, "Verify the last valid set is still there.");
}

void TestParameterSet::testReversePrefix()
{

}

void TestParameterSet::testReverseSuffix()
{

}

QTEST_MAIN(TestParameterSet)
#include "TestParameterSet.moc"
