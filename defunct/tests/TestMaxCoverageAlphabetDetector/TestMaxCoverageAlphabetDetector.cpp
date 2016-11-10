/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "MaxCoverageAlphabetDetector.h"

class TestMaxCoverageAlphabetDetector : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setInspectors();
    void detectAlphabet();
};

void TestMaxCoverageAlphabetDetector::setInspectors()
{
    // MaxCoverageAlphabetDetector::setInspectors() should sort the inspectors according to their BioStringValidator
    // valid characters length
    BioStringValidator aminoValidator1(constants::kAminoCharacters);
    BioStringValidator aminoValidator2(constants::kAminoExpandedCharacters);

    QList<AlphabetInspector> inspectors;
    inspectors.append(AlphabetInspector(eAminoAlphabet, aminoValidator1));
    inspectors.append(AlphabetInspector(eAminoAlphabet, aminoValidator2));
    inspectors.append(AlphabetInspector(eDnaAlphabet, BioStringValidator(constants::kDnaCharacters)));

    MaxCoverageAlphabetDetector detector;
    detector.setInspectors(inspectors);

    QList<AlphabetInspector> sorted_inspectors = detector.inspectors();
    QVERIFY(sorted_inspectors.size() == 3);
    QCOMPARE(sorted_inspectors.at(0).alphabet(), eDnaAlphabet);
    QCOMPARE(sorted_inspectors.at(1).alphabet(), eAminoAlphabet);
    QVERIFY(sorted_inspectors.at(1).validator() == aminoValidator1);
    QVERIFY(sorted_inspectors.at(2).validator() == aminoValidator2);
}

void TestMaxCoverageAlphabetDetector::detectAlphabet()
{
    BioStringValidator dnaValidator(constants::kDnaExpandedCharacters);
    BioStringValidator rnaValidator(constants::kRnaCharacters);

    QList<AlphabetInspector> inspectors;
    inspectors.append(AlphabetInspector(eDnaAlphabet, dnaValidator));
    inspectors.append(AlphabetInspector(eRnaAlphabet, rnaValidator));

    MaxCoverageAlphabetDetector detector;
    detector.setInspectors(inspectors);

    // Suite: clear-cut cases regardless of ordering
    // Test a) DNA sequence, contains a T, so therefore it cannot be a RNA sequence
    BioString test_dna = "ACGT";
    QCOMPARE(detector.detectAlphabet(test_dna), eDnaAlphabet);

    // Test b) RNA sequence, contains a U so therefore it cannot be a DNA sequence
    BioString test_rna = "ACGU";
    QCOMPARE(detector.detectAlphabet(test_rna), eRnaAlphabet);

    // Test: ambiguous case - could be either Dna or Rna string; however, since it has a higher proportion of the
    //       RNA alphabet (relative to the inspectors under consideration and BioStringValidator alphabets), it
    //       should return eRnaAlphabet
    BioString test_ambiguous = "ACGCAAG";
    QCOMPARE(detector.detectAlphabet(test_ambiguous), eRnaAlphabet);

    // This time with Dna, Rna, and Amino alphabets
    BioStringValidator aminoValidator(constants::kAminoCharacters);
    inspectors.append(AlphabetInspector(eAminoAlphabet, aminoValidator));
    detector.setInspectors(inspectors);

    // Test: Amino acid sequence
    BioString test_amino = "ACDEFGHIKLM";
    QCOMPARE(detector.detectAlphabet(test_amino), eAminoAlphabet);

    // Test: garbage sequence
    BioString test_garbage = "123ABC&*(";
    QCOMPARE(detector.detectAlphabet(test_garbage), eUnknownAlphabet);
}

QTEST_MAIN(TestMaxCoverageAlphabetDetector)
#include "TestMaxCoverageAlphabetDetector.moc"
