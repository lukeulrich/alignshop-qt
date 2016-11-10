/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <cmath>
#include <QtCore/QString>
#include <QtTest/QtTest>

#include "../ThermodynamicCalculator.h"
#include "../ThermodynamicConstants.h"
#include "../../core/BioString.h"

class TestThermodynamicCalculator : public QObject
{
    Q_OBJECT

private slots:
    void enthalpy1();
    void enthalpy2();
    void enthalpyBioString();

    void entropy1();
    void entropy2();
    void entropyBioString();

    void meltingTemperatureBioString();
    void meltingTemperatureFromValues();

    void sodiumCorrectedEntropy();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestThermodynamicCalculator::enthalpy1()
{
    ThermodynamicCalculator x;
    QCOMPARE(x.enthalpy('A'), 2.3);
    QCOMPARE(x.enthalpy('C'), 0.1);
    QCOMPARE(x.enthalpy('G'), 0.1);
    QCOMPARE(x.enthalpy('T'), 2.3);
}

void TestThermodynamicCalculator::enthalpy2()
{
    ThermodynamicCalculator x;
    QCOMPARE(x.enthalpy('A', 'A'), -7.9);
    QCOMPARE(x.enthalpy('A', 'C'), -8.4);
    QCOMPARE(x.enthalpy('A', 'G'), -7.8);
    QCOMPARE(x.enthalpy('A', 'T'), -7.2);

    QCOMPARE(x.enthalpy('C', 'A'), -8.5);
    QCOMPARE(x.enthalpy('C', 'C'), -8.0);
    QCOMPARE(x.enthalpy('C', 'G'), -10.6);
    QCOMPARE(x.enthalpy('C', 'T'), -7.8);

    QCOMPARE(x.enthalpy('G', 'A'), -8.2);
    QCOMPARE(x.enthalpy('G', 'C'), -9.8);
    QCOMPARE(x.enthalpy('G', 'G'), -8.0);
    QCOMPARE(x.enthalpy('G', 'T'), -8.4);

    QCOMPARE(x.enthalpy('T', 'A'), -7.2);
    QCOMPARE(x.enthalpy('T', 'C'), -8.2);
    QCOMPARE(x.enthalpy('T', 'G'), -8.5);
    QCOMPARE(x.enthalpy('T', 'T'), -7.9);
}

void TestThermodynamicCalculator::enthalpyBioString()
{
    ThermodynamicCalculator x;

    // Test: single character BioStrings
    QCOMPARE(x.enthalpy(BioString("A", eDnaGrammar)), x.enthalpy('A'));
    QCOMPARE(x.enthalpy(BioString("C", eDnaGrammar)), x.enthalpy('C'));
    QCOMPARE(x.enthalpy(BioString("G", eDnaGrammar)), x.enthalpy('G'));
    QCOMPARE(x.enthalpy(BioString("T", eDnaGrammar)), x.enthalpy('T'));

    // Test: dual string characters
    QCOMPARE(x.enthalpy(BioString("AT", eDnaGrammar)), x.enthalpy('A') + x.enthalpy('A', 'T') + x.enthalpy('T') + constants::kEnthalpySymmetryCorrection);
    QCOMPARE(x.enthalpy(BioString("CG", eDnaGrammar)), x.enthalpy('C') + x.enthalpy('C', 'G') + x.enthalpy('G') + constants::kEnthalpySymmetryCorrection);
    QCOMPARE(x.enthalpy(BioString("GC", eDnaGrammar)), x.enthalpy('G') + x.enthalpy('G', 'C') + x.enthalpy('C') + constants::kEnthalpySymmetryCorrection);
    QCOMPARE(x.enthalpy(BioString("TA", eDnaGrammar)), x.enthalpy('T') + x.enthalpy('T', 'A') + x.enthalpy('A') + constants::kEnthalpySymmetryCorrection);

    QCOMPARE(x.enthalpy(BioString("TT", eDnaGrammar)), x.enthalpy('T') + x.enthalpy('T', 'T') + x.enthalpy('T'));
    QCOMPARE(x.enthalpy(BioString("GA", eDnaGrammar)), x.enthalpy('G') + x.enthalpy('G', 'A') + x.enthalpy('A'));
    QCOMPARE(x.enthalpy(BioString("AG", eDnaGrammar)), x.enthalpy('A') + x.enthalpy('A', 'G') + x.enthalpy('G'));

    // Test: longer strings
    QCOMPARE(x.enthalpy(BioString(eDnaGrammar)), 0.);
    QCOMPARE(x.enthalpy(BioString("ATCG", eDnaGrammar)),
             x.enthalpy('A') +
             x.enthalpy('A', 'T') +
             x.enthalpy('T', 'C') +
             x.enthalpy('C', 'G') +
             x.enthalpy('G'));

    QCOMPARE(x.enthalpy(BioString("AGCGGATAA", eDnaGrammar)),
             x.enthalpy('A') +
             x.enthalpy('A', 'G') +
             x.enthalpy('G', 'C') +
             x.enthalpy('C', 'G') +
             x.enthalpy('G', 'G') +
             x.enthalpy('G', 'A') +
             x.enthalpy('A', 'T') +
             x.enthalpy('T', 'A') +
             x.enthalpy('A', 'A') +
             x.enthalpy('A'));

    // Test: symmetrical longer string
    QCOMPARE(x.enthalpy(BioString("GAATTC", eDnaGrammar)),
             x.enthalpy('G') +
             x.enthalpy('G', 'A') +
             x.enthalpy('A', 'A') +
             x.enthalpy('A', 'T') +
             x.enthalpy('T', 'T') +
             x.enthalpy('T', 'C') +
             x.enthalpy('C') +
             constants::kEnthalpySymmetryCorrection);
}

void TestThermodynamicCalculator::entropy1()
{
    ThermodynamicCalculator x;
    QCOMPARE(x.entropy('A'), 4.1);
    QCOMPARE(x.entropy('C'), -2.8);
    QCOMPARE(x.entropy('G'), -2.8);
    QCOMPARE(x.entropy('T'), 4.1);
}

void TestThermodynamicCalculator::entropy2()
{
    ThermodynamicCalculator x;
    QCOMPARE(x.entropy('A', 'A'), -22.2);
    QCOMPARE(x.entropy('A', 'C'), -22.4);
    QCOMPARE(x.entropy('A', 'G'), -21.0);
    QCOMPARE(x.entropy('A', 'T'), -20.4);

    QCOMPARE(x.entropy('C', 'A'), -22.7);
    QCOMPARE(x.entropy('C', 'C'), -19.9);
    QCOMPARE(x.entropy('C', 'G'), -27.2);
    QCOMPARE(x.entropy('C', 'T'), -21.0);

    QCOMPARE(x.entropy('G', 'A'), -22.2);
    QCOMPARE(x.entropy('G', 'C'), -24.4);
    QCOMPARE(x.entropy('G', 'G'), -19.9);
    QCOMPARE(x.entropy('G', 'T'), -22.4);

    QCOMPARE(x.entropy('T', 'A'), -21.3);
    QCOMPARE(x.entropy('T', 'C'), -22.2);
    QCOMPARE(x.entropy('T', 'G'), -22.7);
    QCOMPARE(x.entropy('T', 'T'), -22.2);
}

void TestThermodynamicCalculator::entropyBioString()
{
    ThermodynamicCalculator x;

    // Test: single character BioStrings
    QCOMPARE(x.entropy(BioString("A", eDnaGrammar)), x.entropy('A'));
    QCOMPARE(x.entropy(BioString("C", eDnaGrammar)), x.entropy('C'));
    QCOMPARE(x.entropy(BioString("G", eDnaGrammar)), x.entropy('G'));
    QCOMPARE(x.entropy(BioString("T", eDnaGrammar)), x.entropy('T'));

    // Test: dual string characters
    QCOMPARE(x.entropy(BioString("AT", eDnaGrammar)), x.entropy('A') + x.entropy('A', 'T') + x.entropy('T') + constants::kEntropySymmetryCorrection);
    QCOMPARE(x.entropy(BioString("CG", eDnaGrammar)), x.entropy('C') + x.entropy('C', 'G') + x.entropy('G') + constants::kEntropySymmetryCorrection);
    QCOMPARE(x.entropy(BioString("GC", eDnaGrammar)), x.entropy('G') + x.entropy('G', 'C') + x.entropy('C') + constants::kEntropySymmetryCorrection);
    QCOMPARE(x.entropy(BioString("TA", eDnaGrammar)), x.entropy('T') + x.entropy('T', 'A') + x.entropy('A') + constants::kEntropySymmetryCorrection);

    QCOMPARE(x.entropy(BioString("TT", eDnaGrammar)), x.entropy('T') + x.entropy('T', 'T') + x.entropy('T'));
    QCOMPARE(x.entropy(BioString("GA", eDnaGrammar)), x.entropy('G') + x.entropy('G', 'A') + x.entropy('A'));
    QCOMPARE(x.entropy(BioString("AG", eDnaGrammar)), x.entropy('A') + x.entropy('A', 'G') + x.entropy('G'));

    // Test: longer strings
    QCOMPARE(x.entropy(BioString(eDnaGrammar)), 0.);
    QCOMPARE(x.entropy(BioString("ATCG", eDnaGrammar)),
             x.entropy('A') +
             x.entropy('A', 'T') +
             x.entropy('T', 'C') +
             x.entropy('C', 'G') +
             x.entropy('G'));

    QCOMPARE(x.entropy(BioString("AGCGGATAA", eDnaGrammar)),
             x.entropy('A') +
             x.entropy('A', 'G') +
             x.entropy('G', 'C') +
             x.entropy('C', 'G') +
             x.entropy('G', 'G') +
             x.entropy('G', 'A') +
             x.entropy('A', 'T') +
             x.entropy('T', 'A') +
             x.entropy('A', 'A') +
             x.entropy('A'));

    // Test: symmetrical longer string
    QCOMPARE(x.entropy(BioString("GAATTC", eDnaGrammar)),
             x.entropy('G') +
             x.entropy('G', 'A') +
             x.entropy('A', 'A') +
             x.entropy('A', 'T') +
             x.entropy('T', 'T') +
             x.entropy('T', 'C') +
             x.entropy('C') +
             constants::kEntropySymmetryCorrection);
}

void TestThermodynamicCalculator::meltingTemperatureBioString()
{
    ThermodynamicCalculator x;

    for (double naConc = 0.5; naConc < 6.7; naConc += .470)
    {
        for (double primerConc = 0.000001; primerConc < 0.0053; primerConc += .0000043)
        {
            BioString dnaString(eDnaGrammar);
            QCOMPARE(x.meltingTemperature(dnaString, naConc, primerConc), 0.);

            QStringList list;
            list << "AT" << "GCGC" << "TTT" << "ACGCGTCAG";
            foreach (const QString &string, list)
            {
                dnaString = string.toAscii();
                QCOMPARE(x.meltingTemperature(dnaString, naConc, primerConc),
                         x.meltingTemperature(x.enthalpy(dnaString),
                                              x.sodiumCorrectedEntropy(x.entropy(dnaString), dnaString.length(), naConc),
                                              primerConc,
                                              dnaString.isPalindrome()));
            }
        }
    }
}

void TestThermodynamicCalculator::meltingTemperatureFromValues()
{
    ThermodynamicCalculator x;

    for (double enthalpy = -40.; enthalpy < 50.; enthalpy += 4.5)
    {
        for (double sodiumCorrectedEntropy = -123.5; sodiumCorrectedEntropy < -68; sodiumCorrectedEntropy += .823)
        {
            for (double primerConc = .000001; primerConc < .0001; primerConc += .00008)
            {
                // First case: no palindrome
                QCOMPARE(x.meltingTemperature(enthalpy, sodiumCorrectedEntropy, primerConc, false),
                         1000. * enthalpy / (sodiumCorrectedEntropy + constants::kR * log(primerConc / 2.)) - 273.15);

                // Second case: with palindrome
                QCOMPARE(x.meltingTemperature(enthalpy, sodiumCorrectedEntropy, primerConc, true),
                         1000. * enthalpy / (sodiumCorrectedEntropy + constants::kR * log(primerConc)) - 273.15);
            }
        }
    }
}

void TestThermodynamicCalculator::sodiumCorrectedEntropy()
{
    ThermodynamicCalculator x;

    for (double entropy = -50.; entropy< 100.; entropy += 4.5)
        for (int n = 1; n< 9; ++n)
            for (double naConc = 0.0003; naConc< 5.; naConc += .93)
                QCOMPARE(x.sodiumCorrectedEntropy(entropy, n, naConc), entropy + 0.368 * (n - 1) * log(naConc));
}


QTEST_APPLESS_MAIN(TestThermodynamicCalculator)
#include "TestThermodynamicCalculator.moc"
