/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../DnaPattern.h"
#include "../BioString.h"

class TestDnaPattern : public QObject
{
    Q_OBJECT

private slots:
    void constructor();     // Also tests pattern

    void clear();
    void isEmpty();
    void isValidPattern_data();
    void isValidPattern();  // Also tests isValid
    void indexIn_data();
    void indexIn();
    void length();
    void matchesAt_data();
    void matchesAt();
    void matchesAtBeginning();
    void matchesAtEnd();
    void setPattern();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestDnaPattern::constructor()
{
    DnaPattern x;

    QVERIFY(x.pattern().isEmpty());
    QVERIFY(x.isValid());

    DnaPattern x2("BLARGH");
    QCOMPARE(x2.pattern(), QByteArray("BLARGH"));
}

void TestDnaPattern::clear()
{
    DnaPattern x("DEF");

    QCOMPARE(x.pattern(), QByteArray("DEF"));
    x.clear();
    QCOMPARE(x.pattern(), QByteArray());
}

void TestDnaPattern::isEmpty()
{
    DnaPattern x;

    QVERIFY(x.isEmpty());
    x.setPattern("ATCG");
    QVERIFY(!x.isEmpty());
    x.clear();
    QVERIFY(x.isEmpty());
}

void TestDnaPattern::isValidPattern_data()
{
    QTest::addColumn<QByteArray>("pattern");
    QTest::addColumn<bool>("valid");

    QTest::newRow("empty") << QByteArray() << true;

    // Each valid character by itself
    QByteArray validChars("GATCRYMKSWHBVDN");
    for (int i=0; i< validChars.length(); ++i)
    {
        QByteArray byteArray(1, validChars.at(i));
        QTest::newRow(byteArray) << byteArray << true;

        // Lower case version should fail
        QTest::newRow(byteArray.toLower()) << byteArray.toLower() << false;

        // Doubles
        QByteArray sameCharTwice = byteArray.repeated(2);
        QTest::newRow(sameCharTwice) << sameCharTwice << true;
    }

    QTest::newRow(" ") << QByteArray(" ") << true;
    QTest::newRow("-") << QByteArray("-") << true;

    // Some invalid tests
    QByteArray invalidChars("abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()");
    for (int i=0; i< invalidChars.length(); ++i)
    {
        QByteArray byteArray(1, invalidChars.at(i));
        QTest::newRow(byteArray) << byteArray << false;
    }

    // Mix of valid and invalid
    QTest::newRow("AaGg") << QByteArray("AaGg") << false;
}

void TestDnaPattern::isValidPattern()
{
    QFETCH(QByteArray, pattern);
    QFETCH(bool, valid);

    {
        DnaPattern x;
        QCOMPARE(x.isValidPattern(pattern), valid);
    }

    {
        DnaPattern x(pattern);
        QCOMPARE(x.isValid(), valid);
    }
}

void TestDnaPattern::indexIn_data()
{
    QTest::addColumn<QByteArray>("pattern");
    QTest::addColumn<BioString>("bioString");
    QTest::addColumn<int>("offset");
    QTest::addColumn<int>("position");

    QTest::newRow("empty") << QByteArray() << BioString() << 1 << -1;

    //                   *  *             *
    //                   123456789012345678901
    BioString bioString("AGTAGTAACCGTNACGTAGTG");
    QByteArray pattern("AGT");
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 1") << pattern << bioString << 1 << 1;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 2") << pattern << bioString << 2 << 4;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 3") << pattern << bioString << 3 << 4;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 4") << pattern << bioString << 4 << 4;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 5") << pattern << bioString << 5 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 6") << pattern << bioString << 6 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 7") << pattern << bioString << 7 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 8") << pattern << bioString << 8 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 9") << pattern << bioString << 9 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 10") << pattern << bioString << 10 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 11") << pattern << bioString << 11 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 12") << pattern << bioString << 12 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 13") << pattern << bioString << 13 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 14") << pattern << bioString << 14 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 15") << pattern << bioString << 15 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 16") << pattern << bioString << 16 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 17") << pattern << bioString << 17 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 18") << pattern << bioString << 18 << 18;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 19") << pattern << bioString << 19 << -1;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 20") << pattern << bioString << 20 << -1;
    QTest::newRow("input: AGTAGTAACCGTNACGTAGTG, offset: 21") << pattern << bioString << 21 << -1;
}

void TestDnaPattern::indexIn()
{
    QFETCH(QByteArray, pattern);
    QFETCH(BioString, bioString);
    QFETCH(int, offset);
    QFETCH(int, position);

    DnaPattern x(pattern);
    QVERIFY(x.isValid());
    QCOMPARE(x.indexIn(bioString, offset), position);
}

void TestDnaPattern::length()
{
    DnaPattern x;

    QCOMPARE(x.length(), 0);
    x.setPattern("ABC");
    QCOMPARE(x.length(), 3);
    x.clear();
    QCOMPARE(x.length(), 0);
}

void TestDnaPattern::matchesAt_data()
{
    QTest::addColumn<QByteArray>("pattern");
    QTest::addColumn<BioString>("bioString");
    QTest::addColumn<int>("offset");
    QTest::addColumn<bool>("matches");

    QTest::newRow("Empty biostring") << QByteArray() << BioString() << 1 << false;
    QTest::newRow("empty pattern, non-empty biostring") << QByteArray() << BioString("ABC") << 1 << false;

    QByteArray pattern("ACGT");
    BioString bioString("ACGT");
    BioString lowerBioString("acgt");
    for (int i=0; i< pattern.length(); ++i)
    {
        QByteArray inputPattern(1, pattern.at(i));
        QTest::newRow(inputPattern + " pattern, " + bioString.asByteArray()) << inputPattern << bioString << i + 1 << true;
        QTest::newRow(inputPattern + " pattern, " + lowerBioString.asByteArray()) << inputPattern << lowerBioString << i + 1 << true;
    }

    // Test: double characters
    bioString = "ACGTacgtBbDdEeFfHhIiJjKkLlMmNnOoPpQqRrSsUuVvWwXxYyZz1234567890~!@#$%^&*()";
    pattern = "R";  // AG
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 1 || i == 3 || i == 5 || i == 7);

    pattern = "Y";  // CT
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 2 || i == 4 || i == 6 || i == 8);

    pattern = "M";  // AC
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 1 || i == 2 || i == 5 || i == 6);

    pattern = "K";  // GT
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 3 || i == 4 || i == 7 || i == 8);

    pattern = "S";  // CG
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 2 || i == 3 || i == 6 || i == 7);

    pattern = "W";  // AT
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 1 || i == 4 || i == 5 || i == 8);

    // Triple characters
    pattern = "H";  // ACT
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i <= 8 && i != 3 && i != 7);

    pattern = "B";  // CTG
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i <= 8 && i != 1 && i != 5);

    pattern = "V";  // ACG
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i <= 8 && i != 4 && i != 8);

    pattern = "D";  // AGT
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i <= 8 && i != 2 && i != 6);

    // Any ACGT
    pattern = "N";  // ACGT
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: ACGTacgt, offset: " + QByteArray::number(i)) << pattern << bioString << i << (i <= 8);

    // Gaps
    pattern = "-";
    bioString = "A-C-G-T-a.c.g.t.BbDdEeFfHhIiJjKkLlMmNnOoPpQqRrSsUuVvWwXxYyZz1234567890~!@#$%^&*()";
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow(pattern + " pattern, input: A-C-G-T-a.c.g.t., offset: " + QByteArray::number(i)) << pattern << bioString << i << (i % 2 == 0 && i <= 16);

    // Space characters
    pattern = " ";
    bioString = "ABCDEFGHJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()_  ";
    for (int i=1; i<= bioString.length(); ++i)
        QTest::newRow("[space] pattern, input: all letters, offset: " + QByteArray::number(i)) << pattern << bioString << i << true;

    // Two characters in pattern
    pattern = "AC";
    bioString = "ACATAC";
    for (int i=1; i<= bioString.length() - pattern.length(); ++i)
        QTest::newRow(pattern + " pattern, input: " + bioString.asByteArray() + ", offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 1 || i == 5);

    pattern = "A R";
    for (int i=1; i<= bioString.length() - pattern.length(); ++i)
        QTest::newRow(pattern + " pattern, input: " + bioString.asByteArray() + ", offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 1 || i == 3);

    pattern = "ADR";
    for (int i=1; i<= bioString.length() - pattern.length(); ++i)
        QTest::newRow(pattern + " pattern, input: " + bioString.asByteArray() + ", offset: " + QByteArray::number(i)) << pattern << bioString << i << (i == 3);
}

void TestDnaPattern::matchesAt()
{
    QFETCH(QByteArray, pattern);
    QFETCH(BioString, bioString);
    QFETCH(int, offset);
    QFETCH(bool, matches);

    DnaPattern x(pattern);
    QVERIFY(x.isValid());
    QCOMPARE(x.matchesAt(bioString, offset), matches);
}

void TestDnaPattern::matchesAtBeginning()
{
    DnaPattern x("AGT");
    QVERIFY(x.isValid());
    QCOMPARE(x.matchesAtBeginning(BioString("AGTACGT")), true);
    QCOMPARE(x.matchesAtBeginning(BioString("ACTACGT")), false);

    x.setPattern("RR -");
    QVERIFY(x.isValid());
    QCOMPARE(x.matchesAtBeginning(BioString("AGT-JUNK")), true);
    QCOMPARE(x.matchesAtBeginning(BioString("rr -")), false);
}

void TestDnaPattern::matchesAtEnd()
{
    DnaPattern x("TAA");
    QVERIFY(x.isValid());
    QCOMPARE(x.matchesAtEnd(BioString("AGTATAA")), true);
    QCOMPARE(x.matchesAtEnd(BioString("ACTACGT")), false);

    x.setPattern("RR -");
    QVERIFY(x.isValid());
    QCOMPARE(x.matchesAtEnd(BioString("AGT-JUNKGG*.")), true);
    QCOMPARE(x.matchesAtEnd(BioString("rr -")), false);
}

void TestDnaPattern::setPattern()
{
    DnaPattern x;

    x.setPattern("BLARGH");
    QCOMPARE(x.pattern(), QByteArray("BLARGH"));

    x.setPattern(QByteArray());
    QCOMPARE(x.pattern(), QByteArray());
}

QTEST_APPLESS_MAIN(TestDnaPattern)
#include "TestDnaPattern.moc"
