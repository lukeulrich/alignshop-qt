/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "PrimerFinder.h"
#include "ThermodynamicCalculator.h"

bool fuzzyCompare(double actual, double expected, double maxdiff) { return (qAbs(actual - expected) <= maxdiff); } // handy function to more easily compare fuzzy values

class TestPrimerFinder: public QObject
{
    Q_OBJECT

private slots:
    void primerFinder_data();
    void primerFinder();

    void pairPrimers_data();
    void pairPrimers();
    void topPrimerPairs();

    void ampliconLength();
    void primerPairScore();

private:
    PrimerDesignInput input(QString currentSequence_, int minAmpliconLength, int maxAmpliconLength, int minPrimerLength,
                            int maxPrimerLength, int minTm, int maxTm, QString forwardTerminalAddition, QString reverseTerminalAddition, QString forwardThreePrimeSeqRequirementsString, QString reverseThreePrimeSeqRequirementsString) const
    {
        PrimerDesignInput i;
        i.amplicon = currentSequence_;
        i.ampliconBounds = Range(0, currentSequence_.length());
        i.ampliconSizeRange = Range(minAmpliconLength, maxAmpliconLength);
        i.forwardPrefix = forwardTerminalAddition;
        i.reversePrefix = reverseTerminalAddition;
        i.forwardSuffix = forwardThreePrimeSeqRequirementsString;
        i.reverseSuffix = reverseThreePrimeSeqRequirementsString;
        i.primerSizeRange = Range(minPrimerLength, maxPrimerLength);
        i.tmRange = RangeF(minTm, maxTm);
        return i;
    }
};

//Public methods

void TestPrimerFinder::primerFinder_data()
{
    QTest::addColumn<QString>("sequence");
    QTest::addColumn<int>("minAmpliconLength");
    QTest::addColumn<int>("maxAmpliconLength");
    QTest::addColumn<int>("minPrimerLength");
    QTest::addColumn<int>("maxPrimerLength");
    QTest::addColumn<double>("minTm");
    QTest::addColumn<double>("maxTm");
    QTest::addColumn<QString>("forwardTerminalAddition");
    QTest::addColumn<QString>("reverseTerminalAddition");
    QTest::addColumn<QString>("fivePrimeSeqRequirementsString");
    QTest::addColumn<QString>("threePrimeSeqRequirementsString");
    QTest::addColumn<QString>("predictedForwardPrimerSequence");
    QTest::addColumn<double>("predictedForwardTm");
    QTest::addColumn<QString>("predictedReversePrimerSequence");
    QTest::addColumn<double>("predictedReverseTm");
    QTest::addColumn<int>("primerNumber"); //0-indexed primerNumber to allow me to test for a specific primer identified

    QTest::newRow("first forward and reverse primers minLength equals 20") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 490 << 20 << 25 << 55.0 << 85.0 << "" << "" << "..." << "..." << "ATGCCCTTCAGCAAAGCATG" << 73.3 << "ATTCCTTGTTAATATTCTTC" << 59.3 << 0;
    QTest::newRow("first forward and reverse primers where the min amplicon length equals maximum") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 490 << 490 << 25 << 25 << 55.0 << 85.0 << "" << ""  << "..." << "..." << "ATGCCCTTCAGCAAAGCATGGCGGA" << 83.1 << "ATTCCTTGTTAATATTCTTCCACCG" << 70.6 << 0;

    QTest::newRow("maxAmpliconLength is longer than sequence length") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 491 << 20 << 25 << 55.0 << 85.0 << "" << "" << "..." << "..." << "Maximum amplicon length must be equal to or lower than target sequence length." << -1.0 << "Maximum amplicon length must be equal to or lower than target sequence length." << -1.0 << 0;
    QTest::newRow("first 20 bases repeated should exclude primer starting on base 1") << "ATGCCCTTCAGCAAAGCATGATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 490 << 20 << 25 << 55.0 << 85.0 << "" << "" << "..." << "..." << "TGCCCTTCAGCAAAGCATGA" << 74.6 << "ATTCCTTGTTAATATTCTTC" << 59.3 << 0;

    QTest::newRow("forward terminal addition") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 490 << 20 << 25 << 55.0 << 85.0 <<"CATCAT" <<"" << "..." << "..." << "CATCATATGCCCTTCAGCAAAGCATG" << 76.1 << "ATTCCTTGTTAATATTCTTC" << 59.3 << 0;
    QTest::newRow("reverse terminal addition") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 490 << 20 << 25 << 55.0 << 85.0 <<"" <<"CATCAT" << "..." << "..." << "ATGCCCTTCAGCAAAGCATG" << 73.3 << "CATCATATTCCTTGTTAATATTCTTC" << 65.5 << 0;

    QTest::newRow("first forward and reverse primers minLength equals 20, Tm range too low") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 490 << 20 << 25 << 55.0 << 59.2 << "" << "" << "..." << "..." << "No forward primers were found" << -1.0 << "No reverse primers were found" << -1.0 << 0;
    QTest::newRow("first forward and reverse primers minLength equals 20, Tm range too high") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 490 << 20 << 25 << 85. << 90.0 << "" << "" << "..." << "..." << "No forward primers were found" << -1.0 << "No reverse primers were found" << -1.0 << 0;

    QTest::newRow("Forward primer Tm too low") << "AAAAAAAAAAAAAAAAAATTCCCCCCCCCCCCCCCCCC" << 38 << 38 << 18 << 18 << 52. << 95. << "" << "" << "..." << "..." << "No forward primers were found" << -1.0 << "GGGGGGGGGGGGGGGGGG" << 89.8 << 0;
    QTest::newRow("Reverse primer Tm too high") << "AAAAAAAAAAAAAAAAAATTCCCCCCCCCCCCCCCCCC" << 38 << 38 << 18 << 18 << 50. << 55. << "" << "" << "..." << "..." << "AAAAAAAAAAAAAAAAAA" << 51.6 << "No reverse primers were found" << -1.0 << 0;

    QTest::newRow("Forward Three Prime Requirement") << "AAAAAAAAAAAAAAAAACCCAAACCCCCCCCCCCCCCCCCCCC" << 43 << 43 << 20 << 20 << 50. << 95. << "" << "" << "CCC" << "..." << "AAAAAAAAAAAAAAAAACCC" << 59.4 << "GGGGGGGGGGGGGGGGGGGG" << 93.6 << 0;
    QTest::newRow("Forward Three Prime Requirement missing") << "AAAAAAAAAAAAAAAAACCAAACCCCCCCCCCCCCCCCCCCC" << 42 << 42 << 20 << 20 << 50. << 95. << "" << "" << "CCC" << "..." << "No forward primers were found" << -1.0 << "GGGGGGGGGGGGGGGGGGGG" << 93.6 << 0;
    QTest::newRow("Reverse Three Prime Requirement") << "AAAAAAAAAAAAAAAAAAAAGGGCCCCCCCCCCCCCCCCC" << 40 << 40 << 20 << 20 << 50. << 95. << "" << "" << "..." << "CCC" << "AAAAAAAAAAAAAAAAAAAA" << 54.7 << "GGGGGGGGGGGGGGGGGCCC" << 94.0 << 0;
    QTest::newRow("Reverse Three Prime Requirement missing") << "AAAAAAAAAAAAAAAAAAAATTTGGCCCCCCCCCCCCCCCCC" << 42 << 42 << 20 << 20 << 50. << 95. << "" << "" << "..." << "CCC" << "AAAAAAAAAAAAAAAAAAAA" << 54.7 << "No reverse primers were found" << -1.0 << 0;
}

void TestPrimerFinder::primerFinder()
{
    // pull in variables from data function
    QFETCH(QString, sequence);
    QFETCH(int, minAmpliconLength);
    QFETCH(int, maxAmpliconLength);
    QFETCH(int, minPrimerLength);
    QFETCH(int, maxPrimerLength);
    QFETCH(double, minTm);      // QTest seems like it can only handle double, not float
    QFETCH(double, maxTm);
    QFETCH(QString, forwardTerminalAddition);
    QFETCH(QString, reverseTerminalAddition);
    QFETCH(QString, fivePrimeSeqRequirementsString);
    QFETCH(QString, threePrimeSeqRequirementsString);
    QFETCH(QString, predictedForwardPrimerSequence);
    QFETCH(double, predictedForwardTm);
    QFETCH(QString, predictedReversePrimerSequence);
    QFETCH(double, predictedReverseTm);
    QFETCH(int, primerNumber);

    QList<QList<primer> > primerList;

    PrimerFinder b;

    PrimerDesignInput i = input(sequence, minAmpliconLength, maxAmpliconLength, minPrimerLength, maxPrimerLength, minTm, maxTm, forwardTerminalAddition, reverseTerminalAddition, fivePrimeSeqRequirementsString, threePrimeSeqRequirementsString);
    primerList = b.findPrimers(&i);

    QCOMPARE(primerList.at(0).at(primerNumber).primerSequence_, predictedForwardPrimerSequence);
    QVERIFY(fuzzyCompare(primerList.at(0).at(primerNumber).tm_, predictedForwardTm, 0.01));        // using the fuzzyCompare() function Luke found to deal with double comparisons and given precision

    QCOMPARE(primerList.at(1).at(primerNumber).primerSequence_, predictedReversePrimerSequence);
    QVERIFY(fuzzyCompare(primerList.at(1).at(primerNumber).tm_, predictedReverseTm, 0.01));
}

void TestPrimerFinder::pairPrimers_data()
{
    QTest::addColumn<QString>("sequence");
    QTest::addColumn<int>("minAmpliconLength");
    QTest::addColumn<int>("maxAmpliconLength");
    QTest::addColumn<int>("minPrimerLength");
    QTest::addColumn<int>("maxPrimerLength");
    QTest::addColumn<double>("minTm");
    QTest::addColumn<double>("maxTm");
    QTest::addColumn<QString>("forwardTerminalAddition");
    QTest::addColumn<QString>("reverseTerminalAddition");
    QTest::addColumn<QString>("fivePrimeSeqRequirementsString");
    QTest::addColumn<QString>("threePrimeSeqRequirementsString");
    QTest::addColumn<QString>("predictedForwardPrimerSequence");
    QTest::addColumn<double>("predictedForwardTm");
    QTest::addColumn<QString>("predictedReversePrimerSequence");
    QTest::addColumn<double>("predictedReverseTm");
    QTest::addColumn<QString>("predictedHeterodimerFirstLine");
    QTest::addColumn<QString>("predictedHeterodimerSecondLine");
    QTest::addColumn<QString>("predictedHeterodimerThirdLine");
    QTest::addColumn<int>("primerNumber");

    QTest::newRow("first forward and reverse primers minLength equals 20") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 490 << 20 << 25 << 55.0 << 85.0 << "" << "" << "..." << "..." << "ATGCCCTTCAGCAAAGCATG" << 73.3 << "ATTCCTTGTTAATATTCTTC" << 59.3 << " ATGCCCTTCAGCAAAGCATG" << " | |    | | ||| | || " << "CTTCTTATAATTGTTCCTTA " << 0;
    QTest::newRow("second forward and reverse primers minLength equals 20") << "ATGCCCTTCAGCAAAGCATGGCGGAGTGCCGTTTATCCTGATTTTAGGGAGCAGGGCGCGTATATTAACTACAAGGCAACGAAAGATACTTTGCATCGCATGAAGGAAGATATCGCGAACCCTGCAACGCCAGATGAATTGTACAATTCGCTGCTCATGCAAAAGGCAACTGTGTATAAGTGGTGTGAAAACAAGGTGAAGGAATTGCAAATGATGGCAGAAGCCCTCATGAAGGCATCGGATTACCTCTCAGAAGAAGAGACACCTACTAATATGAGTATGGTATTCAGTATGGTGGGGAGTAGTGAAGCGAAGTATCTTCCACCCAGTGATGCGAGGCGCGTGGCAGATGCAATCACGTATGAACTACTGCGTTTTGTTGAATGCCGTAATCTGAACACAGACACAATTGAGCACATTATTGCGCGTATGTATCGCTACGCCGTGCTCGGACCCACCGGCGATCGGTGGAAGAATATTAACAAGGAAT" << 485 << 490 << 20 << 25 << 55.0 << 85.0 << "" << "" << "..." << "..." << "ATGCCCTTCAGCAAAGCATG" << 73.3 << "TTCCTTGTTAATATTCTTCC" << 59.3 << "  ATGCCCTTCAGCAAAGCATG" << "  | |    | | ||| | |  " << "CCTTCTTATAATTGTTCCTT  " << 1;

}

void TestPrimerFinder::pairPrimers()
{
    QList<QList<primer> > primerList;
    QList<primerPair>  primerPairList;

    // pull in variables from data function
    QFETCH(QString, sequence);
    QFETCH(int, minAmpliconLength);
    QFETCH(int, maxAmpliconLength);
    QFETCH(int, minPrimerLength);
    QFETCH(int, maxPrimerLength);
    QFETCH(double, minTm);      // QTest seems like it can only handle double, not float
    QFETCH(double, maxTm);
    QFETCH(QString, forwardTerminalAddition);
    QFETCH(QString, reverseTerminalAddition);
    QFETCH(QString, fivePrimeSeqRequirementsString);
    QFETCH(QString, threePrimeSeqRequirementsString);
    QFETCH(QString, predictedForwardPrimerSequence);
    QFETCH(QString, predictedReversePrimerSequence);
    QFETCH(QString, predictedHeterodimerFirstLine);
    QFETCH(QString, predictedHeterodimerSecondLine);
    QFETCH(QString, predictedHeterodimerThirdLine);
    QFETCH(int, primerNumber);

    PrimerFinder b;

    PrimerDesignInput i = input(sequence, minAmpliconLength, maxAmpliconLength, minPrimerLength, maxPrimerLength, minTm, maxTm, forwardTerminalAddition, reverseTerminalAddition, fivePrimeSeqRequirementsString, threePrimeSeqRequirementsString);
    primerList = b.findPrimers(&i);

    primerPairList = b.pairPrimers(sequence, primerList, minAmpliconLength, maxAmpliconLength);

    QCOMPARE(primerPairList.at(primerNumber).forwardPrimerSequence_, predictedForwardPrimerSequence);
    QCOMPARE(primerPairList.at(primerNumber).reversePrimerSequence_, predictedReversePrimerSequence);
    QCOMPARE(primerPairList.at(primerNumber).heteroDimer_.displayLines_.at(0), predictedHeterodimerFirstLine);
    QCOMPARE(primerPairList.at(primerNumber).heteroDimer_.displayLines_.at(1), predictedHeterodimerSecondLine);
    QCOMPARE(primerPairList.at(primerNumber).heteroDimer_.displayLines_.at(2), predictedHeterodimerThirdLine);

}

void TestPrimerFinder::topPrimerPairs()
{
    //Left off here on generating a top list of primer pairs
    PrimerFinder b;
//    b.topPrimerPairs();
}


void TestPrimerFinder::ampliconLength()
{
    PrimerFinder b;
    QCOMPARE(b.ampliconLength(490, 270), 220);
    QCOMPARE(b.ampliconLength(270, 490), -1);
    QCOMPARE(b.ampliconLength(270, 270), -1);
}

void TestPrimerFinder::primerPairScore()
{
    PrimerFinder b;
    QVERIFY2(fuzzyCompare(b.scorePrimerPair(54.0, 60.0, 20, 20, 10, 10, 10), 7.125, .001), "Primer pair score incorrect.");
    QVERIFY2(fuzzyCompare(b.scorePrimerPair(60.0, 54.0, 20, 20, 10, 10, 10), 7.125, .001), "Primer pair score incorrect.");
}



QTEST_MAIN(TestPrimerFinder)
#include "TestPrimerFinder.moc"
