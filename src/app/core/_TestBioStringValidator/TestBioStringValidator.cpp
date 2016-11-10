/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../BioStringValidator.h"
#include "../constants.h"

class TestBioStringValidator : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void isValidChar();
    void isValidQChar();
    void isValidBioString();
    void isValidQByteArray();

    void benchValidTestWithChars();
    void benchValidTestWithSet();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestBioStringValidator::constructor()
{
    BioStringValidator x("ACGT");

    BioString response = x.validCharacters();
    QCOMPARE(response.length(), 4);
    QVERIFY(response.indexOf('A') != -1);
    QVERIFY(response.indexOf('C') != -1);
    QVERIFY(response.indexOf('G') != -1);
    QVERIFY(response.indexOf('T') != -1);
}

void TestBioStringValidator::isValidChar()
{
    BioStringValidator bsv("ACGT");

    // Test: all valid characters should return true
    BioString valid_chars = "ACGT";

    for (int i=0, z=valid_chars.length(); i<z; ++i)
        QVERIFY(bsv.isValid(valid_chars.at(i+1)));

    // Test: all invalid characters should return false
    QByteArray invalid_chars = "abcdefghijklmnopqrstuvwxyz1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z=invalid_chars.length(); i<z; ++i)
        QVERIFY(bsv.isValid(invalid_chars.at(i)) == false);
}

void TestBioStringValidator::isValidQChar()
{
    BioStringValidator bsv("ACGT");

    // Test: all valid characters should return true
    QString valid_chars = "ACGT";

    for (int i=0, z=valid_chars.length(); i<z; ++i)
        QVERIFY(bsv.isValid(valid_chars.at(i)));

    // Test: all invalid characters should return false
    QString invalid_chars = "abcdefghijklmnopqrstuvwxyz1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z=invalid_chars.length(); i<z; ++i)
        QVERIFY(bsv.isValid(invalid_chars.at(i)) == false);
}

void TestBioStringValidator::isValidBioString()
{
    BioStringValidator bsv(constants::kAminoBasicCharacters);

    BioString bs("ACDEF");
    QVERIFY(bsv.isValid(bs));

    bs = "ABC 1234";
    QVERIFY(bsv.isValid(bs) == false);

    // Test string with one of every valid character
    BioString biostring(constants::kAminoBasicCharacters);
    QVERIFY(bsv.isValid(biostring));

    // Test with a slew of invalid characters
    QByteArray invalid_characters = "acdef1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    for (int i=0, z=invalid_characters.length(); i<z; ++i)
    {
        biostring = invalid_characters.at(i);
        QVERIFY2(bsv.isValid(biostring) == false, QString("Invalid character %1 classified as valid").arg(invalid_characters[i]).toAscii());
    }

    // Test validation with sequence that requires normalization
    biostring = "ac def ghi\nkl\nmn\npqr\rst\tvwy";
    biostring = biostring.toUpper();
    QVERIFY2(bsv.isValid(biostring), "Valid sequence with interleaved whitespace not classified as valid");
}

void TestBioStringValidator::isValidQByteArray()
{
    BioStringValidator bsv(constants::kAminoBasicCharacters);

    QByteArray bs("ACDEF");
    QVERIFY(bsv.isValid(bs));

    bs = "ABC 1234";
    QVERIFY(bsv.isValid(bs) == false);

    // Test string with one of every valid character
    QByteArray byteArray(constants::kAminoBasicCharacters);
    QVERIFY(bsv.isValid(byteArray));

    // Test with a slew of invalid characters
    QByteArray invalid_characters = "acdef1234567890`~!@#$%^&()_+=[]{};'\",/<>?|\\";
    char dummy[2] = " ";
    for (int i=0, z=invalid_characters.length(); i<z; ++i)
    {
        dummy[0] = invalid_characters.at(i);
        byteArray = dummy;
        QVERIFY2(bsv.isValid(byteArray) == false, QString("Invalid character %1 classified as valid").arg(invalid_characters[i]).toAscii());
    }
}

void TestBioStringValidator::benchValidTestWithChars()
{
    const char validChars[] = "ABCDGHKMNRSUVWY";
    QByteArray data = "MSMDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFS"
            "VLQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYIC"
            "QALRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELG"
            "HLTTLTDVVKGADSLSAILPGDIAEDDITAVLCFVIEADQITFETVEVSPKISTPPVLKL"
            "AAEQAPTGRVEREKTTRSNESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNH"
            "GDLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSST"
            "ELDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTD"
            "DGAGLNRERILAKAASQGLTVSENMSDDEVAMLIFAPGFSTAEQITDVSGRGVGMDVVKR"
            "NIQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREA"
            "DLHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLI"
            "GQHQVVVKNLESNYRKVPGISAATILGDGSVALIVDVSALQAINREQRMANTAA";

    const char *x;

    QBENCHMARK
    {
        x = data.constData();
        while (*x)
        {
            const char *y = validChars;
            while (*y)
            {
                if (*y == *x)
                    break;
                ++y;
            }
            ++x;
        }
    }
}

void TestBioStringValidator::benchValidTestWithSet()
{
    const char validChars[] = "ABCDGHKMNRSUVWY";
    QSet<char> validCharSet;
    const char *x = validChars;
    while (*x)
    {
        validCharSet << *x;
        ++x;
    }

    QByteArray data = "MSMDISDFYQTFFDEADELLADMEQHLLVLQPEAPDAEQLNAIFRAAHSIKGGAGTFGFS"
            "VLQETTHLMENLLDEARRGEMQLNTDIINLFLETKDIMQEQLDAYKQSQEPDAASFDYIC"
            "QALRQLALEAKGETPSAVTRLSVVAKSEPQDEQSRSQSPRRIILSRLKAGEVDLLEEELG"
            "HLTTLTDVVKGADSLSAILPGDIAEDDITAVLCFVIEADQITFETVEVSPKISTPPVLKL"
            "AAEQAPTGRVEREKTTRSNESTSIRVAVEKVDQLINLVGELVITQSMLAQRSSELDPVNH"
            "GDLITSMGQLQRNARDLQESVMSIRMMPMEYVFSRYPRLVRDLAGKLGKQVELTLVGSST"
            "ELDKSLIERIIDPLTHLVRNSLDHGIELPEKRLAAGKNSVGNLILSAEHQGGNICIEVTD"
            "DGAGLNRERILAKAASQGLTVSENMSDDEVAMLIFAPGFSTAEQITDVSGRGVGMDVVKR"
            "NIQEMGGHVEIQSKQGTGTTIRILLPLTLAILDGMSVRVADEVFILPLNAVMESLQPREA"
            "DLHPLAGGERVLEVRGEYLPIVELWKVFNVAGAKTEATQGIVVILQSGGRRYALLVDQLI"
            "GQHQVVVKNLESNYRKVPGISAATILGDGSVALIVDVSALQAINREQRMANTAA";

    QBENCHMARK
    {
        x = data.constData();
        while (*x)
        {
            if (validCharSet.contains(*x))
                ++x;
            else
                ++x;
        }
    }
}

QTEST_APPLESS_MAIN(TestBioStringValidator)

#include "TestBioStringValidator.moc"
