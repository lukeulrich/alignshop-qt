/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../BioSymbol.h"
#include "../BioSymbolGroup.h"
#include "../LiveMsaCharCountDistribution.h"
#include "../LiveSymbolString.h"
#include "../ObservableMsa.h"
#include "../Services/SymbolStringCalculator.h"
#include "../global.h"
#include "../misc.h"

#include <QtDebug>

class TestLiveSymbolString : public QObject
{
    Q_OBJECT

public:
    TestLiveSymbolString()
    {
        qRegisterMetaType<ClosedIntRange>("ClosedIntRange");
    }

private slots:
    void constructor();     // Also tests calculator and liveCharCountDistribution getters
    void symbolString();

    // Signal based changes
    void sourceColumnsInserted();
    void sourceColumnsRemoved();
    void sourceDataChanged();
};

Q_DECLARE_METATYPE(ClosedIntRange);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
ObservableMsa *createMsa(const QStringList &subseqStringList)
{
    ObservableMsa *msa = new ObservableMsa;
    foreach (QString subseqString, subseqStringList)
    {
        Seq seq(subseqString.toAscii());
        Subseq *subseq = new Subseq(seq);
        if (!subseq->setBioString(subseqString.toAscii()))
        {
            delete msa;
            return 0;
        }

        if (!msa->append(subseq))
        {
            delete msa;
            return 0;
        }
    }

    return msa;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestLiveSymbolString::constructor()
{
    BioSymbolGroup bioSymbolGroup;
    bioSymbolGroup << BioSymbol('%', "ACGTN", .5);
    bioSymbolGroup << BioSymbol('p', "GP", .25);

    SymbolStringCalculator calculator(bioSymbolGroup, '_');

    LiveSymbolString x(nullptr, calculator);
    QVERIFY(x.liveCharCountDistribution() == nullptr);
    QCOMPARE(x.symbolStringCalculator(), calculator);

    LiveMsaCharCountDistribution *liveMsaCharCountDistribution = new LiveMsaCharCountDistribution(nullptr);
    LiveSymbolString x2(liveMsaCharCountDistribution, calculator);
    QVERIFY(x2.liveCharCountDistribution() == liveMsaCharCountDistribution);
}

void TestLiveSymbolString::symbolString()
{
    // ------------------------------------------------------------------------
    // Test: Null liveCharCountDistribution = empty QString
    {
        LiveSymbolString x(nullptr, SymbolStringCalculator(BioSymbolGroup()));
        QVERIFY(x.symbolString().isEmpty());
    }

    // ------------------------------------------------------------------------
    // Test: Valid but empty LiveCharCountDistribution = empty QString
    {
        LiveMsaCharCountDistribution *liveMsaCharCountDistribution = new LiveMsaCharCountDistribution(nullptr);
        LiveSymbolString x(liveMsaCharCountDistribution, SymbolStringCalculator(BioSymbolGroup()));
        QVERIFY(x.symbolString().isEmpty());

        delete liveMsaCharCountDistribution;
        liveMsaCharCountDistribution = 0;
    }

    // ------------------------------------------------------------------------
    // Test: Valid, simple liveCharCountDistribution
    {
        QStringList subseqs;
        subseqs << "--AB--C-DEF"
                << "---B-XC-DE-";
        ObservableMsa *msa = createMsa(subseqs);

        LiveMsaCharCountDistribution msaDist(msa);
        BioSymbolGroup rules;
        rules << BioSymbol('a', "A", .5)
              << BioSymbol('b', "B", .5);
        SymbolStringCalculator calculator(rules, '_');

        LiveSymbolString x(&msaDist, calculator);
        QCOMPARE(x.symbolString(), calculator.computeSymbolString(::divideVectorHashCharInt(msaDist.charCountDistribution().charCounts(), 2)));

        delete msa;
        msa = 0;
    }
}

void TestLiveSymbolString::sourceColumnsInserted()
{
    ObservableMsa *msa = createMsa(QStringList());

    LiveMsaCharCountDistribution msaDist(msa);
    BioSymbolGroup rules;
    rules << BioSymbol('a', "A", .5)
          << BioSymbol('b', "B", .5);
    SymbolStringCalculator calculator(rules, '_');
    LiveSymbolString x(&msaDist, calculator);

    Seq seq("ABCD");
    Subseq *subseq = new Subseq(seq);

    QSignalSpy spySymbolsInserted(&x, SIGNAL(symbolsInserted(ClosedIntRange)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: should insert columns and update symbol string
    QVERIFY(msa->append(subseq));
    QCOMPARE(x.symbolString(), QByteArray("ab__"));
    QCOMPARE(spySymbolsInserted.count(), 1);
    spyArguments = spySymbolsInserted.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 4));

    // ------------------------------------------------------------------------
    // Test: insert a gap columns in the msa should update the symbol string column count
    msa->insertGapColumns(2, 3);
    QCOMPARE(x.symbolString(), QByteArray("a___b__"));
    QCOMPARE(spySymbolsInserted.count(), 1);
    spyArguments = spySymbolsInserted.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(2, 4));

    delete msa;
    msa = 0;
}

void TestLiveSymbolString::sourceColumnsRemoved()
{
    QStringList subseqs;
    subseqs << "-AAB--C--DEF"
            << "--AB-XC--D--"
            << "---B-XC--DE-";
    ObservableMsa *msa = createMsa(subseqs);

    LiveMsaCharCountDistribution msaDist(msa);
    BioSymbolGroup rules;
    rules << BioSymbol('a', "A", .5)
          << BioSymbol('b', "B", .5);
    SymbolStringCalculator calculator(rules, '_');
    LiveSymbolString x(&msaDist, calculator);

    QSignalSpy spySymbolsRemoved(&x, SIGNAL(symbolsRemoved(ClosedIntRange)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: remove gap columns should update symbol string
    QCOMPARE(x.symbolString(), QByteArray("__ab________"));
    msa->removeGapColumns();
    QCOMPARE(x.symbolString(), QByteArray("_ab_____"));
    QCOMPARE(spySymbolsRemoved.count(), 3);
    spyArguments = spySymbolsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(8, 9));

    spyArguments = spySymbolsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(5, 5));

    spyArguments = spySymbolsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 1));

    // ------------------------------------------------------------------------
    // Test: remove all subseqs, should call symbolsRemoved
    msa->clear();
    QCOMPARE(x.symbolString(), QByteArray());
    QCOMPARE(spySymbolsRemoved.count(), 1);
    spyArguments = spySymbolsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 8));

    delete msa;
    msa = 0;
}

void TestLiveSymbolString::sourceDataChanged()
{
    QStringList subseqs;
    subseqs << "-AAB--"
            << "--AB-X"
            << "---A-X";
    ObservableMsa *msa = createMsa(subseqs);

    LiveMsaCharCountDistribution msaDist(msa);
    BioSymbolGroup rules;
    rules << BioSymbol('a', "A", .5)
          << BioSymbol('b', "B", .5);

    SymbolStringCalculator calculator(rules, '_');
    LiveSymbolString x(&msaDist, calculator);

    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: slide a region and it should update the symbol string appropriately
    //
    //  -AAB--       -AAB--
    //  --AB-X  -->  -AB--X
    //  ---A-X       --A--X
    QCOMPARE(x.symbolString(), QByteArray("__ab__"));
    msa->slideRect(PosiRect(3, 2, 2, 2), -1);
    QCOMPARE(x.symbolString(), QByteArray("_aa___"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(2, 4));

    // ------------------------------------------------------------------------
    // Test: slide a region and it should update the symbol string appropriately
    //
    //  -AAB--       -AAB--
    //  -AB--X  -->  -A-B-X
    //  --A--X       ---A-X
    msa->slideRect(PosiRect(3, 2, 1, 2), 1);
    QCOMPARE(x.symbolString(), QByteArray("_a_b__"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(3, 4));

    // ------------------------------------------------------------------------
    // Test: slide a region and it should update the symbol string appropriately
    //
    //  -AAB--       ---AAB
    //  -A-B-X  -->  -A-B-X
    //  ---A-X       ---A-X
    msa->slideRect(PosiRect(2, 1, 3, 1), 2);
    QCOMPARE(x.symbolString(), QByteArray("___a__"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(2, 6));
}

QTEST_APPLESS_MAIN(TestLiveSymbolString)
#include "TestLiveSymbolString.moc"
