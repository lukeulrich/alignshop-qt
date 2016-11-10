/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <typeinfo>
#include "RnaString.h"

class TestRnaString : public QObject
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
    void assignRnaString();
    void assignChar();
    void assignQString();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void isValid();
    void mask();

    void removeGaps();
    void hasGaps();
};

// ------------------------------------------------------------------------------------------------
// Constructors
void TestRnaString::constructorEmpty()
{
    RnaString b;
    QVERIFY2(b.sequence() == "", "Sequence is empty");
}

void TestRnaString::constructorQString()
{
    RnaString b(QString("DEF"));
    QVERIFY2(b.sequence() == "DEF", "Unsuccessful sequence initialization");

    RnaString b2(QString(""));
    QVERIFY2(b2.sequence() == "", "Unsuccessful sequence initialization with empty string");

    RnaString b3(QString("ABC DEF -- ..\nGHI"));
    QVERIFY(b3.sequence() == "ABCDEF--..GHI");
}

void TestRnaString::constructorChar()
{
    RnaString b("DSW");
    QVERIFY2(b.sequence() == "DSW", "Unsuccessful sequence initialization");

    RnaString b2("");
    QVERIFY2(b2.sequence() == "", "Unsuccessful sequence initialization with empty string");

    RnaString b3("ABC DSW -- ..\nNIX");
    QVERIFY(b3.sequence() == "ABCDSW--..NIX");
}

void TestRnaString::constructorCopy()
{
    QString str = "1234567891";
    RnaString b1(str);
    RnaString b2(b1);

    QVERIFY2(b2.sequence() == str, "Copied sequence is incorrect");
}

void TestRnaString::clone()
{
    QString str = "ABC--DEF--GHI";
    RnaString rs(str);

    BioString *bs_ptr = &rs;

    QVERIFY(bs_ptr->sequence() == str);

    BioString *bs2_ptr = bs_ptr->clone();
    QVERIFY(bs2_ptr);
    QVERIFY(QString(typeid(*bs2_ptr).name()).contains("RnaString"));
    *bs2_ptr = bs2_ptr->masked();
    QVERIFY2(bs2_ptr->sequence() == "ABC--DNN--GHI", bs2_ptr->sequence().toAscii());
    QVERIFY(rs.sequence() == str);

    delete bs2_ptr;
    bs2_ptr = 0;
}

void TestRnaString::create()
{
    QString str = "ABC--DEF--GHI";
    RnaString rs(str);

    BioString *bs_ptr = &rs;

    QVERIFY(bs_ptr->sequence() == str);

    BioString *bs2_ptr = bs_ptr->create();
    QVERIFY(bs2_ptr);
    QVERIFY(QString(typeid(*bs2_ptr).name()).contains("RnaString"));
    QVERIFY(bs2_ptr->sequence().isEmpty());
    QVERIFY(rs.sequence() == str);

    delete bs2_ptr;
    bs2_ptr = 0;
}


// ------------------------------------------------------------------------------------------------
// Assignment operators
void TestRnaString::assignRnaString()
{
    RnaString b1("GENOMICS");
    RnaString b2("COMPUTATIONAL");

    b1 = b2;
    QVERIFY2(b1.sequence() == "COMPUTATIONAL", "Unsuccessful assignment");
}

void TestRnaString::assignChar()
{
    RnaString RnaString = "ABCDEF12345";
    QVERIFY2(RnaString.sequence() == "ABCDEF12345", "Failed to assign simple character array");

    char *test2 = "  abc  \n1234 dEf";
    RnaString = test2;
    QVERIFY2(RnaString.sequence() == "ABC1234DEF", "Failed to assign and normalize simple character array");
}

void TestRnaString::assignQString()
{
    QString test1 = "ABCDEF12345";
    RnaString b1;
    b1 = test1;
    QVERIFY2(b1.sequence() == test1, "Failed to assign simple QString");

    QString test2 = "  abc  \n1234 dEf";
    b1 = test2;
    QVERIFY2(b1.sequence() == "ABC1234DEF", "Failed to assign and normalize simple QString");
}


// ------------------------------------------------------------------------------------------------
// Public methods
void TestRnaString::isValid()
{
    RnaString bs("ABCDSW");
    QVERIFY2(bs.isValid(), "Calling isValid directly on RnaString failed with valid sequence");

    bs = "ABC 1234";
    QVERIFY2(bs.isValid() == false, "After assignment of an invalid QString sequence, isValid direct call failed");

    bs = "ABC*";
    QVERIFY(bs.isValid() == true);

    // Test string with one of every valid character
    bs = "ABCDGHIKMNRSUVWXY*-.";
    QVERIFY2(bs.isValid() == true, "Valid characters not classified as valid");

    // Test validation with sequence that requires normalization
    bs = "abc d\n \r \t swn-.";
    QVERIFY2(bs.isValid() == true, "Valid sequence with interleaved whitespace not classified as valid");

    // Test with a slew of invalid characters
    QString invalid_characters = "EFJLOPQTZ1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z=invalid_characters.length(); i<z; ++i)
    {
        bs = invalid_characters[i];
        QVERIFY2(bs.isValid() == false, QString("Invalid character %1 classified as valid").arg(invalid_characters[i]).toAscii());
    }
}

void TestRnaString::mask()
{
    RnaString rnastring = "ABC$%^DEF*";
    QVERIFY2(rnastring.sequence() == "ABC$%^DEF*", QString("Sequence: %1").arg(rnastring.sequence()).toAscii());

    rnastring = rnastring.masked('N');
    QVERIFY2(rnastring.sequence() == "ABCNNNDNN*", QString("Result: %1").arg(rnastring.sequence()).toAscii());

    rnastring = "1234567890";
    rnastring = rnastring.masked('z');
    QVERIFY2(rnastring.sequence() == "ZZZZZZZZZZ", QString("Result: %1").arg(rnastring.sequence()).toAscii());
}

void TestRnaString::removeGaps()
{
    // Test removal of dash gaps
    RnaString str = "--AB--CDSW--";
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

void TestRnaString::hasGaps()
{
    RnaString dnastring = "BOb";

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


QTEST_MAIN(TestRnaString)
#include "TestRnaString.moc"
