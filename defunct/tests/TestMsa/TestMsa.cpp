/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include "AminoString.h"
#include "AnonSeq.h"
#include "BioString.h"
#include "DnaString.h"
#include "Msa.h"
#include "RnaString.h"
#include "Subseq.h"

class TestMsa : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorBasic();
//    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
//    void assignOther();

    // ------------------------------------------------------------------------------------------------
    void subseqGreaterThan();
    void subseqLessThan();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void alphabet();
    void append_and_count();
    void append_alphabet();     // Make sure that only Subseqs of the same alphabet may be appended
    void clear();
    void negativeColIndex();
    void negativeRowIndex();
    void positiveColIndex();
    void positiveRowIndex();
    void operator_paren();
    void at();
    void id();
    void insert();
    void insert_alphabet();     // Make sure that only Subseqs of the same alphabet may be inserted
    void isEmpty();
    void members();
    void prepend();
    void prepend_alphabet();    // Make sure that only Subseqs of the same alphabet may be prepended
    void removeFirst();
    void removeLast();
    void removeAt();
    void subseqIds();
    void swap();                    // Also tests the subseqsSwapped signal
    void moveRow();                 // Also tests the subseqsAboutToBeMoved, subseqsMoved signals
    void moveRowRelative();         // Also tests the subseqsAboutToBeMoved, subseqsMoved signals
    void moveRowRange();            // Also tests the subseqsAboutToBeMoved, subseqsMoved signals
    void moveRowRangeRelative();    // Also tests the subseqsAboutToBeMoved, subseqsMoved signals

    void description();
    void insertGapColumns();
    void name();
    void removeGapColumns();

    // Slide region may be called with any rectangular region defined by two coordinate points which are
    // specified in any order. Thus, we test each of these and indicate a corner using the following
    // notation: l = left, t = top, r = right, b = bottom
    // o left, top -> right, bottom
    // o right, bottom -> left, top
    // o left, bottom -> right, top
    // o right, top -> left, bottom

    void slideRegion_data();
    void slideRegion();         // Also tests, the regionSlid signal
    void slideRegionSpecific();
    void sortGreaterThan();
    void sortLessThan();

    void setSubseqStart();
    void setSubseqStop();

    void extendSubseqsLeft();
    void extendSubseqsRight();
    void trimSubseqsLeft();
    void trimSubseqsRight();
    void levelSubseqsLeft();
    void levelSubseqsRight();

    void collapseLeft();
    void collapseRight();

    // ------------------------------------------------------------------------------------------------
    // Signals
    void gapColumnsInsertedSignals();
    void gapColumnsRemovedSignals();
    void msaResetSignal();
    void subseqsInsertedSignals();
    void subseqsRemovedSignals();

private:
    // Utility function for adding test rows for the slideRegion test
    void addSlideRegionTestRows(const QString &direction, QList<int> leftList, QList<int> topList, QList<int> rightList, QList<int> bottomList);
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
void TestMsa::constructorBasic()
{
    Msa msa();
}

/*
void TestMsa::constructorCopy()
{
    Msa msa1;

    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    QVERIFY(msa1.append(subseq));
    QVERIFY(msa1.append(subseq2));

    Msa msa2(msa1);
    QVERIFY(msa2.length() == 6);
    QVERIFY(msa2.count() == 2);
    QVERIFY(msa2[1] == subseq);
    QVERIFY(msa2[2] == subseq2);
}
*/

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Assign other
/*
void TestMsa::assignOther()
{
    Msa msa1;
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    msa1.append(subseq);
    msa1.append(subseq2);

    Msa msa2;

    msa2 = msa1;

    QVERIFY(msa2.length() == 6);
    QVERIFY(msa2.count() == 2);
    QVERIFY(msa2[1] == subseq);
    QVERIFY(msa2[2] == subseq2);
}
*/

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// SubseqLessThan / SubseqGreaterThan sort helper classes
void TestMsa::subseqGreaterThan()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 1);

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, 2);

    Subseq *subseq3 = new Subseq(anonSeq2, 1);

    // Test: subseq's id is greater than subseq2's id
    SubseqGreaterThan sgt;
    QCOMPARE(sgt(subseq, subseq2), false);
    QCOMPARE(sgt(subseq2, subseq), true);
    QCOMPARE(sgt(subseq, subseq3), false);
    QCOMPARE(sgt(subseq3, subseq), false);

    delete subseq;
    delete subseq2;
    delete subseq3;

    subseq = 0;
    subseq2 = 0;
    subseq3 = 0;
}

void TestMsa::subseqLessThan()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 1);

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, 2);

    Subseq *subseq3 = new Subseq(anonSeq2, 1);

    // Test: subseq's id is greater than subseq2's id
    SubseqLessThan slt;
    QCOMPARE(slt(subseq, subseq2), true);
    QCOMPARE(slt(subseq2, subseq), false);
    QCOMPARE(slt(subseq, subseq3), false);
    QCOMPARE(slt(subseq3, subseq), false);

    delete subseq;
    delete subseq2;
    delete subseq3;

    subseq = 0;
    subseq2 = 0;
    subseq3 = 0;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestMsa::alphabet()
{
    Msa msa;
    Msa msa2(eAminoAlphabet);
    Msa msa3(eDnaAlphabet);
    Msa msa4(eRnaAlphabet);
    Msa msa5(eUnknownAlphabet);

    QCOMPARE(msa.alphabet(), eUnknownAlphabet);
    QCOMPARE(msa2.alphabet(), eAminoAlphabet);
    QCOMPARE(msa3.alphabet(), eDnaAlphabet);
    QCOMPARE(msa4.alphabet(), eRnaAlphabet);
    QCOMPARE(msa5.alphabet(), eUnknownAlphabet);
}

void TestMsa::append_and_count()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CD");

    Msa msa;
    QVERIFY(msa.length() == 0);

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.subseqCount() == 2);
    QVERIFY(msa.append(subseq3) == false);
    QVERIFY(msa.subseqCount() == 2);

    subseq3->insertGaps(5, 2);
    QVERIFY(msa.append(subseq3));
    QVERIFY(msa.subseqCount() == 3);

    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
}

void TestMsa::append_alphabet()
{
    // eUnknownAlphabet append is covered in the append_and_count() test routine

    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "ATCGCG");
    AnonSeq anonSeq3(3, "AUCGCG");

    // Because the Msa takes ownership of the subseqs and these are deleted when the owning Msa is deallocated,
    // it is necessary to allocate new Subseqs for each of the block tests.

    // Because the BioString type (AminoString, DnaString, or RnaString) passed to setBioString does not
    // affect the source BioString type, all of the above sequences should be able to be appended just fine
    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));

        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));

        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa;
        QVERIFY(msa.append(subseq));
        QVERIFY(msa.append(subseq2));
        QVERIFY(msa.append(subseq3));
        QVERIFY(msa.subseqCount() == 3);
    }

    // Test: specify a different alphabet, the above should not be able to be appended
    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));

        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));

        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eAminoAlphabet);
        QVERIFY(msa.append(subseq) == false);
        QVERIFY(msa.append(subseq2) == false);
        QVERIFY(msa.append(subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));

        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));

        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eDnaAlphabet);
        QVERIFY(msa.append(subseq) == false);
        QVERIFY(msa.append(subseq2) == false);
        QVERIFY(msa.append(subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));

        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));

        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eRnaAlphabet);
        QVERIFY(msa.append(subseq) == false);
        QVERIFY(msa.append(subseq2) == false);
        QVERIFY(msa.append(subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    // ------------------------------
    // Test: Same as above, except that the underlying AnonSeqs will have incompatible types
    AnonSeq anonSeq4(4, AminoString("ABCDEF"));
    AnonSeq anonSeq5(5, DnaString("ATCG"));
    AnonSeq anonSeq6(6, RnaString("AUCG"));

    // Because the BioString type (AminoString, DnaString, or RnaString) passed to setBioString does not
    // affect the source BioString type, all of the above sequences should be able to be appended just fine
    // to the msa
    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa;    // Unknown alphabet msa, should reject all subseqs with known types
        QVERIFY(msa.append(subseq4) == false);
        QVERIFY(msa.append(subseq5) == false);
        QVERIFY(msa.append(subseq6) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eAminoAlphabet);    // Should reject all but Amino subseqs
        QVERIFY(msa.append(subseq4));
        QVERIFY(msa.append(subseq5) == false);
        QVERIFY(msa.append(subseq6) == false);
        QVERIFY(msa.subseqCount() == 1);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eDnaAlphabet);    // Should reject all but Dna subseqs
        QVERIFY(msa.append(subseq4) == false);
        QVERIFY(msa.append(subseq5));
        QVERIFY(msa.append(subseq6) == false);
        QVERIFY(msa.subseqCount() == 1);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eRnaAlphabet);    // Should reject all but Dna subseqs
        QVERIFY(msa.append(subseq4) == false);
        QVERIFY(msa.append(subseq5) == false);
        QVERIFY(msa.append(subseq6));
        QVERIFY(msa.subseqCount() == 1);
    }
}

void TestMsa::clear()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CDEF");

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    QVERIFY(msa.subseqCount() == 3);
    msa.clear();
    QVERIFY(msa.subseqCount() == 0);
}

void TestMsa::negativeColIndex()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");
    Msa msa;

    // Test: no sequences in Msa, should return zero
    QVERIFY(msa.negativeColIndex(34) == 0);

    msa.append(subseq);
    int n = msa.length();

    // Test: 0 should return 0
    QVERIFY(msa.negativeColIndex(0) == 0);

    // Test: all positive numbers from 1..n+2
    for (int i=1; i<= n+2; ++i)
        QVERIFY(msa.negativeColIndex(i) == -(n - qAbs(i) + 1));

    // Test: all negative numbers from -1..-n-2
    for (int i=-1; i>= -n-2; --i)
        QVERIFY(msa.negativeColIndex(i) == i);
}

void TestMsa::negativeRowIndex()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    Msa msa;

    // Test: no sequences in Msa, should return zero
    QVERIFY(msa.negativeRowIndex(34) == 0);

    msa.append(subseq);
    int n = msa.subseqCount();

    // Test: 0 should return 0
    QVERIFY(msa.negativeRowIndex(0) == 0);

    // Test: all positive numbers from 1..n+2
    for (int i=1; i<= n+2; ++i)
        QVERIFY(msa.negativeRowIndex(i) == -(n - qAbs(i) + 1));

    // Test: all negative numbers from -1..-n-2
    for (int i=-1; i>= -n-2; --i)
        QVERIFY(msa.negativeRowIndex(i) == i);
}

void TestMsa::positiveColIndex()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");
    Msa msa;

    // Test: no sequences in Msa, should return zero
    QVERIFY(msa.positiveColIndex(34) == 0);

    msa.append(subseq);
    int n = msa.length();

    // Test: 0 should return 0
    QVERIFY(msa.positiveColIndex(0) == 0);

    // Test: all positive numbers from 1..n+2
    for (int i=1; i<= n+2; ++i)
        QVERIFY(msa.positiveColIndex(i) == i);

    // Test: all negative numbers from -1..-n-2
    for (int i=-1; i>= -n-2; --i)
        QVERIFY(msa.positiveColIndex(i) == n - qAbs(i) + 1);
}

void TestMsa::positiveRowIndex()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    Msa msa;

    // Test: no sequences in Msa, should return zero
    QVERIFY(msa.positiveRowIndex(34) == 0);

    msa.append(subseq);
    int n = msa.subseqCount();

    // Test: 0 should return 0
    QVERIFY(msa.positiveRowIndex(0) == 0);

    // Test: all positive numbers from 1..n+2
    for (int i=1; i<= n+2; ++i)
        QVERIFY(msa.positiveRowIndex(i) == i);

    // Test: all negative numbers from -1..-n-2
    for (int i=-1; i>= -n-2; --i)
        QVERIFY(msa.positiveRowIndex(i) == n - qAbs(i) + 1);
}

void TestMsa::operator_paren()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CDEF");

    Msa msa;

    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    QVERIFY(msa.subseqCount() == 3);

    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    QVERIFY(msa(-1) == subseq3);
    QVERIFY(msa(-2) == subseq2);
    QVERIFY(msa(-3) == subseq);
}

void TestMsa::at()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CDEF");

    Msa msa;

    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    QVERIFY(msa.subseqCount() == 3);

    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);

    QVERIFY(msa.at(-1) == subseq3);
    QVERIFY(msa.at(-2) == subseq2);
    QVERIFY(msa.at(-3) == subseq);
}

void TestMsa::id()
{
    Msa msa(eDnaAlphabet);
    QCOMPARE(msa.id(), 0);

    Msa msa2(eAminoAlphabet, 35);
    QCOMPARE(msa2.id(), 35);
}

void TestMsa::insert()
{
    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "GHIJKL");
    AnonSeq anonSeq3(3, "CDEF");

    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CDEF");

    Msa msa;

    // Test: insert at beginning
    QVERIFY(msa.insert(1, subseq));
    QVERIFY(msa.insert(1, subseq2));
    QVERIFY(msa.insert(1, subseq3));

    QVERIFY(msa.subseqCount() == 3);
    QVERIFY(msa.length() == 6);

    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq);

    msa.clear();

    // subseq, subseq2, subseq3 pointers are no longer valid because the msa was cleared
    subseq = 0;
    subseq2 = 0;
    subseq3 = 0;

    subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");
    subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");
    subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CDEF");

    // Test: insert in the middle
    msa.append(subseq);
    msa.append(subseq3);
    QVERIFY(msa.insert(2, subseq2));
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // Test: insert at the end
    msa.clear();

    // subseq, subseq2, subseq3 pointers are no longer valid because the msa was cleared
    subseq = 0;
    subseq2 = 0;
    subseq3 = 0;

    subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");
    subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");
    subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CDEF");

    QVERIFY(msa.insert(msa.subseqCount()+1, subseq));
    QVERIFY(msa.insert(msa.subseqCount()+1, subseq2));
    QVERIFY(msa.insert(msa.subseqCount()+1, subseq3));
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
}

// Not checking the insert position parameter, merely the type of biostring inserted
void TestMsa::insert_alphabet()
{
    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "ATCGCG");
    AnonSeq anonSeq3(3, "AUCGCG");

    // Because the BioString type (AminoString, DnaString, or RnaString) passed to setBioString does not
    // affect the source BioString type, all of the above sequences should be able to be inserted just fine
    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));
        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));
        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa;
        QVERIFY(msa.insert(1, subseq));
        QVERIFY(msa.insert(1, subseq2));
        QVERIFY(msa.insert(1, subseq3));
        QVERIFY(msa.subseqCount() == 3);
    }

    // Test: specify a different alphabet, the above should not be able to be appended
    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));
        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));
        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eAminoAlphabet);
        QVERIFY(msa.insert(1, subseq) == false);
        QVERIFY(msa.insert(1, subseq2) == false);
        QVERIFY(msa.insert(1, subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));
        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));
        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eDnaAlphabet);
        QVERIFY(msa.insert(1, subseq) == false);
        QVERIFY(msa.insert(1, subseq2) == false);
        QVERIFY(msa.insert(1, subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));
        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));
        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eRnaAlphabet);
        QVERIFY(msa.insert(1, subseq) == false);
        QVERIFY(msa.insert(1, subseq2) == false);
        QVERIFY(msa.insert(1, subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    // ------------------------------
    // Test: Same as above, except that the underlying AnonSeqs will have incompatible types
    AnonSeq anonSeq4(4, AminoString("ABCDEF"));
    AnonSeq anonSeq5(5, DnaString("ATCG"));
    AnonSeq anonSeq6(6, RnaString("AUCG"));

    // Because the BioString type (AminoString, DnaString, or RnaString) passed to setBioString does not
    // affect the source BioString type, all of the above sequences should be able to be appended just fine
    // to the msa
    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa;    // Unknown alphabet msa, should reject all subseqs with known types
        QVERIFY(msa.insert(1, subseq4) == false);
        QVERIFY(msa.insert(1, subseq5) == false);
        QVERIFY(msa.insert(1, subseq6) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eAminoAlphabet);    // Should reject all but Amino subseqs
        QVERIFY(msa.insert(1, subseq4));
        QVERIFY(msa.insert(1, subseq5) == false);
        QVERIFY(msa.insert(1, subseq6) == false);
        QVERIFY(msa.subseqCount() == 1);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eDnaAlphabet);    // Should reject all but Dna subseqs
        QVERIFY(msa.insert(1, subseq4) == false);
        QVERIFY(msa.insert(1, subseq5));
        QVERIFY(msa.insert(1, subseq6) == false);
        QVERIFY(msa.subseqCount() == 1);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eRnaAlphabet);    // Should reject all but Dna subseqs
        QVERIFY(msa.insert(1, subseq4) == false);
        QVERIFY(msa.insert(1, subseq5) == false);
        QVERIFY(msa.insert(1, subseq6));
        QVERIFY(msa.subseqCount() == 1);
    }
}

void TestMsa::isEmpty()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    Msa msa;

    QVERIFY(msa.isEmpty());

    msa.append(subseq);
    QVERIFY(msa.isEmpty() == false);

    msa.clear();

    QVERIFY(msa.isEmpty());
}

void TestMsa::members()
{
    Msa msa;

    // Test: initial state should have no members
    QVERIFY(msa.members().isEmpty());

    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CD");

    // Test: 1, 2, and 3 members
    QVERIFY(msa.append(subseq));
    QCOMPARE(msa.members().count(), 1);
    QVERIFY(msa.members().at(0) == subseq);

    QVERIFY(msa.append(subseq2));
    QCOMPARE(msa.members().count(), 2);
    QVERIFY(msa.members().at(0) == subseq);
    QVERIFY(msa.members().at(1) == subseq2);

    msa.clear();
    QVERIFY(msa.members().isEmpty());
}

void TestMsa::prepend()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CD");

    Msa msa;
    QVERIFY(msa.length() == 0);

    QVERIFY(msa.prepend(subseq));
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa.prepend(subseq2));
    QVERIFY(msa.subseqCount() == 2);
    QVERIFY(msa.prepend(subseq3) == false);
    QVERIFY(msa.subseqCount() == 2);

    subseq3->insertGaps(5, 2);
    QVERIFY(msa.prepend(subseq3));
    QVERIFY(msa.subseqCount() == 3);

    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq);
}

void TestMsa::prepend_alphabet()
{
    // eUnknownAlphabet append is covered in the append_and_count() test routine

    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "ATCGCG");
    AnonSeq anonSeq3(3, "AUCGCG");

    // Because the BioString type (AminoString, DnaString, or RnaString) passed to setBioString does not
    // affect the source BioString type, all of the above sequences should be able to be prepended just fine
    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));
        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));
        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa;
        QVERIFY(msa.prepend(subseq));
        QVERIFY(msa.prepend(subseq2));
        QVERIFY(msa.prepend(subseq3));
        QVERIFY(msa.subseqCount() == 3);
    }

    // Test: specify a different alphabet, the above should not be able to be prepended
    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));
        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));
        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eAminoAlphabet);
        QVERIFY(msa.prepend(subseq) == false);
        QVERIFY(msa.prepend(subseq2) == false);
        QVERIFY(msa.prepend(subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));
        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));
        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eDnaAlphabet);
        QVERIFY(msa.prepend(subseq) == false);
        QVERIFY(msa.prepend(subseq2) == false);
        QVERIFY(msa.prepend(subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq = new Subseq(anonSeq);
        subseq->setBioString(AminoString("BC--DE"));
        Subseq *subseq2 = new Subseq(anonSeq2);
        subseq2->setBioString(DnaString("AT--CG"));
        Subseq *subseq3 = new Subseq(anonSeq3);
        subseq3->setBioString(RnaString("AU--CG"));

        Msa msa(eRnaAlphabet);
        QVERIFY(msa.prepend(subseq) == false);
        QVERIFY(msa.prepend(subseq2) == false);
        QVERIFY(msa.prepend(subseq3) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    // ------------------------------
    // Test: Same as above, except that the underlying AnonSeqs will have incompatible types
    AnonSeq anonSeq4(4, AminoString("ABCDEF"));
    AnonSeq anonSeq5(5, DnaString("ATCG"));
    AnonSeq anonSeq6(6, RnaString("AUCG"));

    // Because the BioString type (AminoString, DnaString, or RnaString) passed to setBioString does not
    // affect the source BioString type, all of the above sequences should be able to be prepended just fine
    // to the msa
    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa;    // Unknown alphabet msa, should reject all subseqs with known types
        QVERIFY(msa.prepend(subseq4) == false);
        QVERIFY(msa.prepend(subseq5) == false);
        QVERIFY(msa.prepend(subseq6) == false);
        QVERIFY(msa.subseqCount() == 0);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eAminoAlphabet);    // Should reject all but Amino subseqs
        QVERIFY(msa.prepend(subseq4));
        QVERIFY(msa.prepend(subseq5) == false);
        QVERIFY(msa.prepend(subseq6) == false);
        QVERIFY(msa.subseqCount() == 1);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eDnaAlphabet);    // Should reject all but Dna subseqs
        QVERIFY(msa.prepend(subseq4) == false);
        QVERIFY(msa.prepend(subseq5));
        QVERIFY(msa.prepend(subseq6) == false);
        QVERIFY(msa.subseqCount() == 1);
    }

    {
        Subseq *subseq4 = new Subseq(anonSeq4);
        Subseq *subseq5 = new Subseq(anonSeq5);
        Subseq *subseq6 = new Subseq(anonSeq6);

        Msa msa(eRnaAlphabet);    // Should reject all but Dna subseqs
        QVERIFY(msa.prepend(subseq4) == false);
        QVERIFY(msa.prepend(subseq5) == false);
        QVERIFY(msa.prepend(subseq6));
        QVERIFY(msa.subseqCount() == 1);
    }
}

void TestMsa::removeFirst()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);

    msa.removeFirst();
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa(1) == subseq2);

    msa.removeFirst();
    QVERIFY(msa.subseqCount() == 0);
    msa.removeFirst();
    QVERIFY(msa.subseqCount() == 0);
}

void TestMsa::removeLast()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);

    msa.removeLast();
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa(1) == subseq);

    msa.removeLast();
    QVERIFY(msa.subseqCount() == 0);
    msa.removeLast();
    QVERIFY(msa.subseqCount() == 0);
}

void TestMsa::removeAt()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "ABCDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);

    // Test: remove the middle sequence
    msa.removeAt(2);
    QVERIFY(msa.subseqCount() == 2);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq3);

    msa.removeAt(-2);
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa(1) == subseq3);
}

void TestMsa::subseqIds()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 10);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, -10);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "ABCDEF");
    Subseq *subseq3 = new Subseq(anonSeq3, 30);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);

    QList<int> sids = msa.subseqIds();
    QCOMPARE(sids.count(), 3);
    QCOMPARE(sids.at(0), 10);
    QCOMPARE(sids.at(1), -10);
    QCOMPARE(sids.at(2), 30);
}

void TestMsa::swap()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "ABCDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);

    QSignalSpy spySubseqAboutToBeSwapped(&msa, SIGNAL(subseqAboutToBeSwapped(int,int)));
    QSignalSpy spySubseqSwapped(&msa, SIGNAL(subseqSwapped(int,int)));
    QVERIFY(spySubseqAboutToBeSwapped.isValid());
    QVERIFY(spySubseqSwapped.isValid());
    QList<QVariant> spyArguments;

    int n = msa.subseqCount();

    // Test: swap with itself does nothing
    for (int i=-n; i <= n; ++i)
    {
        if (i == 0)
            continue;

        msa.swap(i, i);

        // Nothing should have changed
        QVERIFY(msa(1) == subseq);
        QVERIFY(msa(2) == subseq2);
        QVERIFY(msa(3) == subseq3);

        QVERIFY(spySubseqAboutToBeSwapped.isEmpty());
        QVERIFY(spySubseqSwapped.isEmpty());
    }

    // Test: swap positive indices
    msa.swap(1, 3);
    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq);
    QCOMPARE(spySubseqAboutToBeSwapped.count(), 1);
    spyArguments = spySubseqAboutToBeSwapped.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spySubseqSwapped.count(), 1);
    QCOMPARE(spyArguments, spySubseqSwapped.takeFirst());
    spySubseqAboutToBeSwapped.clear();
    spySubseqSwapped.clear();

    msa.swap(3, 1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqAboutToBeSwapped.count(), 1);
    spyArguments = spySubseqAboutToBeSwapped.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spySubseqSwapped.count(), 1);
    QCOMPARE(spyArguments, spySubseqSwapped.takeFirst());
    spySubseqAboutToBeSwapped.clear();
    spySubseqSwapped.clear();

    msa.swap(1, 2);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq3);

    msa.swap(2, 1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // Test: same thing with negative indices
    spySubseqAboutToBeSwapped.clear();
    spySubseqSwapped.clear();
    msa.swap(-3, -1);
    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq);
    QCOMPARE(spySubseqAboutToBeSwapped.count(), 1);
    spyArguments = spySubseqAboutToBeSwapped.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spySubseqSwapped.count(), 1);
    QCOMPARE(spyArguments, spySubseqSwapped.takeFirst());
    spySubseqAboutToBeSwapped.clear();
    spySubseqSwapped.clear();

    msa.swap(-1, -3);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqAboutToBeSwapped.count(), 1);
    spyArguments = spySubseqAboutToBeSwapped.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spySubseqSwapped.count(), 1);
    QCOMPARE(spyArguments, spySubseqSwapped.takeFirst());
    spySubseqAboutToBeSwapped.clear();
    spySubseqSwapped.clear();

    msa.swap(-3, -2);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq3);

    msa.swap(-2, -3);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // Test: mix of positive and negative indices
    msa.swap(1, -1);
    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq);

    msa.swap(-1, 1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
}

void TestMsa::moveRow()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "ABCDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);

    QSignalSpy spySubseqsAboutToBeMoved(&msa, SIGNAL(subseqsAboutToBeMoved(int,int,int)));
    QSignalSpy spySubseqsMoved(&msa, SIGNAL(subseqsMoved(int,int,int)));
    QVERIFY(spySubseqsAboutToBeMoved.isValid());
    QVERIFY(spySubseqsMoved.isValid());
    QList<QVariant> spyArguments;

    int n = msa.subseqCount();

    // Test: swap with itself does nothing
    for (int i=-n; i <= n; ++i)
    {
        if (i == 0)
            continue;

        msa.moveRow(i, i);

        // Nothing should have changed
        QVERIFY(msa(1) == subseq);
        QVERIFY(msa(2) == subseq2);
        QVERIFY(msa(3) == subseq3);

        QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
        QVERIFY(spySubseqsMoved.isEmpty());
    }

    // Test: move positive indices
    msa.moveRow(1, 3);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRow(3, 1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRow(1, 2);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq3);

    msa.moveRow(2, 1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // Test: same thing with negative indices
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();
    msa.moveRow(-3, -1);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRow(-1, -3);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRow(-3, -2);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq3);

    msa.moveRow(-2, -3);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // Test: mix of positive and negative indices
    msa.moveRow(1, -1);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);

    msa.moveRow(-1, 1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
}

void TestMsa::moveRowRelative()
{
    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "GHIJKL");
    AnonSeq anonSeq3(3, "ABCDEF");

    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");
    Subseq *subseq3 = new Subseq(anonSeq3);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);

    QSignalSpy spySubseqsAboutToBeMoved(&msa, SIGNAL(subseqsAboutToBeMoved(int,int,int)));
    QSignalSpy spySubseqsMoved(&msa, SIGNAL(subseqsMoved(int,int,int)));
    QVERIFY(spySubseqsAboutToBeMoved.isValid());
    QVERIFY(spySubseqsMoved.isValid());
    QList<QVariant> spyArguments;

    // Test: moving row 0 elements, should do nothing
    QVERIFY(msa.moveRowRelative(1, 0) == 0);
    QVERIFY(msa.moveRowRelative(2, 0) == 0);
    QVERIFY(msa.moveRowRelative(3, 0) == 0);
    QVERIFY(msa.moveRowRelative(-3, 0) == 0);
    QVERIFY(msa.moveRowRelative(-2, 0) == 0);
    QVERIFY(msa.moveRowRelative(-1, 0) == 0);
    QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
    QVERIFY(spySubseqsMoved.isEmpty());

    // Test: moving beyond borders should not change anything
    QVERIFY(msa.moveRowRelative(1, -1) == 0);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    QVERIFY(msa.moveRowRelative(-3, -1) == 0);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    QVERIFY(msa.moveRowRelative(3, 1) == 0);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    QVERIFY(msa.moveRowRelative(-1, 1) == 0);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // All of the above tests should not have emitted any signals since nothing changed
    QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
    QVERIFY(spySubseqsMoved.isEmpty());

    // Test: Move one spot at a time
    QVERIFY(msa.moveRowRelative(1, 1) == 1);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRelative(2, 1) == 1);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);

    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();
    QVERIFY(msa.moveRowRelative(3, -1) == -1);

    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 2);

    QVERIFY(msa.moveRowRelative(2, -1) == -1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // Test: move 2 spots at a time
    QVERIFY(msa.moveRowRelative(1, 2) == 2);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);

    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();
    QVERIFY(msa.moveRowRelative(3, -2) == -2);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // Test: move beyond spots should only move allowable spots
    QVERIFY(msa.moveRowRelative(1, 3) == 2);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRelative(3, -3) == -2);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // -----
    // Test: negative from indices - same as above but with negative indices
    // Test: Move one spot at a time
    QVERIFY(msa.moveRowRelative(-3, 1) == 1);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRelative(-2, 1) == 1);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);

    QVERIFY(msa.moveRowRelative(-1, -1) == -1);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq3);

    QVERIFY(msa.moveRowRelative(-2, -1) == -1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // Test: move 2 spots at a time
    QVERIFY(msa.moveRowRelative(-3, 2) == 2);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);

    QVERIFY(msa.moveRowRelative(-1, -2) == -2);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);

    // Test: move beyond spots should only move allowable spots
    QVERIFY(msa.moveRowRelative(-3, 3) == 2);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq);

    QVERIFY(msa.moveRowRelative(-1, -3) == -2);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
}

void TestMsa::moveRowRange()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "ABCDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);

    AnonSeq anonSeq4(4, "XYZTUV");
    Subseq *subseq4 = new Subseq(anonSeq4);

    AnonSeq anonSeq5(5, "WRSTID");
    Subseq *subseq5 = new Subseq(anonSeq5);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    msa.append(subseq4);
    msa.append(subseq5);

    QSignalSpy spySubseqsAboutToBeMoved(&msa, SIGNAL(subseqsAboutToBeMoved(int,int,int)));
    QSignalSpy spySubseqsMoved(&msa, SIGNAL(subseqsMoved(int,int,int)));
    QVERIFY(spySubseqsAboutToBeMoved.isValid());
    QVERIFY(spySubseqsMoved.isValid());
    QList<QVariant> spyArguments;

    int n = msa.subseqCount();

    // Test: swap with itself does nothing
    for (int i=-n; i <= n; ++i)
    {
        if (i == 0)
            continue;

        msa.moveRowRange(i, i, i);

        // Nothing should have changed
        QVERIFY(msa(1) == subseq);
        QVERIFY(msa(2) == subseq2);
        QVERIFY(msa(3) == subseq3);
        QVERIFY(msa(4) == subseq4);
        QVERIFY(msa(5) == subseq5);

        QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
        QVERIFY(spySubseqsMoved.isEmpty());
    }

    // Test: two at a time
    for (int i=-n; i<= n-1; ++i)
    {
        if (i == 0 || i+1 == 0)
            continue;

        msa.moveRowRange(i, i+1, i);
        QVERIFY(msa(1) == subseq);
        QVERIFY(msa(2) == subseq2);
        QVERIFY(msa(3) == subseq3);
        QVERIFY(msa(4) == subseq4);
        QVERIFY(msa(5) == subseq5);

        QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
        QVERIFY(spySubseqsMoved.isEmpty());
    }

    // Test: move positive indices
    msa.moveRowRange(1, 2, 4);
    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq4);
    QVERIFY(msa(3) == subseq5);
    QVERIFY(msa(4) == subseq);
    QVERIFY(msa(5) == subseq2);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 4);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 4);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRowRange(4, 5, 1);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq4);
    QVERIFY(msa(5) == subseq5);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRowRange(2, 5, 1);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq4);
    QVERIFY(msa(4) == subseq5);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRowRange(1, 4, 2);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq4);
    QVERIFY(msa(5) == subseq5);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // Same thing with negative indices
    msa.moveRowRange(-5, -4, -2);
    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq4);
    QVERIFY(msa(3) == subseq5);
    QVERIFY(msa(4) == subseq);
    QVERIFY(msa(5) == subseq2);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 4);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 4);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRowRange(-2, -1, -5);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq4);
    QVERIFY(msa(5) == subseq5);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRowRange(-4, -1, -5);
    QVERIFY(msa(1) == subseq2);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq4);
    QVERIFY(msa(4) == subseq5);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRowRange(-5, -2, -4);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq4);
    QVERIFY(msa(5) == subseq5);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // Mix of negative and positive
    msa.moveRowRange(1, 3, -3);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq5);
    QVERIFY(msa(3) == subseq);
    QVERIFY(msa(4) == subseq2);
    QVERIFY(msa(5) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    msa.moveRowRange(-4, 4, 3);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq3);
    QVERIFY(msa(3) == subseq5);
    QVERIFY(msa(4) == subseq);
    QVERIFY(msa(5) == subseq2);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();
}

void TestMsa::moveRowRangeRelative()
{
    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "GHIJKL");
    AnonSeq anonSeq3(3, "ABCDEF");
    AnonSeq anonSeq4(4, "XYZTUV");
    AnonSeq anonSeq5(5, "WRSTID");

    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");
    Subseq *subseq3 = new Subseq(anonSeq3);
    Subseq *subseq4 = new Subseq(anonSeq4);
    Subseq *subseq5 = new Subseq(anonSeq5);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    msa.append(subseq4);
    msa.append(subseq5);

    QSignalSpy spySubseqsAboutToBeMoved(&msa, SIGNAL(subseqsAboutToBeMoved(int,int,int)));
    QSignalSpy spySubseqsMoved(&msa, SIGNAL(subseqsMoved(int,int,int)));
    QVERIFY(spySubseqsAboutToBeMoved.isValid());
    QVERIFY(spySubseqsMoved.isValid());
    QList<QVariant> spyArguments;

    // Test: moving beyond borders should not change anything
    QVERIFY(msa.moveRowRangeRelative(1, 3, -1) == 0);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq4);
    QVERIFY(msa(5) == subseq5);
    QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
    QVERIFY(spySubseqsMoved.isEmpty());

    QVERIFY(msa.moveRowRangeRelative(-5, -3, -1) == 0);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq4);
    QVERIFY(msa(5) == subseq5);
    QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
    QVERIFY(spySubseqsMoved.isEmpty());

    QVERIFY(msa.moveRowRangeRelative(3, 5, 1) == 0);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq4);
    QVERIFY(msa(5) == subseq5);
    QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
    QVERIFY(spySubseqsMoved.isEmpty());

    QVERIFY(msa.moveRowRangeRelative(-3, -1, 1) == 0);
    QVERIFY(msa(1) == subseq);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq4);
    QVERIFY(msa(5) == subseq5);
    QVERIFY(spySubseqsAboutToBeMoved.isEmpty());
    QVERIFY(spySubseqsMoved.isEmpty());

    // Test: Move one spot at a time
    QVERIFY(msa.moveRowRangeRelative(1, 3, 1) == 1);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq2);
    QVERIFY(msa(4) == subseq3);
    QVERIFY(msa(5) == subseq5);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(2, 4, 1) == 1);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq5);
    QVERIFY(msa(3) == subseq);
    QVERIFY(msa(4) == subseq2);
    QVERIFY(msa(5) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(4, 5, -1) == -1);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq5);
    QVERIFY(msa(3) == subseq2);
    QVERIFY(msa(4) == subseq3);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(3, 4, -1) == -1);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq5);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // Test: move 2 spots at a time
    QVERIFY(msa.moveRowRangeRelative(1, 2, 2) == 2);
    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq5);
    QVERIFY(msa(3) == subseq4);
    QVERIFY(msa(4) == subseq2);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(3, 4, -2) == -2);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq5);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // Test: move beyond spots should only move allowable spots
    QVERIFY(msa.moveRowRangeRelative(1, 3, 5) == 2);
    QVERIFY(msa(1) == subseq5);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq4);
    QVERIFY(msa(4) == subseq2);
    QVERIFY(msa(5) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(3, 5, -3) == -2);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq5);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // -----
    // Test: negative indices - same as above but with negative indices
    msa.clear();

    subseq = 0;
    subseq2 = 0;
    subseq3 = 0;
    subseq4 = 0;
    subseq5 = 0;

    subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");
    subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");
    subseq3 = new Subseq(anonSeq3);
    subseq4 = new Subseq(anonSeq4);
    subseq5 = new Subseq(anonSeq5);

    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    msa.append(subseq4);
    msa.append(subseq5);

    // Test: Move one spot at a time
    QVERIFY(msa.moveRowRangeRelative(-5, -3, 1) == 1);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq2);
    QVERIFY(msa(4) == subseq3);
    QVERIFY(msa(5) == subseq5);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(-4, -2, 1) == 1);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq5);
    QVERIFY(msa(3) == subseq);
    QVERIFY(msa(4) == subseq2);
    QVERIFY(msa(5) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(-2, -1, -1) == -1);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq5);
    QVERIFY(msa(3) == subseq2);
    QVERIFY(msa(4) == subseq3);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(-3, -2, -1) == -1);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq5);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // Test: move 2 spots at a time
    QVERIFY(msa.moveRowRangeRelative(-5, -4, 2) == 2);
    QVERIFY(msa(1) == subseq3);
    QVERIFY(msa(2) == subseq5);
    QVERIFY(msa(3) == subseq4);
    QVERIFY(msa(4) == subseq2);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(-3, -2, -2) == -2);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq5);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    // Test: move beyond spots should only move allowable spots
    QVERIFY(msa.moveRowRangeRelative(-5, -3, 5) == 2);
    QVERIFY(msa(1) == subseq5);
    QVERIFY(msa(2) == subseq);
    QVERIFY(msa(3) == subseq4);
    QVERIFY(msa(4) == subseq2);
    QVERIFY(msa(5) == subseq3);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();

    QVERIFY(msa.moveRowRangeRelative(-3, -1, -3) == -2);
    QVERIFY(msa(1) == subseq4);
    QVERIFY(msa(2) == subseq2);
    QVERIFY(msa(3) == subseq3);
    QVERIFY(msa(4) == subseq5);
    QVERIFY(msa(5) == subseq);
    QCOMPARE(spySubseqsAboutToBeMoved.count(), 1);
    spyArguments = spySubseqsAboutToBeMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    QCOMPARE(spySubseqsMoved.count(), 1);
    spyArguments = spySubseqsMoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spySubseqsAboutToBeMoved.clear();
    spySubseqsMoved.clear();
}


void TestMsa::description()
{
    Msa msa;

    QVERIFY(msa.description().isEmpty());
    msa.setDescription("Test description");
    QVERIFY(msa.description() == "Test description");
}

void TestMsa::insertGapColumns()
{
    AnonSeq anonSeq(1, "ABC");
    Subseq *subseq = new Subseq(anonSeq);

    AnonSeq anonSeq2(2, "DEF");
    Subseq *subseq2 = new Subseq(anonSeq2);

    AnonSeq anonSeq3(3, "GHI");
    Subseq *subseq3 = new Subseq(anonSeq3);

    AnonSeq anonSeq4(4, "JKL");
    Subseq *subseq4 = new Subseq(anonSeq4);

    AnonSeq anonSeq5(5, "MNO");
    Subseq *subseq5 = new Subseq(anonSeq5);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    msa.append(subseq4);
    msa.append(subseq5);

    // Test: inserting gap columns at any position with n = 0, does nothing
    for (int i=1; i<= msa.length()+1; ++i)
    {
        msa.insertGapColumns(i, 0);
        QVERIFY(msa(1)->bioString() == "ABC");
        QVERIFY(msa(2)->bioString() == "DEF");
        QVERIFY(msa(3)->bioString() == "GHI");
        QVERIFY(msa(4)->bioString() == "JKL");
        QVERIFY(msa(5)->bioString() == "MNO");
    }

    // Test: insert gap of 1 length at beginning
    msa.insertGapColumns(1, 1);
    QVERIFY(msa(1)->bioString() == "-ABC");
    QVERIFY(msa(2)->bioString() == "-DEF");
    QVERIFY(msa(3)->bioString() == "-GHI");
    QVERIFY(msa(4)->bioString() == "-JKL");
    QVERIFY(msa(5)->bioString() == "-MNO");

    // Test: insert another gap at the beginning
    msa.insertGapColumns(1, 1);
    QVERIFY(msa(1)->bioString() == "--ABC");
    QVERIFY(msa(2)->bioString() == "--DEF");
    QVERIFY(msa(3)->bioString() == "--GHI");
    QVERIFY(msa(4)->bioString() == "--JKL");
    QVERIFY(msa(5)->bioString() == "--MNO");

    // Test: insert gap in the middle
    msa.insertGapColumns(4, 1, '.');
    QVERIFY(msa(1)->bioString() == "--A.BC");
    QVERIFY(msa(2)->bioString() == "--D.EF");
    QVERIFY(msa(3)->bioString() == "--G.HI");
    QVERIFY(msa(4)->bioString() == "--J.KL");
    QVERIFY(msa(5)->bioString() == "--M.NO");

    // Test: insert another gap in the middle
    msa.insertGapColumns(5, 1);
    QVERIFY(msa(1)->bioString() == "--A--BC");
    QVERIFY(msa(2)->bioString() == "--D--EF");
    QVERIFY(msa(3)->bioString() == "--G--HI");
    QVERIFY(msa(4)->bioString() == "--J--KL");
    QVERIFY(msa(5)->bioString() == "--M--NO");

    // Test: insert gap at end
    msa.insertGapColumns(8, 1);
    QVERIFY(msa(1)->bioString() == "--A--BC-");
    QVERIFY(msa(2)->bioString() == "--D--EF-");
    QVERIFY(msa(3)->bioString() == "--G--HI-");
    QVERIFY(msa(4)->bioString() == "--J--KL-");
    QVERIFY(msa(5)->bioString() == "--M--NO-");

    // Test: insert another gap at end
    msa.insertGapColumns(8, 1);
    QVERIFY(msa(1)->bioString() == "--A--BC--");
    QVERIFY(msa(2)->bioString() == "--D--EF--");
    QVERIFY(msa(3)->bioString() == "--G--HI--");
    QVERIFY(msa(4)->bioString() == "--J--KL--");
    QVERIFY(msa(5)->bioString() == "--M--NO--");

    // Test: insertion of several gaps
    msa.insertGapColumns(7, 4);
    QVERIFY(msa(1)->bioString() == "--A--B----C--");
    QVERIFY(msa(2)->bioString() == "--D--E----F--");
    QVERIFY(msa(3)->bioString() == "--G--H----I--");
    QVERIFY(msa(4)->bioString() == "--J--K----L--");
    QVERIFY(msa(5)->bioString() == "--M--N----O--");
}

void TestMsa::name()
{
    Msa msa;

    msa.setName("Test name");
    QVERIFY(msa.name() == "Test name");
}

void TestMsa::removeGapColumns()
{
    AnonSeq anonSeq(1, "ABC");
    Subseq *subseq = new Subseq(anonSeq);

    AnonSeq anonSeq2(2, "DEF");
    Subseq *subseq2 = new Subseq(anonSeq2);

    AnonSeq anonSeq3(3, "GHI");
    Subseq *subseq3 = new Subseq(anonSeq3);

    subseq->setBioString("-A.B-C--");
    subseq2->setBioString("-D.E-F..");
    subseq3->setBioString("-G.H.I.-");

    Msa msa;

    // Test: empty msa
    QCOMPARE(msa.removeGapColumns(), 0);
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);

    QCOMPARE(msa.removeGapColumns(), 5);
    QVERIFY(msa(1)->bioString() == "ABC");
    QVERIFY(msa(2)->bioString() == "DEF");
    QVERIFY(msa(3)->bioString() == "GHI");

    QCOMPARE(msa.removeGapColumns(), 0);
    QVERIFY(msa(1)->bioString() == "ABC");
    QVERIFY(msa(2)->bioString() == "DEF");
    QVERIFY(msa(3)->bioString() == "GHI");
}

void TestMsa::slideRegion_data()
{
    QTest::addColumn<int>("left");
    QTest::addColumn<int>("top");
    QTest::addColumn<int>("right");
    QTest::addColumn<int>("bottom");
    QTest::addColumn<int>("delta");
    QTest::addColumn<int>("expectedDelta");
    QTest::addColumn<QString>("bioString1");
    QTest::addColumn<QString>("expectedBioString1");
    QTest::addColumn<QString>("bioString2");
    QTest::addColumn<QString>("expectedBioString2");
    QTest::addColumn<QString>("bioString3");
    QTest::addColumn<QString>("expectedBioString3");

    QTest::addColumn<bool>("signalEmitted");
    QTest::addColumn<int>("signalLeft");
    QTest::addColumn<int>("signalTop");
    QTest::addColumn<int>("signalRight");
    QTest::addColumn<int>("signalBottom");
    QTest::addColumn<int>("signalDelta");
    QTest::addColumn<int>("signalFinalLeft");
    QTest::addColumn<int>("signalFinalRight");

    QList<int> leftPositives;
    QList<int> leftNegatives;
    QList<int> rightPositives;
    QList<int> rightNegatives;
    QList<int> topPositives;
    QList<int> topNegatives;
    QList<int> bottomPositives;
    QList<int> bottomNegatives;

    leftPositives   <<  3 <<  3 <<  3 <<  4 <<  3 <<  5 <<  4 <<  6 <<  8 <<  6 <<  2 <<  1 <<  2 <<  3 <<  5 <<  3 <<  2 <<  2 <<  1 <<  3 <<  6 <<  7 <<  5;
    leftNegatives   << -6 << -6 << -6 << -5 << -6 << -4 << -5 << -3 << -1 << -3 << -7 << -8 << -7 << -6 << -4 << -6 << -7 << -7 << -8 << -6 << -3 << -2 << -4;

    topPositives    <<  1 <<  1 <<  1 <<  2 <<  2 <<  1 <<  3 <<  1 <<  1 <<  3 <<  2 <<  2 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  2 <<  2 <<  1;
    topNegatives    << -3 << -3 << -3 << -2 << -2 << -3 << -1 << -3 << -3 << -1 << -2 << -2 << -3 << -3 << -3 << -3 << -3 << -3 << -3 << -3 << -2 << -2 << -3;

    rightPositives  <<  5 <<  5 <<  4 <<  5 <<  4 <<  5 <<  4 <<  6 <<  8 <<  6 <<  2 <<  1 <<  2 <<  5 <<  7 <<  5 <<  4 <<  2 <<  1 <<  3 <<  7 <<  8 <<  5;
    rightNegatives  << -4 << -4 << -5 << -4 << -5 << -4 << -5 << -3 << -1 << -3 << -7 << -8 << -7 << -4 << -2 << -4 << -5 << -7 << -8 << -6 << -2 << -1 << -4;

    bottomPositives <<  3 <<  3 <<  2 <<  3 <<  3 <<  2 <<  3 <<  2 <<  2 <<  3 <<  3 <<  3 <<  1 <<  2 <<  2 <<  2 <<  2 <<  2 <<  2 <<  2 <<  3 <<  3 <<  2;
    bottomNegatives << -1 << -1 << -2 << -1 << -1 << -2 << -1 << -2 << -2 << -1 << -1 << -1 << -3 << -2 << -2 << -2 << -2 << -2 << -2 << -2 << -1 << -1 << -2;

    addSlideRegionTestRows("(left, top) -> (right, bottom) ", leftPositives, topPositives, rightPositives, bottomPositives);
    addSlideRegionTestRows("[N] (left, top) -> [N] (right, bottom) ", leftNegatives, topNegatives, rightNegatives, bottomNegatives);

    addSlideRegionTestRows("(right, bottom) -> (top, left) ", rightPositives, bottomPositives, leftPositives, topPositives);
    addSlideRegionTestRows("[N] (right, bottom) -> [N] (left, top) ", rightNegatives, bottomNegatives, leftNegatives, topNegatives);

    addSlideRegionTestRows("(left, bottom) -> (right, top) ", leftPositives, bottomPositives, rightPositives, topPositives);
    addSlideRegionTestRows("[N] (left, bottom) -> [N] (right, top) ", leftNegatives, bottomNegatives, rightNegatives, topNegatives);

    addSlideRegionTestRows("(right, top) -> (left, bottom) ", rightPositives, topPositives, leftPositives, bottomPositives);
    addSlideRegionTestRows("[N] (right, top) -> [N] (left, bottom) ", rightNegatives, topNegatives, leftNegatives, bottomNegatives);

    addSlideRegionTestRows("(left, top) -> [N] (right, bottom) ", leftPositives, topPositives, rightNegatives, bottomNegatives);
}

void TestMsa::addSlideRegionTestRows(const QString &direction, QList<int> leftList, QList<int> topList, QList<int> rightList, QList<int> bottomList)
{
    QTest::newRow(QString("%1 - sliding unmovable region :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << 0
            << "-A-B-C--" << "-A-B-C--"
            << "-D-E-F--" << "-D-E-F--"
            << "-G-H-I--" << "-G-H-I--"
            << false << 0 << 0 << 0 << 0 << 0 << 0 << 0;

    QTest::newRow(QString("%1 - sliding unmovable region :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 0
            << "-A-B-C--" << "-A-B-C--"
            << "-D-E-F--" << "-D-E-F--"
            << "-G-H-I--" << "-G-H-I--"
            << false << 0 << 0 << 0 << 0 << 0 << 0 << 0;

    QTest::newRow(QString("%1 - sliding internal non-gap region :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 1
            << "-A-B-C--" << "-A--BC--"
            << "-D-E-F--" << "-D--EF--"
            << "-G-H-I--" << "-G-H-I--"
            << true << 3 << 1 << 4 << 2 << 1 << 4 << 5;

    QTest::newRow(QString("%1 - sliding internal non-gap region :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << -1
            << "-A--BC--" << "-A--BC--"
            << "-D--EF--" << "-D-E-F--"
            << "-G-H-I--" << "-GH--I--"
            << true << 4 << 2 << 5 << 3 << -1 << 3 << 4;

    QTest::newRow(QString("%1 - attempt to move more than allowable amount :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 4 << 1
            << "-A--BC--" << "-A--BC--"
            << "-D-E-F--" << "-D--EF--"
            << "-GH--I--" << "-G-H-I--"
            << true << 3 << 2 << 4 << 3 << 1 << 4 << 5;

    QTest::newRow(QString("%1 - attempt to move more than allowable amount :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -5 << -2
            << "-A--BC--" << "-AB--C--"
            << "-D--EF--" << "-DE--F--"
            << "-G-H-I--" << "-G-H-I--"
            << true << 5 << 1 << 5 << 2 << -2 << 3 << 3;

    QTest::newRow(QString("%1 - slide single character :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << -1
            << "-AB--C--" << "-AB--C--"
            << "-DE--F--" << "-DE--F--"
            << "-G-H-I--" << "-GH--I--"
            << true << 4 << 3 << 4 << 3 << -1 << 3 << 3;

    QTest::newRow(QString("%1 - slide characters to rightmost position").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 2 << 2
            << "-AB--C--" << "-AB----C"
            << "-DE--F--" << "-DE----F"
            << "-GH--I--" << "-GH--I--"
            << true << 6 << 1 << 6 << 2 << 2 << 8 << 8;

    QTest::newRow(QString("%1 - attempt slide beyond right edge").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 0
            << "-AB----C" << "-AB----C"
            << "-DE----F" << "-DE----F"
            << "-GH--I--" << "-GH--I--"
            << false << 0 << 0 << 0 << 0 << 0 << 0 << 0;

    QTest::newRow(QString("%1 - attempt slide beyond right edge when not adjacent to right edge").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 4 << 2
            << "-AB----C" << "-AB----C"
            << "-DE----F" << "-DE----F"
            << "-GH--I--" << "-GH----I"
            << true << 6 << 3 << 6 << 3 << 2 << 8 << 8;

    QTest::newRow(QString("%1 - slide characters to leftmost position").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << -1
            << "-AB----C" << "-AB----C"
            << "-DE----F" << "D-E----F"
            << "-GH----I" << "G-H----I"
            << true << 2 << 2 << 2 << 3 << -1 << 1 << 1;

    QTest::newRow(QString("%1 - attempt to move beyond left edge").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << 0
            << "-AB----C" << "-AB----C"
            << "D-E----F" << "D-E----F"
            << "G-H----I" << "G-H----I"
            << false << 0 << 0 << 0 << 0 << 0 << 0 << 0;

    QTest::newRow(QString("%1 - attempt slide beyond left edge when not adjacent to left edge").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -4 << -1
            << "-AB----C" << "A-B----C"
            << "D-E----F" << "D-E----F"
            << "G-H----I" << "G-H----I"
            << true << 2 << 1 << 2 << 1 << -1 << 1 << 1;

    QTest::newRow(QString("%1 - slide region with non-gap characters, but rightmost column in rectangular region is purely gap characters :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 5 << 2
            << "A-B----C" << "A---B--C"
            << "D-E----F" << "D---E--F"
            << "G-H----I" << "G-H----I"
            << true << 3 << 1 << 5 << 2 << 2 << 5 << 7;

    QTest::newRow(QString("%1 - attempt unallowable slide with rectangular region :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 5 << 0
            << "A---B--C" << "A---B--C"
            << "D---E--F" << "D---E--F"
            << "G-H----I" << "G-H----I"
            << false << 0 << 0 << 0 << 0 << 0 << 0 << 0;

    QTest::newRow(QString("%1 - slide region with non-gap characters, but leftmost column in rectangular region is purely gap characters :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -5 << -1
            << "A---B--C" << "A--B---C"
            << "D---E--F" << "D--E---F"
            << "G-H----I" << "G-H----I"
            << true << 3 << 1 << 5 << 2 << -1 << 2 << 4;

    QTest::newRow(QString("%1 - attempt unallowable slide with rectangular region :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -5 << 0
            << "A--B---C" << "A--B---C"
            << "D--E---F" << "D--E---F"
            << "G-H----I" << "G-H----I"
            << false << 0 << 0 << 0 << 0 << 0 << 0 << 0;

    QTest::newRow(QString("%1 - sliding internal gap region :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << -1
            << "A--B---C" << "-A-B---C"
            << "D--E---F" << "-D-E---F"
            << "G-H----I" << "G-H----I"
            << true << 2 << 1 << 2 << 2 << -1 << 1 << 1;

    QTest::newRow(QString("%1 - sliding internal gap region beyond edge should fail :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << 0
            << "-A-B---C" << "-A-B---C"
            << "-D-E---F" << "-D-E---F"
            << "G-H----I" << "G-H----I"
            << false << 0 << 0 << 0 << 0 << 0 << 0 << 0;

    QTest::newRow(QString("%1 - sliding internal gap region beyond edge (partially allowable) :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -6 << -2
            << "-A-B---C" << "--AB---C"
            << "-D-E---F" << "--DE---F"
            << "G-H----I" << "G-H----I"
            << true << 3 << 1 << 3 << 2 << -2 << 1 << 1;

    QTest::newRow(QString("%1 - sliding internal gap region :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 1
            << "--AB---C" << "--AB---C"
            << "--DE---F" << "--DE-F--"
            << "G-H----I" << "G-H--I--"
            << true << 6 << 2 << 7 << 3 << 1 << 7 << 8;

    QTest::newRow(QString("%1 - sliding internal gap region beyond edge :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 0
            << "--AB---C" << "--AB---C"
            << "--DE-F--" << "--DE-F--"
            << "G-H--I--" << "G-H--I--"
            << false << 0 << 0 << 0 << 0 << 0 << 0 << 0;

    QTest::newRow(QString("%1 - sliding internal gap region beyond edge (partially allowable) :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 8 << 3
            << "--AB---C" << "--AB--C-"
            << "--DE-F--" << "--DEF---"
            << "G-H--I--" << "G-H--I--"
            << true << 5 << 1 << 5 << 2 << 3 << 8 << 8;
}

void TestMsa::slideRegion()
{
    QFETCH(int, left);
    QFETCH(int, top);
    QFETCH(int, right);
    QFETCH(int, bottom);
    QFETCH(int, delta);
    QFETCH(int, expectedDelta);
    QFETCH(QString, bioString1);
    QFETCH(QString, expectedBioString1);
    QFETCH(QString, bioString2);
    QFETCH(QString, expectedBioString2);
    QFETCH(QString, bioString3);
    QFETCH(QString, expectedBioString3);

    QFETCH(bool, signalEmitted);
    QFETCH(int, signalLeft);
    QFETCH(int, signalTop);
    QFETCH(int, signalRight);
    QFETCH(int, signalBottom);
    QFETCH(int, signalDelta);
    QFETCH(int, signalFinalLeft);
    QFETCH(int, signalFinalRight);

    AnonSeq anonSeq(1, bioString1);
    AnonSeq anonSeq2(2, bioString2);
    AnonSeq anonSeq3(3, bioString3);

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);
    Subseq *subseq3 = new Subseq(anonSeq3);

    subseq->setBioString(bioString1);
    subseq2->setBioString(bioString2);
    subseq3->setBioString(bioString3);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);

    QSignalSpy spyRegionSlid(&msa, SIGNAL(regionSlid(int,int,int,int,int,int,int)));
    QVERIFY(spyRegionSlid.isValid());
    QList<QVariant> spyArguments;

    QCOMPARE(msa.slideRegion(left, top, right, bottom, delta), expectedDelta);
    QCOMPARE(msa(1)->bioString().sequence(), expectedBioString1);
    QCOMPARE(msa(2)->bioString().sequence(), expectedBioString2);
    QCOMPARE(msa(3)->bioString().sequence(), expectedBioString3);

    if (signalEmitted)
    {
        QCOMPARE(spyRegionSlid.count(), 1);
        spyArguments = spyRegionSlid.takeFirst();
        QCOMPARE(spyArguments.at(0).toInt(), signalLeft);
        QCOMPARE(spyArguments.at(1).toInt(), signalTop);
        QCOMPARE(spyArguments.at(2).toInt(), signalRight);
        QCOMPARE(spyArguments.at(3).toInt(), signalBottom);
        QCOMPARE(spyArguments.at(4).toInt(), signalDelta);
        QCOMPARE(spyArguments.at(5).toInt(), signalFinalLeft);
        QCOMPARE(spyArguments.at(6).toInt(), signalFinalRight);
    }
    else
    {
        QVERIFY(spyRegionSlid.isEmpty());
    }
}

void TestMsa::collapseLeft()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq1 = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq);
    Subseq *subseq3 = new Subseq(anonSeq);

    Subseq *subseq4 = new Subseq(anonSeq);

    Msa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QSignalSpy spyCollapsedLeft(&msa, SIGNAL(collapsedLeft(MsaRect,int)));
    QSignalSpy spySubseqInternallyChanged(&msa, SIGNAL(subseqInternallyChanged(int,int,QString,QString)));
    QVariantList spyArguments;

    QStringList seqs;
    seqs << "A-BC-D-E"
         << "-A-BCD--"
         << "--A---BC";

    // Also tests that un-normalized rects produce expected result

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
                    QVERIFY(subseq1->setBioString(seqs.at(0)));
                    QVERIFY(subseq2->setBioString(seqs.at(1)));
                    QVERIFY(subseq3->setBioString(seqs.at(2)));

                    if (left % 2 == 0)
                    {
                        msaRect.setTop(top);
                        msaRect.setLeft(left);
                        msaRect.setRight(right);
                        msaRect.setBottom(bottom);
                    }
                    else
                    {
                        msaRect.setTop(bottom);
                        msaRect.setLeft(right);
                        msaRect.setRight(left);
                        msaRect.setBottom(top);
                    }

                    // Perform collapse
                    msa.collapseLeft(msaRect);

                    // Check the result
                    bool sequenceChanged = false;
                    int rightMostModifiedColumn = 0;
                    QList<QVariantList> expectedSubseqInternalChanges;

                    for (int i=1; i<=nSeqs; ++i)
                    {
                        if (i >= top && i <= bottom)
                        {
                            subseq4->setBioString(seqs.at(i-1));
                            QPair<int, int> affectedRange = subseq4->collapseLeft(left, right);
                            if (affectedRange.first != 0)
                            {
                                if (!sequenceChanged)
                                    sequenceChanged = true;

                                if (affectedRange.second > rightMostModifiedColumn)
                                    rightMostModifiedColumn = affectedRange.second;

                                int l = affectedRange.second - affectedRange.first + 1;
                                expectedSubseqInternalChanges.append(QVariantList() << i << affectedRange.first
                                                                     << subseq4->bioString().mid(affectedRange.first, l)
                                                                     << seqs.at(i-1).mid(affectedRange.first - 1, l));
                            }

                            QCOMPARE(msa.at(i)->bioString().sequence(), subseq4->bioString().sequence());
                        }
                        else
                        {
                            QCOMPARE(msa.at(i)->bioString().sequence(), seqs.at(i-1));
                        }
                    }

                    if (sequenceChanged)
                    {
                        // Check the signals
                        QCOMPARE(spyCollapsedLeft.count(), 1);
                        spyArguments = spyCollapsedLeft.takeFirst();
                        QCOMPARE(qvariant_cast<MsaRect>(spyArguments.at(0)), msaRect.normalized());
                        QCOMPARE(spyArguments.at(1).toInt(), rightMostModifiedColumn);

                        QCOMPARE(spySubseqInternallyChanged.count(), expectedSubseqInternalChanges.count());
                        for (int i=0; spySubseqInternallyChanged.count(); ++i)
                            QCOMPARE(spySubseqInternallyChanged.takeFirst(), expectedSubseqInternalChanges.takeFirst());
                    }
                    else
                    {
                        // Nothing should have changed as evidenced by a lack of signals
                        QVERIFY(spyCollapsedLeft.isEmpty());
                        QVERIFY(spySubseqInternallyChanged.isEmpty());
                    }
                }
            }
        }
    }

    delete subseq4;
    subseq4 = 0;
}

void TestMsa::collapseRight()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq1 = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq);
    Subseq *subseq3 = new Subseq(anonSeq);

    Subseq *subseq4 = new Subseq(anonSeq);

    Msa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QSignalSpy spyCollapsedRight(&msa, SIGNAL(collapsedRight(MsaRect,int)));
    QSignalSpy spySubseqInternallyChanged(&msa, SIGNAL(subseqInternallyChanged(int,int,QString,QString)));
    QVariantList spyArguments;

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
                    QVERIFY(subseq1->setBioString(seqs.at(0)));
                    QVERIFY(subseq2->setBioString(seqs.at(1)));
                    QVERIFY(subseq3->setBioString(seqs.at(2)));

                    if (left % 2 == 0)
                    {
                        msaRect.setTop(top);
                        msaRect.setLeft(left);
                        msaRect.setRight(right);
                        msaRect.setBottom(bottom);
                    }
                    else
                    {
                        msaRect.setTop(bottom);
                        msaRect.setLeft(right);
                        msaRect.setRight(left);
                        msaRect.setBottom(top);
                    }

                    // Perform collapse
                    msa.collapseRight(msaRect);

                    // Check the result
                    bool sequenceChanged = false;
                    int leftMostModifiedColumn = 99999999; // Some arbitrarily large number
                    QList<QVariantList> expectedSubseqInternalChanges;

                    for (int i=1; i<=nSeqs; ++i)
                    {
                        if (i >= top && i <= bottom)
                        {
                            subseq4->setBioString(seqs.at(i-1));

                            QPair<int, int> affectedRange = subseq4->collapseRight(left, right);
                            if (affectedRange.first != 0)
                            {
                                if (!sequenceChanged)
                                    sequenceChanged = true;

                                if (affectedRange.first < leftMostModifiedColumn)
                                    leftMostModifiedColumn = affectedRange.first;

                                int l = affectedRange.second - affectedRange.first + 1;
                                expectedSubseqInternalChanges.append(QVariantList() << i << affectedRange.first
                                                                     << subseq4->bioString().mid(affectedRange.first, l)
                                                                     << seqs.at(i-1).mid(affectedRange.first - 1, l));
                            }

//                            qDebug() << i << seqs.at(i-1) << subseq4->bioString().sequence();

                            QCOMPARE(msa.at(i)->bioString().sequence(), subseq4->bioString().sequence());
                        }
                        else
                        {
                            QCOMPARE(msa.at(i)->bioString().sequence(), seqs.at(i-1));
                        }
                    }

                    if (sequenceChanged)
                    {
                        // Check the signals
                        QCOMPARE(spyCollapsedRight.count(), 1);
                        spyArguments = spyCollapsedRight.takeFirst();
                        QCOMPARE(qvariant_cast<MsaRect>(spyArguments.at(0)), msaRect.normalized());
                        QCOMPARE(spyArguments.at(1).toInt(), leftMostModifiedColumn);

                        QCOMPARE(spySubseqInternallyChanged.count(), expectedSubseqInternalChanges.count());
                        for (int i=0; spySubseqInternallyChanged.count(); ++i)
                            QCOMPARE(spySubseqInternallyChanged.takeFirst(), expectedSubseqInternalChanges.takeFirst());
                    }
                    else
                    {
                        // Nothing should have changed as evidenced by a lack of signals
                        QVERIFY(spyCollapsedRight.isEmpty());
                        QVERIFY(spySubseqInternallyChanged.isEmpty());
                    }
                }
            }
        }
    }

    delete subseq4;
    subseq4 = 0;
}

void TestMsa::gapColumnsInsertedSignals()
{
    AnonSeq anonSeq(1, "ABC");
    Subseq *subseq = new Subseq(anonSeq);

    AnonSeq anonSeq2(2, "DEF");
    Subseq *subseq2 = new Subseq(anonSeq2);

    AnonSeq anonSeq3(3, "GHI");
    Subseq *subseq3 = new Subseq(anonSeq3);

    AnonSeq anonSeq4(4, "JKL");
    Subseq *subseq4 = new Subseq(anonSeq4);

    AnonSeq anonSeq5(5, "MNO");
    Subseq *subseq5 = new Subseq(anonSeq5);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    msa.append(subseq4);
    msa.append(subseq5);

    QSignalSpy spyGapColumnsAboutToBeInserted(&msa, SIGNAL(gapColumnsAboutToBeInserted(int,int)));
    QSignalSpy spyGapColumnsInserted(&msa, SIGNAL(gapColumnsInserted(int,int)));
    QVERIFY(spyGapColumnsAboutToBeInserted.isValid());
    QVERIFY(spyGapColumnsInserted.isValid());

    // Test: inserting gap columns at any position with n = 0 should not emit any signals
    for (int i=1; i<= msa.length()+1; ++i)
    {
        msa.insertGapColumns(i, 0);
        QCOMPARE(spyGapColumnsAboutToBeInserted.count(), 0);
        QCOMPARE(spyGapColumnsInserted.count(), 0);
    }

    // Test: insert a single gap column
    //
    // ABC  -->  -ABC
    // DEF  -->  -DEF
    // GHI  -->  -GHI
    // JKL  -->  -JKL
    // MNO  -->  -MNO
    msa.insertGapColumns(1, 1);
    QCOMPARE(spyGapColumnsAboutToBeInserted.count(), 1);
    QList<QVariant> spyArguments = spyGapColumnsAboutToBeInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);

    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);

    // Test: insert gap columns in middle
    //
    // -ABC  --> -AB--C
    // -DEF  --> -DE--F
    // -GHI  --> -GH--I
    // -JKL  --> -JK--L
    // -MNO  --> -MN--O
    spyGapColumnsAboutToBeInserted.clear();
    spyGapColumnsInserted.clear();
    msa.insertGapColumns(4, 2);
    QCOMPARE(spyGapColumnsAboutToBeInserted.count(), 1);
    spyArguments = spyGapColumnsAboutToBeInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

    // Test: insert gap column at end
    //
    // -AB--C  --> -AB--C---
    // -DE--F  --> -DE--F---
    // -GH--I  --> -GH--I---
    // -JK--L  --> -JK--L---
    // -MN--O  --> -MN--O---
    spyGapColumnsAboutToBeInserted.clear();
    spyGapColumnsInserted.clear();
    msa.insertGapColumns(7, 3);
    QCOMPARE(spyGapColumnsAboutToBeInserted.count(), 1);
    spyArguments = spyGapColumnsAboutToBeInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 7);
    QCOMPARE(spyArguments.at(1).toInt(), 3);

    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 7);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
}

void TestMsa::gapColumnsRemovedSignals()
{
    AnonSeq anonSeq(1, "ABC");
    AnonSeq anonSeq2(2, "DEF");
    AnonSeq anonSeq3(3, "GHI");
    AnonSeq anonSeq4(4, "JKL");
    AnonSeq anonSeq5(5, "MNO");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);
    Subseq *subseq3 = new Subseq(anonSeq3);
    Subseq *subseq4 = new Subseq(anonSeq4);
    Subseq *subseq5 = new Subseq(anonSeq5);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    msa.append(subseq4);
    msa.append(subseq5);

    QSignalSpy spyGapColumnsAboutToBeRemoved(&msa, SIGNAL(gapColumnsAboutToBeRemoved(int)));
    QSignalSpy spyGapColumnsRemoved(&msa, SIGNAL(gapColumnsRemoved(int)));
    QVERIFY(spyGapColumnsAboutToBeRemoved.isValid());
    QVERIFY(spyGapColumnsRemoved.isValid());
    QList<QVariant> spyArguments;

    // Test: Remove gap columns when there are none, should not emit any signal
    QCOMPARE(msa.removeGapColumns(), 0);
    QCOMPARE(spyGapColumnsAboutToBeRemoved.count(), 0);
    QCOMPARE(spyGapColumnsRemoved.count(), 0);

    subseq->setBioString("-AB--C---");
    subseq2->setBioString("-DE--F---");
    subseq3->setBioString("-GH--I---");
    subseq4->setBioString("-JK--L---");
    subseq5->setBioString("-MN--O---");

    QCOMPARE(msa.removeGapColumns(), 6);
    QCOMPARE(spyGapColumnsAboutToBeRemoved.count(), 1);
    spyArguments = spyGapColumnsAboutToBeRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 6);
    QCOMPARE(spyGapColumnsRemoved.count(), 1);
    spyArguments = spyGapColumnsRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 6);
}

void TestMsa::msaResetSignal()
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CDEF");

    Msa msa;

    QSignalSpy spyMsaReset(&msa, SIGNAL(msaReset()));
    QVERIFY(spyMsaReset.isValid());

    msa.clear();
    QCOMPARE(spyMsaReset.count(), 1);
    spyMsaReset.clear();

    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    msa.clear();
    QCOMPARE(spyMsaReset.count(), 1);
    spyMsaReset.clear();
}

void TestMsa::subseqsInsertedSignals()
{
    AnonSeq anonSeq(1, "ABC");
    AnonSeq anonSeq2(2, "DEF");
    AnonSeq anonSeq3(3, "GHI");
    AnonSeq anonSeq4(4, "JKL");
    AnonSeq anonSeq5(5, "MNO");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);
    Subseq *subseq3 = new Subseq(anonSeq3);
    Subseq *subseq4 = new Subseq(anonSeq4);

    Msa msa;

    QSignalSpy spySubseqsAboutToBeInserted(&msa, SIGNAL(subseqsAboutToBeInserted(int,int)));
    QSignalSpy spySubseqsInserted(&msa, SIGNAL(subseqsInserted(int,int)));
    QVERIFY(spySubseqsAboutToBeInserted.isValid());
    QVERIFY(spySubseqsInserted.isValid());
    QList<QVariant> spyArguments;

    // Three methods for adding sequences: insert, prepend, append. Need to test each of these and ensure
    // that all three emit the signals for insertion
    QVERIFY(msa.append(subseq));
    QCOMPARE(spySubseqsAboutToBeInserted.count(), 1);
    spyArguments = spySubseqsAboutToBeInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spySubseqsInserted.count(), 1);
    spyArguments = spySubseqsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    spySubseqsAboutToBeInserted.clear();
    spySubseqsInserted.clear();

    QVERIFY(msa.append(subseq2));
    spyArguments = spySubseqsAboutToBeInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spySubseqsInserted.count(), 1);
    spyArguments = spySubseqsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    spySubseqsAboutToBeInserted.clear();
    spySubseqsInserted.clear();

    QVERIFY(msa.prepend(subseq3));
    spyArguments = spySubseqsAboutToBeInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spySubseqsInserted.count(), 1);
    spyArguments = spySubseqsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    spySubseqsAboutToBeInserted.clear();
    spySubseqsInserted.clear();

    QVERIFY(msa.insert(3, subseq4));
    spyArguments = spySubseqsAboutToBeInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spySubseqsInserted.count(), 1);
    spyArguments = spySubseqsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    spySubseqsAboutToBeInserted.clear();
    spySubseqsInserted.clear();
}

void TestMsa::subseqsRemovedSignals()
{
    AnonSeq anonSeq(1, "ABC");
    AnonSeq anonSeq2(2, "DEF");
    AnonSeq anonSeq3(3, "GHI");
    AnonSeq anonSeq4(4, "JKL");
    AnonSeq anonSeq5(5, "MNO");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);
    Subseq *subseq3 = new Subseq(anonSeq3);
    Subseq *subseq4 = new Subseq(anonSeq4);
    Subseq *subseq5 = new Subseq(anonSeq5);

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);
    msa.append(subseq3);
    msa.append(subseq4);
    msa.append(subseq5);

    QSignalSpy spySubseqsAboutToBeRemoved(&msa, SIGNAL(subseqsAboutToBeRemoved(int,int)));
    QSignalSpy spySubseqsRemoved(&msa, SIGNAL(subseqsRemoved(int,int)));
    QVERIFY(spySubseqsAboutToBeRemoved.isValid());
    QVERIFY(spySubseqsRemoved.isValid());
    QList<QVariant> spyArguments;

    // Three methods for adding sequences: removeFirst, removeLast, removeAt. Need to test each of these and ensure
    // that all three emit the signals for removal
    msa.removeFirst();
    QCOMPARE(spySubseqsAboutToBeRemoved.count(), 1);
    spyArguments = spySubseqsAboutToBeRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spySubseqsRemoved.count(), 1);
    QCOMPARE(spyArguments, spySubseqsRemoved.takeFirst());
    spySubseqsAboutToBeRemoved.clear();
    spySubseqsRemoved.clear();

    msa.removeLast();
    QCOMPARE(spySubseqsAboutToBeRemoved.count(), 1);
    spyArguments = spySubseqsAboutToBeRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spySubseqsRemoved.count(), 1);
    QCOMPARE(spyArguments, spySubseqsRemoved.takeFirst());
    spySubseqsAboutToBeRemoved.clear();
    spySubseqsRemoved.clear();

    msa.removeAt(2);
    QCOMPARE(spySubseqsAboutToBeRemoved.count(), 1);
    spyArguments = spySubseqsAboutToBeRemoved.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spySubseqsRemoved.count(), 1);
    QCOMPARE(spyArguments, spySubseqsRemoved.takeFirst());
    spySubseqsAboutToBeRemoved.clear();
    spySubseqsRemoved.clear();
}

// Specific test cases to make sure things are working as expected!
void TestMsa::slideRegionSpecific()
{
    AnonSeq anonSeq1(1, "AAB");
    AnonSeq anonSeq2(2, "ABX");
    AnonSeq anonSeq3(3, "AX");

    Subseq *subseq1 = new Subseq(anonSeq1, 1);
    Subseq *subseq2 = new Subseq(anonSeq2, 2);
    Subseq *subseq3 = new Subseq(anonSeq3, 3);

    subseq1->setBioString("-AAB--");
    subseq2->setBioString("--AB-X");
    subseq3->setBioString("---A-X");

    Msa *msa = new Msa();
    msa->append(subseq1);
    msa->append(subseq2);
    msa->append(subseq3);

    // ------------------------------------------------------------------------
    // Test: slide region in negative direction, but limit to delta requested spots even if more are available
    msa->slideRegion(3, 2, 4, 3, -1);
    QCOMPARE(msa->at(1)->bioString().sequence(), QString("-AAB--"));
    QCOMPARE(msa->at(2)->bioString().sequence(), QString("-AB--X"));
    QCOMPARE(msa->at(3)->bioString().sequence(), QString("--A--X"));

    // ------------------------------------------------------------------------
    // Test: slide region in positive direction, but limit to delta requested spots even if more are available
    msa->slideRegion(2, 2, 3, 3, 1);
    QCOMPARE(msa->at(1)->bioString().sequence(), QString("-AAB--"));
    QCOMPARE(msa->at(2)->bioString().sequence(), QString("--AB-X"));
    QCOMPARE(msa->at(3)->bioString().sequence(), QString("---A-X"));
}

// Descending order
void TestMsa::sortGreaterThan()
{
    Msa msa;

    QSignalSpy spySubseqOrderAboutToBeChanged(&msa, SIGNAL(subseqsAboutToBeSorted()));
    QSignalSpy spySubseqOrderChanged(&msa, SIGNAL(subseqsSorted()));

    // Test: with no subseqs, nothing should happen and no signals emitted when sort is called
    QVERIFY(msa.isEmpty());
    msa.sort(SubseqGreaterThan());
    QVERIFY(spySubseqOrderAboutToBeChanged.isEmpty());
    QVERIFY(spySubseqOrderChanged.isEmpty());

    // Test: already in descending order, should not change anything, but still emit signals
    AnonSeq anonSeq1(1, "ABC");
    AnonSeq anonSeq2(2, "DEF");
    AnonSeq anonSeq3(3, "GHI");

    Subseq *subseq1 = new Subseq(anonSeq1, 1);
    Subseq *subseq2 = new Subseq(anonSeq2, 2);
    Subseq *subseq3 = new Subseq(anonSeq3, 3);

    msa.append(subseq3);
    msa.append(subseq2);
    msa.append(subseq1);

    msa.sort(SubseqGreaterThan());

    QVERIFY(msa.at(1) == subseq3);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq1);
    QCOMPARE(spySubseqOrderAboutToBeChanged.count(), 1);
    QCOMPARE(spySubseqOrderChanged.count(), 1);
    spySubseqOrderAboutToBeChanged.clear();
    spySubseqOrderChanged.clear();

    // Test: out of order, should order based on their id in descending order
    msa.clear();    // This also frees the memory still pointed to by subseq{1,2,3}, must re-allocated those
    subseq1 = new Subseq(anonSeq1, 1);
    subseq2 = new Subseq(anonSeq2, 2);
    subseq3 = new Subseq(anonSeq3, 3);

    msa.append(subseq2);
    msa.append(subseq1);
    msa.append(subseq3);

    msa.sort(SubseqGreaterThan());

    QVERIFY(msa.at(1) == subseq3);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq1);
    QCOMPARE(spySubseqOrderAboutToBeChanged.count(), 1);
    QCOMPARE(spySubseqOrderChanged.count(), 1);
    spySubseqOrderAboutToBeChanged.clear();
    spySubseqOrderChanged.clear();
}

// Ascending order
void TestMsa::sortLessThan()
{
    Msa msa;

    QSignalSpy spySubseqOrderAboutToBeChanged(&msa, SIGNAL(subseqsAboutToBeSorted()));
    QSignalSpy spySubseqOrderChanged(&msa, SIGNAL(subseqsSorted()));

    // Test: with no subseqs, nothing should happen and no signals emitted when sort is called
    QVERIFY(msa.isEmpty());
    msa.sort(SubseqLessThan());
    QVERIFY(spySubseqOrderAboutToBeChanged.isEmpty());
    QVERIFY(spySubseqOrderChanged.isEmpty());

    // Test: already in ascending order, should not change anything
    AnonSeq anonSeq1(1, "ABC");
    AnonSeq anonSeq2(2, "DEF");
    AnonSeq anonSeq3(3, "GHI");

    Subseq *subseq1 = new Subseq(anonSeq1, 1);
    Subseq *subseq2 = new Subseq(anonSeq2, 2);
    Subseq *subseq3 = new Subseq(anonSeq3, 3);

    msa.append(subseq1);
    msa.append(subseq2);
    msa.append(subseq3);

    msa.sort(SubseqLessThan());

    QVERIFY(msa.at(1) == subseq1);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
    QCOMPARE(spySubseqOrderAboutToBeChanged.count(), 1);
    QCOMPARE(spySubseqOrderChanged.count(), 1);
    spySubseqOrderAboutToBeChanged.clear();
    spySubseqOrderChanged.clear();

    // Test: out of order, should order based on their id in ascending order
    msa.clear();    // This also frees the memory still pointed to by subseq{1,2,3}, must re-allocated those
    subseq1 = new Subseq(anonSeq1, 1);
    subseq2 = new Subseq(anonSeq2, 2);
    subseq3 = new Subseq(anonSeq3, 3);

    msa.append(subseq3);
    msa.append(subseq1);
    msa.append(subseq2);

    msa.sort(SubseqLessThan());

    QVERIFY(msa.at(1) == subseq1);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
    QCOMPARE(spySubseqOrderAboutToBeChanged.count(), 1);
    QCOMPARE(spySubseqOrderChanged.count(), 1);
    spySubseqOrderAboutToBeChanged.clear();
    spySubseqOrderChanged.clear();
}

void TestMsa::setSubseqStart()
{
    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "TUVWXYZ");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);

    QVERIFY(subseq->setBioString( "ABC-"));
    QVERIFY(subseq2->setBioString("--WX"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spySubseqStartChanged(&msa, SIGNAL(subseqStartChanged(int,int,int)));
    QSignalSpy spySubseqStopChanged(&msa, SIGNAL(subseqStopChanged(int,int,int)));
    QSignalSpy spyGapColumnsInserted(&msa, SIGNAL(gapColumnsInserted(int,int)));
    QSignalSpy spySubseqExtended(&msa, SIGNAL(subseqExtended(int,int,QString)));
    QSignalSpy spySubseqTrimmed(&msa, SIGNAL(subseqTrimmed(int,int,QString)));
    QSignalSpy spyExtendOrTrimFinished(&msa, SIGNAL(extendOrTrimFinished(int,int)));
    QVariantList spyArguments;

//    QSignalSpy spySubseqBorderChanged(&msa, SIGNAL(subseqBorderChanged(int,int,QString,QString)));

    // ------------------------------------------------------------------------
    // Test: set start to identical start - no change
    msa.setSubseqStart(1, 1);
    QVERIFY(spySubseqStartChanged.isEmpty());
    QVERIFY(spySubseqStopChanged.isEmpty());
    QVERIFY(spySubseqExtended.isEmpty());
    QVERIFY(spySubseqTrimmed.isEmpty());
    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    // ------------------------------------------------------------------------
    // Test: move start forward 2 spots
    // ABC- >>> --C-
    msa.setSubseqStart(1, 3);

    // Start position changes
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 3);    // new start
    QCOMPARE(spyArguments.at(2).toInt(), 1);    // old start
    QCOMPARE(subseq->bioString().sequence(), QString("--C-"));

    // No stop updated
    QVERIFY(spySubseqStopChanged.isEmpty());

    // No extension
    QVERIFY(spySubseqExtended.isEmpty());

    // Trim signal
    QCOMPARE(spySubseqTrimmed.count(), 1);
    spyArguments = spySubseqTrimmed.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toString(), QString("AB"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 1);    // Column 1
//    QCOMPARE(spyArguments.at(2).toString(), QString("--")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("AB")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: move start beyond stop
    // --C- >>> ---D
    //
    // Behind the scenes: --C- >>> --CD >>> ---D ; and thus both an extended and trimmed signal
    msa.setSubseqStart(1, 4);

    // Stop position updated
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 4);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 3);    // old stop

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toString(), QString("D"));

    // Start position updated
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 4);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 3);    // old stop

    // Trimmed signal
    QCOMPARE(spySubseqTrimmed.count(), 1);
    spyArguments = spySubseqTrimmed.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toString(), QString("C"));

    // Check that sequence is as expected
    QCOMPARE(subseq->bioString().sequence(), QString("---D"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 3);    // Column 1
//    QCOMPARE(spyArguments.at(2).toString(), QString("-D")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("C-")); // Old alignment


    // ------------------------------------------------------------------------
    // Test: move start beyond right alignment terminus
    // ---D >>> -----F
    //
    // Real order of operations:
    // ---D   >>> ---D-- (insert two terminal gap columns)
    // ---D-- >>> -----F (move the start and stop)
    msa.setSubseqStart(1, 6);

    // Gap columns inserted
    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);    // column
    QCOMPARE(spyArguments.at(1).toInt(), 2);    // count

    // Stop position updated
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 6);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 4);    // old stop

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toString(), QString("EF"));

    // Start position updated
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 6);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 4);    // old stop

    // Trimmed signal
    QCOMPARE(spySubseqTrimmed.count(), 1);
    spyArguments = spySubseqTrimmed.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 4);
    QCOMPARE(spyArguments.at(2).toString(), QString("DE"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 6);

    // Check final sequence
    QCOMPARE(subseq->bioString().sequence(), QString( "-----F"));
    QCOMPARE(subseq2->bioString().sequence(), QString("--WX--"));

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 4);    // Column 1
//    QCOMPARE(spyArguments.at(2).toString(), QString("--F")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("D--")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: move start backward (to the left)
    // --WX-- >>> UVWX--
    msa.setSubseqStart(2, 2);

    // Start position updated
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 2);    // new start
    QCOMPARE(spyArguments.at(2).toInt(), 4);    // old start

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toString(), QString("UV"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

    // Check the sequence
    QCOMPARE(subseq2->bioString().sequence(), QString("UVWX--"));

    // No other signals
    QVERIFY(spyGapColumnsInserted.isEmpty());
    QVERIFY(spySubseqStopChanged.isEmpty());
    QVERIFY(spySubseqTrimmed.isEmpty());

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 2);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 1);    // Column 1
//    QCOMPARE(spyArguments.at(2).toString(), QString("UV")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("--")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: move start beyond left alignment border
    // UVWX-- >>> TUVWX--
    //
    // Real order of operations:
    // UVWX--  >>> -UVWX--  (insert terminal gap column)
    // -UVWX-- >>> TUVWX--  (move the start)
    msa.setSubseqStart(2, 1);

    // Check that gap column was inserted
    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // column
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // count

    // Start position updated
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // new start
    QCOMPARE(spyArguments.at(2).toInt(), 2);    // old start

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toString(), QString("T"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);

    // Check the sequence
    QCOMPARE(subseq->bioString().sequence(), QString( "------F"));
    QCOMPARE(subseq2->bioString().sequence(), QString("TUVWX--"));

    // No other signals should be present
    QVERIFY(spySubseqStopChanged.isEmpty());
    QVERIFY(spySubseqTrimmed.isEmpty());

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 2);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 1);    // Column 1
//    QCOMPARE(spyArguments.at(2).toString(), QString("T")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("-")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: moving start beyond stop in operation that also includes gaps
    // -A-B >>> ----C
    //
    // Real order of operations:
    // -A-B  >>> -A-B- (insert terminal gap)
    // -A-B- >>> -A-BC (extend stop)
    // -A-BC >>> ----C (move start)
    msa.clear();
    subseq = new Subseq(anonSeq);
    subseq2 = 0;

    QVERIFY(subseq->setBioString("-A-B"));
    QVERIFY(msa.append(subseq));

    msa.setSubseqStart(1, 3);

    // Check that gap column was inserted
    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);    // column
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // count

    // Stop position updated
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 3);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 2);    // old stop

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
    QCOMPARE(spyArguments.at(2).toString(), QString("C"));

    // Start position updated
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 3);    // new start
    QCOMPARE(spyArguments.at(2).toInt(), 1);    // old start

    // Trimmed signal
    QCOMPARE(spySubseqTrimmed.count(), 1);
    spyArguments = spySubseqTrimmed.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toString(), QString("A-B"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    // Check final sequence
    QCOMPARE(subseq->bioString().sequence(), QString( "----C"));
}

void TestMsa::setSubseqStop()
{
    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "TUVWXYZ");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);

    //                              345
    QVERIFY(subseq->setBioString( "-CDE--"));
    //                                45
    QVERIFY(subseq2->setBioString("---WX-"));

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);

    QSignalSpy spySubseqStartChanged(&msa, SIGNAL(subseqStartChanged(int,int,int)));
    QSignalSpy spySubseqStopChanged(&msa, SIGNAL(subseqStopChanged(int,int,int)));
    QSignalSpy spyGapColumnsInserted(&msa, SIGNAL(gapColumnsInserted(int,int)));
    QSignalSpy spySubseqExtended(&msa, SIGNAL(subseqExtended(int,int,QString)));
    QSignalSpy spySubseqTrimmed(&msa, SIGNAL(subseqTrimmed(int,int,QString)));
    QSignalSpy spyExtendOrTrimFinished(&msa, SIGNAL(extendOrTrimFinished(int,int)));
    QVariantList spyArguments;

//    QSignalSpy spySubseqBorderChanged(&msa, SIGNAL(subseqBorderChanged(int,int,QString,QString)));

    // ------------------------------------------------------------------------
    // Test: set stop to identical stop - no change
    msa.setSubseqStop(1, 5);
    QVERIFY(spySubseqStartChanged.isEmpty());
    QVERIFY(spySubseqStopChanged.isEmpty());
    QVERIFY(spySubseqExtended.isEmpty());
    QVERIFY(spySubseqTrimmed.isEmpty());
    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    // ------------------------------------------------------------------------
    // Test: move stop backward 2 spots
    // -CDE-- >>> -C----
    msa.setSubseqStop(1, 3);

    // Stop position changed
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 3);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 5);    // old stop

    // No start updated
    QVERIFY(spySubseqStartChanged.isEmpty());

    // No extension
    QVERIFY(spySubseqExtended.isEmpty());

    // Trim signal
    QCOMPARE(spySubseqTrimmed.count(), 1);
    spyArguments = spySubseqTrimmed.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toString(), QString("DE"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // Sequence check
    QCOMPARE(subseq->bioString().sequence(), QString("-C----"));

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 3);    // Column
//    QCOMPARE(spyArguments.at(2).toString(), QString("--")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("DE")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: move stop beyond start
    // -C---- >>> B-----
    //
    // Behind the scenes: -C---- >>> BC---- >>> B-----
    msa.setSubseqStop(1, 2);

    // Start position updated
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 2);    // new start
    QCOMPARE(spyArguments.at(2).toInt(), 3);    // old start

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toString(), QString("B"));

    // Stop position updated
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 2);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 3);    // old stop

    // Trimmed signal
    QCOMPARE(spySubseqTrimmed.count(), 1);
    spyArguments = spySubseqTrimmed.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toString(), QString("C"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

    // Sequence check
    QCOMPARE(subseq->bioString().sequence(), QString("B-----"));

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 1);    // Column
//    QCOMPARE(spyArguments.at(2).toString(), QString("B-")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("-C")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: move stop beyond left alignment terminus
    // B----- >>> A------
    //
    // Real order of operations:
    // B-----  >>> -B----- (insert one terminal gap column)
    // -B----- >>> AB----- (move the start - extend)
    // AB----- >>> A------ (move stop - trim)
    msa.setSubseqStop(1, 1);

    // Gap columns inserted
    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // column
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // count

    // Start position updated
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // new start
    QCOMPARE(spyArguments.at(2).toInt(), 2);    // old start

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toString(), QString("A"));

    // Stop position updated
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 2);    // old stop

    // Trimmed signal
    QCOMPARE(spySubseqTrimmed.count(), 1);
    spyArguments = spySubseqTrimmed.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toString(), QString("B"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

    // Sequence check
    QCOMPARE(subseq->bioString().sequence(), QString( "A------"));
    QCOMPARE(subseq2->bioString().sequence(), QString("----WX-"));

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 1);    // Column
//    QCOMPARE(spyArguments.at(2).toString(), QString("A-")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("-B")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: move stop forward (to the right)
    // ----WX- >> ----WXY
    msa.setSubseqStop(2, 6);

    // Stop position updated
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 6);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 5);    // old stop

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 7);
    QCOMPARE(spyArguments.at(2).toString(), QString("Y"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 7);
    QCOMPARE(spyArguments.at(1).toInt(), 7);

    // Sequence check
    QCOMPARE(subseq2->bioString().sequence(), QString("----WXY"));

    // No other signals
    QVERIFY(spySubseqStartChanged.isEmpty());
    QVERIFY(spyGapColumnsInserted.isEmpty());
    QVERIFY(spySubseqTrimmed.isEmpty());

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 2);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 7);    // Column
//    QCOMPARE(spyArguments.at(2).toString(), QString("Y")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("-")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: move stop beyond right alignment border
    // ----WXY >>> ----WXYZ
    //
    // Real order of operations:
    // ----WXY  >>> ----WXY-  (insert terminal gap column)
    // ----WXY- >>> ----WXYZ  (move the stop)
    msa.setSubseqStop(2, 7);

    // Check that gap column was inserted
    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 8);    // column
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // count

    // Stop position updated
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 7);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 6);    // old stop

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 8);
    QCOMPARE(spyArguments.at(2).toString(), QString("Z"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 8);
    QCOMPARE(spyArguments.at(1).toInt(), 8);

    // Sequence check
    QCOMPARE(subseq->bioString().sequence(), QString( "A-------"));
    QCOMPARE(subseq2->bioString().sequence(), QString("----WXYZ"));

    // No other signals should be present
    QVERIFY(spySubseqStartChanged.isEmpty());
    QVERIFY(spySubseqTrimmed.isEmpty());

//    QCOMPARE(spySubseqBorderChanged.count(), 1);
//    spyArguments = spySubseqBorderChanged.takeFirst();
//    QCOMPARE(spyArguments.at(0).toInt(), 2);    // Subseq index
//    QCOMPARE(spyArguments.at(1).toInt(), 8);    // Column
//    QCOMPARE(spyArguments.at(2).toString(), QString("Z")); // Final alignment
//    QCOMPARE(spyArguments.at(3).toString(), QString("-")); // Old alignment

    // ------------------------------------------------------------------------
    // Test: moving stop beyond start in operation that also includes gaps
    // C-D- >>> B----
    //
    // Real order of operations:
    // C-D-  >>> -C-D- (insert terminal gap)
    // -C-D- >>> BC-D- (extend start)
    // BC-D- >>> B---- (move stop)
    msa.clear();
    subseq = new Subseq(anonSeq);
    subseq2 = 0;

    QVERIFY(subseq->setBioString("C-D-"));
    QVERIFY(msa.append(subseq));

    msa.setSubseqStop(1, 2);

    // Check that gap column was inserted
    QCOMPARE(spyGapColumnsInserted.count(), 1);
    spyArguments = spyGapColumnsInserted.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // column
    QCOMPARE(spyArguments.at(1).toInt(), 1);    // count

    // Start position updated
    QCOMPARE(spySubseqStartChanged.count(), 1);
    spyArguments = spySubseqStartChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 2);    // new start
    QCOMPARE(spyArguments.at(2).toInt(), 3);    // old start

    // Extended signal
    QCOMPARE(spySubseqExtended.count(), 1);
    spyArguments = spySubseqExtended.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    QCOMPARE(spyArguments.at(2).toString(), QString("B"));

    // Stop position updated
    QCOMPARE(spySubseqStopChanged.count(), 1);
    spyArguments = spySubseqStopChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);    // Subseq index
    QCOMPARE(spyArguments.at(1).toInt(), 2);    // new stop
    QCOMPARE(spyArguments.at(2).toInt(), 4);    // old stop

    // Trimmed signal
    QCOMPARE(spySubseqTrimmed.count(), 1);
    spyArguments = spySubseqTrimmed.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toString(), QString("C-D"));

    // Extend or trim signal
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // Check final sequence
    QCOMPARE(subseq->bioString().sequence(), QString( "B----"));
}

void TestMsa::extendSubseqsLeft()
{
    AnonSeq anonSeq(1, "ABCDEF");
    AnonSeq anonSeq2(2, "TUVWXYZ");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);

    //                                3 45
    QVERIFY(subseq->setBioString( "---C-DE--"));
    //                                  4 5
    QVERIFY(subseq2->setBioString("-----W-X-"));

    Msa msa;
    msa.append(subseq);
    msa.append(subseq2);

    QSignalSpy spyExtendOrTrimFinished(&msa, SIGNAL(extendOrTrimFinished(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Tests: extend subseqs that do nothing
    for (int i=6; i<= 9; ++i)
    {
        msa.extendSubseqsLeft(1, 1, i);
        msa.extendSubseqsLeft(2, 2, i);
        msa.extendSubseqsLeft(1, 2, i);

        QCOMPARE(subseq->bioString().sequence(), QString( "---C-DE--"));
        QCOMPARE(subseq2->bioString().sequence(), QString("-----W-X-"));
        QCOMPARE(subseq->start(), 3);
        QCOMPARE(subseq2->start(), 4);
        QVERIFY(spyExtendOrTrimFinished.isEmpty());
    }

    // ------------------------------------------------------------------------
    // Test: extend second sequence one space
    // -----W-X- >>> ----VW-X-
    msa.extendSubseqsLeft(2, 2, 5);
    QCOMPARE(subseq2->bioString().sequence(), QString("----VW-X-"));
    QCOMPARE(subseq2->start(), 3);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    // ------------------------------------------------------------------------
    // Test: extend both sequences, but only one can extend
    // ---C-DE-- >>> ---C-DE--
    // ----VW-X- >>> ---UVW-X-
    msa.extendSubseqsLeft(1, 2, 4);
    QCOMPARE(subseq->bioString().sequence(), QString( "---C-DE--"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---UVW-X-"));
    QCOMPARE(subseq->start(), 3);
    QCOMPARE(subseq2->start(), 2);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // ------------------------------------------------------------------------
    // Test: extend both sequence as far as possible
    // ---C-DE-- >>> -ABC-DE--
    // ---UVW-X- >>> --TUVW-X-
    msa.extendSubseqsLeft(1, 2, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "-ABC-DE--"));
    QCOMPARE(subseq2->bioString().sequence(), QString("--TUVW-X-"));
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(subseq2->start(), 1);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 3);

    // ------------------------------------------------------------------------
    // Test: extend multiple spots at once
    msa.clear();
    subseq = new Subseq(anonSeq);
    subseq2 = new Subseq(anonSeq2);

    //                                3 45
    QVERIFY(subseq->setBioString( "---C-DE--"));
    //                                  4 5
    QVERIFY(subseq2->setBioString("-----W-X-"));

    msa.append(subseq);
    msa.append(subseq2);

    msa.extendSubseqsLeft(1, 2, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "-ABC-DE--"));
    QCOMPARE(subseq2->bioString().sequence(), QString("--TUVW-X-"));
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(subseq2->start(), 1);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    // ------------------------------------------------------------------------
    // Test: extension only occurs within available gap spaces
    msa.clear();
    subseq = new Subseq(anonSeq);
    subseq2 = new Subseq(anonSeq2);

    //                              3 45
    QVERIFY(subseq->setBioString( "-C-DE--"));
    //                               4 5
    QVERIFY(subseq2->setBioString("--W-X--"));

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    msa.extendSubseqsLeft(1, 2, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "BC-DE--"));
    QCOMPARE(subseq2->bioString().sequence(), QString("UVW-X--"));
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 2);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
}

void TestMsa::extendSubseqsRight()
{
    AnonSeq anonSeq(1,  "ABCDEF");
    AnonSeq anonSeq2(2, "TUVWXYZ");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D---"));
    //                              3 4
    QVERIFY(subseq2->setBioString("-V-W-----"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spyExtendOrTrimFinished(&msa, SIGNAL(extendOrTrimFinished(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Tests: extend subseqs that do nothing
    for (int i=1; i<= 4; ++i)
    {
        msa.extendSubseqsRight(1, 1, i);
        msa.extendSubseqsRight(2, 2, i);
        msa.extendSubseqsRight(1, 2, i);

        QCOMPARE(subseq->bioString().sequence(), QString( "--BC-D---"));
        QCOMPARE(subseq2->bioString().sequence(), QString("-V-W-----"));
        QCOMPARE(subseq->stop(), 4);
        QCOMPARE(subseq2->stop(), 4);
        QVERIFY(spyExtendOrTrimFinished.isEmpty());
    }

    // ------------------------------------------------------------------------
    // Test: extend second sequence one space
    // -V-W----- >>> -V-WX----
    msa.extendSubseqsRight(2, 2, 5);
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-WX----"));
    QCOMPARE(subseq2->stop(), 5);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    // ------------------------------------------------------------------------
    // Test: extend both sequences, but only one can extend
    // --BC-D--- >>> --BC-D---
    // -V-WX---- >>> -V-WXY---
    msa.extendSubseqsRight(1, 2, 6);
    QCOMPARE(subseq->bioString().sequence(), QString( "--BC-D---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-WXY---"));
    QCOMPARE(subseq->stop(), 4);
    QCOMPARE(subseq2->stop(), 6);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 6);
    QCOMPARE(spyArguments.at(1).toInt(), 6);

    // ------------------------------------------------------------------------
    // Test: extend both sequence as far as possible
    // --BC-D--- >>> --BC-DEF-
    // -V-WXY--- >>> -V-WXYZ--
    msa.extendSubseqsRight(1, 2, 9);
    QCOMPARE(subseq->bioString().sequence(), QString( "--BC-DEF-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-WXYZ--"));
    QCOMPARE(subseq->stop(), 6);
    QCOMPARE(subseq2->stop(), 7);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 7);
    QCOMPARE(spyArguments.at(1).toInt(), 8);

    // ------------------------------------------------------------------------
    // Test: extend multiple spots at once
    msa.clear();
    subseq = new Subseq(anonSeq);
    subseq2 = new Subseq(anonSeq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D---"));
    //                              3 4
    QVERIFY(subseq2->setBioString("-V-W-----"));

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    msa.extendSubseqsRight(1, 2, 9);
    QCOMPARE(subseq->bioString().sequence(), QString( "--BC-DEF-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-WXYZ--"));
    QCOMPARE(subseq->stop(), 6);
    QCOMPARE(subseq2->stop(), 7);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);
    QCOMPARE(spyArguments.at(1).toInt(), 8);

    // ------------------------------------------------------------------------
    // Test: extension only occurs within available gap spaces
    msa.clear();
    subseq = new Subseq(anonSeq);
    subseq2 = new Subseq(anonSeq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D-"));
    //                              3  4
    QVERIFY(subseq2->setBioString("-V--W--"));

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    msa.extendSubseqsRight(1, 2, 7);
    QCOMPARE(subseq->bioString().sequence(), QString( "--BC-DE"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V--WXY"));
    QCOMPARE(subseq->stop(), 5);
    QCOMPARE(subseq2->stop(), 6);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 6);
    QCOMPARE(spyArguments.at(1).toInt(), 7);
}

void TestMsa::trimSubseqsLeft()
{
    AnonSeq anonSeq(1,  "ABCDEF");
    AnonSeq anonSeq2(2, "TUVWXYZ");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D---"));
    //                              3 4
    QVERIFY(subseq2->setBioString("-V-W-----"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spyExtendOrTrimFinished(&msa, SIGNAL(extendOrTrimFinished(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: trim subseqs that do nothing
    msa.trimSubseqsLeft(1, 2, 1);
    msa.trimSubseqsLeft(1, 1, 1);
    msa.trimSubseqsLeft(2, 2, 1);
    msa.trimSubseqsLeft(1, 1, 2);

    QCOMPARE(subseq->bioString().sequence(), QString( "--BC-D---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-W-----"));
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 3);

    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from one sequence
    //  *
    // --BC-D---
    // -V-W-----
    msa.trimSubseqsLeft(1, 2, 2);
    QCOMPARE(subseq->bioString().sequence(), QString( "--BC-D---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---W-----"));
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 4);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from one sequence
    //   *
    // --BC-D---
    // ---W-----
    msa.trimSubseqsLeft(1, 2, 3);
    QCOMPARE(subseq->bioString().sequence(), QString( "---C-D---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---W-----"));
    QCOMPARE(subseq->start(), 3);
    QCOMPARE(subseq2->start(), 4);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 3);

    // ------------------------------------------------------------------------
    // Test: trim subseqs but prevent from removing last non-gap character from second sequence
    //    *
    // ---C-D---
    // ---W-----
    msa.trimSubseqsLeft(1, 2, 4);
    QCOMPARE(subseq->bioString().sequence(), QString( "-----D---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---W-----"));
    QCOMPARE(subseq->start(), 4);
    QCOMPARE(subseq2->start(), 4);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    msa.trimSubseqsLeft(1, 2, 6);
    QCOMPARE(subseq->bioString().sequence(), QString( "-----D---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---W-----"));
    QCOMPARE(subseq->start(), 4);
    QCOMPARE(subseq2->start(), 4);

    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    // ------------------------------------------------------------------------
    // Test: trim subseqs all the way, should leave one character
    msa.__setSubseqStart(1, 1);     // Use private version to avoid emitting the extendOrTrimFinished signal
    msa.__setSubseqStart(2, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "--ABCD---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("TUVW-----"));
    //         *
    // --ABCD---
    // TUVW-----
    msa.trimSubseqsLeft(1, 2, 9);
    QCOMPARE(subseq->bioString().sequence(), QString( "-----D---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---W-----"));
    QCOMPARE(subseq->start(), 4);
    QCOMPARE(subseq2->start(), 4);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 1);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    // ------------------------------------------------------------------------
    // Test: specific extendOrTrim signal test
    //      *
    // --A-B-C-
    msa.clear();
    subseq2 = 0;
    subseq = new Subseq(anonSeq);
    //                                 *
    QVERIFY(subseq->setBioString("--A-B-C-"));
    msa.append(subseq);

    msa.trimSubseqsLeft(1, 1, 6);
    QCOMPARE(subseq->start(), 3);
    QCOMPARE(subseq->bioString().sequence(), QString("------C-"));

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 5);        // <-- This is the kicker
}

void TestMsa::trimSubseqsRight()
{
    AnonSeq anonSeq(1,  "ABCDEF");
    AnonSeq anonSeq2(2, "TUVWXYZ");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D-"));
    //                              3 45
    QVERIFY(subseq2->setBioString("-V-WX--"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spyExtendOrTrimFinished(&msa, SIGNAL(extendOrTrimFinished(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: trim subseqs that do nothing
    msa.trimSubseqsRight(1, 2, 7);
    msa.trimSubseqsRight(1, 1, 7);
    msa.trimSubseqsRight(2, 2, 7);
    msa.trimSubseqsRight(2, 2, 6);

    QCOMPARE(subseq->bioString().sequence(), QString( "--BC-D-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-WX--"));
    QCOMPARE(subseq->stop(), 4);
    QCOMPARE(subseq2->stop(), 5);

    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from one sequence
    //      *
    // --BC-D-
    // -V-WX--
    msa.trimSubseqsRight(1, 2, 6);
    QCOMPARE(subseq->bioString().sequence(), QString( "--BC---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-WX--"));
    QCOMPARE(subseq->stop(), 3);
    QCOMPARE(subseq2->stop(), 5);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 6);
    QCOMPARE(spyArguments.at(1).toInt(), 6);

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from other sequence
    //     *
    // --BC---
    // -V-WX--
    msa.trimSubseqsRight(1, 2, 5);
    QCOMPARE(subseq->bioString().sequence(), QString( "--BC---"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-W---"));
    QCOMPARE(subseq->stop(), 3);
    QCOMPARE(subseq2->stop(), 4);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from both sequences
    //    *
    // --BC---
    // -V-W---
    msa.trimSubseqsRight(1, 2, 4);
    QCOMPARE(subseq->bioString().sequence(), QString( "--B----"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-----"));
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // ------------------------------------------------------------------------
    // Test: trim subseqs but prevent from removing last non-gap characters
    //   *
    // --B----
    // -V-----
    msa.trimSubseqsRight(1, 2, 3);
    QCOMPARE(subseq->bioString().sequence(), QString( "--B----"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-----"));
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    msa.trimSubseqsRight(1, 2, 2);
    QCOMPARE(subseq->bioString().sequence(), QString( "--B----"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-----"));
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    msa.trimSubseqsRight(1, 2, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "--B----"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-----"));
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    // ------------------------------------------------------------------------
    // Test: trim subseqs all the way, should leave one character
    msa.__setSubseqStop(1, 6);      // Use private version to avoid emitting the extendOrTrimFinished signal
    msa.__setSubseqStop(2, 7);
    QCOMPARE(subseq->bioString().sequence(), QString( "--BCDEF"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-VWXYZ-"));
    // *
    // --BCDEF
    // -VWXYZ-
    msa.trimSubseqsRight(1, 2, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "--B----"));
    QCOMPARE(subseq2->bioString().sequence(), QString("-V-----"));
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 7);

    // ------------------------------------------------------------------------
    // Test: specific extendOrTrim signal test
    //    *
    // --A-B-C-
    msa.clear();
    subseq2 = 0;
    subseq = new Subseq(anonSeq);
    //                               *
    QVERIFY(subseq->setBioString("--A-B-C-"));
    msa.append(subseq);

    msa.trimSubseqsRight(1, 1, 4);
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(subseq->bioString().sequence(), QString("--A-----"));

    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);        // <-- This is the kicker
    QCOMPARE(spyArguments.at(1).toInt(), 7);
}

void TestMsa::levelSubseqsLeft()
{
    AnonSeq anonSeq(1,  "ABCDEF");
    AnonSeq anonSeq2(2, "TUVWXYZ");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);

    //                                 23 4
    QVERIFY(subseq->setBioString( "----BC-D-"));
    //                                3 45
    QVERIFY(subseq2->setBioString("---V-WX--"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spyExtendOrTrimFinished(&msa, SIGNAL(extendOrTrimFinished(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: level operations that do nothing
    msa.levelSubseqsLeft(1, 1, 5);
    msa.levelSubseqsLeft(2, 2, 4);
    QCOMPARE(subseq->bioString().sequence(), QString( "----BC-D-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---V-WX--"));
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 3);

    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    // ------------------------------------------------------------------------
    // Test: extend and trim a subseq
    // ----BC-D- >>> ---ABC-D-
    msa.levelSubseqsLeft(1, 1, 4);
    QCOMPARE(subseq->bioString().sequence(), QString( "---ABC-D-"));
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    msa.levelSubseqsLeft(1, 1, 5);
    QCOMPARE(subseq->bioString().sequence(), QString( "----BC-D-"));
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    msa.levelSubseqsLeft(1, 1, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "---ABC-D-"));
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    msa.levelSubseqsLeft(1, 1, 6);
    QCOMPARE(subseq->bioString().sequence(), QString( "-----C-D-"));
    QCOMPARE(subseq->start(), 3);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    msa.levelSubseqsLeft(1, 1, 2);
    QCOMPARE(subseq->bioString().sequence(), QString( "---ABC-D-"));
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    // ------------------------------------------------------------------------
    // Test: shifting by one character
    // ---V-WX-- >>> ----VWX--
    msa.levelSubseqsLeft(2, 2, 5);
    QCOMPARE(subseq2->bioString().sequence(), QString("----VWX--"));
    QCOMPARE(subseq2->start(), 3);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 5);

    // ------------------------------------------------------------------------
    // Test: multiple sequence level
    // ---ABC-D- >>> ---ABC-D-
    // ----VWX-- >>> --TUVWX--
    msa.levelSubseqsLeft(1, 2, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "---ABC-D-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("--TUVWX--"));
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(subseq2->start(), 1);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // ---ABC-D- >>> ----BC-D-
    // --TUVWX-- >>> ----VWX--
    msa.levelSubseqsLeft(1, 2, 5);
    QCOMPARE(subseq->bioString().sequence(), QString( "----BC-D-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("----VWX--"));
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 3);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 3);
    QCOMPARE(spyArguments.at(1).toInt(), 4);

    // ------------------------------------------------------------------------
    // Test: multiple sequence, level to end
    msa.levelSubseqsLeft(1, 2, 9);
    QCOMPARE(subseq->bioString().sequence(), QString( "-------D-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("------X--"));
    QCOMPARE(subseq->start(), 4);
    QCOMPARE(subseq2->start(), 5);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);
    QCOMPARE(spyArguments.at(1).toInt(), 6);
}

void TestMsa::levelSubseqsRight()
{
    AnonSeq anonSeq(1,  "ABCDEF");
    AnonSeq anonSeq2(2, "TUVWXYZ");

    Subseq *subseq = new Subseq(anonSeq);
    Subseq *subseq2 = new Subseq(anonSeq2);

    //                                 23 4
    QVERIFY(subseq->setBioString( "----BC-D-"));
    //                                3 45
    QVERIFY(subseq2->setBioString("---V-WX--"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QSignalSpy spyExtendOrTrimFinished(&msa, SIGNAL(extendOrTrimFinished(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: level operations that do nothing
    msa.levelSubseqsRight(1, 1, 8);
    msa.levelSubseqsRight(2, 2, 7);
    QCOMPARE(subseq->bioString().sequence(), QString( "----BC-D-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---V-WX--"));
    QCOMPARE(subseq->stop(), 4);
    QCOMPARE(subseq2->stop(), 5);

    QVERIFY(spyExtendOrTrimFinished.isEmpty());

    // ------------------------------------------------------------------------
    // Test: extend and trim a subseq
    // ---V-WX-- >>> ---V-WXY-
    msa.levelSubseqsRight(2, 2, 8);
    QCOMPARE(subseq2->bioString().sequence(), QString( "---V-WXY-"));
    QCOMPARE(subseq2->stop(), 6);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 8);
    QCOMPARE(spyArguments.at(1).toInt(), 8);

    msa.levelSubseqsRight(2, 2, 9);
    QCOMPARE(subseq2->bioString().sequence(), QString( "---V-WXYZ"));
    QCOMPARE(subseq2->stop(), 7);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 9);
    QCOMPARE(spyArguments.at(1).toInt(), 9);

    msa.levelSubseqsRight(2, 2, 7);
    QCOMPARE(subseq2->bioString().sequence(), QString( "---V-WX--"));
    QCOMPARE(subseq2->stop(), 5);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 8);
    QCOMPARE(spyArguments.at(1).toInt(), 9);

    msa.levelSubseqsRight(2, 2, 9);
    QCOMPARE(subseq2->bioString().sequence(), QString( "---V-WXYZ"));
    QCOMPARE(subseq2->stop(), 7);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 8);
    QCOMPARE(spyArguments.at(1).toInt(), 9);

    msa.levelSubseqsRight(2, 2, 7);
    QCOMPARE(subseq2->bioString().sequence(), QString( "---V-WX--"));
    QCOMPARE(subseq2->stop(), 5);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 8);
    QCOMPARE(spyArguments.at(1).toInt(), 9);

    // ------------------------------------------------------------------------
    // Test: shifting by one character
    // ----BC-D- >>> ----BCD--
    msa.levelSubseqsRight(1, 1, 7);
    QCOMPARE(subseq->bioString().sequence(), QString("----BCD--"));
    QCOMPARE(subseq->stop(), 4);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 7);
    QCOMPARE(spyArguments.at(1).toInt(), 8);

    // ------------------------------------------------------------------------
    // Test: multiple sequence level
    // ----BCD-- >>> ----BCDE-
    // ---V-WX-- >>> ---V-WXY-
    msa.levelSubseqsRight(1, 2, 8);
    QCOMPARE(subseq->bioString().sequence(), QString( "----BCDE-"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---V-WXY-"));
    QCOMPARE(subseq->stop(), 5);
    QCOMPARE(subseq2->stop(), 6);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 8);
    QCOMPARE(spyArguments.at(1).toInt(), 8);

    // ----BCDE- >>> ----B----
    // ---V-WXY- >>> ---VW----
    msa.levelSubseqsRight(1, 2, 5);
    QCOMPARE(subseq->bioString().sequence(), QString( "----B----"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---VW----"));
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 4);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);
    QCOMPARE(spyArguments.at(1).toInt(), 8);

    // ------------------------------------------------------------------------
    // Test: multiple sequence, level to end
    msa.levelSubseqsRight(1, 2, 1);
    QCOMPARE(subseq->bioString().sequence(), QString( "----B----"));
    QCOMPARE(subseq2->bioString().sequence(), QString("---V-----"));
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);
    QCOMPARE(spyExtendOrTrimFinished.count(), 1);
    spyArguments = spyExtendOrTrimFinished.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 5);
    QCOMPARE(spyArguments.at(1).toInt(), 5);
}




QTEST_MAIN(TestMsa)
#include "TestMsa.moc"
