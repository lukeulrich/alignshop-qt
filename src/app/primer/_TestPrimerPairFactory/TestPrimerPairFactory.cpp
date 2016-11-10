/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../DimerScoreCalculator.h"
#include "../Primer.h"
#include "../PrimerPair.h"
#include "../PrimerFactory.h"
#include "../PrimerPairFactory.h"

class TestPrimerPairFactory : public QObject
{
    Q_OBJECT

private slots:
    void makePrimerPair();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPrimerPairFactory::makePrimerPair()
{
    PrimerPairFactory primerPairFactory;

    PrimerPair primerPair = primerPairFactory.makePrimerPair(Primer(), Primer());
    QCOMPARE(primerPair.deltaTm(), 0.);
    QVERIFY(primerPair.forwardPrimer().isNull());
    QVERIFY(primerPair.reversePrimer().isNull());
    QVERIFY(primerPair.name().isEmpty());
    QCOMPARE(primerPair.score(), 0.);

    primerPair = primerPairFactory.makePrimerPair("Alpha", Primer(), Primer());
    QCOMPARE(primerPair.deltaTm(), 0.);
    QVERIFY(primerPair.forwardPrimer().isNull());
    QVERIFY(primerPair.reversePrimer().isNull());
    QCOMPARE(primerPair.name(), QString("Alpha"));
    QCOMPARE(primerPair.score(), 0.);

    // Test: make some real primers
    DimerScoreCalculator dimerScoreCalculator;
    PrimerFactory primerFactory;
    Primer forwardPrimer = primerFactory.makePrimer(BioString("ATGCAGTA", eDnaGrammar), RestrictionEnzyme(), 45.);
    primerPair = primerPairFactory.makePrimerPair("Beta", forwardPrimer, Primer());
    QCOMPARE(primerPair.name(), QString("Beta"));
    QCOMPARE(primerPair.forwardPrimer(), forwardPrimer);
    QCOMPARE(primerPair.reversePrimer(), Primer());
    QCOMPARE(primerPair.deltaTm(), 45.0);
    QCOMPARE(primerPair.score(), dimerScoreCalculator.dimerScore(forwardPrimer.sequence(), BioString(eDnaGrammar)));

    Primer reversePrimer = primerFactory.makePrimer(BioString("GCTGTAAGC", eDnaGrammar), RestrictionEnzyme(), 53.2);
    primerPair = primerPairFactory.makePrimerPair("Gamma", forwardPrimer, reversePrimer);
    QCOMPARE(primerPair.name(), QString("Gamma"));
    QCOMPARE(primerPair.forwardPrimer(), forwardPrimer);
    QCOMPARE(primerPair.reversePrimer(), reversePrimer);
    QCOMPARE(primerPair.deltaTm(), qAbs(45. - 53.2));
    QCOMPARE(primerPair.score(), dimerScoreCalculator.dimerScore(forwardPrimer.sequence(), reversePrimer.sequence()));
}


QTEST_APPLESS_MAIN(TestPrimerPairFactory)
#include "TestPrimerPairFactory.moc"
