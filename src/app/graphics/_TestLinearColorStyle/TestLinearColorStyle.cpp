/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../LinearColorStyle.h"

class TestLinearColorStyle : public QObject
{
    Q_OBJECT

private slots:
    void constructorDefault();
    void constructorAlpha();
    void constructorBeta();
    void linearColorDiscrete();
    void linearColorContinuous();
    void setRange();
    void setIncrements();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestLinearColorStyle::constructorDefault()
{
    LinearColorStyle x;

    QCOMPARE(x.increments(), 0);
    QCOMPARE(x.isContinuous(), true);
    QCOMPARE(x.isDiscrete(), false);

    QCOMPARE(x.range(), PairDouble(0., 1.));
    QCOMPARE(x.colors(), PairQColor(QColor::fromHsvF(0., 0., 0.), QColor::fromHsvF(0., 0., 1.)));
    QCOMPARE(x.linearColor(0.), QColor::fromHsvF(0., 0., 0.));
    QCOMPARE(x.linearColor(1.), QColor::fromHsvF(0., 0., 1.));
}

void TestLinearColorStyle::constructorAlpha()
{
    LinearColorStyle x(qMakePair(QColor::fromHsvF(.9, .7, .5), .4),
                       qMakePair(QColor::fromHsvF(.1, .2, .3), .8),
                       3);

    QCOMPARE(x.increments(), 3);
    QCOMPARE(x.isContinuous(), false);
    QCOMPARE(x.isDiscrete(), true);

    QCOMPARE(x.range(), PairDouble(.4, .8));
    QCOMPARE(x.colors(), PairQColor(QColor::fromHsvF(.9, .7, .5), QColor::fromHsvF(.1, .2, .3)));
    QCOMPARE(x.linearColor(.3), QColor::fromHsvF(.9, .7, .5));
    QCOMPARE(x.linearColor(.4), QColor::fromHsvF(.9, .7, .5));
    QCOMPARE(x.linearColor(.8), QColor::fromHsvF(.1, .2, .3));
    QCOMPARE(x.linearColor(.9), QColor::fromHsvF(.1, .2, .3));
}

void TestLinearColorStyle::constructorBeta()
{
    LinearColorStyle x(PairQColor(QColor::fromHsvF(.9, .7, .5), QColor::fromHsvF(.1, .2, .3)),
                       3,
                       PairDouble(.4, .8));

    QCOMPARE(x.increments(), 3);
    QCOMPARE(x.isContinuous(), false);
    QCOMPARE(x.isDiscrete(), true);

    QCOMPARE(x.range(), PairDouble(.4, .8));
    QCOMPARE(x.colors(), PairQColor(QColor::fromHsvF(.9, .7, .5), QColor::fromHsvF(.1, .2, .3)));
    QCOMPARE(x.linearColor(.3), QColor::fromHsvF(.9, .7, .5));
    QCOMPARE(x.linearColor(.4), QColor::fromHsvF(.9, .7, .5));
    QCOMPARE(x.linearColor(.8), QColor::fromHsvF(.1, .2, .3));
    QCOMPARE(x.linearColor(.9), QColor::fromHsvF(.1, .2, .3));
}

void TestLinearColorStyle::linearColorDiscrete()
{
    QColor start = QColor::fromHsvF(0., .5, 1.);
    QColor stop = QColor::fromHsvF(1., .5, 0.);

    LinearColorStyle x(PairQColor(start, stop), 5);

    QCOMPARE(x.range(), PairDouble(0., 1.));

    // Test: below bottom range should be clamped to start color
    QCOMPARE(x.linearColor(-1), start);

    // Test: 0
    QCOMPARE(x.linearColor(0), start);

    // Test: .1 -> .9
    QCOMPARE(x.linearColor(.1), start);
    QCOMPARE(x.linearColor(.199999), start);
    QCOMPARE(x.linearColor(.2),  QColor::fromHsvF(.25, .5, .75));
    QCOMPARE(x.linearColor(.3),  QColor::fromHsvF(.25, .5, .75));
    QCOMPARE(x.linearColor(.4),  QColor::fromHsvF(.50, .5, .5));
    QCOMPARE(x.linearColor(.5),  QColor::fromHsvF(.50, .5, .5));
    QCOMPARE(x.linearColor(.6),  QColor::fromHsvF(.75, .5, .25));
    QCOMPARE(x.linearColor(.7),  QColor::fromHsvF(.75, .5, .25));
    QCOMPARE(x.linearColor(.7999),  QColor::fromHsvF(.75, .5, .25));
    QCOMPARE(x.linearColor(.8), stop);
    QCOMPARE(x.linearColor(.9), stop);

    // Test: 1
    QCOMPARE(x.linearColor(1), stop);

    // Test: exceed top range should be clamped to stop color
    QCOMPARE(x.linearColor(2), stop);
}

void TestLinearColorStyle::linearColorContinuous()
{
    QColor start = QColor::fromHsvF(0., .5, 1.);
    QColor stop = QColor::fromHsvF(1., .5, 0.);

    LinearColorStyle x(PairQColor(start, stop));
    QVERIFY(x.isContinuous());

    for (qreal i=0; i<= 1.; i += .13)
        QCOMPARE(x.linearColor(i), QColor::fromHsvF(i, .5, 1. - i));
}

void TestLinearColorStyle::setRange()
{
    QColor start = QColor::fromHsvF(0., .5, 1.);
    QColor stop = QColor::fromHsvF(1., .5, 0.);

    LinearColorStyle x(PairQColor(start, stop));
    QVERIFY(x.isContinuous());

    x.setRange(PairDouble(-1., 2.));
    QCOMPARE(x.range(), PairDouble(-1., 2.));

    for (qreal i=-1; i<= 2.; i += .13)
    {
        qreal v = (i - -1) / 3;
        QCOMPARE(x.linearColor(i), QColor::fromHsvF(v, .5, 1. - v));
    }
}

void TestLinearColorStyle::setIncrements()
{
    QColor start = QColor::fromHsvF(0., .5, 1.);
    QColor stop = QColor::fromHsvF(1., .5, 0.);

    LinearColorStyle x(PairQColor(start, stop));
    QVERIFY(x.isContinuous());
    x.setIncrements(5);
    QVERIFY(x.isDiscrete());

    // Test: below bottom range should be clamped to start color
    QCOMPARE(x.linearColor(-1), start);

    // Test: 0
    QCOMPARE(x.linearColor(0), start);

    // Test: .1 -> .9
    QCOMPARE(x.linearColor(.1), start);
    QCOMPARE(x.linearColor(.199999), start);
    QCOMPARE(x.linearColor(.2),  QColor::fromHsvF(.25, .5, .75));
    QCOMPARE(x.linearColor(.3),  QColor::fromHsvF(.25, .5, .75));
    QCOMPARE(x.linearColor(.4),  QColor::fromHsvF(.50, .5, .5));
    QCOMPARE(x.linearColor(.5),  QColor::fromHsvF(.50, .5, .5));
    QCOMPARE(x.linearColor(.6),  QColor::fromHsvF(.75, .5, .25));
    QCOMPARE(x.linearColor(.7),  QColor::fromHsvF(.75, .5, .25));
    QCOMPARE(x.linearColor(.7999),  QColor::fromHsvF(.75, .5, .25));
    QCOMPARE(x.linearColor(.8), stop);
    QCOMPARE(x.linearColor(.9), stop);

    // Test: 1
    QCOMPARE(x.linearColor(1), stop);

    // Test: exceed top range should be clamped to stop color
    QCOMPARE(x.linearColor(2), stop);
}

QTEST_APPLESS_MAIN(TestLinearColorStyle)
#include "TestLinearColorStyle.moc"
