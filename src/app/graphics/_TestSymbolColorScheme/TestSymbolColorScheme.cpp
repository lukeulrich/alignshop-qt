/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../SymbolColorScheme.h"
#include "../TextColorStyle.h"

class TestSymbolColorScheme : public QObject
{
    Q_OBJECT

private slots:
    void defaultTextColorStyle();
    void setSymbolsTextColorStyle();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestSymbolColorScheme::defaultTextColorStyle()
{
    SymbolColorScheme x;

    QCOMPARE(x.textColorStyle('A', '%'), TextColorStyle());

    TextColorStyle style(Qt::green, Qt::red);
    SymbolColorScheme x2(style);
    QCOMPARE(x2.textColorStyle('A', '%'), style);
}

void TestSymbolColorScheme::setSymbolsTextColorStyle()
{
    // Setup
    TextColorStyle defaultStyle(Qt::green, Qt::red);
    TextColorStyle whiteBlue(Qt::white, Qt::blue);
    TextColorStyle yellowGray(Qt::yellow, Qt::gray);
    TextColorStyle cyanBlack(Qt::cyan, Qt::black);

    // ------------------------------------------------------------------------
    // Test: setting/getting single character color styles
    {
        SymbolColorScheme x(defaultStyle);

        x.setSymbolsTextColorStyle('A', "%", whiteBlue);
        QCOMPARE(x.textColorStyle('A', '%'), whiteBlue);
        QCOMPARE(x.textColorStyle('B', '%'), defaultStyle);

        x.setSymbolsTextColorStyle('B', "%", yellowGray);
        QCOMPARE(x.textColorStyle('B', '%'), yellowGray);
        QCOMPARE(x.textColorStyle('A', '#'), defaultStyle);
    }

    // ------------------------------------------------------------------------
    // Test: setting/getting symbol sets
    {
        SymbolColorScheme x(defaultStyle);

        x.setSymbolsTextColorStyle('A', "%#$A", whiteBlue);
        QCOMPARE(x.textColorStyle('A', '%'), whiteBlue);
        QCOMPARE(x.textColorStyle('A', '#'), whiteBlue);
        QCOMPARE(x.textColorStyle('A', '$'), whiteBlue);
        QCOMPARE(x.textColorStyle('A', 'A'), whiteBlue);
        QCOMPARE(x.textColorStyle('A', '}'), defaultStyle);

        x.setSymbolsTextColorStyle('B', "b", yellowGray);
        QCOMPARE(x.textColorStyle('B', 'b'), yellowGray);
        QCOMPARE(x.textColorStyle('B', 'B'), defaultStyle);
    }

    // ------------------------------------------------------------------------
    // Test: overriding previously defined colors
    {
        SymbolColorScheme x(defaultStyle);

        x.setSymbolsTextColorStyle('A', "%#", whiteBlue);
        QCOMPARE(x.textColorStyle('A', '%'), whiteBlue);
        QCOMPARE(x.textColorStyle('A', '#'), whiteBlue);

        x.setSymbolsTextColorStyle('A', "#*", cyanBlack);
        QCOMPARE(x.textColorStyle('A', '%'), whiteBlue);
        QCOMPARE(x.textColorStyle('A', '*'), cyanBlack);
        QCOMPARE(x.textColorStyle('A', '#'), cyanBlack);
    }

    // ------------------------------------------------------------------------
    // Test: Default text color for single character but not a symbol combination
    {
        SymbolColorScheme x(defaultStyle);

        x.setTextColorStyle('A', whiteBlue);
        QCOMPARE(x.textColorStyle('A', '%'), whiteBlue);
        QCOMPARE(x.textColorStyle('A', '#'), whiteBlue);

        x.setSymbolsTextColorStyle('A', "%", cyanBlack);
        QCOMPARE(x.textColorStyle('A', '%'), cyanBlack);
        QCOMPARE(x.textColorStyle('A', '*'), whiteBlue);
    }
}

QTEST_APPLESS_MAIN(TestSymbolColorScheme)
#include "TestSymbolColorScheme.moc"
