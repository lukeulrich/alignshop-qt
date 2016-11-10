/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../UngappedSubseq.h"
#include "../Seq.h"

class TestUngappedSubseq : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Remaining operators
    void operator_eqeq();       // Also tests operator!=
    void operator_lt();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void inverseStart();
    void inverseStop();
    void moveStart();
    void moveStop();
    void relativeStart();
    void relativeStop();
    void setStart();
    void setStop();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestUngappedSubseq::constructor()
{
    UngappedSubseq usubseq(Seq("ABCDEF", eAminoGrammar));

    QCOMPARE(usubseq.constData(), "ABCDEF");
    QCOMPARE(usubseq.grammar(), eAminoGrammar);
    QCOMPARE(usubseq.parentSeq_.constData(), "ABCDEF");

    QCOMPARE(usubseq.start(), 1);
    QCOMPARE(usubseq.stop(), 6);
    QCOMPARE(usubseq.length(), 6);

    UngappedSubseq usubseq2(Seq("", eRnaGrammar));
    QCOMPARE(usubseq2.constData(), "");
    QCOMPARE(usubseq2.grammar(), eRnaGrammar);
    QCOMPARE(usubseq2.parentSeq_.constData(), "");

    QCOMPARE(usubseq2.start(), 0);
    QCOMPARE(usubseq2.stop(), 0);
    QCOMPARE(usubseq2.length(), 0);
}

void TestUngappedSubseq::operator_eqeq()
{
    UngappedSubseq usubseq(Seq("ABC"));
    QVERIFY(usubseq == usubseq);

    UngappedSubseq usubseq2(Seq("ABCD"));
    QVERIFY(!(usubseq2 == usubseq));
    QVERIFY(usubseq2 != usubseq);

    UngappedSubseq usubseq3(Seq("ABC", eDnaGrammar));
    QVERIFY(usubseq3.grammar() != usubseq.grammar());
    QVERIFY(!(usubseq3 == usubseq));
    QVERIFY(usubseq3 != usubseq);

    // Test: identical subseqs but with different parents
    UngappedSubseq usubseq4(Seq("ABCDEF", eAminoGrammar));
    UngappedSubseq usubseq5(Seq("ABCDEF", eDnaGrammar));
    QVERIFY(usubseq4 != usubseq5);

    // Test: same parent, but different subseqs
    Seq parent("ABCDEF");
    UngappedSubseq usubseq6(parent);
    usubseq6.setStop(3);
    UngappedSubseq usubseq7(parent);
    usubseq7.setStart(4);
    QVERIFY(usubseq6 != usubseq7);

    // Test: same parent, same subseqs
    usubseq7.setStart(1);
    usubseq7.setStop(3);
    QVERIFY(usubseq6 == usubseq7);

    usubseq.parentSeq_ == usubseq2.parentSeq_;

    // Test: check for relative start
    UngappedSubseq usubseq8(parent);
    UngappedSubseq usubseq9(parent);
    usubseq8.setRelativeStart(10);
    QVERIFY(usubseq8 != usubseq9);
    usubseq9.setRelativeStart(10);
    QVERIFY(usubseq8 == usubseq9);
}

void TestUngappedSubseq::operator_lt()
{
    UngappedSubseq alpha(Seq("A"));
    UngappedSubseq beta(Seq("B"));

    QVERIFY(alpha < beta);
}

void TestUngappedSubseq::inverseStart()
{
    //                          7654321
    //                          1234567
    Seq seq("ABCDEFG");
    UngappedSubseq usubseq(seq);

    for (int i=1; i<=seq.length(); ++i)
    {
        usubseq.setStart(i);
        QCOMPARE(usubseq.inverseStart(), -seq.length() + i - 1);
    }
}

void TestUngappedSubseq::inverseStop()
{
    Seq seq("ABCDEFG");
    UngappedSubseq usubseq(seq);

    for (int i=1; i<=seq.length(); ++i)
    {
        usubseq.setStop(i);
        QCOMPARE(usubseq.inverseStop(), -seq.length() + i - 1);
    }
}

void TestUngappedSubseq::moveStart()
{
    UngappedSubseq usubseq(Seq("ABCDEF"));

    QCOMPARE(usubseq.start(), 1);
    QCOMPARE(usubseq.stop(), 6);

    QVERIFY(usubseq.moveStart(0) == 0);
    QVERIFY(usubseq == "ABCDEF");
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 6);

    usubseq.setStart(3); usubseq.setStop(4);

    QVERIFY(usubseq.moveStart(-1) == 1);
    QVERIFY(usubseq.start() == 2 && usubseq.stop() == 4);
    QVERIFY(usubseq == "BCD");

    QVERIFY(usubseq.moveStart(-1) == 1);
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 4);
    QVERIFY(usubseq == "ABCD");

    QVERIFY(usubseq.moveStart(1) == 1);
    QVERIFY(usubseq.start() == 2 && usubseq.stop() == 4);
    QVERIFY(usubseq == "BCD");

    QVERIFY(usubseq.moveStart(1) == 1);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 4);
    QVERIFY(usubseq == "CD");


    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStart(-2) == 2);
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 4);
    QVERIFY(usubseq == "ABCD");

    QVERIFY(usubseq.moveStart(3) == 3);
    QVERIFY(usubseq.start() == 4 && usubseq.stop() == 4);
    QVERIFY(usubseq == "D");


    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStart(-3) == 2);
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 4);
    QVERIFY(usubseq == "ABCD");
    QVERIFY(usubseq.moveStart(-1) == 0);
    QVERIFY(usubseq.moveStart(-10) == 0);


    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStart(2) == 2);
    QVERIFY(usubseq.start() == 5 && usubseq.stop() == 5);
    QVERIFY(usubseq == "E");

    QVERIFY(usubseq.moveStart(1) == 1);
    QVERIFY(usubseq.start() == 6 && usubseq.stop() == 6);
    QVERIFY(usubseq == "F");

    // Test: moving beyond sequence limits does not work
    QVERIFY(usubseq.moveStart(1) == 0);
    QVERIFY(usubseq.moveStart(10) == 0);
    QVERIFY(usubseq.start() == 6 && usubseq.stop() == 6);
    QVERIFY(usubseq == "F");

    // Move backwards after pushing stop forwards
    QVERIFY(usubseq.moveStart(-3) == 3);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 6);
    QVERIFY(usubseq == "CDEF");

    // Attempt to move beyond stop from get-go
    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStart(10) == 3);
    QVERIFY(usubseq == "F");
}

void TestUngappedSubseq::moveStop()
{
    UngappedSubseq usubseq(Seq("ABCDEF"));

    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStop(0) == 0);
    QVERIFY(usubseq == "CD");
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 4);

    QVERIFY(usubseq.moveStop(1) == 1);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 5);
    QVERIFY(usubseq == "CDE");

    QVERIFY(usubseq.moveStop(1) == 1);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 6);
    QVERIFY(usubseq == "CDEF");

    QVERIFY(usubseq.moveStop(-1) == 1);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 5);
    QVERIFY(usubseq == "CDE");

    QVERIFY(usubseq.moveStop(-1) == 1);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 4);
    QVERIFY(usubseq == "CD");


    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStop(2) == 2);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 6);
    QVERIFY(usubseq == "CDEF");

    QVERIFY(usubseq.moveStop(-3) == 3);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 3);
    QVERIFY(usubseq == "C");


    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStop(3) == 2);
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 6);
    QVERIFY(usubseq == "CDEF");
    QVERIFY(usubseq.moveStop(1) == 0);
    QVERIFY(usubseq.moveStop(10) == 0);


    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStop(-2) == 2);
    QVERIFY(usubseq.start() == 2 && usubseq.stop() == 2);
    QVERIFY(usubseq == "B");

    QVERIFY(usubseq.moveStop(-1) == 1);
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 1);
    QVERIFY(usubseq == "A");

    // Test: moving beyond sequence limits does not work
    QVERIFY(usubseq.moveStop(-1) == 0);
    QVERIFY(usubseq.moveStop(-10) == 0);
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 1);
    QVERIFY(usubseq == "A");

    // Move backwards after pushing stop forwards
    QVERIFY(usubseq.moveStop(3) == 3);
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 4);
    QVERIFY(usubseq == "ABCD");

    // Attempt to move beyond stop from get-go
    usubseq.setStart(3); usubseq.setStop(4);
    QVERIFY(usubseq.moveStop(-10) == 3);
    QVERIFY(usubseq == "A");
}

void TestUngappedSubseq::relativeStart()
{
    UngappedSubseq usubseq(Seq("ABCDEF"));
    QCOMPARE(usubseq.relativeStart(), usubseq.start());

    for (int i=1; i< 100; ++i)
    {
        usubseq.setRelativeStart(i);
        QCOMPARE(usubseq.relativeStart(), i);
    }
}

void TestUngappedSubseq::relativeStop()
{
    UngappedSubseq usubseq(Seq("ABCDEF"));
    QCOMPARE(usubseq.relativeStop(), usubseq.stop());

    for (int i=1; i< 100; ++i)
    {
        usubseq.setRelativeStart(i);
        QCOMPARE(usubseq.relativeStop(), i + usubseq.stop() - 1);
    }
}

void TestUngappedSubseq::setStart()
{
    UngappedSubseq usubseq(Seq("ABCDEF"));

    // Setup and check
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 6);

    // Test: setting start to existing start should also be valid
    usubseq.setStart(3);
    QVERIFY(usubseq == "CDEF");

    // Test: tweaking start by one
    usubseq.setStart(2);
    QVERIFY(usubseq == "BCDEF");
    usubseq.setStart(3);
    QVERIFY(usubseq == "CDEF");

    // Test: move start beyond current sequence space
    usubseq.setStart(1);
    QVERIFY(usubseq == "ABCDEF");

    // Move it back and verify that we have an extra gap character now
    usubseq.setStart(3);
    QVERIFY(usubseq == "CDEF");

    // Test: set start up to D and then back to A
    usubseq.setStop(4);
    QVERIFY(usubseq == "CD");
    usubseq.setStart(4);
    QVERIFY(usubseq == "D");
    QVERIFY(usubseq.start() == 4 && usubseq.stop() == 4);
    usubseq.setStart(1);
    QVERIFY(usubseq == "ABCD");

    // Test: Move start beyond stop
    usubseq.setStart(5);

    QVERIFY(usubseq == "E");
    QVERIFY(usubseq.start() == 5 && usubseq.stop() == 5);
    usubseq.setStart(6);
    QVERIFY(usubseq == "F");

    usubseq.setStop(4);
    usubseq.setStart(6);
    QVERIFY(usubseq == "F");
    QVERIFY(usubseq.start() == 6 && usubseq.stop() == 6);

    usubseq.setStart(1);
    QVERIFY(usubseq == "ABCDEF");
}

void TestUngappedSubseq::setStop()
{
    UngappedSubseq usubseq(Seq("ABCDEF"));
    usubseq.setStart(3); usubseq.setStop(4);

    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 4);
    QVERIFY(usubseq == "CD");

    // Test: setting stop to existing stop should also be valid
    usubseq.setStop(4);
    QVERIFY(usubseq == "CD");

    // Test: tweaking stop by one
    usubseq.setStop(5);
    QVERIFY(usubseq == "CDE");
    usubseq.setStop(4);
    QVERIFY(usubseq == "CD");

    // Test: move start beyond current sequence space
    usubseq.setStop(6);
    QVERIFY(usubseq == "CDEF");

    usubseq.setStop(4);
    QVERIFY(usubseq == "CD");

    // Test: set stop down to C and then back to F
    usubseq.setStart(3); usubseq.setStop(4);
    usubseq.setStop(3);
    QVERIFY(usubseq == "C");
    QVERIFY(usubseq.start() == 3 && usubseq.stop() == 3);
    usubseq.setStop(6);
    QVERIFY(usubseq == "CDEF");

    // Test: Move stop before start
    usubseq.setStart(3); usubseq.setStop(4);
    //                 CD
    usubseq.setStop(2);
    QVERIFY(usubseq == "B");
    QVERIFY(usubseq.start() == 2 && usubseq.stop() == 2);
    usubseq.setStop(1);
    QVERIFY(usubseq == "A");

    usubseq.setStart(3); usubseq.setStop(4);
    //                 CD
    usubseq.setStop(1);
    QVERIFY(usubseq == "A");
    QVERIFY(usubseq.start() == 1 && usubseq.stop() == 1);

    usubseq.setStop(6);
    QVERIFY(usubseq == "ABCDEF");
}

QTEST_APPLESS_MAIN(TestUngappedSubseq)
#include "TestUngappedSubseq.moc"
