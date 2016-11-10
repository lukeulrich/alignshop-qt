/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../LiveMsaCharCountDistribution.h"
#include "../Seq.h"
#include "../BioString.h"
#include "../CharCountDistribution.h"
#include "../ObservableMsa.h"
#include "../util/MsaAlgorithms.h"

#include "../global.h"

typedef QPair<int, int> PairInt;
Q_DECLARE_METATYPE(PairInt);

class TestLiveMsaCharCountDistribution : public QObject
{
    Q_OBJECT

public:
    TestLiveMsaCharCountDistribution() : QObject()
    {
        qRegisterMetaType<VectorHashCharInt>("VectorHashCharInt");
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
    void msaRectangleSlid_data();
    void msaRectangleSlid();
    void msaExtendTrim_data();
    void msaExtendTrim();
    void msaCollapse();

//    void msaExtendAndTrimOp();  // Tests cases where the subseqChangePod has an operation of ExtendLeftTrimRight or
                                  // ExtendRightTrimLeft

private:
    ObservableMsa *createMsa(const QStringList &subseqs) const;   // Helper function for creating a ObservableMsa from subseqs
};

Q_DECLARE_METATYPE(ObservableMsa *);
Q_DECLARE_METATYPE(VectorHashCharInt);
Q_DECLARE_METATYPE(ClosedIntRange);
Q_DECLARE_METATYPE(PosiRect);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper functions
ObservableMsa *TestLiveMsaCharCountDistribution::createMsa(const QStringList &subseqStringList) const
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
void TestLiveMsaCharCountDistribution::constructor_data()
{
    QTest::addColumn<ObservableMsa *>("msa");
    QTest::addColumn<VectorHashCharInt>("expectedCounts");

    QTest::newRow("Null msa") << ((ObservableMsa *)0) << VectorHashCharInt();
    QTest::newRow("Empty msa") << new ObservableMsa() << VectorHashCharInt();

    ObservableMsa *msa = createMsa(QStringList() << "ABC");
    QTest::newRow("Single sequence ObservableMsa") << msa << ::calculateMsaCharCountDistribution(*msa).charCounts();

    QStringList subseqs;
    subseqs << "--AB--C-DEF"
            << "---B-XC-DE-";
    msa = createMsa(subseqs);
    QTest::newRow("Two sequence MSA") << msa << ::calculateMsaCharCountDistribution(*msa).charCounts();

    subseqs << "ZYAB-XC--EF";
    msa = createMsa(subseqs);
    QTest::newRow("Three sequence MSA") << msa << ::calculateMsaCharCountDistribution(*msa).charCounts();
}

void TestLiveMsaCharCountDistribution::constructor()
{
    QFETCH(ObservableMsa *, msa);
    QFETCH(VectorHashCharInt, expectedCounts);

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

    QStringList subseqs;
    subseqs << "--AB--C-DEF"
            << "---B-XC-DE-";
    ObservableMsa *msa = createMsa(subseqs);
    LiveMsaCharCountDistribution x2(msa);
    QCOMPARE(x2.charCountDistribution().divisor(), 2);

    Seq seq("--AB--C-DEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("--AB--C-DEF"));
    QVERIFY(msa->append(subseq));

    QCOMPARE(x2.charCountDistribution().divisor(), 3);

    msa->removeAt(3);
    QCOMPARE(x2.charCountDistribution().divisor(), 2);

    msa->removeAt(2);
    QCOMPARE(x2.charCountDistribution().divisor(), 1);

    msa->removeAt(1);
    QCOMPARE(x2.charCountDistribution().divisor(), 0);

    delete msa;
    msa = 0;
}

void TestLiveMsaCharCountDistribution::msaGapsInserted_data()
{
    QTest::addColumn<QStringList>("sourceMsaSubseqs");
    QTest::addColumn<int>("insertGapPosition");
    QTest::addColumn<int>("nGaps");
    QTest::addColumn<ClosedIntRange>("signalColumns");
    QTest::addColumn<QStringList>("expectedMsaSubseqs");

    QStringList sourceMsaSubseqs;
    sourceMsaSubseqs << "ABC"
                     << "A-C"
                     << "-BC";

    QTest::newRow("insert single gap at end")
            << sourceMsaSubseqs
            << 4
            << 1
            << ClosedIntRange(4, 4)
            << (QStringList() << "ABC-" << "A-C-" << "-BC-");

    QTest::newRow("insert two gaps in middle")
            << sourceMsaSubseqs
            << 2
            << 2
            << ClosedIntRange(2, 3)
            << (QStringList() << "A--BC" << "A---C" << "---BC");

    QTest::newRow("insert three gaps at beginning")
            << sourceMsaSubseqs
            << 1
            << 3
            << ClosedIntRange(1, 3)
            << (QStringList() << "---ABC" << "---A-C" << "----BC");
}

void TestLiveMsaCharCountDistribution::msaGapsInserted()
{
    QFETCH(QStringList, sourceMsaSubseqs);
    QFETCH(int, insertGapPosition);
    QFETCH(int, nGaps);
    QFETCH(ClosedIntRange, signalColumns);
    QFETCH(QStringList, expectedMsaSubseqs);

    ObservableMsa *msa = createMsa(sourceMsaSubseqs);
    QVERIFY(msa);

    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyGapsInserted(&x, SIGNAL(columnsInserted(ClosedIntRange)));
    QVariantList spyArguments;

    msa->insertGapColumns(insertGapPosition, nGaps);
    QCOMPARE(spyGapsInserted.count(), 1);
    spyArguments = spyGapsInserted.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), signalColumns);

    ObservableMsa *expectedMsa = createMsa(expectedMsaSubseqs);
    QVERIFY(expectedMsa);
    /*
    VectorHashCharInt result = ::calculateMsaCharCountDistribution(*expectedMsa);
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

    QCOMPARE(x.charCountDistribution(), ::calculateMsaCharCountDistribution(*expectedMsa));
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

    ObservableMsa *expectedMsa = createMsa(sourceMsaSubseqs);
    CharCountDistribution expectedResult = ::calculateMsaCharCountDistribution(*expectedMsa);

    ObservableMsa *msa = createMsa(sourceMsaSubseqs);
    QVERIFY(msa);

    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(ClosedIntRange)));
    QVariantList spyArguments;

    msa->removeGapColumns();
    QVERIFY(spyColumnsRemoved.isEmpty());

    // ------------------------------------------------------------------------
    // Test: Removing 2 gap columns from beginning
    msa->insertGapColumns(1, 2);
    msa->removeGapColumns();
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 2));
    QCOMPARE(x.charCountDistribution().charCounts(), expectedResult.charCounts());

    // ------------------------------------------------------------------------
    // Test: Removing 1 gap columns from middle
    msa->insertGapColumns(2, 1);
    msa->removeGapColumns();
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(2, 2));
    QCOMPARE(x.charCountDistribution().charCounts(), expectedResult.charCounts());

    // ------------------------------------------------------------------------
    // Test: Removing 3 gaps columns from end
    msa->insertGapColumns(4, 3);
    msa->removeGapColumns();
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(4, 6));
    QCOMPARE(x.charCountDistribution().charCounts(), expectedResult.charCounts());

    // ------------------------------------------------------------------------
    // Test: Removing 1 gap columns from beginning, 2 from middle, 3 from end
    msa->insertGapColumns(4, 3);
    msa->insertGapColumns(2, 2);
    msa->insertGapColumns(1, 1);
    msa->removeGapColumns();
    QCOMPARE(spyColumnsRemoved.count(), 3);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(7, 9));
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(3, 4));
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 1));

    QCOMPARE(x.charCountDistribution().charCounts(), expectedResult.charCounts());
}

void TestLiveMsaCharCountDistribution::msaReset()
{
    QStringList sourceMsaSubseqs;
    sourceMsaSubseqs << "ABC"
                     << "A-B";

    ObservableMsa *msa = new ObservableMsa();
    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(ClosedIntRange)));

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
    QSignalSpy spyColumnsRemoved2(&x2, SIGNAL(columnsRemoved(ClosedIntRange)));
    QVariantList spyArguments;
    QVERIFY(msa);

    msa->clear();

    QCOMPARE(spyColumnsRemoved2.count(), 1);
    spyArguments = spyColumnsRemoved2.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 3));
    QCOMPARE(x2.charCountDistribution().charCounts(), VectorHashCharInt());
}

void TestLiveMsaCharCountDistribution::msaSubseqInsertedRemoved()
{
    Seq seq1("ABC");
    Subseq *subseq1 = new Subseq(seq1);

    Seq seq2("AC");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("A-C"));

    ObservableMsa *msa = new ObservableMsa();

    LiveMsaCharCountDistribution x(msa);

    QSignalSpy spyColumnsInserted(&x, SIGNAL(columnsInserted(ClosedIntRange)));
    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(ClosedIntRange)));
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: appending subseq to empty msa
    msa->append(subseq1);
    QVERIFY(spyDataChanged.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QCOMPARE(spyColumnsInserted.count(), 1);
    spyArguments = spyColumnsInserted.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 3));
    QCOMPARE(x.charCountDistribution(), ::calculateMsaCharCountDistribution(*msa));

    // ------------------------------------------------------------------------
    // Test: appending subseq to non-empty msa
    QVERIFY(msa->append(subseq2));
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 3));
    QCOMPARE(x.charCountDistribution(), ::calculateMsaCharCountDistribution(*msa));

    // ------------------------------------------------------------------------
    // Test: removing subseq from non-empty msa
    QVERIFY(spyDataChanged.isEmpty());
    msa->removeAt(2);
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 3));
    QCOMPARE(x.charCountDistribution().divisor(), 1);
    QCOMPARE(x.charCountDistribution(), ::calculateMsaCharCountDistribution(*msa));

    // ------------------------------------------------------------------------
    // Test: removing subseq from non-empty msa -> empty msa
    msa->removeAt(1);
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyDataChanged.isEmpty());
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 1));
    QCOMPARE(x.charCountDistribution().divisor(), 0);
    QCOMPARE(x.charCountDistribution().charCounts(), VectorHashCharInt());

    delete msa;
    msa = 0;
}

void TestLiveMsaCharCountDistribution::msaRectangleSlid_data()
{
    QTest::addColumn<QStringList>("subseqs");
    QTest::addColumn<PosiRect>("msaRect");
    QTest::addColumn<int>("delta");
    QTest::addColumn<ClosedIntRange>("dataChanged");
    QTest::addColumn<QStringList>("finalSubseqs");

    // ------------------------------------------------------------------------
    QStringList subseqs;
    subseqs << "--AB-C-D";
    QStringList finalSubseqs;
    finalSubseqs << "---ABC-D";
    QTest::newRow("single sequence slide - positive delta (+1)")
            << subseqs
            << PosiRect(QPoint(3, 1), QPoint(4, 1))    // AB
            << 1
            << ClosedIntRange(3, 5)
            << finalSubseqs;
    finalSubseqs.clear();

    finalSubseqs << "AB---C-D";
    QTest::newRow("single sequence slide - negative delta (-2)")
            << subseqs
            << PosiRect(QPoint(3, 1), QPoint(4, 1))    // AB
            << -2
            << ClosedIntRange(1, 4)
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
            << PosiRect(QPoint(4, 2), QPoint(6, 3))
            << 2
            << ClosedIntRange(4, 8)
            << finalSubseqs;
    finalSubseqs.clear();

    // ------------------------------------------------------------------------
    finalSubseqs << "-A-B---C"
                 << "-D-E---F"
                 << "-G-H---I";
    QTest::newRow("three sequence slide, single column - positive delta (+2)")
            << subseqs
            << PosiRect(QPoint(6, 1), QPoint(6, 3))
            << 2
            << ClosedIntRange(6, 8)
            << finalSubseqs;
    finalSubseqs.clear();

    // ------------------------------------------------------------------------
    finalSubseqs << "A-B--C--"
                 << "D-E--F--"
                 << "G-H--I--";
    QTest::newRow("three sequence slide, containing gaps - negative delta (-1)")
            << subseqs
            << PosiRect(QPoint(2, 1), QPoint(5, 3))
            << -1
            << ClosedIntRange(1, 5)
            << finalSubseqs;
    finalSubseqs.clear();

    // ------------------------------------------------------------------------
    finalSubseqs << "A-B--C--"
                 << "D-E--F--"
                 << "G-H--I--";
    QTest::newRow("three sequence slide, inverted msaRect, containing gaps - negative delta (-1)")
            << subseqs
            << PosiRect(QPoint(5, 3), QPoint(2, 1))
            << -1
            << ClosedIntRange(1, 5)
            << finalSubseqs;
    finalSubseqs.clear();

    // ------------------------------------------------------------------------
    // Moving only gaps
    finalSubseqs << "A--B-C--"
                 << "-D-E-F--"
                 << "-G-H-I--";
    QTest::newRow("moving single gap")
            << subseqs
            << PosiRect(QPoint(1, 1), QPoint(1, 1))
            << 1
            << ClosedIntRange(1, 2)
            << finalSubseqs;
    finalSubseqs.clear();


    finalSubseqs << "-A-B-C--"
                 << "-D-E---F"
                 << "-G-H---I";
    QTest::newRow("moving multi gap block")
            << subseqs
            << PosiRect(QPoint(7, 2), QPoint(8, 3))
            << -1
            << ClosedIntRange(6, 8)
            << finalSubseqs;
    finalSubseqs.clear();


    finalSubseqs << "-AB--C--"
                 << "-DE--F--"
                 << "-GH--I--";
    QTest::newRow("moving entire column of blocks")
            << subseqs
            << PosiRect(QPoint(3, 1), QPoint(3, 3))
            << 2
            << ClosedIntRange(3, 5)
            << finalSubseqs;
    finalSubseqs.clear();

    // ------------------------------------------------------------------------
    // Swapping a block of gaps with another block of gaps
    // DEFUNCT!!!
//    finalSubseqs << "-A-B-C--"
//                 << "-D-E-F--"
//                 << "-G-H-I--";
//    QTest::newRow("moving entire column of blocks")
//            << subseqs
//            << PosiRect(QPoint(7, 1), QPoint(7, 3))
//            << 1
//            << ClosedIntRange()
//            << finalSubseqs;
//    finalSubseqs.clear();


    // ------------------------------------------------------------------------
    // Moving non-normal PosiRect gap block
    finalSubseqs << "-A-B-C--"
                 << "-D-E---F"
                 << "-G-H---I";
    QTest::newRow("moving entire column of blocks - non normal PosiRect")
            << subseqs
            << PosiRect(QPoint(8, 3), QPoint(7, 2))
            << -1
            << ClosedIntRange(6, 8)
            << finalSubseqs;

    // Reverse the above operation
    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "-A-B-C--"
                 << "-D-E-F--"
                 << "-G-H-I--";
    QTest::newRow("moving entire column of blocks - non normal PosiRect")
            << subseqs
            << PosiRect(QPoint(6, 3), QPoint(7, 2))
            << 1
            << ClosedIntRange(6, 8)
            << finalSubseqs;

    // ------------------------------------------------------------------------
    // Moving normal PosiRect with some rows containing entirely gaps
    subseqs.clear();
    finalSubseqs.clear();
    //           12345678
    subseqs  << "-A-B-C--"
             << "-D-E---F"
             << "-G-H---I";
    finalSubseqs << "-A-B---C"
                 << "-D-E--F-"
                 << "-G-H---I";
    QTest::newRow("moving block with row containing pure gaps and others non-gaps - normal PosiRect")
            << subseqs
            << PosiRect(QPoint(7, 1), QPoint(8, 2))
            << -1
            << ClosedIntRange(6, 8)
            << finalSubseqs;
}

void TestLiveMsaCharCountDistribution::msaRectangleSlid()
{
    QFETCH(QStringList, subseqs);
    QFETCH(PosiRect, msaRect);
    QFETCH(int, delta);
    QFETCH(ClosedIntRange, dataChanged);
    QFETCH(QStringList, finalSubseqs);

    ObservableMsa *msa = createMsa(subseqs);
    QVERIFY(msa);
    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
    QVariantList spyArguments;

    ObservableMsa *finalMsa = createMsa(finalSubseqs);
    QVERIFY(finalMsa);

    QCOMPARE(msa->slideRect(msaRect, delta), delta);

    if (x.charCountDistribution() != ::calculateMsaCharCountDistribution(*finalMsa))
    {
        qDebug() << "\n" << finalSubseqs.join("\n");
        qDebug() << "Expected" << ::calculateMsaCharCountDistribution(*finalMsa).charCounts();
        qDebug() << "Actual" << x.charCountDistribution().charCounts();
    }

    QCOMPARE(x.charCountDistribution(), ::calculateMsaCharCountDistribution(*finalMsa));

    if (dataChanged.isEmpty() == false)
    {
        QCOMPARE(spyDataChanged.count(), 1);
        spyArguments = spyDataChanged.takeFirst();
        QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), dataChanged);
    }
    else
    {
        QVERIFY(spyDataChanged.isEmpty());
    }

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
    QTest::addColumn<ClosedIntRange>("dataChanged");
    QTest::addColumn<QStringList>("finalSubseqs");

    // NOTE!! It is vital that only the characters ABCD are used because of a hack used in the actual test
    // method below
    QStringList subseqs, finalSubseqs;
    subseqs << "--ABCD--"
            << "-A-B-C-D";
    finalSubseqs << "---BCD--"
                 << "-A-B-C-D";
    QTest::newRow("Subseq 1, increasing start") << subseqs << 1 << 2 << 0 << ClosedIntRange(3, 3) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "-A-B-C-D";
    QTest::newRow("Subseq 1, decreasing start") << subseqs << 1 << 1 << 0 << ClosedIntRange(3, 3) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABC---"
                 << "-A-B-C-D";
    QTest::newRow("Subseq 1, decreasing stop") << subseqs << 1 << 0 << 3 << ClosedIntRange(6, 6) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "-A-B-C-D";
    QTest::newRow("Subseq 1, increasing stop") << subseqs << 1 << 0 << 4 << ClosedIntRange(6, 6) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "-----C-D";
    QTest::newRow("Subseq 2, increasing start") << subseqs << 2 << 3 << 0 << ClosedIntRange(2, 4) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "---ABC-D";
    QTest::newRow("Subseq 2, decreasing start") << subseqs << 2 << 1 << 0 << ClosedIntRange(4, 5) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "---ABC--";
    QTest::newRow("Subseq 2, decreasing stop") << subseqs << 2 << 0 << 3 << ClosedIntRange(8, 8) << finalSubseqs;

    subseqs = finalSubseqs;
    finalSubseqs.clear();
    finalSubseqs << "--ABCD--"
                 << "---ABCD-";
    QTest::newRow("Subseq 2, increasing stop") << subseqs << 2 << 0 << 4 << ClosedIntRange(7, 7) << finalSubseqs;
}

void TestLiveMsaCharCountDistribution::msaExtendTrim()
{
    QFETCH(QStringList, subseqs);
    QFETCH(int, subseqIndex);
    QFETCH(int, start);
    QFETCH(int, stop);
    QFETCH(ClosedIntRange, dataChanged);
    QFETCH(QStringList, finalSubseqs);

    ObservableMsa *msa = new ObservableMsa;
    Seq seq("ABCD");
    foreach (QString sequence, subseqs)
    {
        Subseq *subseq = new Subseq(seq);
        QVERIFY(subseq->setBioString(sequence.toAscii()));

        QVERIFY(msa->append(subseq));
    }

    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
    QVariantList spyArguments;

    ObservableMsa *finalMsa = createMsa(finalSubseqs);
    QVERIFY(finalMsa);

    if (start != 0)
        msa->setSubseqStart(subseqIndex, start);
    else
        msa->setSubseqStop(subseqIndex, stop);

    if (x.charCountDistribution() != ::calculateMsaCharCountDistribution(*finalMsa))
    {
        qDebug() << "\n" << finalSubseqs.join("\n");
        qDebug() << "Expected" << ::calculateMsaCharCountDistribution(*finalMsa).charCounts();
        qDebug() << "Actual" << x.charCountDistribution().charCounts();
    }

    QCOMPARE(x.charCountDistribution(), ::calculateMsaCharCountDistribution(*finalMsa));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), dataChanged);

    delete msa;
    msa = 0;

    delete finalMsa;
    finalMsa = 0;
}

void TestLiveMsaCharCountDistribution::msaCollapse()
{
    Seq seq("ABCDEF");
    Subseq *subseq1 = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);
    Subseq *subseq3 = new Subseq(seq);

    ObservableMsa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QVector<QByteArray> seqs;
    seqs << "A-BC-D-E"
         << "-A-BCD--"
         << "--A---BC";

    PosiRect msaRect;
    int msaLength = seqs.at(0).length();
    int nSeqs = seqs.size();
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
                        QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
                        QSignalSpy spyCollapsedLeft(&msa, SIGNAL(collapsedLeft(PosiRect)));

                        // Assuming that the collapseLeft function works properly and has been tested
                        msa.collapseLeft(msaRect);

                        // Check that the char count distribution is as expected
                        if (x.charCountDistribution() != ::calculateMsaCharCountDistribution(msa))
                        {
                            qDebug() << "\n" << subseq1->asByteArray() << "\n"
                                     << subseq2->asByteArray() << "\n"
                                     << subseq3->asByteArray();
                            qDebug() << "Expected" << ::calculateMsaCharCountDistribution(msa).charCounts();
                            qDebug() << "Actual" << x.charCountDistribution().charCounts();
                        }
                        QCOMPARE(x.charCountDistribution(), ::calculateMsaCharCountDistribution(msa));

                        // Check the signals
                        if (spyCollapsedLeft.isEmpty())
                        {
                            QVERIFY(spyDataChanged.isEmpty());
                            continue;
                        }

                        // Else at least one sequence was modified, check that we get a change event for these columns
                        if (spyDataChanged.count() == 1)
                        {
                            ClosedIntRange range = qVariantValue<ClosedIntRange>(spyDataChanged.takeFirst().at(0));
                            PosiRect posiRect = qVariantValue<PosiRect>(spyCollapsedLeft.takeFirst().at(0));

                            // Depending on the implementation the range may or may not exactly parallel the posiRect
                            // horizontal range. For example, if the observable msa distribution simply updates relative
                            // to the aboutToCollapse, collapse signals, then the range should equal the posiRect;
                            // however, if it simply updates relative to the subseqsChanged signal, then it will be
                            // contained by posiRect.
                            QVERIFY(range.begin_ >= posiRect.left());
                            QVERIFY(range.end_ <= posiRect.right());
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
                        QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
                        QSignalSpy spyCollapsedRight(&msa, SIGNAL(collapsedRight(PosiRect)));

                        // Assuming that the collapseLeft function works properly and has been tested
                        msa.collapseRight(msaRect);

                        // Check that the char count distribution is as expected
                        if (x.charCountDistribution() != ::calculateMsaCharCountDistribution(msa))
                        {
                            qDebug() << "\n" << subseq1->asByteArray() << "\n"
                                     << subseq2->asByteArray() << "\n"
                                     << subseq3->asByteArray();
                            qDebug() << "Expected" << ::calculateMsaCharCountDistribution(msa).charCounts();
                            qDebug() << "Actual" << x.charCountDistribution().charCounts();
                        }
                        QCOMPARE(x.charCountDistribution(), ::calculateMsaCharCountDistribution(msa));

                        // Check the signals
                        if (spyCollapsedRight.isEmpty())
                        {
                            QVERIFY(spyDataChanged.isEmpty());
                            continue;
                        }

                        if (spyDataChanged.count() == 1)
                        {
                            ClosedIntRange range = qVariantValue<ClosedIntRange>(spyDataChanged.takeFirst().at(0));
                            PosiRect posiRect = qVariantValue<PosiRect>(spyCollapsedRight.takeFirst().at(0));

                            // See above notes for collapseLeft test
                            QVERIFY(range.begin_ >= posiRect.left());
                            QVERIFY(range.end_ <= posiRect.right());
                        }
                    }
                }
            }
        }
    }
}

/*
  !!! No longer relevant since the updated Msa contract that start positions cannot be moved beyond their stop positions
      and vice versa. More specifically, the extendLeftTrimRight and extendRightTrimLeft operations are no longer
      implemented, which this method was intended to test

void TestLiveMsaCharCountDistribution::msaExtendAndTrimOp()
{
    // Setup: create an ObservableMsa
    ObservableMsa *msa = new ObservableMsa;

    Seq seq("ABCDEF");
    Subseq *subseq1 = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);
    QVERIFY(subseq1->setBioString("---DEF"));

    QVERIFY(msa->append(subseq1));
    QVERIFY(msa->append(subseq2));

    LiveMsaCharCountDistribution x(msa);
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(ClosedIntRange)));
    QVariantList spyArguments;

    msa->setSubseqStop(1, 1);
    QCOMPARE(msa->at(1)->asByteArray(), QByteArray("A-----"));
    if (x.charCountDistribution().charCounts() != ::calculateMsaCharCountDistribution(*msa))
    {
        qDebug() << msa->at(1)->asByteArray();
        qDebug() << msa->at(2)->asByteArray();
        qDebug() << "Expected" << ::calculateMsaCharCountDistribution(*msa);
        qDebug() << "Actual" << x.charCountDistribution().charCounts();
    }
    QCOMPARE(x.charCountDistribution().charCounts(), ::calculateMsaCharCountDistribution(*msa));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments.at(0)), ClosedIntRange(1, 6));

    QVERIFY(subseq1->setBioString("A-B---"));

    // Now for extendRightTrimLeft
    LiveMsaCharCountDistribution x2(msa);
    QSignalSpy spyDataChanged2(&x2, SIGNAL(dataChanged(ClosedIntRange)));
    QVariantList spyArguments2;
    msa->setSubseqStart(1, 5);
    QCOMPARE(msa->at(1)->asByteArray(), QByteArray("-----E"));
    if (x2.charCountDistribution().charCounts() != ::calculateMsaCharCountDistribution(*msa))
    {
        qDebug() << msa->at(1)->asByteArray();
        qDebug() << msa->at(2)->asByteArray();
        qDebug() << "Expected" << ::calculateMsaCharCountDistribution(*msa);
        qDebug() << "Actual" << x.charCountDistribution().charCounts();
    }
    QCOMPARE(x2.charCountDistribution().charCounts(), ::calculateMsaCharCountDistribution(*msa));
    QCOMPARE(spyDataChanged2.count(), 1);
    spyArguments2 = spyDataChanged2.takeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyArguments2.at(0)), ClosedIntRange(1, 6));

    delete msa;
    msa = 0;
}
*/

QTEST_APPLESS_MAIN(TestLiveMsaCharCountDistribution)
#include "TestLiveMsaCharCountDistribution.moc"
