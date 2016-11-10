/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../OptionSet.h"

class TestOptionSet : public QObject
{
    Q_OBJECT

private slots:
    void add();
    void operatorltlt();
    void clear();
    void contains();
    void isEmpty();
    void remove();
    void set();
    void value();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestOptionSet::add()
{
    OptionSet x;

    x.add(Option("Luke", "Ulrich"));
    x.add(Option("Megan", "Ulrich"));
    QCOMPARE(x.asVector(), QVector<Option>() << Option("Luke", "Ulrich") << Option("Megan", "Ulrich"));
}

void TestOptionSet::operatorltlt()
{
    OptionSet x;

    x << Option("Luke", "Ulrich");
    x << Option("Megan", "Ulrich");
    QCOMPARE(x.asVector(), QVector<Option>() << Option("Luke", "Ulrich") << Option("Megan", "Ulrich"));

    OptionSet y;
    y << x;
    QCOMPARE(y.asVector(), QVector<Option>() << Option("Luke", "Ulrich") << Option("Megan", "Ulrich"));

    y.clear();
    y << Option("Pens", 5);
    x << y;
    QCOMPARE(x.asVector(), QVector<Option>()
             << Option("Luke", "Ulrich")
             << Option("Megan", "Ulrich")
             << Option("Pens", 5));
}

void TestOptionSet::clear()
{
    OptionSet x;

    x.add(Option("Luke", "Ulrich"));

    x.clear();
    QVERIFY(x.asVector().isEmpty());
}

void TestOptionSet::contains()
{
    OptionSet x;

    QVERIFY(!x.contains("Bob"));
    QVERIFY(!x.contains("Megan"));

    x.add(Option("Luke", "Ulrich"));
    x.add(Option("Megan", "Ulrich"));
    x.add(Option("Bob", "Marley"));
    QVERIFY(x.contains("Luke"));
    QVERIFY(x.contains("Luke", "Ulrich"));
    QVERIFY(!x.contains("Paul"));
    QVERIFY(!x.contains("Luke", "Donahue"));
}

void TestOptionSet::isEmpty()
{
    OptionSet x;

    QVERIFY(x.isEmpty());

    x.add(Option("Luke", "Ulrich"));
    QVERIFY(!x.isEmpty());
    x.clear();
    QVERIFY(x.isEmpty());
}

void TestOptionSet::remove()
{
    OptionSet x;

    x.add(Option("Luke", "Ulrich"));
    x.add(Option("Luke", "Content"));
    x.add(Option("Megan", "Ulrich"));
    x.add(Option("Bob", "Marley"));

    x.remove("Bob", "Marley");
    QCOMPARE(x.asVector(), QVector<Option>()
                          << Option("Luke", "Ulrich")
                          << Option("Luke", "Content")
                          << Option("Megan", "Ulrich"));

    x.remove("Luke");
    QCOMPARE(x.asVector().size(), 1);
    QVERIFY(x.contains("Megan"));
}

void TestOptionSet::set()
{
    OptionSet x;

    x.add(Option("Luke", "Ulrich"));
    x.add(Option("Luke", "Content"));
    x.add(Option("Megan", "Ulrich"));
    x.add(Option("Bob", "Marley"));

    x.set("Luke", "Charleston");
    QCOMPARE(x.asVector(), QVector<Option>()
                          << Option("Megan", "Ulrich")
                          << Option("Bob", "Marley")
                          << Option("Luke", "Charleston"));

    x.set("num_threads", "3");
    QCOMPARE(x.asVector(), QVector<Option>()
                          << Option("Megan", "Ulrich")
                          << Option("Bob", "Marley")
                          << Option("Luke", "Charleston")
                          << Option("num_threads", "3"));

    x.set("Megan");
    QCOMPARE(x.asVector(), QVector<Option>()
                          << Option("Bob", "Marley")
                          << Option("Luke", "Charleston")
                          << Option("num_threads", "3")
                          << Option("Megan"));
}

void TestOptionSet::value()
{
    OptionSet x;

    x.add(Option("Luke", "Ulrich"));
    x.add(Option("Luke", "Content"));
    x.add(Option("Megan", "Ulrich"));
    x.add(Option("Bob", ""));

    QCOMPARE(x.value("None"), QString());
    QCOMPARE(x.value("Luke"), QString("Ulrich"));
    QCOMPARE(x.value("Bob"), QString(""));
}

QTEST_APPLESS_MAIN(TestOptionSet)
#include "TestOptionSet.moc"
