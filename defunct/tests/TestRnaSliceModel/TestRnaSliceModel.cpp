/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

#include <QtCore/QFile>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "../../models/AdocTreeModel.h"
#include "../../models/RnaSliceModel.h"

#include "../../exceptions/DatabaseError.h"
#include "../../exceptions/InvalidConnectionError.h"

#include "../AdocDbDataSource.h"
#include "../AdocTreeNode.h"
#include "../MpttNode.h"
#include "../MpttTreeConverter.h"

Q_DECLARE_METATYPE(QModelIndex)

class TestRnaSliceModel : public QObject
{
    Q_OBJECT

public:
    TestRnaSliceModel()
    {
        // For the signal spy to work with model indices
        qRegisterMetaType<QModelIndex>("QModelIndex");
    }

private slots:
    void loadSlice();
    void setData();

private:
    AdocTreeNode *setup();
};

AdocTreeNode *TestRnaSliceModel::setup()
{
    const QString sourceTestDbFile = "../test_databases/adr_slice.db";

    // adr_slice.db contains the following data_tree:
    //
    // Root
    // |___ Amino (Group)
    //      |___ Asubseq1
    //      |___ Asubseq2
    //      |___ Group 1 (Group)
    // |___ Dna (Group)
    //      |___ Dsubseq1
    //      |___ Dsubseq2
    //      |___ Group 2 (Group)
    // |___ Rna (Group)
    //      |___ Rsubseq1
    //      |___ Rsubseq2
    //      |___ Group 3 (Group)
    // |___ Mix (Group)
    //      |___ Asubseq3
    //      |___ Asubseq4
    //      |___ Dsubseq3
    //      |___ Rsubseq3
    //      |___ Rsubseq4
    //      |___ Rsubseq5
    //      |___ Group 4 (Group)
    // |___ NoImmediateSubseqs (Group)
    //      |___ Group 5 (Group)
    //           |___ Asubseq5
    //           |___ Dsubseq4
    //           |___ Rsubseq6
    // |___ SplitSeq (Group)
    //      |___ AminoA (Group)
    //           |___ Asubseq6-1
    //           |___ Asubseq6-3
    //      |___ AminoB (Group)
    //           |___ Asubseq6-2
    //      |___ DnaA (Group)
    //           |___ Dsubseq5-1
    //           |___ Dsubseq5-3
    //      |___ DnaB (Group)
    //           |___ Dsubseq5-2
    //      |___ RnaA (Group)
    //           |___ Rsubseq7-1
    //           |___ Rsubseq7-3
    //      |___ RnaB (Group)
    //           |___ Rsubseq7-2

    if (!QFile::exists(sourceTestDbFile))
        return 0;

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    const QString testDbFile = "adr_slice-test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);

    QFile::copy(sourceTestDbFile, testDbFile);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
    db.setDatabaseName(testDbFile);
    if (!db.open())
        return 0;

    AdocDbDataSource dbs;
    try { dbs.setConnectionName("test_db"); } catch (...) { return 0; }

    QList<MpttNode *> dataTreeRows = dbs.readDataTree(constants::kTableDataTree);

    // Must catch any InvalidMpttNodeError to properly de-allocate dataTreeRows
    AdocTreeNode *root = 0;
    try
    {
        root = static_cast<AdocTreeNode *>(MpttTreeConverter::fromMpttList(dataTreeRows));
    }
    catch (...)
    {
        foreach (MpttNode *mpttNode, dataTreeRows)
        {
            delete mpttNode->treeNode_;
            mpttNode->treeNode_ = 0;
        }
        qDeleteAll(dataTreeRows);
        dataTreeRows.clear();
    }

    return root;
}

// Currently only testing subseq amino and groups
void TestRnaSliceModel::loadSlice()
{
    AdocTreeNode *root = setup();

    AdocDbDataSource dbs;
    try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }

    AdocTreeModel treeModel;
    treeModel.setRoot(root);
    treeModel.adocDbDataSource_ = dbs;

    RnaSliceModel sliceModel;
    sliceModel.setTreeModel(&treeModel);

    try
    {
        // Now ready to begin testing the createSlice method! Whew!
        // Test: Amino folder slice
        sliceModel.loadSlice(treeModel.index(0, 0));
        QModelIndex aminoSliceIndex = sliceModel.index(0, 0);
        QCOMPARE(sliceModel.rowCount(aminoSliceIndex), 1);
        QCOMPARE(aminoSliceIndex.child(0, constants::kColumnRnaName).data().toString(), QString("Group1"));

        // Test: Dna folder
        sliceModel.loadSlice(treeModel.index(1, 0));
        QModelIndex dnaSliceIndex = sliceModel.index(1, 0);
        QCOMPARE(sliceModel.rowCount(dnaSliceIndex), 1);
        QCOMPARE(dnaSliceIndex.child(0, constants::kColumnRnaName).data().toString(), QString("Group2"));

        // Test: Rna folder
        sliceModel.loadSlice(treeModel.index(2, 0));
        QModelIndex rnaSliceIndex = sliceModel.index(2, 0);
        QCOMPARE(sliceModel.rowCount(rnaSliceIndex), 3);
        QCOMPARE(rnaSliceIndex.child(0, constants::kColumnRnaName).data().toString(), QString("Group3"));
        QCOMPARE(rnaSliceIndex.child(1, constants::kColumnRnaName).data().toString(), QString("Rsubseq1_1-60"));
        QCOMPARE(rnaSliceIndex.child(2, constants::kColumnRnaName).data().toString(), QString("Rsubseq2_1-60"));

        // Test: mix group
        sliceModel.loadSlice(treeModel.index(3, 0));
        QModelIndex mixSliceIndex = sliceModel.index(3, 0);
        QCOMPARE(sliceModel.rowCount(mixSliceIndex), 4);
        QCOMPARE(mixSliceIndex.child(0, constants::kColumnRnaName).data().toString(), QString("Group4"));
        QCOMPARE(mixSliceIndex.child(1, constants::kColumnRnaName).data().toString(), QString("Rsubseq3_1-125"));
        QCOMPARE(mixSliceIndex.child(2, constants::kColumnRnaName).data().toString(), QString("Rsubseq4_1-30"));
        QCOMPARE(mixSliceIndex.child(3, constants::kColumnRnaName).data().toString(), QString("Rsubseq5_1-124"));

        // Test: NoImmediateSubseqs
        sliceModel.loadSlice(treeModel.index(4, 0));
        QCOMPARE(sliceModel.rowCount(sliceModel.index(4, 0)), 1);
        QCOMPARE(sliceModel.index(4, 0).child(0, constants::kColumnRnaName).data().toString(), QString("Group5"));
    }
    catch (...) { QVERIFY(0); }

    QSqlDatabase::removeDatabase("test_db");
}

void TestRnaSliceModel::setData()
{
    AdocTreeNode *root = setup();

    AdocDbDataSource dbs;
    try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }

    AdocTreeModel treeModel;
    treeModel.setRoot(root);
    treeModel.adocDbDataSource_ = dbs;

    RnaSliceModel *sliceModel = new RnaSliceModel(this);
    sliceModel->setTreeModel(&treeModel);

    try
    {
        // ---------------------------------------
        // Test that setting seq data propagates the update to multiple slices/rows that
        // share the same seq

        // Setup:
        QModelIndex splitSeqIndex = treeModel.index(5, 0);
        sliceModel->loadSlice(treeModel.index(4, 0, splitSeqIndex)); // RnaA
        sliceModel->loadSlice(treeModel.index(5, 0, splitSeqIndex)); // RnaB

        QSignalSpy spyDataChanged(sliceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
        QVERIFY(spyDataChanged.isValid());
        QList<QVariant> spyArguments;

        // Test: set data on a seq level property and make sure that the signal is emitted
        //       to the other nodes containing this seq node
        QModelIndex sliceRnaA = sliceModel->index(0, 0);
        QModelIndex sliceRnaB = sliceModel->index(1, 0);
        QVERIFY(sliceModel->setData(sliceRnaA.child(0, constants::kColumnRnaSource), "E. coli"));

        // Check that the source column update was propagated to all the other seq nodes
        QCOMPARE(sliceModel->data(sliceRnaA.child(0, constants::kColumnRnaSource)).toString(), QString("E. coli"));
        QCOMPARE(sliceModel->data(sliceRnaA.child(1, constants::kColumnRnaSource)).toString(), QString("E. coli"));
        QCOMPARE(sliceModel->data(sliceRnaB.child(0, constants::kColumnRnaSource)).toString(), QString("E. coli"));

        // Check that the database was updated appropriately
        QSqlQuery query(dbs.database());
        QVERIFY(query.prepare(QString("SELECT source FROM %1 WHERE id = ?").arg(constants::kTableRnaSeqs)));
        query.bindValue(0, sliceModel->data(sliceRnaA.child(0, constants::kColumnRnaSeqId)));
        QVERIFY(query.exec());
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toString(), QString("E. coli"));

        // Check that the dataChanged signal was emitted for each of the above rows
        QCOMPARE(spyDataChanged.count(), 3);
        spyArguments = spyDataChanged.takeFirst();
        QModelIndex topLeft = qvariant_cast<QModelIndex>(spyArguments.at(0));
        QModelIndex bottomRight = qvariant_cast<QModelIndex>(spyArguments.at(1));
        QVERIFY(topLeft == bottomRight);
        QCOMPARE(topLeft.column(), constants::kColumnRnaSource);
        QCOMPARE(topLeft.parent(), sliceRnaA);
        QCOMPARE(topLeft.row(), 0);
        spyArguments = spyDataChanged.takeFirst();
        topLeft = qvariant_cast<QModelIndex>(spyArguments.at(0));
        bottomRight = qvariant_cast<QModelIndex>(spyArguments.at(1));
        QVERIFY(topLeft == bottomRight);
        QCOMPARE(topLeft.column(), constants::kColumnRnaSource);
        QCOMPARE(topLeft.parent(), sliceRnaA);
        QCOMPARE(topLeft.row(), 1);
        spyArguments = spyDataChanged.takeFirst();
        topLeft = qvariant_cast<QModelIndex>(spyArguments.at(0));
        bottomRight = qvariant_cast<QModelIndex>(spyArguments.at(1));
        QVERIFY(topLeft == bottomRight);
        QCOMPARE(topLeft.column(), constants::kColumnRnaSource);
        QCOMPARE(topLeft.parent(), sliceRnaB);
        QCOMPARE(topLeft.row(), 0);
        spyDataChanged.clear();
    }
    catch (...) { QVERIFY(0); }

    QSqlDatabase::removeDatabase("test_db");
}

QTEST_MAIN(TestRnaSliceModel)
#include "TestRnaSliceModel.moc"
