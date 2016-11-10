/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "BioString.h"

#include <QScopedPointer>

class TestBioString : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorEmpty();
    void constructorQString();
    void constructorChar();
    void constructorCopy();
    void clone();
    void create();

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    void assignQString();
    void assignChar();
    void assignBioString();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void alphabet();
    void length();
    void sequence();
    void ungapped();
    void substituteGapsWith();
    void masked();
    void benchRegexIsValid();           // Benchmark valid character checking using regex
    void benchAsciiIsValid();           // Benchmark valid character checking using ascii scan
    void isValid();                     // Winner: ascii scan
    void mask();
    void hasGaps();
    void hasCharacters();
    void reduced();
    void positiveIndex();
    void negativeIndex();
    void nonGapCharsBetween_data();
    void nonGapCharsBetween();

    void prepend();
    void append();
    void operator_add();
    void operator_addeq();
    void operator_eqeq();
    void operator_ne();
    void operator_bracket();
    void segment();
    void mid();
    void indexOf();

    void insert();
    void remove();
    void insertGaps();
    void removeGaps();

    void replace();
    void replaceSegment();

    void gapsLeftOf();
    void gapsRightOf();
    void leftSlidablePositions();
    void rightSlidablePositions();
    void slideSegment();

    // ------------------------------------------------------------------------------------------------
    // Static methods
    // Compare the removal of whitespace via a regular expression vs the straightforward removal technique
    void benchRegexNormalize();         // Benchmark whitespace removal via regex
    void benchQStringNormalize();       // Benchmark whitespace removal via QString removal
    void benchQCharNormalize();         // Benchmark whitespace removal via manipulationg at the QChar level
    void benchQByteArrayNormalize();    // Benchmark whitespace removal via manipulationg at the QChar level
    void benchQByteArrayAtNormalize();  // Same as QByteArrayNormalize, but using the at function instead of accessing via char *
    void benchCharNormalize();          // Benchamrk whitespace removal with pure char array
    void staticNormalize();             // Winner: QString removal
    void staticIsGap();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestBioString::constructorEmpty()
{
    BioString b;
    QVERIFY2(b.sequence() == "", "Sequence is empty");
}

void TestBioString::constructorQString()
{
    BioString b(QString("DEF"));
    QVERIFY2(b.sequence() == "DEF", "Unsuccessful sequence initialization");

    BioString b2(QString(""));
    QVERIFY2(b2.sequence() == "", "Unsuccessful sequence initialization with empty string");

    BioString b3(QString("ABC DEF -- ..\nGHI"));
    QVERIFY(b3.sequence() == "ABCDEF--..GHI");
}

void TestBioString::constructorChar()
{
    BioString b("DEF");
    QVERIFY2(b.sequence() == "DEF", "Unsuccessful sequence initialization");

    BioString b2("");
    QVERIFY2(b2.sequence() == "", "Unsuccessful sequence initialization with empty string");

    BioString b3("ABC DEF -- ..\nGHI");
    QVERIFY(b3.sequence() == "ABCDEF--..GHI");
}

void TestBioString::constructorCopy()
{
    QString str = "1234567891";
    BioString b1(str);
    BioString b2(b1);

    QVERIFY2(b2.sequence() == str, "Copied sequence is incorrect");
}

void TestBioString::clone()
{
    QString str = "123456789";
    BioString b1(str);

    QScopedPointer<BioString> bs_ptr(b1.clone());
    QVERIFY(bs_ptr->sequence() == str);

    // Change the sequence of one and see if it changes the other
    QString str2 = "ABCDEF";
    b1 = str2;
    QVERIFY(b1.sequence() == str2);
    QVERIFY(bs_ptr->sequence() == str);
    QVERIFY(bs_ptr->sequence() != b1.sequence());
}

void TestBioString::create()
{
    QString str = "123456789";
    BioString b1(str);

    QScopedPointer<BioString> bioStringPtr(b1.create());
    QVERIFY(bioStringPtr->sequence().isEmpty());

    // Change the sequence of one and see if it changes the other
    QString str2 = "ABCDEF";
    b1 = str2;
    QVERIFY(b1.sequence() == str2);
    QVERIFY(bioStringPtr->sequence().isEmpty());
    QVERIFY(bioStringPtr->sequence() != b1.sequence());
}

// ------------------------------------------------------------------------------------------------
// Assignment operators
void TestBioString::assignQString()
{
    QString test1 = "ABCDEF12345";
    BioString b1;
    b1 = test1;
    QVERIFY2(b1.sequence() == test1, "Failed to assign simple QString");

    QString test2 = "  abc  \n1234 dEf";
    b1 = test2;
    QVERIFY2(b1.sequence() == "ABC1234DEF", "Failed to assign and normalize simple QString");
}

void TestBioString::assignChar()
{
    BioString biostring = "ABCDEF12345";
    QVERIFY2(biostring.sequence() == "ABCDEF12345", "Failed to assign simple character array");

    char test2[] = "  abc  \n1234 dEf";
    biostring = test2;
    QVERIFY2(biostring.sequence() == "ABC1234DEF", "Failed to assign and normalize simple character array");
}

void TestBioString::assignBioString()
{
    BioString b1("GENOMICS");
    BioString b2("COMPUTATIONAL");

    b1 = b2;
    QVERIFY2(b1.sequence() == "COMPUTATIONAL", "Unsuccessful assignment");
}


// ------------------------------------------------------------------------------------------------
// Public methods
void TestBioString::alphabet()
{
    BioString biostring;
    QCOMPARE(biostring.alphabet(), eUnknownAlphabet);
}

void TestBioString::length()
{
    BioString biostring = "ABCDEF";
    QVERIFY(biostring.length() == 6);

    biostring = "ABC ... --- def";
    QVERIFY(biostring.length() == 12);

    biostring = "123ABC...---DEFxxx";
    biostring = biostring.masked();
    QVERIFY(biostring.length() == 18);
}

void TestBioString::sequence()
{
    BioString biostring = "ABCDEF";
    QVERIFY(biostring.sequence() == "ABCDEF");

    biostring = "";
    QVERIFY(biostring.sequence() == "");
}

void TestBioString::ungapped()
{
    BioString biostring = "ABC---DEF";
    QVERIFY(biostring.ungapped() == "ABCDEF");

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
    QVERIFY2(biostring.sequence() == "ABCDEF", "Failed to remove gaps (dashes)");

    // Test removal of period gaps
    biostring = "..AB..CDEF..";
    biostring = biostring.ungapped();
    QVERIFY2(biostring.sequence() == "ABCDEF", "Failed to remove gaps (dots)");

    // Test removal of mixed dashes and dots
    biostring = "..AB--CD..EF--";
    biostring = biostring.ungapped();
    QVERIFY2(biostring.sequence() == "ABCDEF", "Failed to remove gaps (dots and dashes)");
}

void TestBioString::substituteGapsWith()
{
    BioString x("ABC");

    QCOMPARE(x.substituteGapsWith('-'), QString("ABC"));

    x = "";
    QCOMPARE(x.substituteGapsWith(' '), QString());

    x = "-AB-C-";
    QCOMPARE(x.substituteGapsWith('@'), QString("@AB@C@"));
    QCOMPARE(x.sequence(), QString("-AB-C-"));
}

void TestBioString::masked()
{
    BioString biostring("ABC123DEF");
    QVERIFY(biostring.masked() == "ABCXXXDEF");
    QVERIFY(biostring.masked('@') == "ABC@@@DEF");
    QVERIFY(biostring.sequence() == "ABC123DEF");

    biostring = "ABCDEFGHIJKLMNOPQRSTUVWXYZ*-.1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    QVERIFY(biostring.masked() == "ABCDEFGHIJKLMNOPQRSTUVWXYZ*-.XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX");
}

void TestBioString::benchRegexIsValid()
{
    QRegExp validCharacters_("^[ABCDEFGHIJKLMNOPQRSTUVWXYZ*-.]*$");
    QString test_string = "MDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFSVL\nQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYICQA\nLRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELGHL\nTTLTDVVKGADSLSAILPGDIAED\nITAVLCFVIEADQITFETVEVSPKISTPPVLKLAA\nEQAPTGRVEREKTTRSSESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNHGD\nLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSSTEL\r\nDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTDDG\nAGLNRERILAKAASQGLTVSENMSDDEVAMLIFAPGFSTAEQVTDVSGRGVGMDVVKRNI\nQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREADL\rHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLIGQ\n1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";

    QBENCHMARK {
        validCharacters_.exactMatch(test_string);
    }
}

void TestBioString::benchAsciiIsValid()
{
    QByteArray string = "MDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFSVL\nQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYICQA\nLRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELGHL\nTTLTDVVKGADSLSAILPGDIAED\nITAVLCFVIEADQITFETVEVSPKISTPPVLKLAA\nEQAPTGRVEREKTTRSSESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNHGD\nLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSSTEL\r\nDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTDDG\nAGLNRERILAKAASQGLTVSENMSDDEVAMLIFAPGFSTAEQVTDVSGRGVGMDVVKRNI\nQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREADL\rHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLIGQ\n1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";

    QBENCHMARK {
        for (int i=0, z= string.size(); i< z; ++i)
        {
            char x = string.at(i);
            if ((x >= 65 && x <= 90) // A-Z
                || x == 42 // *
                || x == 45 // -
                || x == 46)
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }
}

void TestBioString::isValid()
{
    BioString bs("ABCDEF");
    QVERIFY2(bs.isValid(), "Calling isValid directly on BioString failed with valid sequence");

    bs = "ABC 1234";
    QVERIFY2(bs.isValid() == false, "After assignment of an invalid QString sequence, isValid direct call failed");

    // Test string with one of every valid character
    BioString biostring("abcdefghijklmnopqrstuvwxyz*-.");
    QVERIFY2(biostring.isValid() == true, "Valid characters not classified as valid");

    // Test with a slew of invalid characters
    QString invalid_characters = "1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z=invalid_characters.length(); i<z; ++i)
    {
        biostring = invalid_characters[i];
        QVERIFY2(biostring.isValid() == false, QString("Invalid character %1 classified as valid").arg(invalid_characters[i]).toAscii());
    }

    // Test validation with sequence that requires normalization
    biostring = "abc def ghi\njkl\nmno\npqr\rstu\tvwxyz*-.";
    QVERIFY2(biostring.isValid() == true, "Valid sequence with interleaved whitespace not classified as valid");
}

void TestBioString::mask()
{
    BioString biostring = "ABC$%^DEF";
    QVERIFY2(biostring.sequence() == "ABC$%^DEF", QString("Sequence: %1").arg(biostring.sequence()).toAscii());

    biostring = biostring.masked('X');
    QVERIFY2(biostring.sequence() == "ABCXXXDEF", QString("Result: %1").arg(biostring.sequence()).toAscii());

    biostring = "1234567890";
    biostring = biostring.masked('z');
    QVERIFY2(biostring.sequence() == "ZZZZZZZZZZ", QString("Result: %1").arg(biostring.sequence()).toAscii());
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

void TestBioString::hasCharacters()
{
    BioString biostring = "---...";

    QVERIFY(!biostring.hasCharacters());

    biostring = "ACD";
    QVERIFY(biostring.hasCharacters());

    biostring = "--A--";
    QVERIFY(biostring.hasCharacters());

    biostring = ".";
    QVERIFY(!biostring.hasCharacters());

    biostring = " .. - ADF .. ---";
    QVERIFY(biostring.hasCharacters());

    biostring = "    ---  - . . -- - -- ..";
    QVERIFY(!biostring.hasCharacters());
}

void TestBioString::reduced()
{
    BioString biostring = "ABC---def 1234";
    QVERIFY(biostring.reduced() == "ABCDEFXXXX");

    QVERIFY(biostring.reduced('T') == "ABCDEFTTTT");
}

void TestBioString::positiveIndex()
{
    BioString biostring = "ABCDEF";
    int n = biostring.length();

    // Test: 0 should return 0
    QVERIFY(biostring.positiveIndex(0) == 0);

    // Test: all positive numbers from 1..n+2
    for (int i=1; i<= n+2; ++i)
        QVERIFY(biostring.positiveIndex(i) == i);

    // Test: all negative numbers from -1..-n-2
    for (int i=-1; i>= -n-2; --i)
        QVERIFY(biostring.positiveIndex(i) == n - qAbs(i) + 1);
}

void TestBioString::negativeIndex()
{
    BioString biostring = "ABCDEF";
    int n = biostring.length();

    // Test: 0 should return 0
    QVERIFY(biostring.negativeIndex(0) == 0);

    // Test: all positive numbers from 1..n+2
    for (int i=1; i<= n+2; ++i)
        QVERIFY(biostring.negativeIndex(i) == -(n - qAbs(i) + 1));

    // Test: all negative numbers from -1..-n-2
    for (int i=-1; i>= -n-2; --i)
        QVERIFY(biostring.negativeIndex(i) == i);
}

void TestBioString::nonGapCharsBetween_data()
{
    QTest::addColumn<QString>("sequence");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("stop");
    QTest::addColumn<int>("nNonGaps");

    QTest::newRow("no gaps") << "ABCDEF" << 1 << 6 << 6;
    QTest::newRow("no gaps part 2") << "ABCDEF" << 2 << 5 << 4;

    QString sequence = "ABC---DEF";
    for (int i=1; i<= sequence.length(); ++i)
        for (int j=i; j<= sequence.length(); ++j)
            QTest::newRow(QString("%1 :: %2-%3").arg(sequence).arg(i).arg(j).toAscii())
                    << sequence
                    << i
                    << j
                    << (j-i+1) - sequence.mid(i-1, j-i+1).count('-');
}

void TestBioString::nonGapCharsBetween()
{
    QFETCH(QString, sequence);
    QFETCH(int, start);
    QFETCH(int, stop);
    QFETCH(int, nNonGaps);

    QCOMPARE(BioString(sequence).nonGapCharsBetween(start, stop), nNonGaps);
}

void TestBioString::prepend()
{
    BioString biostring = "ABC...def";

    biostring.prepend("GHI");
    QVERIFY(biostring.sequence() == "GHIABC...DEF");

    biostring.prepend("vx").prepend("yz");
    QVERIFY(biostring.sequence() == "YZVXGHIABC...DEF");
}

void TestBioString::append()
{
    BioString biostring = "ABC...def";

    biostring.append("GHI");
    QVERIFY(biostring.sequence() == "ABC...DEFGHI");

    biostring.append("vx").append("yz");
    QVERIFY(biostring.sequence() == "ABC...DEFGHIVXYZ");
}

void TestBioString::operator_add()
{
    BioString left = "ABC";
    BioString right = "DEF";
    BioString both = left + right;

    QVERIFY(left.sequence() == "ABC");
    QVERIFY(right.sequence() == "DEF");
    QVERIFY(both.sequence() == "ABCDEF");

    // Test: Check adding blank right
    both = left + "";
    QVERIFY(both.sequence() == "ABC");

    // Test: Check adding blank left
    both = "" + right;
    QVERIFY(both.sequence() == "DEF");
}

void TestBioString::operator_addeq()
{
    BioString biostring = "ABC...def";

    biostring += "GHI";
    QVERIFY(biostring.sequence() == "ABC...DEFGHI");

    biostring += "vx";
    biostring += "yz";
    QVERIFY(biostring.sequence() == "ABC...DEFGHIVXYZ");
}

void TestBioString::operator_eqeq()
{
    BioString biostring = "ABC---def";

    QVERIFY(biostring == biostring);
    QVERIFY(biostring == "ABC---DEF");
    QVERIFY(biostring == "ABC...DEF");
    QVERIFY(biostring == "ABC-.-def");
    QVERIFY(!(biostring == "BC---def"));

    // Check that the original sequence has not been modified
    QVERIFY(biostring.sequence() == "ABC---DEF");

    biostring = "ABC...def";

    QVERIFY(biostring == biostring);
    QVERIFY(biostring == "ABC---DEF");
    QVERIFY(biostring == "ABC...DEF");
    QVERIFY(biostring == "ABC-.-def");
    QVERIFY(!(biostring == "BC---def"));

    // Check that the original sequence has not been modified
    QVERIFY(biostring.sequence() == "ABC...DEF");
}

void TestBioString::operator_ne()
{
    BioString biostring = "ABC---DEF";

    QVERIFY(!(biostring != biostring));
    QVERIFY(biostring != "");
    QVERIFY(biostring != "BC---DEFG");
    QVERIFY(!(biostring != "ABC.-.DEF"));

    // Check that the original sequence has not been modified
    QVERIFY(biostring.sequence() == "ABC---DEF");
}

void TestBioString::operator_bracket()
{
    QString str = "ABC.-.DEF";
    BioString biostring = str;

    int n = biostring.length();
    for (int i=1; i<= n; ++i)
    {
        QVERIFY(biostring[i] == str.at(i-1));
        QVERIFY(biostring[-i] == str.at(n-i));
    }
}

/**
  *  1  2  3  4  5  6  7  8
  *  A  B  C  D  E  F  G  H
  * -8 -7 -6 -5 -4 -3 -2 -1
  *
  * Position: 6 = F
  * Position: -6 = C
  * Position: start of 0 -> 1 = A
  * Position: end of 0 -> 8 = H
  *
  * >>> Normal ranges:
  * Range: 0..0 -> 1..8 = ABCDEFGH
  * Range: 0..3 -> 1..3 = ABC
  * Range: 5..5 -> E
  * Range: -6..-4 -> 3..5 = CDE
  * Range: 1..-2 -> 1..7 = ABCDEFG
  * Range: -7..4 -> 2..4 = BCD
  *
  * >>> Clamped ranges:
  * Range: -10..5 -> 1..5 = ABCDE
  * Range: 6..30 -> 6..8 = FGH
  * Range: -10..-5 -> 1..4 = ABCD
  *
  * >>> Ranges which return NULL:
  * Range: 5..3 -> NULL
  * Range: 9..15 -> NULL
  * Range: -9..-9 -> -1..-1 = NULL
  */
void TestBioString::segment()
{
    QString seq = "ABCDEFGH";
    BioString biostring = seq;

    int n = seq.length();

    // ----------------------------------
    // 0-param tests
    QVERIFY(biostring.segment() == seq);

    // ----------------------------------
    // 1-param tests

    // Test: 0
    QVERIFY(biostring.segment(0) == seq);

    // Test: 1..n
    for (int i=1; i<= n; ++i)
        QVERIFY(biostring.segment(i) == seq.right(n-i+1));

    // Test: n+1 and n+2
    QVERIFY(biostring.segment(n+1).isEmpty());
    QVERIFY(biostring.segment(n+2).isEmpty());

    // Test: -1..-n
    for (int i=-1; i >= -8; --i)
        QVERIFY(biostring.segment(i) == seq.right(qAbs(i)));

    // Test: -n-1 and -n-2
    QVERIFY(biostring.segment(-n - 1) == seq);
    QVERIFY(biostring.segment(-n - 2) == seq);

    // ----------------------------------
    // 2-param tests

    // >>> Suite: -n -2 .. n + 2, 0
    // Test: 0, 0
    QVERIFY(biostring.segment(0, 0) == seq);

    // Test: 1..n, 0
    for (int i=1; i<= n; ++i)
        QVERIFY(biostring.segment(i, 0) == seq.right(n-i+1));

    // Test: n+1, 0 and n+2, 0
    QVERIFY(biostring.segment(n+1, 0).isEmpty());
    QVERIFY(biostring.segment(n+2, 0).isEmpty());

    // Test: -1..-n, 0
    for (int i=-1; i >= -8; --i)
        QVERIFY(biostring.segment(i, 0) == seq.right(qAbs(i)));

    // Test: -n-1, 0 and -n-2, 0
    QVERIFY(biostring.segment(-n - 1, 0) == seq);
    QVERIFY(biostring.segment(-n - 2, 0) == seq);



    // >>> Suite: 0, -n -2 .. n + 2
    // Test: 0, 1..n
    for (int i=1; i<= n; ++i)
        QVERIFY(biostring.segment(0, i) == seq.left(i));

    // Test: 0, n+1 and 0, n+2
    QVERIFY(biostring.segment(0, n+1) == seq);
    QVERIFY(biostring.segment(0, n+2) == seq);

    // Test: 0, -1..-n
    for (int i=-1; i >= -8; --i)
        QVERIFY(biostring.segment(0, i) == seq.left(n - qAbs(i) + 1));
        // 0 .. -1 -> 1 .. 8 = ABCDEFGH
        // 0 .. -2 -> 1 .. 7 = ABCDEFG
        // 0 .. -3 -> 1 .. 6 = ABCDEF

    // Test: 0, -n-1 and 0, -n-2
    QVERIFY(biostring.segment(0, -n - 1).isEmpty());
    QVERIFY(biostring.segment(0, -n - 2).isEmpty());

    // >>> Suite: -n - 2 .. n + 2, -n - 2 .. n + 2
    for (int start=-n-2; start <= n+2; ++start)
    {
        // Already tested the zero case
        if (start == 0)
            continue;

        for (int stop=-n-2; stop <=n+2; ++stop)
        {
            // Already tested the zero case
            if (stop == 0)
                continue;

            int abs_start = (start < 0) ? n - qAbs(start) + 1 : start;  // 1-based
            int abs_stop = (stop < 0) ? n - qAbs(stop) + 1 : stop;      // 1-based

            if (abs_start <= n &&
                abs_stop >= 1 &&
                abs_start <= abs_stop)
            {
                QVERIFY(biostring.segment(start, stop) == seq.mid(abs_start-1, abs_stop - abs_start + 1));
            }
            else
                QVERIFY(biostring.segment(start, stop).isEmpty());
        }
    }


    // --------------------------------------
    // Spot checks

    // Normal ranges
    QVERIFY(biostring.segment(0, 3) == "ABC");
    QVERIFY(biostring.segment(5, 5) == "E");
    QVERIFY(biostring.segment(-6, -4) == "CDE");
    QVERIFY(biostring.segment(1, -2) == "ABCDEFG");
    QVERIFY(biostring.segment(-7, 4) == "BCD");

    // Clamped ranges
    QVERIFY(biostring.segment(-10, 5) == "ABCDE");
    QVERIFY(biostring.segment(6, 30) == "FGH");
    QVERIFY(biostring.segment(-10, -5) == "ABCD");
    QVERIFY(biostring.segment(-30, 30) == "ABCDEFGH");

    // Ranges which return null
    QVERIFY(biostring.segment(5, 3).isEmpty());
    QVERIFY(biostring.segment(9, 15).isEmpty());
    QVERIFY(biostring.segment(-9, -9).isEmpty());
    QVERIFY(biostring.segment(0, -9).isEmpty());
    QVERIFY(biostring.segment(1, -9).isEmpty());

    QVERIFY(biostring.segment(0, -8) == "A");
    QVERIFY(biostring.segment(1, -8) == "A");
    QVERIFY(biostring.segment(2, -8).isEmpty());
    QVERIFY(biostring.segment(-9, 1) == "A");
}

void TestBioString::mid()
{
    QString seq = "ABCDEFGH";
    int n = seq.length();

    BioString biostring = seq;

    // ------------------------------------
    // >>> Suite: 1-param version

    // Test: 0
    QVERIFY(biostring.mid(0) == "A");

    // Test: 1..n
    for (int i=1; i<= n; ++i)
        QVERIFY2(biostring.mid(i) == seq.mid(i-1, 1), QString("biostring.mid(%1) gave %2").arg(i).arg(biostring.mid(i)).toAscii());

    // Test: n+1, n+2
    QVERIFY(biostring.mid(n+1).isEmpty());
    QVERIFY(biostring.mid(n+2).isEmpty());

    // Test: -1..-n
    for (int i=-1; i>= -n; --i)
        QVERIFY(biostring.mid(i) == seq.mid(n - qAbs(i) + 1 - 1, 1));

    // Test -n-1, -n-2
    QVERIFY(biostring.mid(-n-1).isEmpty());
    QVERIFY(biostring.mid(-n-2).isEmpty());

    // ------------------------------------
    // >>> Suite: 2-param version

    // Test: -n-2..n+2, -5..0
    for (int i=0; i >= -5; --i)
        for (int j=-n-2; j<= n+2; ++j)
            QVERIFY(biostring.mid(j, i).isEmpty());

    // Test: -n-2..1, 3
    QVERIFY(biostring.mid(-n-2, 3) == "A");
    QVERIFY(biostring.mid(-n-1, 3) == "AB");
    QVERIFY(biostring.mid(-n, 3) == "ABC");

    // Test: 2..n-2, 3
    for (int i=2, z=n-2; i<=z; ++i)
        QVERIFY(biostring.mid(i, 3) == seq.mid(i-1, 3));

    // Test: n-1, 3 and n, 3
    QVERIFY(biostring.mid(n-1, 3) == seq.right(2));
    QVERIFY(biostring.mid(n, 3) == seq.right(1));

    // --------------------------------------
    // Spot checks
    QVERIFY(biostring.mid(3) == "C");
    QVERIFY(biostring.mid(6, 3) == "FGH");
    QVERIFY(biostring.mid(-7) == "B");
    QVERIFY(biostring.mid(-8, 5) == "ABCDE");
    QVERIFY(biostring.mid(-9, 2) == "A");
    QVERIFY(biostring.mid(-10, 3) == "A");

    QVERIFY(biostring.mid(-12, 3).isEmpty());
    QVERIFY(biostring.mid(9).isEmpty());
}

void TestBioString::indexOf()
{
    //                     12345678901
    BioString biostring = "ABC-D.E-ABC";

    // Test: empty biostring returns 0
    QVERIFY(biostring.indexOf(BioString()) == 0);

    // Test: Either gap character
    QVERIFY(biostring.indexOf(BioString("-")) == 4);
    QVERIFY(biostring.indexOf(BioString(".")) == 4);

    // Test: exact match
    QVERIFY(biostring.indexOf(BioString("-D.E")) == 4);

    // Test: partial matching gap characters, but equivalent gap positions
    QVERIFY(biostring.indexOf(BioString(".D.E")) == 4);

    // Test: Another exact match
    QVERIFY(biostring.indexOf(BioString("E-A")) == 7);

    // Test: completely inverted gaps, but equivalent gap positions
    QVERIFY(biostring.indexOf(BioString(".D-E.A")) == 4);

    // Test: partial match
    QVERIFY(biostring.indexOf(BioString("E.E")) == 0);

    // Test: complete mismatch
    QVERIFY(biostring.indexOf(BioString("xyz")) == 0);

    // >>> Suite: 2-param version
    // Test: Either gap character
    QVERIFY2(biostring.indexOf(BioString("-"), 5) == 6, QString("biostring.indexOf(BioString(\"-\"), 5) gave %1").arg(biostring.indexOf(BioString("-"), 5)).toAscii());
    QVERIFY(biostring.indexOf(BioString("."), 5) == 6);

    QVERIFY(biostring.indexOf(BioString("ABC"), 1) == 1);
    QVERIFY(biostring.indexOf(BioString("ABC"), 2) == 9);
    QVERIFY(biostring.indexOf(BioString("BC"), 4) == 10);
    QVERIFY(biostring.indexOf(BioString("BC"), 10) == 10);
    QVERIFY(biostring.indexOf(BioString("BC"), 11) == 0);

    // Test: negative from
    QVERIFY(biostring.indexOf(BioString("B"), -30) == 2);
    QVERIFY(biostring.indexOf(BioString("ABC"), -4) == 9);

    // Test: from extending beyond biostring length
    QVERIFY(biostring.indexOf(BioString("C"), biostring.length()+1) == 0);
}

void TestBioString::insert()
{
    QString str = "ABCDEF";
    int n = str.length();

    BioString biostring = str;

    // Test: insert at zero
    QVERIFY(biostring.insert(0, "XYZ") == "ABCDEF");

    // Test: 1..n+1
    for (int i=1; i<= n+1; ++i)
    {
        biostring = str;
        QString target = str.left(i-1) + "XYZ" + str.right(n - (i-1));
        QVERIFY(biostring.insert(i, "XYZ") == target);
        QVERIFY(biostring == target);
    }

    // Test: n+2
    biostring = str;
    QVERIFY(biostring.insert(n+2, "XYZ") == str);

    // Test: -1..-n
    for (int i=-1; i>= -n; --i)
    {
        biostring = str;
        QString target = str.left(n - qAbs(i)) + "XYZ" + str.right(qAbs(i));
        QVERIFY2(biostring.insert(i, "XYZ") == target, QString("biostring.insert(%1, \"XYZ\") = %2; expected %3").arg(i).arg(biostring.sequence()).arg(target).toAscii());
        QVERIFY(biostring == target);
    }

    // Test: -n-1
    biostring = str;
    QVERIFY(biostring.insert(-n-1, "XYZ") == str);

    // Test: insert empty string
    QVERIFY(biostring.insert(3, "") == str);

    // ----------------------------------
    // Spot checks
    biostring = "ABCDEF"; QVERIFY(biostring.insert(-1, "---") == "ABCDE---F");
    biostring = "ABCDEF"; QVERIFY(biostring.insert(6, "---") == "ABCDE---F");
    biostring = "ABCDEF"; QVERIFY(biostring.insert(7, "..")  == "ABCDEF..");
    biostring = "ABCDEF"; QVERIFY(biostring.insert(-3, "ST") == "ABCSTDEF");
    biostring = "ABCDEF"; QVERIFY(biostring.insert(-6, "---") == "---ABCDEF");

    // Invalid positions
    biostring = "ABCDEF"; QVERIFY(biostring.insert(0, "---") == "ABCDEF");
    biostring = "ABCDEF"; QVERIFY(biostring.insert(-7, "---")  == "ABCDEF");
    biostring = "ABCDEF"; QVERIFY(biostring.insert(-32, "---") == "ABCDEF");
    biostring = "ABCDEF"; QVERIFY(biostring.insert(8, "---")   == "ABCDEF");
}

void TestBioString::remove()
{
    QString str = "ABCDEF";
    int n = str.length();

    BioString biostring = str;

    // Test: position 0 and < -n regardless of n should return the same string
    QVERIFY(biostring.remove(0, 3) == str);
    QVERIFY(biostring.remove(0, -3) == str);
    for (int i=-n-1; i>= -n-5; --i)
        QVERIFY(biostring.remove(i, 3) == str);

    // Test: removal of valid positions, but values of n [0..-n] should return the same string
    for (int i=0; i >= -n; --i)
        QVERIFY(biostring.remove(3, i) == str);

    // Test: 1..n, 1
    for (int i=1; i<=n; ++i)
    {
        biostring = str;
        QVERIFY(biostring.remove(i, 1) == str.left(i-1) + str.right(n-i));
    }

    // Test: -1..-n, 1
    for (int i=-1; i>= -n; --i)
    {
        biostring = str;
        QVERIFY(biostring.remove(i, 1) == str.left(n-qAbs(i)) + str.right(qAbs(i)-1));
    }

    // ----------------------------------
    // Spot checks
    biostring = "ABCDEF"; QVERIFY(biostring.remove(1, 1) == "BCDEF");
    biostring = "ABCDEF"; QVERIFY(biostring.remove(3, 1) == "ABDEF");
    biostring = "ABCDEF"; QVERIFY(biostring.remove(3, 3) == "ABF");
    biostring = "ABCDEF"; QVERIFY(biostring.remove(3, 20) == "AB");
    biostring = "ABCDEF"; QVERIFY(biostring.remove(-1, 1) == "ABCDE");
    biostring = "ABCDEF"; QVERIFY(biostring.remove(-6, 2) == "CDEF");
    biostring = "ABCDEF"; QVERIFY(biostring.remove(-7, 3) == "ABCDEF");
}

void TestBioString::insertGaps()
{
    QString str = "ABCDEF";
    int n = str.length();

    BioString biostring = str;

    // >>> Suite: 2-param version, assuming that default gap character is '-'
    // Test: 0 for position and/or n
    QVERIFY(biostring.insertGaps(0, 3) == str);
    QVERIFY(biostring.insertGaps(3, 0) == str);
    QVERIFY(biostring.insertGaps(0, 0) == str);

    // Test: -n
    QVERIFY(biostring.insertGaps(3, -3) == str);

    // Test: valid -position, -n
    QVERIFY(biostring.insertGaps(-3, -3) == str);

    // Test: invalid position and n
    QVERIFY(biostring.insertGaps(-7, 0) == str);

    // Test: insert gaps at 1..n+1
    for (int i=1; i<=n+1; ++i)
    {
        biostring = str;
        QVERIFY(biostring.insertGaps(i, 2) == str.left(i-1) + "--" + str.right(n-i+1));
    }

    // Test: n+2
    biostring = str;
    QVERIFY(biostring.insertGaps(n+2, 3) == str);

    // Test: -1 .. -n
    for (int i=-1; i >= -n; --i)
    {
        biostring = str;
        QVERIFY(biostring.insertGaps(i, 2) == str.left(n-qAbs(i)) + "--" + str.right(qAbs(i)));
    }

    // Test: n-1
    biostring = str;
    QVERIFY(biostring.insertGaps(-n-1, 2) == str);

    // ----------------------------------
    // Spot checks
    biostring = str; QVERIFY(biostring.insertGaps(1, 2) == "--ABCDEF");
    biostring = str; QVERIFY(biostring.insertGaps(-1, 2) == "ABCDE--F");

    // >>> Suite: 3-param version
    // Test: 0 for position and/or n
    biostring = str;
    QVERIFY2(biostring.insertGaps(0, 3, '.') == str, QString("biostring.insertGaps(0, 3, '.') returned: %1").arg(biostring.insertGaps(0, 3, '.').sequence()).toAscii());
    QVERIFY(biostring.insertGaps(3, 0, 'x') == str);
    QVERIFY(biostring.insertGaps(0, 0, '@') == str);

    // Test: -n
    QVERIFY(biostring.insertGaps(3, -3, '!') == str);

    // Test: valid -position, -n
    QVERIFY(biostring.insertGaps(-3, -3, 'o') == str);

    // Test: invalid position and n
    QVERIFY(biostring.insertGaps(-7, 0, '.') == str);

    // Test: insert gaps at 1..n+1
    for (int i=1; i<=n+1; ++i)
    {
        biostring = str;
        QVERIFY(biostring.insertGaps(i, 2, '.') == str.left(i-1) + ".." + str.right(n-i+1));
    }

    // Test: n+2
    biostring = str;
    QVERIFY(biostring.insertGaps(n+2, 3, 'x') == str);

    // Test: -1 .. -n
    for (int i=-1; i >= -n; --i)
    {
        biostring = str;
        QVERIFY(biostring.insertGaps(i, 2, '-') == str.left(n-qAbs(i)) + "--" + str.right(qAbs(i)));
    }

    // Test: n-1
    biostring = str;
    QVERIFY(biostring.insertGaps(-n-1, 2, '{') == str);

    // ----------------------------------
    // Spot checks
    biostring = str; QVERIFY(biostring.insertGaps(1, 2, '~') == "~~ABCDEF");
    biostring = str; QVERIFY(biostring.insertGaps(-1, 2, '~') == "ABCDE~~F");
}

void TestBioString::removeGaps()
{
    //             123456789012345
    QString str = ".-A-B-C-.-DEF-.";
    int n = str.length();

    BioString biostring = str;

    // Test: removing gaps at position 0 does nothing
    QVERIFY(biostring.removeGaps(0, 1) == str);

    // Test: position outside of bounds does nothing
    QVERIFY(biostring.removeGaps(-n-1, 1) == str);
    QVERIFY(biostring.removeGaps(-n-1, 3) == str);
    QVERIFY(biostring.removeGaps(n+1, 1) == str);
    QVERIFY(biostring.removeGaps(n+1, 5) == str);

    // Test: removing n < 1 gaps does nothing
    QVERIFY(biostring.removeGaps(1, 0) == str);
    QVERIFY(biostring.removeGaps(9, 0) == str);
    QVERIFY(biostring.removeGaps(14, 0) == str);

    // Test: Removing single gap at each specific position that is a gap character
    for (int i=-n; i< n; ++i)
    {
        if (i == 0)
            continue;

        biostring.removeGaps(i, 1);
        int abs_i = (i > 0) ? i-1 : n + i;
        if (str.at(abs_i) == '.' || str.at(abs_i) == '-')
        {
            QString tmp = str;
            tmp.remove(abs_i, 1);
            QVERIFY(biostring == BioString(tmp));
        }
        else
            QVERIFY(biostring == str);

        biostring = str;
    }

    // Test: exact multi-gap removal
    QVERIFY(biostring.removeGaps(1, 2) == BioString("A-B-C---DEF--"));

    // Test: attempting to delete more gaps than is contiguous at position
    biostring = str;
    QVERIFY(biostring.removeGaps(1, 23) == BioString("A-B-C---DEF--"));

    // Test: gap removal in middle of string
    biostring = str;
    QVERIFY(biostring.removeGaps(9, 2) == BioString("--A-B-C-DEF--"));
    biostring = str;
    QVERIFY(biostring.removeGaps(9, 3) == BioString("--A-B-C-DEF--"));
    biostring = str;
    QVERIFY(biostring.removeGaps(8, 3) == BioString("--A-B-CDEF--"));

    // Test: gap removal at end of string
    biostring = str;
    QVERIFY(biostring.removeGaps(-2, 2) == BioString("--A-B-C---DEF"));
    biostring = str;
    QVERIFY(biostring.removeGaps(-2, 7) == BioString("--A-B-C---DEF"));
}

void TestBioString::replace()
{
    QString str = "ABCDEF";
    BioString biostring = str;
    int n = biostring.length();

    // Test: empty biostring
    QVERIFY(biostring.replace(2, 2, "") == "ADEF");

    // Test: invalid n
    biostring = str;
    QVERIFY(biostring.replace(3, 0, "XYZ") == str);
    QVERIFY(biostring.replace(3, -3, "XYZ") == str);

    // Test: far away left
    QVERIFY(biostring.replace(-n-6, 3, "XYZ") == str);

    // Test: both 0's
    biostring = str;
    QVERIFY(biostring.replace(0, 0, "XYZ") == str);

    // Test: 0 and 1
    biostring = str;
    QVERIFY(biostring.replace(0, 1, "XYZ") == str);

    // Test: 1 and 1
    biostring = str;
    QVERIFY(biostring.replace(1, 1, "XYZ") == "XYZBCDEF");

    // Test: replacing the same letter
    biostring = str;
    for (int i=1; i<=n; ++i)
        QVERIFY(biostring.replace(i, 1, QString(str.at(i-1))) == str);

    // Test: exact length valid XYZ replace
    for (int i=1; i<= n-3; ++i)
    {
        biostring = str;
        QVERIFY(biostring.replace(i, 3, "XYZ") == str.left(i-1) + "XYZ" + str.right(n-i-3+1));
    }

    // Same as above with negative indices
    for (int i=-3; i>= -n; --i)
    {
        biostring = str;
        QVERIFY(biostring.replace(i, 3, "XYZ") == str.left(n - qAbs(i)) + "XYZ" + str.right(qAbs(i) - 3));
    }

    biostring = str;
    QVERIFY(biostring.replace(1, 4, "XYZ") == "XYZEF");

    // Test: position before left end
    biostring = str;
    QVERIFY2(biostring.replace(-n-1, 3, "XYZ") == str, QString("biostring.replace(-n-1, 3, \"XYZ\") returned %1").arg(biostring.sequence()).toAscii());

    // Test: replacement extends beyond right end
    biostring = str;
    QVERIFY(biostring.replace(5, 4, "XYZ") == "ABCDXYZ");
}

void TestBioString::replaceSegment()
{
    QString str = "ABCDEF";
    BioString biostring = str;
    int n = biostring.length();

    // Test: empty biostring
    QVERIFY(biostring.replaceSegment(2, 4, "") == "AEF");

    // Test: both 0's
    biostring = str;
    QVERIFY(biostring.replaceSegment(0, 0, "XYZ") == "XYZ");

    // Test: 0 and 1
    biostring = str;
    QVERIFY(biostring.replaceSegment(0, 1, "XYZ") == "XYZBCDEF");

    // Test: 1 and 1
    biostring = str;
    QVERIFY(biostring.replaceSegment(1, 1, "XYZ") == "XYZBCDEF");

    // Test: end < start for 2..n+2
    biostring = str;
    for (int i=2; i<= n+2; ++i)
        QVERIFY(biostring.replaceSegment(i, 1, "XYZ") == str);

    // Test: end < start for -n+1..-n-2
    for (int i=-n+1; i<= -n-2; ++i)
        QVERIFY(biostring.replaceSegment(i, 1, "XYZ") == str);

    // Test: replacing the same letter
    for (int i=1; i<=n; ++i)
        QVERIFY(biostring.replaceSegment(i, i, QString(str.at(i-1))) == str);

    // Test: exact length valid XYZ replace
    for (int i=1; i<= n-3; ++i)
    {
        biostring = str;
        QVERIFY(biostring.replaceSegment(i, i+2, "XYZ") == str.left(i-1) + "XYZ" + str.right(n-i-3+1));
    }

    // Same as above with negative indices
    for (int i=-3; i>= -n; --i)
    {
        biostring = str;
        QVERIFY(biostring.replaceSegment(i, i+2, "XYZ") == str.left(n - qAbs(i)) + "XYZ" + str.right(qAbs(i) - 3));
    }

    // Test: clamp left end
    biostring = str;
    QVERIFY(biostring.replaceSegment(0, 4, "XYZ") == "XYZEF");

    // Test: clamp right end
    biostring = str;
    QVERIFY(biostring.replaceSegment(5, n+4, "XYZ") == "ABCDXYZ");

    // Test: clamp both ends
    biostring = str;
    QVERIFY(biostring.replaceSegment(-n-3, n+3, "XYZ") == "XYZ");
}

void TestBioString::gapsLeftOf()
{
    //                     1234567890123
    BioString biostring = "--AB----CD---";
    int n = biostring.length();

    // Test: position 0 returns zero
    QVERIFY(biostring.gapsLeftOf(0) == 0);

    // Test: outside positive boundary
    QVERIFY(biostring.gapsLeftOf(n+1) == 0);

    // Test: negative boundary and beyond
    QVERIFY(biostring.gapsLeftOf(-n) == 0);
    QVERIFY(biostring.gapsLeftOf(-n-1) == 0);

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

    // Test: full negative range
    QVERIFY(biostring.gapsLeftOf(-1) == 2);
    QVERIFY(biostring.gapsLeftOf(-2) == 1);
    QVERIFY(biostring.gapsLeftOf(-3) == 0);
    QVERIFY(biostring.gapsLeftOf(-4) == 0);
    QVERIFY(biostring.gapsLeftOf(-5) == 4);
    QVERIFY(biostring.gapsLeftOf(-6) == 3);
    QVERIFY(biostring.gapsLeftOf(-7) == 2);
    QVERIFY(biostring.gapsLeftOf(-8) == 1);
    QVERIFY(biostring.gapsLeftOf(-9) == 0);
    QVERIFY(biostring.gapsLeftOf(-10) == 0);
    QVERIFY(biostring.gapsLeftOf(-11) == 2);
    QVERIFY(biostring.gapsLeftOf(-12) == 1);
    QVERIFY(biostring.gapsLeftOf(-13) == 0);
}

void TestBioString::gapsRightOf()
{
    //                     1234567890123
    BioString biostring = "--AB----CD---";
    int n = biostring.length();

    // Test: position 0 returns zero
    QVERIFY(biostring.gapsRightOf(0) == 0);

    // Test: positive boundary and beyond
    QVERIFY(biostring.gapsRightOf(n) == 0);
    QVERIFY(biostring.gapsRightOf(n+1) == 0);

    // Test: oustide negative boundary
    QVERIFY(biostring.gapsRightOf(-n-1) == 0);

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

    // Test: full negative range
    QVERIFY(biostring.gapsRightOf(-1) == 0);
    QVERIFY(biostring.gapsRightOf(-2) == 1);
    QVERIFY(biostring.gapsRightOf(-3) == 2);
    QVERIFY(biostring.gapsRightOf(-4) == 3);
    QVERIFY(biostring.gapsRightOf(-5) == 0);
    QVERIFY(biostring.gapsRightOf(-6) == 0);
    QVERIFY(biostring.gapsRightOf(-7) == 1);
    QVERIFY(biostring.gapsRightOf(-8) == 2);
    QVERIFY(biostring.gapsRightOf(-9) == 3);
    QVERIFY(biostring.gapsRightOf(-10) == 4);
    QVERIFY(biostring.gapsRightOf(-11) == 0);
    QVERIFY(biostring.gapsRightOf(-12) == 0);
    QVERIFY(biostring.gapsRightOf(-13) == 1);
}

void TestBioString::leftSlidablePositions()
{
    //             1234567890
    QString str = "AB--C-D--E";
    BioString biostring = str;

    // Test: each individual character
    QVERIFY(biostring.leftSlidablePositions(1, 1) == 0);
    QVERIFY(biostring.leftSlidablePositions(2, 2) == 0);
    QVERIFY(biostring.leftSlidablePositions(3, 3) == 2);
    QVERIFY(biostring.leftSlidablePositions(4, 4) == 3);
    QVERIFY(biostring.leftSlidablePositions(5, 5) == 2);
    QVERIFY(biostring.leftSlidablePositions(6, 6) == 5);
    QVERIFY(biostring.leftSlidablePositions(7, 7) == 1);
    QVERIFY(biostring.leftSlidablePositions(8, 8) == 7);
    QVERIFY(biostring.leftSlidablePositions(9, 9) == 8);
    QVERIFY(biostring.leftSlidablePositions(10, 10) == 2);

    // Test: segment containing bounded on both sides by non-gap character
    QVERIFY(biostring.leftSlidablePositions(5, 7) == 2);

    // Test: segment containing gap on left bound, but also containing non-gap characters
    QVERIFY(biostring.leftSlidablePositions(4, 7) == 1);

    // Test: segment containing internal non-gap character and gaps on both terminii
    QVERIFY(biostring.leftSlidablePositions(4, 6) == 1);

    // Test: segment consisting solely of gaps
    QVERIFY(biostring.leftSlidablePositions(3, 4) == 2);
}

void TestBioString::rightSlidablePositions()
{
    //             1234567890
    QString str = "AB--C-D--E";
    BioString biostring = str;

    // Test: each individual character
    QVERIFY(biostring.rightSlidablePositions(1, 1) == 0);
    QVERIFY(biostring.rightSlidablePositions(2, 2) == 2);
    QVERIFY(biostring.rightSlidablePositions(3, 3) == 7);
    QVERIFY(biostring.rightSlidablePositions(4, 4) == 6);
    QVERIFY(biostring.rightSlidablePositions(5, 5) == 1);
    QVERIFY(biostring.rightSlidablePositions(6, 6) == 4);
    QVERIFY(biostring.rightSlidablePositions(7, 7) == 2);
    QVERIFY(biostring.rightSlidablePositions(8, 8) == 2);
    QVERIFY(biostring.rightSlidablePositions(9, 9) == 1);
    QVERIFY(biostring.rightSlidablePositions(10, 10) == 0);

    // Test: segment containing bounded on both sides by non-gap character
    QVERIFY(biostring.rightSlidablePositions(5, 7) == 2);

    // Test: segment containing gap on right bound, but also containing non-gap characters
    QVERIFY(biostring.rightSlidablePositions(5, 8) == 1);

    // Test: segment containing internal non-gap character and gaps on both terminii
    QVERIFY(biostring.rightSlidablePositions(6, 8) == 1);

    // Test: segment consisting solely of gaps
    QVERIFY(biostring.rightSlidablePositions(3, 4) == 6);
}

void TestBioString::slideSegment()
{
    //             1234567890123
    QString str = "ABC--D-EF--GH";
    int n = str.length();
    BioString biostring = str;

    // Test: delta of zero and valid positions does nothing
    QVERIFY(biostring.slideSegment(6, 9, 0) == 0 && biostring.sequence() == str);

    // Test: Invalid position
//    QVERIFY(biostring.slideSegment(6, 14, -3) == 0 && biostring.sequence() == str);
//    QVERIFY(biostring.slideSegment(-14, 3, 3) == 0 && biostring.sequence() == str);
//    QVERIFY(biostring.slideSegment(-14, 14, 3) == 0 && biostring.sequence() == str);

    // Test: start_pos > stop_pos
//    QVERIFY(biostring.slideSegment(9, 6, 3) == 0 && biostring.sequence() == str);

    // Test: slide entire sequence does not change anything
    QVERIFY(biostring.slideSegment(1, -1, 3) == 0 && biostring.sequence() == str);
    QVERIFY(biostring.slideSegment(1, -1, -3) == 0 && biostring.sequence() == str);

    // Test: positive direction
                     QVERIFY(biostring.slideSegment(1, 3, 1) == 1 && biostring.sequence() == "-ABC-D-EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(1, 3, 2) == 2 && biostring.sequence() == "--ABCD-EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(1, 3, 3) == 2 && biostring.sequence() == "--ABCD-EF--GH");

    // Same thing with negative indices
    biostring = str; QVERIFY(biostring.slideSegment(-n, -11, 1) == 1 && biostring.sequence() == "-ABC-D-EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(-n, -11, 2) == 2 && biostring.sequence() == "--ABCD-EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(-n, -11, 3) == 2 && biostring.sequence() == "--ABCD-EF--GH");

    // Test: postive direction with region containing gaps
    biostring = str; QVERIFY(biostring.slideSegment(6, 9, 1) == 1 && biostring.sequence() == "ABC---D-EF-GH");
    biostring = str; QVERIFY(biostring.slideSegment(6, 9, 2) == 2 && biostring.sequence() == "ABC----D-EFGH");
    biostring = str; QVERIFY(biostring.slideSegment(6, 9, 3) == 2 && biostring.sequence() == "ABC----D-EFGH");

    // Same thing with negative indices
    biostring = str; QVERIFY(biostring.slideSegment(-8, -5, 1) == 1 && biostring.sequence() == "ABC---D-EF-GH");
    biostring = str; QVERIFY(biostring.slideSegment(-8, -5, 2) == 2 && biostring.sequence() == "ABC----D-EFGH");
    biostring = str; QVERIFY(biostring.slideSegment(-8, -5, 3) == 2 && biostring.sequence() == "ABC----D-EFGH");

    // Test: region with terminal gaps
    biostring = str; QVERIFY(biostring.slideSegment(5, 10, 1) == 1 && biostring.sequence() == "ABC---D-EF-GH");
    biostring = str; QVERIFY(biostring.slideSegment(5, 10, 2) == 1 && biostring.sequence() == "ABC---D-EF-GH");
    biostring = str; QVERIFY(biostring.slideSegment(5, 10, -1) == 1 && biostring.sequence() == "ABC-D-EF---GH");
    biostring = str; QVERIFY(biostring.slideSegment(5, 10, -2) == 1 && biostring.sequence() == "ABC-D-EF---GH");

    // Test: move single residue
    biostring = str; QVERIFY(biostring.slideSegment(6, 6, -3) == 2 && biostring.sequence() == "ABCD---EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(6, 6, -1) == 1 && biostring.sequence() == "ABC-D--EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(6, 6, 1) == 1 && biostring.sequence() == "ABC---DEF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(6, 6, 2) == 1 && biostring.sequence() == "ABC---DEF--GH");

    // Test: sliding pure gapped region
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, -1) == 1 && biostring.sequence() == "AB--CD-EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, -2) == 2 && biostring.sequence() == "A--BCD-EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, -3) == 3 && biostring.sequence() == "--ABCD-EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, -4) == 3 && biostring.sequence() == "--ABCD-EF--GH");

    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 1) == 1 && biostring.sequence() == "ABCD---EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 2) == 2 && biostring.sequence() == "ABCD---EF--GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 3) == 3 && biostring.sequence() == "ABCD-E--F--GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 4) == 4 && biostring.sequence() == "ABCD-EF----GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 5) == 5 && biostring.sequence() == "ABCD-EF----GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 6) == 6 && biostring.sequence() == "ABCD-EF----GH");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 7) == 7 && biostring.sequence() == "ABCD-EF--G--H");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 8) == 8 && biostring.sequence() == "ABCD-EF--GH--");
    biostring = str; QVERIFY(biostring.slideSegment(4, 5, 9) == 8 && biostring.sequence() == "ABCD-EF--GH--");
}

// ------------------------------------------------------------------------------------------------
// Static methods
void TestBioString::benchRegexNormalize()
{
    QString str = "MDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFSVL\nQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYICQA\nLRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELGHL\nTTLTDVVKGADSLSAILPGDIAED\nITAVLCFVIEADQITFETVEVSPKISTPPVLKLAA\nEQAPTGRVEREKTTRSSESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNHGD\nLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSSTEL\r\nDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTDDG\nAGLNRERILAKAASQGL\tTVSENMSDDEV\tAMLIFAPGFSTAEQVTDVSGRGVGMDVVKRNI\nQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREADL\rHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLIGQ\n   HQVVVKNLESNYRKVPGISAATILGDGSVALIVDVSALQAINREQRMANTAA";

    QRegExp strip_whitespace("\\s+");

    QBENCHMARK {
        QString copy = str;

        copy.remove(strip_whitespace);
    }
}

void TestBioString::benchQStringNormalize()
{
    QString str = "MDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFSVL\nQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYICQA\nLRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELGHL\nTTLTDVVKGADSLSAILPGDIAED\nITAVLCFVIEADQITFETVEVSPKISTPPVLKLAA\nEQAPTGRVEREKTTRSSESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNHGD\nLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSSTEL\r\nDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTDDG\nAGLNRERILAKAASQGL\tTVSENMSDDEV\tAMLIFAPGFSTAEQVTDVSGRGVGMDVVKRNI\nQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREADL\rHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLIGQ\n   HQVVVKNLESNYRKVPGISAATILGDGSVALIVDVSALQAINREQRMANTAA";

    QBENCHMARK {
        QString copy = str;

        copy.remove(Qt::Key_Space);
        copy.remove('\t');
        copy.remove('\n');
        copy.remove('\v');
        copy.remove('\f');
        copy.remove('\r');
    }
}

void TestBioString::benchQCharNormalize()
{
    QString str = "MDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFSVL\nQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYICQA\nLRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELGHL\nTTLTDVVKGADSLSAILPGDIAED\nITAVLCFVIEADQITFETVEVSPKISTPPVLKLAA\nEQAPTGRVEREKTTRSSESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNHGD\nLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSSTEL\r\nDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTDDG\nAGLNRERILAKAASQGL\tTVSENMSDDEV\tAMLIFAPGFSTAEQVTDVSGRGVGMDVVKRNI\nQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREADL\rHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLIGQ\n   HQVVVKNLESNYRKVPGISAATILGDGSVALIVDVSALQAINREQRMANTAA";

    QBENCHMARK {
        QString copy = str;
        copy.detach();

        const QChar *x = str.constData();
        QChar *y = copy.data();
        while (*x != '\0')
        {
            if (*x != Qt::Key_Space &&
                    *x != '\t' &&
                    *x != '\n' &&
                    *x != '\v' &&
                    *x != '\f' &&
                    *x != '\r')
            {
                *y = *x;
                ++y;
            }

            ++x;
        }
    }
}

void TestBioString::benchQByteArrayNormalize()
{
    QByteArray str = "MDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFSVL\nQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYICQA\nLRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELGHL\nTTLTDVVKGADSLSAILPGDIAED\nITAVLCFVIEADQITFETVEVSPKISTPPVLKLAA\nEQAPTGRVEREKTTRSSESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNHGD\nLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSSTEL\r\nDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTDDG\nAGLNRERILAKAASQGL\tTVSENMSDDEV\tAMLIFAPGFSTAEQVTDVSGRGVGMDVVKRNI\nQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREADL\rHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLIGQ\n   HQVVVKNLESNYRKVPGISAATILGDGSVALIVDVSALQAINREQRMANTAA";

    int l = str.length();
    const char *ptr = str.constData();

    QBENCHMARK {
        QByteArray copy = str;

        const char *x = ptr;
        char *y = copy.data();
        for (int i=0; i<l; ++i)
        {
            if (*x != ' ' &&
                *x != '\t' &&
                *x != '\n' &&
                *x != '\v' &&
                *x != '\f' &&
                *x != '\r')
            {
                *y = *x;
                ++y;
            }
            ++x;
        }
        *y = '\0';
    }
}

void TestBioString::benchQByteArrayAtNormalize()
{
    QByteArray str = "MDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFSVL\nQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYICQA\nLRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELGHL\nTTLTDVVKGADSLSAILPGDIAED\nITAVLCFVIEADQITFETVEVSPKISTPPVLKLAA\nEQAPTGRVEREKTTRSSESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNHGD\nLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSSTEL\r\nDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTDDG\nAGLNRERILAKAASQGL\tTVSENMSDDEV\tAMLIFAPGFSTAEQVTDVSGRGVGMDVVKRNI\nQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREADL\rHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLIGQ\n   HQVVVKNLESNYRKVPGISAATILGDGSVALIVDVSALQAINREQRMANTAA";

    int l = str.length();

    QBENCHMARK {
        QByteArray copy = str;

        char *y = copy.data();
        for (int i=0; i<l; ++i)
        {
            if (str.at(i) != ' ' &&
                str.at(i) != '\t' &&
                str.at(i) != '\n' &&
                str.at(i) != '\v' &&
                str.at(i) != '\f' &&
                str.at(i) != '\r')
            {
                *y = str.at(i);
                ++y;
            }
        }
        *y = '\0';
    }
}

void TestBioString::benchCharNormalize()
{
    char str[] = "MDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFSVL\nQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYICQA\nLRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELGHL\nTTLTDVVKGADSLSAILPGDIAED\nITAVLCFVIEADQITFETVEVSPKISTPPVLKLAA\nEQAPTGRVEREKTTRSSESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNHGD\nLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSSTEL\r\nDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTDDG\nAGLNRERILAKAASQGL\tTVSENMSDDEV\tAMLIFAPGFSTAEQVTDVSGRGVGMDVVKRNI\nQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREADL\rHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLIGQ\n   HQVVVKNLESNYRKVPGISAATILGDGSVALIVDVSALQAINREQRMANTAA";

    int l = qstrlen(str);

    QBENCHMARK {
        char *copy = new char[l+1];

        const char *x = str;
        char *y = copy;
        for (int i=0; i<l; ++i)
        {
            if (*x != ' ' &&
                *x != '\t' &&
                *x != '\n' &&
                *x != '\v' &&
                *x != '\f' &&
                *x != '\r')
            {
                *y = *x;
                ++y;
            }

            ++x;
        }
        *y = '\0';

        delete copy;
        copy = 0;
    }
  }

void TestBioString::staticNormalize()
{
    // Test uppercase
    QVERIFY2(BioString::normalize("aBcDeFg") == "ABCDEFG", "Upper-case all characters");

    // Test removal of whitespace
    QVERIFY2(BioString::normalize("A B C") == "ABC", "Normalize failed to remove whitespace");

    // Test removal of whitespace and uppercase
    QVERIFY2(BioString::normalize("A b C d") == "ABCD", "Normalize failed to remove whitespace and/or upper-case characters");

    // Test removal of newlines
    QVERIFY2(BioString::normalize("LINE1\nLINE2\nLINE3\n") == "LINE1LINE2LINE3", "Failed to remove newlines");

    // Test removal of carriage returns
    QVERIFY2(BioString::normalize("LINE1\rLINE2\rLINE3\r") == "LINE1LINE2LINE3", "Failed to remove carriage returns");

    // Test removal of tab spaces
    QVERIFY2(BioString::normalize("LINE1\tLINE2\tLINE3\t") == "LINE1LINE2LINE3", "Failed to remove tab characters");

    // Test removal of whitespace and newlines
    QVERIFY2(BioString::normalize("ABC DEF\nGHI JKL\r\n\t") == "ABCDEFGHIJKL", "Failed to remove both whitespace and newlines");

    // Test non-alphanumeric characters
    QVERIFY2(BioString::normalize("!@#$%^&*()_+") == "!@#$%^&*()_+", "Funky characters did not remain the same");

    // Test whitespace removal and upper-case
    QVERIFY2(BioString::normalize("abcd\n EFG \n1234\r\n  ") == "ABCDEFG1234", "Failed to properly normalize string");

    // Test does not remove valid characters
    QString valid_characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ-.*";
    for (int i=0, z= valid_characters.length(); i<z; ++i)
        QVERIFY2(BioString::normalize(valid_characters[i]) == QString(valid_characters[i]), QString("Normalize removed valid character %1").arg(valid_characters[i]).toAscii());
}

void TestBioString::staticIsGap()
{
    QString str = "ABCDEFGHIJKLMNOPQRSTUVWXYZ*-.1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z= str.length(); i< z; ++i)
        QVERIFY(BioString::isGap(str.at(i)) == (str.at(i) == '.' || str.at(i) == '-') ? true : false);
}


QTEST_MAIN(TestBioString)
#include "TestBioString.moc"
