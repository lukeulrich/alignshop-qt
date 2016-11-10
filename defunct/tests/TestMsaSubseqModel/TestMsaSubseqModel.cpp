/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include <QtSql/QSqlField>
#include <QtSql/QSqlRecord>

#include "DataRow.h"
#include "SynchronousAdocDataSource.h"
#include "models/RelatedTableModel.h"
#include "models/TableModel.h"
#include "models/RelatedTableModel.h"
#include "models/MsaSubseqModel.h"

class TestMsaSubseqModel : public QObject
{
    Q_OBJECT

public:
    TestMsaSubseqModel(QObject *parent = 0) : QObject(parent)
    {
        qRegisterMetaType<Qt::Orientation>("Qt::Orientation");
        qRegisterMetaType<QModelIndex>("QModelIndex");
    }
    ~TestMsaSubseqModel()
    {
        QFile::remove("test.db");
    }

private slots:
    void setMsa();                  // Also tests msa
    void setAnnotationTables();     // Also tests subseqTable and seqTable
    void setAnnotationTables_ColumnSignals();
    void sourceFriendlyNameChangeHeaderDataSignals();     // Checks that when either an underlying subseqTable or seqTable friendly name change occurs, that the corresponding headerDataChanged signal is emitted

    void isReady();
    void columnCount();
    void rowCount();
    void headerData();
    void data();
    void dataChanged();
    void annotationTableReset();        // Tests that when the subseq or seq table is reset, that the data is preserved

    // Msa changes should update model state appropriately
    void msaSwapSubseqs();
    void msaMoveSubseqs();

private:
    void setupDataSource();
    Msa *createTestMsa(Alphabet alphabet, int id = 0);

    SynchronousAdocDataSource dataSource_;
};

Q_DECLARE_METATYPE(Qt::Orientation);
Q_DECLARE_METATYPE(QModelIndex);

void TestMsaSubseqModel::setupDataSource()
{
    const QString sourceTestDbFile = "../test_databases/synchronous_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    const QString testDbFile = "test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);

    QFile::copy(sourceTestDbFile, testDbFile);

    if (dataSource_.isOpen())
        dataSource_.close();
    dataSource_.open(testDbFile);
    QVERIFY(dataSource_.isOpen());
}

// Produces the following test MSA
// 2    BC--DE
// 3    GH-IJK
// 4    --CD--
Msa *TestMsaSubseqModel::createTestMsa(Alphabet alphabet, int id)
{
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 2);
    if (!subseq->setBioString("BC--DE"))
        return 0;

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, 3);
    if (!subseq2->setBioString("GH-IJK"))
        return 0;

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3, 4);
    if (!subseq3->setBioString("--CD--"))
        return 0;

    AnonSeq anonSeq4(5, "AASEDY");
    Subseq *subseq4 = new Subseq(anonSeq4, 5);
    if (!subseq4->setBioString("--SEDY"))
        return 0;

    Msa *msa = new Msa(alphabet, id, this);
    msa->append(subseq);
    msa->append(subseq2);
    msa->append(subseq3);
    msa->append(subseq4);

    return msa;
}

void TestMsaSubseqModel::setMsa()
{
    MsaSubseqModel x;

    QVERIFY(x.msa() == 0);

    QSignalSpy spyModelReset(&x, SIGNAL(modelReset()));
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    Msa *msa = new Msa();
    x.setMsa(msa);
    QVERIFY(x.msa() == msa);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();

    // ------------------------------------------------------------------------
    // Signals should have been hooked up successfully
    // Signal -> signal
    QVERIFY(disconnect(msa, SIGNAL(msaReset()), &x, SIGNAL(modelReset())));
    QVERIFY(disconnect(msa, SIGNAL(subseqsAboutToBeSorted()), &x, SIGNAL(layoutAboutToBeChanged())));
    QVERIFY(disconnect(msa, SIGNAL(subseqsSorted()), &x, SIGNAL(layoutChanged())));

    // Signal -> slot
    QVERIFY(disconnect(msa, SIGNAL(regionSlid(int,int,int,int,int,int,int)), &x, SLOT(onMsaRegionSlid(int,int,int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqAboutToBeSwapped(int,int)), &x, SLOT(onMsaSubseqAboutToBeSwapped(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsAboutToBeInserted(int,int)), &x, SLOT(onMsaSubseqsAboutToBeInserted(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsAboutToBeMoved(int,int,int)), &x, SLOT(onMsaSubseqsAboutToBeMoved(int,int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsAboutToBeRemoved(int,int)), &x, SLOT(onMsaSubseqsAboutToBeRemoved(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsInserted(int,int)), &x, SLOT(onMsaSubseqsInserted(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsMoved(int,int,int)), &x, SLOT(onMsaSubseqsMoved(int,int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqsRemoved(int,int)), &x, SLOT(onMsaSubseqsRemoved(int,int))));
    QVERIFY(disconnect(msa, SIGNAL(subseqSwapped(int,int)), &x, SLOT(onMsaSubseqSwapped(int,int))));

    x.setMsa(0);
    QVERIFY(x.msa() == 0);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelReset.clear();

    delete msa;
    msa = 0;

    // ------------------------------------------------------------------------
    // Test: signals should be disconnected when msa is unset
    Msa *msa2 = new Msa();
    x.setMsa(msa2);
    QVERIFY(x.msa() == msa2);
    x.setMsa(0);

    QCOMPARE(spyModelReset.count(), 2);
    spyModelReset.clear();

    // Signal -> signal
    QCOMPARE(disconnect(msa2, SIGNAL(msaReset()), &x, SIGNAL(modelReset())), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqsAboutToBeSorted()), &x, SIGNAL(layoutAboutToBeChanged())), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqsSorted()), &x, SIGNAL(layoutChanged())), false);

    // Signal -> slot
    QCOMPARE(disconnect(msa2, SIGNAL(regionSlid(int,int,int,int,int,int,int)), &x, SLOT(onMsaRegionSlid(int,int,int,int))), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqAboutToBeSwapped(int,int)), &x, SLOT(onMsaSubseqAboutToBeSwapped(int,int))), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqsAboutToBeInserted(int,int)), &x, SLOT(onMsaSubseqsAboutToBeInserted(int,int))), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqsAboutToBeMoved(int,int,int)), &x, SLOT(onMsaSubseqsAboutToBeMoved(int,int,int))), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqsAboutToBeRemoved(int,int)), &x, SLOT(onMsaSubseqsAboutToBeRemoved(int,int))), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqsInserted(int,int)), &x, SLOT(onMsaSubseqsInserted(int,int))), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqsMoved(int,int,int)), &x, SLOT(onMsaSubseqsMoved(int,int,int))), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqsRemoved(int,int)), &x, SLOT(onMsaSubseqsRemoved(int,int))), false);
    QCOMPARE(disconnect(msa2, SIGNAL(subseqSwapped(int,int)), &x, SLOT(onMsaSubseqSwapped(int,int))), false);

    delete msa2;
    msa2 = 0;

    // ------------------------------------------------------------------------
    // Test: signals should be connected/disconnected properly when moving from one msa to another valid
    //       msa pointer
    Msa *msa3 = new Msa();
    Msa *msa4 = new Msa();
    x.setMsa(msa3);
    QVERIFY(x.msa() == msa3);
    x.setMsa(msa4);
    QVERIFY(x.msa() == msa4);

    // Check that signals were successfully disconnected from msa3
    // Signal -> signal
    QCOMPARE(disconnect(msa3, SIGNAL(msaReset()), &x, SIGNAL(modelReset())), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqsAboutToBeSorted()), &x, SIGNAL(layoutAboutToBeChanged())), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqsSorted()), &x, SIGNAL(layoutChanged())), false);

    // Signal -> slot
    QCOMPARE(disconnect(msa3, SIGNAL(regionSlid(int,int,int,int,int,int,int)), &x, SLOT(onMsaRegionSlid(int,int,int,int))), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqAboutToBeSwapped(int,int)), &x, SLOT(onMsaSubseqAboutToBeSwapped(int,int))), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqsAboutToBeInserted(int,int)), &x, SLOT(onMsaSubseqsAboutToBeInserted(int,int))), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqsAboutToBeMoved(int,int,int)), &x, SLOT(onMsaSubseqsAboutToBeMoved(int,int,int))), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqsAboutToBeRemoved(int,int)), &x, SLOT(onMsaSubseqsAboutToBeRemoved(int,int))), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqsInserted(int,int)), &x, SLOT(onMsaSubseqsInserted(int,int))), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqsMoved(int,int,int)), &x, SLOT(onMsaSubseqsMoved(int,int,int))), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqsRemoved(int,int)), &x, SLOT(onMsaSubseqsRemoved(int,int))), false);
    QCOMPARE(disconnect(msa3, SIGNAL(subseqSwapped(int,int)), &x, SLOT(onMsaSubseqSwapped(int,int))), false);

    // Signal -> signal
    QVERIFY(disconnect(msa4, SIGNAL(msaReset()), &x, SIGNAL(modelReset())));
    QVERIFY(disconnect(msa4, SIGNAL(subseqsAboutToBeSorted()), &x, SIGNAL(layoutAboutToBeChanged())));
    QVERIFY(disconnect(msa4, SIGNAL(subseqsSorted()), &x, SIGNAL(layoutChanged())));

    // Signal -> slot
    QVERIFY(disconnect(msa4, SIGNAL(regionSlid(int,int,int,int,int,int,int)), &x, SLOT(onMsaRegionSlid(int,int,int,int))));
    QVERIFY(disconnect(msa4, SIGNAL(subseqAboutToBeSwapped(int,int)), &x, SLOT(onMsaSubseqAboutToBeSwapped(int,int))));
    QVERIFY(disconnect(msa4, SIGNAL(subseqsAboutToBeInserted(int,int)), &x, SLOT(onMsaSubseqsAboutToBeInserted(int,int))));
    QVERIFY(disconnect(msa4, SIGNAL(subseqsAboutToBeMoved(int,int,int)), &x, SLOT(onMsaSubseqsAboutToBeMoved(int,int,int))));
    QVERIFY(disconnect(msa4, SIGNAL(subseqsAboutToBeRemoved(int,int)), &x, SLOT(onMsaSubseqsAboutToBeRemoved(int,int))));
    QVERIFY(disconnect(msa4, SIGNAL(subseqsInserted(int,int)), &x, SLOT(onMsaSubseqsInserted(int,int))));
    QVERIFY(disconnect(msa4, SIGNAL(subseqsMoved(int,int,int)), &x, SLOT(onMsaSubseqsMoved(int,int,int))));
    QVERIFY(disconnect(msa4, SIGNAL(subseqsRemoved(int,int)), &x, SLOT(onMsaSubseqsRemoved(int,int))));
    QVERIFY(disconnect(msa4, SIGNAL(subseqSwapped(int,int)), &x, SLOT(onMsaSubseqSwapped(int,int))));

    x.setMsa(0);

    delete msa3;
    msa3 = 0;
    delete msa4;
    msa4 = 0;

    // ------------------------------------------------------------------------
    // Test: dataChanged signals - all of the above should not have triggered a dataChanged
    //       signal because there were no rows in the MSA and no annotation tables were configured
    QVERIFY(spyDataChanged.isEmpty());

    // Test: setMsa with msa that contains some sequences, should not emit the dataChanged signal
    //       because no annotation tables were configured
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 2);
    QVERIFY(subseq->setBioString("BC--DE"));

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, 3);
    QVERIFY(subseq2->setBioString("GH-IJK"));

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3, 4);
    QVERIFY(subseq3->setBioString("--CD--"));

    Msa *msa5 = new Msa(eUnknownAlphabet, 1, this);
    QVERIFY(msa5->append(subseq));
    QVERIFY(msa5->append(subseq2));
    QVERIFY(msa5->append(subseq3));

    x.setMsa(msa5);
    QVERIFY(spyDataChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Test: msa with annotation tables

    // First clear out the previous msa
    x.setMsa(0);

    // Initialize the data source
    setupDataSource();

    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    QVERIFY(aminoSubseqs->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqs, "amino_seq_id", true));
    aminoSubseqs->load(QList<int>() << 1 << 2 << 3 << 4);
    QCOMPARE(aminoSubseqs->rowCount(), 4);
    QCOMPARE(aminoSeqs->rowCount(), 4);

    x.setAnnotationTables(aminoSubseqs, aminoSeqs);

    x.setMsa(msa5);

    QCOMPARE(spyDataChanged.count(), 1);
    QVariantList spyArguments = spyDataChanged.takeFirst();
    QModelIndex spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.row(), 0);
    QCOMPARE(spyIndex.column(), 3);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(1));
    QCOMPARE(spyIndex.row(), x.rowCount()-1);
    QCOMPARE(spyIndex.column(), x.columnCount()-1);
}

void TestMsaSubseqModel::setAnnotationTables()
{
    MsaSubseqModel x;

    // Test: default state should have null pointers
    QVERIFY(x.seqTable() == 0);
    QVERIFY(x.subseqTable() == 0);

    // Setup
    TableModel *seqTable = new TableModel(this);
    RelatedTableModel *subseqTable = new RelatedTableModel(this);

    // Since none of the above have any columns, no signals should be emitted for them
    QSignalSpy spyColumnsAboutToBeRemoved(&x, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(QModelIndex,int,int)));
    QSignalSpy spyColumnsAboutToBeInserted(&x, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy spyColumnsInserted(&x, SIGNAL(columnsInserted(QModelIndex,int,int)));
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(QModelIndex,QModelIndex)));

    // ------------------------------------------------------------------------
    // Test: set only the subseq table
    x.setAnnotationTables(subseqTable, 0);
    QVERIFY(x.subseqTable() == subseqTable);
    QVERIFY(x.seqTable() == 0);

    // Check that signals were connected appropriately (by attempting to disconnect them)
    QCOMPARE(disconnect(subseqTable, SIGNAL(modelReset()), &x, SLOT(subseqTableReset())), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(dataChanged(int,int)), &x, SLOT(subseqDataChanged(int,int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(friendlyFieldNamesChanged(int,int)), &x, SLOT(subseqFriendlyFieldNamesChanged(int,int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadDone(int)), &x, SLOT(subseqsLoadDone(int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadError(QString,int)), &x, SLOT(subseqsLoadError(QString,int))), true);


    // ------------------------------------------------------------------------
    // Setup
    x.setAnnotationTables(subseqTable, 0);

    // Test: set only the seq table; should reset subseqTable_ to zero and detach from signals
    x.setAnnotationTables(0, seqTable);
    QVERIFY(x.subseqTable() == 0);
    QVERIFY(x.seqTable() == 0);

    QCOMPARE(disconnect(subseqTable, SIGNAL(modelReset()), &x, SLOT(subseqTableReset())), false);
    QCOMPARE(disconnect(subseqTable, SIGNAL(dataChanged(int,int)), &x, SLOT(subseqDataChanged(int,int))), false);
    QCOMPARE(disconnect(subseqTable, SIGNAL(friendlyFieldNamesChanged(int,int)), &x, SLOT(subseqFriendlyFieldNamesChanged(int,int))), false);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadDone(int)), &x, SLOT(subseqsLoadDone(int))), false);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadError(QString,int)), &x, SLOT(subseqsLoadError(QString,int))), false);
    QCOMPARE(disconnect(seqTable, SIGNAL(loadDone(int)), &x, SLOT(seqsOnlyLoadDone(int))), false);
    QCOMPARE(disconnect(seqTable, SIGNAL(loadError(QString,int)), &x, SLOT(seqsOnlyLoadError(QString,int))), false);


    // ------------------------------------------------------------------------
    // Test: set both the subseq and seq table
    x.setAnnotationTables(subseqTable, seqTable);
    QVERIFY(x.subseqTable() == subseqTable);
    QVERIFY(x.seqTable() == seqTable);

    // Check that signals were connected appropriately (by attempting to disconnect them)
    QCOMPARE(disconnect(subseqTable, SIGNAL(modelReset()), &x, SLOT(subseqTableReset())), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(dataChanged(int,int)), &x, SLOT(subseqDataChanged(int,int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(friendlyFieldNamesChanged(int,int)), &x, SLOT(subseqFriendlyFieldNamesChanged(int,int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadDone(int)), &x, SLOT(subseqsLoadDone(int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadError(QString,int)), &x, SLOT(subseqsLoadError(QString,int))), true);
    QCOMPARE(disconnect(seqTable, SIGNAL(modelReset()), &x, SLOT(seqTableReset())), true);
    QCOMPARE(disconnect(seqTable, SIGNAL(dataChanged(int,int)), &x, SLOT(seqDataChanged(int,int))), true);
    QCOMPARE(disconnect(seqTable, SIGNAL(friendlyFieldNamesChanged(int,int)), &x, SLOT(seqFriendlyFieldNamesChanged(int,int))), true);
    QCOMPARE(disconnect(seqTable, SIGNAL(loadDone(int)), &x, SLOT(seqsOnlyLoadDone(int))), true);
    QCOMPARE(disconnect(seqTable, SIGNAL(loadError(QString,int)), &x, SLOT(seqsOnlyLoadError(QString,int))), true);


    // ------------------------------------------------------------------------
    // Setup
    x.setAnnotationTables(subseqTable, seqTable);

    // Test: clear the tables
    x.setAnnotationTables(0, 0);
    QVERIFY(x.subseqTable() == 0);
    QVERIFY(x.seqTable() == 0);

    QCOMPARE(disconnect(subseqTable, SIGNAL(modelReset()), &x, SLOT(subseqTableReset())), false);
    QCOMPARE(disconnect(subseqTable, SIGNAL(dataChanged(int,int)), &x, SLOT(subseqDataChanged(int,int))), false);
    QCOMPARE(disconnect(subseqTable, SIGNAL(friendlyFieldNamesChanged(int,int)), &x, SLOT(subseqFriendlyFieldNamesChanged(int,int))), false);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadDone(int)), &x, SLOT(subseqsLoadDone(int))), false);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadError(QString,int)), &x, SLOT(subseqsLoadError(QString,int))), false);
    QCOMPARE(disconnect(seqTable, SIGNAL(modelReset()), &x, SLOT(seqTableReset())), false);
    QCOMPARE(disconnect(seqTable, SIGNAL(dataChanged(int,int)), &x, SLOT(seqDataChanged(int,int))), false);
    QCOMPARE(disconnect(seqTable, SIGNAL(friendlyFieldNamesChanged(int,int)), &x, SLOT(subseqFriendlyFieldNamesChanged(int,int))), false);
    QCOMPARE(disconnect(seqTable, SIGNAL(loadDone(int)), &x, SLOT(seqsOnlyLoadDone(int))), false);
    QCOMPARE(disconnect(seqTable, SIGNAL(loadError(QString,int)), &x, SLOT(seqsOnlyLoadError(QString,int))), false);

    // ------------------------------------------------------------------------
    // Test: 1 parameter version
    x.setAnnotationTables(subseqTable);
    QVERIFY(x.subseqTable() == subseqTable);
    QVERIFY(x.seqTable() == 0);
    QCOMPARE(disconnect(subseqTable, SIGNAL(modelReset()), &x, SLOT(subseqTableReset())), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(dataChanged(int,int)), &x, SLOT(subseqDataChanged(int,int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(friendlyFieldNamesChanged(int,int)), &x, SLOT(subseqFriendlyFieldNamesChanged(int,int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadDone(int)), &x, SLOT(subseqsLoadDone(int))), true);
    QCOMPARE(disconnect(subseqTable, SIGNAL(loadError(QString,int)), &x, SLOT(subseqsLoadError(QString,int))), true);

    // ------------------------------------------------------------------------
    // Test: 1 parameter version should clear all tables
    x.setAnnotationTables(subseqTable, seqTable);
    x.setAnnotationTables(0);
    QVERIFY(x.subseqTable() == 0);
    QVERIFY(x.seqTable() == 0);

    QVERIFY(spyColumnsAboutToBeRemoved.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QVERIFY(spyColumnsAboutToBeInserted.isEmpty());
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyDataChanged.isEmpty());
}

// This method is similar to theh above, except it tests that the signals relating
// to the insertion/removal of columns associated with each subseq/seq annotation table
// are emitted properly.
void TestMsaSubseqModel::setAnnotationTables_ColumnSignals()
{
    setupDataSource();

    MsaSubseqModel x;

    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 2);
    QVERIFY(subseq->setBioString("BC--DE"));

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, 3);
    QVERIFY(subseq2->setBioString("GH-IJK"));

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3, 4);
    QVERIFY(subseq3->setBioString("--CD--"));

    Msa *msa = new Msa(eUnknownAlphabet, 1, this);
    QVERIFY(msa->append(subseq));
    QVERIFY(msa->append(subseq2));
    QVERIFY(msa->append(subseq3));

    TableModel *seqTable = new TableModel(this);
    seqTable->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "id" << "source");
    RelatedTableModel *subseqTable = new RelatedTableModel(this);
    subseqTable->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "id" << "notes" << "amino_seq_id");
    QVERIFY(subseqTable->setRelation(RelatedTableModel::eRelationBelongsTo, seqTable, "amino_seq_id", true));
    subseqTable->load(QList<int>() << 1 << 2 << 3 << 4);
    QCOMPARE(subseqTable->rowCount(), 4);
    QCOMPARE(seqTable->rowCount(), 4);

    x.setMsa(msa);

    // Since none of the above have any columns, no signals should be emitted for them
    QSignalSpy spyColumnsAboutToBeRemoved(&x, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)));
    QSignalSpy spyColumnsRemoved(&x, SIGNAL(columnsRemoved(QModelIndex,int,int)));
    QSignalSpy spyColumnsAboutToBeInserted(&x, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)));
    QSignalSpy spyColumnsInserted(&x, SIGNAL(columnsInserted(QModelIndex,int,int)));
    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    QVariantList spyArguments;
    QModelIndex spyIndex;

    // ------------------------------------------------------------------------
    // Test: subseqTable
    x.setAnnotationTables(subseqTable, 0);

    // Because there was no previous subseqTable, only should have columns inserted
    QVERIFY(spyColumnsAboutToBeRemoved.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QCOMPARE(spyColumnsAboutToBeInserted.count(), 1);
    QCOMPARE(spyColumnsInserted.count(), 1);
    spyArguments = spyColumnsAboutToBeInserted.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 5);
    spyArguments = spyColumnsInserted.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 5);
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.row(), 0);
    QCOMPARE(spyIndex.column(), 3);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(1));
    QCOMPARE(spyIndex.row(), 2);
    QCOMPARE(spyIndex.column(), 5);

    // ------------------------------------------------------------------------
    // Test: set both the subseq and seq table
    QVERIFY(spyColumnsAboutToBeRemoved.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QVERIFY(spyColumnsAboutToBeInserted.isEmpty());
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyDataChanged.isEmpty());
    x.setAnnotationTables(subseqTable, seqTable);

    // Because there were already is a subseqTable set, should have both columnsInserted and Removed
    QCOMPARE(spyColumnsAboutToBeRemoved.count(), 1);
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsAboutToBeRemoved.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 5);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 5);

    QCOMPARE(spyColumnsAboutToBeInserted.count(), 1);
    QCOMPARE(spyColumnsInserted.count(), 1);
    spyArguments = spyColumnsAboutToBeInserted.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 7);
    spyArguments = spyColumnsInserted.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 7);

    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.row(), 0);
    QCOMPARE(spyIndex.column(), 3);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(1));
    QCOMPARE(spyIndex.row(), 2);
    QCOMPARE(spyIndex.column(), 7);

    // ------------------------------------------------------------------------
    // Test: clearing the tables should solely remove columns
    QVERIFY(spyColumnsAboutToBeRemoved.isEmpty());
    QVERIFY(spyColumnsRemoved.isEmpty());
    QVERIFY(spyColumnsAboutToBeInserted.isEmpty());
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyDataChanged.isEmpty());
    x.setAnnotationTables(0);

    QVERIFY(spyColumnsAboutToBeInserted.isEmpty());
    QVERIFY(spyColumnsInserted.isEmpty());
    QVERIFY(spyDataChanged.isEmpty());

    QCOMPARE(spyColumnsAboutToBeRemoved.count(), 1);
    QCOMPARE(spyColumnsRemoved.count(), 1);
    spyArguments = spyColumnsAboutToBeRemoved.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 7);
    spyArguments = spyColumnsRemoved.takeFirst();
    QCOMPARE(qvariant_cast<QModelIndex>(spyArguments.at(0)), QModelIndex());
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 7);
}

void TestMsaSubseqModel::sourceFriendlyNameChangeHeaderDataSignals()
{
    MsaSubseqModel x;

    // Test: default state should have null pointers
    QVERIFY(x.seqTable() == 0);
    QVERIFY(x.subseqTable() == 0);

    // Setup
    TableModel *seqTable = new TableModel(this);
    RelatedTableModel *subseqTable = new RelatedTableModel(this);

    seqTable->setSource(0, "", QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    subseqTable->setSource(0, "", QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    x.setAnnotationTables(subseqTable, seqTable);

    QSignalSpy spyHeaderDataChanged(&x, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: change friendly names of subseq table
    subseqTable->setFriendlyFieldNames(QStringList() << "Subseq ID" << "Amino Seq ID");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 4);

    // ------------------------------------------------------------------------
    // Test: have more friendly names than columns
    subseqTable->setFriendlyFieldNames(QStringList() << "Subseq ID" << "Amino Seq ID" << "Label" << "Start" << "Stop" << "Sequence" << "Notes" << "Created" << "Junk!");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 10);    // Note, this should not be 11!

    // ------------------------------------------------------------------------
    // Test: Back to one friendly name change
    subseqTable->setFriendlyFieldNames(QStringList() << "Subseq ID");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toInt(), 3);

    // ------------------------------------------------------------------------
    // Test: change friendly names of seq table; note that the column indicated should be 8, and not zero because
    //       all seq columns occur after all subseq columns
    seqTable->setFriendlyFieldNames(QStringList() << "Seq ID" << "Astring ID");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 11);
    QCOMPARE(spyArguments.at(2).toInt(), 12);

    // ------------------------------------------------------------------------
    // Test: have more friendly names than columns
    seqTable->setFriendlyFieldNames(QStringList() << "Seq ID" << "Astring ID" << "Label" << "Source" << "Protein" << "Created" << "Junk");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 11);
    QCOMPARE(spyArguments.at(2).toInt(), 16);    // Note, this should not be 17!

    // ------------------------------------------------------------------------
    // Test: Back to one friendly name change
    seqTable->setFriendlyFieldNames(QStringList() << "Subseq ID");
    QCOMPARE(spyHeaderDataChanged.count(), 1);
    spyArguments = spyHeaderDataChanged.takeFirst();
    QCOMPARE(qvariant_cast<Qt::Orientation>(spyArguments.at(0)), Qt::Horizontal);
    QCOMPARE(spyArguments.at(1).toInt(), 11);
    QCOMPARE(spyArguments.at(2).toInt(), 11);
}

void TestMsaSubseqModel::isReady()
{
    MsaSubseqModel x;

    // Test: default is not ready
    QCOMPARE(x.isReady(), false);

    // Test: A non-zero msa
    Msa *msa = new Msa(eAminoAlphabet, 1, this);
    x.setMsa(msa);
    QCOMPARE(x.isReady(), true);

    x.setMsa(0);
    QCOMPARE(x.isReady(), false);
}

void TestMsaSubseqModel::columnCount()
{
    MsaSubseqModel x;

    // ------------------------------------------------------------------------
    // Test: uninitialized model
    QCOMPARE(x.columnCount(), 3);

    // ------------------------------------------------------------------------
    // Test: with msa should have three columns - id, start, stop
    Msa *msa = new Msa(eAminoAlphabet, 1, this);
    x.setMsa(msa);
    QCOMPARE(x.columnCount(), 3);
    x.setMsa(0);
    QCOMPARE(x.columnCount(), 3);

    // ------------------------------------------------------------------------
    // Test: Seq and subseq tables without any defined fields
    TableModel *seqTable = new TableModel(this);
    RelatedTableModel *subseqTable = new RelatedTableModel(this);

    x.setAnnotationTables(subseqTable, seqTable);
    QCOMPARE(x.columnCount(), 3);
    x.setAnnotationTables(0);

    // ------------------------------------------------------------------------
    // Test: change the number of fields
    subseqTable->setSource(0, QString(), QStringList() << "id" << "name");
    x.setAnnotationTables(subseqTable);
    QCOMPARE(x.columnCount(), 5);

    seqTable->setSource(0, QString(), QStringList() << "id" << "source");
    x.setAnnotationTables(0, seqTable);
    QCOMPARE(x.columnCount(), 3);
    x.setAnnotationTables(subseqTable, seqTable);
    QCOMPARE(x.columnCount(), 7);

    // ------------------------------------------------------------------------
    // Test: change the number of fields dynamically
    subseqTable->setSource(0, QString(), QStringList() << "id" << "start" << "stop");
    QCOMPARE(x.columnCount(), 8);
    seqTable->setSource(0, QString(), QStringList() << "id");
    QCOMPARE(x.columnCount(), 7);
}

void TestMsaSubseqModel::rowCount()
{
    MsaSubseqModel x;

    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq);
    subseq->setBioString("BC--DE");

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2);
    subseq2->setBioString("GH--JK");

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3);
    subseq3->setBioString("--CD");

    Msa *msa = new Msa(eUnknownAlphabet, 1, this);

    // ------------------------------------------------------------------------
    // Test: Default number of rows is zero because no MSA is loaded
    QCOMPARE(x.rowCount(), 0);

    // ------------------------------------------------------------------------
    // Test: setMsa to valid pointer
    x.setMsa(msa);
    QCOMPARE(x.rowCount(), 0);

    // ------------------------------------------------------------------------
    // Test: append some subseqs to msa and recheck the rowcount
    QVERIFY(msa->append(subseq));
    QVERIFY(msa->append(subseq2));

    QCOMPARE(x.rowCount(), 2);

    msa->clear();

    QCOMPARE(x.rowCount(), 0);
}

void TestMsaSubseqModel::headerData()
{
    MsaSubseqModel x;

    // ------------------------------------------------------------------------
    // Test: header data for roles other than DisplayRole
    QVERIFY(x.headerData(0, Qt::Horizontal, Qt::EditRole).isValid() == false);
    QVERIFY(x.headerData(0, Qt::Vertical, Qt::EditRole).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: header data for core columns and core + 1 when no subseqtable has been defined
    for (int i=0; i< 3; ++i)
        QCOMPARE(x.headerData(i, Qt::Horizontal).toString(), QString(MsaSubseqModel::friendlyCoreFieldNames_.at(i)));

    QVERIFY(x.headerData(x.columnCount() + 1, Qt::Horizontal).isNull());

    // ------------------------------------------------------------------------
    // Test: subseqTable with columns
    RelatedTableModel *subseqTable = new RelatedTableModel(this);
    subseqTable->setSource(0, "", QStringList() << "id" << "parent");
    x.setAnnotationTables(subseqTable);

    QCOMPARE(x.columnCount(), 5);
    for (int i=0; i< 3; ++i)
        QCOMPARE(x.headerData(i, Qt::Horizontal).toString(), QString(MsaSubseqModel::friendlyCoreFieldNames_.at(i)));

    QCOMPARE(x.headerData(3, Qt::Horizontal).toString(), QString("id"));
    QCOMPARE(x.headerData(4).toString(), QString("parent"));
    QVERIFY(x.headerData(5).isNull());

    // ------------------------------------------------------------------------
    // Test: friendly names of Table Model should come through
    subseqTable->setFriendlyFieldNames(QStringList() << "My ID" << "Parent");
    QCOMPARE(x.headerData(3, Qt::Horizontal).toString(), QString("My ID"));
    QCOMPARE(x.headerData(4).toString(), QString("Parent"));

    // ------------------------------------------------------------------------
    // Test: seq table
    TableModel *seqTable = new TableModel(this);
    seqTable->setSource(0, "", QStringList() << "id" << "source" << "astring_id");
    x.setAnnotationTables(subseqTable, seqTable);

    QCOMPARE(x.columnCount(), 8);
    for (int i=0; i< 3; ++i)
        QCOMPARE(x.headerData(i, Qt::Horizontal).toString(), QString(MsaSubseqModel::friendlyCoreFieldNames_.at(i)));

    QCOMPARE(x.headerData(3, Qt::Horizontal).toString(), QString("My ID"));
    QCOMPARE(x.headerData(4).toString(), QString("Parent"));
    QCOMPARE(x.headerData(5, Qt::Horizontal).toString(), QString("id"));
    QCOMPARE(x.headerData(6).toString(), QString("source"));
    QCOMPARE(x.headerData(7).toString(), QString("astring_id"));
    QVERIFY(x.headerData(8).isNull());

    // ------------------------------------------------------------------------
    // Test: seq table friendly names
    seqTable->setFriendlyFieldNames(QStringList() << "Seq ID" << "The source" << "Astring ID");
    QCOMPARE(x.headerData(5, Qt::Horizontal).toString(), QString("Seq ID"));
    QCOMPARE(x.headerData(6).toString(), QString("The source"));
    QCOMPARE(x.headerData(7).toString(), QString("Astring ID"));

    // ------------------------------------------------------------------------
    // Test: vertical header data
    for (int i=-3; i< 5; ++i)
        QCOMPARE(x.headerData(i, Qt::Vertical).toInt(), i+1);
}

void TestMsaSubseqModel::data()
{
    setupDataSource();

    MsaSubseqModel x;

    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 2);
    QVERIFY(subseq->setBioString("BC--DE"));

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, 3);
    QVERIFY(subseq2->setBioString("GH-IJK"));

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3, 4);
    QVERIFY(subseq3->setBioString("--CD--"));

    Msa *msa = new Msa(eUnknownAlphabet, 1, this);
    QVERIFY(msa->append(subseq));
    QVERIFY(msa->append(subseq2));
    QVERIFY(msa->append(subseq3));

    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    QVERIFY(aminoSubseqs->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqs, "amino_seq_id", true));
    aminoSubseqs->load(QList<int>() << 1 << 2 << 3 << 4);
    QCOMPARE(aminoSubseqs->rowCount(), 4);
    QCOMPARE(aminoSeqs->rowCount(), 4);

    // ------------------------------------------------------------------------
    // Test: invalid QModelIndex should return invalid data
    QVERIFY(x.data(QModelIndex()).isNull());

    // ------------------------------------------------------------------------
    // Test: uninitialized model should not have any data
    for (int i=0; i< x.columnCount(); ++i)
        for (int j=0; j< 5; ++j)
            QVERIFY(x.index(j, i).data().isNull());

    // ------------------------------------------------------------------------
    // Test: add msa; data for core columns should return as expected
    x.setMsa(msa);

    QCOMPARE(x.index(0, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 2);
    QCOMPARE(x.index(1, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 3);
    QCOMPARE(x.index(2, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 4);

    QCOMPARE(x.index(0, MsaSubseqModel::kCoreSubseqStartColumn_).data().toInt(), 2);
    QCOMPARE(x.index(1, MsaSubseqModel::kCoreSubseqStartColumn_).data().toInt(), 1);
    QCOMPARE(x.index(2, MsaSubseqModel::kCoreSubseqStartColumn_).data().toInt(), 1);

    QCOMPARE(x.index(0, MsaSubseqModel::kCoreSubseqStopColumn_).data().toInt(), 5);
    QCOMPARE(x.index(1, MsaSubseqModel::kCoreSubseqStopColumn_).data().toInt(), 5);
    QCOMPARE(x.index(2, MsaSubseqModel::kCoreSubseqStopColumn_).data().toInt(), 2);

    QVERIFY(x.index(0, MsaSubseqModel::kCoreSubseqStopColumn_ + 1).data().isNull());
    QVERIFY(x.index(1, MsaSubseqModel::kCoreSubseqStopColumn_ + 2).data().isNull());
    QVERIFY(x.index(2, MsaSubseqModel::kCoreSubseqStopColumn_ + 3).data().isNull());
    QVERIFY(x.index(3, MsaSubseqModel::kCoreSubseqIdColumn_).data().isNull());

    // ------------------------------------------------------------------------
    // Test: Associated annotation data (subseq)
    //
    // Column | Field
    // 3        id
    // 4        amino_seq_id
    // 5        label
    // 6        start
    // 7        stop
    // 8        sequence
    // 9        notes
    // 10       created
    x.setAnnotationTables(aminoSubseqs);

    // column 3 = id
    QCOMPARE(x.index(0, 3).data().toInt(), 2);
    QCOMPARE(x.index(1, 3).data().toInt(), 3);
    QCOMPARE(x.index(2, 3).data().toInt(), 4);

    // column 4 = amino_seq_id
    QCOMPARE(x.index(0, 4).data().toInt(), 2);
    QCOMPARE(x.index(1, 4).data().toInt(), 3);
    QCOMPARE(x.index(2, 4).data().toInt(), 4);

    // column 5 = label
    QCOMPARE(x.index(0, 5).data().toString(), QString("bll2760_152-270_1-119"));
    QCOMPARE(x.index(1, 5).data().toString(), QString("SMa1229_128-245_1-118"));
    QCOMPARE(x.index(2, 5).data().toString(), QString("E4_20-133_1-114"));

    // column 6 = start
    QCOMPARE(x.index(0, 6).data().toInt(), 1);
    QCOMPARE(x.index(1, 6).data().toInt(), 1);
    QCOMPARE(x.index(2, 6).data().toInt(), 1);

    // column 7 = stop
    QCOMPARE(x.index(0, 7).data().toInt(), 119);
    QCOMPARE(x.index(1, 7).data().toInt(), 118);
    QCOMPARE(x.index(2, 7).data().toInt(), 114);

    // column 8 = sequence
    QCOMPARE(x.index(0, 8).data().toString(), QString("------------------IPDAMIVIDGHGIIQLFSTAAERLFGWSELEAIGQNVNILMPEPDRSRHDSYISRYRTTSDPHIIGIGRIVTGKRRDGTTFPMHLSIGEMQSGGEPYFTGFVRDLTEHQQTQARLQELQ"));
    QCOMPARE(x.index(1, 8).data().toString(), QString("TEDVVRARDAHLRSILDTVPDATVVSATDGTIVSFNAAAVRQFGYAEEEVIGQNLRILMPEPYRHEHDGYLQRYMATGEKRIIGIDRVVSGQRKDGSTFPMKLAVGEMRSGGERFFTG-------------------"));
    QCOMPARE(x.index(2, 8).data().toString(), QString("--------GIFFPALEQNMMGAVLINEND-EVMFFNPAAEKLWGYKREEVIGNNIDMLIPRDLRPAHPEYIRHNREGGKARVEGMSRELQLEKKDGSKIWTRFALSKVSAEGKVYYLALVRDA--------------"));

    // column 9 = notes
    QVERIFY(x.index(0, 9).data().isNull());
    QVERIFY(x.index(1, 9).data().isNull());
    QVERIFY(x.index(2, 9).data().isNull());

    // column 10 = created
    QCOMPARE(x.index(0, 10).data().toString(), QString("2010-09-23 14:19:02"));
    QCOMPARE(x.index(1, 10).data().toString(), QString("2010-09-23 14:19:02"));
    QCOMPARE(x.index(2, 10).data().toString(), QString("2010-09-23 14:19:02"));

    // column 11 - does not exist
    QVERIFY(x.index(0, 11).data().isNull());

    // ------------------------------------------------------------------------
    // Test: Associated annotation data (subseq and seq)
    //
    // Column | Field
    // 3        id
    // 4        amino_seq_id
    // 5        label
    // 6        start
    // 7        stop
    // 8        sequence
    // 9        notes
    // 10       created
    //
    // 11       id
    // 12       astring_id
    // 13       label
    // 14       source
    // 15       protein
    // 16       created
    x.setAnnotationTables(aminoSubseqs, aminoSeqs);

    // column 3 = id
    QCOMPARE(x.index(0, 3).data().toInt(), 2);
    QCOMPARE(x.index(1, 3).data().toInt(), 3);
    QCOMPARE(x.index(2, 3).data().toInt(), 4);

    // column 4 = amino_seq_id
    QCOMPARE(x.index(0, 4).data().toInt(), 2);
    QCOMPARE(x.index(1, 4).data().toInt(), 3);
    QCOMPARE(x.index(2, 4).data().toInt(), 4);

    // column 5 = label
    QCOMPARE(x.index(0, 5).data().toString(), QString("bll2760_152-270_1-119"));
    QCOMPARE(x.index(1, 5).data().toString(), QString("SMa1229_128-245_1-118"));
    QCOMPARE(x.index(2, 5).data().toString(), QString("E4_20-133_1-114"));

    // column 6 = start
    QCOMPARE(x.index(0, 6).data().toInt(), 1);
    QCOMPARE(x.index(1, 6).data().toInt(), 1);
    QCOMPARE(x.index(2, 6).data().toInt(), 1);

    // column 7 = stop
    QCOMPARE(x.index(0, 7).data().toInt(), 119);
    QCOMPARE(x.index(1, 7).data().toInt(), 118);
    QCOMPARE(x.index(2, 7).data().toInt(), 114);

    // column 8 = sequence
    QCOMPARE(x.index(0, 8).data().toString(), QString("------------------IPDAMIVIDGHGIIQLFSTAAERLFGWSELEAIGQNVNILMPEPDRSRHDSYISRYRTTSDPHIIGIGRIVTGKRRDGTTFPMHLSIGEMQSGGEPYFTGFVRDLTEHQQTQARLQELQ"));
    QCOMPARE(x.index(1, 8).data().toString(), QString("TEDVVRARDAHLRSILDTVPDATVVSATDGTIVSFNAAAVRQFGYAEEEVIGQNLRILMPEPYRHEHDGYLQRYMATGEKRIIGIDRVVSGQRKDGSTFPMKLAVGEMRSGGERFFTG-------------------"));
    QCOMPARE(x.index(2, 8).data().toString(), QString("--------GIFFPALEQNMMGAVLINEND-EVMFFNPAAEKLWGYKREEVIGNNIDMLIPRDLRPAHPEYIRHNREGGKARVEGMSRELQLEKKDGSKIWTRFALSKVSAEGKVYYLALVRDA--------------"));

    // column 9 = notes
    QVERIFY(x.index(0, 9).data().isNull());
    QVERIFY(x.index(1, 9).data().isNull());
    QVERIFY(x.index(2, 9).data().isNull());

    // column 10 = created
    QCOMPARE(x.index(0, 10).data().toString(), QString("2010-09-23 14:19:02"));
    QCOMPARE(x.index(1, 10).data().toString(), QString("2010-09-23 14:19:02"));
    QCOMPARE(x.index(2, 10).data().toString(), QString("2010-09-23 14:19:02"));

    // column 11 - seq id
    QCOMPARE(x.index(0, 11).data().toInt(), 2);
    QCOMPARE(x.index(1, 11).data().toInt(), 3);
    QCOMPARE(x.index(2, 11).data().toInt(), 4);

    // column 12 - astring_id
    QCOMPARE(x.index(0, 12).data().toInt(), 2);
    QCOMPARE(x.index(1, 12).data().toInt(), 3);
    QCOMPARE(x.index(2, 12).data().toInt(), 4);

    // column 13 - label
    QCOMPARE(x.index(0, 13).data().toString(), QString("bll2760_152-270"));
    QCOMPARE(x.index(1, 13).data().toString(), QString("SMa1229_128-245"));
    QCOMPARE(x.index(2, 13).data().toString(), QString("E4_20-133"));

    // column 14,15 - source, protein
    QVERIFY(x.index(0, 14).data().isNull());
    QVERIFY(x.index(1, 14).data().isNull());
    QVERIFY(x.index(2, 14).data().isNull());
    QVERIFY(x.index(0, 15).data().isNull());
    QVERIFY(x.index(1, 15).data().isNull());
    QVERIFY(x.index(2, 15).data().isNull());

    // column 16 - created
    QCOMPARE(x.index(0, 16).data().toString(), QString("2010-09-23 14:19:02"));
    QCOMPARE(x.index(1, 16).data().toString(), QString("2010-09-23 14:19:02"));
    QCOMPARE(x.index(2, 16).data().toString(), QString("2010-09-23 14:19:02"));

    // column 17 - does not exist
    QVERIFY(x.index(0, 17).data().isNull());
}

// Whenever a subseq data value changes, a dataChanged signal should be emitted for the
// corresponding element in MsaSubseqModel
//
// TODO: test updating rest of subseq elements: amino_seq_id, start, stop
// TODO: multiple dataChanged signals for subseqs with the same seq
void TestMsaSubseqModel::dataChanged()
{
    setupDataSource();

    MsaSubseqModel x;

    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 2);
    QVERIFY(subseq->setBioString("BC--DE"));

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, 3);
    QVERIFY(subseq2->setBioString("GH-IJK"));

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3, 4);
    QVERIFY(subseq3->setBioString("--CD--"));

    Msa *msa = new Msa(eUnknownAlphabet, 1, this);
    QVERIFY(msa->append(subseq));
    QVERIFY(msa->append(subseq2));
    QVERIFY(msa->append(subseq3));

    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    QVERIFY(aminoSubseqs->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqs, "amino_seq_id", true));
    aminoSubseqs->load(QList<int>() << 1 << 2 << 3 << 4);
    QCOMPARE(aminoSubseqs->rowCount(), 4);
    QCOMPARE(aminoSeqs->rowCount(), 4);

    x.setAnnotationTables(aminoSubseqs, aminoSeqs);
    x.setMsa(msa);

    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: update subseq not in MSA
    aminoSubseqs->setData(1, 2, "New label");
    QVERIFY(spyDataChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Test: update subseq label should cascade change to Msa
    QVERIFY(x.index(0, 5).data().toString() != QString("New label"));
    aminoSubseqs->setData(2, 2, "New label");
    QCOMPARE(x.index(0, 5).data().toString(), QString("New label"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QModelIndex changedIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(changedIndex.row(), 0);
    QCOMPARE(changedIndex.column(), 5);
    QCOMPARE(changedIndex, qvariant_cast<QModelIndex>(spyArguments.takeFirst()));

    // ------------------------------------------------------------------------
    // Test: update subseq sequence, notes, created
    for (int i=5; i<8; ++i)
    {
        QString string = "More data";

        // Row 1, column i of MsaSubseqModel
        QVERIFY(x.index(1, i+3).data().toString() != string);

        // Id 3, column i-3 of RelatedTableModel
        aminoSubseqs->setData(3, i, string);
        QCOMPARE(x.index(1, i+3).data().toString(), string);
        QCOMPARE(spyDataChanged.count(), 1);
        spyArguments = spyDataChanged.takeFirst();
        QModelIndex changedIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
        QCOMPARE(changedIndex.row(), 1);
        QCOMPARE(changedIndex.column(), i+3);
        QCOMPARE(changedIndex, qvariant_cast<QModelIndex>(spyArguments.takeFirst()));
    }

    // ------------------------------------------------------------------------
    // ------------------------------------------------------------------------
    // Seq level changes

    // ------------------------------------------------------------------------
    // Test: update subseq not in MSA
    aminoSeqs->setData(1, 2, "New label");
    QVERIFY(spyDataChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Test: update seq label, source, protein, created
    for (int i=2; i<6; ++i)
    {
        int modelColumn = 3 + aminoSubseqs->columnCount() + i;

        QString string = "More data";

        // Row 1, column i of MsaSubseqModel
        QVERIFY(x.index(1, modelColumn).data().toString() != string);
        // Id 3, column i-3 of RelatedTableModel
        aminoSeqs->setData(3, i, string);
        QCOMPARE(x.index(1, modelColumn).data().toString(), string);
        QCOMPARE(spyDataChanged.count(), 1);
        spyArguments = spyDataChanged.takeFirst();
        QModelIndex changedIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
        QCOMPARE(changedIndex.row(), 1);
        QCOMPARE(changedIndex.column(), i+3+aminoSubseqs->columnCount());
        QCOMPARE(changedIndex, qvariant_cast<QModelIndex>(spyArguments.takeFirst()));
    }
}

void TestMsaSubseqModel::annotationTableReset()
{
    setupDataSource();

    MsaSubseqModel x;
    AnonSeq anonSeq(1, "ABCDEF");
    Subseq *subseq = new Subseq(anonSeq, 2);
    QVERIFY(subseq->setBioString("BC--DE"));

    AnonSeq anonSeq2(2, "GHIJKL");
    Subseq *subseq2 = new Subseq(anonSeq2, 3);
    QVERIFY(subseq2->setBioString("GH-IJK"));

    AnonSeq anonSeq3(3, "CDEF");
    Subseq *subseq3 = new Subseq(anonSeq3, 4);
    QVERIFY(subseq3->setBioString("--CD--"));

    Msa *msa = new Msa(eUnknownAlphabet, 1, this);

    TableModel *aminoSeqs = new TableModel(this);
    aminoSeqs->setSource(&dataSource_, constants::kTableAminoSeqs, QStringList() << "astring_id" << "label" << "source" << "protein" << "created");
    RelatedTableModel *aminoSubseqs = new RelatedTableModel(this);
    aminoSubseqs->setSource(&dataSource_, constants::kTableAminoSubseqs, QStringList() << "amino_seq_id" << "label" << "start" << "stop" << "sequence" << "notes" << "created");
    QVERIFY(aminoSubseqs->setRelation(RelatedTableModel::eRelationBelongsTo, aminoSeqs, "amino_seq_id", true));
    aminoSubseqs->load(QList<int>() << 1 << 2 << 3 << 4);
    QCOMPARE(aminoSubseqs->rowCount(), 4);
    QCOMPARE(aminoSeqs->rowCount(), 4);

    x.setAnnotationTables(aminoSubseqs, aminoSeqs);

    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(QModelIndex,QModelIndex)));
    QVariantList spyArguments;
    QModelIndex spyIndex;

    // ------------------------------------------------------------------------
    // Test: Reset the subseq and seq source model without any MSA defined
    QCOMPARE(x.rowCount(), 0);
    aminoSubseqs->clear();
    aminoSeqs->clear();
    QVERIFY(spyDataChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Test: Reset the subseq and seq source model with a valid MSA, but no sequences
    x.setMsa(msa);
    QCOMPARE(x.rowCount(), 0);
    aminoSubseqs->clear();
    aminoSeqs->clear();
    QVERIFY(spyDataChanged.isEmpty());

    // Setup
    QVERIFY(msa->append(subseq));
    QVERIFY(msa->append(subseq2));
    QVERIFY(msa->append(subseq3));

    // ------------------------------------------------------------------------
    // Test: Reset the subseq source model
    aminoSubseqs->clear();

    QCOMPARE(spyDataChanged.count(), 2);
    // Both signals should contain identical ranges
    for (int i=0; i<2; ++i)
    {
        spyArguments = spyDataChanged.takeFirst();
        spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
        QCOMPARE(spyIndex.row(), 0);
        QCOMPARE(spyIndex.column(), 3);
        spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(1));
        QCOMPARE(spyIndex.row(), x.rowCount()-1);
        QCOMPARE(spyIndex.column(), x.columnCount()-1);
    }

    // ------------------------------------------------------------------------
    // Test: Reset the seq source model
    aminoSeqs->clear();

    QCOMPARE(spyDataChanged.count(), 2);
    // Both signals should contain identical ranges
    for (int i=0; i<2; ++i)
    {
        spyArguments = spyDataChanged.takeFirst();
        spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
        QCOMPARE(spyIndex.row(), 0);
        QCOMPARE(spyIndex.column(), 11);
        spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(1));
        QCOMPARE(spyIndex.row(), x.rowCount()-1);
        QCOMPARE(spyIndex.column(), x.columnCount()-1);
    }
}

void TestMsaSubseqModel::msaSwapSubseqs()
{
    MsaSubseqModel x;

    Msa *msa = createTestMsa(eUnknownAlphabet, 1);
    x.setMsa(msa);

    QCOMPARE(x.rowCount(), 4);

    QSignalSpy spyRowsAboutToBeMoved(&x, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    QSignalSpy spyRowsMoved(&x, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)));
    QVariantList spyArguments;
    QModelIndex spyIndex;

    // ------------------------------------------------------------------------
    // Test: swapping sequences 1 and 2
    msa->swap(1, 2);
    QCOMPARE(spyRowsAboutToBeMoved.count(), 1);
    spyArguments = spyRowsAboutToBeMoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(3));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(4).toInt(), 2);

    QCOMPARE(spyRowsMoved.count(), 1);
    spyArguments = spyRowsMoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(3));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(4).toInt(), 2);

    QCOMPARE(x.index(0, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 3);
    QCOMPARE(x.index(1, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 2);
    QCOMPARE(x.index(2, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 4);

    // ------------------------------------------------------------------------
    // Test: swapping sequences 3 and 2
    msa->swap(3, 2);
    QCOMPARE(spyRowsAboutToBeMoved.count(), 1);
    spyArguments = spyRowsAboutToBeMoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(3));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(4).toInt(), 1);

    QCOMPARE(spyRowsMoved.count(), 1);
    spyArguments = spyRowsMoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 2);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(3));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(4).toInt(), 1);

    QCOMPARE(x.index(0, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 3);
    QCOMPARE(x.index(1, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 4);
    QCOMPARE(x.index(2, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 2);
}

void TestMsaSubseqModel::msaMoveSubseqs()
{
    MsaSubseqModel x;

    Msa *msa = createTestMsa(eUnknownAlphabet, 1);
    x.setMsa(msa);

    QCOMPARE(x.rowCount(), 4);

    QSignalSpy spyRowsAboutToBeMoved(&x, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
    QSignalSpy spyRowsMoved(&x, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)));
    QVariantList spyArguments;
    QModelIndex spyIndex;

    // ------------------------------------------------------------------------
    // Test: moving 1st and 2nd subseqs to position 2
    msa->moveRowRange(1, 2, 2);
    QCOMPARE(spyRowsAboutToBeMoved.count(), 1);
    spyArguments = spyRowsAboutToBeMoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(3));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(4).toInt(), 3);

    QCOMPARE(spyRowsMoved.count(), 1);
    spyArguments = spyRowsMoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    QCOMPARE(spyArguments.at(2).toInt(), 1);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(3));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(4).toInt(), 3);

    QCOMPARE(x.index(0, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 4);
    QCOMPARE(x.index(1, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 2);
    QCOMPARE(x.index(2, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 3);

    // ------------------------------------------------------------------------
    // Test: moving subseqs 1 and 2 to position 2
    msa->moveRowRange(3, 4, 2);

    QCOMPARE(spyRowsAboutToBeMoved.count(), 1);
    spyArguments = spyRowsAboutToBeMoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(3));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(4).toInt(), 1);

    QCOMPARE(spyRowsMoved.count(), 1);
    spyArguments = spyRowsMoved.takeFirst();
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(0));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(1).toInt(), 2);
    QCOMPARE(spyArguments.at(2).toInt(), 3);
    spyIndex = qvariant_cast<QModelIndex>(spyArguments.at(3));
    QCOMPARE(spyIndex.isValid(), false);
    QCOMPARE(spyArguments.at(4).toInt(), 1);

    QCOMPARE(x.index(0, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 4);
    QCOMPARE(x.index(1, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 3);
    QCOMPARE(x.index(2, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 5);
    QCOMPARE(x.index(3, MsaSubseqModel::kCoreSubseqIdColumn_).data().toInt(), 2);
}

QTEST_MAIN(TestMsaSubseqModel)
#include "TestMsaSubseqModel.moc"
