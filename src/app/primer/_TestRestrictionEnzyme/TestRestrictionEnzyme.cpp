/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../RestrictionEnzyme.h"

class TestRestrictionEnzyme : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void cutsOnlyOneStrand();
    void forwardCuts();
    void isBlunt();
    void isSticky();
    void name();
    void numCuts();
    void recognitionSite();
    void reverseCuts();
    void setName();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestRestrictionEnzyme::constructor()
{
    RestrictionEnzyme re;

    QVERIFY(re.isEmpty());
    QVERIFY(re.recognitionSite().isEmpty());
    QCOMPARE(re.recognitionSite().grammar(), eDnaGrammar);
    QVERIFY(re.forwardCuts().isEmpty());
    QVERIFY(re.reverseCuts().isEmpty());
    QCOMPARE(re.isBlunt(), false);
    QCOMPARE(re.isSticky(), false);
    QVERIFY(re.name().isEmpty());
    QCOMPARE(re.numCuts(), 0);
}

void TestRestrictionEnzyme::cutsOnlyOneStrand()
{
    RestrictionEnzyme re("", "TTATTA", QVector<int>() << 3, QVector<int>());
    QVERIFY(re.cutsOnlyOneStrand());

    RestrictionEnzyme re2("", "TTATTA", QVector<int>() << 3 << 1, QVector<int>() << -5);
    QVERIFY(!re2.cutsOnlyOneStrand());
}

void TestRestrictionEnzyme::forwardCuts()
{
    RestrictionEnzyme re("", "TTATTA", QVector<int>() << 3 << 1 << 1, QVector<int>());

    QCOMPARE(re.forwardCuts(), QVector<int>() << 1 << 1 << 3);
    QCOMPARE(re.isBlunt(), false);
    QCOMPARE(re.isSticky(), false);
}

void TestRestrictionEnzyme::isBlunt()
{
    // Test: only forward cuts
    RestrictionEnzyme re("AatI", "TTATTA", QVector<int>() << 3 << 1 << 1, QVector<int>());
    QCOMPARE(re.isBlunt(), false);

    // Test: only reverse cuts
    RestrictionEnzyme re2("AatI", "TTATTA", QVector<int>(), QVector<int>() << 3 << 2 << 1);
    QCOMPARE(re2.isBlunt(), false);

    // Test; forward and reverse cuts but not in same position
    RestrictionEnzyme re3("AatI", "TTATTA", QVector<int>() << 5, QVector<int>() << 3 << 2 << 1);
    QCOMPARE(re3.isBlunt(), false);

    // Test: exactly same cuts on both strands
    RestrictionEnzyme re4("AatI", "TTATTA", QVector<int>() << -2 << 4, QVector<int>() << -2 << 4);
    QCOMPARE(re4.isBlunt(), true);
}

void TestRestrictionEnzyme::isSticky()
{
    // Test: only forward cuts
    RestrictionEnzyme re("AatI", "TTATTA", QVector<int>() << 3 << 1 << 1, QVector<int>());
    QCOMPARE(re.isSticky(), false);

    // Test: only reverse cuts
    RestrictionEnzyme re2("AatI", "TTATTA", QVector<int>(), QVector<int>() << 3 << 2 << 1);
    QCOMPARE(re2.isSticky(), false);

    // Test; forward and reverse cuts but not in same position
    RestrictionEnzyme re3("AatI", "TTATTA", QVector<int>() << 5, QVector<int>() << 3 << 2 << 1);
    QCOMPARE(re3.isSticky(), true);

    // Test: exactly same cuts on both strands
    RestrictionEnzyme re4("AatI", "TTATTA", QVector<int>() << -2 << 4, QVector<int>() << -2 << 4);
    QCOMPARE(re4.isSticky(), false);
}

void TestRestrictionEnzyme::name()
{
    RestrictionEnzyme re("AatI", "TTATTA", QVector<int>() << 3 << 1 << 1, QVector<int>());
    QCOMPARE(re.name(), QString("AatI"));
}

void TestRestrictionEnzyme::numCuts()
{
    RestrictionEnzyme re("AatI", "TTATTA", QVector<int>() << 3 << 1, QVector<int>());
    QCOMPARE(re.numCuts(), 2);

    RestrictionEnzyme re2("AatI", "TTATTA", QVector<int>() << 3, QVector<int>() << 6);
    QCOMPARE(re2.numCuts(), 2);

    RestrictionEnzyme re3("AatI", "TTATTA", QVector<int>() << 3 << -1 << -1, QVector<int>() << 6);
    QCOMPARE(re3.numCuts(), 4);
}

void TestRestrictionEnzyme::recognitionSite()
{
    RestrictionEnzyme re("EcoRI", "", QVector<int>(), QVector<int>());
    QVERIFY(re.isEmpty());

    RestrictionEnzyme re2("", "TTATTA", QVector<int>(), QVector<int>());
    QCOMPARE(re2.isEmpty(), false);
}

void TestRestrictionEnzyme::reverseCuts()
{
    RestrictionEnzyme re("", "TTATTA", QVector<int>(), QVector<int>() << 3 << 1 << 1);

    QCOMPARE(re.reverseCuts(), QVector<int>() << 1 << 1 << 3);
    QCOMPARE(re.isBlunt(), false);
    QCOMPARE(re.isSticky(), false);
}
void TestRestrictionEnzyme::setName()
{
    RestrictionEnzyme re("", "TTATTA", QVector<int>(), QVector<int>() << 3 << 1 << 1);
    QVERIFY(re.name().isEmpty());
    re.setName("AatI");
    QCOMPARE(re.name(), QString("AatI"));
}


QTEST_APPLESS_MAIN(TestRestrictionEnzyme)
#include "TestRestrictionEnzyme.moc"
