/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../AlphabetDetector.h"

class TestAlphabetDetector : public QObject
{
    Q_OBJECT

private slots:
    void detectAlphabet_data();
    void detectAlphabet();
};

typedef QVector<Alphabet> AlphabetVector;
Q_DECLARE_METATYPE(AlphabetVector)
Q_DECLARE_METATYPE(Alphabet)

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAlphabetDetector::detectAlphabet_data()
{
    QTest::addColumn<AlphabetVector>("alphabetVector");
    QTest::addColumn<bool>("acceptAnyCharacter");
    QTest::addColumn<QString>("string");
    QTest::addColumn<Alphabet>("alphabet");

    // Test: no alphabet vector
    QTest::newRow("No alphabet vector, empty biostring") << AlphabetVector() << true << "" << Alphabet();
    QTest::newRow("No alphabet vector, 'ACGT'") << AlphabetVector() << true << "ACGT" << Alphabet();
    QTest::newRow("No alphabet vector, ' A C DEFG'") << AlphabetVector() << true << "A C DEFG" << Alphabet();

    // Test: Basic Dna alphabet
    AlphabetVector basicDnaAlphabetVector;
    basicDnaAlphabetVector << constants::kDnaBasicAlphabet;
    QTest::newRow("Dna alphabet only, empty string") << basicDnaAlphabetVector << true << "" << Alphabet();
    QTest::newRow("Dna alphabet only, 'BDEF'") << basicDnaAlphabetVector << true << "BDEF" << Alphabet();
    QTest::newRow("Dna alphabet only, 'ACGTACGTB'") << basicDnaAlphabetVector << true << "ACGTACGTB" << Alphabet();
    QTest::newRow("Dna alphabet only, 'ACGT'") << basicDnaAlphabetVector << true << "ACGT" << basicDnaAlphabetVector.first();
    QTest::newRow("Dna alphabet only, 'GC'") << basicDnaAlphabetVector << true << "GC" << basicDnaAlphabetVector.first();
    QTest::newRow("Dna alphabet permit 'any' character, 'ACGTN'") << basicDnaAlphabetVector << true << "ACGTN" << basicDnaAlphabetVector.first();
    QTest::newRow("Dna alphabet forbid 'any' character, 'ACGTN'") << basicDnaAlphabetVector << false << "ACGTN" << Alphabet();

    // Test: Basic Dna and Rna alphabets
    AlphabetVector dnaRnaAlphabetVector;
    dnaRnaAlphabetVector << constants::kDnaBasicAlphabet
                         << constants::kRnaBasicAlphabet;
    QTest::newRow("Dna + Rna alphabets, empty string") << dnaRnaAlphabetVector << true << "" << Alphabet();
    QTest::newRow("Dna + Rna alphabets, ACGT") << dnaRnaAlphabetVector << false << "ACGT" << constants::kDnaBasicAlphabet;
    QTest::newRow("Dna + Rna alphabets, ACGU") << dnaRnaAlphabetVector << false << "ACGU" << constants::kRnaBasicAlphabet;
    QTest::newRow("Dna + Rna alphabets, ACGTU") << dnaRnaAlphabetVector << false << "ACGTU" << Alphabet();

    // Note: even though ACG is ambiguous with respect to its alphabet, the first one should be returned
    QTest::newRow("Dna + Rna alphabets, ACG") << dnaRnaAlphabetVector << false << "ACG" << constants::kDnaBasicAlphabet;
    QTest::newRow("Dna + Rna alphabets, ACGN") << dnaRnaAlphabetVector << false << "ACGN" << Alphabet();
    QTest::newRow("Dna + Rna alphabets, ACGN") << dnaRnaAlphabetVector << true << "ACGN" << constants::kDnaBasicAlphabet;

    // Test: Ambiguous case where one alphabet is less than the other
    AlphabetVector dnaAminoAlphabetVector;
    dnaAminoAlphabetVector << constants::kDnaBasicAlphabet
                           << constants::kAminoBasicAlphabet;
    QTest::newRow("Dna + Amino alphabets, A") << dnaAminoAlphabetVector << true << "A" << constants::kDnaBasicAlphabet;
    QTest::newRow("Dna + Amino alphabets, H") << dnaAminoAlphabetVector << true << "H" << constants::kAminoBasicAlphabet;
    QTest::newRow("Dna + Amino alphabets, Z") << dnaAminoAlphabetVector << true << "Z" << Alphabet();
    QTest::newRow("Dna + Amino alphabets, permit 'any' character, X") << dnaAminoAlphabetVector << true << "X" << constants::kAminoBasicAlphabet;
    QTest::newRow("Dna + Amino alphabets, forbid 'any' character, X") << dnaAminoAlphabetVector << false << "X" << Alphabet();

    // Test: Standard alphabet vectors
    QTest::newRow("All alphabets, forbid 'any' character, ACGT") << constants::kStandardAlphabetVector << false << "ACGT" << constants::kDnaBasicAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ACGTN") << constants::kStandardAlphabetVector << false << "ACGTN" << constants::kDnaAmbiguousAlphabet;
    QTest::newRow("All alphabets, permit 'any' character, ACGTN") << constants::kStandardAlphabetVector << true << "ACGTN" << constants::kDnaBasicAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ABCDGSTW") << constants::kStandardAlphabetVector << false << "ABCDGSTW" << constants::kDnaExtendedAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ABCDGSTWN") << constants::kStandardAlphabetVector << false << "ABCDGSTWN" << constants::kDnaAmbiguousAlphabet;
    QTest::newRow("All alphabets, permit 'any' character, ABCDGSTWN") << constants::kStandardAlphabetVector << true << "ABCDGSTWN" << constants::kDnaExtendedAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ABCDGHKMNRSTVWY") << constants::kStandardAlphabetVector << false << "ABCDGHKMNRSTVWY" << constants::kDnaAmbiguousAlphabet;
    QTest::newRow("All alphabets, permit 'any' character, ABCDGHKMNRSTVWY") << constants::kStandardAlphabetVector << true << "ABCDGHKMNRSTVWY" << constants::kDnaAmbiguousAlphabet;

    QTest::newRow("All alphabets, forbid 'any' character, ACGU") << constants::kStandardAlphabetVector << false << "ACGU" << constants::kRnaBasicAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ACGUN") << constants::kStandardAlphabetVector << false << "ACGUN" << constants::kRnaAmbiguousAlphabet;
    QTest::newRow("All alphabets, permit 'any' character, ACGUN") << constants::kStandardAlphabetVector << true << "ACGUN" << constants::kRnaBasicAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ABCDGHKMNRSUVWY") << constants::kStandardAlphabetVector << false << "ABCDGHKMNRSUVWY" << constants::kRnaAmbiguousAlphabet;
    QTest::newRow("All alphabets, permit 'any' character, ABCDGHKMNRSUVWY") << constants::kStandardAlphabetVector << true << "ABCDGHKMNRSUVWY" << constants::kRnaAmbiguousAlphabet;

    QTest::newRow("All alphabets, forbid 'any' character, ACDEFGHIKLMNPQRSTVWY") << constants::kStandardAlphabetVector << false << "ACDEFGHIKLMNPQRSTVWY" << constants::kAminoBasicAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ACDEFGHIKLMNPQRSTVWYX") << constants::kStandardAlphabetVector << false << "ACDEFGHIKLMNPQRSTVWYX" << constants::kAminoAmbiguousAlphabet;
    QTest::newRow("All alphabets, permit 'any' character, ACDEFGHIKLMNPQRSTVWYX") << constants::kStandardAlphabetVector << true << "ACDEFGHIKLMNPQRSTVWYX" << constants::kAminoBasicAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ACDEFGHIKLMNOPQRSTUVWY") << constants::kStandardAlphabetVector << false << "ACDEFGHIKLMNOPQRSTUVWY" << constants::kAminoExtendedAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ACDEFGHIKLMNOPQRSTUVWYX") << constants::kStandardAlphabetVector << false << "ACDEFGHIKLMNOPQRSTUVWYX" << constants::kAminoAmbiguousAlphabet;
    QTest::newRow("All alphabets, permit 'any' character, ACDEFGHIKLMNOPQRSTUVWYX") << constants::kStandardAlphabetVector << true << "ACDEFGHIKLMNOPQRSTUVWYX" << constants::kAminoExtendedAlphabet;
    QTest::newRow("All alphabets, forbid 'any' character, ABCDEFGHIJKLMNOPQRSTUVWXYZ") << constants::kStandardAlphabetVector << false << "ABCDEFGHIJKLMNOPQRSTUVWXYZ" << constants::kAminoAmbiguousAlphabet;
    QTest::newRow("All alphabets, permit 'any' character, ABCDEFGHIJKLMNOPQRSTUVWXYZ") << constants::kStandardAlphabetVector << true << "ABCDEFGHIJKLMNOPQRSTUVWXYZ" << constants::kAminoAmbiguousAlphabet;

    // Spot tests
    QTest::newRow("All alphabets, @") << constants::kStandardAlphabetVector << false << "@" << Alphabet();
    QTest::newRow("Pure gaps, ---") << constants::kStandardAlphabetVector << false << "---..." << Alphabet();
}

void TestAlphabetDetector::detectAlphabet()
{
    QFETCH(AlphabetVector, alphabetVector);
    QFETCH(bool, acceptAnyCharacter);
    QFETCH(QString, string);
    QFETCH(Alphabet, alphabet);

    AlphabetDetector x;
    x.setAcceptAnyCharacter(acceptAnyCharacter);
    x.setAlphabets(alphabetVector);

    QCOMPARE(x.detectAlphabet(string.toAscii()), alphabet);

    if (alphabet != Alphabet())
        QVERIFY(x.validatorForAlphabet(alphabet) != BioStringValidator());
}


QTEST_APPLESS_MAIN(TestAlphabetDetector)
#include "TestAlphabetDetector.moc"
