/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../Msa.h"
#include "../enums.h"
#include "../global.h"

#include "../Entities/AminoSeq.h"
// #include "../Entities/Astring.h"

class TestMsa : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorBasic();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void at();
    void append_and_count();
    void append_grammar();      // Ensure that only sequences with the same grammar may be appended
    void canCollapseLeft();
    void canCollapseRight();
    void canExtendLeft();
    void canExtendRight();
    void canLevelLeft();
    void canLevelRight();
    void canTrimLeft();
    void canTrimRight();
    void clear();
    void collapseLeft();
    void collapseRight();
    void columnCount();
    void extendLeftColumnRows();
    void extendLeftRowNumChars();
    void extendLeftColumnRowBioString();
    void extendRight();
    void extendRightRowNumChars();
    void extendRightColumnRowBioString();
    void grammar();
    void indexOfAbstractSeq();
    void insert();
    void insertGapColumns();
    void insertRows();
    void isCompatibleSubseq();
    void isCompatibleSubseqVector();
    void isEmpty();
    void isValidColumn();
    void isValidRow();
    void isValidRowRange();
    void leftExtendableLength();
    void leftTrimmableLength();
    void length();
    void levelLeft();
    void levelRight();
    void members();
    void moveRow();
    void moveRowRange();
    void moveRowRangeRelative();
    void moveRowRelative();
    void prepend();
    void removeAt();
    void removeFirst();
    void removeRows();
    void removeGapColumns();
    void removeGapColumnsInRange();
    void removeLast();
    void rightExtendableLength();
    void rightTrimmableLength();
    void setSubseqStart();
    void setSubseqStop();

    // Slide region may be called with any rectangular region defined by two coordinate points which are
    // specified in any order. Thus, we test each of these and indicate a corner using the following
    // notation: l = left, t = top, r = right, b = bottom
    // o left, top -> right, bottom
    // o right, bottom -> left, top
    // o left, bottom -> right, top
    // o right, top -> left, bottom
    void slideRect_data();
    void slideRect();
    void slideRectSpecific();
    void sortAscending();
    void sortDescending();
    void swap();
    void takeRows();
    void trimLeft();
    void trimLeftRowNumChars();
    void trimRight();
    void trimRightRowNumChars();
    void undo();

private:
    // Utility function for adding test rows for the slideRect test
    void addSlideRectTestRows(const QString &direction,       // Should be QVector's but do not care to change right now
                                QList<int> leftList,
                                QList<int> topList,
                                QList<int> rightList,
                                QList<int> bottomList);
};



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private helper functions
void TestMsa::addSlideRectTestRows(const QString &direction,
                                     QList<int> leftList,
                                     QList<int> topList,
                                     QList<int> rightList,
                                     QList<int> bottomList)
{
    QTest::newRow(QString("%1 - sliding unmovable region :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << 0
            << "-A-B-C--" << "-A-B-C--"
            << "-D-E-F--" << "-D-E-F--"
            << "-G-H-I--" << "-G-H-I--";

    QTest::newRow(QString("%1 - sliding unmovable region :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 0
            << "-A-B-C--" << "-A-B-C--"
            << "-D-E-F--" << "-D-E-F--"
            << "-G-H-I--" << "-G-H-I--";

    QTest::newRow(QString("%1 - sliding internal non-gap region :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 1
            << "-A-B-C--" << "-A--BC--"
            << "-D-E-F--" << "-D--EF--"
            << "-G-H-I--" << "-G-H-I--";

    QTest::newRow(QString("%1 - sliding internal non-gap region :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << -1
            << "-A--BC--" << "-A--BC--"
            << "-D--EF--" << "-D-E-F--"
            << "-G-H-I--" << "-GH--I--";

    QTest::newRow(QString("%1 - attempt to move more than allowable amount :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 4 << 1
            << "-A--BC--" << "-A--BC--"
            << "-D-E-F--" << "-D--EF--"
            << "-GH--I--" << "-G-H-I--";

    QTest::newRow(QString("%1 - attempt to move more than allowable amount :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -5 << -2
            << "-A--BC--" << "-AB--C--"
            << "-D--EF--" << "-DE--F--"
            << "-G-H-I--" << "-G-H-I--";

    QTest::newRow(QString("%1 - slide single character :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << -1
            << "-AB--C--" << "-AB--C--"
            << "-DE--F--" << "-DE--F--"
            << "-G-H-I--" << "-GH--I--";

    QTest::newRow(QString("%1 - slide characters to rightmost position").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 2 << 2
            << "-AB--C--" << "-AB----C"
            << "-DE--F--" << "-DE----F"
            << "-GH--I--" << "-GH--I--";

    QTest::newRow(QString("%1 - attempt slide beyond right edge").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 0
            << "-AB----C" << "-AB----C"
            << "-DE----F" << "-DE----F"
            << "-GH--I--" << "-GH--I--";

    QTest::newRow(QString("%1 - attempt slide beyond right edge when not adjacent to right edge").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 4 << 2
            << "-AB----C" << "-AB----C"
            << "-DE----F" << "-DE----F"
            << "-GH--I--" << "-GH----I";

    QTest::newRow(QString("%1 - slide characters to leftmost position").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << -1
            << "-AB----C" << "-AB----C"
            << "-DE----F" << "D-E----F"
            << "-GH----I" << "G-H----I";

    QTest::newRow(QString("%1 - attempt to move beyond left edge").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << 0
            << "-AB----C" << "-AB----C"
            << "D-E----F" << "D-E----F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - attempt slide beyond left edge when not adjacent to left edge").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -4 << -1
            << "-AB----C" << "A-B----C"
            << "D-E----F" << "D-E----F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - slide region with non-gap characters, but rightmost column in rectangular region is purely gap characters :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 5 << 2
            << "A-B----C" << "A---B--C"
            << "D-E----F" << "D---E--F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - attempt unallowable slide with rectangular region :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 5 << 0
            << "A---B--C" << "A---B--C"
            << "D---E--F" << "D---E--F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - slide region with non-gap characters, but leftmost column in rectangular region is purely gap characters :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -5 << -1
            << "A---B--C" << "A--B---C"
            << "D---E--F" << "D--E---F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - attempt unallowable slide with rectangular region :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -5 << 0
            << "A--B---C" << "A--B---C"
            << "D--E---F" << "D--E---F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - sliding internal gap region :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << -1
            << "A--B---C" << "-A-B---C"
            << "D--E---F" << "-D-E---F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - sliding internal gap region beyond edge should fail :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -1 << 0
            << "-A-B---C" << "-A-B---C"
            << "-D-E---F" << "-D-E---F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - sliding internal gap region beyond edge (partially allowable) :: negative direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << -6 << -2
            << "-A-B---C" << "--AB---C"
            << "-D-E---F" << "--DE---F"
            << "G-H----I" << "G-H----I";

    QTest::newRow(QString("%1 - sliding internal gap region :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 1
            << "--AB---C" << "--AB---C"
            << "--DE---F" << "--DE-F--"
            << "G-H----I" << "G-H--I--";

    QTest::newRow(QString("%1 - sliding internal gap region beyond edge :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 1 << 0
            << "--AB---C" << "--AB---C"
            << "--DE-F--" << "--DE-F--"
            << "G-H--I--" << "G-H--I--";

    QTest::newRow(QString("%1 - sliding internal gap region beyond edge (partially allowable) :: positive direction").arg(direction).toAscii()) << leftList.takeFirst() << topList.takeFirst() << rightList.takeFirst() << bottomList.takeFirst() << 8 << 3
            << "--AB---C" << "--AB--C-"
            << "--DE-F--" << "--DEF---"
            << "G-H--I--" << "G-H--I--";
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
void TestMsa::constructorBasic()
{
    Msa msa;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestMsa::at()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    subseq->setBioString("BC--DE");

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    subseq2->setBioString("GH--JK");

    Seq seq3("CDEF");
    Subseq *subseq3 = new Subseq(seq3);
    subseq3->setBioString("--CDEF");

    Msa msa;

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));
    QVERIFY(msa.subseqCount() == 3);

    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
}

void TestMsa::append_and_count()
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

    Msa msa;
    QVERIFY(msa.length() == 0);
    QCOMPARE(msa.rowCount(), 0);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.subseqCount() == 1);
    QCOMPARE(msa.rowCount(), 1);

    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.subseqCount() == 2);
    QCOMPARE(msa.rowCount(), 2);

    QCOMPARE(msa.append(subseq3), false);
    QVERIFY(msa.subseqCount() == 2);
    QCOMPARE(msa.rowCount(), 2);

    subseq3->insertGaps(5, 2, constants::kDefaultGapCharacter);
    QVERIFY(msa.append(subseq3));
    QCOMPARE(msa.subseqCount(), 3);
    QCOMPARE(msa.rowCount(), 3);

    QVERIFY(msa[1] == subseq1);
    QVERIFY(msa[2] == subseq2);
    QVERIFY(msa[3] == subseq3);
}

void TestMsa::append_grammar()
{
    Msa msa(eDnaGrammar);

    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);

    Seq seq2("ACTG");
    Subseq *subseq2 = new Subseq(seq2);

    QCOMPARE(msa.append(subseq1), true);
    msa.clear();

    QCOMPARE(msa.append(subseq2), false);
    delete subseq2;
    subseq2 = nullptr;
}

void TestMsa::canCollapseLeft()
{
    Msa msa(eDnaGrammar);
    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    // Test: null PosiRect
    QCOMPARE(msa.canCollapseLeft(PosiRect()), false);

    // Test: all others should be valid
    QCOMPARE(msa.canCollapseLeft(PosiRect(QPoint(1, 1), QPoint(4, 2))), false);

    QVERIFY(subseq1->setBioString("A-C-T-G"));
    QVERIFY(subseq2->setBioString("AC---TG"));

    QCOMPARE(msa.canCollapseLeft(PosiRect(1, 1, 1, 1)), false);
    QCOMPARE(msa.canCollapseLeft(PosiRect(1, 1, 2, 1)), false);
    QCOMPARE(msa.canCollapseLeft(PosiRect(1, 1, 3, 1)), true);
    QCOMPARE(msa.canCollapseLeft(PosiRect(2, 1, 2, 1)), true);
    QCOMPARE(msa.canCollapseLeft(PosiRect(2, 1, 3, 1)), true);
    QCOMPARE(msa.canCollapseLeft(PosiRect(1, 2, 5, 1)), false);
}

void TestMsa::canCollapseRight()
{
    Msa msa(eDnaGrammar);
    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    // Test: null PosiRect
    QCOMPARE(msa.canCollapseRight(PosiRect()), false);

    // Test: all others should be valid
    QCOMPARE(msa.canCollapseRight(PosiRect(QPoint(1, 1), QPoint(4, 2))), false);

    QVERIFY(subseq1->setBioString("A-C-T-G"));
    QVERIFY(subseq2->setBioString("AC---TG"));

    QCOMPARE(msa.canCollapseRight(PosiRect(1, 1, 1, 1)), false);
    QCOMPARE(msa.canCollapseRight(PosiRect(1, 1, 2, 1)), true);
    QCOMPARE(msa.canCollapseRight(PosiRect(1, 1, 3, 1)), true);
    QCOMPARE(msa.canCollapseRight(PosiRect(2, 1, 2, 1)), false);
    QCOMPARE(msa.canCollapseRight(PosiRect(2, 1, 3, 1)), true);
    QCOMPARE(msa.canCollapseRight(PosiRect(1, 2, 5, 1)), true);
    QCOMPARE(msa.canCollapseRight(PosiRect(6, 1, 2, 2)), false);

    // Test: just gaps
    QCOMPARE(msa.canCollapseRight(PosiRect(4, 1, 1, 2)), false);
}

void TestMsa::canExtendLeft()
{
    Msa msa(eDnaGrammar);
    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    // Test: msaColumn of 0 should return false
    QCOMPARE(msa.canExtendLeft(0, ClosedIntRange()), false);

    for (int i=1; i<= 4; ++i)
    {
        QCOMPARE(msa.canExtendLeft(i, ClosedIntRange(1, 1)), false);
        QCOMPARE(msa.canExtendLeft(i, ClosedIntRange(1, 2)), false);
        QCOMPARE(msa.canExtendLeft(i, ClosedIntRange(2, 2)), false);
    }

    QVERIFY(subseq1->setBioString("-ACT"));
    QVERIFY(subseq2->setBioString("-CTG"));

    QCOMPARE(msa.canExtendLeft(1, ClosedIntRange(1, 1)), false);
    QCOMPARE(msa.canExtendLeft(1, ClosedIntRange(1, 2)), true);
    QCOMPARE(msa.canExtendLeft(1, ClosedIntRange(2, 2)), true);
}

void TestMsa::canExtendRight()
{
    Msa msa(eDnaGrammar);
    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    // Test: msaColumn of 0 should return false
    QCOMPARE(msa.canExtendRight(0, ClosedIntRange(1, 901)), false);

    for (int i=1; i<= 4; ++i)
    {
        QCOMPARE(msa.canExtendRight(i, ClosedIntRange(1, 1)), false);
        QCOMPARE(msa.canExtendRight(i, ClosedIntRange(1, 2)), false);
        QCOMPARE(msa.canExtendRight(i, ClosedIntRange(2, 2)), false);
    }

    QVERIFY(subseq1->setBioString("CTG-"));
    QVERIFY(subseq2->setBioString("ACT-"));

    QCOMPARE(msa.canExtendRight(4, ClosedIntRange(1, 1)), false);
    QCOMPARE(msa.canExtendRight(4, ClosedIntRange(1, 2)), true);
    QCOMPARE(msa.canExtendRight(4, ClosedIntRange(2, 2)), true);
}

void TestMsa::canLevelLeft()
{
    Msa msa(eDnaGrammar);
    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    // Test: msaColumn of 0 should return false
    QCOMPARE(msa.canLevelLeft(0, ClosedIntRange()), false);

    QCOMPARE(msa.canLevelLeft(1, ClosedIntRange(1, 1)), false);
    QCOMPARE(msa.canLevelLeft(1, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canLevelLeft(1, ClosedIntRange(2, 2)), false);
    for (int i=2; i<= 4; ++i)
    {
        QCOMPARE(msa.canLevelLeft(i, ClosedIntRange(1, 1)), true);
        QCOMPARE(msa.canLevelLeft(i, ClosedIntRange(1, 2)), true);
        QCOMPARE(msa.canLevelLeft(i, ClosedIntRange(2, 2)), true);
    }

    QVERIFY(subseq1->setBioString("--ACTG"));
    QVERIFY(subseq2->setBioString("---ACT"));

    QCOMPARE(msa.canLevelLeft(1, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canLevelLeft(2, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canLevelLeft(3, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canLevelLeft(4, ClosedIntRange(1, 2)), true);
}

void TestMsa::canLevelRight()
{
    Msa msa(eDnaGrammar);
    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    // Test: msaColumn of 0 should return false
    QCOMPARE(msa.canLevelRight(0, ClosedIntRange()), false);

    QCOMPARE(msa.canLevelRight(4, ClosedIntRange(1, 1)), false);
    QCOMPARE(msa.canLevelRight(4, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canLevelRight(4, ClosedIntRange(2, 2)), false);
    for (int i=1; i<= 3; ++i)
    {
        QCOMPARE(msa.canLevelRight(i, ClosedIntRange(1, 1)), true);
        QCOMPARE(msa.canLevelRight(i, ClosedIntRange(1, 2)), true);
        QCOMPARE(msa.canLevelRight(i, ClosedIntRange(2, 2)), true);
    }

    QVERIFY(subseq1->setBioString("ACTG--"));
    QVERIFY(subseq2->setBioString("CTG---"));

    QCOMPARE(msa.canLevelRight(6, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canLevelRight(5, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canLevelRight(4, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canLevelRight(3, ClosedIntRange(1, 2)), true);
}

void TestMsa::canTrimLeft()
{
    Msa msa(eDnaGrammar);
    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    // Test: msaColumn of 0 should return false
    QCOMPARE(msa.canTrimLeft(0, ClosedIntRange()), false);

    for (int i=1; i<= 4; ++i)
    {
        QCOMPARE(msa.canTrimLeft(i, ClosedIntRange(1, 1)), true);
        QCOMPARE(msa.canTrimLeft(i, ClosedIntRange(1, 2)), true);
        QCOMPARE(msa.canTrimLeft(i, ClosedIntRange(2, 2)), true);
    }

    QVERIFY(subseq1->setBioString("-AC"));
    QVERIFY(subseq2->setBioString("-G-"));

    QCOMPARE(msa.canTrimLeft(1, ClosedIntRange(1, 1)), false);
    QCOMPARE(msa.canTrimLeft(1, ClosedIntRange(1, 2)), false);
    QCOMPARE(msa.canTrimLeft(1, ClosedIntRange(2, 2)), false);

    QCOMPARE(msa.canTrimLeft(2, ClosedIntRange(1, 1)), true);
    QCOMPARE(msa.canTrimLeft(2, ClosedIntRange(1, 2)), true);
    QCOMPARE(msa.canTrimLeft(2, ClosedIntRange(2, 2)), false);
}

void TestMsa::canTrimRight()
{
    Msa msa(eDnaGrammar);
    Seq seq1("ACTG", eDnaGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq1);

    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    // Test: msaColumn of 0 should return false
    QCOMPARE(msa.canTrimRight(0, ClosedIntRange()), false);

    for (int i=1; i<= 4; ++i)
    {
        QCOMPARE(msa.canTrimRight(i, ClosedIntRange(1, 1)), true);
        QCOMPARE(msa.canTrimRight(i, ClosedIntRange(1, 2)), true);
        QCOMPARE(msa.canTrimRight(i, ClosedIntRange(2, 2)), true);
    }

    QVERIFY(subseq1->setBioString("-AC"));
    QVERIFY(subseq2->setBioString("-G-"));

    QCOMPARE(msa.canTrimRight(3, ClosedIntRange(1, 1)), true);
    QCOMPARE(msa.canTrimRight(3, ClosedIntRange(1, 2)), true);
    QCOMPARE(msa.canTrimRight(3, ClosedIntRange(2, 2)), false);

    QCOMPARE(msa.canTrimRight(2, ClosedIntRange(1, 1)), true);
    QCOMPARE(msa.canTrimRight(2, ClosedIntRange(1, 2)), true);
    QCOMPARE(msa.canTrimRight(2, ClosedIntRange(2, 2)), false);
}

void TestMsa::clear()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    subseq->setBioString("BC--DE");

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    subseq2->setBioString("GH--JK");

    Seq seq3("CDEF");
    Subseq *subseq3 = new Subseq(seq3);
    subseq3->setBioString("--CDEF");

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));
    QVERIFY(msa.subseqCount() == 3);
    msa.clear();
    QVERIFY(msa.subseqCount() == 0);
}

void TestMsa::collapseLeft()
{
    Seq seq("ABCDEF");
    Subseq *subseq1 = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);
    Subseq *subseq3 = new Subseq(seq);
    Subseq *subseq4 = new Subseq(seq);

    Msa msa;
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QVector<QByteArray> seqs;
    //       12345678
    seqs << "A-BC-D-E"
         << "-A-BCD--"
         << "--A---BC";

    // Also tests that un-normalized rects produce expected result

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
                    for (int i=1; i<=nSeqs; ++i)
                    {
                        if (i >= top && i <= bottom)
                        {
                            subseq4->setBioString(seqs.at(i-1));
                            subseq4->collapseLeft(ClosedIntRange(left, right));
                            QCOMPARE(*msa.at(i), *subseq4);
                        }
                        else
                        {
                            QVERIFY(*msa.at(i) == seqs.at(i-1));
                        }
                    }
                }
            }
        }
    }

    delete subseq4;
    subseq4 = 0;

    // ----------------------------------------
    // Reset to original alignment
    QVERIFY(subseq1->setBioString(seqs.at(0)));
    QVERIFY(subseq2->setBioString(seqs.at(1)));
    QVERIFY(subseq3->setBioString(seqs.at(2)));

    // Test: check for appropriate subseq pod change vectors
    SubseqChangePodVector pods = msa.collapseLeft(PosiRect(QPoint(4, 1), QPoint(5, 2)));
    QVERIFY(pods.isEmpty());

    // A-BC-D-E        A-BC-D-E
    // -A-BCD--   ==>  -AB-CD--
    // --A---BC        --A---BC
    pods = msa.collapseLeft(PosiRect(QPoint(3, 1), QPoint(4, 3)));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eInternal);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(3, 4));
    QVERIFY(pods.at(0).difference_ == "-B");
    QVERIFY(msa.collapseLeft(PosiRect(QPoint(3, 1), QPoint(4, 3))).isEmpty());

    // 12345678
    // A-BC-D-E        A-BCD--E
    // -AB-CD--   ==>  -ABCD---
    // --A---BC        --AB---C
    //   |<==+
    pods = msa.collapseLeft(PosiRect(QPoint(3, 1), QPoint(7, 3)));
    QCOMPARE(pods.size(), 3);
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eInternal);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(5, 6));
    QVERIFY(pods.at(0).difference_ == "-D");
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eInternal);
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(4, 6));
    QVERIFY(pods.at(1).difference_ == "-CD");
    QCOMPARE(pods.at(2).operation_, SubseqChangePod::eInternal);
    QCOMPARE(pods.at(2).row_, 3);
    QCOMPARE(pods.at(2).columns_, ClosedIntRange(4, 7));
    QVERIFY(pods.at(2).difference_ == "---B");
}

void TestMsa::collapseRight()
{
    Seq seq("ABCDEF");
    Subseq *subseq1 = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq);
    Subseq *subseq3 = new Subseq(seq);

    Subseq *subseq4 = new Subseq(seq);

    Msa msa;
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

                    for (int i=1; i<=nSeqs; ++i)
                    {
                        if (i >= top && i <= bottom)
                        {
                            subseq4->setBioString(seqs.at(i-1));
                            subseq4->collapseRight(ClosedIntRange(left, right));
                            QCOMPARE(*msa.at(i), *subseq4);
                        }
                        else
                        {
                            QVERIFY(*msa.at(i) == seqs.at(i-1));
                        }
                    }
                }
            }
        }
    }

    delete subseq4;
    subseq4 = 0;

    // ----------------------------------------
    // Reset to original alignment
    QVERIFY(subseq1->setBioString(seqs.at(0)));
    QVERIFY(subseq2->setBioString(seqs.at(1)));
    QVERIFY(subseq3->setBioString(seqs.at(2)));

    // Test: check for appropriate subseq pod change vectors
    SubseqChangePodVector pods = msa.collapseRight(PosiRect(QPoint(7, 1), QPoint(8, 2)));
    QVERIFY(pods.isEmpty());

    // A-BC-D-E        -ABC-D-E
    // -A-BCD--   ==>  --ABCD--
    // --A---BC        --A---BC
    pods = msa.collapseRight(PosiRect(QPoint(1, 1), QPoint(3, 3)));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eInternal);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(1, 2));
    QVERIFY(pods.at(0).difference_ == "A-");
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eInternal);
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(2, 3));
    QVERIFY(pods.at(1).difference_ == "A-");
    QVERIFY(msa.collapseRight(PosiRect(QPoint(1, 1), QPoint(3, 3))).isEmpty());

    // 12345678
    // -ABC-D-E        -A-BCD-E
    // --ABCD--   ==>  --ABCD--
    // --A---BC        -----ABC
    //   +=>|
    pods = msa.collapseRight(PosiRect(QPoint(3, 1), QPoint(6, 3)));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eInternal);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(3, 5));
    QVERIFY(pods.at(0).difference_ == "BC-");
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eInternal);
    QCOMPARE(pods.at(1).row_, 3);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(3, 6));
    QVERIFY(pods.at(1).difference_ == "A---");
}

void TestMsa::columnCount()
{
    Msa msa;

    QCOMPARE(msa.length(), 0);

    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));

    QVERIFY(msa.append(subseq1));
    QCOMPARE(msa.columnCount(), 6);
}

void TestMsa::extendLeftColumnRows()
{
    Seq seq1("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq2);

    //                                3 45
    QVERIFY(subseq->setBioString( "---C-DE--"));
    //                                  4 5
    QVERIFY(subseq2->setBioString("-----W-X-"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    // ------------------------------------------------------------------------
    // Tests: extend subseqs that do nothing
    for (int i=6; i<= 9; ++i)
    {
        QVERIFY(msa.extendLeft(i, ClosedIntRange(1, 1)).isEmpty());
        QVERIFY(msa.extendLeft(i, ClosedIntRange(2, 2)).isEmpty());
        QVERIFY(msa.extendLeft(i, ClosedIntRange(1, 2)).isEmpty());

        QVERIFY(*subseq == "---C-DE--");
        QVERIFY(*subseq2 == "-----W-X-");
        QCOMPARE(subseq->start(), 3);
        QCOMPARE(subseq2->start(), 4);
    }

    QVector<SubseqChangePod> pods;

    // ------------------------------------------------------------------------
    // Test: extend second sequence one space
    // -----W-X- >>> ----VW-X-
    pods = msa.extendLeft(5, ClosedIntRange(2, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(5, 5));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "V");
    QVERIFY(*subseq2 == "----VW-X-");
    QCOMPARE(subseq2->start(), 3);

    // ------------------------------------------------------------------------
    // Test: extend both sequences, but only one can extend
    // ---C-DE-- >>> ---C-DE--
    // ----VW-X- >>> ---UVW-X-
    pods = msa.extendLeft(4, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "U");
    QVERIFY(*subseq  == "---C-DE--");
    QVERIFY(*subseq2 == "---UVW-X-");
    QCOMPARE(subseq->start(), 3);
    QCOMPARE(subseq2->start(), 2);

    // ------------------------------------------------------------------------
    // Test: extend both sequence as far as possible
    // ---C-DE-- >>> -ABC-DE--
    // ---UVW-X- >>> --TUVW-X-
    pods = msa.extendLeft(1, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(2, 3));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "AB");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(3, 3));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(1).difference_ == "T");
    QVERIFY(*subseq  == "-ABC-DE--");
    QVERIFY(*subseq2 == "--TUVW-X-");
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(subseq2->start(), 1);

    // ------------------------------------------------------------------------
    // Test: extend multiple spots at once
    msa.clear();
    subseq = new Subseq(seq1);
    subseq2 = new Subseq(seq2);

    //                                3 45
    QVERIFY(subseq->setBioString( "---C-DE--"));
    //                                  4 5
    QVERIFY(subseq2->setBioString("-----W-X-"));

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    pods = msa.extendLeft(1, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(2, 3));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "AB");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(3, 5));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(1).difference_ == "TUV");

    QVERIFY(*subseq  == "-ABC-DE--");
    QVERIFY(*subseq2 == "--TUVW-X-");
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(subseq2->start(), 1);

    // ------------------------------------------------------------------------
    // Test: extension only occurs within available gap spaces
    msa.clear();
    subseq = new Subseq(seq1);
    subseq2 = new Subseq(seq2);

    //                              3 45
    QVERIFY(subseq->setBioString( "-C-DE--"));
    //                               4 5
    QVERIFY(subseq2->setBioString("--W-X--"));

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    pods = msa.extendLeft(1, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(1, 1));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "B");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(1, 2));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(1).difference_ == "UV");
    QVERIFY(*subseq  == "BC-DE--");
    QVERIFY(*subseq2 == "UVW-X--");
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 2);
}

void TestMsa::extendLeftRowNumChars()
{
    Seq seq1("ABCDEF");
    Subseq *subseq = new Subseq(seq1);
    QVERIFY(subseq->setBioString("---D-E-F"));

    Msa msa;
    QVERIFY(msa.append(subseq));

    SubseqChangePod pod = msa.extendLeft(1, 1);
    QVERIFY(*subseq  == "--CD-E-F");
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pod.difference_ == "C");
    QCOMPARE(pod.columns_, ClosedIntRange(3, 3));

    pod = msa.extendLeft(1, 2);
    QVERIFY(*subseq  == "ABCD-E-F");
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pod.difference_ == "AB");
    QCOMPARE(pod.columns_, ClosedIntRange(1, 2));
}

void TestMsa::extendLeftColumnRowBioString()
{
    Seq seq1("ABCDEF");
    Subseq *subseq = new Subseq(seq1);
    QVERIFY(subseq->setBioString("-----EF"));

    Msa msa;
    QVERIFY(msa.append(subseq));

    SubseqChangePod pod = msa.extendLeft(2, 1, "C-D-");
    QVERIFY(*subseq  == "-C-D-EF");
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pod.difference_ == "C-D-");
    QCOMPARE(pod.columns_, ClosedIntRange(2, 5));
}

void TestMsa::extendRight()
{
    Seq seq1("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D---"));
    //                              3 4
    QVERIFY(subseq2->setBioString("-V-W-----"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    // ------------------------------------------------------------------------
    // Tests: extend subseqs that do nothing
    for (int i=1; i<= 4; ++i)
    {
        QVERIFY(msa.extendRight(i, ClosedIntRange(1, 1)).isEmpty());
        QVERIFY(msa.extendRight(i, ClosedIntRange(2, 2)).isEmpty());
        QVERIFY(msa.extendRight(i, ClosedIntRange(1, 2)).isEmpty());

        QVERIFY(*subseq  == "--BC-D---");
        QVERIFY(*subseq2 == "-V-W-----");
        QCOMPARE(subseq->stop(), 4);
        QCOMPARE(subseq2->stop(), 4);
    }

    QVector<SubseqChangePod> pods;

    // ------------------------------------------------------------------------
    // Test: extend second sequence one space
    // -V-W----- >>> -V-WX----
    pods = msa.extendRight(5, ClosedIntRange(2, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(5, 5));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "X");

    QVERIFY(*subseq2 == "-V-WX----");
    QCOMPARE(subseq2->stop(), 5);

    // ------------------------------------------------------------------------
    // Test: extend both sequences, but only one can extend
    // --BC-D--- >>> --BC-D---
    // -V-WX---- >>> -V-WXY---
    pods = msa.extendRight(6, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(6, 6));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "Y");
    QVERIFY(*subseq  == "--BC-D---");
    QVERIFY(*subseq2 == "-V-WXY---");
    QCOMPARE(subseq->stop(), 4);
    QCOMPARE(subseq2->stop(), 6);

    // ------------------------------------------------------------------------
    // Test: extend both sequence as far as possible
    // --BC-D--- >>> --BC-DEF-
    // -V-WXY--- >>> -V-WXYZ--
    pods = msa.extendRight(9, ClosedIntRange(1, 2));

    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(7, 8));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "EF");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(7, 7));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(1).difference_ == "Z");
    QVERIFY(*subseq  == "--BC-DEF-");
    QVERIFY(*subseq2 == "-V-WXYZ--");
    QCOMPARE(subseq->stop(), 6);
    QCOMPARE(subseq2->stop(), 7);

    // ------------------------------------------------------------------------
    // Test: extend multiple spots at once
    msa.clear();
    subseq = new Subseq(seq1);
    subseq2 = new Subseq(seq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D---"));
    //                              3 4
    QVERIFY(subseq2->setBioString("-V-W-----"));

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    pods = msa.extendRight(9, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(7, 8));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "EF");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(5, 7));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(1).difference_ == "XYZ");
    QVERIFY(*subseq  == "--BC-DEF-");
    QVERIFY(*subseq2 == "-V-WXYZ--");
    QCOMPARE(subseq->stop(), 6);
    QCOMPARE(subseq2->stop(), 7);

    // ------------------------------------------------------------------------
    // Test: extension only occurs within available gap spaces
    msa.clear();
    subseq = new Subseq(seq1);
    subseq2 = new Subseq(seq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D-"));
    //                              3  4
    QVERIFY(subseq2->setBioString("-V--W--"));

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    pods = msa.extendRight(7, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(7, 7));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "E");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(6, 7));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(1).difference_ == "XY");
    QVERIFY(*subseq  == "--BC-DE");
    QVERIFY(*subseq2 == "-V--WXY");
    QCOMPARE(subseq->stop(), 5);
    QCOMPARE(subseq2->stop(), 6);
}

void TestMsa::extendRightRowNumChars()
{
    Seq seq1("ABCDEF");
    Subseq *subseq = new Subseq(seq1);
    QVERIFY(subseq->setBioString("A-B-C---"));

    Msa msa;
    QVERIFY(msa.append(subseq));

    SubseqChangePod pod = msa.extendRight(1, 1);
    QVERIFY(*subseq  == "A-B-CD--");
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pod.difference_ == "D");
    QCOMPARE(pod.columns_, ClosedIntRange(6, 6));

    pod = msa.extendRight(1, 2);
    QVERIFY(*subseq  == "A-B-CDEF");
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pod.difference_ == "EF");
    QCOMPARE(pod.columns_, ClosedIntRange(7, 8));
}

void TestMsa::extendRightColumnRowBioString()
{
    Seq seq1("ABCDEF");
    Subseq *subseq = new Subseq(seq1);
    QVERIFY(subseq->setBioString("AB-----"));

    Msa msa;
    QVERIFY(msa.append(subseq));

    SubseqChangePod pod = msa.extendRight(4, 1, "-C-D");
    QVERIFY(*subseq  == "AB--C-D");
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pod.difference_ == "-C-D");
    QCOMPARE(pod.columns_, ClosedIntRange(4, 7));
}

void TestMsa::grammar()
{
    Msa msa;
    Msa msa2(eAminoGrammar);
    Msa msa3(eDnaGrammar);
    Msa msa4(eRnaGrammar);
    Msa msa5(eUnknownGrammar);

    QCOMPARE(msa.grammar(), eUnknownGrammar);
    QCOMPARE(msa2.grammar(), eAminoGrammar);
    QCOMPARE(msa3.grammar(), eDnaGrammar);
    QCOMPARE(msa4.grammar(), eRnaGrammar);
    QCOMPARE(msa5.grammar(), eUnknownGrammar);
}

void TestMsa::indexOfAbstractSeq()
{
    Seq seq("ABCDEF", eAminoGrammar);
    Subseq *subseq = new Subseq(seq);
    subseq->seqEntity_ = AminoSeq::createEntity("First", new Astring(1, seq));
    QVERIFY(subseq->setBioString("BC--DE"));

    Subseq *subseq2 = new Subseq(seq);
    QVERIFY(subseq2->seqEntity_ == nullptr);
    QVERIFY(subseq2->setBioString("--BCDE"));

    Subseq *subseq3 = new Subseq(seq);
    subseq3->seqEntity_ = AminoSeq::createEntity("Third", new Astring(2, seq));

    Msa msa(eAminoGrammar);
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QCOMPARE(msa.indexOfAbstractSeq(nullptr), 0);
    QCOMPARE(msa.indexOfAbstractSeq(subseq->seqEntity_), 1);
    QCOMPARE(msa.indexOfAbstractSeq(subseq3->seqEntity_), 3);
}

void TestMsa::insert()
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

    Msa msa;

    // Test: insert at beginning
    QVERIFY(msa.insert(1, subseq));
    QVERIFY(msa.insert(1, subseq2));
    QVERIFY(msa.insert(1, subseq3));

    QVERIFY(msa.subseqCount() == 3);
    QVERIFY(msa.length() == 6);

    QVERIFY(msa.at(1) == subseq3);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq);

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
    QVERIFY(msa.insert(2, subseq2));
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);

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
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
}

void TestMsa::insertGapColumns()
{
    Seq seq("ABC");
    Subseq *subseq = new Subseq(seq);

    Seq seq2("DEF");
    Subseq *subseq2 = new Subseq(seq2);

    Seq seq3("GHI");
    Subseq *subseq3 = new Subseq(seq3);

    Seq seq4("JKL");
    Subseq *subseq4 = new Subseq(seq4);

    Seq seq5("MNO");
    Subseq *subseq5 = new Subseq(seq5);

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));
    QVERIFY(msa.append(subseq4));
    QVERIFY(msa.append(subseq5));

    // Test: inserting gap columns at any position with n = 0, does nothing
    for (int i=1; i<= msa.length()+1; ++i)
    {
        msa.insertGapColumns(i, 0);
        QVERIFY(*msa.at(1) == "ABC");
        QVERIFY(*msa.at(2) == "DEF");
        QVERIFY(*msa.at(3) == "GHI");
        QVERIFY(*msa.at(4) == "JKL");
        QVERIFY(*msa.at(5) == "MNO");
    }

    // Test: insert gap of 1 length at beginning
    msa.insertGapColumns(1, 1);
    QVERIFY(*msa.at(1) == "-ABC");
    QVERIFY(*msa.at(2) == "-DEF");
    QVERIFY(*msa.at(3) == "-GHI");
    QVERIFY(*msa.at(4) == "-JKL");
    QVERIFY(*msa.at(5) == "-MNO");

    // Test: insert another gap at the beginning
    msa.insertGapColumns(1, 1);
    QVERIFY(*msa.at(1) == "--ABC");
    QVERIFY(*msa.at(2) == "--DEF");
    QVERIFY(*msa.at(3) == "--GHI");
    QVERIFY(*msa.at(4) == "--JKL");
    QVERIFY(*msa.at(5) == "--MNO");

    // Test: insert gap in the middle
    msa.insertGapColumns(4, 1, '.');
    QVERIFY(*msa.at(1) == "--A.BC");
    QVERIFY(*msa.at(2) == "--D.EF");
    QVERIFY(*msa.at(3) == "--G.HI");
    QVERIFY(*msa.at(4) == "--J.KL");
    QVERIFY(*msa.at(5) == "--M.NO");

    // Test: insert another gap in the middle
    msa.insertGapColumns(5, 1);
    QVERIFY(*msa.at(1) == "--A.-BC");
    QVERIFY(*msa.at(2) == "--D.-EF");
    QVERIFY(*msa.at(3) == "--G.-HI");
    QVERIFY(*msa.at(4) == "--J.-KL");
    QVERIFY(*msa.at(5) == "--M.-NO");

    // Test: insert gap at end
    msa.insertGapColumns(8, 1);
    QVERIFY(*msa.at(1) == "--A.-BC-");
    QVERIFY(*msa.at(2) == "--D.-EF-");
    QVERIFY(*msa.at(3) == "--G.-HI-");
    QVERIFY(*msa.at(4) == "--J.-KL-");
    QVERIFY(*msa.at(5) == "--M.-NO-");

    // Test: insert another gap at end
    msa.insertGapColumns(8, 1);
    QVERIFY(*msa.at(1) == "--A.-BC--");
    QVERIFY(*msa.at(2) == "--D.-EF--");
    QVERIFY(*msa.at(3) == "--G.-HI--");
    QVERIFY(*msa.at(4) == "--J.-KL--");
    QVERIFY(*msa.at(5) == "--M.-NO--");

    // Test: insertion of several gaps
    msa.insertGapColumns(7, 4);
    QVERIFY(*msa.at(1) == "--A.-B----C--");
    QVERIFY(*msa.at(2) == "--D.-E----F--");
    QVERIFY(*msa.at(3) == "--G.-H----I--");
    QVERIFY(*msa.at(4) == "--J.-K----L--");
    QVERIFY(*msa.at(5) == "--M.-N----O--");
}

void TestMsa::insertRows()
{
    Seq seq1("ABCDEF");
    QVector<Subseq *> subseqs;
    for (int i=0; i<6; ++i)
        subseqs << new Subseq(seq1);

    Msa msa;

    QVERIFY(msa.insertRows(1, subseqs));
    for (int i=0; i< 6; ++i)
        QVERIFY(msa.at(i+1) == subseqs.at(i));
}

void TestMsa::isCompatibleSubseq()
{
    Msa msa;

    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));

    // Test: generic
    QVERIFY(msa.isCompatibleSubseq(subseq1));

    // Test: grammar check
    {
        Seq seq("ABC", eAminoGrammar);
        Subseq subseq(seq);
        QCOMPARE(msa.isCompatibleSubseq(&subseq), false);

        Msa msa2(eAminoGrammar);
        QCOMPARE(msa2.isCompatibleSubseq(&subseq), true);
    }

    // Test: length check
    QVERIFY(msa.append(subseq1));
    Subseq *subseq2 = new Subseq(seq1);
    QVERIFY(subseq2->setBioString("A"));

    QCOMPARE(msa.isCompatibleSubseq(subseq2), false);
    subseq2->setBioString("ABCDEF");
    QCOMPARE(msa.isCompatibleSubseq(subseq2), true);
    delete subseq2;
    subseq2 = nullptr;

    msa.clear();

    // Test: Has at least one non-gap
    Seq seq2;
    subseq2 = new Subseq(seq2);
    QCOMPARE(msa.isCompatibleSubseq(subseq2), false);
    delete subseq2;
    subseq2 = nullptr;
}

void TestMsa::isCompatibleSubseqVector()
{
    Msa msa;

    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));

    // Test: generic
    QVERIFY(msa.isCompatibleSubseqVector(QVector<Subseq *>() << subseq1));
    QVERIFY(msa.append(subseq1));

    Subseq *subseq2 = new Subseq(seq1);

    Seq seq2("ABCDEF", eAminoGrammar);
    Subseq *subseq3 = new Subseq(seq2);

    QCOMPARE(msa.isCompatibleSubseqVector(QVector<Subseq *>() << subseq2 << subseq3), false);
}

void TestMsa::isEmpty()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Msa msa;
    QVERIFY(msa.isEmpty());

    QVERIFY(msa.append(subseq));
    QVERIFY(msa.isEmpty() == false);

    msa.clear();

    QVERIFY(msa.isEmpty());
}

void TestMsa::isValidColumn()
{
    Msa msa;

    for (int i=-3; i<=3; ++i)
        QVERIFY(!msa.isValidColumn(i));

    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    QVERIFY(subseq->setBioString( "----BC-D-"));
    QVERIFY(subseq2->setBioString("---V-WX--"));
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QVERIFY(!msa.isValidColumn(0));
    for (int i=1; i<= msa.length(); ++i)
        QVERIFY(msa.isValidColumn(i));
    QVERIFY(!msa.isValidColumn(msa.length()+1));
}

void TestMsa::isValidRow()
{
    Msa msa;

    for (int i=-3; i<=3; ++i)
        QVERIFY(!msa.isValidRow(i));

    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    QVERIFY(subseq->setBioString( "----BC-D-"));
    QVERIFY(subseq2->setBioString("---V-WX--"));
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QVERIFY(!msa.isValidRow(0));
    QVERIFY(msa.isValidRow(1));
    QVERIFY(msa.isValidRow(2));
    QVERIFY(!msa.isValidRow(3));
}

void TestMsa::isValidRowRange()
{
    Msa msa;

    for (int i=-3; i<=3; ++i)
        QVERIFY(!msa.isValidRowRange(ClosedIntRange(i, i)));

    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    QVERIFY(subseq->setBioString( "----BC-D-"));
    QVERIFY(subseq2->setBioString("---V-WX--"));
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    QVERIFY(msa.isValidRowRange(ClosedIntRange(1, 1)));
    QVERIFY(msa.isValidRowRange(ClosedIntRange(2, 2)));
    QVERIFY(msa.isValidRowRange(ClosedIntRange(1, 2)));
    QVERIFY(!msa.isValidRowRange(ClosedIntRange()));
    QVERIFY(!msa.isValidRowRange(ClosedIntRange(2, 1)));
    QVERIFY(!msa.isValidRowRange(ClosedIntRange(0, 3)));
    QVERIFY(!msa.isValidRowRange(ClosedIntRange(3, 0)));
    QVERIFY(!msa.isValidRowRange(ClosedIntRange(-1, 1)));
    QVERIFY(!msa.isValidRowRange(ClosedIntRange(3, 3)));
    QVERIFY(!msa.isValidRowRange(ClosedIntRange(0, 1)));
}

void TestMsa::leftExtendableLength()
{
    Msa msa;
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));
    QVERIFY(msa.append(subseq1));

    for (int i=1; i<= subseq1->length(); ++i)
        QCOMPARE(msa.leftExtendableLength(i, 1), 0);

    QVERIFY(subseq1->setBioString("--EF"));
    QCOMPARE(msa.leftExtendableLength(1, 1), 2);
    QCOMPARE(msa.leftExtendableLength(2, 1), 1);
    QCOMPARE(msa.leftExtendableLength(3, 1), 0);

    QVERIFY(subseq1->setBioString("--AB"));
    QCOMPARE(msa.leftExtendableLength(1, 1), 0);
    QCOMPARE(msa.leftExtendableLength(2, 1), 0);
    QCOMPARE(msa.leftExtendableLength(3, 1), 0);
}

void TestMsa::leftTrimmableLength()
{
    Msa msa;
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(msa.append(subseq1));

    QCOMPARE(msa.leftTrimmableLength(1, 1), 1);
    QCOMPARE(msa.leftTrimmableLength(2, 1), 2);
    QCOMPARE(msa.leftTrimmableLength(3, 1), 3);
    QCOMPARE(msa.leftTrimmableLength(4, 1), 4);
    QCOMPARE(msa.leftTrimmableLength(5, 1), 5);
    QCOMPARE(msa.leftTrimmableLength(6, 1), 5);

    QVERIFY(subseq1->setBioString("--C--"));
    for (int i=1; i<= 5; ++i)
        QCOMPARE(msa.leftTrimmableLength(i, 1), 0);

    QVERIFY(subseq1->setBioString("A-B--C-D"));
    QCOMPARE(msa.leftTrimmableLength(1, 1), 1);
    QCOMPARE(msa.leftTrimmableLength(2, 1), 1);
    QCOMPARE(msa.leftTrimmableLength(3, 1), 2);
    QCOMPARE(msa.leftTrimmableLength(4, 1), 2);
    QCOMPARE(msa.leftTrimmableLength(5, 1), 2);
    QCOMPARE(msa.leftTrimmableLength(6, 1), 3);
    QCOMPARE(msa.leftTrimmableLength(7, 1), 3);
    QCOMPARE(msa.leftTrimmableLength(8, 1), 3);
}

void TestMsa::length()
{
    Msa msa;

    QCOMPARE(msa.length(), 0);

    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));

    QVERIFY(msa.append(subseq1));
    QCOMPARE(msa.length(), 6);
}

void TestMsa::levelLeft()
{
    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    //                                 23 4
    QVERIFY(subseq->setBioString( "----BC-D-"));
    //                                3 45
    QVERIFY(subseq2->setBioString("---V-WX--"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    // ------------------------------------------------------------------------
    // Test: level operations that do nothing
    QVERIFY(msa.levelLeft(5, ClosedIntRange(1, 1)).isEmpty());
    QVERIFY(msa.levelLeft(4, ClosedIntRange(2, 2)).isEmpty());
    QVERIFY(*subseq == "----BC-D-");
    QVERIFY(*subseq2 == "---V-WX--");
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 3);

    // ------------------------------------------------------------------------
    // Test: extend and trim a subseq
    // ----BC-D- >>> ---ABC-D-
    QVector<SubseqChangePod> pods;
    pods = msa.levelLeft(4, ClosedIntRange(1, 1));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "A");
    QVERIFY(*subseq == "---ABC-D-");
    QCOMPARE(subseq->start(), 1);

    pods = msa.levelLeft(5, ClosedIntRange(1, 1));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "A");
    QVERIFY(*subseq == "----BC-D-");
    QCOMPARE(subseq->start(), 2);

    pods = msa.levelLeft(1, ClosedIntRange(1, 1));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "A");
    QVERIFY(*subseq == "---ABC-D-");
    QCOMPARE(subseq->start(), 1);

    pods = msa.levelLeft(6, ClosedIntRange(1, 1));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 5));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "AB");
    QVERIFY(*subseq == "-----C-D-");
    QCOMPARE(subseq->start(), 3);

    pods = msa.levelLeft(2, ClosedIntRange(1, 1));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 5));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "AB");
    QVERIFY(*subseq == "---ABC-D-");
    QCOMPARE(subseq->start(), 1);

    // ------------------------------------------------------------------------
    // Test: shifting by one character
    // ---V-WX-- >>> ----VWX--
    pods = msa.levelLeft(5, ClosedIntRange(2, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "V");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(5, 5));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(1).difference_ == "V");
    QVERIFY(*subseq2 == "----VWX--");
    QCOMPARE(subseq2->start(), 3);

    // ------------------------------------------------------------------------
    // Test: multiple sequence level
    // ---ABC-D- >>> ---ABC-D-
    // ----VWX-- >>> --TUVWX--
    pods = msa.levelLeft(1, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(3, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pods.at(0).difference_ == "TU");
    QVERIFY(*subseq == "---ABC-D-");
    QVERIFY(*subseq2 == "--TUVWX--");
    QCOMPARE(subseq->start(), 1);
    QCOMPARE(subseq2->start(), 1);

    // ---ABC-D- >>> ----BC-D-
    // --TUVWX-- >>> ----VWX--
    pods = msa.levelLeft(5, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "A");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(3, 4));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(1).difference_ == "TU");
    QVERIFY(*subseq == "----BC-D-");
    QVERIFY(*subseq2 == "----VWX--");
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 3);

    // ------------------------------------------------------------------------
    // Test: multiple sequence, level to end
    // ----BC-D- >>> -------D-
    // ----VWX-- >>> ------X--
    pods = msa.levelLeft(9, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(5, 6));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "BC");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(5, 6));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(1).difference_ == "VW");
    QVERIFY(*subseq == "-------D-");
    QVERIFY(*subseq2 == "------X--");
    QCOMPARE(subseq->start(), 4);
    QCOMPARE(subseq2->start(), 5);
}

void TestMsa::levelRight()
{
    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    //                                 23 4
    QVERIFY(subseq->setBioString( "----BC-D-"));
    //                                3 45
    QVERIFY(subseq2->setBioString("---V-WX--"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    // ------------------------------------------------------------------------
    // Test: level operations that do nothing
    QVERIFY(msa.levelRight(8, ClosedIntRange(1, 1)).isEmpty());
    QVERIFY(msa.levelRight(7, ClosedIntRange(2, 2)).isEmpty());
    QVERIFY(*subseq == "----BC-D-");
    QVERIFY(*subseq2 == "---V-WX--");
    QCOMPARE(subseq->stop(), 4);
    QCOMPARE(subseq2->stop(), 5);

    // ------------------------------------------------------------------------
    // Test: extend and trim a subseq
    // ---V-WX-- >>> ---V-WXY-
    QVector<SubseqChangePod> pods;
    pods = msa.levelRight(8, ClosedIntRange(2, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(8, 8));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "Y");
    QVERIFY(*subseq2 ==  "---V-WXY-");
    QCOMPARE(subseq2->stop(), 6);

    pods = msa.levelRight(9, ClosedIntRange(2, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(9, 9));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "Z");
    QVERIFY(*subseq2 ==  "---V-WXYZ");
    QCOMPARE(subseq2->stop(), 7);

    pods = msa.levelRight(7, ClosedIntRange(2, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(8, 9));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "YZ");
    QVERIFY(*subseq2 ==  "---V-WX--");
    QCOMPARE(subseq2->stop(), 5);

    pods = msa.levelRight(9, ClosedIntRange(2, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(8, 9));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "YZ");
    QVERIFY(*subseq2 ==  "---V-WXYZ");
    QCOMPARE(subseq2->stop(), 7);

    pods = msa.levelRight(7, ClosedIntRange(2, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(8, 9));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "YZ");
    QVERIFY(*subseq2 ==  "---V-WX--");
    QCOMPARE(subseq2->stop(), 5);

    // ------------------------------------------------------------------------
    // Test: shifting by one character
    // ----BC-D- >>> ----BCD--
    pods = msa.levelRight(7, ClosedIntRange(1, 1));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(8, 8));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "D");
    QCOMPARE(pods.at(1).row_, 1);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(7, 7));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(1).difference_ == "D");
    QVERIFY(*subseq == "----BCD--");
    QCOMPARE(subseq->stop(), 4);

    // ------------------------------------------------------------------------
    // Test: multiple sequence level
    // ----BCD-- >>> ----BCDE-
    // ---V-WX-- >>> ---V-WXY-
    pods = msa.levelRight(8, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(8, 8));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(0).difference_ == "E");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(8, 8));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(1).difference_ == "Y");
    QVERIFY(*subseq ==  "----BCDE-");
    QVERIFY(*subseq2 == "---V-WXY-");
    QCOMPARE(subseq->stop(), 5);
    QCOMPARE(subseq2->stop(), 6);

    // ----BCDE- >>> ----B----
    // ---V-WXY- >>> ---VW----
    // Note: does all trims and then extensions as necessary
    pods = msa.levelRight(5, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 3);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(6, 8));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "CDE");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(6, 8));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(1).difference_ == "WXY");
    QCOMPARE(pods.at(2).row_, 2);
    QCOMPARE(pods.at(2).columns_, ClosedIntRange(5, 5));
    QCOMPARE(pods.at(2).operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pods.at(2).difference_ == "W");
    QVERIFY(*subseq ==  "----B----");
    QVERIFY(*subseq2 == "---VW----");
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 4);

    // ------------------------------------------------------------------------
    // Test: multiple sequence, level to end
    pods = msa.levelRight(1, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(5, 5));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "W");
    QVERIFY(*subseq ==  "----B----");
    QVERIFY(*subseq2 == "---V-----");
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);
}

void TestMsa::members()
{
    Msa msa;

    // Test: initial state should have no members
    QVERIFY(msa.members().isEmpty());

    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Seq seq3("CDEF");
    Subseq *subseq3 = new Subseq(seq3);
    QVERIFY(subseq3->setBioString("--CD"));

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

void TestMsa::moveRow()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Seq seq3("ABCDEF");
    Subseq *subseq3 = new Subseq(seq3);

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    int n = msa.subseqCount();

    // Test: move with itself does nothing
    for (int i=-n; i <= n; ++i)
    {
        if (i == 0)
            continue;

        msa.moveRow(i, i);

        // Nothing should have changed
        QVERIFY(msa.at(1) == subseq);
        QVERIFY(msa.at(2) == subseq2);
        QVERIFY(msa.at(3) == subseq3);
    }

    // Test: move positive indices
    msa.moveRow(1, 3);
    QVERIFY(msa.at(1) == subseq2);
    QVERIFY(msa.at(2) == subseq3);
    QVERIFY(msa.at(3) == subseq);

    msa.moveRow(3, 1);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);

    msa.moveRow(1, 2);
    QVERIFY(msa.at(1) == subseq2);
    QVERIFY(msa.at(2) == subseq);
    QVERIFY(msa.at(3) == subseq3);

    msa.moveRow(2, 1);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
}

void TestMsa::moveRowRange()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Seq seq3("ABCDEF");
    Subseq *subseq3 = new Subseq(seq3);

    Seq seq4("XYZTUV");
    Subseq *subseq4 = new Subseq(seq4);

    Seq seq5("WRSTID");
    Subseq *subseq5 = new Subseq(seq5);

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));
    QVERIFY(msa.append(subseq4));
    QVERIFY(msa.append(subseq5));

    int n = msa.subseqCount();

    // Test: swap with itself does nothing
    for (int i=1; i <= n; ++i)
    {
        msa.moveRowRange(ClosedIntRange(i, i), i);

        // Nothing should have changed
        QVERIFY(msa.at(1) == subseq);
        QVERIFY(msa.at(2) == subseq2);
        QVERIFY(msa.at(3) == subseq3);
        QVERIFY(msa.at(4) == subseq4);
        QVERIFY(msa.at(5) == subseq5);
    }

    // Test: two at a time
    for (int i=1; i<= n-1; ++i)
    {
        msa.moveRowRange(ClosedIntRange(i, i+1), i);
        QVERIFY(msa.at(1) == subseq);
        QVERIFY(msa.at(2) == subseq2);
        QVERIFY(msa.at(3) == subseq3);
        QVERIFY(msa.at(4) == subseq4);
        QVERIFY(msa.at(5) == subseq5);
    }

    // Test: move positive indices
    msa.moveRowRange(ClosedIntRange(1, 2), 4);
    QVERIFY(msa.at(1) == subseq3);
    QVERIFY(msa.at(2) == subseq4);
    QVERIFY(msa.at(3) == subseq5);
    QVERIFY(msa.at(4) == subseq);
    QVERIFY(msa.at(5) == subseq2);

    msa.moveRowRange(ClosedIntRange(4, 5), 1);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
    QVERIFY(msa.at(4) == subseq4);
    QVERIFY(msa.at(5) == subseq5);

    msa.moveRowRange(ClosedIntRange(2, 5), 1);
    QVERIFY(msa.at(1) == subseq2);
    QVERIFY(msa.at(2) == subseq3);
    QVERIFY(msa.at(3) == subseq4);
    QVERIFY(msa.at(4) == subseq5);
    QVERIFY(msa.at(5) == subseq);

    msa.moveRowRange(ClosedIntRange(1, 4), 2);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
    QVERIFY(msa.at(4) == subseq4);
    QVERIFY(msa.at(5) == subseq5);
}

void TestMsa::moveRowRangeRelative()
{
    Seq seq("ABCDEF");
    Seq seq2("GHIJKL");
    Seq seq3("ABCDEF");
    Seq seq4("XYZTUV");
    Seq seq5("WRSTID");

    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));
    Subseq *subseq3 = new Subseq(seq3);
    Subseq *subseq4 = new Subseq(seq4);
    Subseq *subseq5 = new Subseq(seq5);

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));
    QVERIFY(msa.append(subseq4));
    QVERIFY(msa.append(subseq5));

    // Test: moving beyond borders should not change anything
    msa.moveRowRangeRelative(ClosedIntRange(1, 3), 0);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
    QVERIFY(msa.at(4) == subseq4);
    QVERIFY(msa.at(5) == subseq5);

    // Test: Move one spot at a time
    msa.moveRowRangeRelative(ClosedIntRange(1, 3), 1);
    QVERIFY(msa.at(1) == subseq4);
    QVERIFY(msa.at(2) == subseq);
    QVERIFY(msa.at(3) == subseq2);
    QVERIFY(msa.at(4) == subseq3);
    QVERIFY(msa.at(5) == subseq5);

    msa.moveRowRangeRelative(ClosedIntRange(2, 4), 1);
    QVERIFY(msa.at(1) == subseq4);
    QVERIFY(msa.at(2) == subseq5);
    QVERIFY(msa.at(3) == subseq);
    QVERIFY(msa.at(4) == subseq2);
    QVERIFY(msa.at(5) == subseq3);

    msa.moveRowRangeRelative(ClosedIntRange(4, 5), -1);
    QVERIFY(msa.at(1) == subseq4);
    QVERIFY(msa.at(2) == subseq5);
    QVERIFY(msa.at(3) == subseq2);
    QVERIFY(msa.at(4) == subseq3);
    QVERIFY(msa.at(5) == subseq);

    msa.moveRowRangeRelative(ClosedIntRange(3, 4), -1);
    QVERIFY(msa.at(1) == subseq4);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
    QVERIFY(msa.at(4) == subseq5);
    QVERIFY(msa.at(5) == subseq);

    // Test: move 2 spots at a time
    msa.moveRowRangeRelative(ClosedIntRange(1, 2), 2);
    QVERIFY(msa.at(1) == subseq3);
    QVERIFY(msa.at(2) == subseq5);
    QVERIFY(msa.at(3) == subseq4);
    QVERIFY(msa.at(4) == subseq2);
    QVERIFY(msa.at(5) == subseq);

    msa.moveRowRangeRelative(ClosedIntRange(3, 4), -2);
    QVERIFY(msa.at(1) == subseq4);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
    QVERIFY(msa.at(4) == subseq5);
    QVERIFY(msa.at(5) == subseq);

    // Test: move beyond spots should only move allowable spots
    msa.moveRowRangeRelative(ClosedIntRange(1, 3), 2);
    QVERIFY(msa.at(1) == subseq5);
    QVERIFY(msa.at(2) == subseq);
    QVERIFY(msa.at(3) == subseq4);
    QVERIFY(msa.at(4) == subseq2);
    QVERIFY(msa.at(5) == subseq3);

    msa.moveRowRangeRelative(ClosedIntRange(3, 5), -2);
    QVERIFY(msa.at(1) == subseq4);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
    QVERIFY(msa.at(4) == subseq5);
    QVERIFY(msa.at(5) == subseq);
}

void TestMsa::moveRowRelative()
{
    Seq seq("ABCDEF");
    Seq seq2("GHIJKL");
    Seq seq3("ABCDEF");

    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));
    Subseq *subseq3 = new Subseq(seq3);

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    // Test: moving row 0 elements, should do nothing
    msa.moveRowRelative(1, 0);
    msa.moveRowRelative(2, 0);
    msa.moveRowRelative(3, 0);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);

    // Test: Move one spot at a time
    msa.moveRowRelative(1, 1);
    QVERIFY(msa.at(1) == subseq2);
    QVERIFY(msa.at(2) == subseq);
    QVERIFY(msa.at(3) == subseq3);

    msa.moveRowRelative(2, 1);
    QVERIFY(msa.at(1) == subseq2);
    QVERIFY(msa.at(2) == subseq3);
    QVERIFY(msa.at(3) == subseq);

    msa.moveRowRelative(3, -1);
    QVERIFY(msa.at(1) == subseq2);
    QVERIFY(msa.at(2) == subseq);
    QVERIFY(msa.at(3) == subseq3);

    msa.moveRowRelative(2, -1);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);

    // Test: move 2 spots at a time
    msa.moveRowRelative(1, 2);
    QVERIFY(msa.at(1) == subseq2);
    QVERIFY(msa.at(2) == subseq3);
    QVERIFY(msa.at(3) == subseq);

    msa.moveRowRelative(3, -2);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
}

void TestMsa::prepend()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Seq seq3("CDEF");
    Subseq *subseq3 = new Subseq(seq3);
    QVERIFY(subseq3->setBioString("--CD"));

    Msa msa;
    QVERIFY(msa.length() == 0);

    QVERIFY(msa.prepend(subseq));
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa.prepend(subseq2));
    QVERIFY(msa.subseqCount() == 2);
    QVERIFY(msa.prepend(subseq3) == false);
    QVERIFY(msa.subseqCount() == 2);

    subseq3->insertGaps(5, 2, constants::kDefaultGapCharacter);
    QVERIFY(msa.prepend(subseq3));
    QVERIFY(msa.subseqCount() == 3);

    QVERIFY(msa.at(1) == subseq3);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq);
}

void TestMsa::removeAt()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Seq seq3("ABCDEF");
    Subseq *subseq3 = new Subseq(seq3);

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    // Test: remove the middle sequence
    msa.removeAt(2);
    QVERIFY(msa.subseqCount() == 2);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq3);

    msa.removeAt(1);
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa.at(1) == subseq3);
}

void TestMsa::removeFirst()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    msa.removeFirst();
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa.at(1) == subseq2);

    msa.removeFirst();
    QVERIFY(msa.subseqCount() == 0);
}

void TestMsa::removeRows()
{
    Seq seq("ABCDEF");

    Msa msa;
    Subseq *subseqs[6];
    for (int i=0; i<6; ++i)
    {
        subseqs[i] = new Subseq(seq);
        QVERIFY(msa.append(subseqs[i]));
    }

    msa.removeRows(ClosedIntRange(2, 4));

    QCOMPARE(msa.rowCount(), 3);
    QVERIFY(msa.at(1) == subseqs[0]);
    QVERIFY(msa.at(2) == subseqs[4]);
    QVERIFY(msa.at(3) == subseqs[5]);
}

void TestMsa::removeGapColumns()
{
    Seq seq("ABC");
    Subseq *subseq = new Subseq(seq);

    Seq seq2("DEF");
    Subseq *subseq2 = new Subseq(seq2);

    Seq seq3("GHIX");
    Subseq *subseq3 = new Subseq(seq3);

    QVERIFY(subseq->setBioString( "-A.B-C---"));
    QVERIFY(subseq2->setBioString("-D.E-F..-"));
    QVERIFY(subseq3->setBioString("-G.H.I.-X"));

    Msa msa;

    // Test: empty msa
    QVector<ClosedIntRange> removedGapRanges = msa.removeGapColumns();
    QCOMPARE(removedGapRanges.size(), 0);
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    removedGapRanges = msa.removeGapColumns();
    QCOMPARE(removedGapRanges.size(), 4);
    QCOMPARE(removedGapRanges.at(0), ClosedIntRange(1, 1));
    QCOMPARE(removedGapRanges.at(1), ClosedIntRange(3, 3));
    QCOMPARE(removedGapRanges.at(2), ClosedIntRange(5, 5));
    QCOMPARE(removedGapRanges.at(3), ClosedIntRange(7, 8));
    QVERIFY(*msa.at(1) == "ABC-");
    QVERIFY(*msa.at(2) == "DEF-");
    QVERIFY(*msa.at(3) == "GHIX");

    removedGapRanges = msa.removeGapColumns();
    QCOMPARE(removedGapRanges.size(), 0);
    QVERIFY(*msa.at(1) == "ABC-");
    QVERIFY(*msa.at(2) == "DEF-");
    QVERIFY(*msa.at(3) == "GHIX");
}

void TestMsa::removeGapColumnsInRange()
{
    Seq seq("ABC");
    Subseq *subseq = new Subseq(seq);

    Seq seq2("DEF");
    Subseq *subseq2 = new Subseq(seq2);

    Seq seq3("GHIX");
    Subseq *subseq3 = new Subseq(seq3);

    //                             123456789
    QVERIFY(subseq->setBioString( "-A.B-C---"));
    QVERIFY(subseq2->setBioString("-D.E-F..-"));
    QVERIFY(subseq3->setBioString("-G.H.I.-X"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    // Test: removing gap columns from those areas without gaps
    QVERIFY(msa.removeGapColumns(ClosedIntRange(2, 2)).isEmpty());
    QVERIFY(msa.removeGapColumns(ClosedIntRange(6, 6)).isEmpty());

    QVector<ClosedIntRange> removedGapColumns;
    removedGapColumns = msa.removeGapColumns(ClosedIntRange(8, 9));
    QCOMPARE(removedGapColumns.size(), 1);
    QCOMPARE(removedGapColumns.at(0), ClosedIntRange(8, 8));
    QVERIFY(*msa.at(1) == "-A.B-C--");
    QVERIFY(*msa.at(2) == "-D.E-F.-");
    QVERIFY(*msa.at(3) == "-G.H.I.X");

    removedGapColumns = msa.removeGapColumns(ClosedIntRange(2, 6));
    QCOMPARE(removedGapColumns.size(), 2);
    QCOMPARE(removedGapColumns.at(0), ClosedIntRange(3, 3));
    QCOMPARE(removedGapColumns.at(1), ClosedIntRange(5, 5));
    QVERIFY(*msa.at(1) == "-ABC--");
    QVERIFY(*msa.at(2) == "-DEF.-");
    QVERIFY(*msa.at(3) == "-GHI.X");

    removedGapColumns = msa.removeGapColumns(ClosedIntRange(1, 6));
    QCOMPARE(removedGapColumns.size(), 2);
    QCOMPARE(removedGapColumns.at(0), ClosedIntRange(1, 1));
    QCOMPARE(removedGapColumns.at(1), ClosedIntRange(5, 5));
    QVERIFY(*msa.at(1) == "ABC-");
    QVERIFY(*msa.at(2) == "DEF-");
    QVERIFY(*msa.at(3) == "GHIX");

    QVERIFY(msa.removeGapColumns(ClosedIntRange(1, 4)).isEmpty());
    QVERIFY(*msa.at(1) == "ABC-");
    QVERIFY(*msa.at(2) == "DEF-");
    QVERIFY(*msa.at(3) == "GHIX");
}

void TestMsa::removeLast()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    msa.removeLast();
    QVERIFY(msa.subseqCount() == 1);
    QVERIFY(msa.at(1) == subseq);

    msa.removeLast();
    QVERIFY(msa.subseqCount() == 0);
}

void TestMsa::rightExtendableLength()
{
    Msa msa;
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(subseq1->setBioString("BC--DE"));
    QVERIFY(msa.append(subseq1));

    for (int i=1; i<= subseq1->length(); ++i)
        QCOMPARE(msa.rightExtendableLength(i, 1), 0);

    QVERIFY(subseq1->setBioString("AB--"));
    QCOMPARE(msa.rightExtendableLength(2, 1), 0);
    QCOMPARE(msa.rightExtendableLength(3, 1), 1);
    QCOMPARE(msa.rightExtendableLength(4, 1), 2);

    QVERIFY(subseq1->setBioString("EF--"));
    QCOMPARE(msa.rightExtendableLength(2, 1), 0);
    QCOMPARE(msa.rightExtendableLength(3, 1), 0);
    QCOMPARE(msa.rightExtendableLength(4, 1), 0);
}

void TestMsa::rightTrimmableLength()
{
    Msa msa;
    Seq seq1("ABCDEF");
    Subseq *subseq1 = new Subseq(seq1);
    QVERIFY(msa.append(subseq1));

    QCOMPARE(msa.rightTrimmableLength(1, 1), 5);
    QCOMPARE(msa.rightTrimmableLength(2, 1), 5);
    QCOMPARE(msa.rightTrimmableLength(3, 1), 4);
    QCOMPARE(msa.rightTrimmableLength(4, 1), 3);
    QCOMPARE(msa.rightTrimmableLength(5, 1), 2);
    QCOMPARE(msa.rightTrimmableLength(6, 1), 1);

    QVERIFY(subseq1->setBioString("--C--"));
    for (int i=1; i<= 5; ++i)
        QCOMPARE(msa.rightTrimmableLength(i, 1), 0);

    QVERIFY(subseq1->setBioString("A-B--C-D"));
    QCOMPARE(msa.rightTrimmableLength(1, 1), 3);
    QCOMPARE(msa.rightTrimmableLength(2, 1), 3);
    QCOMPARE(msa.rightTrimmableLength(3, 1), 3);
    QCOMPARE(msa.rightTrimmableLength(4, 1), 2);
    QCOMPARE(msa.rightTrimmableLength(5, 1), 2);
    QCOMPARE(msa.rightTrimmableLength(6, 1), 2);
    QCOMPARE(msa.rightTrimmableLength(7, 1), 1);
    QCOMPARE(msa.rightTrimmableLength(8, 1), 1);
}

void TestMsa::setSubseqStart()
{
    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    QVERIFY(subseq->setBioString( "ABC-"));
    QVERIFY(subseq2->setBioString("--WX"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    // ------------------------------------------------------------------------
    // Test: set start to identical start - no change
    SubseqChangePod pod = msa.setSubseqStart(1, 1);
    QVERIFY(pod.isNull());

    // ------------------------------------------------------------------------
    // Test: move start forward 2 spots
    // ABC- >>> --C-
    pod = msa.setSubseqStart(1, 3);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(1, 2));
    QCOMPARE(pod.operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pod.difference_ == "AB");
    QVERIFY(*subseq == "--C-");

    /*
       !!! Updated Msa contract meant that the following tests no longer apply. See Msa introduction

    // ------------------------------------------------------------------------
    // Test: move start beyond stop
    // --C- >>> ---D
    //
    // Behind the scenes: --C- >>> --CD >>> ---D ; and thus both an extended and trimmed signal
    pod = msa.setSubseqStart(1, 4);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(3, 4));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendRightTrimLeft);
    QVERIFY(pod.difference_ == "C");
    QVERIFY(*subseq == "---D");

    // ------------------------------------------------------------------------
    // Test: move start beyond right alignment terminus
    // ---D >>> -----F
    //
    // Real order of operations:
    // ---D   >>> ---D-- (insert two terminal gap columns)
    // ---D-- >>> -----F (move the start and stop)
    pod = msa.setSubseqStart(1, 6);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(4, 6));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendRightTrimLeft);
    QVERIFY(pod.difference_ == "D");

    QVERIFY(*subseq  == "-----F");
    QVERIFY(*subseq2 == "--WX--");
    */

    // ------------------------------------------------------------------------
    // Test: move start backward (to the left)
    // --WX-- >>> UVWX--
    QVERIFY(subseq->setBioString("-----F"));
    QVERIFY(subseq2->setBioString("--WX--"));
    pod = msa.setSubseqStart(2, 2);
    QCOMPARE(pod.row_, 2);
    QCOMPARE(pod.columns_, ClosedIntRange(1, 2));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pod.difference_ == "UV");
    QVERIFY(*subseq2 == "UVWX--");

    /*
       !!! More tests removed because they have become irrelevant

    // ------------------------------------------------------------------------
    // Test: move start beyond left alignment border
    // UVWX-- >>> TUVWX--
    //
    // Real order of operations:
    // UVWX--  >>> -UVWX--  (insert terminal gap column)
    // -UVWX-- >>> TUVWX--  (move the start)
    pod = msa.setSubseqStart(2, 1);
    QCOMPARE(pod.row_, 2);
    QCOMPARE(pod.columns_, ClosedIntRange(1, 1));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendLeft);
    QVERIFY(pod.difference_ == "T");
    QVERIFY(*subseq  == "------F");
    QVERIFY(*subseq2 == "TUVWX--");

    // ------------------------------------------------------------------------
    // Test: moving start beyond stop in operation that also includes gaps
    // -A-B >>> ----C
    //
    // Real order of operations:
    // -A-B  >>> -A-B- (insert terminal gap)
    // -A-B- >>> -A-BC (extend stop)
    // -A-BC >>> ----C (move start)
    msa.clear();
    subseq = new Subseq(seq);
    subseq2 = 0;

    QVERIFY(subseq->setBioString("-A-B"));
    QVERIFY(msa.append(subseq));

    pod = msa.setSubseqStart(1, 3);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(2, 5));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendRightTrimLeft);
    QVERIFY(pod.difference_ == "A-B");
    QVERIFY(*subseq == "----C");

    */
}

void TestMsa::setSubseqStop()
{
    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    //                              345
    QVERIFY(subseq->setBioString( "-CDE--"));
    //                                45
    QVERIFY(subseq2->setBioString("---WX-"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    // ------------------------------------------------------------------------
    // Test: set stop to identical stop - no change
    SubseqChangePod pod = msa.setSubseqStop(1, 5);
    QVERIFY(pod.isNull());

    // ------------------------------------------------------------------------
    // Test: move stop backward 2 spots
    // -CDE-- >>> -C----
    pod = msa.setSubseqStop(1, 3);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(3, 4));
    QCOMPARE(pod.operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pod.difference_ == "DE");
    QVERIFY(*subseq == "-C----");

    /*
       !!! Updated Msa contract meant that the following tests no longer apply. See Msa introduction

    // ------------------------------------------------------------------------
    // Test: move stop beyond start
    // -C---- >>> B-----
    //
    // Behind the scenes: -C---- >>> BC---- >>> B-----
    pod = msa.setSubseqStop(1, 2);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(1, 2));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendLeftTrimRight);
    QVERIFY(pod.difference_ == "C");
    QVERIFY(*subseq == "B-----");

    // ------------------------------------------------------------------------
    // Test: move stop beyond left alignment terminus
    // B----- >>> A------
    //
    // Real order of operations:
    // B-----  >>> -B----- (insert one terminal gap column)
    // -B----- >>> AB----- (move the start - extend)
    // AB----- >>> A------ (move stop - trim)
    pod = msa.setSubseqStop(1, 1);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(1, 2));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendLeftTrimRight);
    QVERIFY(pod.difference_ == "B");
    QVERIFY(*subseq  == "A------");
    QVERIFY(*subseq2 == "----WX-");

    */

    // ------------------------------------------------------------------------
    // Test: move stop forward (to the right)
    // ----WX- >> ----WXY
    QVERIFY(subseq->setBioString("A------"));
    QVERIFY(subseq2->setBioString("----WX-"));
    pod = msa.setSubseqStop(2, 6);
    QCOMPARE(pod.row_, 2);
    QCOMPARE(pod.columns_, ClosedIntRange(7, 7));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pod.difference_ == "Y");
    QVERIFY(*subseq2 == "----WXY");

    /*
       !!! More tests removed because they have become irrelevant

    // ------------------------------------------------------------------------
    // Test: move stop beyond right alignment border
    // ----WXY >>> ----WXYZ
    //
    // Real order of operations:
    // ----WXY  >>> ----WXY-  (insert terminal gap column)
    // ----WXY- >>> ----WXYZ  (move the stop)
    pod = msa.setSubseqStop(2, 7);
    QCOMPARE(pod.row_, 2);
    QCOMPARE(pod.columns_, ClosedIntRange(8, 8));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendRight);
    QVERIFY(pod.difference_ == "Z");
    QVERIFY(*subseq  == "A-------");
    QVERIFY(*subseq2 == "----WXYZ");

    // ------------------------------------------------------------------------
    // Test: moving stop beyond start in operation that also includes gaps
    // C-D- >>> B----
    //
    // Real order of operations:
    // C-D-  >>> -C-D- (insert terminal gap)
    // -C-D- >>> BC-D- (extend start)
    // BC-D- >>> B---- (move stop)
    msa.clear();
    subseq = new Subseq(seq);
    subseq2 = 0;

    QVERIFY(subseq->setBioString("C-D-"));
    QVERIFY(msa.append(subseq));

    pod = msa.setSubseqStop(1, 2);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(1, 4));
    QCOMPARE(pod.operation_, SubseqChangePod::eExtendLeftTrimRight);
    QVERIFY(pod.difference_ == "C-D");
    QVERIFY(*subseq == "B----");

    */
}

void TestMsa::slideRect_data()
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

    QList<int> leftPositives;
    QList<int> rightPositives;
    QList<int> topPositives;
    QList<int> bottomPositives;

    leftPositives   <<  3 <<  3 <<  3 <<  4 <<  3 <<  5 <<  4 <<  6 <<  8 <<  6 <<  2 <<  1 <<  2 <<  3 <<  5 <<  3 <<  2 <<  2 <<  1 <<  3 <<  6 <<  7 <<  5;
    topPositives    <<  1 <<  1 <<  1 <<  2 <<  2 <<  1 <<  3 <<  1 <<  1 <<  3 <<  2 <<  2 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  1 <<  2 <<  2 <<  1;
    rightPositives  <<  5 <<  5 <<  4 <<  5 <<  4 <<  5 <<  4 <<  6 <<  8 <<  6 <<  2 <<  1 <<  2 <<  5 <<  7 <<  5 <<  4 <<  2 <<  1 <<  3 <<  7 <<  8 <<  5;
    bottomPositives <<  3 <<  3 <<  2 <<  3 <<  3 <<  2 <<  3 <<  2 <<  2 <<  3 <<  3 <<  3 <<  1 <<  2 <<  2 <<  2 <<  2 <<  2 <<  2 <<  2 <<  3 <<  3 <<  2;

    addSlideRectTestRows("(left, top) -> (right, bottom) ", leftPositives, topPositives, rightPositives, bottomPositives);
    addSlideRectTestRows("(right, bottom) -> (top, left) ", rightPositives, bottomPositives, leftPositives, topPositives);
    addSlideRectTestRows("(left, bottom) -> (right, top) ", leftPositives, bottomPositives, rightPositives, topPositives);
    addSlideRectTestRows("(right, top) -> (left, bottom) ", rightPositives, topPositives, leftPositives, bottomPositives);
}

void TestMsa::slideRect()
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

    Seq seq(bioString1.toAscii());
    Seq seq2(bioString2.toAscii());
    Seq seq3(bioString3.toAscii());

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);
    Subseq *subseq3 = new Subseq(seq3);

    QVERIFY(subseq->setBioString(bioString1.toAscii()));
    QVERIFY(subseq2->setBioString(bioString2.toAscii()));
    QVERIFY(subseq3->setBioString(bioString3.toAscii()));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    QCOMPARE(msa.slideRect(PosiRect(QPoint(left, top), QPoint(right, bottom)), delta), expectedDelta);
    QVERIFY(*msa.at(1) == expectedBioString1.toAscii());
    QVERIFY(*msa.at(2) == expectedBioString2.toAscii());
    QVERIFY(*msa.at(3) == expectedBioString3.toAscii());
}

// Specific test cases to make sure things are working as expected!
void TestMsa::slideRectSpecific()
{
    Seq seq("AAB");
    Seq seq2("ABX");
    Seq seq3("AX");

    Subseq *subseq1 = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);
    Subseq *subseq3 = new Subseq(seq3);

    QVERIFY(subseq1->setBioString("-AAB--"));
    QVERIFY(subseq2->setBioString("--AB-X"));
    QVERIFY(subseq3->setBioString("---A-X"));

    Msa *msa = new Msa();
    QVERIFY(msa->append(subseq1));
    QVERIFY(msa->append(subseq2));
    QVERIFY(msa->append(subseq3));

    // ------------------------------------------------------------------------
    // Test: slide region in negative direction, but limit to delta requested spots even if more are available
    QCOMPARE(msa->slideRect(PosiRect(QPoint(3, 2), QPoint(4, 3)), -1), -1);
    QVERIFY(*msa->at(1) == "-AAB--");
    QVERIFY(*msa->at(2) == "-AB--X");
    QVERIFY(*msa->at(3) == "--A--X");

    // ------------------------------------------------------------------------
    // Test: slide region in positive direction, but limit to delta requested spots even if more are available
    QCOMPARE(msa->slideRect(PosiRect(QPoint(2, 2), QPoint(3, 3)), 1), 1);
    QVERIFY(*msa->at(1) == "-AAB--");
    QVERIFY(*msa->at(2) == "--AB-X");
    QVERIFY(*msa->at(3) == "---A-X");
}

/*
bool subseqsLessThan(const Subseq *a, const Subseq *b)
{
    return *a < *b;
}

bool subseqsGreaterThan(const Subseq *a, const Subseq *b)
{
    return !subseqsLessThan(a, b);
}
*/

class SubseqLessThan : public ISubseqLessThan
{
public:
    virtual bool lessThan(const Subseq *a, const Subseq *b) const
    {
        return *a < *b;
    }
};

void TestMsa::sortAscending()
{
    Msa msa;

    // Test: already in ascending order, should not change anything
    Seq seq1("ABC");
    Seq seq2("DEF");
    Seq seq3("GHI");

    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq2);
    Subseq *subseq3 = new Subseq(seq3);

    QVERIFY(msa.append(subseq3));
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq2));

    msa.sort(SubseqLessThan(), Qt::AscendingOrder);

    QVERIFY(msa.at(1) == subseq1);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
}

void TestMsa::sortDescending()
{
    Msa msa;

    // Test: already in ascending order, should not change anything
    Seq seq1("ABC");
    Seq seq2("DEF");
    Seq seq3("GHI");

    Subseq *subseq1 = new Subseq(seq1);
    Subseq *subseq2 = new Subseq(seq2);
    Subseq *subseq3 = new Subseq(seq3);

    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq1));
    QVERIFY(msa.append(subseq3));

    msa.sort(SubseqLessThan(), Qt::DescendingOrder);

    QVERIFY(msa.at(1) == subseq3);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq1);
}

void TestMsa::swap()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("BC--DE"));

    Seq seq2("GHIJKL");
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("GH--IJ"));

    Seq seq3("ABCDEF");
    Subseq *subseq3 = new Subseq(seq3);

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));
    QVERIFY(msa.append(subseq3));

    int n = msa.subseqCount();

    // Test: swap with itself does nothing
    for (int i=-n; i <= n; ++i)
    {
        if (i == 0)
            continue;

        msa.swap(i, i);

        // Nothing should have changed
        QVERIFY(msa.at(1) == subseq);
        QVERIFY(msa.at(2) == subseq2);
        QVERIFY(msa.at(3) == subseq3);
    }

    // Test: swap positive indices
    msa.swap(1, 3);
    QVERIFY(msa.at(1) == subseq3);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq);

    msa.swap(3, 1);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);

    msa.swap(1, 2);
    QVERIFY(msa.at(1) == subseq2);
    QVERIFY(msa.at(2) == subseq);
    QVERIFY(msa.at(3) == subseq3);

    msa.swap(2, 1);
    QVERIFY(msa.at(1) == subseq);
    QVERIFY(msa.at(2) == subseq2);
    QVERIFY(msa.at(3) == subseq3);
}

void TestMsa::takeRows()
{
    Seq seq("ABCDEF");

    Msa msa;
    Subseq *subseqs[6];
    for (int i=0; i<6; ++i)
    {
        subseqs[i] = new Subseq(seq);
        QVERIFY(msa.append(subseqs[i]));
    }

    QVector<Subseq *> extract = msa.takeRows(ClosedIntRange(2, 4));

    QCOMPARE(msa.rowCount(), 3);
    QVERIFY(msa.at(1) == subseqs[0]);
    QVERIFY(msa.at(2) == subseqs[4]);
    QVERIFY(msa.at(3) == subseqs[5]);

    QCOMPARE(extract.size(), 3);
    QVERIFY(extract.at(0) == subseqs[1]);
    QVERIFY(extract.at(1) == subseqs[2]);
    QVERIFY(extract.at(2) == subseqs[3]);

    qDeleteAll(extract);
}

void TestMsa::trimLeft()
{
    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D---"));
    //                              3 4
    QVERIFY(subseq2->setBioString("-V-W-----"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    // ------------------------------------------------------------------------
    // Test: trim subseqs that do nothing
    QVERIFY(msa.trimLeft(1, ClosedIntRange(1, 2)).isEmpty());
    QVERIFY(msa.trimLeft(1, ClosedIntRange(1, 1)).isEmpty());
    QVERIFY(msa.trimLeft(1, ClosedIntRange(2, 2)).isEmpty());

    QVERIFY(*subseq == "--BC-D---");
    QVERIFY(*subseq2 == "-V-W-----");
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 3);

    QVector<SubseqChangePod> pods;

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from one sequence
    //  *
    // --BC-D---
    // -V-W-----
    pods = msa.trimLeft(2, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(2, 2));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "V");
    QVERIFY(*subseq == "--BC-D---");
    QVERIFY(*subseq2 == "---W-----");
    QCOMPARE(subseq->start(), 2);
    QCOMPARE(subseq2->start(), 4);

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from one sequence
    //   *
    // --BC-D---
    // ---W-----
    pods = msa.trimLeft(3, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(3, 3));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "B");
    QVERIFY(*subseq == "---C-D---");
    QVERIFY(*subseq2 == "---W-----");
    QCOMPARE(subseq->start(), 3);
    QCOMPARE(subseq2->start(), 4);

    // ------------------------------------------------------------------------
    // Test: trim subseqs but prevent from removing last non-gap character from second sequence
    //    *
    // ---C-D---
    // ---W-----
    pods = msa.trimLeft(4, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "C");
    QVERIFY(*subseq == "-----D---");
    QVERIFY(*subseq2 == "---W-----");
    QCOMPARE(subseq->start(), 4);
    QCOMPARE(subseq2->start(), 4);

    pods = msa.trimLeft(6, ClosedIntRange(1, 2));
    QVERIFY(pods.isEmpty());
    QVERIFY(*subseq == "-----D---");
    QVERIFY(*subseq2 == "---W-----");
    QCOMPARE(subseq->start(), 4);
    QCOMPARE(subseq2->start(), 4);

    // ------------------------------------------------------------------------
    // Test: trim subseqs all the way, should leave one character
    msa.setSubseqStart(1, 1);
    msa.setSubseqStart(2, 1);
    QVERIFY(*subseq == "--ABCD---");
    QVERIFY(*subseq2 == "TUVW-----");
    //         *
    // --ABCD---
    // TUVW-----
    pods = msa.trimLeft(9, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(3, 5));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "ABC");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(1, 3));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(1).difference_ == "TUV");
    QVERIFY(*subseq == "-----D---");
    QVERIFY(*subseq2 == "---W-----");
    QCOMPARE(subseq->start(), 4);
    QCOMPARE(subseq2->start(), 4);

    // ------------------------------------------------------------------------
    // Test: specific test
    //      *
    // --A-B-C-
    msa.clear();
    subseq2 = 0;
    subseq = new Subseq(seq);
    //                                 *
    QVERIFY(subseq->setBioString("--A-B-C-"));
    msa.append(subseq);

    pods = msa.trimLeft(6, ClosedIntRange(1, 1));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(3, 5));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pods.at(0).difference_ == "A-B");
    QCOMPARE(subseq->start(), 3);
    QVERIFY(*subseq == "------C-");
}

void TestMsa::trimLeftRowNumChars()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("--A-B--C-D-"));

    Msa msa;
    QVERIFY(msa.append(subseq));

    SubseqChangePod pod = msa.trimLeft(1, 3);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(3, 8));
    QCOMPARE(pod.operation_, SubseqChangePod::eTrimLeft);
    QVERIFY(pod.difference_ == "A-B--C");
    QCOMPARE(subseq->start(), 4);
    QVERIFY(*subseq == "---------D-");
}

void TestMsa::trimRight()
{
    Seq seq("ABCDEF");
    Seq seq2("TUVWXYZ");

    Subseq *subseq = new Subseq(seq);
    Subseq *subseq2 = new Subseq(seq2);

    //                               23 4
    QVERIFY(subseq->setBioString( "--BC-D-"));
    //                              3 45
    QVERIFY(subseq2->setBioString("-V-WX--"));

    Msa msa;
    QVERIFY(msa.append(subseq));
    QVERIFY(msa.append(subseq2));

    // ------------------------------------------------------------------------
    // Test: trim subseqs that do nothing
    QVERIFY(msa.trimRight(7, ClosedIntRange(1, 2)).isEmpty());
    QVERIFY(msa.trimRight(7, ClosedIntRange(1, 1)).isEmpty());
    QVERIFY(msa.trimRight(7, ClosedIntRange(2, 2)).isEmpty());
    QVERIFY(msa.trimRight(6, ClosedIntRange(2, 2)).isEmpty());

    QVERIFY(*subseq == "--BC-D-");
    QVERIFY(*subseq2 == "-V-WX--");
    QCOMPARE(subseq->stop(), 4);
    QCOMPARE(subseq2->stop(), 5);

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from one sequence
    //      *
    // --BC-D-
    // -V-WX--
    QVector<SubseqChangePod> pods;
    pods = msa.trimRight(6, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(6, 6));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "D");
    QVERIFY(*subseq == "--BC---");
    QVERIFY(*subseq2 == "-V-WX--");
    QCOMPARE(subseq->stop(), 3);
    QCOMPARE(subseq2->stop(), 5);

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from other sequence
    //     *
    // --BC---
    // -V-WX--
    pods = msa.trimRight(5, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 2);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(5, 5));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "X");
    QVERIFY(*subseq == "--BC---");
    QVERIFY(*subseq2 == "-V-W---");
    QCOMPARE(subseq->stop(), 3);
    QCOMPARE(subseq2->stop(), 4);

    // ------------------------------------------------------------------------
    // Test: trim subseqs one character from both sequences
    //    *
    // --BC---
    // -V-W---
    pods = msa.trimRight(4, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "C");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(4, 4));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(1).difference_ == "W");
    QVERIFY(*subseq == "--B----");
    QVERIFY(*subseq2 == "-V-----");
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    // ------------------------------------------------------------------------
    // Test: trim subseqs but prevent from removing last non-gap characters
    //   *
    // --B----
    // -V-----
    pods = msa.trimRight(3, ClosedIntRange(1, 2));
    QVERIFY(pods.isEmpty());
    QVERIFY(*subseq == "--B----");
    QVERIFY(*subseq2 == "-V-----");
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    pods = msa.trimRight(2, ClosedIntRange(1, 2));
    QVERIFY(pods.isEmpty());
    QVERIFY(*subseq == "--B----");
    QVERIFY(*subseq2 == "-V-----");
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    pods = msa.trimRight(1, ClosedIntRange(1, 2));
    QVERIFY(*subseq == "--B----");
    QVERIFY(*subseq2 == "-V-----");
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    // ------------------------------------------------------------------------
    // Test: trim subseqs all the way, should leave one character
    msa.setSubseqStop(1, 6);
    msa.setSubseqStop(2, 7);
    QVERIFY(*subseq == "--BCDEF");
    QVERIFY(*subseq2 == "-VWXYZ-");
    // *
    // --BCDEF
    // -VWXYZ-
    pods = msa.trimRight(1, ClosedIntRange(1, 2));
    QCOMPARE(pods.size(), 2);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(4, 7));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "CDEF");
    QCOMPARE(pods.at(1).row_, 2);
    QCOMPARE(pods.at(1).columns_, ClosedIntRange(3, 6));
    QCOMPARE(pods.at(1).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(1).difference_ == "WXYZ");
    QVERIFY(*subseq == "--B----");
    QVERIFY(*subseq2 == "-V-----");
    QCOMPARE(subseq->stop(), 2);
    QCOMPARE(subseq2->stop(), 3);

    // ------------------------------------------------------------------------
    // Test: specific extendOrTrim signal test
    //    *
    // --A-B-C-
    msa.clear();
    subseq2 = 0;
    subseq = new Subseq(seq);
    //                               *
    QVERIFY(subseq->setBioString("--A-B-C-"));
    QVERIFY(msa.append(subseq));

    pods = msa.trimRight(4, ClosedIntRange(1, 1));
    QCOMPARE(pods.size(), 1);
    QCOMPARE(pods.at(0).row_, 1);
    QCOMPARE(pods.at(0).columns_, ClosedIntRange(5, 7));
    QCOMPARE(pods.at(0).operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pods.at(0).difference_ == "B-C");
    QCOMPARE(subseq->start(), 1);
    QVERIFY(*subseq =="--A-----");
}

void TestMsa::trimRightRowNumChars()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("--A-B--C-D-"));

    Msa msa;
    QVERIFY(msa.append(subseq));

    SubseqChangePod pod = msa.trimRight(1, 2);
    QCOMPARE(pod.row_, 1);
    QCOMPARE(pod.columns_, ClosedIntRange(8, 10));
    QCOMPARE(pod.operation_, SubseqChangePod::eTrimRight);
    QVERIFY(pod.difference_ == "C-D");
    QCOMPARE(subseq->stop(), 2);
    QVERIFY(*subseq == "--A-B------");
}

void TestMsa::undo()
{
    Seq seq("ABCDEF");
    Subseq *subseq = new Subseq(seq);
    QVERIFY(subseq->setBioString("----CD----"));

    Msa msa;
    QVERIFY(msa.append(subseq));

    // Test: undo null SubseqChangePod
    SubseqChangePodVector pods = msa.undo(SubseqChangePodVector() << SubseqChangePod());
    QVERIFY(pods.isEmpty());

    // Test: undo extend left by 2 characters
    {
        SubseqChangePod pod = msa.extendLeft(1, 2);
        QVERIFY(*subseq == "--ABCD----");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "----CD----");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eTrimLeft);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "--ABCD----");
    }

    // Test: undo extend left with gapped biostring
    {
        QVERIFY(subseq->setBioString("----CD----"));

        SubseqChangePod pod = msa.extendLeft(1, 1, "A-B");
        QVERIFY(*subseq == "A-B-CD----");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "----CD----");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eTrimLeft);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "A-B-CD----");
    }

    // Test: undo extend right by 2 characters
    {
        QVERIFY(subseq->setBioString("----CD----"));

        SubseqChangePod pod = msa.extendRight(1, 2);
        QVERIFY(*subseq == "----CDEF--");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "----CD----");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eTrimRight);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "----CDEF--");
    }

    // Test: undo extend right with gapped biostring
    {
        QVERIFY(subseq->setBioString("----CD----"));

        SubseqChangePod pod = msa.extendRight(8, 1, "E-F");
        QVERIFY(*subseq == "----CD-E-F");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "----CD----");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eTrimRight);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "----CD-E-F");
    }

    // Test: undo trim left by 2 characters
    {
        QVERIFY(subseq->setBioString("--ABCD----"));

        SubseqChangePod pod = msa.trimLeft(1, 2);
        QVERIFY(*subseq == "----CD----");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "--ABCD----");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eExtendLeft);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "----CD----");
    }

    // Test: undo trim left by 2 characters (gapped)
    {
        QVERIFY(subseq->setBioString("-A-BCD----"));

        SubseqChangePod pod = msa.trimLeft(1, 2);
        QVERIFY(*subseq == "----CD----");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "-A-BCD----");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eExtendLeft);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "----CD----");
    }

    // Test: undo trim right by 2 characters
    {
        QVERIFY(subseq->setBioString("----CDEF--"));

        SubseqChangePod pod = msa.trimRight(1, 2);
        QVERIFY(*subseq == "----CD----");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "----CDEF--");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eExtendRight);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "----CD----");
    }

    // Test: undo trim right by 2 characters (gapped)
    {
        QVERIFY(subseq->setBioString("----CD-E--F"));

        SubseqChangePod pod = msa.trimRight(1, 2);
        QVERIFY(*subseq == "----CD-----");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "----CD-E--F");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eExtendRight);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "----CD-----");
    }

    // Test: undo collapse left
    {
        //                            1234567890
        QVERIFY(subseq->setBioString("--C-D-E--F"));

        SubseqChangePod pod = msa.collapseLeft(PosiRect(4, 1, 4, 1)).first();
        QVERIFY(*subseq == "--CDE----F");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "--C-D-E--F");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eInternal);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "--CDE----F");
    }

    // Test: undo collapse right
    {
        //                            1234567890
        QVERIFY(subseq->setBioString("--C-D-E--F"));

        SubseqChangePod pod = msa.collapseRight(PosiRect(4, 1, 5, 1)).first();
        QVERIFY(*subseq == "--C---DE-F");
        SubseqChangePod inversePod = msa.undo(SubseqChangePodVector() << pod).first();
        QVERIFY(*subseq == "--C-D-E--F");
        QCOMPARE(inversePod.operation_, SubseqChangePod::eInternal);
        QCOMPARE(msa.undo(SubseqChangePodVector() << inversePod).first(), pod);
        QVERIFY(*subseq == "--C---DE-F");
    }

    // ----------------------------------------------------
    // Special spot checks
    {
        QVERIFY(subseq->setBioString("CD-EF"));

        SubseqChangePodVector pods = msa.levelLeft(3, ClosedIntRange(1, 1));
        QVERIFY(*subseq == "--DEF");
        SubseqChangePodVector inversePods = msa.undo(pods);
        QVERIFY(*subseq == "CD-EF");

        QCOMPARE(inversePods.at(0).operation_, SubseqChangePod::eTrimLeft);
        QCOMPARE(inversePods.at(1).operation_, SubseqChangePod::eExtendLeft);

        QCOMPARE(msa.undo(inversePods), pods);
        QVERIFY(*subseq == "--DEF");
    }
}

QTEST_APPLESS_MAIN(TestMsa)
#include "TestMsa.moc"
