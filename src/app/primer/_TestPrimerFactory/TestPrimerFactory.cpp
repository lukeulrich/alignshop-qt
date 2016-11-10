/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../PrimerFactory.h"
#include "../Primer.h"
#include "../DimerScoreCalculator.h"
#include "../ThermodynamicCalculator.h"
#include "../../core/BioString.h"
#include "../RestrictionEnzyme.h"

class TestPrimerFactory : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void sodiumConcentration();
    void primerDnaConcentration();
    void makePrimer2Param();
    void makePrimer3Param();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPrimerFactory::constructor()
{
    PrimerFactory primerFactory;

    QCOMPARE(primerFactory.sodiumConcentration(), PrimerFactory::kDefaultSodiumMolarity_);
    QCOMPARE(primerFactory.primerDnaConcentration(), PrimerFactory::kDefaultPrimerDnaMolarity_);

    PrimerFactory primerFactory2(.5, .002);
    QCOMPARE(primerFactory2.sodiumConcentration(), .5);
    QCOMPARE(primerFactory2.primerDnaConcentration(), .002);
}

void TestPrimerFactory::sodiumConcentration()
{
    PrimerFactory primerFactory;

    primerFactory.setSodiumConcentration(2.);
    QCOMPARE(primerFactory.sodiumConcentration(), 2.);
}

void TestPrimerFactory::primerDnaConcentration()
{
    PrimerFactory primerFactory;

    primerFactory.setPrimerDnaConcentration(.01);
    QCOMPARE(primerFactory.primerDnaConcentration(), .01);
}

void TestPrimerFactory::makePrimer2Param()
{
    PrimerFactory primerFactory;
    ThermodynamicCalculator thermoDynamicCalculator;
    DimerScoreCalculator dimerScoreCalculator;
    RestrictionEnzyme re("AatI", "TTATTA", QVector<int>() << 3, QVector<int>() << 3);

    // Test: empty sequence and restriction enzyme
    Primer primer = primerFactory.makePrimer(BioString(eDnaGrammar), RestrictionEnzyme());
    QVERIFY(primer.isNull());
    QCOMPARE(primer.tm(), 0.);
    QCOMPARE(primer.homoDimerScore(), 0.);
    QVERIFY(primer.sequence().isNull());

    // Test: no restriction enzyme
    BioString dnaString("ATGCGTAAACGTA", eDnaGrammar);
    primer = primerFactory.makePrimer(dnaString, RestrictionEnzyme());
    QCOMPARE(primer.coreSequence(), dnaString);
    QVERIFY(primer.restrictionEnzyme().isEmpty());
    QCOMPARE(primer.tm(), thermoDynamicCalculator.meltingTemperature(primer.sequence(), primerFactory.sodiumConcentration(), primerFactory.primerDnaConcentration()));
    QCOMPARE(primer.homoDimerScore(), dimerScoreCalculator.homoDimerScore(dnaString));

    // Test: with restriction enzyme
    primer = primerFactory.makePrimer(dnaString, re);
    QCOMPARE(primer.coreSequence(), dnaString);
    QCOMPARE(primer.restrictionEnzyme(), re);
    QCOMPARE(primer.tm(), thermoDynamicCalculator.meltingTemperature(primer.sequence(), primerFactory.sodiumConcentration(), primerFactory.primerDnaConcentration()));
    QCOMPARE(primer.homoDimerScore(), dimerScoreCalculator.homoDimerScore(BioString("TTATTAATGCGTAAACGTA", eDnaGrammar)));
}

void TestPrimerFactory::makePrimer3Param()
{
    PrimerFactory primerFactory;
    DimerScoreCalculator dimerScoreCalculator;
    RestrictionEnzyme re("AatI", "TTATTA", QVector<int>() << 3, QVector<int>() << 3);

    // Test: empty sequence and restriction enzyme
    Primer primer = primerFactory.makePrimer(BioString(eDnaGrammar), RestrictionEnzyme(), 0.);
    QVERIFY(primer.isNull());
    QCOMPARE(primer.tm(), 0.);
    QCOMPARE(primer.homoDimerScore(), 0.);
    QVERIFY(primer.sequence().isNull());

    // Test: no restriction enzyme, basic tm
    BioString dnaString("ATGCGTAAACGTA", eDnaGrammar);
    primer = primerFactory.makePrimer(dnaString, RestrictionEnzyme(), .35);
    QCOMPARE(primer.coreSequence(), dnaString);
    QVERIFY(primer.restrictionEnzyme().isEmpty());
    QCOMPARE(primer.tm(), .35);
    QCOMPARE(primer.homoDimerScore(), dimerScoreCalculator.homoDimerScore(dnaString));

    // Test: with restriction enzyme
    primer = primerFactory.makePrimer(dnaString, re, .75);
    QCOMPARE(primer.coreSequence(), dnaString);
    QCOMPARE(primer.restrictionEnzyme(), re);
    QCOMPARE(primer.tm(), .75);
    QCOMPARE(primer.homoDimerScore(), dimerScoreCalculator.homoDimerScore(BioString("TTATTAATGCGTAAACGTA", eDnaGrammar)));
}


QTEST_APPLESS_MAIN(TestPrimerFactory)
#include "TestPrimerFactory.moc"
