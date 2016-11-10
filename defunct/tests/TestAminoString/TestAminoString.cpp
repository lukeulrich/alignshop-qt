/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <typeinfo>
#include "AminoString.h"

class TestAminoString : public QObject
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
    void assignAminoString();
    void assignChar();
    void assignQString();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void alphabet();
    void isValid();
    void mask();

    // Although this function does not have any specific code with regard to removing gaps,
    // we still execute this test to ensure that it is possible to call the base call functions
    // using the derived class instance
    void removeGaps();
    void hasGaps();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestAminoString::constructorEmpty()
{
    AminoString b;
    QVERIFY2(b.sequence() == "", "Sequence is empty");
}

void TestAminoString::constructorQString()
{
    AminoString b(QString("DEF"));
    QVERIFY2(b.sequence() == "DEF", "Unsuccessful sequence initialization");

    AminoString b2(QString(""));
    QVERIFY2(b2.sequence() == "", "Unsuccessful sequence initialization with empty string");

    AminoString b3(QString("ABC DEF -- ..\nGHI"));
    QVERIFY(b3.sequence() == "ABCDEF--..GHI");
}

void TestAminoString::constructorChar()
{
    AminoString b("DEF");
    QVERIFY2(b.sequence() == "DEF", "Unsuccessful sequence initialization");

    AminoString b2("");
    QVERIFY2(b2.sequence() == "", "Unsuccessful sequence initialization with empty string");

    AminoString b3("ABC DEF -- ..\nGHI");
    QVERIFY(b3.sequence() == "ABCDEF--..GHI");
}

void TestAminoString::constructorCopy()
{
    QString str = "1234567891";
    AminoString b1(str);
    AminoString b2(b1);

    QVERIFY2(b2.sequence() == str, "Copied sequence is incorrect");
}

void TestAminoString::clone()
{
    QString str = "ABC--DEF--GHI*";
    AminoString as(str);

    BioString *bs_ptr = &as;

    QVERIFY(bs_ptr->sequence() == str);

    BioString *bs2_ptr = bs_ptr->clone();
    QVERIFY(QString(typeid(*bs2_ptr).name()).contains("AminoString"));
    *bs2_ptr = bs2_ptr->masked();
    QVERIFY(bs2_ptr->sequence() == "ABC--DEF--GHIX");
    QVERIFY(as.sequence() == str);

    delete bs2_ptr;
    bs2_ptr = 0;
}

void TestAminoString::create()
{
    QString str = "ABC--DEF--GHI*";
    AminoString as(str);

    BioString *bs_ptr = &as;

    QVERIFY(bs_ptr->sequence() == str);

    BioString *bs2_ptr = bs_ptr->create();
    QVERIFY(bs2_ptr->sequence().isEmpty());
    QVERIFY(QString(typeid(*bs2_ptr).name()).contains("AminoString"));
    QVERIFY(as.sequence() == str);

    delete bs2_ptr;
    bs2_ptr = 0;
}

// ------------------------------------------------------------------------------------------------
// Assignment operators
void TestAminoString::assignAminoString()
{
    AminoString b1("GENOMICS");
    AminoString b2("COMPUTATIONAL");

    b1 = b2;
    QVERIFY2(b1.sequence() == "COMPUTATIONAL", "Unsuccessful assignment");
}

void TestAminoString::assignChar()
{
    AminoString AminoString = "ABCDEF12345";
    QVERIFY2(AminoString.sequence() == "ABCDEF12345", "Failed to assign simple character array");

    char *test2 = "  abc  \n1234 dEf";
    AminoString = test2;
    QVERIFY2(AminoString.sequence() == "ABC1234DEF", "Failed to assign and normalize simple character array");
}

void TestAminoString::assignQString()
{
    QString test1 = "ABCDEF12345";
    AminoString b1;
    b1 = test1;
    QVERIFY2(b1.sequence() == test1, "Failed to assign simple QString");

    QString test2 = "  abc  \n1234 dEf";
    b1 = test2;
    QVERIFY2(b1.sequence() == "ABC1234DEF", "Failed to assign and normalize simple QString");
}


// ------------------------------------------------------------------------------------------------
// Public methods
void TestAminoString::alphabet()
{
    AminoString a;
    QCOMPARE(a.alphabet(), eAminoAlphabet);
}

void TestAminoString::isValid()
{
    AminoString bs("ABCDEF");
    QVERIFY2(bs.isValid(), "Calling isValid directly on AminoString failed with valid sequence");

    bs = "ABC 1234";
    QVERIFY2(bs.isValid() == false, "After assignment of an invalid QString sequence, isValid direct call failed");

    bs = "ABC*";
    QVERIFY(bs.isValid() == false);

    // Test string with one of every valid character
    AminoString aminostring("abcdefghijklmnopqrstuvwxyz-.");
    QVERIFY2(aminostring.isValid() == true, "Valid characters not classified as valid");

    // Test with a slew of invalid characters
    QString invalid_characters = "1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\*";
    for (int i=0, z=invalid_characters.length(); i<z; ++i)
    {
        aminostring = invalid_characters[i];
        QVERIFY2(aminostring.isValid() == false, QString("Invalid character %1 classified as valid").arg(invalid_characters[i]).toAscii());
    }

    // Test validation with sequence that requires normalization
    aminostring = "abc def ghi\njkl\nmno\npqr\rstu\tvwxyz-.";
    QVERIFY2(aminostring.isValid() == true, "Valid sequence with interleaved whitespace not classified as valid");
}

void TestAminoString::mask()
{
    AminoString aminostring = "ABC$%^DEF*";
    QVERIFY2(aminostring.sequence() == "ABC$%^DEF*", QString("Sequence: %1").arg(aminostring.sequence()).toAscii());

    aminostring = aminostring.masked();
    QVERIFY2(aminostring.sequence() == "ABCXXXDEFX", QString("Result: %1").arg(aminostring.sequence()).toAscii());

    aminostring = "1234567890";
    aminostring = aminostring.masked('z');
    QVERIFY2(aminostring.sequence() == "ZZZZZZZZZZ", QString("Result: %1").arg(aminostring.sequence()).toAscii());
}

void TestAminoString::removeGaps()
{
    // Test removal of dash gaps
    AminoString aminostring("--AB--CDEF--");

    aminostring = aminostring.ungapped();
    QVERIFY2(aminostring.sequence() == "ABCDEF", "Failed to remove gaps (dashes)");

    // Test removal of period gaps
    aminostring = "..AB..CDEF..";
    aminostring = aminostring.ungapped();
    QVERIFY2(aminostring.sequence() == "ABCDEF", "Failed to remove gaps (dots)");

    // Test removal of mixed dashes and dots
    aminostring = "..AB--CD..EF--";
    aminostring = aminostring.ungapped();
    QVERIFY2(aminostring.sequence() == "ABCDEF", "Failed to remove gaps (dots and dashes)");
}

void TestAminoString::hasGaps()
{
    AminoString aminostring = "BOb";

    // Test does not have gaps
    QVERIFY(!aminostring.hasGaps());

    aminostring = "--B--ob--";
    QVERIFY(aminostring.hasGaps());

    aminostring = ".";
    QVERIFY(aminostring.hasGaps());

    aminostring = " .. - ADF .. ---";
    QVERIFY(aminostring.hasGaps());

    aminostring = "    asdf e fjsd f  dkfei";
    QVERIFY(!aminostring.hasGaps());
}

QTEST_MAIN(TestAminoString)
#include "TestAminoString.moc"


