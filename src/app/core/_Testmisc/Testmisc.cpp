/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QByteArray>

#include "../util/Rect.h"
#include "../constants.h"
#include "../misc.h"
#include "../types.h"

class Testmisc : public QObject
{
    Q_OBJECT

private slots:
    void isGapCharacter();
    void removeWhiteSpace();
    void convertIntVectorToRanges_data();
    void convertIntVectorToRanges();
    void randomInteger();
    void divideVectorHashCharInt_data();
    void divideVectorHashCharInt();
    void floorPoint();
    void round();
};

typedef QVector<int> IntVector;
typedef QVector<QPair<int, int> > IntPairVector;

Q_DECLARE_METATYPE(IntVector);
Q_DECLARE_METATYPE(IntPairVector);
Q_DECLARE_METATYPE(VectorHashCharInt);
Q_DECLARE_METATYPE(VectorHashCharDouble);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void Testmisc::isGapCharacter()
{
    // ------------------------------------------------------------------------
    // Test: loop through all characters and test whether they should be gaps or not
    for (int i=0; i< 256; ++i)
    {
        char ch = i;
        bool shouldBeGap = false;
        const char *x = constants::kGapCharacters;
        for (; *x; ++x)
        {
            if (ch == *x)
            {
                shouldBeGap = true;
                break;
            }
        }

        QCOMPARE(::isGapCharacter(ch), shouldBeGap);
    }
}

void Testmisc::removeWhiteSpace()
{
    QByteArray buffer = " A B\tC\nD\vE\fF\r";
    ::removeWhiteSpace(buffer);
    QCOMPARE(buffer, QByteArray("ABCDEF"));
}

void Testmisc::convertIntVectorToRanges_data()
{
    QTest::addColumn<IntVector>("intVector");
    QTest::addColumn<IntPairVector>("intPairVector");

    QTest::newRow("IntVector()") << IntVector() << IntPairVector();
    QTest::newRow("IntVector(5)") << (IntVector() << 5) << (IntPairVector() << qMakePair(5, 5));
    QTest::newRow("IntVector(1, 1)") << (IntVector() << 1 << 1) << (IntPairVector() << qMakePair(1, 1));
    QTest::newRow("IntVector(1, 2)") << (IntVector() << 1 << 2) << (IntPairVector() << qMakePair(1, 2));
    QTest::newRow("IntVector(1, 2, 3)") << (IntVector() << 1 << 2 << 3) << (IntPairVector() << qMakePair(1, 3));
    QTest::newRow("IntVector(1, 3, 4)") << (IntVector() << 1 << 3 << 4) << (IntPairVector() << qMakePair(1, 1) << qMakePair(3, 4));
    QTest::newRow("IntVector(1, 2, 3, 4, 10, 20, 21, 22, 23)") << (IntVector() << 1 << 2 << 3 << 4 << 10 << 20 << 21 << 22 << 23) << (IntPairVector() << qMakePair(1, 4) << qMakePair(10, 10) << qMakePair(20, 23));
    QTest::newRow("IntVector(2, 1)") << (IntVector() << 2 << 1) << (IntPairVector() << qMakePair(1, 2));
    QTest::newRow("IntVector(3, 2, 1)") << (IntVector() << 3 << 2 << 1) << (IntPairVector() << qMakePair(1, 3));
    QTest::newRow("IntVector(-5, -8, 0, 11, -2, -4, -6, 10)") << (IntVector() << -5 << -8 << 0 << 11 << -2 << -4 << -6 << 10) << (IntPairVector() << qMakePair(-8, -8) << qMakePair(-6, -4) << qMakePair(-2, -2) << qMakePair(0, 0) << qMakePair(10, 11));
//    QTest::newRow("IntVector()") << (IntVector() << ) << (IntPairVector() << qMakePair());
}


void Testmisc::convertIntVectorToRanges()
{
    QFETCH(IntVector, intVector);
    QFETCH(IntPairVector, intPairVector);

    QCOMPARE(::convertIntVectorToRanges(intVector), intPairVector);
}

void Testmisc::randomInteger()
{
    // ------------------------------------------------------------------------
    // Test: Walk each element of a diagonal 1000x1000 matrix and test that randomInteger
    //       returns a value between these bounds
    for (int i=0; i< 1000; ++i)
    {
        for (int j=i+1; j< 1000; ++j)
        {
            int x = ::randomInteger(i, j);
            QVERIFY(x >= i && x <= j);
        }
    }
}

void Testmisc::divideVectorHashCharInt_data()
{
    QTest::addColumn<VectorHashCharInt>("vectorHashCharInt");
    QTest::addColumn<int>("divisor");
    QTest::addColumn<VectorHashCharDouble>("expectedResult");

    // i -> # of columns in listHashCharInt
    for (int divisor=-10; divisor <=10; ++divisor)
    {
        if (divisor == 0)
            continue;

        for (int i=1; i<=3; ++i)
        {
            VectorHashCharInt vectorHashCharInt;
            VectorHashCharDouble vectorHashCharDouble;
            for (int j=0; j< i; ++j)
            {
                vectorHashCharInt << QHash<char, int>();
                vectorHashCharDouble << QHash<char, qreal>();
                int nMax = ::randomInteger(1, 5);
                for (int n=-nMax; n<nMax; ++n)
                {
                    char ch = 'A' + n + nMax;
                    vectorHashCharInt.last().insert(ch, n);
                    vectorHashCharDouble.last().insert(ch, static_cast<qreal>(n) / static_cast<qreal>(divisor));
                }
            }

            QTest::newRow(QString("Columns: %1, Divisor: %2").arg(i).arg(divisor).toAscii())
                    << vectorHashCharInt
                    << divisor
                    << vectorHashCharDouble;
        }
    }
}

void Testmisc::divideVectorHashCharInt()
{
    QFETCH(VectorHashCharInt, vectorHashCharInt);
    QFETCH(int, divisor);
    QFETCH(VectorHashCharDouble, expectedResult);

    QCOMPARE(::divideVectorHashCharInt(vectorHashCharInt, divisor), expectedResult);
}

void Testmisc::floorPoint()
{
    QCOMPARE(::floorPoint(QPointF(.5, 0)), QPoint(0, 0));
    QCOMPARE(::floorPoint(QPointF(0, .5)), QPoint(0, 0));
    QCOMPARE(::floorPoint(QPointF(.5, .5)), QPoint(0, 0));

    QCOMPARE(::floorPoint(QPointF(5.3, 0)), QPoint(5, 0));
    QCOMPARE(::floorPoint(QPointF(0, 5.3)), QPoint(0, 5));
    QCOMPARE(::floorPoint(QPointF(5.3, 5.3)), QPoint(5, 5));

    QCOMPARE(::floorPoint(QPointF(10.9, 0)), QPoint(10, 0));
    QCOMPARE(::floorPoint(QPointF(0, 10.9)), QPoint(0, 10));
    QCOMPARE(::floorPoint(QPointF(10.9, 10.9)), QPoint(10, 10));
}

void Testmisc::round()
{
    QVERIFY(qFuzzyCompare(::round(5.4, 0), 5.));
    QVERIFY(qFuzzyCompare(::round(5.5, 0), 6.));

    QVERIFY(qFuzzyCompare(::round(5.0, 1), 5.0));
    QVERIFY(qFuzzyCompare(::round(5.5, 1), 5.5));
    QVERIFY(qFuzzyCompare(::round(5.52, 1), 5.5));
    QVERIFY(qFuzzyCompare(::round(5.55, 1), 5.6));
    QVERIFY(qFuzzyCompare(::round(5.49, 1), 5.5));
    QVERIFY(qFuzzyCompare(::round(5.44, 1), 5.4));
    QVERIFY(qFuzzyCompare(::round(5.445, 1), 5.4));
}

QTEST_APPLESS_MAIN(Testmisc)
#include "Testmisc.moc"
