/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QString>

#include "../DimerScoreCalculator.h"
#include "../../core/BioString.h"

class TestDimerScoreCalculator : public QObject
{
    Q_OBJECT

private slots:
    void dimerScoreFromBonds();
    void dimerScore_data();
    void dimerScore();          // Assumes maximumHydrogenBonds works properly
    void homodimerScore_data();
    void homodimerScore();
    void maximumHydrogenBonds_data();
    void maximumHydrogenBonds();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestDimerScoreCalculator::dimerScoreFromBonds()
{
    DimerScoreCalculator x;

    for (int nHBonds=0; nHBonds< 15; ++nHBonds)
        for (int shorterPrimerLength=1; shorterPrimerLength< 12; ++shorterPrimerLength)
            QVERIFY(qFuzzyCompare(x.dimerScore(nHBonds, shorterPrimerLength), nHBonds * 10. / shorterPrimerLength));
}

void TestDimerScoreCalculator::dimerScore_data()
{
    // Using QString for its ease of specifying sequence data; in reality this is a BioString
    QTest::addColumn<QString>("qstring1");
    QTest::addColumn<QString>("qstring2");
    QTest::addColumn<double>("score");

    DimerScoreCalculator x;

    QTest::newRow("empty biostrings") << QString() << "" << 0.;
    QTest::newRow("CCTCACATCGATTA -- GGCCG") << "CCTCACATCGATTA" << "GGCCG" << x.dimerScore(6, 5);
}

void TestDimerScoreCalculator::dimerScore()
{
    QFETCH(QString, qstring1);
    QFETCH(QString, qstring2);
    QFETCH(double, score);

    DimerScoreCalculator x;

    QVERIFY(qFuzzyCompare(x.dimerScore(BioString(qstring1.toAscii(), eDnaGrammar),
                                       BioString(qstring2.toAscii(), eDnaGrammar)), score));
}

void TestDimerScoreCalculator::homodimerScore_data()
{
    // Using QString for its ease of specifying sequence data; in reality this is a BioString
    QTest::addColumn<QString>("qstring");
    QTest::addColumn<double>("homoScore");

    QTest::newRow("empty biostring") << QString() << 0.;
    QTest::newRow("A") << "A" << 0.;
    QTest::newRow("AAAAA") << "AAAAA" << 0.;
    QTest::newRow("CCCCCC") << "CCCCCC" << 0.;

    DimerScoreCalculator x;

    QTest::newRow("ATATG") << "ATATG" << x.dimerScore(8, 5);
    QTest::newRow("GCGCGC") << "GCGCGC" << x.dimerScore(6 * 3, 6);
    QTest::newRow("GATCG") << "GATCG" << x.dimerScore(3 + 2 + 2 + 3, 5);
    QTest::newRow("GGATGCT") << "GGATGCT" << x.dimerScore(3 + 2 + 2 + 3, 7);
}

void TestDimerScoreCalculator::homodimerScore()
{
    QFETCH(QString, qstring);
    QFETCH(double, homoScore);

    DimerScoreCalculator x;

    QCOMPARE(x.homoDimerScore(BioString(qstring.toAscii(), eDnaGrammar)), homoScore);
    QVERIFY(qFuzzyCompare(x.homoDimerScore(BioString(qstring.toAscii(), eDnaGrammar)), homoScore));
}

void TestDimerScoreCalculator::maximumHydrogenBonds_data()
{
    // Using QString for its ease of specifying sequence data; in reality this is a BioString
    QTest::addColumn<QString>("qstring");
    QTest::addColumn<int>("hBonds");

    QTest::newRow("empty biostring") << QString() << 0;
    QTest::newRow("A") << "A" << 0;
    QTest::newRow("AAAAA") << "AAAAA" << 0;
    QTest::newRow("CCCCCC") << "CCCCCC" << 0;

    QTest::newRow("ATATG") << "ATATG" << 8;
    QTest::newRow("GCGCGC") << "GCGCGC" << (6 * 3);
    QTest::newRow("GATCG") << "GATCG" << (3 + 2 + 2 + 3);
    QTest::newRow("GGATGCT") << "GGATGCT" << (3 + 2 + 2 + 3);
}

void TestDimerScoreCalculator::maximumHydrogenBonds()
{
    QFETCH(QString, qstring);
    QFETCH(int, hBonds);

    DimerScoreCalculator x;

    BioString bioString(qstring.toAscii(), eDnaGrammar);
    QCOMPARE(x.maximumHydrogenBonds(bioString, bioString), hBonds);
}

QTEST_APPLESS_MAIN(TestDimerScoreCalculator)
#include "TestDimerScoreCalculator.moc"
