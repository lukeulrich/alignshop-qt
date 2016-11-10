/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "CharCountDistribution.h"

class TestCharCountDistribution : public QObject
{
    Q_OBJECT

public:
    TestCharCountDistribution() : QObject()
    {
        qRegisterMetaType<CharCountDistribution>("CharCountDistribution");
        qRegisterMetaType<ListHashCharInt>("ListHashCharInt");
    }

private slots:
    void constructor();     // Also tests the length function
    void insertBlanks();
    void remove();
    void add_data();
    void add();
    void addString_data();
    void addString();
    void subtract_data();
    void subtract();
    void subtractString_data();
    void subtractString();
    void removeZeroKeyValues_data();
    void removeZeroKeyValues();

private:
    ListHashCharInt createListHashCharInt(bool positive = true) const;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
// Creates a distribution with either positive or negative values
// [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
ListHashCharInt TestCharCountDistribution::createListHashCharInt(bool positive) const
{
    ListHashCharInt data;
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
        QCOMPARE(x.charCounts(), ListHashCharInt());
    }

    // ------------------------------------------------------------------------
    // Test: 1 column, non-empty char counts
    {
        ListHashCharInt charCounts;
        charCounts << QHash<char, int>();
        charCounts.last().insert('A', 10);
        charCounts.last().insert('C', 3);

        CharCountDistribution x(charCounts);

        QCOMPARE(x.length(), 1);
        QCOMPARE(x.charCounts(), charCounts);
    }

    // ------------------------------------------------------------------------
    // Test: 2 column, non-empty char counts
    {
        ListHashCharInt charCounts;
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
    // Test: insert 0 and 1 blank into empty object
    {
        CharCountDistribution x((ListHashCharInt()));
        x.insertBlanks(1, 0);
        QCOMPARE(x.charCounts(), ListHashCharInt());

        // ------------------------------------------------------------------------
        // Test: insert 1 blank into empty object
        x.insertBlanks(1, 1);
        QCOMPARE(x.charCounts(), ListHashCharInt() << (QHash<char, int>()));
    }

    // ------------------------------------------------------------------------
    // Test: insert blanks into non-empty distribution
    {
        ListHashCharInt data = createListHashCharInt();
        CharCountDistribution x(data);

        // Test: insert zero before the first column
        x.insertBlanks(1, 0);
        QCOMPARE(x.charCounts(), data);

        // Test: insert in the first column
        ListHashCharInt expectedX(data);
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
        ListHashCharInt data = createListHashCharInt();
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
        data.removeAt(1);
        QCOMPARE(x.charCounts(), data);

        // ------------------------------------------------------------------------
        // Test: remove both columns
        x.remove(1, 2);
        QCOMPARE(x.charCounts(), ListHashCharInt());
    }

    {
        ListHashCharInt data = createListHashCharInt();
        CharCountDistribution x(data);

        // ------------------------------------------------------------------------
        // Test: 1 parameter version
        x.remove(2);
        data.removeLast();
        QCOMPARE(x.charCounts(), data);

        x.remove(1);
        QCOMPARE(x.charCounts(), ListHashCharInt());
    }
}

void TestCharCountDistribution::add_data()
{
    QTest::addColumn<CharCountDistribution>("x");
    QTest::addColumn<CharCountDistribution>("y");
    QTest::addColumn<int>("offset");
    QTest::addColumn<ListHashCharInt>("expectedResult");

    // ------------------------------------------------------------------------
    QTest::newRow("adding empty distribution")
            << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
            << CharCountDistribution()
            << 1
            << createListHashCharInt();

    // ------------------------------------------------------------------------
    {
        ListHashCharInt xData;
        xData << QHash<char, int>();
        xData << QHash<char, int>();
        QTest::newRow("adding to distribution with two columns but no values")
                << CharCountDistribution(xData)
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << 1
                << createListHashCharInt();
    }

    // ------------------------------------------------------------------------
    {
        ListHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData << QHash<char, int>();
        yData.last().insert('A', 1);
        yData.last().insert('C', 1);

        ListHashCharInt expectedData(createListHashCharInt());
        expectedData[0].insert('G', 2);
        expectedData[1].insert('A', 1);
        expectedData[1].insert('C', 1);

        QTest::newRow("adding distribution of equal length but completely unique characters")
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) ], [ (A, 1) (C, 1) ]
                << 1
                << expectedData;                                    // [ (A, 2) (C, 2) (G, 2) ], [ (T, 1) (G, 3) (A, 1) (C, 1) ]
    }

    // ------------------------------------------------------------------------
    {
        ListHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('A', 1);
        yData.last().insert('G', 1);
        yData << QHash<char, int>();
//        yData.last().insert('-', 1);
        yData.last().insert('G', 1);

        ListHashCharInt expectedData(createListHashCharInt());
        expectedData[0]['A'] = 3;
        expectedData[0]['G'] = 1;
        expectedData[1]['G'] = 4;
//        expectedData[1].insert('-', 1);

        QTest::newRow("adding distribution of equal length and partially unique characters")
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (A, 1) (G, 1) ], [ (-, 1) (G, 1) ]
                << 1
                << expectedData;                                    // [ (A, 3) (C, 2) (G, 1) ], [ (T, 1) (G, 4) ]
    }

    // ------------------------------------------------------------------------
    {
        ListHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData.last().insert('T', 1);

        ListHashCharInt expectedData(createListHashCharInt());
        expectedData[0].insert('G', 2);
        expectedData[0].insert('T', 1);

        QTest::newRow("adding distribution of unequal length with specific offset of 1")
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) (T, 1) ]
                << 1
                << expectedData;                                    // [ (A, 2) (C, 2) (G, 2) (T, 1) ], [ (T, 1) (G, 3) ]
    }

    // ------------------------------------------------------------------------
    {
        ListHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData.last().insert('T', 1);

        ListHashCharInt expectedData(createListHashCharInt());
        expectedData[1]['G'] = 5;
        expectedData[1]['T'] = 2;

        QTest::newRow("adding distribution of unequal length with specific offset of 2")
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
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
    QFETCH(ListHashCharInt, expectedResult);

    x.add(y, offset);

    /*
    qDebug() << x.length();
    qDebug() << y.length();
    qDebug() << "Expected:" << expectedResult;
    qDebug() << "Actual:" << x.charCounts();
    */

    QCOMPARE(x.charCounts(), expectedResult);
}

void TestCharCountDistribution::addString_data()
{
    QTest::addColumn<CharCountDistribution>("x");
    QTest::addColumn<QString>("characters");
    QTest::addColumn<char>("skipChar");
    QTest::addColumn<int>("offset");
    QTest::addColumn<ListHashCharInt>("expectedResult");

    ListHashCharInt initialSourceData = createListHashCharInt();
    CharCountDistribution starter(initialSourceData);
    QTest::newRow("empty string") << starter << QString() << '\0' << 1 << initialSourceData;
    QTest::newRow("solely gaps") << starter << "--" << '-' << 1 << initialSourceData;
    QTest::newRow("one gap, offset = 1") << starter << "-" << '-' << 1 << initialSourceData;
    QTest::newRow("one gap, offset = 2") << starter << "-" << '-' << 2 << initialSourceData;

    {
        ListHashCharInt expectedResult = initialSourceData;
        ++expectedResult[0]['G'];
        ++expectedResult[1]['A'];
        QTest::newRow("GA") << starter << "GA" << '\0' << 1 << expectedResult;
    }

    {
        ListHashCharInt expectedResult = initialSourceData;
        ++expectedResult[0]['C'];
        QTest::newRow("C") << starter << "C" << '\0' << 1 << expectedResult;
    }

    {
        ListHashCharInt expectedResult = initialSourceData;
        ++expectedResult[1]['T'];
        QTest::newRow("T, offset 2") << starter << "T" << '\0' << 2 << expectedResult;
    }

    {
        ListHashCharInt expectedResult = initialSourceData;
        ++expectedResult[1]['T'];
        QTest::newRow("-T") << starter << "-T" << '-' << 1 << expectedResult;
    }

    {
        ListHashCharInt expectedResult = initialSourceData;
        ++expectedResult[0]['T'];
        QTest::newRow("T-") << starter << "T-" << '-' << 1 << expectedResult;
    }
}

void TestCharCountDistribution::addString()
{
    QFETCH(CharCountDistribution, x);
    QFETCH(QString, characters);
    QFETCH(char, skipChar);
    QFETCH(int, offset);
    QFETCH(ListHashCharInt, expectedResult);

    x.add(characters, skipChar, offset);

    QCOMPARE(x.charCounts(), expectedResult);
}

void TestCharCountDistribution::subtract_data()
{
    QTest::addColumn<CharCountDistribution>("x");
    QTest::addColumn<CharCountDistribution>("y");
    QTest::addColumn<int>("offset");
    QTest::addColumn<ListHashCharInt>("expectedResult");

    // ------------------------------------------------------------------------
    QTest::newRow("subtracting empty distribution")
            << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
            << CharCountDistribution()
            << 1
            << createListHashCharInt();

    // ------------------------------------------------------------------------
    {
        ListHashCharInt xData;
        xData << QHash<char, int>();
        xData << QHash<char, int>();
        QTest::newRow("subtracting from distribution with two columns but no values")
                << CharCountDistribution(xData)
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << 1
                << createListHashCharInt(false); // false means negate the values returned by createListHashCharInt
    }

    // ------------------------------------------------------------------------
    {
        ListHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData << QHash<char, int>();
        yData.last().insert('A', 1);
        yData.last().insert('C', 1);

        ListHashCharInt expectedData(createListHashCharInt());
        expectedData[0].insert('G', -2);
        expectedData[1].insert('A', -1);
        expectedData[1].insert('C', -1);

        QTest::newRow("subtracting distribution of equal length but completely unique characters")
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) ], [ (A, 1) (C, 1) ]
                << 1
                << expectedData;                                    // [ (A, 2) (C, 2) (G, -2) ], [ (T, 1) (G, 3) (A, -1) (C, -1) ]
    }

    // ------------------------------------------------------------------------
    {
        ListHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('A', 1);
        yData.last().insert('G', 1);
        yData << QHash<char, int>();
//        yData.last().insert('-', 1);
        yData.last().insert('G', 1);

        ListHashCharInt expectedData(createListHashCharInt());
        expectedData[0]['A'] = 1;
        expectedData[0]['G'] = -1;
        expectedData[1]['G'] = 2;
//        expectedData[1].insert('-', -1);

        QTest::newRow("subtracting distribution of equal length and partially unique characters")
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (A, 1) (G, 1) ], [ (-, 1) (G, 1) ]
                << 1
                << expectedData;                                    // [ (A, 1) (C, 2) (G, -1) ], [ (T, 1) (G, 2) ]
    }

    // ------------------------------------------------------------------------
    {
        ListHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData.last().insert('T', 1);

        ListHashCharInt expectedData(createListHashCharInt());
        expectedData[0].insert('G', -2);
        expectedData[0].insert('T', -1);

        QTest::newRow("subtracting distribution of unequal length with specific offset of 1")
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
                << CharCountDistribution(yData)                     // [ (G, 2) (T, 1) ]
                << 1
                << expectedData;                                    // [ (A, 2) (C, 2) (G, -2) (T, -1) ], [ (T, 1) (G, 3) ]
    }

    // ------------------------------------------------------------------------
    {
        ListHashCharInt yData;
        yData << QHash<char, int>();
        yData.last().insert('G', 2);
        yData.last().insert('T', 1);

        ListHashCharInt expectedData(createListHashCharInt());
        expectedData[1]['G'] = 1;
        expectedData[1]['T'] = 0;

        QTest::newRow("subtracting distribution of unequal length with specific offset of 2")
                << CharCountDistribution(createListHashCharInt())   // [ (A, 2) (C, 2) ], [ (T, 1) (G, 3) ]
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
    QFETCH(ListHashCharInt, expectedResult);

    x.subtract(y, offset);

    /*
    qDebug() << x.length();
    qDebug() << y.length();
    qDebug() << "Expected:" << expectedResult;
    qDebug() << "Actual:" << x.charCounts();
    */

    QCOMPARE(x.charCounts(), expectedResult);
}

void TestCharCountDistribution::subtractString_data()
{
    QTest::addColumn<CharCountDistribution>("x");
    QTest::addColumn<QString>("characters");
    QTest::addColumn<char>("skipChar");
    QTest::addColumn<int>("offset");
    QTest::addColumn<ListHashCharInt>("expectedResult");

    ListHashCharInt initialSourceData = createListHashCharInt();
    CharCountDistribution starter(initialSourceData);
    QTest::newRow("empty string") << starter << QString() << '\0' << 1 << initialSourceData;
    QTest::newRow("solely gaps") << starter << "--" << '-' << 1 << initialSourceData;
    QTest::newRow("one gap, offset = 1") << starter << "-" << '-' << 1 << initialSourceData;
    QTest::newRow("one gap, offset = 2") << starter << "-" << '-' << 2 << initialSourceData;
    {
        ListHashCharInt expectedResult = initialSourceData;
        --expectedResult[0]['G'];
        --expectedResult[1]['A'];
        QTest::newRow("GA") << starter << "GA" << '\0' << 1 << expectedResult;
    }

    {
        ListHashCharInt expectedResult = initialSourceData;
        --expectedResult[0]['C'];
        QTest::newRow("C") << starter << "C" << '\0' << 1 << expectedResult;
    }

    {
        ListHashCharInt expectedResult = initialSourceData;
        --expectedResult[1]['T'];
        QTest::newRow("T, offset 2") << starter << "T" << '\0' << 2 << expectedResult;
    }

    {
        ListHashCharInt expectedResult = initialSourceData;
        --expectedResult[1]['T'];
        QTest::newRow("-T") << starter << "-T" << '-' << 1 << expectedResult;
    }

    {
        ListHashCharInt expectedResult = initialSourceData;
        --expectedResult[0]['T'];
        QTest::newRow("T-") << starter << "T-" << '-' << 1 << expectedResult;
    }
}

void TestCharCountDistribution::subtractString()
{
    QFETCH(CharCountDistribution, x);
    QFETCH(QString, characters);
    QFETCH(char, skipChar);
    QFETCH(int, offset);
    QFETCH(ListHashCharInt, expectedResult);

    x.subtract(characters, skipChar, offset);

    QCOMPARE(x.charCounts(), expectedResult);
}

void TestCharCountDistribution::removeZeroKeyValues_data()
{
    QTest::addColumn<ListHashCharInt>("input");
    QTest::addColumn<int>("from");
    QTest::addColumn<int>("to");
    QTest::addColumn<ListHashCharInt>("result");

    ListHashCharInt baseData;
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

    ListHashCharInt derivedData(baseData);
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
    ListHashCharInt expectedResult(derivedData);
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
    QFETCH(ListHashCharInt, input);
    QFETCH(int, from);
    QFETCH(int, to);
    QFETCH(ListHashCharInt, result);

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
