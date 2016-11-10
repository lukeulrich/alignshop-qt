/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "LiveMsaCharCountDistribution.h"


#include "AnonSeq.h"
#include "BioString.h"
#include "CharCountDistribution.h"
#include "Msa.h"
#include "MsaAlgorithms.h"

#include "global.h"

typedef QPair<int, int> PairInt;
Q_DECLARE_METATYPE(PairInt);

class TestLiveMsaCharCountDistribution : public QObject
{
    Q_OBJECT

public:
    TestLiveMsaCharCountDistribution() : QObject()
    {
        qRegisterMetaType<ListHashCharInt>("ListHashCharInt");
        qRegisterMetaType<PairInt>("PairInt");
    }

private slots:
    void constructor_data();
    void constructor();
    void divisor();

    // Check that distribution is updated when the msa is changed
    void msaGapsInserted_data();
    void msaGapsInserted();
    void msaGapsRemoved();
    void msaReset();
    void msaSubseqInsertedRemoved();
    void msaRegionSlid_data();
    void msaRegionSlid();
    void msaExtendTrim_data();
    void msaExtendTrim();
    void msaCollapse();

private:
    Msa *createMsa(const QStringList &subseqs) const;   // Helper function for creating a Msa from subseqs
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
Msa *TestLiveMsaCharCountDistribution::createMsa(const QStringList &subseqStringList) const
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
void TestLiveMsaCharCountDistribution::constructor_data()
{
    QTest::addColumn<Msa *>("msa");
    QTest::addColumn<ListHashCharInt>("expectedCounts");

    QTest::newRow("Null msa") << ((Msa *)0) << ListHashCharInt();
    QTest::newRow("Empty msa") << new Msa() << ListHashCharInt();

    Msa *msa = createMsa(QStringList() << "ABC");
    QTest::newRow("Single sequence Msa") << msa << calculateMsaCharCountDistribution(*msa);

    QStringList subseqs;
    subseqs << "--AB--C-DEF"
            << "---B-XC-DE-";
    msa = createMsa(subseqs);
    QTest::newRow("Two sequence MSA") << msa << calculateMsaCharCountDistribution(*msa);

    subseqs << "ZYAB-XC--EF";
    msa = createMsa(subseqs);
    QTest::newRow("Three sequence MSA") << msa << calculateMsaCharCountDistribution(*msa);
}

void TestLiveMsaCharCountDistribution::constructor()
{
    QFETCH(Msa *, msa);
    QFETCH(ListHashCharInt, expectedCounts);

    LiveMsaCharCountDistribution x(msa);

    QVERIFY(x.msa() == msa);
    QCOMPARE(x.charCountDistribution().charCounts(), expectedCounts);

    if (msa)
    {
        delete msa;
        msa = 0;
    }
}

void TestLiveMsaCharCountDistribution::divisor()
{
    LiveMsaCharCountDistribution x(0);
    QCOMPARE(x.divisor(), 0);

    QStringList subseqs;
    subseqs << "--AB--C-DEF"
            << "---B-XC-DE-";
    Msa *msa = createMsa(subseqs);
    LiveMsaCharCountDistribution x2(msa);
    QCOMPARE(x2.divisor(), 2);

    AnonSeq anonSeq(5, "--AB--C-DEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("--AB--C-DEF");
    msa->append(subseq);

    QCOMPARE(x2.divisor(), 3);

    msa->removeAt(3);
    QCOMPARE(x2.divisor(), 2);

    msa->removeAt(2);
    QCOMPARE(x2.divisor(), 1);

    msa->removeAt(1);
    QCOMPARE(x2.divisor(), 0);

    delete msa;
    msa = 0;
}

void TestLiveMsaCharCountDistribution::msaGapsInserted_data()
{
    QTest::addColumn<QStringList>("sourceMsaSubseqs");
    QTest::addColumn<int>("insertGapPosition");
    QTest::addColumn<int>("nGaps");
    QTest::addColumn<int>("signalColumnFirst");
    QTest::addColumn<int>("signalColumnLast");
    QTest::addColumn<QStringList>("expectedMsaSubseqs");

    QStringList sourceMsaSubseqs;
    sourceMsaSubseqs << "ABC"
                     << "A-C"
                     << "-BC";

    QTest::newRow("insert single gap at end")
            << sourceMsaSubseqs
            << 4
            << 1
            << 4
            << 4
            << (QStringList() << "ABC-" << "A-C-" << "-BC-");

    QTest::newRow("insert two gaps in middle")
            << sourceMsaSubseqs
            << 2
            << 2
            << 2
            << 3
            << (QStringList() << "A--BC" << "A---C" << "---BC");

    QTest::newRow("insert three gaps at beginning")
            << sourceMsaSubseqs
            << 1
            << 3
            << 1
            << 3
            << (QStringList() << "---ABC" << "---A-C" << "----BC");
}

void TestLiveMsaCharCountDistribution::msaGapsInserted()
{
    QFETCH(QStringList, sourceMsaSubseqs);
    QFETCH(int, insertGapPosition);
    QFETCH(int, nGaps);
    QFETCH(int, signalColumnFirst);
    QFETCH(int, signalColumnLast);
    QFETCH(QStringList, expectedMsaSubseqs);

    Msa *msa = createMsa(sourceMsaSubseqs);
    QVERIFY(msa);

    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyGapsInserted(&x, SIGNAL(columnsInserted(int,int)));
    QVariantList spyArguments;

    msa->insertGapColumns(insertGapPosition, nGaps);
    QCOMPARE(spyGapsInserted.count(), 1);
    spyArguments = spyGapsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), signalColumnFirst);
    QCOMPARE(spyArguments.at(1).toInt(), signalColumnLast);

    Msa *expectedMsa = createMsa(expectedMsaSubseqs);
    QVERIFY(expectedMsa);
    /*
    ListHashCharInt result = calculateMsaCharCountDistribution(*expectedMsa);
    if (result != x.charCountDistribution().charCounts())
    {
        QStringList bob;
        for (int i=0; i< msa->subseqCount(); ++i)
            bob << msa->at(i+1)->bioString().sequence();

        qDebug() << "\n" << bob.join("\n");
        qDebug() << "Expected" << result;
        qDebug() << "Actual" << x.charCountDistribution().charCounts();
    }
    */

    QCOMPARE(x.charCountDistribution().charCounts(), calculateMsaCharCountDistribution(*expectedMsa));
    delete expectedMsa;
    expectedMsa = 0;

    delete msa;
    msa = 0;
}

void TestLiveMsaCharCountDistribution::msaGapsRemoved()
{
    QStringList sourceMsaSubseqs;
    sourceMsaSubseqs << "ABC"
                     << "A-B";

    Msa *expectedMsa = createMsa(sourceMsaSubseqs);
    ListHashCharInt expectedResult = calculateMsaCharCountDistribution(*expectedMsa);

    Msa *msa = createMsa(sourceMsaSubseqs);
    QVERIFY(msa);

    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(int,int)));
    QVariantList spyArguments;

    msa->removeGapColumns();
    QVERIFY(spyColumnsRemoved.isEmpty());

    // ------------------------------------------------------------------------
    // Test: Removing 2 gap columns from beginning
    msa->insertGapColumns(1, 2);
    msa->removeGapColumns();
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(x.charCountDistribution().charCounts(), expectedResult);

    // ------------------------------------------------------------------------
    // Test: Removing 1 gap columns from middle
    msa->insertGapColumns(2, 1);
    msa->removeGapColumns();
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(x.charCountDistribution().charCounts(), expectedResult);

    // ------------------------------------------------------------------------
    // Test: Removing 3 gaps columns from end
    msa->insertGapColumns(4, 3);
    msa->removeGapColumns();
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 6);
    QCOMPARE(x.charCountDistribution().charCounts(), expectedResult);

    // ------------------------------------------------------------------------
    // Test: Removing 1 gap columns from beginning, 2 from middle, 3 from end
    msa->insertGapColumns(4, 3);
    msa->insertGapColumns(2, 2);
    msa->insertGapColumns(1, 1);
    msa->removeGapColumns();
    QCOMPARE(spyColumnsRemoved.count(), 3);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 7);
    QCOMPARE(spyArguments.at(1).toInt(), 9);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);

    QCOMPARE(x.charCountDistribution().charCounts(), expectedResult);
}

void TestLiveMsaCharCountDistribution::msaReset()
{
    QStringList sourceMsaSubseqs;
    sourceMsaSubseqs << "ABC"
                     << "A-B";

    Msa *msa = new Msa();
    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(int,int)));

    // ------------------------------------------------------------------------
    // Test: clearing empty msa - should not emit any columns removed signal
    msa->clear();
    QVERIFY(spyColumnsRemoved.isEmpty());

    delete msa;
    msa = 0;

    // ------------------------------------------------------------------------
    // Test: clearing non-empty msa
    msa = createMsa(sourceMsaSubseqs);
    LiveMsaCharCountDistribution x2(msa);
    QSignalSpy spyColumnsRemoved2(&x2, SIGNAL(columnsRemoved(int,int)));
    QVariantList spyArguments;
    QVERIFY(msa);

    msa->clear();

    QCOMPARE(spyColumnsRemoved2.count(), 1);
    spyArguments = spyColumnsRemoved2.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(x2.charCountDistribution().charCounts(), ListHashCharInt());
}

void TestLiveMsaCharCountDistribution::msaSubseqInsertedRemoved()
{
    AnonSeq anonSeq1(0, "ABC");
    Subseq *subseq1 = new Subseq(anonSeq1);

    AnonSeq anonSeq2(0, "AC");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("A-C");

    Msa *msa = new Msa();

    LiveMsaCharCountDistribution x(msa);

    QSignalSpy spyColumnsInserted(&x, SIGNAL(columnsInserted(int,int)));
    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(int,int)));
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: appending subseq to empty msa
    msa->append(subseq1);
    QVERIFY(spyDataChanged.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QCOMPARE(spyColumnsInserted.count(), 1);
    spyArguments = spyColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(x.charCountDistribution().charCounts(), calculateMsaCharCountDistribution(*msa));

    // ------------------------------------------------------------------------
    // Test: appending subseq to non-empty msa
    QVERIFY(msa->append(subseq2));
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(x.charCountDistribution().charCounts(), calculateMsaCharCountDistribution(*msa));

    // ------------------------------------------------------------------------
    // Test: removing subseq from non-empty msa
    msa->removeAt(2);
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(x.charCountDistribution().charCounts(), calculateMsaCharCountDistribution(*msa));

    // ------------------------------------------------------------------------
    // Test: removing subseq from non-empty msa -> empty msa
    msa->removeAt(1);
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyDataChanged.isEmpty());
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(x.charCountDistribution().charCounts(), ListHashCharInt());

    delete msa;
    msa = 0;
}

void TestLiveMsaCharCountDistribution::msaRegionSlid_data()
{
    QTest::addColumn<QStringList>("subseqs");
    QTest::addColumn<QRect>("region");
    QTest::addColumn<int>("delta");
    QTest::addColumn<PairInt>("dataChanged");
    QTest::addColumn<QStringList>("finalSubseqs");

    // ------------------------------------------------------------------------
    QStringList subseqs;
    subseqs << "--AB-C-D";
    QStringList finalSubseqs;
    finalSubseqs << "---ABC-D";
    QTest::newRow("single sequence slide - positive delta (+1)")
            << subseqs
            << QRect(QPoint(3, 1), QPoint(4, 1))    // AB
            << 1
            << qMakePair(3, 5)
            << finalSubseqs;
    finalSubseqs.clear();

    finalSubseqs << "AB---C-D";
    QTest::newRow("single sequence slide - negative delta (-2)")
            << subseqs
            << QRect(QPoint(3, 1), QPoint(4, 1))    // AB
            << -2
            << qMakePair(1, 4)
            << finalSubseqs;
    finalSubseqs.clear();
    subseqs.clear();

    // ------------------------------------------------------------------------
    // Setup
    subseqs << "-A-B-C--"
            << "-D-E-F--"
            << "-G-H-I--";

    // ------------------------------------------------------------------------
    finalSubseqs << "-A-B-C--"
                 << "-D---E-F"
                 << "-G---H-I";
    QTest::newRow("two sequence slide, containing gaps - positive delta (+2)")
            << subseqs
            << QRect(QPoint(4, 2), QPoint(6, 3))
            << 2
            << qMakePair(4, 8)
            << finalSubseqs;
    finalSubseqs.clear();

    // ------------------------------------------------------------------------
    finalSubseqs << "-A-B---C"
                 << "-D-E---F"
                 << "-G-H---I";
    QTest::newRow("three sequence slide, single column - positive delta (+2)")
            << subseqs
            << QRect(QPoint(6, 1), QPoint(6, 3))
            << 2
            << qMakePair(6, 8)
            << finalSubseqs;
    finalSubseqs.clear();

    // ------------------------------------------------------------------------
    finalSubseqs << "A-B--C--"
                 << "D-E--F--"
                 << "G-H--I--";
    QTest::newRow("three sequence slide, containing gaps - negative delta (-1)")
            << subseqs
            << QRect(QPoint(2, 1), QPoint(5, 3))
            << -1
            << qMakePair(1, 5)
            << finalSubseqs;
    finalSubseqs.clear();
}

void TestLiveMsaCharCountDistribution::msaRegionSlid()
{
    QFETCH(QStringList, subseqs);
    QFETCH(QRect, region);
    QFETCH(int, delta);
    QFETCH(PairInt, dataChanged);
    QFETCH(QStringList, finalSubseqs);

    Msa *msa = createMsa(subseqs);
    QVERIFY(msa);
    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(int, int)));
    QVariantList spyArguments;

    Msa *finalMsa = createMsa(finalSubseqs);
    QVERIFY(finalMsa);

    QCOMPARE(msa->slideRegion(region.left(), region.top(), region.right(), region.bottom(), delta), delta);

    if (x.charCountDistribution().charCounts() != calculateMsaCharCountDistribution(*finalMsa))
    {
        qDebug() << "\n" << finalSubseqs.join("\n");
        qDebug() << "Expected" << calculateMsaCharCountDistribution(*finalMsa);
        qDebug() << "Actual" << x.charCountDistribution().charCounts();
    }

    QCOMPARE(x.charCountDistribution().charCounts(), calculateMsaCharCountDistribution(*finalMsa));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), dataChanged.first);
    QCOMPARE(spyArguments.at(1).toInt(), dataChanged.second);

    delete msa;
    msa = 0;

    delete finalMsa;
    finalMsa = 0;
}

void TestLiveMsaCharCountDistribution::msaExtendTrim_data()
{
    QTest::addColumn<QStringList>("subseqs");
    QTest::addColumn<int>("subseqIndex");
    QTest::addColumn<int>("start");
    QTest::addColumn<int>("stop");
    QTest::addColumn<PairInt>("dataChanged");
    QTest::addColumn<QStringList>("finalSubseqs");

    // NOTE!! It is vital that only the characters ABCD are used because of a hack used in the actual test
    // method below
    QStringList subseqs, finalSubseqs;
    subseqs << "--ABCD--"
            << "-A-B-C-D";
    finalSubseqs << "---BCD--"
                 << "-A-B-C-D";
    QTest::newRow("Subseq 1, increasing start") << subseqs << 1 << 2 << 0 << qMakePair(3, 3) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "-A-B-C-D";
    QTest::newRow("Subseq 1, decreasing start") << subseqs << 1 << 1 << 0 << qMakePair(3, 3) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABC---"
                 << "-A-B-C-D";
    QTest::newRow("Subseq 1, decreasing stop") << subseqs << 1 << 0 << 3 << qMakePair(6, 6) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "-A-B-C-D";
    QTest::newRow("Subseq 1, increasing stop") << subseqs << 1 << 0 << 4 << qMakePair(6, 6) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "-----C-D";
    QTest::newRow("Subseq 2, increasing start") << subseqs << 2 << 3 << 0 << qMakePair(2, 4) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "---ABC-D";
    QTest::newRow("Subseq 2, decreasing start") << subseqs << 2 << 1 << 0 << qMakePair(4, 5) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "---ABC--";
    QTest::newRow("Subseq 2, decreasing stop") << subseqs << 2 << 0 << 3 << qMakePair(8, 8) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "---ABCD-";
    QTest::newRow("Subseq 2, increasing stop") << subseqs << 2 << 0 << 4 << qMakePair(7, 7) << finalSubseqs;
}

void TestLiveMsaCharCountDistribution::msaExtendTrim()
{
    QFETCH(QStringList, subseqs);
    QFETCH(int, subseqIndex);
    QFETCH(int, start);
    QFETCH(int, stop);
    QFETCH(PairInt, dataChanged);
    QFETCH(QStringList, finalSubseqs);

    Msa *msa = new Msa;
    AnonSeq anonSeq(0, "ABCD");
    foreach (QString sequence, subseqs)
    {
        Subseq *subseq = new Subseq(anonSeq);
        QVERIFY(subseq->setBioString(sequence));

        msa->append(subseq);
    }

    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(int, int)));
    QVariantList spyArguments;

    Msa *finalMsa = createMsa(finalSubseqs);
    QVERIFY(finalMsa);

    if (start != 0)
        msa->setSubseqStart(subseqIndex, start);
    else
        msa->setSubseqStop(subseqIndex, stop);

    if (x.charCountDistribution().charCounts() != calculateMsaCharCountDistribution(*finalMsa))
    {
        qDebug() << "\n" << finalSubseqs.join("\n");
        qDebug() << "Expected" << calculateMsaCharCountDistribution(*finalMsa);
        qDebug() << "Actual" << x.charCountDistribution().charCounts();
    }

    QCOMPARE(x.charCountDistribution().charCounts(), calculateMsaCharCountDistribution(*finalMsa));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), dataChanged.first);
    QCOMPARE(spyArguments.at(1).toInt(), dataChanged.second);

    delete msa;
    msa = 0;

    delete finalMsa;
    finalMsa = 0;
}

void TestLiveMsaCharCountDistribution::msaCollapse()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq1 = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq);
    Subseq *subseq3 = new Subseq(anonSeq);

    Msa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QStringList seqs;
    seqs << "A-BC-D-E"
         << "-A-BCD--"
         << "--A---BC";

    MsaRect msaRect;
    int msaLength = seqs.at(0).length();
    int nSeqs = seqs.length();
    for (int top=1; top<= nSeqs; ++top)
    {
        for (int left=1; left<= msaLength; ++left)
        {
            for (int bottom=top; bottom<= nSeqs; ++bottom)
            {
                for (int right=left; right<= msaLength; ++right)
                {
                    msaRect.setTop(top);
                    msaRect.setLeft(left);
                    msaRect.setRight(right);
                    msaRect.setBottom(bottom);

                    // -----------------------------------------
                    // Test 1: collapseLeft
                    {
                        // Reset the alignment to its initial state
                        QVERIFY(subseq1->setBioString(seqs.at(0)));
                        QVERIFY(subseq2->setBioString(seqs.at(1)));
                        QVERIFY(subseq3->setBioString(seqs.at(2)));

                        LiveMsaCharCountDistribution x(&msa);
                        QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(int, int)));
                        QSignalSpy spySubseqInternallyChanged(&msa, SIGNAL(subseqInternallyChanged(int,int,QString,QString)));
                        QVariantList spyArguments;

                        // Assuming that the collapseLeft function works properly and has been tested
                        msa.collapseLeft(msaRect);

                        // Check that the char count distribution is as expected
                        if (x.charCountDistribution().charCounts() != calculateMsaCharCountDistribution(msa))
                        {
                            qDebug() << "\n" << subseq1->bioString().sequence() << "\n"
                                     << subseq2->bioString().sequence() << "\n"
                                     << subseq3->bioString().sequence();
                            qDebug() << "Expected" << calculateMsaCharCountDistribution(msa);
                            qDebug() << "Actual" << x.charCountDistribution().charCounts();
                        }
                        QCOMPARE(x.charCountDistribution().charCounts(), calculateMsaCharCountDistribution(msa));

                        // Check the signals
                        if (spySubseqInternallyChanged.isEmpty())
                        {
                            QVERIFY(spyDataChanged.isEmpty());
                            continue;
                        }

                        // Else at least one sequence was modified, check that we get a change event for these columns
                        QVERIFY(spySubseqInternallyChanged.count() == spyDataChanged.count());

                        for (int i=0, z= spySubseqInternallyChanged.count(); i<z; ++i)
                        {
                            spyArguments = spySubseqInternallyChanged.takeFirst();
                            QVariantList dataChangedArguments = spyDataChanged.takeFirst();

                            int startColumn = spyArguments.at(1).toInt();
                            QCOMPARE(startColumn, dataChangedArguments.at(0).toInt());

                            int endColumn = startColumn + spyArguments.at(2).toString().length() - 1;
                            QCOMPARE(endColumn, dataChangedArguments.at(1).toInt());
                        }
                    }

                    // -----------------------------------------
                    // Test 2: collapseRight
                    {
                        // Reset the alignment to its initial state
                        QVERIFY(subseq1->setBioString(seqs.at(0)));
                        QVERIFY(subseq2->setBioString(seqs.at(1)));
                        QVERIFY(subseq3->setBioString(seqs.at(2)));

                        LiveMsaCharCountDistribution x(&msa);
                        QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(int, int)));
                        QSignalSpy spySubseqInternallyChanged(&msa, SIGNAL(subseqInternallyChanged(int,int,QString,QString)));
                        QVariantList spyArguments;

                        // Assuming that the collapseLeft function works properly and has been tested
                        msa.collapseRight(msaRect);

                        // Check that the char count distribution is as expected
                        QCOMPARE(x.charCountDistribution().charCounts(), calculateMsaCharCountDistribution(msa));

                        // Check the signals
                        if (spySubseqInternallyChanged.isEmpty())
                        {
                            QVERIFY(spyDataChanged.isEmpty());
                            continue;
                        }

                        // Else at least one sequence was modified, check that we get a change event for these columns
                        QVERIFY(spySubseqInternallyChanged.count() == spyDataChanged.count());

                        for (int i=0, z= spySubseqInternallyChanged.count(); i<z; ++i)
                        {
                            spyArguments = spySubseqInternallyChanged.takeFirst();
                            QVariantList dataChangedArguments = spyDataChanged.takeFirst();

                            int startColumn = spyArguments.at(1).toInt();
                            QCOMPARE(startColumn, dataChangedArguments.at(0).toInt());

                            int endColumn = startColumn + spyArguments.at(2).toString().length() - 1;
                            QCOMPARE(endColumn, dataChangedArguments.at(1).toInt());
                        }
                    }
                }
            }
        }
    }
}

QTEST_APPLESS_MAIN(TestLiveMsaCharCountDistribution)
#include "TestLiveMsaCharCountDistribution.moc"
