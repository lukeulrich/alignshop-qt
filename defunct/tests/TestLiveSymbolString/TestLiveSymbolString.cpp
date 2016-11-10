/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "BioSymbolGroup.h"
#include "LiveMsaCharCountDistribution.h"
#include "LiveSymbolString.h"
#include "Msa.h"

#include <QtDebug>

class TestLiveSymbolString : public QObject
{
    Q_OBJECT

private slots:
    void constructor();     // Also tests bioSymbolGroup and liveCharCountDistribution getters
    void symbolString();

    // Signal based changes
    void sourceColumnsInserted();
    void sourceColumnsRemoved();
    void sourceDataChanged();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
Msa *createMsa(const QStringList &subseqStringList)
{
    Msa *msa = new Msa;
    foreach (QString subseqString, subseqStringList)
    {
        AnonSeq anonSeq(0, subseqString);
        Subseq *subseq = new Subseq(anonSeq);
        if (!subseq->setBioString(subseqString))
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
    BioSymbolGroup bioSymbolGroup('_');
    bioSymbolGroup << BioSymbol('%', "ACGTN", .5);
    bioSymbolGroup << BioSymbol('p', "GP", .25);

    LiveSymbolString x(0, bioSymbolGroup);
    QVERIFY(x.liveCharCountDistribution() == 0);
    QCOMPARE(x.bioSymbolGroup().defaultSymbol(), '_');
    QCOMPARE(x.bioSymbolGroup().bioSymbols().value('%'), bioSymbolGroup.bioSymbols().value('%'));
    QCOMPARE(x.bioSymbolGroup().bioSymbols().value('p'), bioSymbolGroup.bioSymbols().value('p'));
    QCOMPARE(x.bioSymbolGroup().bioSymbols().count(), 2);

    LiveMsaCharCountDistribution *liveMsaCharCountDistribution = new LiveMsaCharCountDistribution(0);
    LiveSymbolString x2(liveMsaCharCountDistribution, BioSymbolGroup());
    QVERIFY(x2.liveCharCountDistribution() == liveMsaCharCountDistribution);
}

void TestLiveSymbolString::symbolString()
{
    // ------------------------------------------------------------------------
    // Test: Null liveCharCountDistribution = empty QString
    {
        LiveSymbolString x(0, BioSymbolGroup());
        QCOMPARE(x.symbolString(), QString());
    }

    // ------------------------------------------------------------------------
    // Test: Valid but empty LiveCharCountDistribution = empty QString
    {
        LiveMsaCharCountDistribution *liveMsaCharCountDistribution = new LiveMsaCharCountDistribution(0);
        LiveSymbolString x(liveMsaCharCountDistribution, BioSymbolGroup());
        QCOMPARE(x.symbolString(), QString());

        delete liveMsaCharCountDistribution;
        liveMsaCharCountDistribution = 0;
    }

    // ------------------------------------------------------------------------
    // Test: Valid, simple liveCharCountDistribution
    {
        QStringList subseqs;
        subseqs << "--AB--C-DEF"
                << "---B-XC-DE-";
        Msa *msa = createMsa(subseqs);

        LiveMsaCharCountDistribution msaDist(msa);
        BioSymbolGroup rules('_');
        rules << BioSymbol('a', "A", .5)
              << BioSymbol('b', "B", .5);

        LiveSymbolString x(&msaDist, rules);
        QCOMPARE(x.symbolString(), rules.calculateSymbolString(::divideListHashCharInt(msaDist.charCountDistribution().charCounts(), 2)));

        delete msa;
        msa = 0;
    }
}

void TestLiveSymbolString::sourceColumnsInserted()
{
    Msa *msa = createMsa(QStringList());

    LiveMsaCharCountDistribution msaDist(msa);
    BioSymbolGroup rules('_');
    rules << BioSymbol('a', "A", .5)
          << BioSymbol('b', "B", .5);

    LiveSymbolString x(&msaDist, rules);

    AnonSeq anonSeq(1, "ABCD");
    Subseq *subseq = new Subseq(anonSeq);

    QSignalSpy spySymbolsInserted(&x, SIGNAL(symbolsInserted(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: should insert columns and update symbol string
    msa->append(subseq);
    QCOMPARE(x.symbolString(), QString("ab__"));
    QCOMPARE(spySymbolsInserted.count(), 1);
    spyArguments = spySymbolsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // ------------------------------------------------------------------------
    // Test: insert a gap columns in the msa should update the symbol string column count
    msa->insertGapColumns(2, 3);
    QCOMPARE(x.symbolString(), QString("a___b__"));
    QCOMPARE(spySymbolsInserted.count(), 1);
    spyArguments = spySymbolsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    delete msa;
    msa = 0;
}

void TestLiveSymbolString::sourceColumnsRemoved()
{
    QStringList subseqs;
    subseqs << "-AAB--C--DEF"
            << "--AB-XC--D--"
            << "---B-XC--DE-";
    Msa *msa = createMsa(subseqs);

    LiveMsaCharCountDistribution msaDist(msa);
    BioSymbolGroup rules('_');
    rules << BioSymbol('a', "A", .5)
          << BioSymbol('b', "B", .5);

    LiveSymbolString x(&msaDist, rules);

    QSignalSpy spySymbolsRemoved(&x, SIGNAL(symbolsRemoved(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: remove gap columns should update symbol string
    QCOMPARE(x.symbolString(), QString("__ab________"));
    msa->removeGapColumns();
    QCOMPARE(x.symbolString(), QString("_ab_____"));
    QCOMPARE(spySymbolsRemoved.count(), 3);
    spyArguments = spySymbolsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 8);
    QCOMPARE(spyArguments.at(1).toInt(), 9);

    spyArguments = spySymbolsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    spyArguments = spySymbolsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);

    // ------------------------------------------------------------------------
    // Test: remove all subseqs, should call symbolsRemoved
    msa->clear();
    QCOMPARE(x.symbolString(), QString());
    QCOMPARE(spySymbolsRemoved.count(), 1);
    spyArguments = spySymbolsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 8);

    delete msa;
    msa = 0;
}

void TestLiveSymbolString::sourceDataChanged()
{
    QStringList subseqs;
    subseqs << "-AAB--"
            << "--AB-X"
            << "---A-X";
    Msa *msa = createMsa(subseqs);

    LiveMsaCharCountDistribution msaDist(msa);
    BioSymbolGroup rules('_');
    rules << BioSymbol('a', "A", .5)
          << BioSymbol('b', "B", .5);

    LiveSymbolString x(&msaDist, rules);

    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: slide a region and it should update the symbol string appropriately
    //
    //  -AAB--       -AAB--
    //  --AB-X  -->  -AB--X
    //  ---A-X       --A--X
    QCOMPARE(x.symbolString(), QString("__ab__"));
    msa->slideRegion(3, 2, 4, 3, -1);
    QCOMPARE(x.symbolString(), QString("_aa___"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // ------------------------------------------------------------------------
    // Test: slide a region and it should update the symbol string appropriately
    //
    //  -AAB--       -AAB--
    //  -AB--X  -->  -A-B-X
    //  --A--X       ---A-X
    msa->slideRegion(3, 2, 3, 3, 1);
    QCOMPARE(x.symbolString(), QString("_a_b__"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // ------------------------------------------------------------------------
    // Test: slide a region and it should update the symbol string appropriately
    //
    //  -AAB--       ---AAB
    //  -A-B-X  -->  -A-B-X
    //  ---A-X       ---A-X
    msa->slideRegion(2, 1, 4, 1, 2);
    QCOMPARE(x.symbolString(), QString("___a__"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 6);
}

QTEST_APPLESS_MAIN(TestLiveSymbolString)
#include "TestLiveSymbolString.moc"
