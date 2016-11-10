/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Paul Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "ThermodynamicCalculator.h"
#include "DnaString.h"


class TestThermodynamicCalculator: public QObject
{
    Q_OBJECT

private slots:
    void sequenceEmpty();   //reports if no sequence assigned
    void setSequence();     //ensures that correct sequence was passed

    void calculateEnthalpy();   //enthalpy calculations
    void calculateEntropy();    //entropy calculations
    void sumEntropyAndEnthalpy();          //verify proper summing across hash of thermodynamic values for a sequence
    void meltingTemperature();             //check melting temperature against knowns
    void meltingTemperature_data();
    void symmetry();                    //symmetry check to set symmetry correction
    void symmetry_data();
    //void hairpinScore();
};


//Public methods
void TestThermodynamicCalculator::sequenceEmpty()
{
    ThermodynamicCalculator b;
    b.setSequence("");
    QVERIFY2(b.sequence() == "", "Sequence is not empty");
}

void TestThermodynamicCalculator::setSequence()
{
    ThermodynamicCalculator b;
    b.setSequence("ATGC");
    QVERIFY2(b.sequence() == "ATGC", "Sequence ATGC failed to pass");
}

void TestThermodynamicCalculator::calculateEntropy()
{
    ThermodynamicCalculator b;
    b.setSequence("");
    float currentEntropy = b.calculateEntropy();
    QVERIFY2(currentEntropy == 0, "Empty sequence entropy calculation incorrect");

    b.setSequence("ATGC");
    currentEntropy = b.calculateEntropy();
    float expectedEntropy = -66.2;
    QCOMPARE(currentEntropy, expectedEntropy);

    //calculation to test that the reverse complement has equivalent entropy value
    currentEntropy = 0;
    DnaString c = "ATGC";
    QString bReverse = c.reverseComplement();
    b.setSequence(bReverse);
    currentEntropy = b.calculateEntropy();
    QCOMPARE(currentEntropy, expectedEntropy);

    //tests a symmetrical sequence to ensure that symmetry correction is applied to entropy
    //this is not necessary for enthalpy calculations
    currentEntropy = 0;
    expectedEntropy = -99.7;
    b.setSequence("ATTAAT");
    currentEntropy = b.calculateEntropy();
    QCOMPARE(currentEntropy, expectedEntropy);
}

void TestThermodynamicCalculator::calculateEnthalpy()
{
    ThermodynamicCalculator b;
    b.setSequence("");
    float currentEnthalpy = b.calculateEnthalpy();
    QVERIFY2(currentEnthalpy == 0, "Empty sequence. Enthalpy calculation incorrect");

    b.setSequence("ATGC");
    currentEnthalpy = b.calculateEnthalpy();
    float expectedEnthalpy = -23.1;
    QVERIFY2(currentEnthalpy == expectedEnthalpy, "ATGC Enthalpy calculation incorrect");

    currentEnthalpy =0;
    b.setSequence("TTGC");
    currentEnthalpy = b.calculateEnthalpy();
    expectedEnthalpy = -23.8;
    QCOMPARE(currentEnthalpy, expectedEnthalpy);

    //calculation to test that the reverse complement has equivalent enthalpy value
    currentEnthalpy = 0;
    DnaString c = "ATGC";
    QString bReverse = c.reverseComplement();
    b.setSequence(bReverse);
    currentEnthalpy = b.calculateEnthalpy();
    expectedEnthalpy = -23.1;
    QCOMPARE(currentEnthalpy, expectedEnthalpy);
}

void TestThermodynamicCalculator::sumEntropyAndEnthalpy()
{
    ThermodynamicCalculator b;
    b.setSequence("CGAT");
    b.calculateEntropy();
    b.calculateEnthalpy();
    qreal currentEntropy = 0, currentEnthalpy = 0;
    qreal expectedEntropy = -68.5, expectedEnthalpy = -23.6;      //necessary to define these explicity as floats; direct comparison fails with -26 because it is an integer
    b.sumEntropyAndEnthalpy(currentEntropy, currentEnthalpy);
    QCOMPARE(currentEntropy, expectedEntropy);
    QCOMPARE(currentEnthalpy, expectedEnthalpy);
}


void TestThermodynamicCalculator::meltingTemperature_data()
{
    QTest::addColumn<QString>("sequence");
    QTest::addColumn<int>("expectedTemperature");
    QTest::addColumn<double>("saltConcentration"); //[salt] molarity

    QTest::newRow("short standard") << "ATGCATGC" << 328 << 1.0;
    QTest::newRow("short standard, , 50 mM salt") << "ATGCATGC" << 207 << 0.05;
    QTest::newRow("M13 reverse") << "AGCGGATAACAATTTCACACAGGA" << 742 << 1.0;
    QTest::newRow("M13 reverse, 5' G") << "AGCGGATAACAATTTCACACAGGG" << 749 << 1.0;
    QTest::newRow("M13 reverse, 5' G, , 50 mM salt") << "AGCGGATAACAATTTCACACAGGG" << 592 << 0.05;
    QTest::newRow("M13 reverse, 5' C") << "AGCGGATAACAATTTCACACAGGC" << 754 << 1.0;
    QTest::newRow("M13 reverse, 5' T") << "AGCGGATAACAATTTCACACAGGT" << 744 << 1.0;
    QTest::newRow("M13 reverse, 3' G") << "GGCGGATAACAATTTCACACAGGA" << 749 << 1.0;
    QTest::newRow("M13 reverse, 3' C") << "CGCGGATAACAATTTCACACAGGA" << 750 << 1.0;
    QTest::newRow("M13 reverse, 3' T") << "TGCGGATAACAATTTCACACAGGA" << 744 << 1.0;
    QTest::newRow("T7 universal") << "TAATACGACTCACTATAGGG" << 640 << 1.0;
    QTest::newRow("T7 universal, 50 mM salt") << "TAATACGACTCACTATAGGG" << 487 << 0.05;
    QTest::newRow("primer from long sequence") << "ATGCCCTTCAGCAAAGCATGGCGGA" << 831 << 1.0;
    //QTest::newRow("second primer from long sequence") << "TGCCCTTCAGCAAAGCATGGCGGAG" << 833;
}

/*test a series of different primer sequences against known predicted values
*/
void TestThermodynamicCalculator::meltingTemperature()
{
    QFETCH(QString, sequence);
    QFETCH(int, expectedTemperature);
    QFETCH(double, saltConcentration);
    qreal newExpect = qreal(expectedTemperature)/10.;    //this workaround necessary because of QCOMPARE's fuzzy compare of floats/doubles
    ThermodynamicCalculator b;
    b.setSequence(sequence);
    QCOMPARE(b.meltingTemperature(saltConcentration), newExpect);
}

/*test symmetry or a variety of sequences
*/
void TestThermodynamicCalculator::symmetry_data()
{
    QTest::addColumn<QString>("sequence");
    QTest::addColumn<int>("result");

    QTest::newRow("positive Short") << "ATTAAT" << 1;    //this is a palindrome; it should return  1
    QTest::newRow("negative Short") << "ATATATA" << 0;  //not symmetrical; should return 4
    QTest::newRow("EcoRI Site") <<"GAATTC" << 1;
    QTest::newRow("long Positive") << "GGCCATTAATGGCC" << 1;
    QTest::newRow("another Primer") << "ATGCCCTTCAGCAAAGCATGGCGGA" << 0;

}

void TestThermodynamicCalculator::symmetry()
{
    QFETCH(QString, sequence);
    QFETCH(int, result);

    ThermodynamicCalculator b;
    b.setSequence(sequence);
    QCOMPARE(b.testSymmetry(), result);

}

//void TestThermodynamicCalculator::hairpinScore()
//{
//    ThermodynamicCalculator b;
//    //b.setSequence("GCCGCGGGCCGAAAAAACCCCCCCGGCCCGCGGC"); //example sequence that should form a hairpin loop; mfold predicts it
//    b.setSequence("ATGCATGC");
//    b.hairpinScore();
//}


QTEST_MAIN(TestThermodynamicCalculator)
#include "TestThermodynamicCalculator.moc"
