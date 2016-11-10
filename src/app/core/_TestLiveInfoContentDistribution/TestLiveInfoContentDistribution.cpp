/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../InfoContentDistribution.h"
#include "../LiveInfoContentDistribution.h"
#include "../_Mocks/MockCharCountDistributions.h"
#include "../_Mocks/MockLiveCharCountDistribution.h"

class TestLiveInfoContentDistribution : public QObject
{
    Q_OBJECT

public:
    TestLiveInfoContentDistribution()
    {
        qRegisterMetaType<ClosedIntRange>("ClosedIntRange");
    }

private slots:
    void constructor();
    void setSmallSampleErrorCorrection();
    void dataChangedSignal();
    void columnsRemovedSignal();
    void columnsInsertedSignal();
};
Q_DECLARE_METATYPE(ClosedIntRange)

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestLiveInfoContentDistribution::constructor()
{
    // Test: empty distribution
    {
        MockLiveCharCountDistribution mockLiveDist;
        LiveInfoContentDistribution x(&mockLiveDist, 4, true);
        QVERIFY(x.liveCharCountDistribution() == &mockLiveDist);

        QVERIFY(x.infoContent().isEmpty());
    }

    // Test: complex distribution - no small sample error correction
    {
        MockLiveCharCountDistribution mockLiveDist;
        mockLiveDist.setDistribution(::charCountDistribution2());
        LiveInfoContentDistribution x(&mockLiveDist, 4, false);
        QVERIFY(x.liveCharCountDistribution() == &mockLiveDist);

        InfoContentDistribution y(x.liveCharCountDistribution()->charCountDistribution(), 4, false);
        QVERIFY(isEqual(x.infoContent(), y.infoContent(), .00001));
    }

    // Test: complex distribution - small sample error correction
    {
        MockLiveCharCountDistribution mockLiveDist;
        mockLiveDist.setDistribution(::charCountDistribution2());
        LiveInfoContentDistribution x(&mockLiveDist, 4, true);
        QVERIFY(x.liveCharCountDistribution() == &mockLiveDist);

        InfoContentDistribution y(x.liveCharCountDistribution()->charCountDistribution(), 4, true);
        QVERIFY(isEqual(x.infoContent(), y.infoContent(), .00001));
    }
}

void TestLiveInfoContentDistribution::setSmallSampleErrorCorrection()
{
    // Test: empty distribution
    {
        MockLiveCharCountDistribution mockLiveDist;
        LiveInfoContentDistribution x(&mockLiveDist, 4, true);

        QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
        x.setSmallSampleErrorCorrection(true);
        x.setSmallSampleErrorCorrection(false);
        QCOMPARE(spyDataChanged.size(), 0);
    }

    // Test: complex distribution - no small sample error correction
    CharCountDistribution sourceDist = ::charCountDistribution2();
    {
        MockLiveCharCountDistribution mockLiveDist;
        mockLiveDist.setDistribution(sourceDist);
        LiveInfoContentDistribution x(&mockLiveDist, 4, false);

        QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
        x.setSmallSampleErrorCorrection(false);

        QVERIFY(spyDataChanged.isEmpty());

        x.setSmallSampleErrorCorrection(true);
        QCOMPARE(spyDataChanged.size(), 1);
        QCOMPARE(qvariant_cast<ClosedIntRange>(spyDataChanged.at(0).at(0)), ClosedIntRange(1, sourceDist.length()));
    }

    // Test: complex distribution
    {
        MockLiveCharCountDistribution mockLiveDist;
        mockLiveDist.setDistribution(sourceDist);
        LiveInfoContentDistribution x(&mockLiveDist, 4, true);

        QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
        x.setSmallSampleErrorCorrection(true);

        QVERIFY(spyDataChanged.isEmpty());

        x.setSmallSampleErrorCorrection(false);
        QCOMPARE(spyDataChanged.size(), 1);
        QCOMPARE(qvariant_cast<ClosedIntRange>(spyDataChanged.at(0).at(0)), ClosedIntRange(1, sourceDist.length()));
    }
}

void TestLiveInfoContentDistribution::dataChangedSignal()
{
    CharCountDistribution sourceDist = ::charCountDistribution2();

    MockLiveCharCountDistribution mockLiveDist;
    mockLiveDist.setDistribution(sourceDist);
    LiveInfoContentDistribution x(&mockLiveDist, 4, true);

    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));

    mockLiveDist.addData(QByteArray("ACT"), 2);
    QCOMPARE(spyDataChanged.size(), 1);
    QCOMPARE(qvariant_cast<ClosedIntRange>(spyDataChanged.at(0).at(0)), ClosedIntRange(2, 4));

    sourceDist.add(QByteArray("ACT"), '\0', 2);
    InfoContentDistribution y(sourceDist, 4, true);
    QVERIFY(isEqual(x.infoContent(), y.infoContent(), .00001));
}

void TestLiveInfoContentDistribution::columnsRemovedSignal()
{
    CharCountDistribution sourceDist = ::charCountDistribution2();

    MockLiveCharCountDistribution mockLiveDist;
    mockLiveDist.setDistribution(sourceDist);
    LiveInfoContentDistribution x(&mockLiveDist, 4, true);

    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(ClosedIntRange)));

    mockLiveDist.remove(5, 2);
    QCOMPARE(spyColumnsRemoved.size(), 1);
    QCOMPARE(qvariant_cast<ClosedIntRange>(spyColumnsRemoved.at(0).at(0)), ClosedIntRange(5, 6));

    sourceDist.remove(5, 2);
    InfoContentDistribution y(sourceDist, 4, true);
    QVERIFY(isEqual(x.infoContent(), y.infoContent(), .00001));
}

void TestLiveInfoContentDistribution::columnsInsertedSignal()
{
    CharCountDistribution sourceDist = ::charCountDistribution2();

    MockLiveCharCountDistribution mockLiveDist;
    mockLiveDist.setDistribution(sourceDist);
    LiveInfoContentDistribution x(&mockLiveDist, 4, true);

    QSignalSpy spyColumnsInserted(&x, SIGNAL(columnsInserted(ClosedIntRange)));

    mockLiveDist.insert(3, 3);
    QCOMPARE(spyColumnsInserted.size(), 1);
    QCOMPARE(qvariant_cast<ClosedIntRange>(spyColumnsInserted.at(0).at(0)), ClosedIntRange(3, 5));

    sourceDist.insertBlanks(3, 3);
    InfoContentDistribution y(sourceDist, 4, true);
    QVERIFY(isEqual(x.infoContent(), y.infoContent(), .00001));
}

QTEST_APPLESS_MAIN(TestLiveInfoContentDistribution)
#include "TestLiveInfoContentDistribution.moc"
