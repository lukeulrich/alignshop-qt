/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
** Other authors: Paul Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <typeinfo>
#include "DnaString.h"

class TestDnaString : public QObject
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
    void assignDnaString();
    void assignChar();
    void assignQString();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void alphabet();
    void isValid();
    void mask();

    void removeGaps();
    void hasGaps();

    void reverseComplement(); //2010-05-11 Paul; creates reverse complement fo a sequence
    void reverse();           //2010-05-20 Paul; reverses a sequence; DOES NOT CREATE COMPLEMENT
};

// ------------------------------------------------------------------------------------------------
// Constructors
void TestDnaString::constructorEmpty()
{
    DnaString b;
    QVERIFY2(b.sequence() == "", "Sequence is empty");
}

void TestDnaString::constructorQString()
{
    DnaString b(QString("DEF"));
    QVERIFY2(b.sequence() == "DEF", "Unsuccessful sequence initialization");

    DnaString b2(QString(""));
    QVERIFY2(b2.sequence() == "", "Unsuccessful sequence initialization with empty string");

    DnaString b3(QString("ABC DEF -- ..\nGHI"));
    QVERIFY(b3.sequence() == "ABCDEF--..GHI");
}

void TestDnaString::constructorChar()
{
    DnaString b("DSW");
    QVERIFY2(b.sequence() == "DSW", "Unsuccessful sequence initialization");

    DnaString b2("");
    QVERIFY2(b2.sequence() == "", "Unsuccessful sequence initialization with empty string");

    DnaString b3("ABC DSW -- ..\nNIX");
    QVERIFY(b3.sequence() == "ABCDSW--..NIX");
}

void TestDnaString::constructorCopy()
{
    QString str = "1234567891";
    DnaString b1(str);
    DnaString b2(b1);

    QVERIFY2(b2.sequence() == str, "Copied sequence is incorrect");
}

void TestDnaString::clone()
{
    QString str = "ABC--DEF--GHI";
    DnaString ds(str);

    BioString *bs_ptr = &ds;

    QVERIFY(bs_ptr->sequence() == str);

    BioString *bs2_ptr = bs_ptr->clone();
    QVERIFY(bs2_ptr);
    *bs2_ptr = bs2_ptr->masked();
    QVERIFY(QString(typeid(*bs2_ptr).name()).contains("DnaString"));
    QVERIFY2(bs2_ptr->sequence() == "ABC--DNN--GHI", bs2_ptr->sequence().toAscii());
    QVERIFY(ds.sequence() == str);

    delete bs2_ptr;
    bs2_ptr = 0;
}

void TestDnaString::create()
{
    QString str = "ABC--DEF--GHI*";
    DnaString ds(str);

    BioString *bs_ptr = &ds;

    QVERIFY(bs_ptr->sequence() == str);

    BioString *bs2_ptr = bs_ptr->create();
    QVERIFY(bs2_ptr);
    QVERIFY(bs2_ptr->sequence().isEmpty());
    QVERIFY(QString(typeid(*bs2_ptr).name()).contains("DnaString"));
    QVERIFY(ds.sequence() == str);

    delete bs2_ptr;
    bs2_ptr = 0;
}


// ------------------------------------------------------------------------------------------------
// Assignment operators
void TestDnaString::assignDnaString()
{
    DnaString b1("GENOMICS");
    DnaString b2("COMPUTATIONAL");

    b1 = b2;
    QVERIFY2(b1.sequence() == "COMPUTATIONAL", "Unsuccessful assignment");
}

void TestDnaString::assignChar()
{
    DnaString DnaString = "ABCDEF12345";
    QVERIFY2(DnaString.sequence() == "ABCDEF12345", "Failed to assign simple character array");

    const char *test2 = "  abc  \n1234 dEf";
    DnaString = test2;
    QVERIFY2(DnaString.sequence() == "ABC1234DEF", "Failed to assign and normalize simple character array");
}

void TestDnaString::assignQString()
{
    QString test1 = "ABCDEF12345";
    DnaString b1;
    b1 = test1;
    QVERIFY2(b1.sequence() == test1, "Failed to assign simple QString");

    QString test2 = "  abc  \n1234 dEf";
    b1 = test2;
    QVERIFY2(b1.sequence() == "ABC1234DEF", "Failed to assign and normalize simple QString");
}


// ------------------------------------------------------------------------------------------------
// Public methods
void TestDnaString::alphabet()
{
    DnaString d;
    QCOMPARE(d.alphabet(), eDnaAlphabet);
}

void TestDnaString::isValid()
{
    DnaString bs("ABCDSW");
    QVERIFY2(bs.isValid(), "Calling isValid directly on DnaString failed with valid sequence");

    bs = "ABC 1234";
    QVERIFY2(bs.isValid() == false, "After assignment of an invalid QString sequence, isValid direct call failed");

    bs = "ABC*";
    QVERIFY(bs.isValid() == true);

    // Test string with one of every valid character
    bs = "ABCDGHIKMNRSTVWXY*-.";
    QVERIFY2(bs.isValid() == true, "Valid characters not classified as valid");

    // Test validation with sequence that requires normalization
    bs = "abc d\n \r \t swn-.";
    QVERIFY2(bs.isValid() == true, "Valid sequence with interleaved whitespace not classified as valid");

    // Test with a slew of invalid characters
    QString invalid_characters = "EFJLOPQUZ1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z=invalid_characters.length(); i<z; ++i)
    {
        bs = invalid_characters[i];
        QVERIFY2(bs.isValid() == false, QString("Invalid character %1 classified as valid").arg(invalid_characters[i]).toAscii());
    }
}

void TestDnaString::mask()
{
    DnaString dnastring = "ABC$%^DEF*";
    QVERIFY2(dnastring.sequence() == "ABC$%^DEF*", QString("Sequence: %1").arg(dnastring.sequence()).toAscii());

    dnastring = dnastring.masked();
    QVERIFY2(dnastring.sequence() == "ABCNNNDNN*", QString("Result: %1").arg(dnastring.sequence()).toAscii());

    dnastring = "1234567890";
    dnastring = dnastring.masked('z');
    QVERIFY2(dnastring.sequence() == "ZZZZZZZZZZ", QString("Result: %1").arg(dnastring.sequence()).toAscii());
}

void TestDnaString::removeGaps()
{
    // Test removal of dash gaps
    DnaString str = "--AB--CDSW--";
    str = str.ungapped();
    QVERIFY2(str.sequence() == "ABCDSW", "Failed to remove gaps (dashes)");

    // Test removal of period gaps
    str = "..AB..CDSW..";
    str = str.ungapped();
    QVERIFY2(str.sequence() == "ABCDSW", "Failed to remove gaps (dots)");

    // Test removal of mixed dashes and dots
    str = "..AB--CD..SW--";
    str = str.ungapped();
    QVERIFY2(str.sequence() == "ABCDSW", "Failed to remove gaps (dashes and dots)");
}

void TestDnaString::hasGaps()
{
    DnaString dnastring = "BOb";

    // Test does not have gaps
    QVERIFY(!dnastring.hasGaps());

    dnastring = "--B--ob--";
    QVERIFY(dnastring.hasGaps());

    dnastring = ".";
    QVERIFY(dnastring.hasGaps());

    dnastring = " .. - ADF .. ---";
    QVERIFY(dnastring.hasGaps());

    dnastring = "    asdf e fjsd f  dkfei";
    QVERIFY(!dnastring.hasGaps());
}

void TestDnaString::reverseComplement()
{
    DnaString dnastring = "ATGTCAG";

    QVERIFY2(dnastring.reverseComplement().length() == dnastring.length(), "Reverse complement is incorrect length");
    QCOMPARE(dnastring.reverseComplement(), QString("CTGACAT"));
}

void TestDnaString::reverse()
{
    DnaString dnastring = "ATGTCAG";

    QCOMPARE(dnastring.reverse(), QString("GACTGTA"));
}

QTEST_APPLESS_MAIN(TestDnaString)
#include "TestDnaString.moc"
