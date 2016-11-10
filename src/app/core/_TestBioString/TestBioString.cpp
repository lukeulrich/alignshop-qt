/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QTime>

#include "../BioString.h"
#include "../misc.h"

#include "../constants.h"

class TestBioString : public QObject
{
    Q_OBJECT

public:
    TestBioString() : QObject()
    {
        // Build the arrays of all possible positive and negative chars
        for (char c = -128; c < 0; ++c)
            negativeChars[c+128] = c;
        negativeChars[128] = '\0';

        for (char c = 1; c < 127; ++c)
            positiveChars[c-1] = c;
        positiveChars[126] = 127;
        positiveChars[127] = '\0';

        // Build the array of allowable chars
        for (char c = 32; c < 127; ++c)
            allowableChars[c-32] = c;
        allowableChars[127-32] = '\0';
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorEmpty();
    void constructorGrammar();
    void constructorChar();
    void constructorByteArray();
    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    void assignChar();
    void assignBioString();
    void assignByteArray();

    // ------------------------------------------------------------------------------------------------
    // Remaining operators
    void operator_addeq();
    void operator_eqeq();
    void operator_ne();
    void operator_add();
    void operator_lt();

    // ------------------------------------------------------------------------------------------------
    // Static methods
    void containsUnallowedChars();
    void maskUnallowedChars_cstr();
    void maskUnallowedChars_bytearray();
    void removeUnallowedChars_cstr();
    void removeUnallowedChars_bytearray();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void append_biostring();
    void append_cstr();
    void append_char();
    void asByteArray();
    void backTranscribe();
    void collapseLeft();
    void collapseRight();
    void complement();
    void count();
    void digest();
    void gapsBetween();
    void gapsLeftOf();
    void gapsRightOf();
    void grammar();         // Also tests setGrammar
    void hasGapAt();
    void hasGaps();
    void hasNonGaps();
    void headGaps();
    void insert();
    void insertGaps();
    void isEquivalentTo();
    void isExactMatch();
    void isPalindrome_data();
    void isPalindrome();
    void isValidPosition();
    void isValidRange();
    void leftSlidablePositions();
    void mid();
    void mid_range();
    void nonGapsBetween_data();
    void nonGapsBetween();
    void onlyContainsACGT();
    void onlyContainsACGT_range();
    void prepend_biostring();
    void prepend_cstr();
    void prepend_char();
    void remove_range();
    void removeGaps();
    void removeGaps_poslen();
    void replace();
    void replace_range();
    void reverse();
    void reverseComplement();
    void rightSlidablePositions();
    void slide();
    void tailgaps();
    void tr();
    void transcribe();
    void translateGaps();
    void ungapped();
    void ungappedLength();


    // ------------------------------------------------------------------------------------------------
    // Benchmarks
    void benchSlide();
    void benchSlideViaSwap();

private:
    char negativeChars[129];
    char positiveChars[128];
    char allowableChars[96];    // 127 - 32 + 1

};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions

// ------------------------------------------------------------------------------------------------
// Constructors
void TestBioString::constructorEmpty()
{
    BioString b;
    QVERIFY(b.isEmpty());
    QVERIFY(b.isNull());
    QCOMPARE(b.grammar(), eUnknownGrammar);
}

void TestBioString::constructorGrammar()
{
    BioString b(eUnknownGrammar);
    QCOMPARE(b.grammar(), eUnknownGrammar);

    BioString b1(eAminoGrammar);
    QCOMPARE(b1.grammar(), eAminoGrammar);

    BioString b2(eDnaGrammar);
    QCOMPARE(b2.grammar(), eDnaGrammar);

    BioString b3(eRnaGrammar);
    QCOMPARE(b3.grammar(), eRnaGrammar);
}

void TestBioString::constructorChar()
{
    BioString b("C", eDnaGrammar);
    QCOMPARE(b.grammar(), eDnaGrammar);
    QCOMPARE(b.constData(), "C");

    BioString b1("");
    QCOMPARE(b1.grammar(), eUnknownGrammar);
    QCOMPARE(b1.constData(), "");

    BioString b2("ABC DEF -- ..GHI", eRnaGrammar);
    QCOMPARE(b2.grammar(), eRnaGrammar);
    QCOMPARE(b2.constData(), "ABCDEF--..GHI");

    BioString b3("\r\t\v\f\n");
    QCOMPARE(b3.constData(), "");
    QVERIFY(b3.isEmpty());
    QCOMPARE(b3.length(), 0);
}

void TestBioString::constructorByteArray()
{
    char str[] = "\t\r\n\v\f ABcdEFGHJI";
    char str2[] = "ABcdEFGHJI";
    QByteArray ba(str);
    BioString bioString(ba);
    QCOMPARE(bioString.grammar(), eUnknownGrammar);
    QCOMPARE(bioString.constData(), str2);

    BioString bioString2(ba, eDnaGrammar);
    QCOMPARE(bioString2.grammar(), eDnaGrammar);
}

void TestBioString::constructorCopy()
{
    char str[] = "1234567891ab";
    BioString b1(str, eAminoGrammar);
    BioString b2(b1);

    QCOMPARE(b2.grammar(), eAminoGrammar);
    QCOMPARE(b2.constData(), str);
}


// ------------------------------------------------------------------------------------------------
// Assignment operators
void TestBioString::assignChar()
{
    BioString biostring = "ABcdEF12345@";
    QCOMPARE(biostring.constData(), "ABcdEF12345@");
    QCOMPARE(biostring.grammar(), eUnknownGrammar);

    biostring.setGrammar(eAminoGrammar);

    char test2[] = "  abc  1234 dEf";
    biostring = test2;
    QCOMPARE(biostring.constData(), "abc1234dEf");
    QCOMPARE(biostring.grammar(), eAminoGrammar);

    biostring = 'X';
    QCOMPARE(biostring.constData(), "X");
}

void TestBioString::assignBioString()
{
    BioString b1("GENOMICS");
    BioString b2("COMPUTATIONAL");

    b1 = b2;
    QCOMPARE(b1.constData(), "COMPUTATIONAL");
}

void TestBioString::assignByteArray()
{
    BioString b1(eRnaGrammar);
    QByteArray ba = "A B\tC\rD\nE\fF";

    b1 = ba;
    QCOMPARE(b1.constData(), "ABCDEF");
    QCOMPARE(b1.grammar(), eRnaGrammar);
}


// ------------------------------------------------------------------------------------------------
// Remaining operators
void TestBioString::operator_addeq()
{
    BioString biostring = "ABC...def";

    // Test: adding BioString
    biostring += BioString("GHI");
    QCOMPARE(biostring.constData(), "ABC...defGHI");

    // Test: adding char strings
    biostring += "vx";
    biostring += "yz";
    QCOMPARE(biostring.constData(), "ABC...defGHIvxyz");

    biostring = "";
    // Test: adding chars
    biostring += 'C';
    QCOMPARE(biostring.constData(), "C");
    biostring += 'D';
    QCOMPARE(biostring.constData(), "CD");

    // Test: adding QByteArray
    biostring += QByteArray("EF");
    QCOMPARE(biostring.constData(), "CDEF");

    // Test: QByteArray with whitespace
    biostring = "ABC";
    biostring += QByteArray(" D\nE\fF\rG\vH\tI");
    QCOMPARE(biostring.constData(), "ABCDEFGHI");

    // Test: adding whitespace
    biostring = "";
    char whitespace[] = "\t\f\r\n\v ";
    const char *x = whitespace;
    for (; *x; ++x)
    {
        biostring += *x;
        QVERIFY(biostring.isEmpty());
    }

    biostring += whitespace;
    QVERIFY(biostring.isEmpty());
}

void TestBioString::operator_eqeq()
{
    BioString biostring = "ABC---def";

    // Test: comparing to BioStrings
    QVERIFY(biostring == biostring);
    QVERIFY(biostring == BioString("ABC---def"));
    QVERIFY(!(biostring == "BC---def"));

    biostring.setGrammar(eDnaGrammar);
    BioString biostring2("ABC---def", eDnaGrammar);
    QVERIFY(biostring == biostring2);
    biostring2.setGrammar(eRnaGrammar);
    QVERIFY(!(biostring == biostring2));

    // Test: comparing to rote strings
    QVERIFY(biostring == "ABC---def");
    QVERIFY(!(biostring == "BC---def"));
}

void TestBioString::operator_ne()
{
    BioString biostring = "ABC---DEF";

    // Test: comparing to BioStrings
    QVERIFY(!(biostring != biostring));
    QVERIFY(biostring != BioString("BC---DEF"));
    QVERIFY(biostring != QByteArray("BC---DEF"));

    BioString biostring2(biostring);
    biostring2.setGrammar(eAminoGrammar);
    QVERIFY(biostring != biostring2);

    // Test: comparing to rote strings
    QVERIFY(biostring != "");
    QVERIFY(biostring != "BC---DEFG");
    QVERIFY(biostring != "ABC.-.DEF");
}

void TestBioString::operator_add()
{
    BioString left = "ABC";
    BioString right = "DEF";
    BioString both = left + right;

    QCOMPARE(left.constData(), "ABC");
    QCOMPARE(right.constData(), "DEF");
    QCOMPARE(both.constData(), "ABCDEF");
    QCOMPARE(both.grammar(), eUnknownGrammar);

    // Test: Check adding blank right
    both = left + "";
    QCOMPARE(both.constData(), "ABC");

    // Test: Check adding blank left
    both = "" + right;
    QCOMPARE(both.constData(), "DEF");

    left.setGrammar(eDnaGrammar);
    right.setGrammar(eDnaGrammar);
    both = left + right;
    QCOMPARE(both.grammar(), eDnaGrammar);

    // Test: QByteArray
    both = left + QByteArray("");
    QCOMPARE(both.constData(), "ABC");
    QCOMPARE(both.grammar(), eDnaGrammar);

    both = QByteArray("") + right;
    QCOMPARE(both.constData(), "DEF");
    QCOMPARE(both.grammar(), eDnaGrammar);

    both = left + QByteArray("GHI");
    QCOMPARE(both.constData(), "ABCGHI");

    both = QByteArray("X.-Y-Z") + right;
    QCOMPARE(both.constData(), "X.-Y-ZDEF");
}

void TestBioString::operator_lt()
{
    BioString alpha("A");
    BioString beta("B");

    QVERIFY(alpha < beta);
}

void TestBioString::containsUnallowedChars()
{
    QByteArray buffer = "ABCDEF";
    QCOMPARE(BioString::containsUnallowedChars(buffer), false);

    buffer[5] = 2;
    QCOMPARE(BioString::containsUnallowedChars(buffer), true);
}

void TestBioString::maskUnallowedChars_cstr()
{
    // Test: negative values
    int l = qstrlen(negativeChars);
    QCOMPARE(BioString::maskUnallowedChars(negativeChars, '@'), QByteArray("@").repeated(l));

    // Test: positive values
    QCOMPARE(BioString::maskUnallowedChars(positiveChars, 'X'), QByteArray("X").repeated(31) + QByteArray(allowableChars) + "X");

    // Test: allowed and unallowed interspersed
    //             012345
    char test[] = "ABCDEF";
    test[1] = 5;
    test[4] = 127;
    QCOMPARE(BioString::maskUnallowedChars(test, '^'), QByteArray("A^CD^F"));
}

void TestBioString::maskUnallowedChars_bytearray()
{
    // Test: negative values
    int l = qstrlen(negativeChars);
    QCOMPARE(BioString::maskUnallowedChars(QByteArray(negativeChars), '@'), QByteArray("@").repeated(l));

    // Test: positive values
    QCOMPARE(BioString::maskUnallowedChars(QByteArray(positiveChars), 'X'), QByteArray("X").repeated(31) + QByteArray(allowableChars) + "X");

    // Test: allowed and unallowed interspersed
    //             012345
    char test[] = "ABCDEF";
    test[1] = 5;
    test[4] = 127;
    QCOMPARE(BioString::maskUnallowedChars(QByteArray(test), '^'), QByteArray("A^CD^F"));
}

void TestBioString::removeUnallowedChars_cstr()
{
    // Test: all negative valued chars
    QVERIFY(BioString::removeUnallowedChars(negativeChars).isEmpty());

    // Test: all positive valued chars
    QCOMPARE(BioString::removeUnallowedChars(positiveChars), QByteArray(allowableChars));

    // Test: allowed and unallowed interspersed
    //             012345
    char test[] = "ABCDEF";
    test[1] = 5;
    test[4] = 127;
    QCOMPARE(BioString::removeUnallowedChars(test), QByteArray("ACDF"));
}

void TestBioString::removeUnallowedChars_bytearray()
{
    // Test: all negative valued chars
    QVERIFY(BioString::removeUnallowedChars(QByteArray(negativeChars)).isEmpty());

    // Test: all positive valued chars
    QCOMPARE(BioString::removeUnallowedChars(QByteArray(positiveChars)), QByteArray(allowableChars));

    // Test: allowed and unallowed interspersed
    //             012345
    char test[] = "ABCDEF";
    test[1] = 5;
    test[4] = 127;
    QCOMPARE(BioString::removeUnallowedChars(QByteArray(test)), QByteArray("ACDF"));
}

void TestBioString::append_biostring()
{
    BioString biostring = "ABC";

    biostring.append(BioString("DEF"));
    QCOMPARE(biostring.constData(), "ABCDEF");

    biostring.append("G").append("H");
    QCOMPARE(biostring.constData(), "ABCDEFGH");
}

void TestBioString::append_cstr()
{
    BioString biostring;

    biostring.append("ABC");
    QCOMPARE(biostring.constData(), "ABC");

    biostring.append("DEF").append("GHI");
    QCOMPARE(biostring.constData(), "ABCDEFGHI");
}

void TestBioString::append_char()
{
    BioString biostring;

    biostring.append('a');
    QCOMPARE(biostring.constData(), "a");

    biostring.append('b').append('C');
    QCOMPARE(biostring.constData(), "abC");
}

void TestBioString::asByteArray()
{
    BioString amino("ABCDEF", eAminoGrammar);

    QByteArray x = amino.asByteArray();
    QCOMPARE(x, QByteArray("ABCDEF"));
}

void TestBioString::backTranscribe()
{
    BioString rna(" AuUcCgG..-u", eRnaGrammar);

    BioString dna = rna.backTranscribe();
    QCOMPARE(dna.grammar(), eDnaGrammar);
    QCOMPARE(dna.constData(), "AtTcCgG..-t");
}

QByteArray buildExpectedCollapseLeft(QByteArray input, int start, int stop)
{
    int l = stop - start + 1;
    QByteArray extract = input.mid(start - 1, l).replace('-', "");
    extract += QByteArray("-").repeated(l - extract.length());
    return input.replace(start - 1, l, extract);
}

ClosedIntRange findDifferences(const QByteArray &a, const QByteArray &b)
{
    ClosedIntRange range;

    const char *x = a.constData();
    const char *y = b.constData();
    for (int i=1; *x != '\0'; ++i, ++x, ++y)
    {
        if (*x != *y)
        {
            if (range.begin_ == 0)
                range.begin_ = i;

            range.end_ = i;
        }
    }

    return range;
}

void TestBioString::collapseLeft()
{
    QList<QByteArray> seqs;
    seqs << "A-BC-D-E"
         << "-A-BCD--"
         << "--A---BC";

    foreach (const QByteArray &seq, seqs)
    {
        int l = seq.length();
        for (int start=1; start <= l; ++start)
        {
            for (int stop=start; stop <= l; ++stop)
            {
                BioString x(seq);
                QByteArray expected = buildExpectedCollapseLeft(seq, start, stop);
                ClosedIntRange range = x.collapseLeft(ClosedIntRange(start, stop));
                QCOMPARE(x.constData(), expected.constData());

                ClosedIntRange expectedRange = findDifferences(seq, expected);
                QCOMPARE(range.begin_, expectedRange.begin_);
                QCOMPARE(range.end_, expectedRange.end_);
            }
        }
    }
}

QByteArray buildExpectedCollapseRight(QByteArray input, int start, int stop)
{
    int l = stop - start + 1;
    QByteArray extract = input.mid(start - 1, l).replace('-', "");
    extract = QByteArray("-").repeated(l - extract.length()) + extract;
    return input.replace(start - 1, l, extract);
}

void TestBioString::collapseRight()
{
    QVector<QByteArray> seqs;
    seqs << "A-BC-D-E"
         << "-A-BCD--"
         << "--A--BC-";

    foreach (const QByteArray &seq, seqs)
    {
        int l = seq.length();
        for (int start=1; start <= l; ++start)
        {
            for (int stop=start; stop <= l; ++stop)
            {
                BioString x(seq);
                QByteArray expected = buildExpectedCollapseRight(seq, start, stop);
                ClosedIntRange range = x.collapseRight(ClosedIntRange(start, stop));
                QCOMPARE(x.constData(), expected.constData());

                ClosedIntRange expectedRange = findDifferences(seq, expected);
                QCOMPARE(range.begin_, expectedRange.begin_);
                QCOMPARE(range.end_, expectedRange.end_);
            }
        }
    }
}

void TestBioString::complement()
{
    BioString biostring;
    BioString dna = biostring.complement();

    QVERIFY(dna.isEmpty());
    QCOMPARE(dna.grammar(), eDnaGrammar);

    biostring = allowableChars;
    dna = biostring.complement();
    QCOMPARE(dna.grammar(), eDnaGrammar);
    biostring.tr("ABCDGHKMTVabcdghkmtv",
                 "TVGHCDMKABtvghcdmkab");
    QCOMPARE(biostring.constData(), dna.constData());

    biostring = "ABCDGHKMTVabcdghkmtv";
    dna = biostring.complement();
    QCOMPARE(dna.constData(), "TVGHCDMKABtvghcdmkab");
}

void TestBioString::count()
{
    BioString emptyBioString;
    BioString atgBioString("ATG");
    BioString aBioString("AAAAA");

    QCOMPARE(emptyBioString.count(emptyBioString), 0);
    QCOMPARE(atgBioString.count(emptyBioString), 0);
    QCOMPARE(emptyBioString.count(atgBioString), 0);

    QCOMPARE(atgBioString.count("AT"), 1);
    QCOMPARE(atgBioString.count("TG"), 1);
    QCOMPARE(atgBioString.count("ATG"), 1);

    QCOMPARE(aBioString.count("A"), 5);
    QCOMPARE(aBioString.count("AA"), 4);
}

void TestBioString::mid()
{
    BioString biostring("ABCDEF", eAminoGrammar);

    BioString bob = biostring.mid(2, 2);
    QVERIFY(bob == "BC");
    QCOMPARE(bob.grammar(), eAminoGrammar);
}

void TestBioString::mid_range()
{
    QByteArray seq = "ABCDEFGH";
    int n = seq.length();

    BioString biostring = seq;
    biostring.setGrammar(eAminoGrammar);

    // Test: 1..n, i..n
    for (int i=1; i<= n; ++i)
    {
        for (int j=i; j<= n; ++j)
        {
            QCOMPARE(biostring.mid(ClosedIntRange(i, j)).constData(), seq.mid(i-1, j-i+1).constData());
            QCOMPARE(biostring.mid(ClosedIntRange(i, j)).grammar(), eAminoGrammar);
        }
    }

    // --------------------------------------
    // Spot checks
    QCOMPARE(biostring.mid(ClosedIntRange(3, 3)).constData(), "C");
    QCOMPARE(biostring.mid(ClosedIntRange(6, 8)).constData(), "FGH");
}

void TestBioString::nonGapsBetween_data()
{
    QTest::addColumn<QByteArray>("sequence");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("stop");
    QTest::addColumn<int>("nNonGaps");

    QTest::newRow("no gaps") << QByteArray("ABCDEF") << 1 << 6 << 6;
    QTest::newRow("no gaps part 2") << QByteArray("ABCDEF") << 2 << 5 << 4;

    QByteArray sequence = "ABC---DEF";
    for (int i=1; i<= sequence.length(); ++i)
        for (int j=i; j<= sequence.length(); ++j)
            QTest::newRow(QString("ABC---DEF :: %2-%3").arg(i).arg(j).toAscii())
                    << sequence
                    << i
                    << j
                    << (j-i+1) - sequence.mid(i-1, j-i+1).count('-');
}

void TestBioString::nonGapsBetween()
{
    QFETCH(QByteArray, sequence);
    QFETCH(int, start);
    QFETCH(int, stop);
    QFETCH(int, nNonGaps);

    QCOMPARE(BioString(sequence).nonGapsBetween(ClosedIntRange(start, stop)), nNonGaps);
}

void TestBioString::onlyContainsACGT()
{
    BioString bioString;

    QVERIFY(!bioString.onlyContainsACGT());

    bioString = "ACGT";
    QVERIFY(bioString.onlyContainsACGT());
    bioString += "N";
    QVERIFY(!bioString.onlyContainsACGT());
    bioString = "aCGT";
    QVERIFY(!bioString.onlyContainsACGT());
}

void TestBioString::onlyContainsACGT_range()
{
    BioString bioString;

    QVERIFY(!bioString.onlyContainsACGT(ClosedIntRange()));

    //           1234567890123
    bioString = "ACGTnacgtACGT";
    QVERIFY(bioString.onlyContainsACGT(ClosedIntRange(1, 4)));
    QVERIFY(bioString.onlyContainsACGT(ClosedIntRange(10, 13)));
    QVERIFY(bioString.onlyContainsACGT(ClosedIntRange(2, 3)));
    QVERIFY(bioString.onlyContainsACGT(ClosedIntRange(12, 12)));

    QVERIFY(!bioString.onlyContainsACGT(ClosedIntRange(1, 5)));
    QVERIFY(!bioString.onlyContainsACGT(ClosedIntRange(5, 9)));
    QVERIFY(!bioString.onlyContainsACGT(ClosedIntRange(8, 10)));
    QVERIFY(!bioString.onlyContainsACGT(ClosedIntRange(4, 5)));
}

void TestBioString::digest()
{
    BioString biostring = "ABCDEF--..GHIJ";

    QCOMPARE(biostring.digest(), QCryptographicHash::hash(biostring.asByteArray(), QCryptographicHash::Md5));
}

void TestBioString::gapsBetween()
{
    BioString biostring = "--A..B--CDE.F-G--";

    for (int i=1; i<= biostring.length(); ++i)
    {
        for (int j=i; j<= biostring.length(); ++j)
        {
            BioString part = biostring.mid(i, j-i+1);
            QCOMPARE(biostring.gapsBetween(ClosedIntRange(i, j)), part.count('-') + part.count('.'));
        }
    }
}

void TestBioString::gapsLeftOf()
{
    //                     1234567890123
    BioString biostring = "--AB----CD---";

    // Test: full positive range
    QVERIFY(biostring.gapsLeftOf(1) == 0);
    QVERIFY(biostring.gapsLeftOf(2) == 1);
    QVERIFY(biostring.gapsLeftOf(3) == 2);
    QVERIFY(biostring.gapsLeftOf(4) == 0);
    QVERIFY(biostring.gapsLeftOf(5) == 0);
    QVERIFY(biostring.gapsLeftOf(6) == 1);
    QVERIFY(biostring.gapsLeftOf(7) == 2);
    QVERIFY(biostring.gapsLeftOf(8) == 3);
    QVERIFY(biostring.gapsLeftOf(9) == 4);
    QVERIFY(biostring.gapsLeftOf(10) == 0);
    QVERIFY(biostring.gapsLeftOf(11) == 0);
    QVERIFY(biostring.gapsLeftOf(12) == 1);
    QVERIFY(biostring.gapsLeftOf(13) == 2);
}

void TestBioString::gapsRightOf()
{
    //                     1234567890123
    BioString biostring = "--AB----CD---";

    // Test: full positive range
    QVERIFY(biostring.gapsRightOf(1) == 1);
    QVERIFY(biostring.gapsRightOf(2) == 0);
    QVERIFY(biostring.gapsRightOf(3) == 0);
    QVERIFY(biostring.gapsRightOf(4) == 4);
    QVERIFY(biostring.gapsRightOf(5) == 3);
    QVERIFY(biostring.gapsRightOf(6) == 2);
    QVERIFY(biostring.gapsRightOf(7) == 1);
    QVERIFY(biostring.gapsRightOf(8) == 0);
    QVERIFY(biostring.gapsRightOf(9) == 0);
    QVERIFY(biostring.gapsRightOf(10) == 3);
    QVERIFY(biostring.gapsRightOf(11) == 2);
    QVERIFY(biostring.gapsRightOf(12) == 1);
    QVERIFY(biostring.gapsRightOf(13) == 0);
}

void TestBioString::grammar()
{
    BioString biostring("ABC", eAminoGrammar);
    QCOMPARE(biostring.grammar(), eAminoGrammar);

    biostring.setGrammar(eRnaGrammar);
    QCOMPARE(biostring.grammar(), eRnaGrammar);
}

void TestBioString::hasGapAt()
{
    //                     123456789
    BioString biostring = "-A.B-CD.-";

    QCOMPARE(biostring.hasGapAt(1), true);
    QCOMPARE(biostring.hasGapAt(2), false);
    QCOMPARE(biostring.hasGapAt(3), true);
    QCOMPARE(biostring.hasGapAt(4), false);
    QCOMPARE(biostring.hasGapAt(5), true);
    QCOMPARE(biostring.hasGapAt(6), false);
    QCOMPARE(biostring.hasGapAt(7), false);
    QCOMPARE(biostring.hasGapAt(8), true);
    QCOMPARE(biostring.hasGapAt(9), true);
}

void TestBioString::hasGaps()
{
    BioString biostring = "BOb";

    // Test does not have gaps
    QVERIFY(!biostring.hasGaps());

    biostring = "--B--ob--";
    QVERIFY(biostring.hasGaps());

    biostring = ".";
    QVERIFY(biostring.hasGaps());

    biostring = " .. - ADF .. ---";
    QVERIFY(biostring.hasGaps());

    biostring = "    asdf e fjsd f  dkfei";
    QVERIFY(!biostring.hasGaps());
}

void TestBioString::hasNonGaps()
{
    BioString biostring;

    QVERIFY(!biostring.hasNonGaps());

    biostring = "A";
    QVERIFY(biostring.hasNonGaps());

    biostring = "-";
    QVERIFY(!biostring.hasNonGaps());

    biostring = ".";
    QVERIFY(!biostring.hasNonGaps());

    biostring = "A-";
    QVERIFY(biostring.hasNonGaps());

    biostring = "ABCDEF..--";
    QVERIFY(biostring.hasNonGaps());
}

void TestBioString::headGaps()
{
    BioString biostring = "-C---D-";
    QVERIFY(biostring.headGaps() == 1);

    biostring = "CDE";
    QVERIFY(biostring.headGaps() == 0);

    biostring = "------EF";
    QVERIFY(biostring.headGaps() == 6);

    biostring = "EF---";
    QVERIFY(biostring.headGaps() == 0);

    biostring = "E--F";
    QVERIFY(biostring.headGaps() == 0);
}

void TestBioString::insert()
{
    QByteArray byteArray = "ABCDEF";
    int n = byteArray.length();

    BioString biostring = byteArray;

    // Test: 1..n+1
    for (int i=1; i<= n+1; ++i)
    {
        biostring = byteArray;
        QByteArray target = byteArray.left(i-1) + "XYZ" + byteArray.right(n - (i-1));
        QCOMPARE(biostring.insert(i, "XYZ").constData(), target.constData());
    }

    // ----------------------------------
    // Spot checks
    biostring = "ABCDEF"; QCOMPARE(biostring.insert(3, "").constData(), "ABCDEF");
    biostring = "ABCDEF"; QCOMPARE(biostring.insert(6, "---").constData(), "ABCDE---F");
    biostring = "ABCDEF"; QCOMPARE(biostring.insert(7, "..").constData(), "ABCDEF..");
}

void TestBioString::insertGaps()
{
    QByteArray byteArray = "ABCDEF";
    int n = byteArray.length();

    BioString biostring = byteArray;

    // Test: 0 for position and/or n
    QCOMPARE(biostring.insertGaps(3, 0, '-').constData(), byteArray.constData());

    // Test: insert gaps at 1..n+1
    for (int i=1; i<=n+1; ++i)
    {
        QByteArray expect = byteArray.left(i-1) + "--" + byteArray.right(n-i+1);

        biostring = byteArray;
        QCOMPARE(biostring.insertGaps(i, 2, '-').constData(), expect.constData());
    }

    // ----------------------------------
    // Spot checks
    biostring = byteArray;
    QCOMPARE(biostring.insertGaps(1, 2, '#').constData(), "##ABCDEF");

    // Test: different gap character
    biostring = byteArray;
    QCOMPARE(biostring.insertGaps(3, 0, 'x').constData(), byteArray.constData());

    // Test: insert gaps at 1..n+1
    for (int i=1; i<=n+1; ++i)
    {
        QByteArray expect = byteArray.left(i-1) + "@@" + byteArray.right(n-i+1);

        biostring = byteArray;
        QCOMPARE(biostring.insertGaps(i, 2, '@').constData(), expect.constData());
    }
}

void TestBioString::isEquivalentTo()
{
    BioString biostring = "ABC---def";

    QVERIFY(biostring.isEquivalentTo(biostring));
    QVERIFY(biostring.isEquivalentTo("ABC---def"));
    QVERIFY(biostring.isEquivalentTo("ABC...def"));
    QVERIFY(biostring.isEquivalentTo("ABC-.-def"));
    QVERIFY(!(biostring.isEquivalentTo("BC---def")));

    biostring = "ABC...def";

    QVERIFY(biostring.isEquivalentTo(biostring));
    QVERIFY(biostring.isEquivalentTo("ABC---def"));
    QVERIFY(biostring.isEquivalentTo("ABC...def"));
    QVERIFY(biostring.isEquivalentTo("ABC-.-def"));
    QVERIFY(!(biostring.isEquivalentTo("BC---def")));

    // Same everything but different grammar
    BioString biostring2(biostring);
    biostring2.setGrammar(eAminoGrammar);
    QVERIFY(biostring.grammar() != eAminoGrammar);
    QVERIFY(!biostring.isEquivalentTo(biostring2));
}

void TestBioString::isExactMatch()
{
    BioString bioString = "ABCDEF";

    for (int i=1; i<= 6; ++i)
        QCOMPARE(bioString.isExactMatch(i, ""), false);

    QCOMPARE(bioString.isExactMatch(1, "A"), true);
    QCOMPARE(bioString.isExactMatch(1, "AB"), true);
    QCOMPARE(bioString.isExactMatch(1, "ABC"), true);
    QCOMPARE(bioString.isExactMatch(1, "ABCD"), true);
    QCOMPARE(bioString.isExactMatch(1, "ABCDE"), true);
    QCOMPARE(bioString.isExactMatch(1, "ABCDEF"), true);

    QCOMPARE(bioString.isExactMatch(1, "AABCDEF"), false);
    QCOMPARE(bioString.isExactMatch(1, "ABCDEFF"), false);
    QCOMPARE(bioString.isExactMatch(1, "-ABCDE"), false);
    QCOMPARE(bioString.isExactMatch(1, "A-BCDE"), false);
    QCOMPARE(bioString.isExactMatch(1, "ABCDEF-"), false);
    QCOMPARE(bioString.isExactMatch(1, "BCDE"), false);
    QCOMPARE(bioString.isExactMatch(1, "CDE"), false);
    QCOMPARE(bioString.isExactMatch(1, "DE"), false);
    QCOMPARE(bioString.isExactMatch(1, "E"), false);
    QCOMPARE(bioString.isExactMatch(1, "--.."), false);

    QCOMPARE(bioString.isExactMatch(2, "A"), false);
    QCOMPARE(bioString.isExactMatch(2, "AB"), false);
    QCOMPARE(bioString.isExactMatch(2, "ABC"), false);
    QCOMPARE(bioString.isExactMatch(2, "ABCD"), false);
    QCOMPARE(bioString.isExactMatch(2, "ABCDE"), false);
    QCOMPARE(bioString.isExactMatch(2, "ABCDEF"), false);

    QCOMPARE(bioString.isExactMatch(2, "B"), true);
    QCOMPARE(bioString.isExactMatch(2, "BC"), true);
    QCOMPARE(bioString.isExactMatch(2, "BCD"), true);
    QCOMPARE(bioString.isExactMatch(2, "BCDE"), true);
    QCOMPARE(bioString.isExactMatch(2, "BCDEF"), true);

    QCOMPARE(bioString.isExactMatch(2, "BBCDEF"), false);
    QCOMPARE(bioString.isExactMatch(2, "BCDEFF"), false);
    QCOMPARE(bioString.isExactMatch(2, "-BCDE"), false);
    QCOMPARE(bioString.isExactMatch(2, "-BCDE"), false);
    QCOMPARE(bioString.isExactMatch(2, "BCDEF-"), false);
    QCOMPARE(bioString.isExactMatch(2, "CDE"), false);
    QCOMPARE(bioString.isExactMatch(2, "DE"), false);
    QCOMPARE(bioString.isExactMatch(2, "E"), false);
    QCOMPARE(bioString.isExactMatch(2, "--.."), false);

    QCOMPARE(bioString.isExactMatch(6, "F"), true);
    QCOMPARE(bioString.isExactMatch(6, "F-"), false);
    QCOMPARE(bioString.isExactMatch(6, "-F"), false);
    QCOMPARE(bioString.isExactMatch(6, "-"), false);
    QCOMPARE(bioString.isExactMatch(6, "."), false);
}

void TestBioString::isPalindrome_data()
{
    QTest::addColumn<BioString>("bioString");
    QTest::addColumn<bool>("isPalindrome");

    QTest::newRow("default bioString") << BioString() << false;

    QTest::newRow("empty bioString, unknown grammar") << BioString(eUnknownGrammar) << false;
    QTest::newRow("empty bioString, Amino grammar") << BioString(eAminoGrammar) << false;
    QTest::newRow("empty bioString, DnaGrammar") << BioString(eDnaGrammar) << false;
    QTest::newRow("empty bioString, RnaGrammar") << BioString(eRnaGrammar) << false;

    QTest::newRow("A, unknown grammar") << BioString("A", eUnknownGrammar) << false;
    QTest::newRow("A, Amino grammar") << BioString("A", eAminoGrammar) << false;
    QTest::newRow("A, DnaGrammar") << BioString("A", eDnaGrammar) << false;
    QTest::newRow("A, RnaGrammar") << BioString("A", eRnaGrammar) << false;

    QTest::newRow("AA, unknown grammar") << BioString("AA", eUnknownGrammar) << false;
    QTest::newRow("AA, Amino grammar") << BioString("AA", eAminoGrammar) << false;
    QTest::newRow("AA, DnaGrammar") << BioString("AA", eDnaGrammar) << false;
    QTest::newRow("AA, RnaGrammar") << BioString("AA", eRnaGrammar) << false;

    QTest::newRow("AT, unknown grammar") << BioString("AT", eUnknownGrammar) << false;
    QTest::newRow("AT, Amino grammar") << BioString("AT", eAminoGrammar) << false;
    QTest::newRow("AT, DnaGrammar") << BioString("AT", eDnaGrammar) << true;
    QTest::newRow("AT, RnaGrammar") << BioString("AT", eRnaGrammar) << false;

    QTest::newRow("ATA, unknown grammar") << BioString("ATA", eUnknownGrammar) << false;
    QTest::newRow("ATA, Amino grammar") << BioString("ATA", eAminoGrammar) << false;
    QTest::newRow("ATA, DnaGrammar") << BioString("ATA", eDnaGrammar) << false;
    QTest::newRow("ATA, RnaGrammar") << BioString("ATA", eRnaGrammar) << false;

    QTest::newRow("ATC, unknown grammar") << BioString("ATC", eUnknownGrammar) << false;
    QTest::newRow("ATC, Amino grammar") << BioString("ATC", eAminoGrammar) << false;
    QTest::newRow("ATC, DnaGrammar") << BioString("ATC", eDnaGrammar) << false;
    QTest::newRow("ATC, RnaGrammar") << BioString("ATC", eRnaGrammar) << false;

    QTest::newRow("ATT, unknown grammar") << BioString("ATT", eUnknownGrammar) << false;
    QTest::newRow("ATT, Amino grammar") << BioString("ATT", eAminoGrammar) << false;
    QTest::newRow("ATT, DnaGrammar") << BioString("ATT", eDnaGrammar) << false;
    QTest::newRow("ATT, RnaGrammar") << BioString("ATT", eRnaGrammar) << false;

    QTest::newRow("AATT, unknown grammar") << BioString("AATT", eUnknownGrammar) << false;
    QTest::newRow("AATT, Amino grammar") << BioString("AATT", eAminoGrammar) << false;
    QTest::newRow("AATT, DnaGrammar") << BioString("AATT", eDnaGrammar) << true;
    QTest::newRow("AATT, RnaGrammar") << BioString("AATT", eRnaGrammar) << false;

    QTest::newRow("ACTGT, unknown grammar") << BioString("ACTGT", eUnknownGrammar) << false;
    QTest::newRow("ACTGT, Amino grammar") << BioString("ACTGT", eAminoGrammar) << false;
    QTest::newRow("ACTGT, DnaGrammar") << BioString("ACTGT", eDnaGrammar) << false;
    QTest::newRow("ACTGT, RnaGrammar") << BioString("ACTGT", eRnaGrammar) << false;

    QTest::newRow("ACTAGT, unknown grammar") << BioString("ACTAGT", eUnknownGrammar) << false;
    QTest::newRow("ACTAGT, Amino grammar") << BioString("ACTAGT", eAminoGrammar) << false;
    QTest::newRow("ACTAGT, DnaGrammar") << BioString("ACTAGT", eDnaGrammar) << true;
    QTest::newRow("ACTAGT, RnaGrammar") << BioString("ACTAGT", eRnaGrammar) << false;

    QTest::newRow("ACT--AGT, DnaGrammar") << BioString("ACT--AGT", eDnaGrammar) << false;
    QTest::newRow("--, DnaGrammar") << BioString("--", eDnaGrammar) << false;
}

void TestBioString::isPalindrome()
{
    QFETCH(BioString, bioString);
    QFETCH(bool, isPalindrome);

    QCOMPARE(bioString.isPalindrome(), isPalindrome);
}

void TestBioString::isValidPosition()
{
    BioString biostring = "ABCDEF";

    for (int i=-5; i< biostring.length() + 5; ++i)
        QCOMPARE(biostring.isValidPosition(i), i >= 1 && i <= biostring.length());
}

void TestBioString::isValidRange()
{
    BioString bioString = "ABCD";

    int length = bioString.length();

    for (int i=-length; i<= length; ++i)
        for (int j=-length; j<= length; ++j)
            QCOMPARE(bioString.isValidRange(ClosedIntRange(i, j)), i > 0 && j > 0 && i < length + 1 && j < length + 1 && i <= j);
}

void TestBioString::leftSlidablePositions()
{
    //             1234567890
    QByteArray str = "AB--C-D--E";
    BioString biostring = str;

    // Test: each individual character
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(1, 1)), 0);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(2, 2)), 0);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(3, 3)), 2);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(4, 4)), 3);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(5, 5)), 2);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(6, 6)), 5);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(7, 7)), 1);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(8, 8)), 7);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(9, 9)), 8);
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(10, 10)), 2);

    // Test: segment containing bounded on both sides by non-gap character
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(5, 7)), 2);

    // Test: segment containing gap on left bound, but also containing non-gap characters
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(4, 7)), 1);

    // Test: segment containing internal non-gap character and gaps on both terminii
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(4, 6)), 1);

    // Test: segment consisting solely of gaps
    QCOMPARE(biostring.leftSlidablePositions(ClosedIntRange(3, 4)), 2);
}

void TestBioString::prepend_biostring()
{
    BioString biostring = "ABC";

    biostring.prepend(BioString("DEF"));
    QCOMPARE(biostring.constData(), "DEFABC");

    biostring.prepend("G").prepend("H");
    QCOMPARE(biostring.constData(), "HGDEFABC");
}

void TestBioString::prepend_cstr()
{
    BioString biostring;

    biostring.prepend("ABC");
    QCOMPARE(biostring.constData(), "ABC");

    biostring.prepend("DEF").prepend("GHI");
    QCOMPARE(biostring.constData(), "GHIDEFABC");
}

void TestBioString::prepend_char()
{
    BioString biostring;

    biostring.prepend('a');
    QCOMPARE(biostring.constData(), "a");

    biostring.prepend('b').prepend('C');
    QCOMPARE(biostring.constData(), "Cba");
}

void TestBioString::remove_range()
{
    QByteArray str = "ABCDEF";
    int n = str.length();

    BioString biostring = str;

    // Test: 1..n, 1
    for (int i=1; i<=n; ++i)
    {
        biostring = str;
        QByteArray expect = str.left(i-1) + str.right(n-i);
        QCOMPARE(biostring.remove(ClosedIntRange(i, i)).constData(), expect.constData());
    }

    // ----------------------------------
    // Spot checks
    biostring = "ABCDEF"; QCOMPARE(biostring.remove(ClosedIntRange(1, 1)).constData(), "BCDEF");
    biostring = "ABCDEF"; QCOMPARE(biostring.remove(ClosedIntRange(3, 4)).constData(), "ABEF");
    biostring = "ABCDEF"; QCOMPARE(biostring.remove(ClosedIntRange(2, 5)).constData(), "AF");
}

void TestBioString::removeGaps()
{
    BioString biostring = "ABC";
    biostring.removeGaps();
    QVERIFY(biostring == "ABC");

    biostring = "-A-B.C-";
    biostring.removeGaps();
    QVERIFY(biostring == "ABC");

    biostring = "---";
    biostring.removeGaps();
    QVERIFY(biostring.isEmpty());
}

void TestBioString::removeGaps_poslen()
{
    //                123456789012345
    QByteArray str = "--A-B-C---DEF--";
    int n = str.length();

    BioString biostring = str;

    // Test: removing n < 1 gaps does nothing
    QCOMPARE(biostring.removeGaps(1, 0).constData(), str.constData());
    QCOMPARE(biostring.removeGaps(9, 0).constData(), str.constData());
    QCOMPARE(biostring.removeGaps(14, 0).constData(), str.constData());

    // Test: Removing single gap at each specific position that is a gap character
    for (int i=1; i<= n; ++i)
    {
        if (!::isGapCharacter(str.at(i-1)))
            continue;

        biostring.removeGaps(i, 1);
        QByteArray tmp = str;
        tmp.remove(i-1, 1);
        QCOMPARE(biostring.constData(), tmp.constData());

        biostring = str;
    }

    // Test: exact multi-gap removal
    biostring = str;
    QCOMPARE(biostring.removeGaps(1, 2), BioString("A-B-C---DEF--"));

    /*
      NOTE: 23 June 2011
            removeGaps does essentially the same thing as BioString::remove. In debug mode, however, it additionally
            asserts that the number of gaps present is less than or equal to the requested range.

    // Test: attempting to delete more gaps than is contiguous at position
    biostring = str;
    QCOMPARE(biostring.removeGaps(1, 23), BioString("A-B-C---DEF--"));

    // Test: gap removal in middle of string
    biostring = str;
    QCOMPARE(biostring.removeGaps(9, 2), BioString("--A-B-C-DEF--"));
    biostring = str;
    QCOMPARE(biostring.removeGaps(9, 3), BioString("--A-B-C-DEF--"));
    biostring = str;
    QCOMPARE(biostring.removeGaps(8, 3), BioString("--A-B-CDEF--"));
    */
}

void TestBioString::replace()
{
    QByteArray str = "ABCDEF";
    BioString biostring = str;
    int n = biostring.length();

    // Test: empty biostring
    QCOMPARE(biostring.replace(2, 2, "").constData(), "ADEF");

    biostring = str;
    QCOMPARE(biostring.replace(3, 0, "XYZ").constData(), "ABXYZCDEF");

    // Test: 1 and 1
    biostring = str;
    QCOMPARE(biostring.replace(1, 1, "XYZ").constData(), "XYZBCDEF");

    // Test: replace nothing but at end of sequence (insert)
    biostring = str;
    QCOMPARE(biostring.replace(7, 0, "XYZ").constData(), "ABCDEFXYZ");

    // Test: replace nothing but at beginning of sequence (insert)
    biostring = str;
    QCOMPARE(biostring.replace(1, 0, "XYZ").constData(), "XYZABCDEF");

    // Test: replacing the same letter
    biostring = str;
    for (int i=1; i<=n; ++i)
    {
        QByteArray letter;
        letter.resize(1);
        letter[0] = str.at(i-1);
        QCOMPARE(biostring.replace(i, 1, letter).constData(), str.constData());
    }

    // Test: exact length valid XYZ replace
    for (int i=1; i<= n-3; ++i)
    {
        QByteArray expect = str.left(i-1) + "XYZ" + str.right(n-i-3+1);

        biostring = str;
        QCOMPARE(biostring.replace(i, 3, "XYZ").constData(), expect.constData());
    }

    biostring = str;
    QCOMPARE(biostring.replace(1, 4, "XYZ").constData(), "XYZEF");

    // Test: replacement right end
    biostring = str;
    QCOMPARE(biostring.replace(5, 2, "XYZ").constData(), "ABCDXYZ");
}

void TestBioString::replace_range()
{
    QByteArray str = "ABCDEF";
    BioString biostring = str;
    int n = biostring.length();

    // Test: empty biostring
    QCOMPARE(biostring.replace(ClosedIntRange(2, 3), "").constData(), "ADEF");

    // Test: replacing the same letter
    biostring = str;
    for (int i=1; i<=n; ++i)
    {
        QByteArray letter;
        letter.resize(1);
        letter[0] = str.at(i-1);
        QCOMPARE(biostring.replace(ClosedIntRange(i, i), letter).constData(), str.constData());
    }

    // Test: exact length valid XYZ replace
    for (int i=1; i<= n-3; ++i)
    {
        QByteArray expect = str.left(i-1) + "XYZ" + str.right(n-i-3+1);

        biostring = str;
        QCOMPARE(biostring.replace(ClosedIntRange(i, i+2), "XYZ").constData(), expect.constData());
    }

    biostring = str;
    QCOMPARE(biostring.replace(ClosedIntRange(1, 4), "XYZ").constData(), "XYZEF");

    // Test: replacement right end
    biostring = str;
    QCOMPARE(biostring.replace(ClosedIntRange(5, 6), "XYZ").constData(), "ABCDXYZ");
}

void TestBioString::reverse()
{
    BioString biostring = "A";
    biostring.reverse();
    QCOMPARE(biostring.constData(), "A");

    biostring = "AB";
    biostring.reverse();
    QCOMPARE(biostring.constData(), "BA");

    biostring = "ABC";
    biostring.reverse();
    QCOMPARE(biostring.constData(), "CBA");

    biostring = "123abcdef456";
    biostring.reverse();
    QCOMPARE(biostring.constData(), "654fedcba321");

    biostring.reverse();
    QCOMPARE(biostring.constData(), "123abcdef456");
}

void TestBioString::reverseComplement()
{
    BioString biostring;
    BioString dna = biostring.reverseComplement();

    QVERIFY(dna.isEmpty());
    QCOMPARE(dna.grammar(), eDnaGrammar);

    biostring = allowableChars;
    dna = biostring.reverseComplement();
    QCOMPARE(dna.grammar(), eDnaGrammar);
    biostring.reverse();
    biostring.tr("ABCDGHKMTVabcdghkmtv",
                 "TVGHCDMKABtvghcdmkab");
    QCOMPARE(biostring.constData(), dna.constData());

    biostring = "ABCDGHKMTVabcdghkmtv";
    dna = biostring.reverseComplement();
    QCOMPARE(dna.constData(), "bakmdchgvtBAKMDCHGVT");
}

void TestBioString::rightSlidablePositions()
{
    //             1234567890
    QByteArray str = "AB--C-D--E";
    BioString biostring = str;

    // Test: each individual character
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(1, 1)), 0);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(2, 2)), 2);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(3, 3)), 7);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(4, 4)), 6);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(5, 5)), 1);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(6, 6)), 4);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(7, 7)), 2);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(8, 8)), 2);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(9, 9)), 1);
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(10, 10)), 0);

    // Test: segment containing bounded on both sides by non-gap character
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(5, 7)), 2);

    // Test: segment containing gap on right bound, but also containing non-gap characters
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(5, 8)), 1);

    // Test: segment containing internal non-gap character and gaps on both terminii
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(6, 8)), 1);

    // Test: segment consisting solely of gaps
    QCOMPARE(biostring.rightSlidablePositions(ClosedIntRange(3, 4)), 6);
}

void TestBioString::slide()
{
    //                1234567890123
    QByteArray str = "ABC--D-EF--GH";
    BioString biostring = str;

    // Test: delta of zero and valid positions does nothing
    QVERIFY(biostring.slide(ClosedIntRange(6, 9), 0) == 0 && biostring == str);

    // Test: slide entire sequence does not change anything
    QVERIFY(biostring.slide(ClosedIntRange(1, 13), 3) == 0 && biostring == str);
    QVERIFY(biostring.slide(ClosedIntRange(1, 13), -3) == 0 && biostring == str);

    // Test: positive direction
                     QVERIFY(biostring.slide(ClosedIntRange(1, 3), 1) == 1 && biostring == "-ABC-D-EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(1, 3), 2) == 2 && biostring == "--ABCD-EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(1, 3), 3) == 2 && biostring == "--ABCD-EF--GH");

    // Test: postive direction with region containing gaps
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(6, 9), 1) == 1 && biostring == "ABC---D-EF-GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(6, 9), 2) == 2 && biostring == "ABC----D-EFGH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(6, 9), 3) == 2 && biostring == "ABC----D-EFGH");

    // Test: region with terminal gaps
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(5, 10), 1) == 1 && biostring == "ABC---D-EF-GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(5, 10), 2) == 1 && biostring == "ABC---D-EF-GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(5, 10), -1) == 1 && biostring == "ABC-D-EF---GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(5, 10), -2) == 1 && biostring == "ABC-D-EF---GH");

    // Test: move single residue
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(6, 6), -3) == 2 && biostring == "ABCD---EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(6, 6), -1) == 1 && biostring == "ABC-D--EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(6, 6), 1) == 1 && biostring == "ABC---DEF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(6, 6), 2) == 1 && biostring == "ABC---DEF--GH");

    // Test: sliding pure gapped region
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), -1) == 1 && biostring == "AB--CD-EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), -2) == 2 && biostring == "A--BCD-EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), -3) == 3 && biostring == "--ABCD-EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), -4) == 3 && biostring == "--ABCD-EF--GH");

    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 1) == 1 && biostring == "ABCD---EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 2) == 2 && biostring == "ABCD---EF--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 3) == 3 && biostring == "ABCD-E--F--GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 4) == 4 && biostring == "ABCD-EF----GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 5) == 5 && biostring == "ABCD-EF----GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 6) == 6 && biostring == "ABCD-EF----GH");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 7) == 7 && biostring == "ABCD-EF--G--H");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 8) == 8 && biostring == "ABCD-EF--GH--");
    biostring = str; QVERIFY(biostring.slide(ClosedIntRange(4, 5), 9) == 8 && biostring == "ABCD-EF--GH--");
}

void TestBioString::tailgaps()
{
    BioString biostring = "-C---D-";

    biostring = "-C---D-";
    QCOMPARE(biostring.tailGaps(), 1);

    biostring = "CDE";
    QCOMPARE(biostring.tailGaps(), 0);

    biostring = "EF------";
    QCOMPARE(biostring.tailGaps(), 6);

    biostring = "---EF";
    QCOMPARE(biostring.tailGaps(), 0);

    biostring = "E--F";
    QCOMPARE(biostring.tailGaps(), 0);

    biostring = "";
    QCOMPARE(biostring.tailGaps(), 0);
}

void TestBioString::tr()
{
    BioString biostring("ABCDEFabcdef..--aA");

    // Test: single letter replacement
    biostring.tr("A", "Z");
    QCOMPARE(biostring.constData(), "ZBCDEFabcdef..--aZ");

    // Test: multiple letter replacement
    biostring.tr("Za.-Xb", "aA-.sb");
    QCOMPARE(biostring.constData(), "aBCDEFAbcdef--..Aa");

    // Test: the duplicate query rule
    biostring.tr("BB", "12");
    QCOMPARE(biostring.constData(), "a1CDEFAbcdef--..Aa");

    // Test: individual character version
    biostring.tr('.', '@');
    QCOMPARE(biostring.constData(), "a1CDEFAbcdef--@@Aa");
}

void TestBioString::transcribe()
{
    BioString dna(" AtTcCgG..-t", eDnaGrammar);

    BioString rna = dna.transcribe();
    QCOMPARE(rna.grammar(), eRnaGrammar);
    QCOMPARE(rna.constData(), "AuUcCgG..-u");
}

void TestBioString::translateGaps()
{
    BioString x("ABC");

    QVERIFY(x.translateGaps('-') == "ABC");

    x = "";
    QVERIFY(x.translateGaps(' ') == "");

    x = "-AB-C-.";
    QVERIFY(x.translateGaps('@') == "@AB@C@@");
}

void TestBioString::ungapped()
{
    BioString biostring("ABC---DEF", eDnaGrammar);
    BioString result = biostring.ungapped();
    QVERIFY(result == "ABCDEF");
    QCOMPARE(result.grammar(), eDnaGrammar);

    biostring = "ABC...DEF";
    QVERIFY(biostring.ungapped() == "ABCDEF");

    biostring = "---ABCDEF";
    QVERIFY(biostring.ungapped() == "ABCDEF");

    biostring = "ABCDEF---";
    QVERIFY(biostring.ungapped() == "ABCDEF");

    biostring = "...ABCDEF";
    QVERIFY(biostring.ungapped() == "ABCDEF");

    biostring = "ABCDEF...";
    QVERIFY(biostring.ungapped() == "ABCDEF");

    // Test removal of dash gaps
    biostring = "--AB--CDEF--";
    biostring = biostring.ungapped();
    QVERIFY2(biostring == "ABCDEF", "Failed to remove gaps (dashes)");

    // Test removal of period gaps
    biostring = "..AB..CDEF..";
    biostring = biostring.ungapped();
    QVERIFY2(biostring == "ABCDEF", "Failed to remove gaps (dots)");

    // Test removal of mixed dashes and dots
    biostring = "..AB--CD..EF--";
    biostring = biostring.ungapped();
    QVERIFY2(biostring == "ABCDEF", "Failed to remove gaps (dots and dashes)");
}

void TestBioString::ungappedLength()
{
    BioString biostring = "ABC";
    QCOMPARE(biostring.ungappedLength(), 3);

    biostring = "---";
    QCOMPARE(biostring.ungappedLength(), 0);

    biostring = "ABC---DEF...GHI";
    QCOMPARE(biostring.ungappedLength(), 9);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Benchmarks
int randInt(int low, int high)
{
    // Random number between low and high
    return qrand() % ((high + 1) - low) + low;
}

void TestBioString::benchSlide()
{
    QTime time = QTime::currentTime();
    qsrand((uint)time.msec());

    QByteArray str = "ABC--D-EF--GH---DF-DFE--DFSDF-----------EFASFEF-DF------";
    int l = str.length();

    QBENCHMARK
    {
        BioString biostring = str;
        int start = randInt(1, l);
        int length = randInt(1, l + 1 - start);
        ClosedIntRange range(start, start + length - 1);
        int leading = range.begin_ - 1;
        int tail = range.length() - range.end_;
        biostring.slide(range, -leading);
        biostring.slide(range, tail);
    }
}

void TestBioString::benchSlideViaSwap()
{
    QByteArray str = "ABC--D-EF--GH---DF-DFE--DFSDF-----------EFASFEF-DF------";
    int l = str.length();

    QBENCHMARK
    {
        BioString biostring = str;
        int start = randInt(1, l);
        int length = randInt(1, l + 1 - start);
        ClosedIntRange range(start, start + length - 1);
        int leading = range.begin_ - 1;
        int tail = range.length() - range.end_;
        biostring.slideViaSwap(range, -leading);
        biostring.slideViaSwap(range, tail);
    }
}

QTEST_APPLESS_MAIN(TestBioString)
#include "TestBioString.moc"
