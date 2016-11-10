/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../SymbolStringCalculator.h"
#include "../../BioSymbol.h"
#include "../../BioSymbolGroup.h"
#include "../../misc.h"

class TestSymbolStringCalculator : public QObject
{
    Q_OBJECT

public:
    TestSymbolStringCalculator()
    {
        qRegisterMetaType<BioSymbolGroup>("BioSymbolGroup");
    }

private slots:
    void constructor();
    void calculateSymbolString_data();
    void calculateSymbolString();
    void setBioSymbolGroup();
    void setDefaultSymbol();
};

Q_DECLARE_METATYPE(BioSymbolGroup);
Q_DECLARE_METATYPE(VectorHashCharDouble);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestSymbolStringCalculator::constructor()
{
    BioSymbolGroup group;

    SymbolStringCalculator x(group);
    QCOMPARE(x.defaultSymbol(), ' ');

    SymbolStringCalculator y(group, 'X');
    QCOMPARE(y.defaultSymbol(), 'X');
}

void TestSymbolStringCalculator::calculateSymbolString_data()
{
    QTest::addColumn<BioSymbolGroup>("bioSymbolGroup");
    QTest::addColumn<char>("defaultSymbol");
    QTest::addColumn<VectorHashCharDouble>("vectorHashCharDouble");
    QTest::addColumn<QByteArray>("expectedSymbolString");

    BioSymbolGroup group;
    SymbolStringCalculator x(group);

    char defaultSymbol = x.defaultSymbol();
    char dummy[2];
    dummy[1] = '\0';

    // ------------------------------------------------------------------------
    // Test: completely empty BioSymbolGroup and empty VectorHashCharDouble except dimensionally
    //
    // Subtest 1: [ {} ]
    // Subtest 2: [ {}, {}, ]
    // ...
    for (int i=0; i<3; ++i)
    {
        VectorHashCharDouble vectorHashCharDouble;
        for (int j=0; j<i+1; ++j)
            vectorHashCharDouble << QHash<char, qreal>();

        dummy[0] = defaultSymbol;

        QTest::newRow(QString("Default BioSymbolGroup, %1 element VectorHashCharDouble (empty)").arg(i+1).toAscii())
                << BioSymbolGroup()
                << defaultSymbol
                << vectorHashCharDouble
                << QByteArray(dummy).repeated(i+1);
    }

    // ------------------------------------------------------------------------
    // Test: completely empty BioSymbolGroup and single element VectorHashCharDoubles
    //
    // Subtest 1: [ {A} ]
    // Subtest 2: [ {A}, {B}, ]
    // ...
    for (int i=0; i<3; ++i)
    {
        VectorHashCharDouble vectorHashCharDouble;
        for (int j=0; j<i+1; ++j)
        {
            vectorHashCharDouble << QHash<char, qreal>();
            vectorHashCharDouble.last().insert(static_cast<char>(j+65), randomInteger(0, 100) / 100.);
        }

        dummy[0] = '%';

        QTest::newRow(QString("Default BioSymbolGroup, %1 element VectorHashCharDouble (empty)").arg(i+1).toAscii())
                << BioSymbolGroup()
                << '%'
                << vectorHashCharDouble
                << QByteArray(dummy).repeated(i+1);
    }

    // ------------------------------------------------------------------------
    // Test: completely empty BioSymbolGroup and multi-element VectorHashCharDoubles
    //
    // Subtest 1: [ {A, B, ... ?} ]
    // Subtest 2: [ {A, ...?}, {B, ... ?}, ]
    // ...
    for (int i=0; i<3; ++i)
    {
        VectorHashCharDouble vectorHashCharDouble;
        for (int j=0; j<i+1; ++j)
        {
            vectorHashCharDouble << QHash<char, qreal>();
            qreal pool = 100;
            for (int k=0, z=randomInteger(1, 10); k<z; ++k)
            {
                int amount = randomInteger(0, pool);
                vectorHashCharDouble.last().insert(static_cast<char>(k+65), amount / 100.);
                pool -= amount;
            }
        }

        dummy[0] = '-';

        QTest::newRow(QString("Default BioSymbolGroup, %1 element VectorHashCharDouble (empty)").arg(i+1).toAscii())
                << BioSymbolGroup()
                << '-'
                << vectorHashCharDouble
                << QByteArray(dummy).repeated(i+1);
    }

    // ------------------------------------------------------------------------
    // Test: Single rule, single character must exceed threshold
    //       [ {A => .49},
    //         {A => .5, a => .25, C => .1},
    //         {A => .51},
    //         {A => .4, P => .5, C => .1} ]
    //       Cutoff = A for .5
    {
        VectorHashCharDouble vectorHashCharDouble;
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .49);

        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .50);
        vectorHashCharDouble.last().insert('a', .25);
        vectorHashCharDouble.last().insert('C', .1);

        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .51);

        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .4);
        vectorHashCharDouble.last().insert('P', .5);
        vectorHashCharDouble.last().insert('C', .1);

        QTest::newRow("Single rule, A => a (.5)")
                << (BioSymbolGroup() << BioSymbol('a', "A", .5))
                << '_'
                << vectorHashCharDouble
                << QByteArray("_aa_");
    }

    // ------------------------------------------------------------------------
    // Test: single rule, sum of multiple characters must exceed threshold
    {
        VectorHashCharDouble vectorHashCharDouble;
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .25);
        vectorHashCharDouble.last().insert('B', .24);
        vectorHashCharDouble.last().insert('C', .5);

        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .49);
        vectorHashCharDouble.last().insert('B', .01);
        vectorHashCharDouble.last().insert('a', .25);
        vectorHashCharDouble.last().insert('C', .1);

        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .51);

        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('B', .5);

        QTest::newRow("Single rule, AB => a (.5)")
                << (BioSymbolGroup() << BioSymbol('a', "AB", .5))
                << '_'
                << vectorHashCharDouble
                << QByteArray("_aaa");
    }

    // ------------------------------------------------------------------------
    // Test: multiple non-overlapping rules, overlapping and non-overlapping columns
    // There are two possibilites to test here, each possibility is tested in a
    // different column
    // These are:
    // sum(AB) < .5
    // sum(AB) = .5
    // sum(AB) > .5
    // sum(CD) < .3
    // sum(CD) = .3
    // sum(CD) > .3
    //
    // Options dealing with both rules in the same column
    // sum(AB) < .5, sum(CD) < .3
    // sum(AB) > .5, sum(CD) < .3
    // sum(AB) < .5, sum(CD) > .3
    // sum(AB) > .5, sum(CD) > .3
    {
        VectorHashCharDouble vectorHashCharDouble;
        // sum(AB) < .5
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .25);
        vectorHashCharDouble.last().insert('B', .24);

        // sum(AB) = .5
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .49);
        vectorHashCharDouble.last().insert('B', .01);
        vectorHashCharDouble.last().insert('a', .25);
        vectorHashCharDouble.last().insert('C', .1);

        // sum(AB) > .5
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .51);

        // sum(CD) < .3
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('C', .1);
        vectorHashCharDouble.last().insert('D', .1);

        // sum(CD) = .3
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('C', .2);
        vectorHashCharDouble.last().insert('D', .1);

        // sum(CD) > .3
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('C', .3);
        vectorHashCharDouble.last().insert('D', .3);

        // sum(AB) < .5, sum(CD) < .3
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .07);
        vectorHashCharDouble.last().insert('B', .03);
        vectorHashCharDouble.last().insert('C', .05);
        vectorHashCharDouble.last().insert('D', .1);

        // sum(AB) > .5, sum(CD) < .3
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .7);
        vectorHashCharDouble.last().insert('B', .01);
        vectorHashCharDouble.last().insert('C', .05);
        vectorHashCharDouble.last().insert('D', .1);

        // sum(AB) < .5, sum(CD) > .3
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .3);
        vectorHashCharDouble.last().insert('B', .1);
        vectorHashCharDouble.last().insert('C', .25);
        vectorHashCharDouble.last().insert('D', .2);

        // sum(AB) > .5, sum(CD) > .3
        // Result should be the AB rule wins because it has the same number of characters and a higher threshold
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .3);
        vectorHashCharDouble.last().insert('B', .25);
        vectorHashCharDouble.last().insert('C', .15);
        vectorHashCharDouble.last().insert('D', .2);

        QTest::newRow("Two rules: 1) AB => a (.5); 2) CD => c (.3)")
                << (BioSymbolGroup() << BioSymbol('a', "AB", .5) << BioSymbol('c', "CD", .3))
                << '_'
                << vectorHashCharDouble
                << QByteArray("_aa_cc_aca");
    }

    // ------------------------------------------------------------------------
    // Test: Two rules, same number of characters, same thresholds
    {
        VectorHashCharDouble vectorHashCharDouble;

        // sum(AB) = .5, sum(CD) = .4; both rules match, but since sum(AB) / 2 = .25 > sum(CD) / 2 = .2, AB wins
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .3);
        vectorHashCharDouble.last().insert('B', .2);
        vectorHashCharDouble.last().insert('C', .15);
        vectorHashCharDouble.last().insert('D', .25);

        // sum(AB) == sum(CD) == .3; tie, first rule wins
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .15);
        vectorHashCharDouble.last().insert('B', .15);
        vectorHashCharDouble.last().insert('C', .15);
        vectorHashCharDouble.last().insert('D', .15);

        // sum(AB) = .4, sum(CD) = .6; both rules match, but since sum(AB) / 2 = .2 < sum(CD) / 2 = .3, CD wins
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .3);
        vectorHashCharDouble.last().insert('B', .1);
        vectorHashCharDouble.last().insert('C', .3);
        vectorHashCharDouble.last().insert('D', .3);

        QTest::newRow("Two rules: 1) AB => a (.3); 2) CD => c (.3)")
                << (BioSymbolGroup() << BioSymbol('a', "AB", .3) << BioSymbol('c', "CD", .3))
                << '_'
                << vectorHashCharDouble
                << QByteArray("aac");
    }

    // ------------------------------------------------------------------------
    // Test: Two rules, different number of characters, same thresholds
    {
        VectorHashCharDouble vectorHashCharDouble;

        // sum(AB) == sum(C) == .3; C wins
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .2);
        vectorHashCharDouble.last().insert('B', .1);
        vectorHashCharDouble.last().insert('C', .3);

        // sum(AB) = .59, sum(CD) = .3; C wins
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .29);
        vectorHashCharDouble.last().insert('B', .3);
        vectorHashCharDouble.last().insert('C', .3);

        // sum(AB) = .6, sum(CD) = .3; AB wins because of tie and it is first rule
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('A', .3);
        vectorHashCharDouble.last().insert('B', .3);
        vectorHashCharDouble.last().insert('C', .3);

        // sum(AB) = .7 sum(CD) = .3; AB wins
        vectorHashCharDouble << QHash<char, qreal>();
        vectorHashCharDouble.last().insert('B', .7);
        vectorHashCharDouble.last().insert('C', .3);

        QTest::newRow("Two rules: 1) AB => a (.3); 2) C => c (.3)")
                << (BioSymbolGroup() << BioSymbol('a', "AB", .3) << BioSymbol('c', "C", .3))
                << '_'
                << vectorHashCharDouble
                << QByteArray("ccaa");
    }
}

void TestSymbolStringCalculator::calculateSymbolString()
{
    QFETCH(BioSymbolGroup, bioSymbolGroup);
    QFETCH(char, defaultSymbol);
    QFETCH(VectorHashCharDouble, vectorHashCharDouble);
    QFETCH(QByteArray, expectedSymbolString);

    SymbolStringCalculator x(bioSymbolGroup, defaultSymbol);

    QCOMPARE(x.computeSymbolString(vectorHashCharDouble), expectedSymbolString);
}

void TestSymbolStringCalculator::setDefaultSymbol()
{
    BioSymbolGroup group;
    SymbolStringCalculator x(group);

    for (int i=0; i< 255; ++i)
    {
        x.setDefaultSymbol(static_cast<char>(i));
        QCOMPARE(x.defaultSymbol(), static_cast<char>(i));
    }
}

void TestSymbolStringCalculator::setBioSymbolGroup()
{
    BioSymbolGroup group1;
    group1 << BioSymbol('-', "KR", .3);

    BioSymbolGroup group2;
    group2 << BioSymbol('+', "DE", .5);

    SymbolStringCalculator x(group1);
    QCOMPARE(x.bioSymbolGroup(), group1);
    x.setBioSymbolGroup(group2);
    QCOMPARE(x.bioSymbolGroup(), group2);
}

QTEST_APPLESS_MAIN(TestSymbolStringCalculator)
#include "TestSymbolStringCalculator.moc"
