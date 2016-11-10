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

#include "../../models/AdocTreeModel.h"
#include "../../models/RelatedTableModel.h"
#include "../../models/SubseqSliceModel.h"
#include "../../models/TableModel.h"

#include "../../SynchronousAdocDataSource.h"
#include "../../AdocTreeNode.h"

#include <QtDebug>

Q_DECLARE_METATYPE(QModelIndex)

/**
  * Because SubseqSliceModel is another abstraction of the Slice model concept, it needs a concrete class to fully test its
  * implementation. Rather than create a mock class for this purpose, utilize TestSubseqSliceModel for this purpose. Thus,
  * we are testing both SubseqSliceModel and SubseqSliceModel here.
  */
class TestSubseqSliceModel : public QObject
{
    Q_OBJECT

public:
    TestSubseqSliceModel()
    {
        // For the signal spy to work with model indices
        qRegisterMetaType<QModelIndex>("QModelIndex");
        qRegisterMetaType<AdocTreeNode *>("AdocTreeNode *");
        qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
    }

private slots:
    // ------------------
    // Subseq Slice Model
    void isReady();
    void setSourceTables(); // Also tests alphabet,
    void columnCount();
    void headerData();
    void sourceFriendlyNameChangeHeaderDataSignals();     // Checks that when either an underlying subseqTable or seqTable friendly name change occurs, that the corresponding headerDataChanged signal is emitted
    void setSourceParent();     // Also tests data
    void setData();
    void clearingSourceTable();

private:
    SynchronousAdocDataSource dataSource_;
    AdocTreeNode *setup();
};

Q_DECLARE_METATYPE(Qt::Orientation);

AdocTreeNode *TestSubseqSliceModel::setup()
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

    if (dataSource_.isOpen())
        dataSource_.close();
    dataSource_.open(testDbFile);
    if (!dataSource_.isOpen())
        return 0;

    QSignalSpy spyDataTreeReady(&dataSource_, SIGNAL(dataTreeReady(AdocTreeNode *)));
    dataSource_.readDataTree();
    if (spyDataTreeReady.isEmpty())
        return 0;

    return qvariant_cast<AdocTreeNode *>(spyDataTreeReady.takeFirst().at(0));
}

void TestSubseqSliceModel::isReady()
{
    SubseqSliceModel sliceModel;
    QCOMPARE(sliceModel.isInitialized(), false);

    setup();        // To initialize dataSource_
    AdocTreeModel treeModel;

    sliceModel.setTreeModel(&treeModel);
    QCOMPARE(sliceModel.isInitialized(), false);

    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "id" << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "id" << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    aminoSubseqs->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqs, "amino_seq_id");

    sliceModel.setSourceTables(eUnknownAlphabet, aminoSubseqs, aminoSeqs);
    QCOMPARE(sliceModel.isInitialized(), false);

    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    QVERIFY(sliceModel.isInitialized());

    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, 0);
    QCOMPARE(sliceModel.isInitialized(), false);

    sliceModel.setSourceTables(eAminoAlphabet, 0, aminoSeqs);
    QCOMPARE(sliceModel.isInitialized(), false);

    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    QVERIFY(sliceModel.isInitialized());

    sliceModel.setTreeModel(0);
    QCOMPARE(sliceModel.isInitialized(), false);
}

void TestSubseqSliceModel::setSourceTables()
{
    TableModel *aminoSeqs = new TableModel(this);
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);

    SubseqSliceModel sliceModel;
    QSignalSpy spyModelReset(&sliceModel, SIGNAL(modelReset()));

    // ------------------------------------------------------------------------
    // Test: default alphabet should be unknown
    QCOMPARE(sliceModel.alphabet(), eUnknownAlphabet);

    // ------------------------------------------------------------------------
    // Test: setSourceTables should define the alphabet regardless of the other parameters
    sliceModel.setSourceTables(eDnaAlphabet, 0, 0);
    QCOMPARE(sliceModel.alphabet(), eDnaAlphabet);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();

    sliceModel.setSourceTables(eRnaAlphabet, aminoSubseqs, 0);
    QCOMPARE(sliceModel.alphabet(), eRnaAlphabet);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();

    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    QCOMPARE(sliceModel.alphabet(), eAminoAlphabet);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();

    sliceModel.setSourceTables(eUnknownAlphabet, 0, aminoSeqs);
    QCOMPARE(sliceModel.alphabet(), eUnknownAlphabet);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();

    // ------------------------------------------------------------------------
    // Test: signals are hooked up properly
    SubseqSliceModel sliceModel2;
    sliceModel2.setSourceTables(eUnknownAlphabet, aminoSubseqs, aminoSeqs);
    QVERIFY(QObject::disconnect(aminoSubseqs, SIGNAL(loadDone(int)), &sliceModel2, SLOT(subseqsLoadDone(int))));
    QVERIFY(QObject::disconnect(aminoSubseqs, SIGNAL(loadError(QString,int)), &sliceModel2, SLOT(subseqsLoadError(QString,int))));

    QVERIFY(QObject::disconnect(aminoSubseqs, SIGNAL(friendlyFieldNamesChanged(int,int)), &sliceModel2, SLOT(subseqFriendlyFieldNamesChanged(int,int))));
    QVERIFY(QObject::disconnect(aminoSubseqs, SIGNAL(modelReset()), &sliceModel2, SLOT(refreshSourceParent())));
    QVERIFY(QObject::disconnect(aminoSubseqs, SIGNAL(dataChanged(int,int)), &sliceModel2, SLOT(subseqDataChanged(int,int))));
    QVERIFY(QObject::disconnect(aminoSeqs, SIGNAL(friendlyFieldNamesChanged(int,int)), &sliceModel2, SLOT(seqFriendlyFieldNamesChanged(int,int))));
    QVERIFY(QObject::disconnect(aminoSeqs, SIGNAL(modelReset()), &sliceModel2, SLOT(refreshSourceParent())));
    QVERIFY(QObject::disconnect(aminoSeqs, SIGNAL(dataChanged(int,int)), &sliceModel2, SLOT(seqDataChanged(int,int))));

    SubseqSliceModel sliceModel3;
    QSignalSpy spyModelReset3(&sliceModel3, SIGNAL(modelReset()));
    sliceModel3.setSourceTables(eUnknownAlphabet, aminoSubseqs, aminoSeqs);
    RelatedTableModel *aminoSubseqs2 = new RelatedTableModel(this);
    RelatedTableModel *aminoSeqs2 = new RelatedTableModel(this);
    sliceModel3.setSourceTables(eAminoAlphabet, aminoSubseqs2, aminoSeqs2);
    QCOMPARE(spyModelReset3.count(), 2);
    spyModelReset3.clear();

    // Should not be able to disconnect from aminoSubseqs/aminoSeqs - this should be done in the setSourceTables method
    QCOMPARE(QObject::disconnect(aminoSubseqs, SIGNAL(loadDone(int)), &sliceModel3, SLOT(subseqsLoadDone(int))), false);
    QCOMPARE(QObject::disconnect(aminoSubseqs, SIGNAL(loadError(QString,int)), &sliceModel3, SLOT(subseqsLoadError(QString,int))), false);

    QCOMPARE(QObject::disconnect(aminoSubseqs, SIGNAL(friendlyFieldNamesChanged(int,int)), &sliceModel3, SLOT(subseqFriendlyFieldNamesChanged(int,int))), false);
    QCOMPARE(QObject::disconnect(aminoSubseqs, SIGNAL(modelReset()), &sliceModel3, SLOT(refreshSourceParent())), false);
    QCOMPARE(QObject::disconnect(aminoSubseqs, SIGNAL(dataChanged(int,int)), &sliceModel3, SLOT(subseqDataChanged(int,int))), false);
    QCOMPARE(QObject::disconnect(aminoSeqs, SIGNAL(friendlyFieldNamesChanged(int,int)), &sliceModel3, SLOT(seqFriendlyFieldNamesChanged(int,int))), false);
    QCOMPARE(QObject::disconnect(aminoSeqs, SIGNAL(modelReset()), &sliceModel3, SLOT(refreshSourceParent())), false);
    QCOMPARE(QObject::disconnect(aminoSeqs, SIGNAL(dataChanged(int,int)), &sliceModel3, SLOT(subseqDataChanged(int,int))), false);

    // However, this is not true for aminoSubseqs2
    QVERIFY(QObject::disconnect(aminoSubseqs2, SIGNAL(loadDone(int)), &sliceModel3, SLOT(subseqsLoadDone(int))));
    QVERIFY(QObject::disconnect(aminoSubseqs2, SIGNAL(loadError(QString,int)), &sliceModel3, SLOT(subseqsLoadError(QString,int))));

    QVERIFY(QObject::disconnect(aminoSubseqs2, SIGNAL(friendlyFieldNamesChanged(int,int)), &sliceModel3, SLOT(subseqFriendlyFieldNamesChanged(int,int))));
    QVERIFY(QObject::disconnect(aminoSubseqs2, SIGNAL(modelReset()), &sliceModel3, SLOT(refreshSourceParent())));
    QVERIFY(QObject::disconnect(aminoSubseqs2, SIGNAL(dataChanged(int,int)), &sliceModel3, SLOT(subseqDataChanged(int,int))));
    QVERIFY(QObject::disconnect(aminoSeqs2, SIGNAL(friendlyFieldNamesChanged(int,int)), &sliceModel3, SLOT(seqFriendlyFieldNamesChanged(int,int))));
    QVERIFY(QObject::disconnect(aminoSeqs2, SIGNAL(modelReset()), &sliceModel3, SLOT(refreshSourceParent())));
    QVERIFY(QObject::disconnect(aminoSeqs2, SIGNAL(dataChanged(int,int)), &sliceModel3, SLOT(seqDataChanged(int,int))));

    // Test: model should be reset/cleared even with exact same parameters
    sliceModel3.setSourceTables(eAminoAlphabet, aminoSubseqs2, 0);
    QCOMPARE(spyModelReset3.count(), 1);
    spyModelReset3.clear();
}

void TestSubseqSliceModel::columnCount()
{
    SubseqSliceModel sliceModel;
    QCOMPARE(sliceModel.columnCount(), 0);

    TableModel *aminoSeqs = new TableModel(this);
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);

    // ------------------------------------------------------------------------
    // Test: after setting fields with invalid subseq and seq tables, should still be zero
    sliceModel.setSourceTables(eAminoAlphabet, 0, 0);
    QCOMPARE(sliceModel.columnCount(), 0);

    // ------------------------------------------------------------------------
    // Test: other various situations that should still return zero
    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, 0);
    QCOMPARE(sliceModel.columnCount(), 0);

    sliceModel.setSourceTables(eAminoAlphabet, 0, aminoSeqs);
    QCOMPARE(sliceModel.columnCount(), 0);

    // This one still returns zero because aminoSeqs and aminoSubseqs have not been configured with any fields
    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    QCOMPARE(sliceModel.columnCount(), 0);

    // ------------------------------------------------------------------------
    // Test: same as above except with defined fields for the above tables
    aminoSeqs->setSource(0, "", QStringList() << "name" << "source");
    aminoSubseqs->setSource(0, "amino_subseqs", QStringList() << "id" << "amino_seq_id" << "start" << "stop");

    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, 0);
    QCOMPARE(sliceModel.columnCount(), 0);

    sliceModel.setSourceTables(eAminoAlphabet, 0, aminoSeqs);
    QCOMPARE(sliceModel.columnCount(), 0);

    // This one still returns zero because aminoSeqs and aminoSubseqs have not been configured with any fields
    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    QCOMPARE(sliceModel.columnCount(), 7);

    // ------------------------------------------------------------------------
    // Test: Setting with invalid condition goes back to zero
    sliceModel.setSourceTables(eAminoAlphabet, 0, aminoSeqs);
    QCOMPARE(sliceModel.columnCount(), 0);

    // ------------------------------------------------------------------------
    // Test: Alphabet does not matter
    sliceModel.setSourceTables(eDnaAlphabet, aminoSubseqs, aminoSeqs);
    QCOMPARE(sliceModel.columnCount(), 7);

    sliceModel.setSourceTables(eRnaAlphabet, aminoSubseqs, aminoSeqs);
    QCOMPARE(sliceModel.columnCount(), 7);

    sliceModel.setSourceTables(eUnknownAlphabet, aminoSubseqs, aminoSeqs);
    QCOMPARE(sliceModel.columnCount(), 7);
}

void TestSubseqSliceModel::headerData()
{
    setup();

    SubseqSliceModel x;
    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");

    // ------------------------------------------------------------------------
    // Test: default state
    for (int i=-3; i< 15; ++i)
        QCOMPARE(x.headerData(i, Qt::Horizontal).isValid(), false);

    // ------------------------------------------------------------------------
    // Test: with one invalid source table
    x.setSourceTables(eAminoAlphabet, aminoSubseqs, 0);
    for (int i=-3; i< 15; ++i)
        QCOMPARE(x.headerData(i, Qt::Horizontal).isValid(), false);

    x.setSourceTables(eAminoAlphabet, 0, aminoSeqs);
    for (int i=-3; i< 15; ++i)
        QCOMPARE(x.headerData(i, Qt::Horizontal).isValid(), false);

    // ------------------------------------------------------------------------
    // Test: with both valid tables
    x.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    for (int i=0; i< aminoSubseqs->columnCount(); ++i)
        QCOMPARE(x.headerData(i, Qt::Horizontal).toString(), QString(aminoSubseqs->fields().at(i)));

    for (int i=0; i< aminoSeqs->columnCount(); ++i)
        QCOMPARE(x.headerData(aminoSubseqs->columnCount() + i, Qt::Horizontal).toString(), QString(aminoSeqs->fields().at(i)));
}

void TestSubseqSliceModel::sourceFriendlyNameChangeHeaderDataSignals()
{
    setup();

    SubseqSliceModel x;
    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    x.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);

    QSignalSpy spyHeaderDataChanged(&x, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: change friendly names of subseq table
    aminoSubseqs->setFriendlyFieldNames(QStringList() << "Subseq ID" << "Amino Seq ID");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 1);

    // ------------------------------------------------------------------------
    // Test: have more friendly names than columns
    aminoSubseqs->setFriendlyFieldNames(QStringList() << "Subseq ID" << "Amino Seq ID" << "Label" << "Start" << "Stop" << "Sequence" << "Notes" << "Created" << "Junk!");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 7);    // Note, this should not be 8!

    // ------------------------------------------------------------------------
    // Test: Back to one friendly name change
    aminoSubseqs->setFriendlyFieldNames(QStringList() << "Subseq ID");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);

    // ------------------------------------------------------------------------
    // Test: change friendly names of seq table; note that the column indicated should be 8, and not zero because
    //       all seq columns occur after all subseq columns
    aminoSeqs->setFriendlyFieldNames(QStringList() << "Seq ID" << "Astring ID");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 8);
    QCOMPARE(spyArguments.at(2).toInt(), 9);

    // ------------------------------------------------------------------------
    // Test: have more friendly names than columns
    aminoSeqs->setFriendlyFieldNames(QStringList() << "Seq ID" << "Astring ID" << "Label" << "Source" << "Protein" << "Created" << "Junk");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 8);
    QCOMPARE(spyArguments.at(2).toInt(), 13);    // Note, this should not be 14!

    // ------------------------------------------------------------------------
    // Test: Back to one friendly name change
    aminoSeqs->setFriendlyFieldNames(QStringList() << "Subseq ID");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 8);
    QCOMPARE(spyArguments.at(2).toInt(), 8);
}

// Currently only testing subseq amino and groups
void TestSubseqSliceModel::setSourceParent()
{
    AdocTreeNode *root = setup();
    QVERIFY(root);

    AdocTreeModel treeModel;
    treeModel.setRoot(root);

    SubseqSliceModel sliceModel;
    sliceModel.setTreeModel(&treeModel);

    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    QVERIFY(aminoSubseqs->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqs, "amino_seq_id", true));
    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);

    sliceModel.setPrimaryColumn(2);

    // ------------------------------------------------------------------------
    // Now ready to begin testing the createSlice method! Whew!
    // Test: Amino folder slice
    sliceModel.setSourceParent(treeModel.index(0, 0));

    QCOMPARE(sliceModel.rowCount(), 3);
    // Group should be first and only contain non-null data for the primary column
    for (int i=0; i< sliceModel.columnCount(); ++i)
    {
        if (i != sliceModel.primaryColumn())
            QVERIFY2(sliceModel.index(0, i).data().isNull(), QString("I: %1").arg(i).toAscii());
        else
            QCOMPARE(sliceModel.index(0, sliceModel.primaryColumn()).data().toString(), QString("Group1"));
    }

    // Check first amino subseq
    QCOMPARE(sliceModel.index(1, 0).data().toInt(), 1);                             // id
    QCOMPARE(sliceModel.index(1, 1).data().toInt(), 1);                             // amino_seq_id
    QCOMPARE(sliceModel.index(1, 2).data().toString(), QString("Asubseq1_1-60"));   // label
    QCOMPARE(sliceModel.index(1, 3).data().toInt(), 1);                             // start
    QCOMPARE(sliceModel.index(1, 4).data().toInt(), 60);                            // stop
    QCOMPARE(sliceModel.index(1, 5).data().toString(), QString("MHTSELLKHIYDINLSYLLLAQRLIVQDKASAMFRLGINEEMATTLAALTLPQMVKLAET"));    // sequence
    QCOMPARE(sliceModel.index(1, 6).data().toString(), QString());                  // notes
    QCOMPARE(sliceModel.index(1, 7).data().toString(), QString("2010-08-27 15:45:25"));  // created
    // Now check seq properties
    QCOMPARE(sliceModel.index(1, 8).data().toInt(), 1);                             // id
    QCOMPARE(sliceModel.index(1, 9).data().toInt(), 1);                             // astring_id
    QCOMPARE(sliceModel.index(1, 10).data().toString(), QString("Asubseq1"));       // label
    QCOMPARE(sliceModel.index(1, 11).data().toString(), QString());                 // source
    QCOMPARE(sliceModel.index(1, 12).data().toString(), QString());                 // protein
    QCOMPARE(sliceModel.index(1, 13).data().toString(), QString("2010-08-27 15:45:25")); // created

    // Check second amino subseq
    QCOMPARE(sliceModel.index(2, 0).data().toInt(), 2);                             // id
    QCOMPARE(sliceModel.index(2, 1).data().toInt(), 2);                             // amino_seq_id
    QCOMPARE(sliceModel.index(2, 2).data().toString(), QString("Asubseq2_1-60"));   // label
    QCOMPARE(sliceModel.index(2, 3).data().toInt(), 1);                             // start
    QCOMPARE(sliceModel.index(2, 4).data().toInt(), 60);                            // stop
    QCOMPARE(sliceModel.index(2, 5).data().toString(), QString("MSEKSIVQEARDIQLAMELITLGARLQMLESETQLSRGRLIKLYKELRGSPPPKGMLPFS"));    // sequence
    QCOMPARE(sliceModel.index(2, 6).data().toString(), QString());                  // notes
    QCOMPARE(sliceModel.index(2, 7).data().toString(), QString("2010-08-27 15:45:25"));  // created
    // Now check seq properties
    QCOMPARE(sliceModel.index(2, 8).data().toInt(), 2);                             // id
    QCOMPARE(sliceModel.index(2, 9).data().toInt(), 2);                             // astring_id
    QCOMPARE(sliceModel.index(2, 10).data().toString(), QString("Asubseq2"));       // label
    QCOMPARE(sliceModel.index(2, 11).data().toString(), QString());                 // source
    QCOMPARE(sliceModel.index(2, 12).data().toString(), QString());                 // protein
    QCOMPARE(sliceModel.index(2, 13).data().toString(), QString("2010-08-27 15:45:25")); // created


    // ----------------
    // Test: Dna folder
    sliceModel.setSourceParent(treeModel.index(1, 0));
    QCOMPARE(sliceModel.rowCount(), 1);
    // Group should be first and only contain non-null data for the primary column
    for (int i=0; i< sliceModel.columnCount(); ++i)
    {
        if (i != sliceModel.primaryColumn())
            QVERIFY2(sliceModel.index(0, i).data().isNull(), QString("I: %1").arg(i).toAscii());
        else
            QCOMPARE(sliceModel.index(0, sliceModel.primaryColumn()).data().toString(), QString("Group2"));
    }

    // ----------------
    // Test: Rna folder
    sliceModel.setSourceParent(treeModel.index(2, 0));
    QCOMPARE(sliceModel.rowCount(), 1);
    // Group should be first and only contain non-null data for the primary column
    for (int i=0; i< sliceModel.columnCount(); ++i)
    {
        if (i != sliceModel.primaryColumn())
            QVERIFY2(sliceModel.index(0, i).data().isNull(), QString("I: %1").arg(i).toAscii());
        else
            QCOMPARE(sliceModel.index(0, sliceModel.primaryColumn()).data().toString(), QString("Group3"));
    }

    // ----------------
    // Test: mix group
    sliceModel.setSourceParent(treeModel.index(3, 0));
    QCOMPARE(sliceModel.rowCount(), 3);
    // Group should be first and only contain non-null data for the primary column
    for (int i=0; i< sliceModel.columnCount(); ++i)
    {
        if (i != sliceModel.primaryColumn())
            QVERIFY2(sliceModel.index(0, i).data().isNull(), QString("I: %1").arg(i).toAscii());
        else
            QCOMPARE(sliceModel.index(0, sliceModel.primaryColumn()).data().toString(), QString("Group4"));
    }

    // Check first amino subseq
    QCOMPARE(sliceModel.index(1, 0).data().toInt(), 3);                             // id
    QCOMPARE(sliceModel.index(1, 1).data().toInt(), 3);                             // amino_seq_id
    QCOMPARE(sliceModel.index(1, 2).data().toString(), QString("Asubseq3_1-120"));  // label
    QCOMPARE(sliceModel.index(1, 3).data().toInt(), 1);                             // start
    QCOMPARE(sliceModel.index(1, 4).data().toInt(), 120);                           // stop
    QCOMPARE(sliceModel.index(1, 5).data().toString(), QString("MLILLGYLVVLGTVFGGYLMTGGSLGALYQPAELVIIAGAGIGSFIVGNNGKAIKGTLKALPLLFRRSKYTKAMYMDLLALLYRLMAKSRQMGMFSLERDIENPRESEIFASYPRILADS"));    // sequence
    QCOMPARE(sliceModel.index(1, 6).data().toString(), QString());                  // notes
    QCOMPARE(sliceModel.index(1, 7).data().toString(), QString("2010-08-27 15:46:31"));  // created
    // Now check seq properties
    QCOMPARE(sliceModel.index(1, 8).data().toInt(), 3);                             // id
    QCOMPARE(sliceModel.index(1, 9).data().toInt(), 3);                             // astring_id
    QCOMPARE(sliceModel.index(1, 10).data().toString(), QString("Asubseq3"));       // label
    QCOMPARE(sliceModel.index(1, 11).data().toString(), QString());                 // source
    QCOMPARE(sliceModel.index(1, 12).data().toString(), QString());                 // protein
    QCOMPARE(sliceModel.index(1, 13).data().toString(), QString("2010-08-27 15:46:31")); // created

    // Check second amino subseq
    QCOMPARE(sliceModel.index(2, 0).data().toInt(), 4);                             // id
    QCOMPARE(sliceModel.index(2, 1).data().toInt(), 4);                             // amino_seq_id
    QCOMPARE(sliceModel.index(2, 2).data().toString(), QString("Asubseq4_1-120"));  // label
    QCOMPARE(sliceModel.index(2, 3).data().toInt(), 1);                             // start
    QCOMPARE(sliceModel.index(2, 4).data().toInt(), 120);                            // stop
    QCOMPARE(sliceModel.index(2, 5).data().toString(), QString("MKNQAHPIIVVKRRKAKSHGAAHGSWKIAYADFMTAMMAFFLVMWLISISSPKELIQIAEYFRTPLATAVTGGDRISNSESPIPGGGDDYTQSQGEVNKQPNIEELKKRMEQSRLRKLRG"));    // sequence
    QCOMPARE(sliceModel.index(2, 6).data().toString(), QString());                  // notes
    QCOMPARE(sliceModel.index(2, 7).data().toString(), QString("2010-08-27 15:46:31"));  // created
    // Now check seq properties
    QCOMPARE(sliceModel.index(2, 8).data().toInt(), 4);                             // id
    QCOMPARE(sliceModel.index(2, 9).data().toInt(), 4);                             // astring_id
    QCOMPARE(sliceModel.index(2, 10).data().toString(), QString("Asubseq4"));       // label
    QCOMPARE(sliceModel.index(2, 11).data().toString(), QString());                 // source
    QCOMPARE(sliceModel.index(2, 12).data().toString(), QString());                 // protein
    QCOMPARE(sliceModel.index(2, 13).data().toString(), QString("2010-08-27 15:46:31")); // created

    // Test: NoImmediateSubseqs
    sliceModel.setSourceParent(treeModel.index(4, 0));
    QCOMPARE(sliceModel.rowCount(), 1);
    // Group should be first and only contain non-null data for the primary column
    for (int i=0; i< sliceModel.columnCount(); ++i)
    {
        if (i != sliceModel.primaryColumn())
            QVERIFY2(sliceModel.index(0, i).data().isNull(), QString("I: %1").arg(i).toAscii());
        else
            QCOMPARE(sliceModel.index(0, sliceModel.primaryColumn()).data().toString(), QString("Group5"));
    }
}

void TestSubseqSliceModel::setData()
{
    AdocTreeNode *root = setup();
    QVERIFY(root);

    AdocTreeModel treeModel;
    treeModel.setRoot(root);

    SubseqSliceModel sliceModel;
    sliceModel.setTreeModel(&treeModel);

    QSignalSpy spyDataChanged(&sliceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    QVariantList spyArguments;

    // Test: uninitialized model
    QCOMPARE(sliceModel.setData(QModelIndex(), "newValue"), false);
    QCOMPARE(sliceModel.setData(sliceModel.index(1, 0), "newValue"), false);
    QCOMPARE(sliceModel.setData(sliceModel.index(0, 1), "newValue", Qt::DisplayRole), false);

    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    QVERIFY(aminoSubseqs->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqs, "amino_seq_id", true));
    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    sliceModel.setPrimaryColumn(2);

    // ------------------------------------------------------------------------
    // Test: no parent has been selected and therefore all calls to index should return false, which
    //       in turn should make all setData calls return false
    QCOMPARE(sliceModel.rowCount(), 0);
    QCOMPARE(sliceModel.setData(QModelIndex(), "newValue"), false);
    QCOMPARE(sliceModel.setData(sliceModel.index(1, 0), "newValue"), false);
    QCOMPARE(sliceModel.setData(sliceModel.index(0, 1), "newValue", Qt::DisplayRole), false);

    // Test: rows loaded, but invalid roles and a mix of valid and invalid rows/columns
    sliceModel.setSourceParent(treeModel.index(0, 0));
    QVERIFY(sliceModel.rowCount() > 0);
    QList<int> testRoles;
    testRoles << Qt::DisplayRole << Qt::FontRole << Qt::DecorationRole;
    foreach (int role, testRoles)
        for (int i=0; i< 5; ++i)
            for (int j=-5; j< 15; ++j)
                QCOMPARE(sliceModel.setData(sliceModel.index(i, j), "newValue", role), false);

    // ------------------------------------------------------------------------
    // The next two tests are probably unneeded because the model will not contain any rows and thus
    // no valid indices will ever be created. Regardless for thoroughness, I have left them.
    //
    // Test: Leave out the seq table
    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, 0);
    sliceModel.setSourceParent(treeModel.index(0, 0));
    foreach (int role, testRoles)
        for (int i=0; i< 5; ++i)
            for (int j=-5; j< 15; ++j)
                QCOMPARE(sliceModel.setData(sliceModel.index(i, j), "newValue", role), false);

    // Test: Leave out the subseq table
    sliceModel.setSourceTables(eAminoAlphabet, 0, aminoSeqs);
    sliceModel.setSourceParent(treeModel.index(0, 0));
    foreach (int role, testRoles)
        for (int i=0; i< 5; ++i)
            for (int j=-5; j< 15; ++j)
                QCOMPARE(sliceModel.setData(sliceModel.index(i, j), "newValue", role), false);
    // ------------------------------------------------------------------------


    // ------------------------------------------------------------------------
    // Test: Valid setup, only primary column of group rows should be able to be modified
    //
    // Rows are as follows:
    // [0] -> Group1
    // [1] -> Asubseq1
    // [2] -> Asubseq2
    QVERIFY(spyDataChanged.isEmpty());
    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    sliceModel.setSourceParent(treeModel.index(0, 0));
    QCOMPARE(sliceModel.rowCount(), 3);
    QCOMPARE(sliceModel.index(0, sliceModel.primaryColumn()).data().toString(), QString("Group1"));
    for (int i=0; i< sliceModel.columnCount(); ++i)
        if (i != sliceModel.primaryColumn())
            QCOMPARE(sliceModel.setData(sliceModel.index(0, i), "newValue"), false);
    QVERIFY(spyDataChanged.isEmpty());

    // Test: update the group name, should return true and dataChanged signal should be emitted for this index
    QModelIndex groupPrimaryIndex = sliceModel.index(0, sliceModel.primaryColumn());
    QCOMPARE(sliceModel.setData(groupPrimaryIndex, "New group name"), true);
    QCOMPARE(groupPrimaryIndex.data().toString(), QString("New group name"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), groupPrimaryIndex);
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(1)), groupPrimaryIndex);

    // Test: change the subseq id - should fail
    QCOMPARE(sliceModel.setData(sliceModel.index(1, 0), 100), false);
    QCOMPARE(sliceModel.index(1, 0).data().toInt(), 1);

    // Test: change seq id - should fail
    QCOMPARE(sliceModel.setData(sliceModel.index(1, 8), 100), false);
    QCOMPARE(sliceModel.index(1, 8).data().toInt(), 1);
    QVERIFY(spyDataChanged.isEmpty());

    // Test: change some subseq fields should work and emit the proper update signals
    QModelIndex subseqASequenceIndex = sliceModel.index(1, 5);
    QVERIFY(sliceModel.setData(subseqASequenceIndex, "ABC"));
    QCOMPARE(subseqASequenceIndex.data().toString(), QString("ABC"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), subseqASequenceIndex);
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(1)), subseqASequenceIndex);

    QModelIndex subseqAStopIndex = sliceModel.index(1, 4);
    QVERIFY(sliceModel.setData(subseqAStopIndex, 30));
    QCOMPARE(subseqAStopIndex.data().toInt(), 30);
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), subseqAStopIndex);
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(1)), subseqAStopIndex);

    // Test: change some seq fields of second subseq
    QModelIndex seqBSourceIndex = sliceModel.index(2, 11);
    QVERIFY(sliceModel.setData(seqBSourceIndex, "Azotobacter"));
    QCOMPARE(seqBSourceIndex.data().toString(), QString("Azotobacter"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), seqBSourceIndex);
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(1)), seqBSourceIndex);

    QModelIndex seqBAstringIndex = sliceModel.index(2, 9);
    QVERIFY(sliceModel.setData(seqBAstringIndex, 999));
    QCOMPARE(seqBAstringIndex.data().toInt(), 999);
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), seqBAstringIndex);
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(1)), seqBAstringIndex);
}

// When one of the source table models comprising the SubseqSliceModel is reset, all records
// contained in the SubseqSliceModel should be cleared. This test checks that this happens.
void TestSubseqSliceModel::clearingSourceTable()
{
    AdocTreeNode *root = setup();
    QVERIFY(root);

    AdocTreeModel treeModel;
    treeModel.setRoot(root);

    SubseqSliceModel sliceModel;
    sliceModel.setTreeModel(&treeModel);

    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    QVERIFY(aminoSubseqs->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqs, "amino_seq_id", true));
    sliceModel.setSourceTables(eAminoAlphabet, aminoSubseqs, aminoSeqs);
    sliceModel.setPrimaryColumn(2);

    sliceModel.setSourceParent(treeModel.index(0, 0));  // Loading some records into the slice model

    // Make sure they have been loaded properly
    QCOMPARE(sliceModel.rowCount(), 3);

    aminoSubseqs->clear();

    QSignalSpy spyModelReset(&sliceModel, SIGNAL(modelReset()));



}

QTEST_MAIN(TestSubseqSliceModel)
#include "TestSubseqSliceModel.moc"
