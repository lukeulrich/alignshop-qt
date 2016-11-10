/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QString>

#include "../Subseq.h"
#include "../global.h"

class TestSubseq : public QObject
{
    Q_OBJECT

public:
    TestSubseq()
    {
        qRegisterMetaType<ClosedIntRange>("ClosedIntRange");
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Remaining operators
    void operator_eqeq();       // Also tests operator!=

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void extendLeft_intBioString();
    void extendLeft_simpleExtension();
    void extendLeft_intClosedIntRange_data();
    void extendLeft_intClosedIntRange();
    void extendRight_intBioString();
    void extendRight_simpleExtension();
    void extendRight_intClosedIntRange_data();
    void extendRight_intClosedIntRange();
    void leftRightUnusedLength();
    void leftTrimRange();
    void moveStart();
    void moveStop();
    void mapToSeq();
    void rearrange();
    void replace_intint();
    void replace_range();
    void rightTrimRange();
    void setBioString_data();
    void setBioString();
    void setStart();
    void setStop();
    void trimLeft();
    void trimRight();
};

Q_DECLARE_METATYPE(ClosedIntRange);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestSubseq::constructor()
{
    Subseq subseq(Seq("ABC...DEF", eAminoGrammar));
    QVERIFY(subseq.seqEntity_ == nullptr);

    QCOMPARE(subseq.constData(), "ABCDEF");
    QCOMPARE(subseq.grammar(), eAminoGrammar);
    QCOMPARE(subseq.parentSeq_.constData(), "ABCDEF");

    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 6);
    QCOMPARE(subseq.length(), 6);

    Subseq subseq2(Seq("", eRnaGrammar));
    QCOMPARE(subseq2.constData(), "");
    QCOMPARE(subseq2.grammar(), eRnaGrammar);
    QCOMPARE(subseq2.parentSeq_.constData(), "");

    QCOMPARE(subseq2.start(), 0);
    QCOMPARE(subseq2.stop(), 0);
    QCOMPARE(subseq2.length(), 0);
}

void TestSubseq::operator_eqeq()
{
    Subseq subseq(Seq("ABC"));
    QVERIFY(subseq == subseq);

    Subseq subseq2(Seq("ABCD"));
    QVERIFY(!(subseq2 == subseq));
    QVERIFY(subseq2 != subseq);

    Subseq subseq3(Seq("ABC", eDnaGrammar));
    QVERIFY(subseq3.grammar() != subseq.grammar());
    QVERIFY(!(subseq3 == subseq));
    QVERIFY(subseq3 != subseq);

    // Test: identical subseqs but with different parents
    Subseq subseq4(Seq("ABCDEF", eAminoGrammar));
    Subseq subseq5(Seq("ABCDEF", eDnaGrammar));
    QVERIFY(subseq4 != subseq5);

    // Test: same parent, but different subseqs
    Seq parent("ABCDEF");
    Subseq subseq6(parent);
    QVERIFY(subseq6.setBioString("ABC"));
    Subseq subseq7(parent);
    QVERIFY(subseq7.setBioString("DEF"));
    QVERIFY(subseq6 != subseq7);

    // Test: same parent, same subseqs
    QVERIFY(subseq7.setBioString("ABC"));
    QVERIFY(subseq6 == subseq7);

    subseq.parentSeq_ == subseq2.parentSeq_;

    Seq a("AB");
    Seq b("CD");
    a == b;
}

void TestSubseq::extendLeft_intBioString()
{
    Subseq subseq(Seq("ABCDEF"));
    QVERIFY(subseq.setBioString("-DEF"));

    // Test: Add single character
    subseq.extendLeft(1, "C");
    QVERIFY(subseq == "CDEF");
    QCOMPARE(subseq.start(), 3);

    // Test: replace add one character with gaps
    QVERIFY(subseq.setBioString("---DEF"));
    subseq.extendLeft(2, "C-");
    QVERIFY(subseq == "-C-DEF");
    QCOMPARE(subseq.start(), 3);

    QVERIFY(subseq.setBioString("---DEF"));
    subseq.extendLeft(1, "-C-");
    QVERIFY(subseq == "-C-DEF");
    QCOMPARE(subseq.start(), 3);

    // Test: replace add multiple chars
    QVERIFY(subseq.setBioString("---DEF"));
    subseq.extendLeft(1, "ABC");
    QVERIFY(subseq == "ABCDEF");
    QCOMPARE(subseq.start(), 1);

    // Test: replace add multiple chars with gap
    QVERIFY(subseq.setBioString("---DEF"));
    subseq.extendLeft(1, "B-C");
    QVERIFY(subseq == "B-CDEF");
    QCOMPARE(subseq.start(), 2);
}

void TestSubseq::extendLeft_simpleExtension()
{
    Subseq subseq(Seq("ABCDEF"));
    QVERIFY(subseq.setBioString("-DEF"));

    // Test: Add single character
    Subseq::SimpleExtension extension;
    extension.seqRange_.begin_ = 3;
    extension.seqRange_.end_ = 3;
    extension.subseqPosition_ = 1;
    subseq.extendLeft(extension);
    QVERIFY(subseq == "CDEF");
    QCOMPARE(subseq.start(), 3);

    // Test: replace add one character with gaps
    QVERIFY(subseq.setBioString("---DEF"));
    extension.seqRange_.begin_ = 3;
    extension.seqRange_.end_ = 3;
    extension.subseqPosition_ = 2;
    subseq.extendLeft(extension);
    QVERIFY(subseq == "-C-DEF");
    QCOMPARE(subseq.start(), 3);

    QVERIFY(subseq.setBioString("---DEF"));
    extension.seqRange_.begin_ = 3;
    extension.seqRange_.end_ = 3;
    extension.subseqPosition_ = 1;
    subseq.extendLeft(extension);
    QVERIFY(subseq == "C--DEF");
    QCOMPARE(subseq.start(), 3);

    // Test: replace add multiple chars
    QVERIFY(subseq.setBioString("---DEF"));
    extension.seqRange_.begin_ = 1;
    extension.seqRange_.end_ = 3;
    extension.subseqPosition_ = 1;
    subseq.extendLeft(extension);
    QVERIFY(subseq == "ABCDEF");
    QCOMPARE(subseq.start(), 1);

    // Test: replace add multiple chars and leave gap
    QVERIFY(subseq.setBioString("---DEF"));
    extension.seqRange_.begin_ = 2;
    extension.seqRange_.end_ = 3;
    extension.subseqPosition_ = 1;
    subseq.extendLeft(extension);
    QVERIFY(subseq == "BC-DEF");
    QCOMPARE(subseq.start(), 2);
}

void TestSubseq::extendLeft_intClosedIntRange_data()
{
    QTest::addColumn<QString>("seqChars");
    QTest::addColumn<QByteArray>("inBioString");
    QTest::addColumn<int>("position");
    QTest::addColumn<ClosedIntRange>("parentSeqRange");
    QTest::addColumn<QString>("outBioString");
    QTest::addColumn<int>("expectedStart");

    QTest::newRow("-DEF -> CDEF") << "ABCDEF" << QByteArray("-DEF") << 1 << ClosedIntRange(3, 3) << "CDEF" << 3;
    QTest::newRow("---DEF -> -C-DEF") << "ABCDEF" << QByteArray("---DEF") << 2 << ClosedIntRange(3, 3) << "-C-DEF" << 3;
    QTest::newRow("---DEF -> C--DEF") << "ABCDEF" << QByteArray("---DEF") << 1 << ClosedIntRange(3, 3) << "C--DEF" << 3;
    QTest::newRow("---DEF -> ABCDEF") << "ABCDEF" << QByteArray("---DEF") << 1 << ClosedIntRange(1, 3) << "ABCDEF" << 1;
    QTest::newRow("---DEF -> BC-DEF") << "ABCDEF" << QByteArray("---DEF") << 1 << ClosedIntRange(2, 3) << "BC-DEF" << 2;
}

void TestSubseq::extendLeft_intClosedIntRange()
{
    QFETCH(QString, seqChars);
    QFETCH(QByteArray, inBioString);
    QFETCH(int, position);
    QFETCH(ClosedIntRange, parentSeqRange);
    QFETCH(QString, outBioString);
    QFETCH(int, expectedStart);

    Subseq subseq(Seq(seqChars.toAscii()));
    QVERIFY(subseq.setBioString(inBioString));
    subseq.extendLeft(position, parentSeqRange);
    QVERIFY(subseq == outBioString.toAscii());
    QCOMPARE(subseq.start(), expectedStart);
}

void TestSubseq::extendRight_intBioString()
{
    Subseq subseq(Seq("ABCDEF"));
    QVERIFY(subseq.setBioString("ABC-"));

    // Test: Add single character
    subseq.extendRight(4, "D");
    QVERIFY(subseq == "ABCD");
    QCOMPARE(subseq.stop(), 4);

    // Test: replace add one character with gaps
    QVERIFY(subseq.setBioString("ABC---"));
    subseq.extendRight(5, "D-");
    QVERIFY(subseq == "ABC-D-");
    QCOMPARE(subseq.stop(), 4);

    QVERIFY(subseq.setBioString("ABC---"));
    subseq.extendRight(4, "-D-");
    QVERIFY(subseq == "ABC-D-");
    QCOMPARE(subseq.stop(), 4);

    // Test: replace add multiple chars
    QVERIFY(subseq.setBioString("ABC---"));
    subseq.extendRight(4, "DEF");
    QVERIFY(subseq == "ABCDEF");
    QCOMPARE(subseq.stop(), 6);

    // Test: replace add multiple chars with gap
    QVERIFY(subseq.setBioString("ABC---"));
    subseq.extendRight(4, "D-E");
    QVERIFY(subseq == "ABCD-E");
    QCOMPARE(subseq.stop(), 5);
}

void TestSubseq::extendRight_simpleExtension()
{
    Subseq subseq(Seq("ABCDEF"));
    QVERIFY(subseq.setBioString("ABC-"));

    // Test: Add single character
    Subseq::SimpleExtension extension;
    extension.seqRange_.begin_ = 4;
    extension.seqRange_.end_ = 4;
    extension.subseqPosition_ = 4;
    subseq.extendRight(extension);
    QVERIFY(subseq == "ABCD");
    QCOMPARE(subseq.stop(), 4);

    // Test: replace add one character with gaps
    QVERIFY(subseq.setBioString("ABC---"));
    extension.seqRange_.begin_ = 4;
    extension.seqRange_.end_ = 4;
    extension.subseqPosition_ = 5;
    subseq.extendRight(extension);
    QVERIFY(subseq == "ABC-D-");
    QCOMPARE(subseq.stop(), 4);

    QVERIFY(subseq.setBioString("ABC---"));
    extension.seqRange_.begin_ = 4;
    extension.seqRange_.end_ = 4;
    extension.subseqPosition_ = 6;
    subseq.extendRight(extension);
    QVERIFY(subseq == "ABC--D");
    QCOMPARE(subseq.stop(), 4);

    // Test: replace add multiple chars
    QVERIFY(subseq.setBioString("ABC---"));
    extension.seqRange_.begin_ = 4;
    extension.seqRange_.end_ = 6;
    extension.subseqPosition_ = 4;
    subseq.extendRight(extension);
    QVERIFY(subseq == "ABCDEF");
    QCOMPARE(subseq.stop(), 6);

    // Test: replace add multiple chars and leave gap
    QVERIFY(subseq.setBioString("ABC---"));
    extension.seqRange_.begin_ = 4;
    extension.seqRange_.end_ = 5;
    extension.subseqPosition_ = 5;
    subseq.extendRight(extension);
    QVERIFY(subseq == "ABC-DE");
    QCOMPARE(subseq.stop(), 5);
}

void TestSubseq::extendRight_intClosedIntRange_data()
{
    QTest::addColumn<QString>("seqChars");
    QTest::addColumn<QByteArray>("inBioString");
    QTest::addColumn<int>("position");
    QTest::addColumn<ClosedIntRange>("parentSeqRange");
    QTest::addColumn<QString>("outBioString");
    QTest::addColumn<int>("expectedStop");

    QTest::newRow("ABC- -> ABCD") << "ABCDEF" << QByteArray("ABC-") << 4 << ClosedIntRange(4, 4) << "ABCD" << 4;
    QTest::newRow("ABC--- -> ABC-D-") << "ABCDEF" << QByteArray("ABC---") << 5 << ClosedIntRange(4, 4) << "ABC-D-" << 4;
    QTest::newRow("ABC--- -> ABC--D") << "ABCDEF" << QByteArray("ABC---") << 6 << ClosedIntRange(4, 4) << "ABC--D" << 4;
    QTest::newRow("ABC--- -> ABCDEF") << "ABCDEF" << QByteArray("ABC---") << 4 << ClosedIntRange(4, 6) << "ABCDEF" << 6;
    QTest::newRow("ABC--- -> ABC-DE") << "ABCDEF" << QByteArray("ABC---") << 5 << ClosedIntRange(4, 5) << "ABC-DE" << 5;
}

void TestSubseq::extendRight_intClosedIntRange()
{
    QFETCH(QString, seqChars);
    QFETCH(QByteArray, inBioString);
    QFETCH(int, position);
    QFETCH(ClosedIntRange, parentSeqRange);
    QFETCH(QString, outBioString);
    QFETCH(int, expectedStop);

    Subseq subseq(Seq(seqChars.toAscii()));
    QVERIFY(subseq.setBioString(inBioString));
    subseq.extendRight(position, parentSeqRange);
    QVERIFY(subseq == outBioString.toAscii());
    QCOMPARE(subseq.stop(), expectedStop);
}

void TestSubseq::leftRightUnusedLength()
{
    Subseq subseq(Seq("ABCDEFG"));
    QVERIFY(subseq.setBioString("--C-D--EF-----"));

    QCOMPARE(subseq.leftUnusedLength(), 2);
    QCOMPARE(subseq.rightUnusedLength(), 1);
}

void TestSubseq::leftTrimRange()
{
    Subseq subseq(Seq("ABCDE"));
    //                           123456789
    QVERIFY(subseq.setBioString("-AB-C-DE-"));

    QCOMPARE(subseq.leftTrimRange(1), ClosedIntRange());
    QCOMPARE(subseq.leftTrimRange(2), ClosedIntRange(2, 2));
    QCOMPARE(subseq.leftTrimRange(3), ClosedIntRange(2, 3));
    QCOMPARE(subseq.leftTrimRange(4), ClosedIntRange(2, 3));
    QCOMPARE(subseq.leftTrimRange(5), ClosedIntRange(2, 5));
    QCOMPARE(subseq.leftTrimRange(6), ClosedIntRange(2, 5));
    QCOMPARE(subseq.leftTrimRange(7), ClosedIntRange(2, 7));
    QCOMPARE(subseq.leftTrimRange(8), ClosedIntRange(2, 7));
    QCOMPARE(subseq.leftTrimRange(9), ClosedIntRange(2, 7));
}

void TestSubseq::moveStart()
{
    Subseq subseq(Seq("ABCDEF"));
    BioString test_string = "-C---D-";

    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(0) == 0);
    QVERIFY(subseq == test_string);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);

    QVERIFY(subseq.moveStart(-1) == 1);
    QVERIFY(subseq.start() == 2 && subseq.stop() == 4);
    QVERIFY(subseq == "BC---D-");

    QVERIFY(subseq.moveStart(-1) == 1);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 4);
    QVERIFY(subseq == "ABC---D-");

    QVERIFY(subseq.moveStart(1) == 1);
    QVERIFY(subseq.start() == 2 && subseq.stop() == 4);
    QVERIFY(subseq == "-BC---D-");

    QVERIFY(subseq.moveStart(1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);
    QVERIFY(subseq == "--C---D-");


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(-2) == 2);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 4);
    QVERIFY(subseq == "ABC---D-");

    QVERIFY(subseq.moveStart(3) == 3);
    QVERIFY(subseq.start() == 4 && subseq.stop() == 4);
    QVERIFY(subseq == "------D-");
    QVERIFY(subseq.headGaps() == 6);


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(-3) == 2);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 4);
    QVERIFY(subseq == "ABC---D-");
    QVERIFY(subseq.moveStart(-1) == 0);
    QVERIFY(subseq.moveStart(-10) == 0);


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(2) == 2);
    QVERIFY(subseq.start() == 5 && subseq.stop() == 5);
    QVERIFY(subseq == "------E");

    QVERIFY(subseq.moveStart(1) == 1);
    QVERIFY(subseq.start() == 6 && subseq.stop() == 6);
    QVERIFY(subseq == "-------F");

    // Test: moving beyond sequence limits does not work
    QVERIFY(subseq.moveStart(1) == 0);
    QVERIFY(subseq.moveStart(10) == 0);
    QVERIFY(subseq.start() == 6 && subseq.stop() == 6);
    QVERIFY(subseq == "-------F");

    // Move backwards after pushing stop forwards
    QVERIFY(subseq.moveStart(-3) == 3);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 6);
    QVERIFY(subseq == "----CDEF");

    // Attempt to move beyond stop from get-go
    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStart(10) == 3);
    QVERIFY(subseq == "-------F");
}

void TestSubseq::moveStop()
{
    Subseq subseq(Seq("ABCDEF"));
    BioString test_string = "-C---D-";

    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(0) == 0);
    QVERIFY(subseq == test_string);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);

    QVERIFY(subseq.moveStop(1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 5);
    QVERIFY(subseq == "-C---DE");

    QVERIFY(subseq.moveStop(1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 6);
    QVERIFY(subseq == "-C---DEF");

    QVERIFY(subseq.moveStop(-1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 5);
    QVERIFY(subseq == "-C---DE-");

    QVERIFY(subseq.moveStop(-1) == 1);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);
    QVERIFY(subseq == "-C---D--");


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(2) == 2);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 6);
    QVERIFY(subseq == "-C---DEF");

    QVERIFY(subseq.moveStop(-3) == 3);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 3);
    QVERIFY(subseq == "-C------");
    QVERIFY(subseq.tailGaps() == 6);


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(3) == 2);
    QVERIFY(subseq.start() == 3 && subseq.stop() == 6);
    QVERIFY(subseq == "-C---DEF");
    QVERIFY(subseq.moveStop(1) == 0);
    QVERIFY(subseq.moveStop(10) == 0);


    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(-2) == 2);
    QVERIFY(subseq.start() == 2 && subseq.stop() == 2);
    QVERIFY(subseq == "B------");

    QVERIFY(subseq.moveStop(-1) == 1);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 1);
    QVERIFY(subseq == "A-------");

    // Test: moving beyond sequence limits does not work
    QVERIFY(subseq.moveStop(-1) == 0);
    QVERIFY(subseq.moveStop(-10) == 0);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 1);
    QVERIFY(subseq == "A-------");

    // Move backwards after pushing stop forwards
    QVERIFY(subseq.moveStop(3) == 3);
    QVERIFY(subseq.start() == 1 && subseq.stop() == 4);
    QVERIFY(subseq == "ABCD----");

    // Attempt to move beyond stop from get-go
    subseq.setBioString(test_string);
    QVERIFY(subseq.moveStop(-10) == 3);
    QVERIFY(subseq == "A-------");
}

void TestSubseq::mapToSeq()
{
    //                 1234567
    Subseq subseq(Seq("ABCDEFG"));
    //                           12345678901
    QVERIFY(subseq.setBioString("--CD-EF-G--"));

    QCOMPARE(subseq.mapToSeq(1), -1);
    QCOMPARE(subseq.mapToSeq(2), -1);
    QCOMPARE(subseq.mapToSeq(3), 3);
    QCOMPARE(subseq.mapToSeq(4), 4);
    QCOMPARE(subseq.mapToSeq(5), -1);
    QCOMPARE(subseq.mapToSeq(6), 5);
    QCOMPARE(subseq.mapToSeq(7), 6);
    QCOMPARE(subseq.mapToSeq(8), -1);
    QCOMPARE(subseq.mapToSeq(9), 7);
    QCOMPARE(subseq.mapToSeq(10), -1);
    QCOMPARE(subseq.mapToSeq(11), -1);
}

void TestSubseq::rearrange()
{
    Subseq subseq(Seq("ABCDEF"));
    //                           123456789012345
    QVERIFY(subseq.setBioString("--AB--C-D--EF--"));

    subseq.rearrange(ClosedIntRange(1, 4), "A-B-");
    QVERIFY(subseq == "A-B---C-D--EF--");
    subseq.rearrange(ClosedIntRange(1, 15), "ABCD--E------F-");
    QVERIFY(subseq == "ABCD--E------F-");
}

void TestSubseq::replace_intint()
{
    Subseq subseq(Seq("ABCDEF"));

    // Test: replace-insert in all positions
    QCOMPARE(subseq.replace(1, 0, "A"), false);
    QCOMPARE(subseq.replace(2, 0, "A"), false);
    QCOMPARE(subseq.replace(3, 0, "A"), false);
    QCOMPARE(subseq.replace(4, 0, "A"), false);
    QCOMPARE(subseq.replace(5, 0, "A"), false);
    QCOMPARE(subseq.replace(6, 0, "A"), false);
    QCOMPARE(subseq.replace(7, 0, "A"), false);
    QVERIFY(subseq == "ABCDEF");

    // Test: internal replace-remove should fail
    QCOMPARE(subseq.replace(2, 1, ""), false);
    QCOMPARE(subseq.replace(3, 1, ""), false);
    QCOMPARE(subseq.replace(4, 1, ""), false);
    QCOMPARE(subseq.replace(5, 1, ""), false);
    QVERIFY(subseq == "ABCDEF");

    // Test: replace remove last and first character
    QCOMPARE(subseq.replace(6, 1, ""), true);
    QVERIFY(subseq == "ABCDE");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 5);

    QCOMPARE(subseq.replace(1, 1, ""), true);
    QVERIFY(subseq == "BCDE");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    // Test: replace each character with itself should work
    QVERIFY(subseq.setBioString("ABCDEF"));
    QCOMPARE(subseq.replace(1, 1, subseq.mid(1, 1)), true);
    QCOMPARE(subseq.replace(2, 1, subseq.mid(2, 1)), true);
    QCOMPARE(subseq.replace(3, 1, subseq.mid(3, 1)), true);
    QCOMPARE(subseq.replace(4, 1, subseq.mid(4, 1)), true);
    QCOMPARE(subseq.replace(5, 1, subseq.mid(5, 1)), true);
    QCOMPARE(subseq.replace(6, 1, subseq.mid(6, 1)), true);
    QVERIFY(subseq == "ABCDEF");

    // Test: replace-insert a gap before and after each letter
    subseq.replace(7, 0, "-");      QVERIFY(subseq == "ABCDEF-");
    subseq.replace(6, 0, "-");      QVERIFY(subseq == "ABCDE-F-");
    subseq.replace(5, 0, "-");      QVERIFY(subseq == "ABCD-E-F-");
    subseq.replace(4, 0, "-");      QVERIFY(subseq == "ABC-D-E-F-");
    subseq.replace(3, 0, "-");      QVERIFY(subseq == "AB-C-D-E-F-");
    subseq.replace(2, 0, "-");      QVERIFY(subseq == "A-B-C-D-E-F-");
    subseq.replace(1, 0, "-");      QVERIFY(subseq == "-A-B-C-D-E-F-");

    // Test: replace-remove all should fail
    subseq.setBioString("ABCDEF");
    QCOMPARE(subseq.replace(1, 6, "------"), false);
    QVERIFY(subseq == "ABCDEF");

    // Test: replace-remove all but one letter should work
    QCOMPARE(subseq.replace(1, 5, "-"), true);
    QVERIFY(subseq == "-F");
    QCOMPARE(subseq.start(), 6);
    QCOMPARE(subseq.stop(), 6);

    // Test: replace-insert it back
    QCOMPARE(subseq.replace(1, 0, "ABCDE"), true);
    QVERIFY(subseq == "ABCDE-F");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 6);

    // Test: replace remove at beginning
    QCOMPARE(subseq.replace(1, 1, ""), true);
    QVERIFY(subseq == "BCDE-F");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 6);

    // Test: replace remove at end
    QCOMPARE(subseq.replace(6, 1, ""), true);
    QVERIFY(subseq == "BCDE-");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    // Test: partial replace in middle
    QCOMPARE(subseq.replace(3, 2, "--D--"), true);
    QVERIFY(subseq == "BC--D---");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 4);

    // Test: partial overlap replace at left end
    QCOMPARE(subseq.replace(1, 1, ".A--B-"), true);
    QVERIFY(subseq == ".A--B-C--D---");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 4);

    // Test: replace remove of gaps
    QCOMPARE(subseq.replace(3, 7, "BC"), true);
    QVERIFY(subseq == ".ABCD---");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 4);

    // Test: add replacement to beginning with more characters than in parent seq
    subseq.setBioString("ABCDEF");
    QCOMPARE(subseq.replace(1, 1, "BABABAB"), false);
    QVERIFY(subseq == "ABCDEF");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 6);

    // Test: add replacement that extends both terminii
    subseq.setBioString("CD");
    QCOMPARE(subseq.replace(1, 2, "BCDE"), true);
    QVERIFY(subseq == "BCDE");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    subseq.setBioString("-CD-");
    QCOMPARE(subseq.replace(2, 2, "BCDE"), true);
    QVERIFY(subseq == "-BCDE-");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    subseq.setBioString("AB");
    QCOMPARE(subseq.replace(1, 2, "EF"), true);
    QVERIFY(subseq == "EF");
    QCOMPARE(subseq.start(), 5);
    QCOMPARE(subseq.stop(), 6);
}

void TestSubseq::replace_range()
{
    Subseq subseq(Seq("ABCDEF"));

    // Test: internal replace-remove should fail
    QCOMPARE(subseq.replace(ClosedIntRange(2, 2), ""), false);
    QCOMPARE(subseq.replace(ClosedIntRange(3, 3), ""), false);
    QCOMPARE(subseq.replace(ClosedIntRange(4, 4), ""), false);
    QCOMPARE(subseq.replace(ClosedIntRange(5, 5), ""), false);
    QVERIFY(subseq == "ABCDEF");

    // Test: replace remove last and first character
    QCOMPARE(subseq.replace(ClosedIntRange(6, 6), ""), true);
    QVERIFY(subseq == "ABCDE");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 5);

    QCOMPARE(subseq.replace(ClosedIntRange(1, 1), ""), true);
    QVERIFY(subseq == "BCDE");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    // Test: replace each character with itself should work
    QVERIFY(subseq.setBioString("ABCDEF"));
    QCOMPARE(subseq.replace(ClosedIntRange(1, 1), subseq.mid(1, 1)), true);
    QCOMPARE(subseq.replace(ClosedIntRange(2, 2), subseq.mid(2, 1)), true);
    QCOMPARE(subseq.replace(ClosedIntRange(3, 3), subseq.mid(3, 1)), true);
    QCOMPARE(subseq.replace(ClosedIntRange(4, 4), subseq.mid(4, 1)), true);
    QCOMPARE(subseq.replace(ClosedIntRange(5, 5), subseq.mid(5, 1)), true);
    QCOMPARE(subseq.replace(ClosedIntRange(6, 6), subseq.mid(6, 1)), true);
    QVERIFY(subseq == "ABCDEF");

    // Test: replace-insert a gap before and after each letter
    QVERIFY(subseq.replace(ClosedIntRange(6, 6), "-"));      QVERIFY(subseq == "ABCDE-");
    QVERIFY(subseq.replace(ClosedIntRange(5, 5), "-"));      QVERIFY(subseq == "ABCD--");
    QVERIFY(subseq.replace(ClosedIntRange(4, 4), "-"));      QVERIFY(subseq == "ABC---");
    QVERIFY(subseq.replace(ClosedIntRange(3, 3), "-"));      QVERIFY(subseq == "AB----");
    QVERIFY(subseq.replace(ClosedIntRange(2, 2), "-"));      QVERIFY(subseq == "A-----");
    QVERIFY(subseq.replace(ClosedIntRange(1, 1), "-") == false);
    QVERIFY(subseq == "A-----");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 1);

    // Test: replace-remove all should fail
    subseq.setBioString("ABCDEF");
    QCOMPARE(subseq.replace(ClosedIntRange(1, 6), "------"), false);
    QVERIFY(subseq == "ABCDEF");

    // Test: replace-remove all but one letter should work
    QCOMPARE(subseq.replace(ClosedIntRange(1, 5), "-"), true);
    QVERIFY(subseq == "-F");
    QCOMPARE(subseq.start(), 6);
    QCOMPARE(subseq.stop(), 6);

    // Test: replace-insert it back
    QCOMPARE(subseq.replace(ClosedIntRange(1, 1), "ABCDE"), true);
    QVERIFY(subseq == "ABCDEF");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 6);

    // Test: replace remove at beginning
    QCOMPARE(subseq.replace(ClosedIntRange(1, 1), ""), true);
    QVERIFY(subseq == "BCDEF");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 6);

    // Test: replace remove at end
    QCOMPARE(subseq.replace(ClosedIntRange(5, 5), ""), true);
    QVERIFY(subseq == "BCDE");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    // Test: partial replace in middle
    QCOMPARE(subseq.replace(ClosedIntRange(3, 4), "--D--"), true);
    QVERIFY(subseq == "BC--D--");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 4);

    // Test: partial overlap replace at left end
    QCOMPARE(subseq.replace(ClosedIntRange(1, 1), ".A--B-"), true);
    QVERIFY(subseq == ".A--B-C--D--");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 4);

    // Test: replace remove of gaps
    QCOMPARE(subseq.replace(ClosedIntRange(3, 9), "BC"), true);
    QVERIFY(subseq == ".ABCD--");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 4);

    // Test: add replacement to beginning with more characters than in parent seq
    subseq.setBioString("ABCDEF");
    QCOMPARE(subseq.replace(ClosedIntRange(1, 1), "BABABAB"), false);
    QVERIFY(subseq == "ABCDEF");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 6);

    // Test: add replacement that extends both terminii
    subseq.setBioString("CD");
    QCOMPARE(subseq.replace(ClosedIntRange(1, 2), "BCDE"), true);
    QVERIFY(subseq == "BCDE");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    subseq.setBioString("-CD-");
    QCOMPARE(subseq.replace(ClosedIntRange(2, 3), "BCDE"), true);
    QVERIFY(subseq == "-BCDE-");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    subseq.setBioString("AB");
    QCOMPARE(subseq.replace(ClosedIntRange(1, 2), "EF"), true);
    QVERIFY(subseq == "EF");
    QCOMPARE(subseq.start(), 5);
    QCOMPARE(subseq.stop(), 6);
}

void TestSubseq::rightTrimRange()
{
    Subseq subseq(Seq("ABCDE"));
    //                           123456789
    QVERIFY(subseq.setBioString("-AB-C-DE-"));

    QCOMPARE(subseq.rightTrimRange(1), ClosedIntRange(3, 8));
    QCOMPARE(subseq.rightTrimRange(2), ClosedIntRange(3, 8));
    QCOMPARE(subseq.rightTrimRange(3), ClosedIntRange(3, 8));
    QCOMPARE(subseq.rightTrimRange(4), ClosedIntRange(5, 8));
    QCOMPARE(subseq.rightTrimRange(5), ClosedIntRange(5, 8));
    QCOMPARE(subseq.rightTrimRange(6), ClosedIntRange(7, 8));
    QCOMPARE(subseq.rightTrimRange(7), ClosedIntRange(7, 8));
    QCOMPARE(subseq.rightTrimRange(8), ClosedIntRange(8, 8));
    QCOMPARE(subseq.rightTrimRange(9), ClosedIntRange());
}

void TestSubseq::setBioString_data()
{
    QTest::addColumn<QString>("parentSeq");
    QTest::addColumn<QString>("gapped_sequence");
    QTest::addColumn<bool>("return_value");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("stop");
    QTest::addColumn<QString>("actual_sequence");

    QTest::newRow("empty biostring") << "ABCDEF" << "" << false << 1 << 6 << "ABCDEF";
    QTest::newRow("pure gaps") << "ABCDEF" << ".-.-.-.--" << false << 1 << 6 << "ABCDEF";
    QTest::newRow("invalid sequence: XYZ") << "ABCDEF" << "XYZ" << false << 1 << 6 << "ABCDEF";
    QTest::newRow("invalid sequence: BACDEF") << "ABCDEF" << "BACDEF" << false << 1 << 6 << "ABCDEF";
    QTest::newRow("partial match: ABC") << "ABCDEF" << "ABC" << true << 1 << 3 << "ABC";
    QTest::newRow("partial match: BCD") << "ABCDEF" << "BCD" << true << 2 << 4 << "BCD";
    QTest::newRow("partial match: DEF") << "ABCDEF" << "DEF" << true << 4 << 6 << "DEF";
    QTest::newRow("gapped partial match: -A-B-C-") << "ABCDEF" << "-A-B-C-" << true << 1 << 3 << "-A-B-C-";
    QTest::newRow("mixed gapped partial match: ...BCD---") << "ABCDEF" << "...BCD---" << true << 2 << 4 << "...BCD---";
    QTest::newRow("gapped partial match: D---EF") << "ABCDEF" << "D---EF" << true << 4 << 6 << "D---EF";
    QTest::newRow("exact parent sequence") << "ABCDEF" << "ABCDEF" << true << 1 << 6 << "ABCDEF";
}

void TestSubseq::setBioString()
{
    QFETCH(QString, parentSeq);
    QFETCH(QString, gapped_sequence);
    QFETCH(bool, return_value);
    QFETCH(int, start);
    QFETCH(int, stop);
    QFETCH(QString, actual_sequence);

    QByteArray parentSeq2 = parentSeq.toAscii();
    QByteArray gapped_sequence2 = gapped_sequence.toAscii();
    QByteArray actual_sequence2 = actual_sequence.toAscii();

    Seq parent(parentSeq2.constData(), eAminoGrammar);
    BioString bioString(gapped_sequence2);

    // BioString variant
    {
        Subseq subseq(parent);
        QCOMPARE(subseq.setBioString(bioString), return_value);
        QCOMPARE(subseq.start(), start);
        QCOMPARE(subseq.stop(), stop);
        QCOMPARE(subseq.constData(), actual_sequence2.constData());
        QCOMPARE(subseq.grammar(), eAminoGrammar);
    }

    // QByteArray variant
    {
        Subseq subseq(parent);
        QCOMPARE(subseq.setBioString(gapped_sequence2), return_value);
        QCOMPARE(subseq.start(), start);
        QCOMPARE(subseq.stop(), stop);
        QCOMPARE(subseq.constData(), actual_sequence2.constData());
        QCOMPARE(subseq.grammar(), eAminoGrammar);
    }

    // const char * variant
    {
        Subseq subseq(parent);
        QCOMPARE(subseq.setBioString(gapped_sequence2.constData()), return_value);
        QCOMPARE(subseq.start(), start);
        QCOMPARE(subseq.stop(), stop);
        QCOMPARE(subseq.constData(), actual_sequence2.constData());
        QCOMPARE(subseq.grammar(), eAminoGrammar);
    }
}

void TestSubseq::setStart()
{
    Subseq subseq(Seq("ABCDEF"));
    BioString test_string = "-C---D-";

    // Setup and check
    QVERIFY(subseq.setBioString(test_string));
    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);

    // Test: setting start to existing start should also be valid
    subseq.setStart(3);
    QVERIFY(subseq == "-C---D-");

    // Test: tweaking start by one
    subseq.setStart(2);
    QVERIFY(subseq == "BC---D-");
    subseq.setStart(3);
    QVERIFY(subseq == "-C---D-");

    // Test: move start beyond current sequence space
    subseq.setStart(1);
    QVERIFY(subseq == "ABC---D-");

    // Move it back and verify that we have an extra gap character now
    subseq.setStart(3);
    QVERIFY(subseq == "--C---D-");

    // Test: set start up to D and then back to A
    QVERIFY(subseq.setBioString(test_string));
    subseq.setStart(4);
    QVERIFY(subseq == "-----D-");
    QVERIFY(subseq.start() == 4 && subseq.stop() == 4);
    subseq.setStart(1);
    QVERIFY(subseq == "--ABCD-");

    // Test: Move start beyond stop
    QVERIFY(subseq.setBioString(test_string));
    subseq.setStart(5);

    QVERIFY(subseq == "------E");
    QVERIFY(subseq.start() == 5 && subseq.stop() == 5);
    subseq.setStart(6);
    QVERIFY(subseq == "-------F");

    QVERIFY(subseq.setBioString(test_string));
    subseq.setStart(6);
    QVERIFY(subseq == "-------F");
    QVERIFY(subseq.start() == 6 && subseq.stop() == 6);

    subseq.setStart(1);
    QVERIFY(subseq == "--ABCDEF");
}

void TestSubseq::setStop()
{
    Subseq subseq(Seq("ABCDEF"));

    BioString test_string = "-C---D-";
    QVERIFY(subseq.setBioString(test_string));

    QVERIFY(subseq.start() == 3 && subseq.stop() == 4);
    QVERIFY(subseq.headGaps() == 1 and subseq.tailGaps() == 1);

    // Test: setting stop to existing stop should also be valid
    subseq.setStop(4);
    QVERIFY(subseq == test_string);

    // Test: tweaking stop by one
    subseq.setStop(5);
    QVERIFY(subseq == "-C---DE");
    subseq.setStop(4);
    QVERIFY(subseq == "-C---D-");

    // Test: move start beyond current sequence space
    subseq.setStop(6);
    QVERIFY(subseq == "-C---DEF");

    // Move it back and verify that we have an extra gap character now
    subseq.setStop(4);
    QVERIFY(subseq == "-C---D--");

    // Test: set stop down to C and then back to F
    QVERIFY(subseq.setBioString(test_string));
    subseq.setStop(3);
    QVERIFY(subseq == "-C-----");
    QVERIFY(subseq.start() == 3 && subseq.stop() == 3);
    QVERIFY(subseq.headGaps() == 1);
    QVERIFY(subseq.tailGaps() == 5);
    subseq.setStop(6);
    QVERIFY(subseq == "-CDEF--");

    // Test: Move stop before start
    QVERIFY(subseq.setBioString(test_string));
    //                 -C---D-
    subseq.setStop(2);
    QVERIFY(subseq == "B------");
    QVERIFY(subseq.start() == 2 && subseq.stop() == 2);
    subseq.setStop(1);
    QVERIFY(subseq == "A-------");

    QVERIFY(subseq.setBioString(test_string));
    subseq.setStop(1);
    QVERIFY(subseq == "A-------");
    QVERIFY(subseq.start() == 1 && subseq.stop() == 1);

    subseq.setStop(6);
    QVERIFY(subseq == "ABCDEF--");
}

void TestSubseq::trimLeft()
{
    Subseq subseq(Seq("ABCDEF"));

    Subseq::Trim trim;
    for (int i=1; i< 6; ++i)
    {
        trim.subseqRange_.begin_ = i;
        trim.subseqRange_.end_ = i;
        trim.nNonGaps_ = 1;

        subseq.trimLeft(trim);
        QVERIFY(subseq == ::QByteArray("-").repeated(i) + subseq.mid(i+1));
        QCOMPARE(subseq.start(), i+1);
    }

    // Test: same as above, but without calculating non-gaps
    QVERIFY(subseq.setBioString("ABCDEF"));
    trim.nNonGaps_ = 0;
    for (int i=1; i< 6; ++i)
    {
        trim.subseqRange_.begin_ = i;
        trim.subseqRange_.end_ = i;

        subseq.trimLeft(trim);
        QVERIFY(subseq == ::QByteArray("-").repeated(i) + subseq.mid(i+1));
        QCOMPARE(subseq.start(), i+1);
    }

    // Test: trim multiple characters
    QVERIFY(subseq.setBioString("ABCDEF"));
    trim.subseqRange_.begin_ = 1;
    trim.subseqRange_.end_ = 5;
    trim.nNonGaps_ = 5;
    subseq.trimLeft(trim);
    QVERIFY(subseq == "-----F");
    QCOMPARE(subseq.start(), 6);
    QCOMPARE(subseq.stop(), 6);

    // Test: without nongaps
    QVERIFY(subseq.setBioString("ABCDEF"));
    trim.subseqRange_.begin_ = 1;
    trim.subseqRange_.end_ = 4;
    trim.nNonGaps_ = 0;
    subseq.trimLeft(trim);
    QVERIFY(subseq == "----EF");
    QCOMPARE(subseq.start(), 5);
    QCOMPARE(subseq.stop(), 6);

    // Test: gapped regions
    QVERIFY(subseq.setBioString("--AB-C-DE--"));
    trim.subseqRange_.begin_ = 1;
    trim.subseqRange_.end_ = 3;
    trim.nNonGaps_ = 1;
    subseq.trimLeft(trim);
    QVERIFY(subseq == "---B-C-DE--");
    QCOMPARE(subseq.start(), 2);
    QCOMPARE(subseq.stop(), 5);

    trim.nNonGaps_ = 0; // From now on, auto-calculate this

    trim.subseqRange_.begin_ = 3;
    trim.subseqRange_.end_ = 5;
    subseq.trimLeft(trim);
    QVERIFY(subseq == "-----C-DE--");
    QCOMPARE(subseq.start(), 3);
    QCOMPARE(subseq.stop(), 5);

    trim.subseqRange_.begin_ = 6;
    trim.subseqRange_.end_ = 7;
    subseq.trimLeft(trim);
    QVERIFY(subseq == "-------DE--");
    QCOMPARE(subseq.start(), 4);
    QCOMPARE(subseq.stop(), 5);
}


void TestSubseq::trimRight()
{
    Subseq subseq(Seq("ABCDEF"));

    Subseq::Trim trim;
    for (int i=6; i>1; --i)
    {
        trim.subseqRange_.begin_ = i;
        trim.subseqRange_.end_ = i;
        trim.nNonGaps_ = 1;

        subseq.trimRight(trim);
        QVERIFY(subseq == subseq.mid(ClosedIntRange(1, i-1)) + ::QByteArray("-").repeated(6 - i + 1));
        QCOMPARE(subseq.stop(), i-1);
    }

    // Test: same as above, but without calculating non-gaps
    QVERIFY(subseq.setBioString("ABCDEF"));
    trim.nNonGaps_ = 0;
    for (int i=6; i>1; --i)
    {
        trim.subseqRange_.begin_ = i;
        trim.subseqRange_.end_ = i;

        subseq.trimRight(trim);
        QVERIFY(subseq == subseq.mid(ClosedIntRange(1, i-1)) + ::QByteArray("-").repeated(6 - i + 1));
        QCOMPARE(subseq.stop(), i-1);
    }

    // Test: trim multiple characters
    QVERIFY(subseq.setBioString("ABCDEF"));
    trim.subseqRange_.begin_ = 2;
    trim.subseqRange_.end_ = 6;
    trim.nNonGaps_ = 5;
    subseq.trimRight(trim);
    QVERIFY(subseq == "A-----");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 1);

    // Test: without nongaps
    QVERIFY(subseq.setBioString("ABCDEF"));
    trim.subseqRange_.begin_ = 3;
    trim.subseqRange_.end_ = 6;
    trim.nNonGaps_ = 0;
    subseq.trimRight(trim);
    QVERIFY(subseq == "AB----");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 2);

    // Test: gapped regions
    //                                   ***
    QVERIFY(subseq.setBioString("--AB-C-DE--"));
    trim.subseqRange_.begin_ = 9;
    trim.subseqRange_.end_ = 11;
    trim.nNonGaps_ = 1;
    subseq.trimRight(trim);
    QVERIFY(subseq == "--AB-C-D---");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 4);

    trim.nNonGaps_ = 0; // From now on, auto-calculate this

    //       ***
    // --AB-C-D---
    trim.subseqRange_.begin_ = 7;
    trim.subseqRange_.end_ = 9;
    subseq.trimRight(trim);
    QVERIFY(subseq == "--AB-C-----");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 3);

    //      **
    // --AB-C-----
    trim.subseqRange_.begin_ = 6;
    trim.subseqRange_.end_ = 7;
    subseq.trimRight(trim);
    QVERIFY(subseq == "--AB-------");
    QCOMPARE(subseq.start(), 1);
    QCOMPARE(subseq.stop(), 2);
}

QTEST_APPLESS_MAIN(TestSubseq)
#include "TestSubseq.moc"
