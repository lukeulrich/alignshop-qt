/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QString>

#include "../PrimerPair.h"
#include "../PrimerFactory.h"

class TestPrimerPair : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void locateForwardPrimerCoreSequenceIn();
    void locateReversePrimerCoreSequenceIn();
    void longestAmpliconLength();
    void name();
    void operator_eqeq();
    void operator_ne();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPrimerPair::constructor()
{
    PrimerPair primerPair;

    QCOMPARE(primerPair.deltaTm(), 0.);
    QVERIFY(primerPair.forwardPrimer().isNull());
    QVERIFY(primerPair.reversePrimer().isNull());
    QVERIFY(primerPair.name().isEmpty());
    QCOMPARE(primerPair.score(), 0.);

    PrimerFactory primerFactory;
    Primer forward = primerFactory.makePrimer(BioString("ATGCGTAGGA", eDnaGrammar), RestrictionEnzyme(), 65.0);
    Primer reverse = primerFactory.makePrimer(BioString("CCGTGAATAA", eDnaGrammar), RestrictionEnzyme(), 68.3);

    primerPair = PrimerPair("Alpha", forward, reverse, 55.0);
    QCOMPARE(primerPair.name(), QString("Alpha"));
    QVERIFY(primerPair.forwardPrimer() == forward);
    QVERIFY(primerPair.reversePrimer() == reverse);
    QCOMPARE(primerPair.deltaTm(), qAbs(65.0 - 68.3));
    QCOMPARE(PrimerPair::deltaTm(forward, reverse), primerPair.deltaTm());
    QCOMPARE(primerPair.score(), 55.0);
}

void TestPrimerPair::locateForwardPrimerCoreSequenceIn()
{
    PrimerFactory primerFactory;
    //                                                   1234567890
    Primer forward = primerFactory.makePrimer(BioString("ATGCGTAGGA", eDnaGrammar), RestrictionEnzyme(), 65.0);
    Primer reverse = primerFactory.makePrimer(BioString("CCGTGAATAA", eDnaGrammar), RestrictionEnzyme(), 68.3);
    PrimerPair primerPair = PrimerPair("Alpha", forward, reverse, 55.0);

    // Test: empty biostring
    QVERIFY(primerPair.locateForwardPrimerCoreSequenceIn(BioString()).isEmpty());
    QVERIFY(primerPair.locateForwardPrimerCoreSequenceIn(BioString(eDnaGrammar)).isEmpty());

    // Test: partial bioString
    QVERIFY(primerPair.locateForwardPrimerCoreSequenceIn(BioString("CAGACGT", eDnaGrammar)).isEmpty());

    // Test: reverse sequence
    QVERIFY(primerPair.locateForwardPrimerCoreSequenceIn(BioString("CCGTGAATAA", eDnaGrammar)).isEmpty());

    // Test: perfect match
    QCOMPARE(primerPair.locateForwardPrimerCoreSequenceIn(BioString("ATGCGTAGGA", eDnaGrammar)), ClosedIntRange(1, 10));

    // Test: offset match
    QCOMPARE(primerPair.locateForwardPrimerCoreSequenceIn(BioString("AGNATGCGTAGGA", eDnaGrammar)), ClosedIntRange(4, 13));

    // Test: only return the first occurrence
    QCOMPARE(primerPair.locateForwardPrimerCoreSequenceIn(BioString("ATGCGTAGGAAGNATGCGTAGGA", eDnaGrammar)), ClosedIntRange(1, 10));
}

void TestPrimerPair::locateReversePrimerCoreSequenceIn()
{
    PrimerFactory primerFactory;
    //                                                   1234567890
    Primer forward = primerFactory.makePrimer(BioString("ATGCGTAGGA", eDnaGrammar), RestrictionEnzyme(), 65.0);
    BioString sensePrimerSequence = BioString("CCGTGAATAA", eDnaGrammar);
    Primer reverse = primerFactory.makePrimer(sensePrimerSequence, RestrictionEnzyme(), 68.3);
    PrimerPair primerPair = PrimerPair("Alpha", forward, reverse, 55.0);

    // Test: empty biostring
    QVERIFY(primerPair.locateReversePrimerCoreSequenceIn(BioString()).isEmpty());
    QVERIFY(primerPair.locateReversePrimerCoreSequenceIn(BioString(eDnaGrammar)).isEmpty());

    // Test: partial bioString
    QVERIFY(primerPair.locateReversePrimerCoreSequenceIn(BioString("CAGACGT", eDnaGrammar)).isEmpty());

    // Test: forward sequence
    QVERIFY(primerPair.locateReversePrimerCoreSequenceIn(BioString("ATGCGTAGGA", eDnaGrammar)).isEmpty());

    // Test: perfect match
    BioString antiSensePrimerSequence = sensePrimerSequence.reverseComplement();
    QCOMPARE(primerPair.locateReversePrimerCoreSequenceIn(antiSensePrimerSequence), ClosedIntRange(1, 10));

    // Test: offset match
    BioString testSequence = "GATN" + antiSensePrimerSequence;
    QCOMPARE(primerPair.locateReversePrimerCoreSequenceIn(testSequence), ClosedIntRange(5, 14));

    // Test: only return the first occurrence - from the right end
    testSequence = antiSensePrimerSequence + "GATN" + antiSensePrimerSequence;
    QCOMPARE(primerPair.locateReversePrimerCoreSequenceIn(testSequence), ClosedIntRange(15, 24));
}

void TestPrimerPair::longestAmpliconLength()
{
    PrimerPair primerPair;

    // Test: empty everything
    QCOMPARE(primerPair.longestAmpliconLength(BioString(eDnaGrammar)), 0);

    // Test: empty primer pair, non-empty BioString
    QCOMPARE(primerPair.longestAmpliconLength(BioString("ATGCGAGTTATA", eDnaGrammar)), 0);

    // Test: valid forward primer, empty reverse, empty BioString
    PrimerFactory primerFactory;
    //                                                   1234567890
    Primer forward = primerFactory.makePrimer(BioString("ATGCGTAGGA", eDnaGrammar), RestrictionEnzyme(), 65.0);
    PrimerPair primerPair2("Alpha", forward, Primer(), 8.3);
    QCOMPARE(primerPair2.longestAmpliconLength(BioString(eDnaGrammar)), 0);

    // Test: valid forward primer, empty reverse, non-matching BioString to forward
    QCOMPARE(primerPair2.longestAmpliconLength(BioString("ATG", eDnaGrammar)), 0);

    // Test: valid forward primer, empty reverse, perfect match BioString to forward
    QCOMPARE(primerPair2.longestAmpliconLength(BioString("ATGCGTAGGA", eDnaGrammar)), 0);

    // Test: empty forward primer, valid reverse, empty BioString
    BioString sensePrimerSequence = BioString("CCGTGAATAA", eDnaGrammar);
    BioString antiSensePrimerSequence = sensePrimerSequence.reverseComplement();
    Primer reverse = primerFactory.makePrimer(sensePrimerSequence, RestrictionEnzyme(), 68.3);
    PrimerPair primerPair3("Beta", Primer(), reverse, 6.7);
    QCOMPARE(primerPair3.longestAmpliconLength(BioString(eDnaGrammar)), 0);

    // Test: empty forward primer, valid reverse, non-empty BioString
    QCOMPARE(primerPair3.longestAmpliconLength(BioString("ATG", eDnaGrammar)), 0);

    // Test: valid forward primer, empty reverse, perfect match BioString to reverse
    QCOMPARE(primerPair3.longestAmpliconLength(antiSensePrimerSequence), 0);

    // Test: valid forward and reverse primers, empty BioString
    PrimerPair primerPair4("Gamma", forward, reverse, 4.5);
    QCOMPARE(primerPair4.longestAmpliconLength(BioString(eDnaGrammar)), 0);

    // Test: valid forward and reverse primers, non-matching BioString
    QCOMPARE(primerPair4.longestAmpliconLength(BioString("ATGCGT", eDnaGrammar)), 0);

    // Test: valid forward and reverse primers, perfect match to forward
    QCOMPARE(primerPair4.longestAmpliconLength(BioString("ATGCGTAGGA", eDnaGrammar)), 0);

    // Test: valid forward and reverse primers, perfect match to reverse
    QCOMPARE(primerPair4.longestAmpliconLength(antiSensePrimerSequence), 0);

    // Test: valid forward and reverse primers, perfect match to forward + reverse
    BioString forwardReverseTestSeq = BioString("ATGCGTAGGA", eDnaGrammar) + antiSensePrimerSequence;
    QCOMPARE(primerPair4.longestAmpliconLength(forwardReverseTestSeq), 20);

    // Test: extra characters at ends and separating primer sequences
    forwardReverseTestSeq.insert(11, "GTG").prepend("CGTGA").append("TGGTG");
    QCOMPARE(primerPair4.longestAmpliconLength(forwardReverseTestSeq), 23);
}

void TestPrimerPair::name()
{
    PrimerPair primerPair;

    QVERIFY(primerPair.name().isEmpty());
    primerPair.setName("Alpha");
    QCOMPARE(primerPair.name(), QString("Alpha"));
}

void TestPrimerPair::operator_eqeq()
{
    PrimerPair primerPair;

    QVERIFY(primerPair == primerPair);

    PrimerFactory primerFactory;
    Primer forward = primerFactory.makePrimer(BioString("ATGCGTAGGA", eDnaGrammar), RestrictionEnzyme(), 65.0);
    Primer reverse = primerFactory.makePrimer(BioString("CCGTGAATAA", eDnaGrammar), RestrictionEnzyme(), 68.3);

    PrimerPair primerPair2 = PrimerPair("Alpha", forward, reverse, 55.0);
    PrimerPair primerPair3 = PrimerPair("Alpha", forward, reverse, 55.0);
    QVERIFY(primerPair2 == primerPair3);
}

void TestPrimerPair::operator_ne()
{
    PrimerPair primerPair;
    PrimerPair primerPair2;
    primerPair2.setName("Beta");

    QVERIFY(primerPair != primerPair2);

    PrimerFactory primerFactory;
    Primer forward = primerFactory.makePrimer(BioString("ATGCGTAGGA", eDnaGrammar), RestrictionEnzyme(), 65.0);
    Primer reverse = primerFactory.makePrimer(BioString("CCGTGAATAA", eDnaGrammar), RestrictionEnzyme(), 68.3);

    primerPair2 = PrimerPair("Alpha", forward, reverse, 55.0);
    QVERIFY(primerPair != primerPair2);
}


QTEST_APPLESS_MAIN(TestPrimerPair)
#include "TestPrimerPair.moc"
