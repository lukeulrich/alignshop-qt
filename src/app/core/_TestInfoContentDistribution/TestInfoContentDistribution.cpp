/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <cmath>

#include "../InfoContentDistribution.h"
#include "../_Mocks/MockCharCountDistributions.h"

class TestInfoContentDistribution : public QObject
{
    Q_OBJECT

public:
    TestInfoContentDistribution()
    {
        qRegisterMetaType<CharCountDistribution>("CharCountDistribution");
        qRegisterMetaType<VectorVectorInfoUnit>("VectorVectorInfoUnit");
    }

private slots:
    void infoUnit();
    void infoContent_data();
    void infoContent();
    void columnInfo();
};
Q_DECLARE_METATYPE(CharCountDistribution)
Q_DECLARE_METATYPE(VectorVectorInfoUnit)

VectorVectorInfoUnit basicInfoContentDataWithNoErrorCorrection()
{
    // Column 1:
    // Entropy = .3×(log.3÷log2)+.3×(log.3÷log2)+.4×(log.4÷log2) = 1.570950594
    // Total col info = log2(4) + entropy = 0.429049406
    // Column 2:
    // completely conserved - all G's
    // Column 3:
    // Entropy = 0.832192809
    // Total col info = 1.167807191
    VectorVectorInfoUnit infoContentNoErrorCorrection;
    infoContentNoErrorCorrection << (VectorInfoUnit()
                                     << InfoUnit('A', .3, 0.128714822)
                                     << InfoUnit('T', .3, 0.128714822)
                                     << InfoUnit('C', .4, 0.171619762))
                                 << (VectorInfoUnit()
                                     << InfoUnit('G', 1., 2.))
                                 << (VectorInfoUnit()
                                     << InfoUnit('C', .1, 0.116780719)
                                     << InfoUnit('A', .5, 0.583903595));

    return infoContentNoErrorCorrection;
}

VectorVectorInfoUnit basicInfoContentDataWithErrorCorrection()
{
    VectorVectorInfoUnit infoContentErrorCorrection;
    // Column 1:
    // Error correction = 3 / (2 * ln(2) * 10) = 0.216404256
    // Total col info = 0.429049406 (see above) - error correction = 0.21264515
    infoContentErrorCorrection << (VectorInfoUnit()
                                   << InfoUnit('A', .3, 0.063793545)
                                   << InfoUnit('T', .3, 0.063793545)
                                   << InfoUnit('C', .4, 0.08505806))
    // Column 2:
    // Error correction = 3 / (2 * ln(2) * 10) = 0.216404256
    // Total col info = 2 - error correction = 1.783595744
                               << (VectorInfoUnit()
                                   << InfoUnit('G', 1., 1.783595744))

    // Column 3:
    // Error correction = 3 / (2 * ln(2) * 6) = 0.36067376
    // Total col info = 1.167807191 - error correction = 0.807133431
                                 << (VectorInfoUnit()
                                     << InfoUnit('C', .1, 0.080713343)
                                     << InfoUnit('A', .5, 0.403566715));
    return infoContentErrorCorrection;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestInfoContentDistribution::infoUnit()
{
    InfoUnit unit;
    QVERIFY(unit.isNull());
    QCOMPARE(unit.ch_, '\0');
    QCOMPARE(unit.percent_, 0.);
    QCOMPARE(unit.info_, 0.);

    InfoUnit unit2('a', .4, .6);
    QVERIFY(unit2.isNull() == false);
    QCOMPARE(unit2.ch_, 'a');
    QCOMPARE(unit2.percent_, .4);
    QCOMPARE(unit2.info_, .6);
}

void TestInfoContentDistribution::infoContent_data()
{
    QTest::addColumn<CharCountDistribution>("charCountDistribution");
    QTest::addColumn<int>("possibleLetters");
    QTest::addColumn<int>("length");
    QTest::addColumn<double>("totalInfoNoErrorCorrection");
    QTest::addColumn<double>("totalInfoErrorCorrection");
    QTest::addColumn<VectorVectorInfoUnit>("infoContentNoErrorCorrection");
    QTest::addColumn<VectorVectorInfoUnit>("infoContentErrorCorrection");

    // Test: empty CharCountDistribution
    QTest::newRow("Empty distribution - 20 letters") << CharCountDistribution() << 20 << 0 << 0. << 0. << VectorVectorInfoUnit() << VectorVectorInfoUnit();
    QTest::newRow("Empty distribution - 4 letters") << CharCountDistribution() << 4 << 0 << 0. << 0. << VectorVectorInfoUnit() << VectorVectorInfoUnit();

    // Test: non-empty CharCountDistribution - varied number of possible letters
    CharCountDistribution dist = ::charCountDistribution1();
    VectorVectorInfoUnit infoContentNoErrorCorrection = basicInfoContentDataWithNoErrorCorrection();
    VectorVectorInfoUnit infoContentErrorCorrection = basicInfoContentDataWithErrorCorrection();
    QTest::newRow("Non-empty distribution, DNA") << dist << 4 << 3 << 3.12973372 << 2.480520952 << infoContentNoErrorCorrection << infoContentErrorCorrection;
}

void TestInfoContentDistribution::infoContent()
{
    QFETCH(CharCountDistribution, charCountDistribution);
    QFETCH(int, possibleLetters);
    QFETCH(int, length);
    QFETCH(double, totalInfoNoErrorCorrection);
    QFETCH(double, totalInfoErrorCorrection);
    QFETCH(VectorVectorInfoUnit, infoContentNoErrorCorrection);
    QFETCH(VectorVectorInfoUnit, infoContentErrorCorrection);

    // Test A: no small sample error correction
    {
        InfoContentDistribution x(charCountDistribution, possibleLetters, false);

        QCOMPARE(x.possibleLetters(), possibleLetters);
        QCOMPARE(x.smallSampleErrorCorrection(), false);
        QCOMPARE(x.length(), length);
        QCOMPARE(x.maxInfo(), log2(possibleLetters));
        QVERIFY(fabs(x.totalInfo() - totalInfoNoErrorCorrection) < .00001);
        QVERIFY(isEqual(x.infoContent(), infoContentNoErrorCorrection, .00001));
    }

    // Test B: small sample error correction
    {
        InfoContentDistribution x(charCountDistribution, possibleLetters, true);

        QCOMPARE(x.possibleLetters(), possibleLetters);
        QCOMPARE(x.smallSampleErrorCorrection(), true);
        QCOMPARE(x.length(), length);
        QCOMPARE(x.maxInfo(), log2(possibleLetters));
        QVERIFY(fabs(x.totalInfo() - totalInfoErrorCorrection) < .00001);
        QVERIFY(isEqual(x.infoContent(), infoContentErrorCorrection, .00001));
    }
}

void TestInfoContentDistribution::columnInfo()
{
    CharCountDistribution dist = ::charCountDistribution1();
    InfoContentDistribution x(dist, 4, false);

    double precision = .0001;
    QVERIFY(fabs(x.columnInfo(1) - 0.429049406) < precision);
    QVERIFY(fabs(x.columnInfo(2) - 2.) < precision);
    QVERIFY(fabs(x.columnInfo(3) - .700684314) < precision);

    x = InfoContentDistribution(dist, 4, true);
    QVERIFY(fabs(x.columnInfo(1) - 0.21264515) < precision);
    QVERIFY(fabs(x.columnInfo(2) - 1.783595744) < precision);
    QVERIFY(fabs(x.columnInfo(3) - 0.484280058) < precision);
}

QTEST_APPLESS_MAIN(TestInfoContentDistribution)
#include "TestInfoContentDistribution.moc"
