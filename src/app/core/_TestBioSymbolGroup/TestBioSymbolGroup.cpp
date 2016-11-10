/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../BioSymbol.h"
#include "../BioSymbolGroup.h"

class TestBioSymbolGroup : public QObject
{
    Q_OBJECT

private slots:
    void addBioSymbol();
    void operator_ltlt();
    void removeBioSymbol();
    void bioSymbolVector();
    void clear();
    void count();
    void isEmpty();
    void isCharAssociatedWithSymbol();
    void serialNumber();
    void setThresholdForAllBioSymbols();
    void symbolsInSerialOrder();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
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

void TestBioSymbolGroup::bioSymbolVector()
{
    BioSymbol one('N', "ACTG", .5);
    BioSymbol two('%', "ACDEF", .3);
    BioSymbol three('Z', "", 1.);
    BioSymbol four('a', "", 0.);

    BioSymbolGroup x;

    QVERIFY(x.bioSymbolVector().isEmpty());
    x << one << two;
    QCOMPARE(x.bioSymbolVector(), QVector<BioSymbol>() << one << two);
    x << three << four;
    QCOMPARE(x.bioSymbolVector(), QVector<BioSymbol>() << one << two << three << four);
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

void TestBioSymbolGroup::serialNumber()
{
    BioSymbolGroup x;

    x << BioSymbol('N', "ACTG", .5)
      << BioSymbol('%', "ACDEF", .3);

    QVERIFY(x.serialNumber('N') < x.serialNumber('%'));
    QCOMPARE(x.serialNumber('@'), 0);
}

void TestBioSymbolGroup::setThresholdForAllBioSymbols()
{
    BioSymbolGroup x;
    x << BioSymbol('N', "ACTG", .5)
      << BioSymbol('%', "ACDEF", .3);

    x.setThresholdForAllBioSymbols(.2);

    QCOMPARE(x.bioSymbols().value('N').threshold(), .2);
    QCOMPARE(x.bioSymbols().value('%').threshold(), .2);
}

void TestBioSymbolGroup::symbolsInSerialOrder()
{
    BioSymbolGroup x;

    QVERIFY(x.symbolsInSerialOrder().isEmpty());
    x << BioSymbol('N', "ACTG", .5)
      << BioSymbol('%', "ACDEF", .3);
    QCOMPARE(x.symbolsInSerialOrder(), QVector<char>() << 'N' << '%');
    x << BioSymbol('Z', "", 1.);
    x << BioSymbol('a', "", 0.);
    QCOMPARE(x.symbolsInSerialOrder(), QVector<char>() << 'N' << '%' << 'Z' << 'a');
}

QTEST_APPLESS_MAIN(TestBioSymbolGroup)
#include "TestBioSymbolGroup.moc"
