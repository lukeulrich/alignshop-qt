/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "SimpleSeq.h"

class TestSimpleSeq : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorEmpty();
    void constructorBasic();
    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    void assignOther();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setHeader();
    void setSequence();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestSimpleSeq::constructorEmpty()
{
    SimpleSeq ss;

    QVERIFY(ss.header().isEmpty());
    QVERIFY(ss.sequence().isEmpty());
}

void TestSimpleSeq::constructorBasic()
{
    SimpleSeq ss("12345");

    QVERIFY(ss.header() == "12345");
    QVERIFY(ss.sequence().isEmpty());

    SimpleSeq ss2("12345", "ABCDEF");
    QVERIFY(ss2.header() == "12345");
    QVERIFY(ss2.sequence() == "ABCDEF");

    SimpleSeq ss3(NULL, "ABCDEF");
    QVERIFY(ss3.header().isEmpty());
    QVERIFY(ss3.sequence() == "ABCDEF");
}

void TestSimpleSeq::constructorCopy()
{
    SimpleSeq ss("123", "ABC");
    SimpleSeq ss2(ss);

    QVERIFY(ss2.header() == "123");
    QVERIFY(ss2.sequence() == "ABC");
}

void TestSimpleSeq::assignOther()
{
    SimpleSeq ss("123", "ABC");

    SimpleSeq ss2;

    ss2 = ss;
    QVERIFY(ss2.header() == "123");
    QVERIFY(ss2.sequence() == "ABC");
}

void TestSimpleSeq::setHeader()
{
    SimpleSeq ss("123", "ABC");

    ss.setHeader("456");
    QVERIFY(ss.header() == "456");

    ss.setHeader(NULL);
    QVERIFY(ss.header().isEmpty());
}

void TestSimpleSeq::setSequence()
{
    SimpleSeq ss("123", "ABC");

    ss.setSequence("DEF");
    QVERIFY(ss.sequence() == "DEF");

    ss.setSequence(NULL);
    QVERIFY(ss.sequence().isEmpty());
}

QTEST_MAIN(TestSimpleSeq)
#include "TestSimpleSeq.moc"
