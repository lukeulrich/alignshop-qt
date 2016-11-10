/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include "../ObservableMsa.h"
#include "../Msa.h"
#include "../enums.h"
#include "../global.h"
#include "../metatypes.h"

class TestObservableMsa : public QObject
{
    Q_OBJECT

public:
    TestObservableMsa()
    {
        qRegisterMetaType<ClosedIntRange>("ClosedIntRange");
        qRegisterMetaType<PosiRect>("PosiRect");
        qRegisterMetaType<SubseqChangePodVector>("SubseqChangePodVector");
        qRegisterMetaType<QVector<ClosedIntRange> >("QVector<ClosedIntRange>");
    }

private slots:
    void append();
    void clear();
    void collapseLeft();
    void collapseRight();
    void extendLeft();
    void extendLeftRowNumChars();
    void extendLeftRowBioString();
    void extendRight();
    void extendRightRowNumChars();
    void extendRightRowBioString();
    void insert();
    void insertGapColumns();
    void insertRows();
    void levelLeft();
    void levelRight();
    void moveRow();
    void moveRowRange();
    void moveRowRangeRelative();
    void moveRowRelative();
    void prepend();
    void removeAt();            // Also tests removeFirst and removeLast
    void removeRows();
    void removeGapColumns();    // Also tests the parameterized version
    void setModified();
    void setSubseqStart();
    void setSubseqStop();
    void slideRect();
    void sort();
    void swap();
    void takeRows();
    void trimLeft();
    void trimRight();
    void undo();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestObservableMsa::append()
{
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Seq seq3("CDEF");
    Subseq *subseq3 = new Subseq(seq3);
    QVERIFY(subseq3->setBioString("--CD"));

    ObservableMsa msa;

    QSignalSpy spyRowsAboutToBeInserted(&msa, SIGNAL(rowsAboutToBeInserted(ClosedIntRange)));
    QSignalSpy spyRowsInserted(&msa, SIGNAL(rowsInserted(ClosedIntRange)));
    QVariantList spyArguments;

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QCOMPARE(msa.append(subseq3), false);

    QCOMPARE(spyRowsAboutToBeInserted.size(), 2);
    QCOMPARE(spyRowsInserted.size(), 2);

    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    ClosedIntRange insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(1, 1));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(0).at(0)));
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(2, 2));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(1).at(0)));
    spyRowsInserted.clear();
}

void TestObservableMsa::clear()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    subseq->setBioString("BC--DE");

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    subseq2->setBioString("GH--IJ");

    Seq seq3("CDEF");
    Subseq *subseq3 = new Subseq(seq3);
    subseq3->setBioString("--CDEF");

    ObservableMsa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spyMsaAboutToBeReset(&msa, SIGNAL(msaAboutToBeReset()));
    QSignalSpy spyMsaReset(&msa, SIGNAL(msaReset()));

    msa.clear();

    QCOMPARE(spyMsaAboutToBeReset.size(), 1);
    QCOMPARE(spyMsaReset.size(), 1);
}

void TestObservableMsa::collapseLeft()
{
    Seq seq("ABCDEF");
    Subseq *subseq1 = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);
    Subseq *subseq3 = new Subseq(seq);
    Subseq *subseq1a = new Subseq(seq);
    Subseq *subseq2a = new Subseq(seq);
    Subseq *subseq3a = new Subseq(seq);

    QVERIFY(subseq1->setBioString("A-BC-D-E"));
    QVERIFY(subseq2->setBioString("-A-BCD--"));
    QVERIFY(subseq3->setBioString("--A---BC"));
    QVERIFY(subseq1a->setBioString("A-BC-D-E"));
    QVERIFY(subseq2a->setBioString("-A-BCD--"));
    QVERIFY(subseq3a->setBioString("--A---BC"));

    ObservableMsa oMsa;
    QVERIFY(oMsa.append(subseq1));
    QVERIFY(oMsa.append(subseq2));
    QVERIFY(oMsa.append(subseq3));

    Msa msa;
    QVERIFY(msa.append(subseq1a));
    QVERIFY(msa.append(subseq2a));
    QVERIFY(msa.append(subseq3a));

    QSignalSpy spyAboutToBeCollapsedLeft(&oMsa, SIGNAL(aboutToBeCollapsedLeft(PosiRect)));
    QSignalSpy spyCollapsedLeft(&oMsa, SIGNAL(collapsedLeft(PosiRect)));
    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    QVariantList spyArguments;

    PosiRect collapseRect(QPoint(4, 3), QPoint(1, 1));
    SubseqChangePodVector actualPods = oMsa.collapseLeft(collapseRect);
    SubseqChangePodVector expectedPods = msa.collapseLeft(collapseRect);
    QCOMPARE(actualPods, expectedPods);

    QCOMPARE(spyAboutToBeCollapsedLeft.size(), 1);
    QCOMPARE(spyCollapsedLeft.size(), 1);
    spyArguments = spyAboutToBeCollapsedLeft.takeFirst();
    QCOMPARE(collapseRect.normalized(), qVariantValue<PosiRect>(spyArguments.at(0)));
    spyArguments = spyCollapsedLeft.takeFirst();
    QCOMPARE(collapseRect.normalized(), qVariantValue<PosiRect>(spyArguments.at(0)));

    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::collapseRight()
{
    Seq seq("ABCDEF");
    Subseq *subseq1 = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);
    Subseq *subseq3 = new Subseq(seq);
    Subseq *subseq1a = new Subseq(seq);
    Subseq *subseq2a = new Subseq(seq);
    Subseq *subseq3a = new Subseq(seq);

    QVERIFY(subseq1->setBioString("A-BC-D-E"));
    QVERIFY(subseq2->setBioString("-A-BCD--"));
    QVERIFY(subseq3->setBioString("--A---BC"));

    QVERIFY(subseq1a->setBioString("A-BC-D-E"));
    QVERIFY(subseq2a->setBioString("-A-BCD--"));
    QVERIFY(subseq3a->setBioString("--A---BC"));

    ObservableMsa oMsa;
    QVERIFY(oMsa.append(subseq1));
    QVERIFY(oMsa.append(subseq2));
    QVERIFY(oMsa.append(subseq3));

    Msa msa;
    QVERIFY(msa.append(subseq1a));
    QVERIFY(msa.append(subseq2a));
    QVERIFY(msa.append(subseq3a));

    QSignalSpy spyAboutToBeCollapsedRight(&oMsa, SIGNAL(aboutToBeCollapsedRight(PosiRect)));
    QSignalSpy spyCollapsedRight(&oMsa, SIGNAL(collapsedRight(PosiRect)));
    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    QVariantList spyArguments;

    PosiRect collapseRect(QPoint(4, 3), QPoint(1, 1));
    SubseqChangePodVector actualPods = oMsa.collapseRight(collapseRect);
    SubseqChangePodVector expectedPods = msa.collapseRight(collapseRect);
    QCOMPARE(actualPods, expectedPods);

    QCOMPARE(spyAboutToBeCollapsedRight.size(), 1);
    QCOMPARE(spyCollapsedRight.size(), 1);
    spyArguments = spyAboutToBeCollapsedRight.takeFirst();
    QCOMPARE(collapseRect.normalized(), qVariantValue<PosiRect>(spyArguments.at(0)));
    spyArguments = spyCollapsedRight.takeFirst();
    QCOMPARE(collapseRect.normalized(), qVariantValue<PosiRect>(spyArguments.at(0)));

    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::extendLeft()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString( "---C-DE--"));
    QVERIFY(subseq2->setBioString("---C-DE--"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePodVector actualPods = oMsa.extendLeft(4, ClosedIntRange(1, 1));
    QVERIFY(actualPods.isEmpty());
    QVERIFY(spySubseqsBorderChanged.isEmpty());

    SubseqChangePodVector expectedPods = msa.extendLeft(1, ClosedIntRange(1, 1));
    actualPods = oMsa.extendLeft(1, ClosedIntRange(1, 1));
    QCOMPARE(actualPods, expectedPods);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::extendLeftRowNumChars()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString( "---C-DE--"));
    QVERIFY(subseq2->setBioString("---C-DE--"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePod actualPod = oMsa.extendLeft(1, 2);
    SubseqChangePod expectedPod = msa.extendLeft(1, 2);
    QCOMPARE(actualPod, expectedPod);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPod, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)).first());
}

void TestObservableMsa::extendLeftRowBioString()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString( "---C-DE--"));
    QVERIFY(subseq2->setBioString("---C-DE--"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePod actualPod = oMsa.extendLeft(1, 1, "A-B");
    SubseqChangePod expectedPod = msa.extendLeft(1, 1, "A-B");
    QCOMPARE(actualPod, expectedPod);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPod, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)).first());
}

void TestObservableMsa::extendRight()
{
    Seq seq("ABCDEFGH");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString( "---C-DE--"));
    QVERIFY(subseq2->setBioString("---C-DE--"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePodVector actualPods = oMsa.extendRight(1, ClosedIntRange(1, 1));
    QVERIFY(actualPods.isEmpty());
    QVERIFY(spySubseqsBorderChanged.isEmpty());

    SubseqChangePodVector expectedPods = msa.extendRight(9, ClosedIntRange(1, 1));
    actualPods = oMsa.extendRight(9, ClosedIntRange(1, 1));
    QCOMPARE(actualPods, expectedPods);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::extendRightRowNumChars()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString( "---C-D---"));
    QVERIFY(subseq2->setBioString("---C-D---"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePod actualPod = oMsa.extendRight(1, 2);
    SubseqChangePod expectedPod = msa.extendRight(1, 2);
    QCOMPARE(actualPod, expectedPod);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPod, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)).first());
}

void TestObservableMsa::extendRightRowBioString()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString( "---C-D----"));
    QVERIFY(subseq2->setBioString("---C-D----"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePod actualPod = oMsa.extendRight(8, 1, "-EF");
    SubseqChangePod expectedPod = msa.extendRight(8, 1, "-EF");
    QCOMPARE(actualPod, expectedPod);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPod, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)).first());
}

void TestObservableMsa::insert()
{
    Seq seq("ABCDEF");
    Seq seq2("GHIJKL");
    Seq seq3("CDEF");

    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Subseq *subseq3 = new Subseq(seq3);
    QVERIFY(subseq3->setBioString("--CDEF"));

    ObservableMsa msa;

    QSignalSpy spyRowsAboutToBeInserted(&msa, SIGNAL(rowsAboutToBeInserted(ClosedIntRange)));
    QSignalSpy spyRowsInserted(&msa, SIGNAL(rowsInserted(ClosedIntRange)));
    QVariantList spyArguments;

    // Test: insert at beginning
    QVERIFY(msa.insert(1, subseq));
    QVERIFY(msa.insert(1, subseq2));
    QVERIFY(msa.insert(1, subseq3));

    QCOMPARE(spyRowsAboutToBeInserted.size(), 3);
    QCOMPARE(spyRowsInserted.size(), 3);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    ClosedIntRange insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(1, 1));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(0).at(0)));
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(1, 1));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(1).at(0)));
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(1, 1));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(2).at(0)));
    spyRowsInserted.clear();

    msa.clear();

    // subseq, subseq2, subseq3 pointers are no longer valid because the msa was cleared
    subseq = 0;
    subseq2 = 0;
    subseq3 = 0;

    subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));
    subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));
    subseq3 = new Subseq(seq3);
    QVERIFY(subseq3->setBioString("--CDEF"));

    // Test: insert in the middle
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq3));

    spyRowsAboutToBeInserted.clear();
    spyRowsInserted.clear();

    QVERIFY(msa.insert(2, subseq2));
    QCOMPARE(spyRowsAboutToBeInserted.size(), 1);
    QCOMPARE(spyRowsInserted.size(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(2, 2));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(0).at(0)));
    spyRowsInserted.clear();

    // Test: insert at the end
    msa.clear();

    // subseq, subseq2, subseq3 pointers are no longer valid because the msa was cleared
    subseq = 0;
    subseq2 = 0;
    subseq3 = 0;

    subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));
    subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));
    subseq3 = new Subseq(seq3);
    QVERIFY(subseq3->setBioString("--CDEF"));

    QVERIFY(msa.insert(msa.subseqCount()+1, subseq));
    QVERIFY(msa.insert(msa.subseqCount()+1, subseq2));
    QVERIFY(msa.insert(msa.subseqCount()+1, subseq3));

    QCOMPARE(spyRowsAboutToBeInserted.size(), 3);
    QCOMPARE(spyRowsInserted.size(), 3);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(1, 1));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(0).at(0)));
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(2, 2));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(1).at(0)));
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(3, 3));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(2).at(0)));
    spyRowsInserted.clear();
}

void TestObservableMsa::insertGapColumns()
{
    Seq seq1("ABCDEF");
    Subseq *subseq = new Subseq(seq1);

    ObservableMsa msa;
    QVERIFY(msa.append(subseq));

    QSignalSpy spyGapColumnsAboutToBeInserted(&msa, SIGNAL(gapColumnsAboutToBeInserted(ClosedIntRange)));
    QSignalSpy spyGapColumnsInserted(&msa, SIGNAL(gapColumnsInserted(ClosedIntRange)));

    // Test: inserting gap columns at any position with n = 0, does nothing
    for (int i=1; i<= msa.length()+1; ++i)
    {
        msa.insertGapColumns(i, 0);
        QVERIFY(spyGapColumnsAboutToBeInserted.isEmpty());
        QVERIFY(spyGapColumnsInserted.isEmpty());
    }

    msa.insertGapColumns(1, 1);
    QCOMPARE(spyGapColumnsAboutToBeInserted.size(), 1);
    QCOMPARE(spyGapColumnsInserted.size(), 1);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyGapColumnsAboutToBeInserted.at(0).at(0)), ClosedIntRange(1, 1));
    QCOMPARE(qVariantValue<ClosedIntRange>(spyGapColumnsInserted.at(0).at(0)), ClosedIntRange(1, 1));
    spyGapColumnsAboutToBeInserted.clear();
    spyGapColumnsInserted.clear();

    msa.insertGapColumns(4, 3);
    QCOMPARE(spyGapColumnsAboutToBeInserted.size(), 1);
    QCOMPARE(spyGapColumnsInserted.size(), 1);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyGapColumnsAboutToBeInserted.at(0).at(0)), ClosedIntRange(4, 6));
    QCOMPARE(qVariantValue<ClosedIntRange>(spyGapColumnsInserted.at(0).at(0)), ClosedIntRange(4, 6));
}

void TestObservableMsa::insertRows()
{
    Seq seq1("ABCDEF");
    QVector<Subseq *> subseqs;
    for (int i=0; i<6; ++i)
        subseqs << new Subseq(seq1);

    ObservableMsa msa;

    QSignalSpy spyRowsAboutToBeInserted(&msa, SIGNAL(rowsAboutToBeInserted(ClosedIntRange)));
    QSignalSpy spyRowsInserted(&msa, SIGNAL(rowsInserted(ClosedIntRange)));
    QVariantList spyArguments;

    QVERIFY(msa.insertRows(1, subseqs));
    for (int i=0; i< 6; ++i)
        QVERIFY(msa.at(i+1) == subseqs.at(i));

    QCOMPARE(spyRowsAboutToBeInserted.size(), 1);
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    ClosedIntRange insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(1, 6));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(0).at(0)));
}

void TestObservableMsa::levelLeft()
{
    Seq seq("ABCDEFGH");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);
    QVERIFY(subseq->setBioString("--CD-EF--"));
    QVERIFY(subseq2->setBioString("--CD-EF--"));

    Msa msa;
    ObservableMsa oMsa;
    QVERIFY(oMsa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePodVector actualPods = oMsa.levelLeft(3, ClosedIntRange(1, 1));
    QVERIFY(actualPods.isEmpty());
    QVERIFY(spySubseqsBorderChanged.isEmpty());

    SubseqChangePodVector expectedPods = msa.levelLeft(1, ClosedIntRange(1, 1));
    actualPods = oMsa.levelLeft(1, ClosedIntRange(1, 1));
    QCOMPARE(actualPods, expectedPods);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::levelRight()
{
    Seq seq("ABCDEFGH");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);
    QVERIFY(subseq->setBioString("--CD-EF--"));
    QVERIFY(subseq2->setBioString("--CD-EF--"));

    Msa msa;
    ObservableMsa oMsa;
    QVERIFY(oMsa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePodVector actualPods = oMsa.levelRight(7, ClosedIntRange(1, 1));
    QVERIFY(actualPods.isEmpty());
    QVERIFY(spySubseqsBorderChanged.isEmpty());

    SubseqChangePodVector expectedPods = msa.levelRight(5, ClosedIntRange(1, 1));
    actualPods = oMsa.levelRight(5, ClosedIntRange(1, 1));
    QCOMPARE(actualPods, expectedPods);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::moveRow()
{
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));
    QVERIFY(subseq2->setBioString("AB--CD"));

    ObservableMsa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spyRowsAboutToBeMoved(&msa, SIGNAL(rowsAboutToBeMoved(ClosedIntRange,int)));
    QSignalSpy spyRowsMoved(&msa, SIGNAL(rowsMoved(ClosedIntRange,int)));

    msa.moveRow(1, 1);
    msa.moveRow(2, 2);
    QVERIFY(spyRowsAboutToBeMoved.isEmpty());
    QVERIFY(spyRowsMoved.isEmpty());

    msa.moveRow(1, 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeMoved.at(0).at(0)), ClosedIntRange(1, 1));
    QCOMPARE(spyRowsAboutToBeMoved.at(0).at(1).toInt(), 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsMoved.at(0).at(0)), ClosedIntRange(1, 1));
    QCOMPARE(spyRowsMoved.at(0).at(1).toInt(), 2);
    spyRowsAboutToBeMoved.clear();
    spyRowsMoved.clear();

    msa.moveRow(2, 1);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeMoved.at(0).at(0)), ClosedIntRange(2, 2));
    QCOMPARE(spyRowsAboutToBeMoved.at(0).at(1).toInt(), 1);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsMoved.at(0).at(0)), ClosedIntRange(2, 2));
    QCOMPARE(spyRowsMoved.at(0).at(1).toInt(), 1);
    spyRowsAboutToBeMoved.clear();
    spyRowsMoved.clear();
}

void TestObservableMsa::moveRowRange()
{
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);
    Subseq *subseq3 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));
    QVERIFY(subseq2->setBioString("AB--CD"));
    QVERIFY(subseq3->setBioString("AB--CD"));

    ObservableMsa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QSignalSpy spyRowsAboutToBeMoved(&msa, SIGNAL(rowsAboutToBeMoved(ClosedIntRange,int)));
    QSignalSpy spyRowsMoved(&msa, SIGNAL(rowsMoved(ClosedIntRange,int)));

    msa.moveRowRange(ClosedIntRange(1, 2), 1);
    msa.moveRowRange(ClosedIntRange(2, 3), 2);
    msa.moveRowRange(ClosedIntRange(1, 1), 1);
    QVERIFY(spyRowsAboutToBeMoved.isEmpty());
    QVERIFY(spyRowsMoved.isEmpty());

    msa.moveRowRange(ClosedIntRange(1, 2), 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeMoved.at(0).at(0)), ClosedIntRange(1, 2));
    QCOMPARE(spyRowsAboutToBeMoved.at(0).at(1).toInt(), 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsMoved.at(0).at(0)), ClosedIntRange(1, 2));
    QCOMPARE(spyRowsMoved.at(0).at(1).toInt(), 2);
    spyRowsAboutToBeMoved.clear();
    spyRowsMoved.clear();

    msa.moveRowRange(ClosedIntRange(3, 3), 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeMoved.at(0).at(0)), ClosedIntRange(3, 3));
    QCOMPARE(spyRowsAboutToBeMoved.at(0).at(1).toInt(), 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsMoved.at(0).at(0)), ClosedIntRange(3, 3));
    QCOMPARE(spyRowsMoved.at(0).at(1).toInt(), 2);
    spyRowsAboutToBeMoved.clear();
    spyRowsMoved.clear();
}

void TestObservableMsa::moveRowRangeRelative()
{
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);
    Subseq *subseq3 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));
    QVERIFY(subseq2->setBioString("AB--CD"));
    QVERIFY(subseq3->setBioString("AB--CD"));

    ObservableMsa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QSignalSpy spyRowsAboutToBeMoved(&msa, SIGNAL(rowsAboutToBeMoved(ClosedIntRange,int)));
    QSignalSpy spyRowsMoved(&msa, SIGNAL(rowsMoved(ClosedIntRange,int)));

    msa.moveRowRangeRelative(ClosedIntRange(1, 2), 0);
    msa.moveRowRangeRelative(ClosedIntRange(2, 3), 0);
    msa.moveRowRangeRelative(ClosedIntRange(1, 1), 0);
    QVERIFY(spyRowsAboutToBeMoved.isEmpty());
    QVERIFY(spyRowsMoved.isEmpty());

    msa.moveRowRangeRelative(ClosedIntRange(1, 2), 1);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeMoved.at(0).at(0)), ClosedIntRange(1, 2));
    QCOMPARE(spyRowsAboutToBeMoved.at(0).at(1).toInt(), 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsMoved.at(0).at(0)), ClosedIntRange(1, 2));
    QCOMPARE(spyRowsMoved.at(0).at(1).toInt(), 2);
    spyRowsAboutToBeMoved.clear();
    spyRowsMoved.clear();

    msa.moveRowRangeRelative(ClosedIntRange(3, 3), -2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeMoved.at(0).at(0)), ClosedIntRange(3, 3));
    QCOMPARE(spyRowsAboutToBeMoved.at(0).at(1).toInt(), 1);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsMoved.at(0).at(0)), ClosedIntRange(3, 3));
    QCOMPARE(spyRowsMoved.at(0).at(1).toInt(), 1);
    spyRowsAboutToBeMoved.clear();
    spyRowsMoved.clear();
}

void TestObservableMsa::moveRowRelative()
{
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);
    Subseq *subseq3 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));
    QVERIFY(subseq2->setBioString("AB--CD"));
    QVERIFY(subseq3->setBioString("AB--CD"));

    ObservableMsa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QSignalSpy spyRowsAboutToBeMoved(&msa, SIGNAL(rowsAboutToBeMoved(ClosedIntRange,int)));
    QSignalSpy spyRowsMoved(&msa, SIGNAL(rowsMoved(ClosedIntRange,int)));

    msa.moveRowRelative(1, 0);
    msa.moveRowRelative(2, 0);
    msa.moveRowRelative(3, 0);
    QVERIFY(spyRowsAboutToBeMoved.isEmpty());
    QVERIFY(spyRowsMoved.isEmpty());

    msa.moveRowRelative(1, 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeMoved.at(0).at(0)), ClosedIntRange(1, 1));
    QCOMPARE(spyRowsAboutToBeMoved.at(0).at(1).toInt(), 3);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsMoved.at(0).at(0)), ClosedIntRange(1, 1));
    QCOMPARE(spyRowsMoved.at(0).at(1).toInt(), 3);
    spyRowsAboutToBeMoved.clear();
    spyRowsMoved.clear();

    msa.moveRowRelative(3, -1);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeMoved.at(0).at(0)), ClosedIntRange(3, 3));
    QCOMPARE(spyRowsAboutToBeMoved.at(0).at(1).toInt(), 2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsMoved.at(0).at(0)), ClosedIntRange(3, 3));
    QCOMPARE(spyRowsMoved.at(0).at(1).toInt(), 2);
    spyRowsAboutToBeMoved.clear();
    spyRowsMoved.clear();
}

void TestObservableMsa::prepend()
{
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Seq seq3("CDEF");
    Subseq *subseq3 = new Subseq(seq3);
    QVERIFY(subseq3->setBioString("--CD"));

    ObservableMsa msa;

    QSignalSpy spyRowsAboutToBeInserted(&msa, SIGNAL(rowsAboutToBeInserted(ClosedIntRange)));
    QSignalSpy spyRowsInserted(&msa, SIGNAL(rowsInserted(ClosedIntRange)));
    QVariantList spyArguments;

    QVERIFY(msa.prepend(subseq1));
    QVERIFY(msa.prepend(subseq2));
    QCOMPARE(msa.prepend(subseq3), false);

    QCOMPARE(spyRowsAboutToBeInserted.size(), 2);
    QCOMPARE(spyRowsInserted.size(), 2);

    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    ClosedIntRange insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(1, 1));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(0).at(0)));
    spyArguments = spyRowsAboutToBeInserted.takeFirst();
    insertRange = qVariantValue<ClosedIntRange>(spyArguments.at(0));
    QCOMPARE(insertRange, ClosedIntRange(1, 1));
    QCOMPARE(insertRange, qVariantValue<ClosedIntRange>(spyRowsInserted.at(1).at(0)));
    spyRowsInserted.clear();
}

void TestObservableMsa::removeAt()
{
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);
    Subseq *subseq3 = new Subseq(seq1);
    Subseq *subseq4 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));
    QVERIFY(subseq2->setBioString("AB--CD"));
    QVERIFY(subseq3->setBioString("AB--CD"));
    QVERIFY(subseq4->setBioString("AB--CD"));

    ObservableMsa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));
    QVERIFY(msa.append(subseq4));

    QSignalSpy spyRowsAboutToBeRemoved(&msa, SIGNAL(rowsAboutToBeRemoved(ClosedIntRange)));
    QSignalSpy spyRowsRemoved(&msa, SIGNAL(rowsRemoved(ClosedIntRange)));

    msa.removeAt(2);
    QCOMPARE(spyRowsAboutToBeRemoved.size(), 1);
    QCOMPARE(spyRowsRemoved.size(), 1);

    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeRemoved.at(0).at(0)), ClosedIntRange(2, 2));
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsRemoved.at(0).at(0)), ClosedIntRange(2, 2));

    spyRowsAboutToBeRemoved.clear();;
    spyRowsRemoved.clear();

    msa.removeFirst();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeRemoved.at(0).at(0)), ClosedIntRange(1, 1));
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsRemoved.at(0).at(0)), ClosedIntRange(1, 1));

    spyRowsAboutToBeRemoved.clear();;
    spyRowsRemoved.clear();
    msa.removeLast();
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeRemoved.at(0).at(0)), ClosedIntRange(2, 2));
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsRemoved.at(0).at(0)), ClosedIntRange(2, 2));

}

void TestObservableMsa::removeRows()
{
    Seq seq("ABCDEF");

    ObservableMsa msa;
    Subseq *subseqs[6];
    for (int i=0; i<6; ++i)
    {
        subseqs[i] = new Subseq(seq);
        QVERIFY(msa.append(subseqs[i]));
    }

    QSignalSpy spyRowsAboutToBeRemoved(&msa, SIGNAL(rowsAboutToBeRemoved(ClosedIntRange)));
    QSignalSpy spyRowsRemoved(&msa, SIGNAL(rowsRemoved(ClosedIntRange)));

    msa.removeRows(ClosedIntRange(2, 4));
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeRemoved.at(0).at(0)), ClosedIntRange(2, 4));
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsRemoved.at(0).at(0)), ClosedIntRange(2, 4));
}

void TestObservableMsa::removeGapColumns()
{
    Seq seq("ABCD");
    Subseq *subseq = new Subseq(seq);

    ObservableMsa msa;
    QVERIFY(msa.append(subseq));

    QSignalSpy spyGapColumnsRemoved(&msa, SIGNAL(gapColumnsRemoved(QVector<ClosedIntRange>)));
    msa.removeGapColumns();
    QVERIFY(spyGapColumnsRemoved.isEmpty());

    QVERIFY(subseq->setBioString("--A--B--"));

    msa.removeGapColumns();
    QCOMPARE(spyGapColumnsRemoved.size(), 1);
    QVector<ClosedIntRange> removedRanges = qVariantValue<QVector<ClosedIntRange> >(spyGapColumnsRemoved.at(0).at(0));
    QCOMPARE(removedRanges.at(0), ClosedIntRange(1, 2));
    QCOMPARE(removedRanges.at(1), ClosedIntRange(4, 5));
    QCOMPARE(removedRanges.at(2), ClosedIntRange(7, 8));
    spyGapColumnsRemoved.clear();

    QVERIFY(subseq->setBioString("--A--B--"));
    msa.removeGapColumns(ClosedIntRange(2, 4));
    QCOMPARE(spyGapColumnsRemoved.size(), 1);
    removedRanges = qVariantValue<QVector<ClosedIntRange> >(spyGapColumnsRemoved.at(0).at(0));
    QCOMPARE(removedRanges.at(0), ClosedIntRange(2, 2));
    QCOMPARE(removedRanges.at(1), ClosedIntRange(4, 4));
}


void TestObservableMsa::setModified()
{
    ObservableMsa msa;

    QVERIFY(msa.isModified() == false);

    QSignalSpy spyModifiedChanged(&msa, SIGNAL(modifiedChanged(bool)));

    msa.setModified(true);
    QVERIFY(msa.isModified());
    QCOMPARE(spyModifiedChanged.size(), 1);
    QCOMPARE(spyModifiedChanged.at(0).at(0).toBool(), true);

    spyModifiedChanged.clear();
    msa.setModified(false);
    QCOMPARE(spyModifiedChanged.size(), 1);
    QCOMPARE(spyModifiedChanged.at(0).at(0).toBool(), false);
    QVERIFY(msa.isModified() == false);
}

void TestObservableMsa::setSubseqStart()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString("--CD--E--"));
    QVERIFY(subseq2->setBioString("--CD--E--"));

    Msa msa;
    ObservableMsa oMsa;

    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePodVector actualPods;
    QVERIFY(oMsa.setSubseqStart(1, 3).isNull());
    QVERIFY(spySubseqsBorderChanged.isEmpty());

    SubseqChangePodVector expectedPods;
    expectedPods << msa.setSubseqStart(1, 5);
    actualPods << oMsa.setSubseqStart(1, 5);
    QCOMPARE(actualPods, expectedPods);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(actualPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::setSubseqStop()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString("--CD--E--"));
    QVERIFY(subseq2->setBioString("--CD--E--"));

    Msa msa;
    ObservableMsa oMsa;

    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePodVector actualPods;
    QVERIFY(oMsa.setSubseqStop(1, 5).isNull());
    QVERIFY(spySubseqsBorderChanged.isEmpty());

    SubseqChangePodVector expectedPods;
    expectedPods << msa.setSubseqStart(1, 1);
    actualPods << oMsa.setSubseqStart(1, 1);
    QCOMPARE(actualPods, expectedPods);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(actualPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::slideRect()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("--CD--E--"));

    ObservableMsa oMsa;
    QVERIFY(oMsa.append(subseq));

    QSignalSpy spyRectangleSlid(&oMsa, SIGNAL(rectangleSlid(PosiRect,int,ClosedIntRange)));
    oMsa.slideRect(PosiRect(3, 1, 2, 1), 0);
    QVERIFY(spyRectangleSlid.isEmpty());

    oMsa.slideRect(PosiRect(3, 1, 2, 1), -4);
    QCOMPARE(spyRectangleSlid.size(), 1);
    QCOMPARE(qVariantValue<PosiRect>(spyRectangleSlid.at(0).at(0)), PosiRect(3, 1, 2, 1));
    QCOMPARE(spyRectangleSlid.at(0).at(1).toInt(), -2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRectangleSlid.at(0).at(2)), ClosedIntRange(1, 2));
    QVERIFY(*subseq == "CD----E--");
    spyRectangleSlid.clear();

    // Test: finalRange is not inverted even if msaRect is inverted
    Subseq *subseq2 = new Subseq(seq);
    QVERIFY(subseq->setBioString( "--BCD--"));
    QVERIFY(subseq2->setBioString("--BCD--"));
    QVERIFY(oMsa.append(subseq2));

    QCOMPARE(oMsa.slideRect(PosiRect(QPoint(5, 2), QPoint(3, 1)), -2), -2);
    QVERIFY(*subseq == "BCD----");
    QVERIFY(*subseq2 == "BCD----");
    QCOMPARE(spyRectangleSlid.size(), 1);
    QCOMPARE(qVariantValue<PosiRect>(spyRectangleSlid.at(0).at(0)), PosiRect(QPoint(5, 2), QPoint(3, 1)));
    QCOMPARE(spyRectangleSlid.at(0).at(1).toInt(), -2);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRectangleSlid.at(0).at(2)), ClosedIntRange(1, 3));
    spyRectangleSlid.clear();
}

bool dummyLessThan(const Subseq */* a */, const Subseq * /* b */)
{
    return false;
}

void TestObservableMsa::sort()
{
    ObservableMsa msa;

    QSignalSpy spyRowsAboutToBeSorted(&msa, SIGNAL(rowsAboutToBeSorted()));
    QSignalSpy spyRowsSorted(&msa, SIGNAL(rowsSorted()));

    msa.sort(dummyLessThan);

    QCOMPARE(spyRowsAboutToBeSorted.size(), 1);
    QCOMPARE(spyRowsSorted.size(), 1);
}

void TestObservableMsa::swap()
{
    Seq seq1("ABCDEF");
    Subseq *subseq = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(subseq->setBioString( "-ABC-DE--"));
    QVERIFY(subseq2->setBioString("-ABC-DE--"));

    ObservableMsa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spyRowsAboutToBeSwapped(&msa, SIGNAL(rowsAboutToBeSwapped(int,int)));
    QSignalSpy spyRowsSwapped(&msa, SIGNAL(rowsSwapped(int,int)));

    msa.swap(1, 1);
    msa.swap(2, 2);
    QVERIFY(spyRowsAboutToBeSwapped.isEmpty());
    QVERIFY(spyRowsSwapped.isEmpty());

    msa.swap(2, 1);

    QCOMPARE(spyRowsAboutToBeSwapped, spyRowsSwapped);
    QCOMPARE(spyRowsSwapped.size(), 1);
    QCOMPARE(spyRowsSwapped.at(0).at(0).toInt(), 2);
    QCOMPARE(spyRowsSwapped.at(0).at(1).toInt(), 1);
}

void TestObservableMsa::takeRows()
{
    Seq seq("ABCDEF");

    ObservableMsa msa;
    Subseq *subseqs[6];
    for (int i=0; i<6; ++i)
    {
        subseqs[i] = new Subseq(seq);
        QVERIFY(msa.append(subseqs[i]));
    }

    QSignalSpy spyRowsAboutToBeRemoved(&msa, SIGNAL(rowsAboutToBeRemoved(ClosedIntRange)));
    QSignalSpy spyRowsRemoved(&msa, SIGNAL(rowsRemoved(ClosedIntRange)));

    QVector<Subseq *> extraction = msa.takeRows(ClosedIntRange(2, 4));
    QCOMPARE(spyRowsAboutToBeRemoved.size(), 1);
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsAboutToBeRemoved.at(0).at(0)), ClosedIntRange(2, 4));
    QCOMPARE(qVariantValue<ClosedIntRange>(spyRowsRemoved.at(0).at(0)), ClosedIntRange(2, 4));

    qDeleteAll(extraction);
}

void TestObservableMsa::trimLeft()
{
    Seq seq1("ABCDEF");
    Seq seq2 = seq1;

    Subseq *subseq = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq2);

    QVERIFY(subseq->setBioString( "-ABC-DE--"));
    QVERIFY(subseq2->setBioString("-ABC-DE--"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePodVector actualPods = oMsa.trimLeft(1, ClosedIntRange(1, 1));
    QVERIFY(actualPods.isEmpty());
    QVERIFY(spySubseqsBorderChanged.isEmpty());

    SubseqChangePodVector expectedPods = msa.trimLeft(5, ClosedIntRange(1, 1));
    actualPods = oMsa.trimLeft(5, ClosedIntRange(1, 1));
    QCOMPARE(actualPods, expectedPods);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::trimRight()
{
    Seq seq1("ABCDEF");
    Seq seq2 = seq1;

    Subseq *subseq = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq2);

    QVERIFY(subseq->setBioString( "-ABC-DE--"));
    QVERIFY(subseq2->setBioString("-ABC-DE--"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));
    SubseqChangePodVector actualPods = oMsa.trimRight(8, ClosedIntRange(1, 1));
    QVERIFY(actualPods.isEmpty());
    QVERIFY(spySubseqsBorderChanged.isEmpty());

    SubseqChangePodVector expectedPods = msa.trimRight(4, ClosedIntRange(1, 1));
    actualPods = oMsa.trimRight(4, ClosedIntRange(1, 1));
    QCOMPARE(actualPods, expectedPods);
    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(expectedPods, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)));
}

void TestObservableMsa::undo()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);

    QVERIFY(subseq->setBioString( "---C-D---"));
    QVERIFY(subseq2->setBioString("---C-D---"));

    ObservableMsa oMsa;
    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(oMsa.append(subseq2));

    SubseqChangePod actualPod = oMsa.extendRight(1, 2);
    SubseqChangePod expectedPod = msa.extendRight(1, 2);

    QSignalSpy spySubseqsBorderChanged(&oMsa, SIGNAL(subseqsChanged(SubseqChangePodVector)));

    SubseqChangePod inversePod = oMsa.undo(SubseqChangePodVector() << actualPod).first();
    SubseqChangePod inverseExpectedPod = msa.undo(SubseqChangePodVector() << expectedPod).first();
    QCOMPARE(inversePod, inverseExpectedPod);

    QCOMPARE(spySubseqsBorderChanged.size(), 1);
    QCOMPARE(inverseExpectedPod, qVariantValue<SubseqChangePodVector>(spySubseqsBorderChanged.at(0).at(0)).first());
}

QTEST_APPLESS_MAIN(TestObservableMsa)
#include "TestObservableMsa.moc"
