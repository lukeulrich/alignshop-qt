/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../Primer.h"
#include "../RestrictionEnzyme.h"
#include "../../core/BioString.h"

class TestPrimer : public QObject
{
    Q_OBJECT

private slots:
    void constructor();
    void operator_eqeq();
    void operator_ne();

    void coreSequence();
    void countCoreSequenceForwardMatchesIn();
    void countCoreSequenceMatchesIn();
    void countCoreSequenceReverseMatchesIn();
    void coreSequenceForwardLocationsIn();
    void coreSequenceReverseLocationsIn();
    void homoDimerScore();
    void isNull();
    void locateCoreSequenceIn();
    void locateCoreSequenceStartIn();
    void locateCoreSequenceStopIn();
    void locateCoreSequenceInCognateStrand();
    void locateCoreSequenceStartInCognateStrand();
    void locateCoreSequenceStopInCognateStrand();
    void restrictionEnzyme();
    void sequence();
    void staticSequence();      // Tests static version of sequence
    void tm();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPrimer::constructor()
{
    Primer primer;

    QCOMPARE(primer.coreSequence().grammar(), eDnaGrammar);
    QVERIFY(primer.coreSequence().isEmpty());
    QVERIFY(primer.restrictionEnzyme().isEmpty());
    QVERIFY(primer.sequence().isEmpty());
    QCOMPARE(primer.isNull(), true);
    QCOMPARE(primer.tm(), 0.);
    QCOMPARE(primer.homoDimerScore(), 0.);
}

void TestPrimer::operator_eqeq()
{
    {
        Primer primer;
        QVERIFY(primer == Primer());
    }

    {
        Primer primer(BioString("ABCD", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>()), 5., 34.);
        QVERIFY(primer == primer);
    }
}

void TestPrimer::operator_ne()
{
    {
        Primer primer(BioString("ABCD", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>()), 5., 34.);
        Primer primer2(BioString("ABCDe", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>()), 5., 34.);
        QVERIFY(primer != primer2);
    }
}

void TestPrimer::coreSequence()
{
    {
        Primer primer(BioString("ABCD", eDnaGrammar), RestrictionEnzyme(), 0., 0.);
        QCOMPARE(primer.coreSequence(), BioString("ABCD", eDnaGrammar));
    }

    // Test: core sequence should only return the core sequence even when a restriction
    // enzyme has been added
    {
        Primer primer(BioString("ABCD", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>()), 0., 0.);
        QCOMPARE(primer.coreSequence(), BioString("ABCD", eDnaGrammar));
    }
}

void TestPrimer::countCoreSequenceForwardMatchesIn()
{
    BioString emptyDnaString(eDnaGrammar);
    BioString dnaString("ATGTTGCAAGTATG", eDnaGrammar);
    // Reverse complement: CATACTTGCAACAT
    BioString rcDnaString = dnaString.reverseComplement();
    Primer emptyPrimer;
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>(), QVector<int>()), 0., 0.);

    QCOMPARE(emptyPrimer.countCoreSequenceForwardMatchesIn(emptyDnaString), 0);
    QCOMPARE(emptyPrimer.countCoreSequenceForwardMatchesIn(dnaString), 0);
    QCOMPARE(primer.countCoreSequenceForwardMatchesIn(emptyDnaString), 0);
    QCOMPARE(emptyPrimer.countCoreSequenceForwardMatchesIn(rcDnaString), 0);

    // Test: simple non-overlapping matches
    QCOMPARE(primer.countCoreSequenceForwardMatchesIn(dnaString), 2);
    QCOMPARE(primer.countCoreSequenceForwardMatchesIn(rcDnaString), 0);

    // Test: overlapping matches
    Primer primer2(BioString("ATGATG", eDnaGrammar), RestrictionEnzyme(), 0., 0.);
    dnaString = "ATGATGATGCGTAATGATG";
    QCOMPARE(primer2.countCoreSequenceForwardMatchesIn(dnaString), 3);
}

void TestPrimer::countCoreSequenceMatchesIn()
{
    BioString emptyDnaString(eDnaGrammar);
    BioString dnaString("ATGTTGCATAGTATG", eDnaGrammar);
    // Reverse complement: CATACTATGCAACAT
    BioString rcDnaString = dnaString.reverseComplement();
    Primer emptyPrimer;
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>(), QVector<int>()), 0., 0.);

    QCOMPARE(emptyPrimer.countCoreSequenceMatchesIn(emptyDnaString), 0);
    QCOMPARE(emptyPrimer.countCoreSequenceMatchesIn(dnaString), 0);
    QCOMPARE(primer.countCoreSequenceMatchesIn(emptyDnaString), 0);

    // Test: simple non-overlapping test
    QCOMPARE(primer.countCoreSequenceMatchesIn(dnaString), 3);  // 2 in the forward strand, 1 one in the reverse
    QCOMPARE(primer.countCoreSequenceMatchesIn(rcDnaString), 3);  // 2 in the reverse strand, 1 one in the forward

    // Test: overlapping matches
    Primer primer2(BioString("CATCAT", eDnaGrammar), RestrictionEnzyme(), 0., 0.);
    dnaString = "ATGATGATGCGTAATGATG";
    QCOMPARE(primer2.countCoreSequenceMatchesIn(dnaString), 3);
}

void TestPrimer::countCoreSequenceReverseMatchesIn()
{
    BioString emptyDnaString(eDnaGrammar);
    BioString dnaString("ATGTTGCAAGTATG", eDnaGrammar);
    // Reverse complement: CATACTTGCAACAT
    BioString rcDnaString = dnaString.reverseComplement();
    Primer emptyPrimer;
    Primer primer(BioString("CAT", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>(), QVector<int>()), 0., 0.);

    QCOMPARE(emptyPrimer.countCoreSequenceReverseMatchesIn(emptyDnaString), 0);
    QCOMPARE(emptyPrimer.countCoreSequenceReverseMatchesIn(dnaString), 0);
    QCOMPARE(primer.countCoreSequenceReverseMatchesIn(emptyDnaString), 0);
    QCOMPARE(emptyPrimer.countCoreSequenceReverseMatchesIn(rcDnaString), 0);

    // Test: simple non-overlapping matches
    QCOMPARE(primer.countCoreSequenceReverseMatchesIn(dnaString), 2);
    QCOMPARE(primer.countCoreSequenceReverseMatchesIn(rcDnaString), 0);

    // Test: overlapping matches
    Primer primer2(BioString("CATCAT", eDnaGrammar), RestrictionEnzyme(), 0., 0.);
    dnaString = "ATGATGATGCGTAATGATG";
    QCOMPARE(primer2.countCoreSequenceReverseMatchesIn(dnaString), 3);
}

void TestPrimer::coreSequenceForwardLocationsIn()
{
    BioString emptyDnaString(eDnaGrammar);
    //                   12345678901234
    BioString dnaString("ATGTTGCAAGTATG", eDnaGrammar);
    // Reverse complement: CATACTTGCAACAT
    BioString rcDnaString = dnaString.reverseComplement();
    Primer emptyPrimer;
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>(), QVector<int>()), 0., 0.);

    QVERIFY(emptyPrimer.coreSequenceForwardLocationsIn(emptyDnaString).isEmpty());
    QVERIFY(emptyPrimer.coreSequenceForwardLocationsIn(dnaString).isEmpty());
    QVERIFY(primer.coreSequenceForwardLocationsIn(emptyDnaString).isEmpty());

    // Test: simple non-overlapping test
    QVector<ClosedIntRange> locations = primer.coreSequenceForwardLocationsIn(dnaString);
    QCOMPARE(locations.size(), 2);
    QCOMPARE(locations.at(0), ClosedIntRange(1, 3));
    QCOMPARE(locations.at(1), ClosedIntRange(12, 14));
    QVERIFY(primer.coreSequenceForwardLocationsIn(rcDnaString).isEmpty());

    // Test: overlapping matches
    Primer primer2(BioString("ATGATG", eDnaGrammar), RestrictionEnzyme(), 0., 0.);
    //           1234567890123456789
    dnaString = "ATGATGATGCGTAATGATG";
    locations = primer2.coreSequenceForwardLocationsIn(dnaString);
    QCOMPARE(locations.size(), 3);
    QCOMPARE(locations.at(0), ClosedIntRange(1, 6));
    QCOMPARE(locations.at(1), ClosedIntRange(4, 9));
    QCOMPARE(locations.at(2), ClosedIntRange(14, 19));
}

void TestPrimer::coreSequenceReverseLocationsIn()
{
    BioString emptyDnaString(eDnaGrammar);
    BioString dnaString("ATGTTGCAAGTATG", eDnaGrammar);
    // Reverse complement: CATACTTGCAACAT
    //                     12345678901234
    BioString rcDnaString = dnaString.reverseComplement();
    Primer emptyPrimer;
    Primer primer(BioString("CAT", eDnaGrammar), RestrictionEnzyme("AanI", "TTATTA", QVector<int>(), QVector<int>()), 0., 0.);

    QVERIFY(emptyPrimer.coreSequenceReverseLocationsIn(emptyDnaString).isEmpty());
    QVERIFY(emptyPrimer.coreSequenceReverseLocationsIn(dnaString).isEmpty());
    QVERIFY(primer.coreSequenceReverseLocationsIn(emptyDnaString).isEmpty());

    // Test: simple non-overlapping matches
    QVector<ClosedIntRange> locations = primer.coreSequenceReverseLocationsIn(dnaString);
    QCOMPARE(locations.size(), 2);
    QCOMPARE(locations.at(0), ClosedIntRange(1, 3));
    QCOMPARE(locations.at(1), ClosedIntRange(12, 14));
    QVERIFY(primer.coreSequenceReverseLocationsIn(rcDnaString).isEmpty());

    // Test: overlapping matches
    Primer primer2(BioString("CATCAT", eDnaGrammar), RestrictionEnzyme(), 0., 0.);
    dnaString = "ATGATGATGCGTAATGATG";
    // RC =      CATCATTACGCATCATCAT
    //           1234567890123456789
    locations = primer2.coreSequenceReverseLocationsIn(dnaString);
    QCOMPARE(locations.size(), 3);
    QCOMPARE(locations.at(0), ClosedIntRange(1, 6));
    QCOMPARE(locations.at(1), ClosedIntRange(4, 9));
    QCOMPARE(locations.at(2), ClosedIntRange(14, 19));
}

void TestPrimer::homoDimerScore()
{
    Primer primer(BioString("ABCD", eDnaGrammar), RestrictionEnzyme(), 0., 36.7);

    QCOMPARE(primer.homoDimerScore(), 36.7);
}

void TestPrimer::isNull()
{
    // Test: Empty biostring for core sequence
    {
        Primer primer(BioString(eDnaGrammar), RestrictionEnzyme(), 0., 0.);
        QCOMPARE(primer.isNull(), true);
    }

    // Test: non-empty core biostring sequence
    {
        Primer primer(BioString("ABCD", eDnaGrammar), RestrictionEnzyme(), 0., 0.);
        QCOMPARE(primer.isNull(), false);
    }
}

void TestPrimer::locateCoreSequenceIn()
{
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme(), 0., 0.);

    BioString bioString;
    QVERIFY(primer.locateCoreSequenceIn(bioString).isEmpty());
    bioString = "AT";
    QVERIFY(primer.locateCoreSequenceIn(bioString).isEmpty());
    bioString += "G";
    QCOMPARE(primer.locateCoreSequenceIn(bioString), ClosedIntRange(1, 3));

    //           1234567890
    bioString = "ACAGATGACG";
    QCOMPARE(primer.locateCoreSequenceIn(bioString), ClosedIntRange(5, 7));

    // Test: should return the first range that it appears in
    bioString += "ATG";
    QCOMPARE(primer.locateCoreSequenceIn(bioString), ClosedIntRange(5, 7));

    // Test: check that restriction enzyme does not change things
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());
    Primer primer2(BioString("ATG", eDnaGrammar), re, 0., 0.);
    QCOMPARE(primer2.locateCoreSequenceIn(bioString), ClosedIntRange(5, 7));

    // Test: empty core sequence
    Primer primer3;
    QVERIFY(primer3.locateCoreSequenceIn(bioString).isEmpty());
}

void TestPrimer::locateCoreSequenceStartIn()
{
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme(), 0., 0.);

    BioString bioString;
    QCOMPARE(primer.locateCoreSequenceStartIn(bioString), -1);
    bioString = "AT";
    QCOMPARE(primer.locateCoreSequenceStartIn(bioString), -1);
    bioString += "G";
    QCOMPARE(primer.locateCoreSequenceStartIn(bioString), 1);

    //           1234567890
    bioString = "ACAGATGACG";
    QCOMPARE(primer.locateCoreSequenceStartIn(bioString), 5);

    // Test: should return the first range that it appears in
    bioString += "ATG";
    QCOMPARE(primer.locateCoreSequenceStartIn(bioString), 5);

    // Test: check that restriction enzyme does not change things
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());
    Primer primer2(BioString("ATG", eDnaGrammar), re, 0., 0.);
    QCOMPARE(primer2.locateCoreSequenceStartIn(bioString), 5);

    // Test: empty core sequence
    Primer primer3;
    QCOMPARE(primer3.locateCoreSequenceStartIn(bioString), -1);
}

void TestPrimer::locateCoreSequenceStopIn()
{
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme(), 0., 0.);

    BioString bioString;
    QCOMPARE(primer.locateCoreSequenceStopIn(bioString), -1);
    bioString = "AT";
    QCOMPARE(primer.locateCoreSequenceStopIn(bioString), -1);
    bioString += "G";
    QCOMPARE(primer.locateCoreSequenceStopIn(bioString), 3);

    //           1234567890
    bioString = "ACAGATGACG";
    QCOMPARE(primer.locateCoreSequenceStopIn(bioString), 7);

    // Test: should return the first range that it appears in
    bioString += "ATG";
    QCOMPARE(primer.locateCoreSequenceStopIn(bioString), 7);

    // Test: check that restriction enzyme does not change things
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());
    Primer primer2(BioString("ATG", eDnaGrammar), re, 0., 0.);
    QCOMPARE(primer2.locateCoreSequenceStopIn(bioString), 7);

    // Test: empty core sequence
    Primer primer3;
    QCOMPARE(primer3.locateCoreSequenceStopIn(bioString), -1);
}

void TestPrimer::locateCoreSequenceInCognateStrand()
{
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme(), 0., 0.);

    BioString bioString;
    QVERIFY(primer.locateCoreSequenceInCognateStrand(bioString).isEmpty());
    bioString = "AT";
    QVERIFY(primer.locateCoreSequenceInCognateStrand(bioString).isEmpty());
    bioString += "G";
    QVERIFY(primer.locateCoreSequenceInCognateStrand(bioString).isEmpty());

    // Test: perfect match
    QCOMPARE(primer.locateCoreSequenceInCognateStrand(BioString("CAT", eDnaGrammar)), ClosedIntRange(1, 3));

    //           1234567890
    bioString = "ACAGATGACG";
    QVERIFY(primer.locateCoreSequenceInCognateStrand(bioString).isEmpty());

    bioString = "ACACATGACG";
    QCOMPARE(primer.locateCoreSequenceInCognateStrand(bioString), ClosedIntRange(4, 6));

    // Test: should return the first range that it appears in - note: this is from the rightmost end of the sequence
    bioString += "CAT";
    QCOMPARE(primer.locateCoreSequenceInCognateStrand(bioString), ClosedIntRange(11, 13));

    // Test: check that restriction enzyme does not change things
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());
    Primer primer2(BioString("ATG", eDnaGrammar), re, 0., 0.);
    QCOMPARE(primer2.locateCoreSequenceInCognateStrand(bioString), ClosedIntRange(11, 13));

    // Test: empty core sequence
    Primer primer3;
    QVERIFY(primer3.locateCoreSequenceInCognateStrand(bioString).isEmpty());
}

void TestPrimer::locateCoreSequenceStartInCognateStrand()
{
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme(), 0., 0.);

    BioString bioString;
    QCOMPARE(primer.locateCoreSequenceStartInCognateStrand(bioString), -1);
    bioString = "AT";
    QCOMPARE(primer.locateCoreSequenceStartInCognateStrand(bioString), -1);
    bioString += "G";
    QCOMPARE(primer.locateCoreSequenceStartInCognateStrand(bioString), -1);

    // Test: perfect match
    QCOMPARE(primer.locateCoreSequenceStartInCognateStrand(BioString("CAT", eDnaGrammar)), 1);

    //           1234567890
    bioString = "ACAGATGACG";
    QCOMPARE(primer.locateCoreSequenceStartInCognateStrand(bioString), -1);

    bioString = "ACACATGACG";
    QCOMPARE(primer.locateCoreSequenceStartInCognateStrand(bioString), 4);

    // Test: should return the first range that it appears in
    bioString += "CAT";
    QCOMPARE(primer.locateCoreSequenceStartInCognateStrand(bioString), 11);

    // Test: check that restriction enzyme does not change things
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());
    Primer primer2(BioString("ATG", eDnaGrammar), re, 0., 0.);
    QCOMPARE(primer2.locateCoreSequenceStartInCognateStrand(bioString), 11);

    // Test: empty core sequence
    Primer primer3;
    QCOMPARE(primer3.locateCoreSequenceStartInCognateStrand(bioString), -1);
}

void TestPrimer::locateCoreSequenceStopInCognateStrand()
{
    Primer primer(BioString("ATG", eDnaGrammar), RestrictionEnzyme(), 0., 0.);

    BioString bioString;
    QCOMPARE(primer.locateCoreSequenceStopInCognateStrand(bioString), -1);
    bioString = "AT";
    QCOMPARE(primer.locateCoreSequenceStopInCognateStrand(bioString), -1);
    bioString += "G";
    QCOMPARE(primer.locateCoreSequenceStopInCognateStrand(bioString), -1);

    // Test: perfect match
    QCOMPARE(primer.locateCoreSequenceStopInCognateStrand(BioString("CAT", eDnaGrammar)), 3);

    //           1234567890
    bioString = "ACAGATGACG";
    QCOMPARE(primer.locateCoreSequenceStopInCognateStrand(bioString), -1);

    bioString = "ACACATGACG";
    QCOMPARE(primer.locateCoreSequenceStopInCognateStrand(bioString), 6);

    // Test: should return the first range that it appears in
    bioString += "CAT";
    QCOMPARE(primer.locateCoreSequenceStopInCognateStrand(bioString), 13);

    // Test: check that restriction enzyme does not change things
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());
    Primer primer2(BioString("ATG", eDnaGrammar), re, 0., 0.);
    QCOMPARE(primer2.locateCoreSequenceStopInCognateStrand(bioString), 13);

    // Test: empty core sequence
    Primer primer3;
    QCOMPARE(primer3.locateCoreSequenceStopInCognateStrand(bioString), -1);
}

void TestPrimer::restrictionEnzyme()
{
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());

    Primer primer(BioString("ABCD", eDnaGrammar), re, 0., 0.);
    QCOMPARE(primer.restrictionEnzyme(), re);
}

void TestPrimer::sequence()
{
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());

    // Test: with empty core sequence but restriction enzyme
    {
        Primer primer(BioString(eDnaGrammar), re, 0., 0.);
        QCOMPARE(primer.sequence(), re.recognitionSite());
    }

    // Test: core sequence but no restriction enzyme
    {
        Primer primer(BioString("ABCD", eDnaGrammar), RestrictionEnzyme(), 0., 0.);
        QCOMPARE(primer.sequence(), BioString("ABCD", eDnaGrammar));
    }

    // Test: both core sequence and restriction enzyme
    {
        Primer primer(BioString("ABCD", eDnaGrammar), re, 0., 0.);
        QCOMPARE(primer.sequence(), BioString("TTATTAABCD", eDnaGrammar));
    }
}

void TestPrimer::staticSequence()
{
    RestrictionEnzyme re("AanI", "TTATTA", QVector<int>() << 3 << 3, QVector<int>());
    BioString bioString("ABCD", eDnaGrammar);

    // Test: with empty core sequence but restriction enzyme
    QCOMPARE(Primer::sequence(BioString(eDnaGrammar), re), re.recognitionSite());

    // Test: core sequence but no restriction enzyme
    QCOMPARE(Primer::sequence(bioString, RestrictionEnzyme()), bioString);

    // Test: both core sequence and restriction enzyme
    QCOMPARE(Primer::sequence(bioString, re), BioString("TTATTAABCD", eDnaGrammar));
}

void TestPrimer::tm()
{
    Primer primer(BioString("ABCD", eDnaGrammar), RestrictionEnzyme(), 35., 0.);
    QCOMPARE(primer.tm(), 35.0);
}


QTEST_APPLESS_MAIN(TestPrimer)
#include "TestPrimer.moc"
