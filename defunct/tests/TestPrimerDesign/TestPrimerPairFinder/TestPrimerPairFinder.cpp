/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QScopedPointer>
#include <PrimerDesign/DimerCalculator.h>
#include <PrimerDesign/PrimerPairFinder.h>
#include <PrimerDesign/PrimerDesignInput.h>
#include "DnaString.h"
#include "global.h"

using namespace PrimerDesign;

class TestPrimerPairFinder : public QObject
{
    Q_OBJECT

private slots:
    void validateDefaultInput()
    {
        PrimerDesignInput input(getValidGenerationInput()->amplicon);

        QVERIFY(input.ampliconBounds.min() == 1);
        QVERIFY(input.ampliconBounds.max() == input.amplicon.length());
        QVERIFY(input.ampliconSizeRange.min() == input.amplicon.length() - 20);
        QVERIFY(input.ampliconSizeRange.max() == input.amplicon.length());
        QVERIFY(input.primerSizeRange.min() == 25);
        QVERIFY(input.primerSizeRange.max() == 25);
        QVERIFY(input.tmRange.min() == 55);
        QVERIFY(input.tmRange.max() == 85);
    }

    void validateGeneration()
    {
        PrimerDesignInputRef input = getValidGenerationInput();
        PrimerPairFinder finder(input.data());
        PrimerPairFinderResult result = finder.findPrimerPairs();
        QList<PrimerPair > expected = getExpectedValidResults();
        QList<PrimerPair > actual = result.value;
        const int EXPECTED_LENGTH = 50;

        qSort(actual.begin(), actual.end(), byScore);

        QVERIFY2(!result.isError, "The result should not have been an error.");
        QVERIFY2(actual.count() == EXPECTED_LENGTH, "The result's length does not equal what was expected.");

        for (int i = 0; i < 10; ++i)
        {
            QVERIFY2(
                expected[i].forwardPrimer().sequence() == actual[i].forwardPrimer().sequence(),
                "Forward primer was not as expected.");
            QVERIFY2(
                expected[i].reversePrimer().sequence() == actual[i].reversePrimer().sequence(),
                "Reverse primer was not as expected.");
        }
    }

    void validateScores()
    {
        QList<QString > sequences;
        sequences << "CGCCAGATGAATTGTACAATT" << "GTCAACTG" << "AGCATGGCGGAGTGC" << "AGGCAACGAAAGATACTTTG";

        QList<int > dimers;
        dimers << 26 << 10 << 22 << 24;

        for (int i = 0; i < sequences.length(); ++i)
        {
            verifyScore(sequences[i], sequences[i], dimers[i]);
        }
    }

private:
    void verifyScore(QString seq1, QString seq2, int score)
    {
        int result = DimerCalculator::score(seq1, seq2);
        QVERIFY2(result == score, "Score was not as expected.");
    }

    static bool byScore(const PrimerPair &p1, const PrimerPair &p2)
    {
        return p1.score() < p2.score();
    }

    PrimerPair createPair(QString forward, QString reverse, const PrimerDesignInputRef &params)
    {
        PrimerPair pair(params);
        pair.setForwardPrimer(Primer(forward, *params.data()));
        pair.setReversePrimer(Primer(reverse, *params.data()));
        return pair;
    }

    QList<PrimerPair > getExpectedValidResults()
    {
        PrimerDesignInputRef params = getValidGenerationInput();
        QList<PrimerPair > results;

        results << createPair("CTTCGTCTCTGTCGGAGCTGGGGTT", "GCTGGCAGTTCTCCCAGAGCGTCTT", params);
        results << createPair("TTCGTCTCTGTCGGAGCTGGGGTTC", "GCTGGCAGTTCTCCCAGAGCGTCTT", params);
        results << createPair("TTCGTCTCTGTCGGAGCTGGGGTTC", "TGGCAGTTCTCCCAGAGCGTCTTGG", params);
        results << createPair("CTTCGTCTCTGTCGGAGCTGGGGTT", "TGGCAGTTCTCCCAGAGCGTCTTGG", params);
        results << createPair("TCGTCTCTGTCGGAGCTGGGGTTCG", "TTCTCCCAGAGCGTCTTGGCGGAGC", params);
        results << createPair("TCGTCTCTGTCGGAGCTGGGGTTCG", "AGCTGGCAGTTCTCCCAGAGCGTCT", params);
        results << createPair("CTTCGTCTCTGTCGGAGCTGGGGTT", "TAGAGGTCGAGCAGCTGGCAGTTCT", params);
        results << createPair("TCGTCTCTGTCGGAGCTGGGGTTCG", "GCTGGCAGTTCTCCCAGAGCGTCTT", params);
        results << createPair("TCGTCTCTGTCGGAGCTGGGGTTCG", "CAGCTGGCAGTTCTCCCAGAGCGTC", params);
        results << createPair("TGAAGAAGGCTTCGTCTCTGTCGGA", "TTAGAGGTCGAGCAGCTGGCAGTTC", params);

        return results;
    }

    PrimerDesignInputRef getValidGenerationInput()
    {
        PrimerDesignInputRef input(new PrimerDesignInput("ATGAAGAAGGCTTCGTCTCTGTCGGAGCTGGGGTTCGACGCGGAGGGCGCGTCGTCGGGGTTCTTCCGTCCGGTGGCGGACGGCGGGTCGACGCCGACGTCGCACCGGCGTCGGCTGACGAAGATATCGGTGATCGGCGCGGGCAACGTGGGGATGGCGATCGCGCAGACCATCCTGACCCGGGAGCATGGCGGACGAGATCGCGCTGGTGGACGCGGTGCCGGACAAGCTGCGCGGGGAGATGCTGGACCTGCAGCACGCGGCGGCGTTCCTCCCCCGCGTCCGCCTCGTCTCCGACACCGACCTGGCCGTCACGCGCGGCTCCGACCTGGCCATCGTCACGGCCGGCGCGCGCCAGATCCCCGGGGAGAGCCGCCTGAACCTGCTGCAGCGGAACGTGGCGCTGTTCCGGAAGATCGTGCCGGCGCTGGCGGAGCACTCGCCGGAGGCGCTGCTGCTGATCGTCTCCAACCCCGTCGACGTGCTGACGTACGTGGCGTGGAAGATGTCGGGGTTCCCGGCGAGCCGCGTCATCGGCTCCGGCACCAACCTCGACTCCTCCTGGTTCCGCTTCCTCCTCGCCGAGCACCTCCAGGTCAACGCCCAGGATGTCCAGTGCAGGCGTACATGGTGGGAGAGCACGGGGACAGCTCGGTGGCGATATGGTCGAGCATGAGCGTGGCCGGGATGCCGGTGCTCAAGTCGCTGCGGGAGAGCCACCAGAGCTTCGACGAGGAGGCCCTGGAGGGAATCCGGCGAGCGGTGGTGGACAGCGCGTACGAGGTGATCAGCCTCAAGGGCTACACCTCCTGGGCCATCGGCTACTCCGTCGCCAGCCTCGCCGCCTCCCTCCTCCGCGACCAGCACCGCATCCACCCCGTCTCCGTCCTCGCCTCCGGCTTCCACGGCATCCCCCAAGACCACGAGGTCTTCCTCAGCCTCCCCGCCCGCCTCGGCCGCGCCGGCGTCCTCGGCGTCGCCGAGATGGAGCTCACCGAGGAGGAGGCCCGCCGCCTCCGCCGCTCCGCCAAGACGCTCTGGGAGAACTGCCAGCTGCTCGACCTCTAA"));
        input->ampliconBounds = Range(1, input->amplicon.length());
        input->ampliconSizeRange = Range(1025, input->amplicon.length());
        input->primerSizeRange = Range(25, 25);
        input->tmRange = RangeF(55, 85);
        return input;
    }
};

QTEST_MAIN(TestPrimerPairFinder)
#include "TestPrimerPairFinder.moc"
