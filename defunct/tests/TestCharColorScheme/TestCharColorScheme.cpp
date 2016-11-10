/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/QColor>

#include "CharColorScheme.h"

class TestCharColorScheme : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void operator_eqeq_ne();
    void setDefaultColorStyle();
    void setTextColorStyle();
    void hasColorStyleFor();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestCharColorScheme::constructor()
{
    // ------------------------------------------------------------------------
    // Test: default color scheme passed to constructor should be the one returned by defaultTextColorStyle
    {
        CharColorScheme x;
        QCOMPARE(x.defaultTextColorStyle(), TextColorStyle());
    }

    {
        TextColorStyle tcs(Qt::blue, Qt::green);
        CharColorScheme x(tcs);
        QCOMPARE(x.defaultTextColorStyle(), tcs);
    }
}

void TestCharColorScheme::operator_eqeq_ne()
{
    CharColorScheme x;
    CharColorScheme y;

    QVERIFY(x == x);
    QVERIFY(x == y);

    x.setDefaultTextColorStyle(TextColorStyle(Qt::red, Qt::white));
    QVERIFY(x != y);
    y.setDefaultTextColorStyle(TextColorStyle(Qt::red, Qt::white));
    QVERIFY(x == y);

    x.setTextColorStyle('P', TextColorStyle(Qt::blue));
    QVERIFY(x != y);
    y.setTextColorStyle('P', TextColorStyle(Qt::blue));
    QVERIFY(x == y);
}

void TestCharColorScheme::setDefaultColorStyle()
{
    CharColorScheme x;

    // ------------------------------------------------------------------------
    // Test: setting the default color scheme should work as expected
    TextColorStyle tcs(Qt::blue, Qt::green);
    x.setDefaultTextColorStyle(tcs);
    QCOMPARE(x.defaultTextColorStyle(), tcs);

    TextColorStyle tcs2(Qt::red, Qt::white);
    x.setDefaultTextColorStyle(tcs2);
    QCOMPARE(x.defaultTextColorStyle(), tcs2);
}

void TestCharColorScheme::setTextColorStyle()
{
    CharColorScheme x;

    // ------------------------------------------------------------------------
    // Test: setting/getting character color styles
    QCOMPARE(x.textColorStyle('a'), x.defaultTextColorStyle());

    TextColorStyle tcs(Qt::blue, Qt::green);
    x.setTextColorStyle('a', tcs);
    QCOMPARE(x.textColorStyle('a'), tcs);

    TextColorStyle tcs2(Qt::red, Qt::white);
    x.setTextColorStyle('a', tcs2);
    QCOMPARE(x.textColorStyle('a'), tcs2);

    TextColorStyle tcs3(Qt::white, Qt::black);
    x.setTextColorStyle('b', tcs3);
    QCOMPARE(x.textColorStyle('b'), tcs3);
    x.setTextColorStyle('a', tcs2);
    QCOMPARE(x.textColorStyle('a'), tcs2);
}

void TestCharColorScheme::hasColorStyleFor()
{
    CharColorScheme x;

    // ------------------------------------------------------------------------
    // Test: no associated character color styles
    for (int i=0; i< 255; ++i)
    {
        char ch = static_cast<char>(i);
        QCOMPARE(x.hasColorStyleFor(ch), false);
    }

    TextColorStyle tcs(Qt::blue, Qt::green);
    x.setTextColorStyle('a', tcs);
    QVERIFY(x.hasColorStyleFor('a'));
    QCOMPARE(x.hasColorStyleFor('B'), false);
    QCOMPARE(x.hasColorStyleFor('C'), false);
    QCOMPARE(x.hasColorStyleFor('D'), false);

    TextColorStyle tcs2(Qt::red, Qt::white);
    x.setTextColorStyle('B', tcs2);
    QVERIFY(x.hasColorStyleFor('a'));
    QVERIFY(x.hasColorStyleFor('B'));
    QCOMPARE(x.hasColorStyleFor('C'), false);
    QCOMPARE(x.hasColorStyleFor('D'), false);
}

QTEST_APPLESS_MAIN(TestCharColorScheme)
#include "TestCharColorScheme.moc"
