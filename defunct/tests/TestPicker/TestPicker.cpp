/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QTest>

#include "Picker.h"
#include "global.h"

class TestPicker : public QObject
{
    Q_OBJECT

private slots:
    void pickMostFrequent();
};

void TestPicker::pickMostFrequent()
{
    // Test: list of integers
    QList<int> list;
    QList<int> result;

    // Test: (1)
    list.append(1);
    QCOMPARE(::pickMostFrequent(list).size(), 1);

    // Test: (1,1)
    list.append(1);
    QCOMPARE(::pickMostFrequent(list).at(0), 1);

    // Test: (1,1,2)
    list.append(2);
    QCOMPARE(::pickMostFrequent(list).at(0), 1);

    // Test: (1,1,2,2)
    list.append(2);
    QVERIFY(::pickMostFrequent(list).size() == 2);
    QVERIFY(::pickMostFrequent(list).contains(1));
    QVERIFY(::pickMostFrequent(list).contains(2));

    // Test: (2,2,1,1)
    list.clear();
    list.append(2);
    list.append(2);
    list.append(1);
    list.append(1);
    QVERIFY(::pickMostFrequent(list).size() == 2);
    QVERIFY(::pickMostFrequent(list).contains(1));
    QVERIFY(::pickMostFrequent(list).contains(2));

    // Test: (1,2)
    list.clear();
    list.append(3);
    list.append(5);
    QVERIFY(::pickMostFrequent(list).size() == 2);
    QVERIFY(::pickMostFrequent(list).contains(3));
    QVERIFY(::pickMostFrequent(list).contains(5));


    // Test: Alphabet list
    QList<Alphabet> alist, aresult;
    alist.append(eAminoAlphabet);
    alist.append(eDnaAlphabet);
    alist.append(eDnaAlphabet);
    alist.append(eDnaAlphabet);
    alist.append(eDnaAlphabet);
    alist.append(eUnknownAlphabet);
    alist.append(eUnknownAlphabet);
    alist.append(eUnknownAlphabet);
    alist.append(eRnaAlphabet);
    QVERIFY(::pickMostFrequent(alist).size() == 1);
    QCOMPARE(::pickMostFrequent(alist).at(0), eDnaAlphabet);
}

QTEST_MAIN(TestPicker)
#include "TestPicker.moc"
