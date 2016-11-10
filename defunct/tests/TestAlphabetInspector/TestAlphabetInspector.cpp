/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "AlphabetInspector.h"
#include "global.h"

class TestAlphabetInspector : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void operator_eqeq();
    void alphabet();            // also tests set alphabet
    void inspect();
    void validator();
};

void TestAlphabetInspector::constructor()
{
    AlphabetInspector inspector(eAminoAlphabet, BioStringValidator(constants::kAminoCharacters));
}

void TestAlphabetInspector::operator_eqeq()
{
    AlphabetInspector aminoInspector(eAminoAlphabet, BioStringValidator(constants::kAminoCharacters));
    QVERIFY(aminoInspector == aminoInspector);

    AlphabetInspector aminoInspector2(eAminoAlphabet, BioStringValidator(constants::kAminoCharacters));
    QVERIFY(aminoInspector == aminoInspector2);
}

void TestAlphabetInspector::alphabet()
{
    AlphabetInspector aminoInspector(eAminoAlphabet, BioStringValidator(constants::kAminoCharacters));
    QCOMPARE(aminoInspector.alphabet(), eAminoAlphabet);

    AlphabetInspector dnaInspector(eDnaAlphabet, BioStringValidator(constants::kDnaCharacters));
    QCOMPARE(dnaInspector.alphabet(), eDnaAlphabet);
}

void TestAlphabetInspector::inspect()
{
    AlphabetInspector aminoInspector(eAminoAlphabet, BioStringValidator(constants::kAminoCharacters));

    // Test: valid amino string
    BioString bs("ACGPPR");
    QCOMPARE(aminoInspector.inspect(bs), eAminoAlphabet);

    // Test: invalid amino string
    bs = "123ABC";
    QCOMPARE(aminoInspector.inspect(bs), eUnknownAlphabet);
}

void TestAlphabetInspector::validator()
{
    BioStringValidator aminoValidator(constants::kAminoCharacters);
    AlphabetInspector aminoInspector(eAminoAlphabet, aminoValidator);
    QVERIFY(aminoInspector.validator() == aminoValidator);

    BioStringValidator dnaValidator(constants::kDnaCharacters);
    AlphabetInspector dnaInspector(eDnaAlphabet, dnaValidator);
    QVERIFY(dnaInspector.validator() == dnaValidator);
}

QTEST_MAIN(TestAlphabetInspector)
#include "TestAlphabetInspector.moc"
