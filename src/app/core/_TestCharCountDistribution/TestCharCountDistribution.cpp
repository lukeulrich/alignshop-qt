/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../CharCountDistribution.h"

class TestCharCountDistribution : public QObject
{
    Q_OBJECT

public:
    TestCharCountDistribution() : QObject()
    {
        qRegisterMetaType<CharCountDistribution>("CharCountDistribution");
        qRegisterMetaType<VectorHashCharInt>("vectorHashCharInt");
    }

private slots:
    void constructor();     // Also tests the length function
    void insertBlanks();
    void remove();
    void add_data();
    void add();
    void addByteArray_data();
    void addByteArray();
    void allColumnsAreEmpty();
    void charPercents();
    void mid();
    void setDivisor();
    void subtract_data();
    void subtract();
    void subtractByteArray_data();
    void subtractByteArray();
    void removeZeroKeyValues_data();
    void removeZeroKeyValues();

private:
    VectorHashCharInt createVectorHashCharInt(bool positive = true) const;
};

Q_DECLARE_METATYPE(VectorHashCharInt);
Q_DECLARE_METATYPE(CharCountDistribution);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
// Creates a distribution with either positive or negative values
// [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
VectorHashCharInt TestCharCountDistribution::createVectorHashCharInt(bool positive) const
{
    VectorHashCharInt data;
    data << QHash<char, int>();
    data.last().insert('A', (positive) ? 2 : -2);
    data.last().insert('C', (positive) ? 2 : -2);
    data << QHash<char, int>();
    data.last().insert('T', (positive) ? 1 : -1);
    data.last().insert('G', (positive) ? 3 : -3);

    return data;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestCharCountDistribution::constructor()
{
    // ------------------------------------------------------------------------
    // Test: empty char counts
    {
        CharCountDistribution x;

        QCOMPARE(x.length(), 0);
        QCOMPARE(x.charCounts(), VectorHashCharInt());
        QCOMPARE(x.divisor(), 0);
    }

    // ------------------------------------------------------------------------
    // Test: 1 column, non-empty char counts
    {
        VectorHashCharInt charCounts;
        charCounts << QHash<char, int>();
        charCounts.last().insert('A', 10);
        charCounts.last().insert('C', 3);

        CharCountDistribution x(charCounts, 5);

        QCOMPARE(x.length(), 1);
        QCOMPARE(x.charCounts(), charCounts);
        QCOMPARE(x.divisor(), 5);
    }

    // ------------------------------------------------------------------------
    // Test: 2 column, non-empty char counts
    {
        VectorHashCharInt charCounts;
        charCounts << QHash<char, int>();
        charCounts.last().insert('A', 10);
        charCounts.last().insert('C', 3);

        charCounts << QHash<char, int>();
        charCounts.last().insert('T', 5);
        charCounts.last().insert('G', 8);

        CharCountDistribution x(charCounts);

        QCOMPARE(x.length(), 2);
        QCOMPARE(x.charCounts(), charCounts);
    }
}

void TestCharCountDistribution::insertBlanks()
{
    // ------------------------------------------------------------------------
    // Test: insert blanks into non-empty distribution
    {
        VectorHashCharInt data = createVectorHashCharInt();
        CharCountDistribution x(data);

        // Test: insert zero before the first column
        x.insertBlanks(1, 0);
        QCOMPARE(x.charCounts(), data);

        // Test: insert in the first column
        VectorHashCharInt expectedX(data);
        expectedX.insert(0, QHash<char, int>());

        x.insertBlanks(1, 1);
        QCOMPARE(x.charCounts(), expectedX);

        // Test: insert at end
        expectedX << (QHash<char, int>());
        x.insertBlanks(4, 1);
        QCOMPARE(x.charCounts(), expectedX);

        // Test: insert multiple in the middle
        expectedX.insert(2, QHash<char, int>());
        expectedX.insert(2, QHash<char, int>());
        x.insertBlanks(3, 2);
        QCOMPARE(x.charCounts(), expectedX);
    }
}

void TestCharCountDistribution::remove()
{
    {
        VectorHashCharInt data = createVectorHashCharInt();
        data.insert(1, QHash<char, int>());
        CharCountDistribution x(data);

        // ------------------------------------------------------------------------
        // Test: removing zero should do nothing
        x.remove(1, 0);
        x.remove(2, 0);
        QCOMPARE(x.charCounts(), data);

        // ------------------------------------------------------------------------
        // Test: remove blank column from middle
        x.remove(2, 1);
        data.remove(1, 1);
        QCOMPARE(x.charCounts(), data);

        // ------------------------------------------------------------------------
        // Test: remove both columns
        x.remove(1, 2);
        QCOMPARE(x.charCounts(), VectorHashCharInt());
    }

    {
        VectorHashCharInt data = createVectorHashCharInt();
        CharCountDistribution x(data);

        // ------------------------------------------------------------------------
        // Test: 1 parameter version
        x.remove(2);
        data.pop_back();
        QCOMPARE(x.charCounts(), data);

        x.remove(1);
        QCOMPARE(x.charCounts(), VectorHashCharInt());
    }
}

void TestCharCountDistribution::add_data()
{
    QTest::addColumn<CharCountDistribution>("x");
    QTest::addColumn<CharCountDistribution>("y");
    QTest::addColumn<int>("offset");
    QTest::addColumn<VectorHashCharInt>("expectedResult");

    // ------------------------------------------------------------------------
    QTest::newRow("adding empty distribution")
            << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
            << CharCountDistribution()
            << 1
            << createVectorHashCharInt();

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt xData;
        xData << QHash<char, int>();
        xData << QHash<char, int>();
        QTest::newRow("adding to distribution with two columns but no values")
                << CharCountDistribution(xData)
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << 1
                << createVectorHashCharInt();
    }

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData << QHash<char, int>();
        yData.last().insert('A', 1);
        yData.last().insert('C', 1);

        VectorHashCharInt expectedData(createVectorHashCharInt());
        expectedData[0].insert('G', 2);
        expectedData[1].insert('A', 1);
        expectedData[1].insert('C', 1);

        QTest::newRow("adding distribution of equal length but completely unique characters")
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) ], [ (A, 1) (C, 1) ]
                << 1
                << expectedData;                                    // [ (A, 2) (C, 2) (G, 2) ], [ (T, 1) (G, 3) (A, 1) (C, 1) ]
    }

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('A', 1);
        yData.last().insert('G', 1);
        yData << QHash<char, int>();
//        yData.last().insert('-', 1);
        yData.last().insert('G', 1);

        VectorHashCharInt expectedData(createVectorHashCharInt());
        expectedData[0]['A'] = 3;
        expectedData[0]['G'] = 1;
        expectedData[1]['G'] = 4;
//        expectedData[1].insert('-', 1);

        QTest::newRow("adding distribution of equal length and partially unique characters")
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (A, 1) (G, 1) ], [ (-, 1) (G, 1) ]
                << 1
                << expectedData;                                    // [ (A, 3) (C, 2) (G, 1) ], [ (T, 1) (G, 4) ]
    }

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData.last().insert('T', 1);

        VectorHashCharInt expectedData(createVectorHashCharInt());
        expectedData[0].insert('G', 2);
        expectedData[0].insert('T', 1);

        QTest::newRow("adding distribution of unequal length with specific offset of 1")
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) (T, 1) ]
                << 1
                << expectedData;                                    // [ (A, 2) (C, 2) (G, 2) (T, 1) ], [ (T, 1) (G, 3) ]
    }

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData.last().insert('T', 1);

        VectorHashCharInt expectedData(createVectorHashCharInt());
        expectedData[1]['G'] = 5;
        expectedData[1]['T'] = 2;

        QTest::newRow("adding distribution of unequal length with specific offset of 2")
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) (T, 1) ]
                << 2
                << expectedData;                                    // [ (A, 2) (C, 2) ], [ (T, 2) (G, 5) ]
    }
}

void TestCharCountDistribution::add()
{
    QFETCH(CharCountDistribution, x);
    QFETCH(CharCountDistribution, y);
    QFETCH(int, offset);
    QFETCH(VectorHashCharInt, expectedResult);

    x.add(y, offset);

    /*
    qDebug() << x.length();
    qDebug() << y.length();
    qDebug() << "Expected:" << expectedResult;
    qDebug() << "Actual:" << x.charCounts();
    */

    QCOMPARE(x.charCounts(), expectedResult);
}

void TestCharCountDistribution::addByteArray_data()
{
    QTest::addColumn<CharCountDistribution>("x");
    QTest::addColumn<QByteArray>("characters");
    QTest::addColumn<char>("skipChar");
    QTest::addColumn<int>("offset");
    QTest::addColumn<VectorHashCharInt>("expectedResult");

    VectorHashCharInt initialSourceData = createVectorHashCharInt();
    CharCountDistribution starter(initialSourceData);
    QTest::newRow("empty string") << starter << QByteArray() << '\0' << 1 << initialSourceData;
    QTest::newRow("solely gaps") << starter << QByteArray("--") << '-' << 1 << initialSourceData;
    QTest::newRow("one gap, offset = 1") << starter << QByteArray("-") << '-' << 1 << initialSourceData;
    QTest::newRow("one gap, offset = 2") << starter << QByteArray("-") << '-' << 2 << initialSourceData;

    {
        VectorHashCharInt expectedResult = initialSourceData;
        ++expectedResult[0]['G'];
        ++expectedResult[1]['A'];
        QTest::newRow("GA") << starter << QByteArray("GA") << '\0' << 1 << expectedResult;
    }

    {
        VectorHashCharInt expectedResult = initialSourceData;
        ++expectedResult[0]['C'];
        QTest::newRow("C") << starter << QByteArray("C") << '\0' << 1 << expectedResult;
    }

    {
        VectorHashCharInt expectedResult = initialSourceData;
        ++expectedResult[1]['T'];
        QTest::newRow("T, offset 2") << starter << QByteArray("T") << '\0' << 2 << expectedResult;
    }

    {
        VectorHashCharInt expectedResult = initialSourceData;
        ++expectedResult[1]['T'];
        QTest::newRow("-T") << starter << QByteArray("-T") << '-' << 1 << expectedResult;
    }

    {
        VectorHashCharInt expectedResult = initialSourceData;
        ++expectedResult[0]['T'];
        QTest::newRow("T-") << starter << QByteArray("T-") << '-' << 1 << expectedResult;
    }
}

void TestCharCountDistribution::addByteArray()
{
    QFETCH(CharCountDistribution, x);
    QFETCH(QByteArray, characters);
    QFETCH(char, skipChar);
    QFETCH(int, offset);
    QFETCH(VectorHashCharInt, expectedResult);

    x.add(characters, skipChar, offset);

    QCOMPARE(x.charCounts(), expectedResult);
}

void TestCharCountDistribution::allColumnsAreEmpty()
{
    // Test: empty char count distribution should not return true for all columns are empty
    {
        CharCountDistribution x;
        QCOMPARE(x.allColumnsAreEmpty(), true);
    }

    // Test: non-empty distribution with one non-gap character
    {
        VectorHashCharInt data;
        data << QHash<char, int>();
        data.last().insert('A', 2);
        CharCountDistribution x(data);
        QCOMPARE(x.allColumnsAreEmpty(), false);
    }

    // Test: distribution with length of one, but no keys
    {
        VectorHashCharInt data;
        data << QHash<char, int>();
        CharCountDistribution x(data);
        QCOMPARE(x.allColumnsAreEmpty(), true);
    }

    // Test: distribution with one non-empty column among others
    {
        VectorHashCharInt data;
        data << QHash<char, int>();
        data << QHash<char, int>();
        data.last().insert('X', 12);
        data << QHash<char, int>();
        CharCountDistribution x(data);
        QCOMPARE(x.allColumnsAreEmpty(), false);
    }
}

void TestCharCountDistribution::charPercents()
{
    // Test: empty range
    {
        CharCountDistribution x;
        QVERIFY(x.charPercents().isEmpty());
    }

    // Test: full range (none defined)
    {
        CharCountDistribution x(createVectorHashCharInt(), 4);
        VectorHashCharDouble y = x.charPercents();
        QCOMPARE(y.size(), 2);
        QCOMPARE(y.first().value('A'), .5);
        QCOMPARE(y.first().value('C'), .5);

        QCOMPARE(y.last().value('T'), .25);
        QCOMPARE(y.last().value('G'), .75);
    }

    // Test: selective range
    {
        CharCountDistribution x(createVectorHashCharInt(), 4);
        VectorHashCharDouble y = x.charPercents(ClosedIntRange(1, 1));
        QCOMPARE(y.size(), 1);
        QCOMPARE(y.first().value('A'), .5);
        QCOMPARE(y.first().value('C'), .5);

        y = x.charPercents(ClosedIntRange(2, 2));
        QCOMPARE(y.size(), 1);
        QCOMPARE(y.first().value('T'), .25);
        QCOMPARE(y.first().value('G'), .75);

        y = x.charPercents(ClosedIntRange(1, 2));
        QCOMPARE(y.size(), 2);
        QCOMPARE(y.first().value('A'), .5);
        QCOMPARE(y.first().value('C'), .5);

        QCOMPARE(y.last().value('T'), .25);
        QCOMPARE(y.last().value('G'), .75);
    }
}

void TestCharCountDistribution::mid()
{
    VectorHashCharInt data;
    data << QHash<char, int>();
    data.last().insert('A', 1);
    data.last().insert('C', 2);
    data << QHash<char, int>();
    data.last().insert('C', 1);
    data.last().insert('T', 1);
    data.last().insert('G', 1);
    data << QHash<char, int>();
    data.last().insert('T', 1);
    data.last().insert('G', 2);
    data << QHash<char, int>();
    data.last().insert('A', 1);
    data.last().insert('C', 1);
    data.last().insert('G', 1);

    CharCountDistribution x(data, 3);

    QCOMPARE(x.mid(ClosedIntRange(1, 4)), x);
    QCOMPARE(x.mid(ClosedIntRange(2, 3)), CharCountDistribution(data.mid(1, 2), 3));
    QCOMPARE(x.mid(ClosedIntRange(4, 4)), CharCountDistribution(data.mid(3, 1), 3));
}

void TestCharCountDistribution::setDivisor()
{
    CharCountDistribution x;

    x.setDivisor(-1);
    QCOMPARE(x.divisor(), -1);

    x.setDivisor(10);
    QCOMPARE(x.divisor(), 10);
}

void TestCharCountDistribution::subtract_data()
{
    QTest::addColumn<CharCountDistribution>("x");
    QTest::addColumn<CharCountDistribution>("y");
    QTest::addColumn<int>("offset");
    QTest::addColumn<VectorHashCharInt>("expectedResult");

    // ------------------------------------------------------------------------
    QTest::newRow("subtracting empty distribution")
            << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
            << CharCountDistribution()
            << 1
            << createVectorHashCharInt();

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt xData;
        xData << QHash<char, int>();
        xData << QHash<char, int>();
        QTest::newRow("subtracting from distribution with two columns but no values")
                << CharCountDistribution(xData)
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << 1
                << createVectorHashCharInt(false); // false means negate the values returned by createVectorHashCharInt
    }

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData << QHash<char, int>();
        yData.last().insert('A', 1);
        yData.last().insert('C', 1);

        VectorHashCharInt expectedData(createVectorHashCharInt());
        expectedData[0].insert('G', -2);
        expectedData[1].insert('A', -1);
        expectedData[1].insert('C', -1);

        QTest::newRow("subtracting distribution of equal length but completely unique characters")
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) ], [ (A, 1) (C, 1) ]
                << 1
                << expectedData;                                    // [ (A, 2) (C, 2) (G, -2) ], [ (T, 1) (G, 3) (A, -1) (C, -1) ]
    }

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('A', 1);
        yData.last().insert('G', 1);
        yData << QHash<char, int>();
//        yData.last().insert('-', 1);
        yData.last().insert('G', 1);

        VectorHashCharInt expectedData(createVectorHashCharInt());
        expectedData[0]['A'] = 1;
        expectedData[0]['G'] = -1;
        expectedData[1]['G'] = 2;
//        expectedData[1].insert('-', -1);

        QTest::newRow("subtracting distribution of equal length and partially unique characters")
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (A, 1) (G, 1) ], [ (-, 1) (G, 1) ]
                << 1
                << expectedData;                                    // [ (A, 1) (C, 2) (G, -1) ], [ (T, 1) (G, 2) ]
    }

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData.last().insert('T', 1);

        VectorHashCharInt expectedData(createVectorHashCharInt());
        expectedData[0].insert('G', -2);
        expectedData[0].insert('T', -1);

        QTest::newRow("subtracting distribution of unequal length with specific offset of 1")
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) (T, 1) ]
                << 1
                << expectedData;                                    // [ (A, 2) (C, 2) (G, -2) (T, -1) ], [ (T, 1) (G, 3) ]
    }

    // ------------------------------------------------------------------------
    {
        VectorHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData.last().insert('T', 1);

        VectorHashCharInt expectedData(createVectorHashCharInt());
        expectedData[1]['G'] = 1;
        expectedData[1]['T'] = 0;

        QTest::newRow("subtracting distribution of unequal length with specific offset of 2")
                << CharCountDistribution(createVectorHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) (T, 1) ]
                << 2
                << expectedData;                                    // [ (A, 2) (C, 2) ], [ (T, 0) (G, 1) ]
    }
}

void TestCharCountDistribution::subtract()
{
    QFETCH(CharCountDistribution, x);
    QFETCH(CharCountDistribution, y);
    QFETCH(int, offset);
    QFETCH(VectorHashCharInt, expectedResult);

    x.subtract(y, offset);

    /*
    qDebug() << x.length();
    qDebug() << y.length();
    qDebug() << "Expected:" << expectedResult;
    qDebug() << "Actual:" << x.charCounts();
    */

    QCOMPARE(x.charCounts(), expectedResult);
}

void TestCharCountDistribution::subtractByteArray_data()
{
    QTest::addColumn<CharCountDistribution>("x");
    QTest::addColumn<QByteArray>("characters");
    QTest::addColumn<char>("skipChar");
    QTest::addColumn<int>("offset");
    QTest::addColumn<VectorHashCharInt>("expectedResult");

    VectorHashCharInt initialSourceData = createVectorHashCharInt();
    CharCountDistribution starter(initialSourceData);
    QTest::newRow("empty string") << starter << QByteArray() << '\0' << 1 << initialSourceData;
    QTest::newRow("solely gaps") << starter << QByteArray("--") << '-' << 1 << initialSourceData;
    QTest::newRow("one gap, offset = 1") << starter << QByteArray("-") << '-' << 1 << initialSourceData;
    QTest::newRow("one gap, offset = 2") << starter << QByteArray("-") << '-' << 2 << initialSourceData;
    {
        VectorHashCharInt expectedResult = initialSourceData;
        --expectedResult[0]['G'];
        --expectedResult[1]['A'];
        QTest::newRow("GA") << starter << QByteArray("GA") << '\0' << 1 << expectedResult;
    }

    {
        VectorHashCharInt expectedResult = initialSourceData;
        --expectedResult[0]['C'];
        QTest::newRow("C") << starter << QByteArray("C") << '\0' << 1 << expectedResult;
    }

    {
        VectorHashCharInt expectedResult = initialSourceData;
        --expectedResult[1]['T'];
        QTest::newRow("T, offset 2") << starter << QByteArray("T") << '\0' << 2 << expectedResult;
    }

    {
        VectorHashCharInt expectedResult = initialSourceData;
        --expectedResult[1]['T'];
        QTest::newRow("-T") << starter << QByteArray("-T") << '-' << 1 << expectedResult;
    }

    {
        VectorHashCharInt expectedResult = initialSourceData;
        --expectedResult[0]['T'];
        QTest::newRow("T-") << starter << QByteArray("T-") << '-' << 1 << expectedResult;
    }
}

void TestCharCountDistribution::subtractByteArray()
{
    QFETCH(CharCountDistribution, x);
    QFETCH(QByteArray, characters);
    QFETCH(char, skipChar);
    QFETCH(int, offset);
    QFETCH(VectorHashCharInt, expectedResult);

    x.subtract(characters, skipChar, offset);

    QCOMPARE(x.charCounts(), expectedResult);
}

void TestCharCountDistribution::removeZeroKeyValues_data()
{
    QTest::addColumn<VectorHashCharInt>("input");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("to");
    QTest::addColumn<VectorHashCharInt>("result");

    VectorHashCharInt baseData;
    baseData.append(QHash<char, int>());
    baseData.last().insert('A', 5);
    baseData.last().insert('C', 3);
    baseData.append(QHash<char, int>());
    baseData.last().insert('G', 8);

    QTest::newRow("default from/to with no zero key values")
            << baseData
            << -1
            << -1
            << baseData;

    VectorHashCharInt derivedData(baseData);
    derivedData.last().insert('T', 0);
    QTest::newRow("default from/to with single zero key value")
            << derivedData
            << -1
            << -1
            << baseData;

    derivedData = baseData;
    derivedData.first().insert('z', 0);
    derivedData.first().insert('O', 0);
    QTest::newRow("default from/to with two key values in one column")
            << derivedData
            << -1
            << -1
            << baseData;

    derivedData = baseData;
    derivedData.first().insert('z', 0);
    derivedData.last().insert('O', 0);
    QTest::newRow("default from/to with two zero key values in different columns")
            << derivedData
            << -1
            << -1
            << baseData;

    QTest::newRow("no zero key values; from starting at 1")
            << baseData
            << 1
            << 0
            << baseData;

    QTest::newRow("no zero key values; from starting at 2")
            << baseData
            << 2
            << 0
            << baseData;

    derivedData = baseData;
    derivedData.first().insert('Z', 0);
    QTest::newRow("first column has single key value; from starting at 1")
            << derivedData
            << 1
            << 0
            << baseData;

    derivedData = baseData;
    derivedData.first().insert('Z', 0);
    QTest::newRow("first column has single key value; from starting at 2")
            << derivedData
            << 2
            << 0
            << derivedData;

    derivedData = baseData;
    derivedData.first().insert('Y', 0);
    derivedData.last().insert('X', 0);
    QTest::newRow("each column has zero key value; from starting at 1")
            << derivedData
            << 1
            << 0
            << baseData;

    derivedData = baseData;
    derivedData.first().insert('X', 0);
    derivedData.last().insert('Y', 0);
    VectorHashCharInt expectedResult(derivedData);
    expectedResult.last().remove('Y');
    QTest::newRow("each column has zero key value; from starting at 2")
            << derivedData
            << 2
            << 0
            << expectedResult;

    QTest::newRow("from/to covering entire region; no zero key values")
            << baseData
            << 1
            << 2
            << baseData;

    QTest::newRow("from = to = 1; no zero key values")
            << baseData
            << 1
            << 1
            << baseData;

    QTest::newRow("from = to = 2; no zero key values")
            << baseData
            << 2
            << 2
            << baseData;

    derivedData = baseData;
    derivedData.first().insert('Y', 0);
    derivedData.last().insert('Z', 0);
    QTest::newRow("from/to covering entire region; zero value in each column")
            << derivedData
            << 1
            << 2
            << baseData;

    expectedResult = derivedData;
    expectedResult.first().remove('Y');
    QTest::newRow("from = to = 1; zero value in each column")
            << derivedData
            << 1
            << 1
            << expectedResult;

    expectedResult = derivedData;
    expectedResult.last().remove('Z');
    QTest::newRow("from = to = 2; zero value in each column")
            << derivedData
            << 2
            << 2
            << expectedResult;
}

void TestCharCountDistribution::removeZeroKeyValues()
{
    QFETCH(VectorHashCharInt, input);
    QFETCH(int, from);
    QFETCH(int, to);
    QFETCH(VectorHashCharInt, result);

    CharCountDistribution x(input);
    if (from == -1 && to == -1)
        x.removeZeroValueKeys();
    else
        x.removeZeroValueKeys(from, to);

    /*
    if (x.charCounts() != result)
    {
        qDebug() << "Expected:" << result;
        qDebug() << "Actual:" << x.charCounts();
    }
    */

    QCOMPARE(x.charCounts(), result);
}

QTEST_APPLESS_MAIN(TestCharCountDistribution)
#include "TestCharCountDistribution.moc"
