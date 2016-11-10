/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../BioSymbolColorScheme.h"
#include "../CharColorScheme.h"
#include "../../core/BioSymbolGroup.h"
#include "../../core/BioSymbol.h"

class TestBioSymbolColorScheme : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void setBioSymbolGroup();
    void setCharColorScheme();
    void setSymbolColorStyle(); // Also tests symbolColorStyle
    void symbolColorStyles();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestBioSymbolColorScheme::constructor()
{
    // ------------------------------------------------------------------------
    // Test: constructed BioSymbolGroup and CharColorScheme
    {
        BioSymbolColorScheme x;
        QCOMPARE(x.bioSymbolGroup().count(), 0);
        QCOMPARE(x.charColorScheme(), CharColorScheme());
    }

    {
        BioSymbolGroup bioSymbolGroup;
        bioSymbolGroup << BioSymbol('a', "ILV", .5)
                       << BioSymbol('f', "FWY", .7);

        BioSymbolColorScheme x(bioSymbolGroup);
        QCOMPARE(x.bioSymbolGroup().bioSymbols(), bioSymbolGroup.bioSymbols());
        QCOMPARE(x.charColorScheme(), CharColorScheme());
    }

    {
        BioSymbolGroup bioSymbolGroup;
        bioSymbolGroup << BioSymbol('a', "ILV", .5)
                       << BioSymbol('f', "FWY", .7);
        CharColorScheme charColorScheme(TextColorStyle(Qt::red, Qt::green));

        BioSymbolColorScheme x(bioSymbolGroup, charColorScheme);
        QCOMPARE(x.bioSymbolGroup().bioSymbols(), bioSymbolGroup.bioSymbols());
        QCOMPARE(x.charColorScheme(), charColorScheme);
    }
}

void TestBioSymbolColorScheme::setBioSymbolGroup()
{
    // ------------------------------------------------------------------------
    // Test: Setting the BioSymbolGroup
    BioSymbolGroup bioSymbolGroup;
    bioSymbolGroup << BioSymbol('a', "ILV", .5)
                   << BioSymbol('f', "FWY", .7);

    BioSymbolColorScheme x;
    x.setBioSymbolGroup(bioSymbolGroup);
    QCOMPARE(x.bioSymbolGroup().bioSymbols(), bioSymbolGroup.bioSymbols());

    BioSymbolGroup bioSymbolGroup2;
    bioSymbolGroup2 << BioSymbol('+', "KR", .5)
                    << BioSymbol('-', "DE", .7);
    x.setBioSymbolGroup(bioSymbolGroup2);
    QCOMPARE(x.bioSymbolGroup().bioSymbols(), bioSymbolGroup2.bioSymbols());
}

void TestBioSymbolColorScheme::setCharColorScheme()
{
    // ------------------------------------------------------------------------
    // Test: Setting the CharColorScheme
    CharColorScheme charColorScheme(TextColorStyle(Qt::red, Qt::white));

    BioSymbolColorScheme x;
    x.setCharColorScheme(charColorScheme);
    QCOMPARE(x.charColorScheme(), charColorScheme);

    CharColorScheme charColorScheme2(TextColorStyle(Qt::blue, Qt::black));
    charColorScheme2.setTextColorStyle('a', TextColorStyle(Qt::green));
    x.setCharColorScheme(charColorScheme2);
    QCOMPARE(x.charColorScheme(), charColorScheme2);
}

void TestBioSymbolColorScheme::setSymbolColorStyle()
{
    BioSymbolColorScheme x;

    // ------------------------------------------------------------------------
    // Test: with no symbol associated colors symbolColorStyle should always return the default color style
    for (int i=0; i< 255; ++i)
    {
        char ch = static_cast<char>(i);
        QCOMPARE(x.symbolColorStyle(ch, '$'), x.charColorScheme().defaultTextColorStyle());
    }

    // ------------------------------------------------------------------------
    // Test: setting the symbol color style for a specific symbol, no BioSymbolGroup
    x.setSymbolColorStyle('P', TextColorStyle(Qt::red));
    QCOMPARE(x.symbolColorStyle('P', '$'), x.charColorScheme().defaultTextColorStyle());

    // ------------------------------------------------------------------------
    // Test: no char color scheme, BioSymbolGroup
    QCOMPARE(x.symbolColorStyle('F', 'a'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('Y', 'a'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('W', 'a'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('H', 'a'), x.charColorScheme().defaultTextColorStyle());

    BioSymbolGroup bioSymbolGroup;
    bioSymbolGroup << BioSymbol('a', "FYWH", .6);
    TextColorStyle tcs(Qt::green, Qt::black);
    x.setBioSymbolGroup(bioSymbolGroup);
    x.setSymbolColorStyle('a', tcs);
    QCOMPARE(x.symbolColorStyle('F', 'a'), tcs);
    QCOMPARE(x.symbolColorStyle('Y', 'a'), tcs);
    QCOMPARE(x.symbolColorStyle('W', 'a'), tcs);
    QCOMPARE(x.symbolColorStyle('H', 'a'), tcs);

    QCOMPARE(x.symbolColorStyle('F', 'p'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('F', '_'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('F', '$'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('F', 'E'), x.charColorScheme().defaultTextColorStyle());

    QCOMPARE(x.symbolColorStyle('Y', 'p'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('Y', '_'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('Y', '$'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('Y', 'E'), x.charColorScheme().defaultTextColorStyle());

    QCOMPARE(x.symbolColorStyle('W', 'p'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('W', '_'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('W', '$'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('W', 'E'), x.charColorScheme().defaultTextColorStyle());

    QCOMPARE(x.symbolColorStyle('H', 'p'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('H', '_'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('H', '$'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('H', 'E'), x.charColorScheme().defaultTextColorStyle());

    // ------------------------------------------------------------------------
    // Test: Char Color Scheme, BioSymbolGroup
    CharColorScheme charColorScheme(TextColorStyle(Qt::white, Qt::black));
    TextColorStyle fCS(Qt::blue, Qt::green);
    charColorScheme.setTextColorStyle('F', fCS);
    x.setCharColorScheme(charColorScheme);

    QCOMPARE(x.symbolColorStyle('F', 'a'), fCS);
    QCOMPARE(x.symbolColorStyle('F', '%'), fCS);
    QCOMPARE(x.symbolColorStyle('F', 'P'), fCS);
    QCOMPARE(x.symbolColorStyle('F', '_'), fCS);

    QCOMPARE(x.symbolColorStyle('Y', 'a'), tcs);
    QCOMPARE(x.symbolColorStyle('W', 'a'), tcs);
    QCOMPARE(x.symbolColorStyle('H', 'a'), tcs);

    // ------------------------------------------------------------------------
    // Test: reset the BioSymbolGroup
    x.setBioSymbolGroup(BioSymbolGroup());
    QCOMPARE(x.symbolColorStyle('F', 'a'), fCS);
    QCOMPARE(x.symbolColorStyle('F', '%'), fCS);
    QCOMPARE(x.symbolColorStyle('F', 'P'), fCS);
    QCOMPARE(x.symbolColorStyle('F', '_'), fCS);

    QCOMPARE(x.symbolColorStyle('W', 'p'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('Y', '_'), x.charColorScheme().defaultTextColorStyle());
    QCOMPARE(x.symbolColorStyle('H', '$'), x.charColorScheme().defaultTextColorStyle());
}

void TestBioSymbolColorScheme::symbolColorStyles()
{
    BioSymbolColorScheme x;
    QVERIFY(x.symbolColorStyles().isEmpty());

    TextColorStyle tcs(Qt::green, Qt::black);
    x.setSymbolColorStyle('a', tcs);

    QCOMPARE(x.symbolColorStyles().count(), 1);
    QCOMPARE(x.symbolColorStyles().value('a'), tcs);

    TextColorStyle fCS(Qt::blue, Qt::green);
    x.setSymbolColorStyle('&', fCS);

    QCOMPARE(x.symbolColorStyles().count(), 2);
    QCOMPARE(x.symbolColorStyles().value('a'), tcs);
    QCOMPARE(x.symbolColorStyles().value('&'), fCS);

    x.setSymbolColorStyle('&', tcs);
    QCOMPARE(x.symbolColorStyles().count(), 2);
    QCOMPARE(x.symbolColorStyles().value('a'), tcs);
    QCOMPARE(x.symbolColorStyles().value('&'), tcs);
}

QTEST_APPLESS_MAIN(TestBioSymbolColorScheme)
#include "TestBioSymbolColorScheme.moc"
