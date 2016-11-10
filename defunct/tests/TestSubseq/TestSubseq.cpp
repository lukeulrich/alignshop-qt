/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QDebug>

#include "AnonSeq.h"
#include "AminoString.h"
#include "BioString.h"
#include "DnaString.h"
#include "RnaString.h"
#include "Subseq.h"

#include "global.h"

class TestSubseq : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorBasic();
    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    void assignOther();
    void operator_bracket();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void anonSeq();
    void at();
    void setLabel();
    void id();
    void startAndStop();
    void bioString();
    void setBioString();
    void headGaps();
    void tailGaps();
    void ungappedLength();
    void setStart();
    void setStop();
    void moveStart();
    void moveStop();
    void insertGaps();
    void removeGaps();
    void slideSegment();
    void collapseLeft();
    void collapseRight();
};

// ------------------------------------------------------------------------------------------------
// Constructors
void TestSubseq::constructorBasic()
{
    AnonSeq anonSeq(1, BioString("ABC...DEF"));

    Subseq subseq(anonSeq);
    Subseq subseq2(anonSeq, 35);

    QVERIFY(subseq.modified_ == false);
    QVERIFY(subseq2.modified_ == false);
}

void TestSubseq::constructorCopy()
{
    AnonSeq anonSeq(1, BioString("ABC.-.DEF"));
    Subseq subseq(anonSeq);

    subseq.setBioString("ABC.-.DEF");
    subseq.modified_ = true;

    Subseq subseq2(subseq);
    QCOMPARE(subseq2.bioString().sequence(), QString("ABC.-.DEF"));
    QVERIFY(subseq2.modified_ == true);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Operators
void TestSubseq::assignOther()
{
    AnonSeq anonSeq(1, BioString("ABC...DEF"));
    Subseq subseq(anonSeq);

    AnonSeq anonSeq2(2, BioString("GHI"));
    Subseq subseq2(anonSeq2);

    QVERIFY(subseq.modified_ == false);
    subseq2.modified_ = true;

    subseq = subseq2;

    QVERIFY(subseq.bioString() == "GHI");
    QVERIFY(subseq.start() == subseq2.start());
    QVERIFY(subseq.stop() == subseq2.stop());
    QVERIFY(subseq.label() == subseq2.label());
    QVERIFY(subseq.modified_ == true);
}

void TestSubseq::operator_bracket()
{
    AnonSeq anonSeq(1, BioString("ABC"));
    Subseq subseq(anonSeq);
    QString str = "-A--B-C";
    subseq.setBioString(str);

    int n = str.length();
    for (int i=1; i<= n; ++i)
    {
        QVERIFY(subseq[i] == str.at(i-1));
        QVERIFY(subseq[-i] == str.at(n-i));
    }
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestSubseq::anonSeq()
{
    AnonSeq anonSeq(1, BioString("ABC"));
    AnonSeq anonSeq2(2, BioString("XYZ"));

    Subseq subseq(anonSeq);
    Subseq subseq2(anonSeq2);

    QCOMPARE(subseq.anonSeq().id(), 1);
    QCOMPARE(subseq.anonSeq().bioString().sequence(), QString("ABC"));

    QCOMPARE(subseq2.anonSeq().id(), 2);
    QCOMPARE(subseq2.anonSeq().bioString().sequence(), QString("XYZ"));
}

void TestSubseq::at()
{
    AnonSeq anonSeq(1, BioString("ABC"));
    Subseq subseq(anonSeq);
    QString str = "-A--B-C";
    subseq.setBioString(str);

    int n = str.length();
    for (int i=1; i<= n; ++i)
    {
        QVERIFY(subseq.at(i) == str.at(i-1));
        QVERIFY(subseq.at(-i) == str.at(n-i));
    }
}

void TestSubseq::setLabel()
{
    AnonSeq anonSeq(1, BioString("ABC...DEF"));
    Subseq subseq(anonSeq);
    subseq.setLabel("Test label");

    QVERIFY(subseq.label() == "Test label");
}

void TestSubseq::id()
{
    AnonSeq anonSeq(1, BioString("ABC...DEF"));
    Subseq subseq(anonSeq, 10);
    QCOMPARE(subseq.id(), 10);

    Subseq subseq2(anonSeq);
    QCOMPARE(subseq2.id(), 0);
}

void TestSubseq::startAndStop()
{
    AnonSeq anonSeq(1, BioString("ABC...DEF"));
    Subseq subseq(anonSeq);
    QVERIFY(subseq.start() == 1);
    QVERIFY(subseq.stop() == 6);
}

void TestSubseq::bioString()
{
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);
    QVERIFY(subseq.bioString() == "ABCDEF");
}

void TestSubseq::setBioString()
{
    BioString bioString = "-C---DEF-";
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);

    // Test: setting empty BioString does not work
    QVERIFY(subseq.setBioString(BioString()) == false);

    // Test: purely gaps
    QVERIFY(subseq.setBioString(BioString(".-.-.-.--")) == false);

    // Test: setting exact BioString works and start and stop are valid
    QVERIFY(subseq.setBioString(BioString("ABCDEF")));
    QVERIFY(subseq.start() == 1);
    QVERIFY(subseq.stop() == 6);

    // Test: setting non substring style biostring
    QVERIFY(subseq.setBioString(BioString("XYZ")) == false);
    QVERIFY(subseq.setBioString(BioString("BACDEF")) == false);

    // Test: setting exact partial match BioString
    QVERIFY(subseq.setBioString(BioString("ABC")));
    QVERIFY(subseq.bioString().sequence() == "ABC");
    QVERIFY(subseq.start() == 1 && subseq.stop() == 3);

    QVERIFY(subseq.setBioString(BioString("BCD")));
    QVERIFY(subseq.bioString() == "BCD");
    QVERIFY(subseq.start() == 2 && subseq.stop() == 4);

    QVERIFY(subseq.setBioString(BioString("DEF")));
    QVERIFY(subseq.bioString() == "DEF");
    QVERIFY(subseq.start() == 4 && subseq.stop() == 6);

    // Test: setting exact partial match with gaps
    QVERIFY(subseq.setBioString(BioString("-A-B-C-")));
    QVERIFY(subseq.bioString() == "-A-B-C-");
    QVERIFY(subseq.start() == 1 && subseq.stop() == 3);

    QVERIFY(subseq.setBioString(BioString("...BCD---")));
    QVERIFY(subseq.bioString() == "...BCD---");
    QVERIFY(subseq.start() == 2 && subseq.stop() == 4);

    QVERIFY(subseq.setBioString(BioString("D---EF")));
    QVERIFY(subseq.bioString() == "D---EF");
    QVERIFY(subseq.start() == 4 && subseq.stop() == 6);

    // Test: setting BioString with various derived types should not change the underlying alphabet (assuming the
    //       method returned true)
    QVERIFY(subseq.setBioString(AminoString("D---EF")));
    QVERIFY(subseq.bioString().alphabet() == eUnknownAlphabet);

    // The temporary DnaString and RnaString will not be valid, but it does not matter in this case
    QVERIFY(subseq.setBioString(DnaString("D---EF")));
    QVERIFY(subseq.bioString().alphabet() == eUnknownAlphabet);

    QVERIFY(subseq.setBioString(RnaString("D---EF")));
    QVERIFY(subseq.bioString().alphabet() == eUnknownAlphabet);
}

void TestSubseq::headGaps()
{
    BioString test_string = "-C---D-";
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);

    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.headGaps() == 1);

    QVERIFY(subseq.setBioString("CDE"));
    QVERIFY(subseq.headGaps() == 0);

    QVERIFY(subseq.setBioString("------EF"));
    QVERIFY(subseq.headGaps() == 6);

    QVERIFY(subseq.setBioString("EF---"));
    QVERIFY(subseq.headGaps() == 0);

    QVERIFY(subseq.setBioString("E--F"));
    QVERIFY(subseq.headGaps() == 0);
}

void TestSubseq::tailGaps()
{
    BioString test_string = "-C---D-";
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);

    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.tailGaps() == 1);

    QVERIFY(subseq.setBioString("CDE"));
    QVERIFY(subseq.tailGaps() == 0);

    QVERIFY(subseq.setBioString("EF------"));
    QVERIFY(subseq.tailGaps() == 6);

    QVERIFY(subseq.setBioString("---EF"));
    QVERIFY(subseq.tailGaps() == 0);

    QVERIFY(subseq.setBioString("E--F"));
    QVERIFY(subseq.tailGaps() == 0);
}

void TestSubseq::ungappedLength()
{
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);

    QCOMPARE(subseq.ungappedLength(), 6);

    QVERIFY(subseq.setBioString("-A"));
    QCOMPARE(subseq.ungappedLength(), 1);

    QVERIFY(subseq.setBioString("-A---B"));
    QCOMPARE(subseq.ungappedLength(), 2);

    QVERIFY(subseq.setBioString("ABCDE----"));
    QCOMPARE(subseq.ungappedLength(), 5);

    QVERIFY(subseq.setBioString("A-B.C------"));
    QCOMPARE(subseq.ungappedLength(), 3);
}

void TestSubseq::setStart()
{
    BioString test_string = "-C---D-";
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);

    // Setup and check
    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);

    // Test: outside valid indices
    QVERIFY(subseq.setStart(0) == false);
    QVERIFY(subseq.setStart(7) == false);
    QVERIFY(subseq.setStart(-7) == false);

    QVERIFY(subseq.bioString() == "-C---D-");
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);

    // Test: setting start to existing start should also be valid
    QVERIFY(subseq.setStart(3));
    QVERIFY(subseq.bioString() == "-C---D-");


    // >>> Suite a - normal indices
    // Test: tweaking start by one
    QVERIFY(subseq.setStart(2));
    QVERIFY2(subseq.bioString() == "BC---D-", QString("subseq.setStart(2) = %1").arg(subseq.bioString().sequence()).toAscii());
    QVERIFY(subseq.setStart(3));
    QVERIFY(subseq.bioString() == "-C---D-");

    // Suite: same as above exception with negative indices
    // Test: tweaking start by one
    QVERIFY(subseq.setStart(-5));
    QVERIFY(subseq.bioString() == "BC---D-");
    QVERIFY(subseq.setStart(-4));
    QVERIFY(subseq.bioString() == "-C---D-");


    // New suite
    // Test: move start beyond current sequence space
    QVERIFY(subseq.setStart(1));
    QVERIFY(subseq.bioString() == "ABC---D-");

    // Move it back and verify that we have an extra gap character now
    QVERIFY(subseq.setStart(3));
    QVERIFY(subseq.bioString() == "--C---D-");

    // Same thing with negative indices
    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.setStart(-6));
    QVERIFY(subseq.bioString() == "ABC---D-");

    // Move it back and verify that we have an extra gap character now
    QVERIFY(subseq.setStart(-4));
    QVERIFY(subseq.bioString() == "--C---D-");

    // Test: set start up to D and then back to A
    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.setStart(4));
    QVERIFY(subseq.bioString() == "-----D-");
    QVERIFY(subseq.start() == 4 && subseq.stop() == 4);
    QVERIFY(subseq.headGaps() == 5);
    QVERIFY(subseq.tailGaps() == 1);
    QVERIFY(subseq.setStart(1));
    QVERIFY2(subseq.bioString() == "--ABCD-", QString("Returned: %1").arg(subseq.bioString().sequence()).toAscii());

    // Test: Move start beyond stop
    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.setStart(5));

    QVERIFY(subseq.bioString() == "------E");
    QVERIFY(subseq.start() == 5 && subseq.stop() == 5);
    QVERIFY(subseq.setStart(6));
    QVERIFY(subseq.bioString() == "-------F");

    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.setStart(6));
    QVERIFY(subseq.bioString() == "-------F");
    QVERIFY(subseq.start() == 6 && subseq.stop() == 6);

    QVERIFY(subseq.setStart(1));
    QVERIFY(subseq.bioString() == "--ABCDEF");
}

void TestSubseq::setStop()
{
    BioString test_string = "-C---D-";
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);

    QVERIFY(subseq.setBioString(test_string));

    QVERIFY(subseq.setStop(0) == false);
    QVERIFY(subseq.setStop(7) == false);
    QVERIFY(subseq.setStop(-7) == false);

    QVERIFY(subseq.bioString() == test_string);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);
    QVERIFY(subseq.headGaps() == 1 and subseq.tailGaps() == 1);

    // Test: setting stop to existing stop should also be valid
    QVERIFY(subseq.setStop(4));
    QVERIFY(subseq.bioString() == test_string);


    // >>> Suite a - normal indices
    // Test: tweaking stop by one
    QVERIFY(subseq.setStop(5));
    QVERIFY(subseq.bioString() == "-C---DE");
    QVERIFY(subseq.setStop(4));
    QVERIFY(subseq.bioString() == "-C---D-");

    // Suite: same as above exception with negative indices
    // Test: tweaking start by one
    QVERIFY(subseq.setStop(-2));
    QVERIFY(subseq.bioString() == "-C---DE");
    QVERIFY(subseq.setStop(-3));
    QVERIFY(subseq.bioString() == "-C---D-");


    // New suite
    // Test: move start beyond current sequence space
    QVERIFY(subseq.setStop(6));
    QVERIFY(subseq.bioString() == "-C---DEF");

    // Move it back and verify that we have an extra gap character now
    QVERIFY(subseq.setStop(4));
    QVERIFY(subseq.bioString() == "-C---D--");

    // Same thing with negative indices
    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.setStop(-1));
    QVERIFY(subseq.bioString() == "-C---DEF");

    // Move it back and verify that we have an extra gap character now
    QVERIFY(subseq.setStop(-3));
    QVERIFY(subseq.bioString() == "-C---D--");

    // Test: set stop down to C and then back to F
    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.setStop(3));
    QVERIFY(subseq.bioString() == "-C-----");
    QVERIFY(subseq.start() == 3 && subseq.stop() == 3);
    QVERIFY(subseq.headGaps() == 1);
    QVERIFY(subseq.tailGaps() == 5);
    QVERIFY(subseq.setStop(6));
    QVERIFY2(subseq.bioString() == "-CDEF--", QString("Returned: %1").arg(subseq.bioString().sequence()).toAscii());

    // Test: Move stop before start
    QVERIFY(subseq.setBioString(test_string));
    //                             -C---D-
    QVERIFY(subseq.setStop(2));
    QVERIFY(subseq.bioString() == "B------");
    QVERIFY(subseq.start() == 2 && subseq.stop() == 2);
    QVERIFY(subseq.setStop(1));
    QVERIFY(subseq.bioString() == "A-------");

    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.setStop(1));
    QVERIFY(subseq.bioString() == "A-------");
    QVERIFY(subseq.start() == 1 && subseq.stop() == 1);

    QVERIFY(subseq.setStop(6));
    QVERIFY(subseq.bioString() == "ABCDEF--");
}

void TestSubseq::moveStart()
{
    BioString test_string = "-C---D-";
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);

    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(0) == 0);
    QVERIFY(subseq.bioString() == test_string);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);

    QVERIFY(subseq.moveStart(-1) == 1);
    QVERIFY(subseq.start() == 2 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "BC---D-");

    QVERIFY(subseq.moveStart(-1) == 1);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "ABC---D-");

    QVERIFY(subseq.moveStart(1) == 1);
    QVERIFY(subseq.start() == 2 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "-BC---D-");

    QVERIFY(subseq.moveStart(1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "--C---D-");


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(-2) == 2);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "ABC---D-");

    QVERIFY(subseq.moveStart(3) == 3);
    QVERIFY(subseq.start() == 4 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "------D-");
    QVERIFY(subseq.headGaps() == 6);


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(-3) == 2);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "ABC---D-");
    QVERIFY(subseq.moveStart(-1) == 0);
    QVERIFY(subseq.moveStart(-10) == 0);


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(2) == 2);
    QVERIFY(subseq.start() == 5 && subseq.stop() == 5);
    QVERIFY(subseq.bioString() == "------E");

    QVERIFY(subseq.moveStart(1) == 1);
    QVERIFY(subseq.start() == 6 && subseq.stop() == 6);
    QVERIFY(subseq.bioString() == "-------F");

    // Test: moving beyond sequence limits does not work
    QVERIFY(subseq.moveStart(1) == 0);
    QVERIFY(subseq.moveStart(10) == 0);
    QVERIFY(subseq.start() == 6 && subseq.stop() == 6);
    QVERIFY(subseq.bioString() == "-------F");

    // Move backwards after pushing stop forwards
    QVERIFY(subseq.moveStart(-3) == 3);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 6);
    QVERIFY(subseq.bioString() == "----CDEF");

    // Attempt to move beyond stop from get-go
    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(10) == 3);
    QVERIFY(subseq.bioString() == "-------F");
}

void TestSubseq::moveStop()
{
    BioString test_string = "-C---D-";
    AnonSeq anonSeq(1, BioString("ABCDEF"));
    Subseq subseq(anonSeq);

    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(0) == 0);
    QVERIFY(subseq.bioString() == test_string);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);

    QVERIFY(subseq.moveStop(1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 5);
    QVERIFY(subseq.bioString() == "-C---DE");

    QVERIFY(subseq.moveStop(1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 6);
    QVERIFY(subseq.bioString() == "-C---DEF");

    QVERIFY(subseq.moveStop(-1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 5);
    QVERIFY(subseq.bioString() == "-C---DE-");

    QVERIFY(subseq.moveStop(-1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "-C---D--");


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(2) == 2);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 6);
    QVERIFY(subseq.bioString() == "-C---DEF");

    QVERIFY(subseq.moveStop(-3) == 3);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 3);
    QVERIFY(subseq.bioString() == "-C------");
    QVERIFY(subseq.tailGaps() == 6);


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(3) == 2);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 6);
    QVERIFY(subseq.bioString() == "-C---DEF");
    QVERIFY(subseq.moveStop(1) == 0);
    QVERIFY(subseq.moveStop(10) == 0);


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(-2) == 2);
    QVERIFY(subseq.start() == 2 && subseq.stop() == 2);
    QVERIFY(subseq.bioString() == "B------");

    QVERIFY(subseq.moveStop(-1) == 1);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 1);
    QVERIFY(subseq.bioString() == "A-------");

    // Test: moving beyond sequence limits does not work
    QVERIFY(subseq.moveStop(-1) == 0);
    QVERIFY(subseq.moveStop(-10) == 0);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 1);
    QVERIFY(subseq.bioString() == "A-------");

    // Move backwards after pushing stop forwards
    QVERIFY(subseq.moveStop(3) == 3);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 4);
    QVERIFY(subseq.bioString() == "ABCD----");

    // Attempt to move beyond stop from get-go
    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(-10) == 3);
    QVERIFY(subseq.bioString() == "A-------");
}

void TestSubseq::insertGaps()
{
    QString str = "ABCDEF";
    int n = str.length();

    BioString bioString = str;
    AnonSeq anonSeq(1, bioString);
    Subseq subseq(anonSeq);

    // >>> Suite: 2-param version, assuming that default gap character is '-'
    // Test: 0 for position and/or n
    QVERIFY(subseq.insertGaps(0, 3).bioString() == str);
    QVERIFY(subseq.insertGaps(3, 0).bioString() == str);
    QVERIFY(subseq.insertGaps(0, 0).bioString() == str);

    // Test: -n
    QVERIFY(subseq.insertGaps(3, -3).bioString() == str);

    // Test: valid -position, -n
    QVERIFY(subseq.insertGaps(-3, -3).bioString() == str);

    // Test: invalid position and n
    QVERIFY(subseq.insertGaps(-7, 0).bioString() == str);

    // Test: insert gaps at 1..n+1
    for (int i=1; i<=n+1; ++i)
    {
        subseq.setBioString(bioString);
        QVERIFY(subseq.insertGaps(i, 2).bioString() == str.left(i-1) + "--" + str.right(n-i+1));
    }

    // Test: n+2
    subseq.setBioString(bioString);
    QVERIFY(subseq.insertGaps(n+2, 3).bioString() == str);

    // Test: -1 .. -n
    for (int i=-1; i >= -n; --i)
    {
        subseq.setBioString(bioString);
        QVERIFY(subseq.insertGaps(i, 2).bioString() == str.left(n-qAbs(i)) + "--" + str.right(qAbs(i)));
    }

    // Test: n-1
    subseq.setBioString(bioString);
    QVERIFY(subseq.insertGaps(-n-1, 2).bioString() == str);

    // ----------------------------------
    // Spot checks
    subseq.setBioString(bioString); QVERIFY(subseq.insertGaps(1, 2).bioString() == "--ABCDEF");
    subseq.setBioString(bioString); QVERIFY(subseq.insertGaps(-1, 2).bioString() == "ABCDE--F");

    // >>> Suite: 3-param version
    // Test: 0 for position and/or n
    subseq.setBioString(bioString);
    QVERIFY(subseq.insertGaps(0, 3, '.').bioString() == str);
    QVERIFY(subseq.insertGaps(3, 0, 'x').bioString() == str);
    QVERIFY(subseq.insertGaps(0, 0, '@').bioString() == str);

    // Test: -n
    QVERIFY(subseq.insertGaps(3, -3, '!').bioString() == str);

    // Test: valid -position, -n
    QVERIFY(subseq.insertGaps(-3, -3, 'o').bioString() == str);

    // Test: invalid position and n
    QVERIFY(subseq.insertGaps(-7, 0, '.').bioString() == str);

    // Test: insert gaps at 1..n+1
    for (int i=1; i<=n+1; ++i)
    {
        subseq.setBioString(bioString);
        QVERIFY(subseq.insertGaps(i, 2, '.').bioString() == str.left(i-1) + ".." + str.right(n-i+1));
    }

    // Test: n+2
    subseq.setBioString(bioString);
    QVERIFY(subseq.insertGaps(n+2, 3, 'x').bioString() == str);

    // Test: -1 .. -n
    for (int i=-1; i >= -n; --i)
    {
        subseq.setBioString(bioString);
        QVERIFY(subseq.insertGaps(i, 2, '-').bioString() == str.left(n-qAbs(i)) + "--" + str.right(qAbs(i)));
    }

    // Test: n-1
    subseq.setBioString(bioString);
    QVERIFY(subseq.insertGaps(-n-1, 2, '{').bioString() == str);

    // ----------------------------------
    // Spot checks
    subseq.setBioString(bioString); QVERIFY(subseq.insertGaps(1, 2, '~').bioString() == "~~ABCDEF");
    subseq.setBioString(bioString); QVERIFY(subseq.insertGaps(-1, 2, '~').bioString() == "ABCDE~~F");


    subseq.setBioString(bioString);
    QVERIFY(subseq.insertGaps(1, 2).bioString() == "--ABCDEF");
    QVERIFY(subseq.headGaps() == 2);
}

void TestSubseq::removeGaps()
{
    //             123456789012345
    QString str = ".-A-B-C-.-DEF-.";
    int n = str.length();

    BioString biostring = str;
    AnonSeq anonSeq(1, biostring);
    Subseq subseq(anonSeq);

    subseq.setBioString(biostring);

    // Test: removing gaps at position 0 does nothing
    QVERIFY2(subseq.removeGaps(0, 1).bioString() == str, QString("removeGaps(0, 1) returned: %1").arg(subseq.bioString().sequence()).toAscii());

    // Test: position outside of bounds does nothing
    QVERIFY(subseq.removeGaps(-n-1, 1).bioString() == str);
    QVERIFY(subseq.removeGaps(-n-1, 3).bioString() == str);
    QVERIFY(subseq.removeGaps(n+1, 1).bioString() == str);
    QVERIFY(subseq.removeGaps(n+1, 5).bioString() == str);

    // Test: removing n < 1 gaps does nothing
    QVERIFY(subseq.removeGaps(1, 0).bioString() == str);
    QVERIFY(subseq.removeGaps(9, 0).bioString() == str);
    QVERIFY(subseq.removeGaps(14, 0).bioString() == str);

    // Test: Removing single gap at each specific position that is a gap character
    for (int i=-n; i< n; ++i)
    {
        if (i == 0)
            continue;

        subseq.removeGaps(i, 1);
        int abs_i = (i > 0) ? i-1 : n + i;
        if (str.at(abs_i) == '.' || str.at(abs_i) == '-')
        {
            QString tmp = str;
            tmp.remove(abs_i, 1);
            QVERIFY(subseq.bioString() == BioString(tmp));
        }
        else
            QVERIFY(subseq.bioString() == str);

        subseq.setBioString(biostring);
    }

    // Test: exact multi-gap removal
    QVERIFY(subseq.removeGaps(1, 2).bioString() == BioString("A-B-C---DEF--"));

    // Test: attempting to delete more gaps than is contiguous at position
    subseq.setBioString(biostring);
    QVERIFY(subseq.removeGaps(1, 23).bioString() == BioString("A-B-C---DEF--"));

    // Test: gap removal in middle of string
    subseq.setBioString(biostring);
    QVERIFY(subseq.removeGaps(9, 2).bioString() == BioString("--A-B-C-DEF--"));
    subseq.setBioString(biostring);
    QVERIFY(subseq.removeGaps(9, 3).bioString() == BioString("--A-B-C-DEF--"));
    subseq.setBioString(biostring);
    QVERIFY(subseq.removeGaps(8, 3).bioString() == BioString("--A-B-CDEF--"));

    // Test: gap removal at end of string
    subseq.setBioString(biostring);
    QVERIFY(subseq.removeGaps(-2, 2).bioString() == BioString("--A-B-C---DEF"));
    subseq.setBioString(biostring);
    QVERIFY(subseq.removeGaps(-2, 7).bioString() == BioString("--A-B-C---DEF"));
}

void TestSubseq::slideSegment()
{
    // Virtually identical to BioString::slideSegment and functions identically since
    // subseq.slideSegment simply passes this command onto the BioString implementation.
    // We repeat all tests here for completeness.

    //             1234567890123
    QString str = "ABC--D-EF--GH";
    int n = str.length();
    BioString biostring = str;
    AnonSeq anonSeq(1, biostring);
    Subseq subseq(anonSeq);

    subseq.setBioString(biostring);

    // Test: delta of zero and valid positions does nothing
    QVERIFY(subseq.slideSegment(6, 9, 0) == 0 && subseq.bioString().sequence() == str);

    // Test: slide entire sequence does not change anything
    QVERIFY(subseq.slideSegment(1, -1, 3) == 0 && subseq.bioString().sequence() == str);
    QVERIFY(subseq.slideSegment(1, -1, -3) == 0 && subseq.bioString().sequence() == str);

    // Test: positive direction
                                    QVERIFY(subseq.slideSegment(1, 3, 1) == 1 && subseq.bioString().sequence() == "-ABC-D-EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(1, 3, 2) == 2 && subseq.bioString().sequence() == "--ABCD-EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(1, 3, 3) == 2 && subseq.bioString().sequence() == "--ABCD-EF--GH");

    // Same thing with negative indices
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(-n, -11, 1) == 1 && subseq.bioString().sequence() == "-ABC-D-EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(-n, -11, 2) == 2 && subseq.bioString().sequence() == "--ABCD-EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(-n, -11, 3) == 2 && subseq.bioString().sequence() == "--ABCD-EF--GH");

    // Test: postive direction with region containing gaps
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(6, 9, 1) == 1 && subseq.bioString().sequence() == "ABC---D-EF-GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(6, 9, 2) == 2 && subseq.bioString().sequence() == "ABC----D-EFGH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(6, 9, 3) == 2 && subseq.bioString().sequence() == "ABC----D-EFGH");

    // Same thing with negative indices
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(-8, -5, 1) == 1 && subseq.bioString().sequence() == "ABC---D-EF-GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(-8, -5, 2) == 2 && subseq.bioString().sequence() == "ABC----D-EFGH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(-8, -5, 3) == 2 && subseq.bioString().sequence() == "ABC----D-EFGH");

    // Test: region with terminal gaps
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(5, 10, 1) == 1 && subseq.bioString().sequence() == "ABC---D-EF-GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(5, 10, 2) == 1 && subseq.bioString().sequence() == "ABC---D-EF-GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(5, 10, -1) == 1 && subseq.bioString().sequence() == "ABC-D-EF---GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(5, 10, -2) == 1 && subseq.bioString().sequence() == "ABC-D-EF---GH");

    // Test: move single residue
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(6, 6, -3) == 2 && subseq.bioString().sequence() == "ABCD---EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(6, 6, -1) == 1 && subseq.bioString().sequence() == "ABC-D--EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(6, 6, 1) == 1 && subseq.bioString().sequence() == "ABC---DEF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(6, 6, 2) == 1 && subseq.bioString().sequence() == "ABC---DEF--GH");

    // Test: sliding pure gapped region
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, -1) == 1 && subseq.bioString().sequence() == "AB--CD-EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, -2) == 2 && subseq.bioString().sequence() == "A--BCD-EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, -3) == 3 && subseq.bioString().sequence() == "--ABCD-EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, -4) == 3 && subseq.bioString().sequence() == "--ABCD-EF--GH");

    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 1) == 1 && subseq.bioString().sequence() == "ABCD---EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 2) == 2 && subseq.bioString().sequence() == "ABCD---EF--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 3) == 3 && subseq.bioString().sequence() == "ABCD-E--F--GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 4) == 4 && subseq.bioString().sequence() == "ABCD-EF----GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 5) == 5 && subseq.bioString().sequence() == "ABCD-EF----GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 6) == 6 && subseq.bioString().sequence() == "ABCD-EF----GH");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 7) == 7 && subseq.bioString().sequence() == "ABCD-EF--G--H");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 8) == 8 && subseq.bioString().sequence() == "ABCD-EF--GH--");
    subseq.setBioString(biostring); QVERIFY(subseq.slideSegment(4, 5, 9) == 8 && subseq.bioString().sequence() == "ABCD-EF--GH--");
}

QString buildExpectedCollapseLeft(QString input, int start, int stop)
{
    int l = stop - start + 1;
    QString extract = BioString(input.mid(start - 1, l)).ungapped();
    extract += QString("-").repeated(l - extract.length());
    return input.replace(start - 1, l, extract);
}

QPair<int, int> findDifferences(const QString &a, const QString &b)
{
    QPair<int, int> range;

    const QChar *x = a.constData();
    const QChar *y = b.constData();
    for (int i=1; *x != '\0'; ++i, ++x, ++y)
    {
        if (*x != *y)
        {
            if (range.first == 0)
                range.first = i;

            range.second = i;
        }
    }

    return range;
}

void TestSubseq::collapseLeft()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);

    QStringList seqs;
    seqs << "A-BC-D-E"
         << "-A-BCD--"
         << "--A---BC";

    foreach (const QString &seq, seqs)
    {
        int l = seq.length();
        for (int start=1; start <= l; ++start)
        {
            for (int stop=start; stop <= l; ++stop)
            {
                QVERIFY(subseq->setBioString(seq));
                QString expected = buildExpectedCollapseLeft(seq, start, stop);
                QPair<int, int> range = subseq->collapseLeft(start, stop);
                QCOMPARE(subseq->bioString().sequence(), expected);
                QPair<int, int> expectedRange = findDifferences(seq, expected);
                QCOMPARE(range.first, expectedRange.first);
                QCOMPARE(range.second, expectedRange.second);
            }
        }
    }
}

QString buildExpectedCollapseRight(QString input, int start, int stop)
{
    int l = stop - start + 1;
    QString extract = BioString(input.mid(start - 1, l)).ungapped();
    extract = QString("-").repeated(l - extract.length()) + extract;
    return input.replace(start - 1, l, extract);
}

void TestSubseq::collapseRight()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);

    QStringList seqs;
    seqs << "A-BC-D-E"
         << "-A-BCD--"
         << "--A--BC-";

    foreach (const QString &seq, seqs)
    {
        int l = seq.length();
        for (int start=1; start <= l; ++start)
        {
            for (int stop=start; stop <= l; ++stop)
            {
                QVERIFY(subseq->setBioString(seq));
                QString expected = buildExpectedCollapseRight(seq, start, stop);
                QPair<int, int> range = subseq->collapseRight(start, stop);
                QCOMPARE(subseq->bioString().sequence(), expected);
                QPair<int, int> expectedRange = findDifferences(seq, expected);
                QCOMPARE(range.first, expectedRange.first);
                QCOMPARE(range.second, expectedRange.second);


//                QVERIFY(subseq->setBioString(seq));
//                subseq->collapseRight(start, stop);
//                QCOMPARE(subseq->bioString().sequence(), buildExpectedCollapseRight(seq, start, stop));
            }
        }
    }
}

QTEST_MAIN(TestSubseq)
#include "TestSubseq.moc"
