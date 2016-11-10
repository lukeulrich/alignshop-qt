/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "BioStringValidator.h"
#include "global.h"

class TestBioStringValidator : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void operator_eqeq();
    void alphabet();            // also tests set alphabet
    void ignoreGaps();          // also tests setIgnoreGaps
    void isValidString();
    void isValidChar();

    void isValidString_gapped();
    void isValidChar_gapped();
};

void TestBioStringValidator::operator_eqeq()
{
    BioStringValidator bsv("ATC");
    QVERIFY(bsv == bsv);

    BioStringValidator bsv2("ATC");
    QVERIFY(bsv == bsv2);
}

void TestBioStringValidator::alphabet()
{
    BioStringValidator bsv("");
    QVERIFY(bsv.validCharacters().isEmpty());
    bsv.setValidCharacters("ACGTN");
    QCOMPARE(bsv.validCharacters(), QString("ACGTN"));

    bsv.setValidCharacters(QString());
    QVERIFY(bsv.validCharacters().isEmpty());

    BioStringValidator bsv2("ACG");
    QCOMPARE(bsv2.validCharacters(), QString("ACG"));
}

void TestBioStringValidator::ignoreGaps()
{
    BioStringValidator bsv("ATC");

    QVERIFY(bsv.ignoreGaps() == true);

    bsv.setIgnoreGaps(false);
    QVERIFY(bsv.ignoreGaps() == false);

    bsv.setIgnoreGaps(true);
    QVERIFY(bsv.ignoreGaps() == true);

    // Check via copy constructor
    BioStringValidator bsv2(bsv);
    QVERIFY(bsv2.ignoreGaps() == true);

    // Check via assignment operator
    bsv.setIgnoreGaps(false);
    bsv2 = bsv;
    QVERIFY(bsv2.ignoreGaps() == false);

    BioStringValidator bsv3(bsv);
    QVERIFY(bsv3.ignoreGaps() == false);

}

void TestBioStringValidator::isValidString()
{
    BioStringValidator bsv(constants::kGenericBioStringCharacters);

    BioString bs("ABCDEF");
    QVERIFY(bsv.isValid(bs));

    bs = "ABC 1234";
    QVERIFY(bsv.isValid(bs) == false);

    // Test string with one of every valid character
    BioString biostring(constants::kGenericBioStringCharacters);
    QVERIFY2(bsv.isValid(biostring), "Valid characters not classified as valid");

    // Test with a slew of invalid characters
    QString invalid_characters = "1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z=invalid_characters.length(); i<z; ++i)
    {
        biostring = invalid_characters[i];
        QVERIFY2(bsv.isValid(biostring) == false, QString("Invalid character %1 classified as valid").arg(invalid_characters[i]).toAscii());
    }

    // Test validation with sequence that requires normalization
    biostring = "abc def ghi\njkl\nmno\npqr\rstu\tvwxyz*-.";
    QVERIFY2(bsv.isValid(biostring), "Valid sequence with interleaved whitespace not classified as valid");
}

void TestBioStringValidator::isValidChar()
{
    BioStringValidator bsv(constants::kGenericBioStringCharacters);

    // Test: all valid characters should return true
    QString valid_chars = constants::kGenericBioStringCharacters;

    for (int i=0, z=valid_chars.length(); i<z; ++i)
        QVERIFY(bsv.isValid(valid_chars.at(i)));

    // Test: all invalid characters should return false
    QString invalid_chars = "abcdefghijklmnopqrstuvwxyz1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z=invalid_chars.length(); i<z; ++i)
        QVERIFY(bsv.isValid(invalid_chars.at(i)) == false);
}

void TestBioStringValidator::isValidString_gapped()
{
    BioStringValidator bsv(constants::kDnaCharacters);

    // Test: by default strings with gaps should be accepted
    BioString bs("A--CT--G..");
    QVERIFY(bsv.isValid(bs));

    // Now do not ignore gaps
    bsv.setIgnoreGaps(false);
    QVERIFY(bsv.isValid(bs) == false);

    // Test string with one of every gap character
    BioString biostring(constants::kGapCharacters);
    QVERIFY(bsv.isValid(biostring) == false);

    bsv.setIgnoreGaps(true);
    QVERIFY(bsv.isValid(biostring));
}

void TestBioStringValidator::isValidChar_gapped()
{
    BioStringValidator bsv(constants::kDnaCharacters);

    // Test: all valid characters should return true
    QString gap_chars = constants::kGapCharacters;

    for (int i=0, z=gap_chars.length(); i<z; ++i)
        QVERIFY(bsv.isValid(gap_chars.at(i)));

    bsv.setIgnoreGaps(false);
    for (int i=0, z=gap_chars.length(); i<z; ++i)
        QVERIFY(bsv.isValid(gap_chars.at(i)) == false);
}


QTEST_MAIN(TestBioStringValidator)
#include "TestBioStringValidator.moc"
