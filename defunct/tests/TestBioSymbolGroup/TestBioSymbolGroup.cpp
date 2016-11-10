/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "BioSymbolGroup.h"

class TestBioSymbolGroup : public QObject
{
    Q_OBJECT

public:
    TestBioSymbolGroup(QObject *parent = 0) : QObject(parent)
    {
        qRegisterMetaType<BioSymbolGroup>("BioSymbolGroup");
    }

private slots:
    void constructor();
    void addBioSymbol();
    void operator_ltlt();
    void removeBioSymbol();
    void clear();
    void count();
    void isEmpty();
    void isCharAssociatedWithSymbol();
    void setDefaultSymbol();
    void calculateSymbolString_data();
    void calculateSymbolString();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestBioSymbolGroup::constructor()
{
    BioSymbolGroup x;
    QCOMPARE(x.defaultSymbol(), ' ');

    BioSymbolGroup y('X');
    QCOMPARE(y.defaultSymbol(), 'X');
}

void TestBioSymbolGroup::addBioSymbol()
{
    // ------------------------------------------------------------------------
    // Test: basic case
    BioSymbolGroup x;
    BioSymbol bioSymbol('N', "ACTG", .5);
    x.addBioSymbol(bioSymbol);
    QVERIFY(x.bioSymbols().contains('N'));
    QCOMPARE(x.bioSymbols().count(), 1);
    QCOMPARE(x.bioSymbols().value('N'), bioSymbol);

    // ------------------------------------------------------------------------
    // Test: add BioSymbol with same symbol character but different member characters and/or threshold
    BioSymbol bioSymbol2('N', "actg", .75);
    x.addBioSymbol(bioSymbol2);
    QVERIFY(x.bioSymbols().contains('N'));
    QCOMPARE(x.bioSymbols().value('N'), bioSymbol2);
    QCOMPARE(x.bioSymbols().count(), 1);

    // ------------------------------------------------------------------------
    // Test: add another bioSymbol
    BioSymbol bioSymbol3('a', "WYF", .6);
    x.addBioSymbol(bioSymbol3);
    QVERIFY(x.bioSymbols().contains('a'));
    QCOMPARE(x.bioSymbols().value('a'), bioSymbol3);
    QCOMPARE(x.bioSymbols().value('N'), bioSymbol2);
    QCOMPARE(x.bioSymbols().count(), 2);
}

void TestBioSymbolGroup::operator_ltlt()
{
    // ------------------------------------------------------------------------
    // Test: basic case
    BioSymbolGroup x;
    BioSymbol bioSymbol('N', "ACTG", .5);
    x << bioSymbol;
    QVERIFY(x.bioSymbols().contains('N'));
    QCOMPARE(x.bioSymbols().count(), 1);
    QCOMPARE(x.bioSymbols().value('N'), bioSymbol);

    // ------------------------------------------------------------------------
    // Test: add BioSymbol with same symbol character but different member characters and/or threshold
    BioSymbol bioSymbol2('N', "actg", .75);
    BioSymbol bioSymbol3('a', "WYF", .6);
    x << bioSymbol2 << bioSymbol3;
    QVERIFY(x.bioSymbols().contains('N'));
    QCOMPARE(x.bioSymbols().value('N'), bioSymbol2);
    QVERIFY(x.bioSymbols().contains('a'));
    QCOMPARE(x.bioSymbols().value('a'), bioSymbol3);
    QCOMPARE(x.bioSymbols().value('N'), bioSymbol2);
    QCOMPARE(x.bioSymbols().count(), 2);
}

void TestBioSymbolGroup::removeBioSymbol()
{
    // Setup
    BioSymbolGroup x;
    x.addBioSymbol(BioSymbol('a', "WFY", .64));
    x.addBioSymbol(BioSymbol('t', "GS", .3));
    x.addBioSymbol(BioSymbol('p', "P", .9));

    QCOMPARE(x.bioSymbols().count(), 3);

    // ------------------------------------------------------------------------
    // Test: remove symbol not in group should do nothing
    x.removeBioSymbol('-');
    x.removeBioSymbol('%');
    x.removeBioSymbol('L');
    QCOMPARE(x.bioSymbols().count(), 3);

    // ------------------------------------------------------------------------
    // Test: Remove each one until empty
    QList<char> symbols;
    symbols << 'a' << 't' << 'p';
    foreach (char symbol, symbols)
    {
        QVERIFY(x.bioSymbols().contains(symbol));
        x.removeBioSymbol(symbol);
        QCOMPARE(x.bioSymbols().contains(symbol), false);
    }

    QVERIFY(x.bioSymbols().isEmpty());
}

void TestBioSymbolGroup::clear()
{
    BioSymbolGroup x;

    // ------------------------------------------------------------------------
    // Test: clear empty list makes no change
    QCOMPARE(x.bioSymbols().count(), 0);
    x.clear();
    QCOMPARE(x.bioSymbols().count(), 0);

    // ------------------------------------------------------------------------
    // Test: basic clear
    x.addBioSymbol(BioSymbol('a', "WFY", .64));
    x.addBioSymbol(BioSymbol('t', "GS", .3));
    x.addBioSymbol(BioSymbol('p', "P", .9));

    QCOMPARE(x.bioSymbols().count(), 3);
    x.clear();
    QCOMPARE(x.bioSymbols().count(), 0);
}

void TestBioSymbolGroup::count()
{
    BioSymbolGroup x;

    // ------------------------------------------------------------------------
    // Test: default group contains no items
    QCOMPARE(x.count(), 0);

    // ------------------------------------------------------------------------
    // Test: add/remove some items and check that count is in sync
    x.addBioSymbol(BioSymbol('a', "WFY", .64));
    QCOMPARE(x.count(), 1);
    x.addBioSymbol(BioSymbol('t', "GS", .3));
    QCOMPARE(x.count(), 2);
    x.removeBioSymbol('-');
    QCOMPARE(x.count(), 2);
    x.removeBioSymbol('a');
    QCOMPARE(x.count(), 1);
    x.addBioSymbol(BioSymbol('p', "P", .9));
    QCOMPARE(x.count(), 2);
    x.removeBioSymbol('t');
    x.removeBioSymbol('p');
    QCOMPARE(x.count(), 0);
}

void TestBioSymbolGroup::isEmpty()
{
    BioSymbolGroup x;

    // ------------------------------------------------------------------------
    // Test: default group contains no items
    QVERIFY(x.isEmpty());

    // ------------------------------------------------------------------------
    // Test: adding an item should make it non-empty
    x.addBioSymbol(BioSymbol('a', "WFY", .64));
    QCOMPARE(x.isEmpty(), false);
    x.removeBioSymbol('a');
    QVERIFY(x.isEmpty());
}

void TestBioSymbolGroup::isCharAssociatedWithSymbol()
{
    BioSymbolGroup x;
    const char *charData = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()_+-=";

    // ------------------------------------------------------------------------
    // Test: empty group should return false for every symbol combination
    const char *i = charData;
    while (*i != '\0')      // character
    {
        const char *j = charData;
        while (*j != '\0')  // symbol
        {
            QCOMPARE(x.isCharAssociatedWithSymbol(*i, *j), false);
            ++j;
        }

        ++i;
    }

    // ------------------------------------------------------------------------
    // Test: one biosymbol
    QSet<char> symbolCharacters;
    symbolCharacters << 'W' << 'F' << 'Y';
    x.addBioSymbol(BioSymbol('a', "WFY", .6));
    i = charData;
    while (*i != '\0')      // character
    {
        const char *j = charData;
        while (*j != '\0')  // symbol
        {
            if (*j != 'a' || !symbolCharacters.contains(*i))
                QCOMPARE(x.isCharAssociatedWithSymbol(*i, *j), false);
            else
                QCOMPARE(x.isCharAssociatedWithSymbol(*i, *j), true);
            ++j;
        }

        ++i;
    }

    // ------------------------------------------------------------------------
    // Test: two biosymbols
    QSet<char> symbolCharacters2;
    symbolCharacters2 << 'G' << 'S';
    x.addBioSymbol(BioSymbol('s', "GS", .8));
    i = charData;
    while (*i != '\0')      // character
    {
        const char *j = charData;
        while (*j != '\0')  // symbol
        {
            if ( (*j != 'a' || !symbolCharacters.contains(*i))          // First bioSymbol
                 && (*j != 's' || !symbolCharacters2.contains(*i)))      // Second bioSymbol
                QCOMPARE(x.isCharAssociatedWithSymbol(*i, *j), false);
            else
                QCOMPARE(x.isCharAssociatedWithSymbol(*i, *j), true);
            ++j;
        }

        ++i;
    }
}

void TestBioSymbolGroup::setDefaultSymbol()
{
    BioSymbolGroup x;

    for (int i=0; i< 255; ++i)
    {
        x.setDefaultSymbol(static_cast<char>(i));
        QCOMPARE(x.defaultSymbol(), static_cast<char>(i));
    }
}

void TestBioSymbolGroup::calculateSymbolString_data()
{
    QTest::addColumn<BioSymbolGroup>("bioSymbolGroup");
    QTest::addColumn<ListHashCharDouble>("listHashCharDouble");
    QTest::addColumn<QString>("expectedSymbolString");

    char defaultSymbol = BioSymbolGroup().defaultSymbol();

    // ------------------------------------------------------------------------
    // Test: completely empty BioSymbolGroup and empty ListHashCharDouble except dimensionally
    //
    // Subtest 1: [ {} ]
    // Subtest 2: [ {}, {}, ]
    // ...
    for (int i=0; i<3; ++i)
    {
        ListHashCharDouble listHashCharDouble;
        for (int j=0; j<i+1; ++j)
            listHashCharDouble << QHash<char, qreal>();

        QTest::newRow(QString("Default BioSymbolGroup, %1 element ListHashCharDouble (empty)").arg(i+1).toAscii())
                << BioSymbolGroup()
                << listHashCharDouble
                << QString(defaultSymbol).repeated(i+1);
    }

    // ------------------------------------------------------------------------
    // Test: completely empty BioSymbolGroup and single element ListHashCharDoubles
    //
    // Subtest 1: [ {A} ]
    // Subtest 2: [ {A}, {B}, ]
    // ...
    for (int i=0; i<3; ++i)
    {
        ListHashCharDouble listHashCharDouble;
        for (int j=0; j<i+1; ++j)
        {
            listHashCharDouble << QHash<char, qreal>();
            listHashCharDouble.last().insert(static_cast<char>(j+65), randomInteger(0, 100) / 100.);
        }

        QTest::newRow(QString("Default BioSymbolGroup, %1 element ListHashCharDouble (empty)").arg(i+1).toAscii())
                << BioSymbolGroup('%')
                << listHashCharDouble
                << QString('%').repeated(i+1);
    }

    // ------------------------------------------------------------------------
    // Test: completely empty BioSymbolGroup and multi-element ListHashCharDoubles
    //
    // Subtest 1: [ {A, B, ... ?} ]
    // Subtest 2: [ {A, ...?}, {B, ... ?}, ]
    // ...
    for (int i=0; i<3; ++i)
    {
        ListHashCharDouble listHashCharDouble;
        for (int j=0; j<i+1; ++j)
        {
            listHashCharDouble << QHash<char, qreal>();
            qreal pool = 100;
            for (int k=0, z=randomInteger(1, 10); k<z; ++k)
            {
                int amount = randomInteger(0, pool);
                listHashCharDouble.last().insert(static_cast<char>(k+65), amount / 100.);
                pool -= amount;
            }
        }

        QTest::newRow(QString("Default BioSymbolGroup, %1 element ListHashCharDouble (empty)").arg(i+1).toAscii())
                << BioSymbolGroup('-')
                << listHashCharDouble
                << QString('-').repeated(i+1);
    }

    // ------------------------------------------------------------------------
    // Test: Single rule, single character must exceed threshold
    //       [ {A => .49},
    //         {A => .5, a => .25, C => .1},
    //         {A => .51},
    //         {A => .4, P => .5, C => .1} ]
    //       Cutoff = A for .5
    {
        ListHashCharDouble listHashCharDouble;
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .49);

        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .50);
        listHashCharDouble.last().insert('a', .25);
        listHashCharDouble.last().insert('C', .1);

        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .51);

        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .4);
        listHashCharDouble.last().insert('P', .5);
        listHashCharDouble.last().insert('C', .1);

        QTest::newRow("Single rule, A => a (.5)")
                << (BioSymbolGroup('_') << BioSymbol('a', "A", .5))
                << listHashCharDouble
                << "_aa_";
    }

    // ------------------------------------------------------------------------
    // Test: single rule, sum of multiple characters must exceed threshold
    {
        ListHashCharDouble listHashCharDouble;
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .25);
        listHashCharDouble.last().insert('B', .24);
        listHashCharDouble.last().insert('C', .5);

        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .49);
        listHashCharDouble.last().insert('B', .01);
        listHashCharDouble.last().insert('a', .25);
        listHashCharDouble.last().insert('C', .1);

        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .51);

        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('B', .5);

        QTest::newRow("Single rule, AB => a (.5)")
                << (BioSymbolGroup('_') << BioSymbol('a', "AB", .5))
                << listHashCharDouble
                << "_aaa";
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
        ListHashCharDouble listHashCharDouble;
        // sum(AB) < .5
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .25);
        listHashCharDouble.last().insert('B', .24);

        // sum(AB) = .5
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .49);
        listHashCharDouble.last().insert('B', .01);
        listHashCharDouble.last().insert('a', .25);
        listHashCharDouble.last().insert('C', .1);

        // sum(AB) > .5
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .51);

        // sum(CD) < .3
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('C', .1);
        listHashCharDouble.last().insert('D', .1);

        // sum(CD) = .3
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('C', .2);
        listHashCharDouble.last().insert('D', .1);

        // sum(CD) > .3
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('C', .3);
        listHashCharDouble.last().insert('D', .3);

        // sum(AB) < .5, sum(CD) < .3
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .07);
        listHashCharDouble.last().insert('B', .03);
        listHashCharDouble.last().insert('C', .05);
        listHashCharDouble.last().insert('D', .1);

        // sum(AB) > .5, sum(CD) < .3
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .7);
        listHashCharDouble.last().insert('B', .01);
        listHashCharDouble.last().insert('C', .05);
        listHashCharDouble.last().insert('D', .1);

        // sum(AB) < .5, sum(CD) > .3
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .3);
        listHashCharDouble.last().insert('B', .1);
        listHashCharDouble.last().insert('C', .25);
        listHashCharDouble.last().insert('D', .2);

        // sum(AB) > .5, sum(CD) > .3
        // Result should be the AB rule wins because it has the same number of characters and a higher threshold
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .3);
        listHashCharDouble.last().insert('B', .25);
        listHashCharDouble.last().insert('C', .15);
        listHashCharDouble.last().insert('D', .2);

        QTest::newRow("Two rules: 1) AB => a (.5); 2) CD => c (.3)")
                << (BioSymbolGroup('_') << BioSymbol('a', "AB", .5) << BioSymbol('c', "CD", .3))
                << listHashCharDouble
                << "_aa_cc_aca";
    }

    // ------------------------------------------------------------------------
    // Test: Two rules, same number of characters, same thresholds
    {
        ListHashCharDouble listHashCharDouble;

        // sum(AB) = .5, sum(CD) = .4; both rules match, but since sum(AB) / 2 = .25 > sum(CD) / 2 = .2, AB wins
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .3);
        listHashCharDouble.last().insert('B', .2);
        listHashCharDouble.last().insert('C', .15);
        listHashCharDouble.last().insert('D', .25);

        // sum(AB) == sum(CD) == .3; tie, first rule wins
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .15);
        listHashCharDouble.last().insert('B', .15);
        listHashCharDouble.last().insert('C', .15);
        listHashCharDouble.last().insert('D', .15);

        // sum(AB) = .4, sum(CD) = .6; both rules match, but since sum(AB) / 2 = .2 < sum(CD) / 2 = .3, CD wins
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .3);
        listHashCharDouble.last().insert('B', .1);
        listHashCharDouble.last().insert('C', .3);
        listHashCharDouble.last().insert('D', .3);

        QTest::newRow("Two rules: 1) AB => a (.3); 2) CD => c (.3)")
                << (BioSymbolGroup('_') << BioSymbol('a', "AB", .3) << BioSymbol('c', "CD", .3))
                << listHashCharDouble
                << "aac";
    }

    // ------------------------------------------------------------------------
    // Test: Two rules, different number of characters, same thresholds
    {
        ListHashCharDouble listHashCharDouble;

        // sum(AB) == sum(C) == .3; C wins
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .2);
        listHashCharDouble.last().insert('B', .1);
        listHashCharDouble.last().insert('C', .3);

        // sum(AB) = .59, sum(CD) = .3; C wins
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .29);
        listHashCharDouble.last().insert('B', .3);
        listHashCharDouble.last().insert('C', .3);

        // sum(AB) = .6, sum(CD) = .3; AB wins because of tie and it is first rule
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('A', .3);
        listHashCharDouble.last().insert('B', .3);
        listHashCharDouble.last().insert('C', .3);

        // sum(AB) = .7 sum(CD) = .3; AB wins
        listHashCharDouble << QHash<char, qreal>();
        listHashCharDouble.last().insert('B', .7);
        listHashCharDouble.last().insert('C', .3);

        QTest::newRow("Two rules: 1) AB => a (.3); 2) C => c (.3)")
                << (BioSymbolGroup('_') << BioSymbol('a', "AB", .3) << BioSymbol('c', "C", .3))
                << listHashCharDouble
                << "ccaa";
    }
}

void TestBioSymbolGroup::calculateSymbolString()
{
    QFETCH(BioSymbolGroup, bioSymbolGroup);
    QFETCH(ListHashCharDouble, listHashCharDouble);
    QFETCH(QString, expectedSymbolString);

    QCOMPARE(bioSymbolGroup.calculateSymbolString(listHashCharDouble), expectedSymbolString);
}

QTEST_APPLESS_MAIN(TestBioSymbolGroup)
#include "TestBioSymbolGroup.moc"
