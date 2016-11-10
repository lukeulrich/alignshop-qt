/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../ConsensusAlphabetDetector.h"
#include "../AlphabetDetector.h"
#include "../../PODs/SimpleSeqPod.h"

class TestConsensusAlphabetDetector : public QObject
{
    Q_OBJECT

private slots:
    void detectConsensusAlphabet_data();
    void detectConsensusAlphabet();
};

typedef QVector<SimpleSeqPod> SimpleSeqPodVector;

Q_DECLARE_METATYPE(SimpleSeqPodVector)
Q_DECLARE_METATYPE(Alphabet)

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestConsensusAlphabetDetector::detectConsensusAlphabet_data()
{
    QTest::addColumn<SimpleSeqPodVector>("simpleSeqPodVector");
    QTest::addColumn<Alphabet>("alphabet");

    // Test: invalid records
    QTest::newRow("no pods") << SimpleSeqPodVector() << Alphabet();
    QTest::newRow("invalid chars") << (SimpleSeqPodVector() << SimpleSeqPod("Domain", "@")) << Alphabet();

    // Test: basic alphabets, single sequence pod
    QTest::newRow("Basic DNA alphabet") << (SimpleSeqPodVector() << SimpleSeqPod("Domain", "ACGT")) << constants::kDnaBasicAlphabet;
    QTest::newRow("Basic RNA alphabet") << (SimpleSeqPodVector() << SimpleSeqPod("Domain", "ACGU")) << constants::kRnaBasicAlphabet;
    QTest::newRow("Basic Amino alphabet") << (SimpleSeqPodVector() << SimpleSeqPod("Domain", "F")) << constants::kAminoBasicAlphabet;

    // Test: multi sequence pods
    QTest::newRow("2 pods, one dna, one rna") << (SimpleSeqPodVector()
                                                  << SimpleSeqPod("", "ACGT")
                                                  << SimpleSeqPod("", "ACGU")) << constants::kDnaBasicAlphabet;
    QTest::newRow("2 pods, one dna extended, one rna") << (SimpleSeqPodVector()
                                                  << SimpleSeqPod("", "ACGTB")
                                                  << SimpleSeqPod("", "ACGU")) << constants::kRnaBasicAlphabet;
    QTest::newRow("3 pods, two dna, one rna") << (SimpleSeqPodVector()
                                                  << SimpleSeqPod("", "ACGT")
                                                  << SimpleSeqPod("", "ACGU")
                                                  << SimpleSeqPod("", "ACGT")) << constants::kDnaBasicAlphabet;
    QTest::newRow("2 pods, one amino, one rna") << (SimpleSeqPodVector()
                                                    << SimpleSeqPod("", "ACDEFG")
                                                    << SimpleSeqPod("", "U")) << constants::kRnaBasicAlphabet;
    QTest::newRow("2 pods, one junk and one dna") << (SimpleSeqPodVector()
                                                      << SimpleSeqPod("", "@!@#@#$@#")
                                                      << SimpleSeqPod("", "ACTB")) << constants::kDnaExtendedAlphabet;

    QTest::newRow("3 pods, all dna, two extended, one basic") << (SimpleSeqPodVector()
                                                                  << SimpleSeqPod("", "ACGT")
                                                                  << SimpleSeqPod("", "ACGTB")
                                                                  << SimpleSeqPod("", "ACGTB")) << constants::kDnaExtendedAlphabet;
}

void TestConsensusAlphabetDetector::detectConsensusAlphabet()
{
    QFETCH(SimpleSeqPodVector, simpleSeqPodVector);
    QFETCH(Alphabet, alphabet);

    AlphabetDetector alphabetDetector;
    alphabetDetector.setAlphabets(constants::kStandardAlphabetVector);
    ConsensusAlphabetDetector<SimpleSeqPodVector> consensusDetector(&alphabetDetector);

    QCOMPARE(consensusDetector.detectConsensusAlphabet(simpleSeqPodVector), alphabet);
}


QTEST_APPLESS_MAIN(TestConsensusAlphabetDetector)
#include "TestConsensusAlphabetDetector.moc"
