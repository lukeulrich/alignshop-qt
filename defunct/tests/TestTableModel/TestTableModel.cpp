/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "SynchronousAdocDataSource.h"
#include "AdocTreeNode.h"
#include "DataRow.h"

#include "TableModel.h"

class TestTableModel : public QObject
{
    Q_OBJECT

public:
    TestTableModel()
    {
        qRegisterMetaType<TableModel *>();
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setSource();
    void load();
    void loadWithForeignKey();
    void clear();
    void tableName();
    void fields();
    void columnCount();
    void dataViaField();
    void dataViaColumn();
    void fieldColumn();
    void setFriendlyFieldNames();       // Also tests friendly field names

    void setDataViaField();              // Updates the column data via named fields
    void setDataViaColumn();
    void rowCount();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestTableModel::setSource()
{
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    TableModel x;

    QSignalSpy spyModelAboutToBeReset(&x, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyModelReset(&x, SIGNAL(modelReset()));
    QSignalSpy spySourceChanged(&x, SIGNAL(sourceChanged(TableModel *)));

    // Test: All null parameters
    x.setSource(0, QString(), QStringList());
    QVERIFY(spySourceChanged.isEmpty());
    QVERIFY(x.tableName().isEmpty());
    QVERIFY(x.fields().isEmpty());
    QCOMPARE(spyModelAboutToBeReset.count(), 0);
    QCOMPARE(spyModelReset.count(), 0);

    // Test: valid datasource
    x.setSource(dataSource, QString(), QStringList());
    QCOMPARE(spySourceChanged.count(), 1);
    QVERIFY(qvariant_cast<TableModel *>(spySourceChanged.takeFirst().at(0)) == &x);
    spySourceChanged.clear();
    QVERIFY(x.tableName().isEmpty());
    QVERIFY(x.fields().isEmpty());
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    // Test: more sourceChanged tests for any parameter
    x.setSource(dataSource, QString(), QStringList());
    QVERIFY(spySourceChanged.isEmpty());
    QVERIFY(spyModelAboutToBeReset.isEmpty());
    QVERIFY(spyModelReset.isEmpty());

    x.setSource(dataSource, constants::kTableAminoMsas, QStringList());
    QCOMPARE(spySourceChanged.count(), 1);
    QVERIFY(qvariant_cast<TableModel *>(spySourceChanged.takeFirst().at(0)) == &x);
    spySourceChanged.clear();
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    x.setSource(dataSource, constants::kTableAminoMsas, QStringList());
    QVERIFY(spySourceChanged.isEmpty());
    QVERIFY(spyModelAboutToBeReset.isEmpty());
    QVERIFY(spyModelReset.isEmpty());

    x.setSource(dataSource, constants::kTableAminoMsas, QStringList() << "id" << "name");
    QCOMPARE(spySourceChanged.count(), 1);
    QVERIFY(qvariant_cast<TableModel *>(spySourceChanged.takeFirst().at(0)) == &x);
    spySourceChanged.clear();
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    x.setSource(dataSource, constants::kTableAminoMsas, QStringList() << "id" << "name");
    QVERIFY(spySourceChanged.isEmpty());
    QVERIFY(spyModelAboutToBeReset.isEmpty());
    QVERIFY(spyModelReset.isEmpty());

    x.setSource(dataSource, constants::kTableAminoMsas, QStringList() << "name");
    QVERIFY(spySourceChanged.isEmpty());
    QVERIFY(spyModelAboutToBeReset.isEmpty());
    QVERIFY(spyModelReset.isEmpty());
}

void TestTableModel::load()
{
    TableModel x;

    QSignalSpy spyLoadDone(&x, SIGNAL(loadDone(int)));
    QSignalSpy spyLoadError(&x, SIGNAL(loadError(QString,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Suite: Invalid data source

    // ------------------------------------------------------------------------
    // Test: default state, empty parameters
    x.load(QList<int>());
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(0).toInt(), 0);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Test: default state, non-empty list
    x.load(QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 10);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Suite: unopened datasource, with not table or fields
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    x.setSource(dataSource, "", QStringList());

    // ------------------------------------------------------------------------
    // Test: default state, empty id list
    x.load(QList<int>());
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    spyLoadError.clear();

    // Test: default state, non-empty list
    x.load(QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 10);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Suite: unopened datasource, valid table, no fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList());

    // ------------------------------------------------------------------------
    // Test: default state, empty id list
    x.load(QList<int>());
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 0);
    spyLoadDone.clear();

    // Test: default state, non-empty list
    x.load(QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 10);
    spyLoadDone.clear();

    // ------------------------------------------------------------------------
    // Suite: unopened datasource, valid table, fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "digest" << "sequence");

    // ------------------------------------------------------------------------
    // Test: default state, empty id list
    x.load(QList<int>());
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 0);
    spyLoadDone.clear();

    // Test: default state, non-empty list
    x.load(QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 10);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Suite: opened datasource, empty table, no fields
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());
    x.setSource(dataSource, "", QStringList());

    // ------------------------------------------------------------------------
    // Test: empty id list => error, because no table is defined
    x.load(QList<int>());
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    spyLoadError.clear();

    // Test: non-empty list => error, because no table is defined
    x.load(QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 10);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Suite: opened datasource, valid table, no fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList());

    // ------------------------------------------------------------------------
    // Test: empty id list => success, because table is defined, but no fields
    x.load(QList<int>());
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 0);
    spyLoadDone.clear();
    QVERIFY(x.rowCount() == 0);

    // Test: valid id list => success, because table is defined, but no fields, internal data remains empty
    x.load(QList<int>() << 1 << 2, 20);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 20);
    spyLoadDone.clear();
    QVERIFY(x.rowCount() == 0);

    // ------------------------------------------------------------------------
    // Test: opened datasource, valid table, invalid field
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "invalid_field");

    x.load(QList<int>() << 1 << 2, 30);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 30);
    spyLoadError.clear();
    QVERIFY(x.rowCount() == 0);

    // ------------------------------------------------------------------------
    // Test: opened datasource, valid table, valid field
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "digest");
    x.load(QList<int>() << 1 << 2 << -3, 40);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 40);
    spyLoadDone.clear();
    QCOMPARE(x.rowCount(), 2);

    // ------------------------------------------------------------------------
    // Test: opened datasource, valid table, redundant ids
    x.load(QList<int>() << 1 << 2, 50);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 50);
    spyLoadDone.clear();
    QCOMPARE(x.rowCount(), 2);

    // ------------------------------------------------------------------------
    // Test: opened datasource, valid table, redundant ids with a new one
    x.load(QList<int>() << 1 << 2 << 3, 60);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 60);
    spyLoadDone.clear();
    QCOMPARE(x.rowCount(), 3);

    QFile::remove(testDbFile);
}

void TestTableModel::loadWithForeignKey()
{
    TableModel x;

    QSignalSpy spyLoadDone(&x, SIGNAL(loadDone(int)));
    QSignalSpy spyLoadError(&x, SIGNAL(loadError(QString,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Suite: Invalid data source

    // ------------------------------------------------------------------------
    // Test: default state, empty parameters
    x.loadWithForeignKey("", QList<int>());
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(0).toInt(), 0);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Test: default state, non-empty list
    x.loadWithForeignKey("", QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 10);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Test: default state, non-empty table name, non-empty list
    x.loadWithForeignKey("amino_seq_id", QList<int>() << 1 << 2, -10);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), -10);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Suite: unopened datasource, with no table or fields
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    x.setSource(dataSource, "", QStringList());

    // ------------------------------------------------------------------------
    // Test: default state, empty id list
    x.loadWithForeignKey("", QList<int>());
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    spyLoadError.clear();

    // Test: default state, non-empty list
    x.loadWithForeignKey("amino_seq_id", QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 10);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Suite: unopened datasource, valid table, no fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList());

    // ------------------------------------------------------------------------
    // Test: default state, empty id list
    x.loadWithForeignKey("amino_seq_id", QList<int>());
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 0);
    spyLoadDone.clear();

    // Test: default state, non-empty list
    x.loadWithForeignKey("amino_seq_id", QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 10);
    spyLoadDone.clear();

    // ------------------------------------------------------------------------
    // Suite: unopened datasource, valid table, fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "digest" << "sequence");

    // ------------------------------------------------------------------------
    // Suite: opened datasource, empty table, no fields
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());
    x.setSource(dataSource, "", QStringList());

    // ------------------------------------------------------------------------
    // Test: empty id list => error, because no table is defined
    x.loadWithForeignKey("amino_seq_id", QList<int>());
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 0);
    spyLoadError.clear();

    // Test: non-empty list => error, because no table is defined
    x.loadWithForeignKey("amino_seq_id", QList<int>() << 1 << 2, 10);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 10);
    spyLoadError.clear();

    // ------------------------------------------------------------------------
    // Suite: opened datasource, valid table, no fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList());

    // ------------------------------------------------------------------------
    // Test: empty id list => success, because table is defined, but no fields
    x.load(QList<int>());
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 0);
    spyLoadDone.clear();
    QVERIFY(x.rowCount() == 0);

    // Test: valid id list => success, because table is defined, but no fields, internal data remains empty
    x.load(QList<int>() << 1 << 2, 20);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 20);
    spyLoadDone.clear();
    QVERIFY(x.rowCount() == 0);

    // ------------------------------------------------------------------------
    // Test: opened datasource, valid table, invalid field
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "invalid_field");

    x.loadWithForeignKey("aseq_id", QList<int>() << 1 << 2, 30);
    QVERIFY(spyLoadDone.isEmpty());
    QCOMPARE(spyLoadError.count(), 1);
    spyArguments = spyLoadError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty() == false);
    QCOMPARE(spyArguments.at(1).toInt(), 30);
    spyLoadError.clear();
    QVERIFY(x.rowCount() == 0);

    // ------------------------------------------------------------------------
    // Test: opened datasource, valid table, valid field
    x.setSource(dataSource, constants::kTableAminoMsaSubseqs, QStringList() << "id" << "amino_msa_id" << "amino_subseq_id");
    x.loadWithForeignKey("amino_subseq_id", QList<int>() << 2 << 3 << -3, 40);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 40);
    spyLoadDone.clear();
    QCOMPARE(x.rowCount(), 2);

    QCOMPARE(x.data(1, "amino_msa_id").toInt(), 1);
    QCOMPARE(x.data(2, "amino_msa_id").toInt(), 1);

    // ------------------------------------------------------------------------
    // Test: opened datasource, valid table, redundant ids
    x.loadWithForeignKey("amino_subseq_id", QList<int>() << 2 << 3, 50);
    QVERIFY(spyLoadError.isEmpty());
    QCOMPARE(spyLoadDone.count(), 1);
    QCOMPARE(spyLoadDone.takeFirst().at(0).toInt(), 50);
    spyLoadDone.clear();
    QCOMPARE(x.rowCount(), 2);

    // ------------------------------------------------------------------------
    // Test: laod with foreign key that is not part of the fields selected by setSource

    QFile::remove(testDbFile);
}

void TestTableModel::clear()
{
    TableModel x;

    QSignalSpy spyModelAboutToBeReset(&x, SIGNAL(modelAboutToBeReset()));
    QSignalSpy spyModelReset(&x, SIGNAL(modelReset()));

    // ------------------------------------------------------------------------
    // Test: clear should function independent of its data source status
    x.clear();
    QVERIFY(x.rowCount() == 0);
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    // ------------------------------------------------------------------------
    // Test: clear with opened and valid database connection
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // ------------------------------------------------------------------------
    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "sequence");
    x.load(QList<int>() << 1 << 2 << 3);
    QCOMPARE(x.rowCount(), 3);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();
    x.clear();
    QVERIFY(x.rowCount() == 0);
    QCOMPARE(spyModelAboutToBeReset.count(), 1);
    QCOMPARE(spyModelReset.count(), 1);
    spyModelAboutToBeReset.clear();
    spyModelReset.clear();

    QFile::remove(testDbFile);
}

void TestTableModel::tableName()
{
    TableModel x;

    QVERIFY(x.tableName().isEmpty());

    // ------------------------------------------------------------------------
    x.setSource(0, "Bob", QStringList());
    QCOMPARE(x.tableName(), QString("Bob"));
}

void TestTableModel::fields()
{
    TableModel x;

    // ------------------------------------------------------------------------
    // Test: default state should have no fields
    QVERIFY(x.fields().isEmpty());

    // ------------------------------------------------------------------------
    // Test: single field = id
    x.setSource(0, "amino_seqs", QStringList() << "id");
    QCOMPARE(x.fields().count(), 1);
    QCOMPARE(x.fields().at(0), QString("id"));

    // ------------------------------------------------------------------------
    // Test: input of single non-id field, should result in id being prepended to front of list
    x.setSource(0, "amino_seqs", QStringList() << "sequence");
    QCOMPARE(x.fields().count(), 2);
    QCOMPARE(x.fields().at(0), QString("id"));
    QCOMPARE(x.fields().at(1), QString("sequence"));

    // ------------------------------------------------------------------------
    // Test: input of two fields including id which is not first, should be moved to the first position
    x.setSource(0, "amino_seqs", QStringList() << "sequence" << "id");
    QCOMPARE(x.fields().count(), 2);
    QCOMPARE(x.fields().at(0), QString("id"));
    QCOMPARE(x.fields().at(1), QString("sequence"));
}

void TestTableModel::columnCount()
{
    TableModel x;

    // ------------------------------------------------------------------------
    // Test: default state should have no columns
    QCOMPARE(x.columnCount(), 0);

    // ------------------------------------------------------------------------
    // Test: single field = id
    x.setSource(0, "amino_seqs", QStringList() << "id");
    QCOMPARE(x.columnCount(), 1);

    // ------------------------------------------------------------------------
    // Test: input of single non-id field, should result in id being prepended to front of list
    x.setSource(0, "amino_seqs", QStringList() << "sequence");
    QCOMPARE(x.columnCount(), 2);

    // ------------------------------------------------------------------------
    // Test: input of two fields including id which is not first, should be moved to the first position
    x.setSource(0, "amino_seqs", QStringList() << "sequence" << "label");
    QCOMPARE(x.columnCount(), 3);

    // ------------------------------------------------------------------------
    // Test: input of two fields including id which is not first, should be moved to the first position
    x.setSource(0, "amino_seqs", QStringList());
    QCOMPARE(x.columnCount(), 0);
}

void TestTableModel::dataViaField()
{
    TableModel x;

    QList<QPair<int, QString> > dummyInput;
    dummyInput << qMakePair(1, QString("id"))
               << qMakePair(2, QString(""))
               << qMakePair(3, QString("id"))
               << qMakePair(4, QString("digest"))
               << qMakePair(5, QString("  "))
               << qMakePair(-1, QString("__"))
               << qMakePair(0, QString("sequence"))
               << qMakePair(-3, QString("id"))
               << qMakePair(-4, QString("name"))
               << qMakePair(-5, QString("start"));
    int nDummyInput = dummyInput.count();

    // ------------------------------------------------------------------------
    // Test: uninitialized model should return null QVariant every time
    for (int i=0; i< nDummyInput; ++i)
        QVERIFY(x.data(dummyInput.at(i).first, dummyInput.at(i).second).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, empty table, no fields
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    x.setSource(dataSource, "", QStringList());
    for (int i=0; i< nDummyInput; ++i)
        QVERIFY(x.data(dummyInput.at(i).first, dummyInput.at(i).second).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, valid table, no fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList());
    for (int i=0; i< nDummyInput; ++i)
        QVERIFY(x.data(dummyInput.at(i).first, dummyInput.at(i).second).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, valid table, fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "sequence");
    for (int i=0; i< nDummyInput; ++i)
        QVERIFY(x.data(dummyInput.at(i).first, dummyInput.at(i).second).isValid() == false);

    // ------------------------------------------------------------------------
    // Setup
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());

    // ------------------------------------------------------------------------
    // Test: valid opened data source, no data loaded
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "digest");
    for (int i=0; i< nDummyInput; ++i)
        QVERIFY(x.data(dummyInput.at(i).first, dummyInput.at(i).second).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: loaded data
    x.load(QList<int>() << 2 << 4);
    QCOMPARE(x.rowCount(), 2);
    QCOMPARE(x.data(2, "id").toInt(), 2);
    QCOMPARE(x.data(4, "id").toInt(), 4);
    QVERIFY(x.data(1, "id").isValid() == false);
    QVERIFY(x.data(3, "id").isValid() == false);

    QCOMPARE(x.data(2, "digest").toString(), QString("/H7/5HS9vDhXRYn88h7ksiZf3I4="));
    QCOMPARE(x.data(4, "digest").toString(), QString("2TNJX+qEvph6dZglQGSPctRx8Zs="));
    QVERIFY(x.data(3, "digest").isValid() == false);

    x.load(QList<int>() << 2 << 3);
    QCOMPARE(x.rowCount(), 3);
    QCOMPARE(x.data(2, "id").toInt(), 2);
    QCOMPARE(x.data(3, "id").toInt(), 3);
    QCOMPARE(x.data(4, "id").toInt(), 4);
    QVERIFY(x.data(1, "id").isValid() == false);

    QCOMPARE(x.data(2, "digest").toString(), QString("/H7/5HS9vDhXRYn88h7ksiZf3I4="));
    QCOMPARE(x.data(3, "digest").toString(), QString("DsV+iExTk5eyjf+l6A2rQCBiEP0="));
    QCOMPARE(x.data(4, "digest").toString(), QString("2TNJX+qEvph6dZglQGSPctRx8Zs="));
}

void TestTableModel::dataViaColumn()
{
    TableModel x;

    // ------------------------------------------------------------------------
    // Test: uninitialized model should return null QVariant every time for all row column variations
    for (int i=-5; i< 5; ++i)
        for (int j=-5; j< 5; ++j)
            QVERIFY(x.data(i, j).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, empty table, no fields
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    x.setSource(dataSource, "", QStringList());
    for (int i=-5; i< 5; ++i)
        for (int j=-5; j< 5; ++j)
            QVERIFY(x.data(i, j).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, valid table, no fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList());
    for (int i=-5; i< 5; ++i)
        for (int j=-5; j< 5; ++j)
            QVERIFY(x.data(i, j).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, valid table, fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "sequence");
    for (int i=-5; i< 5; ++i)
        for (int j=-5; j< 5; ++j)
            QVERIFY(x.data(i, j).isValid() == false);

    // ------------------------------------------------------------------------
    // Setup
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());

    // ------------------------------------------------------------------------
    // Test: valid opened data source, no data loaded
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "digest" << "sequence");
    for (int i=-5; i< 5; ++i)
        for (int j=-5; j< 5; ++j)
            QVERIFY(x.data(i, j).isValid() == false);

    // ------------------------------------------------------------------------
    // Test: loaded data
    x.load(QList<int>() << 2 << 4);
    QCOMPARE(x.rowCount(), 2);
    QCOMPARE(x.data(2, 0).toInt(), 2);
    QCOMPARE(x.data(4, 0).toInt(), 4);
    QVERIFY(x.data(1, 0).isValid() == false);
    QVERIFY(x.data(3, 0).isValid() == false);

    QCOMPARE(x.data(2, 1).toString(), QString("/H7/5HS9vDhXRYn88h7ksiZf3I4="));
    QCOMPARE(x.data(4, 1).toString(), QString("2TNJX+qEvph6dZglQGSPctRx8Zs="));
    QVERIFY(x.data(3, 1).isValid() == false);

    QCOMPARE(x.data(2, 2).toString(), QString("IPDAMIVIDGHGIIQLFSTAAERLFGWSELEAIGQNVNILMPEPDRSRHDSYISRYRTTSDPHIIGIGRIVTGKRRDGTTFPMHLSIGEMQSGGEPYFTGFVRDLTEHQQTQARLQELQ"));
    QCOMPARE(x.data(4, 2).toString(), QString("GIFFPALEQNMMGAVLINENDEVMFFNPAAEKLWGYKREEVIGNNIDMLIPRDLRPAHPEYIRHNREGGKARVEGMSRELQLEKKDGSKIWTRFALSKVSAEGKVYYLALVRDA"));

    // Test: invalid columns
    QVERIFY(x.data(2, -2).isValid() == false);
    QVERIFY(x.data(2, -1).isValid() == false);
    QVERIFY(x.data(2, 3).isValid() == false);
    QVERIFY(x.data(2, 4).isValid() == false);

    x.load(QList<int>() << 2 << 3);
    QCOMPARE(x.rowCount(), 3);
    QCOMPARE(x.data(2, 0).toInt(), 2);
    QCOMPARE(x.data(3, 0).toInt(), 3);
    QCOMPARE(x.data(4, 0).toInt(), 4);
    QVERIFY(x.data(1, 0).isValid() == false);

    QCOMPARE(x.data(2, 1).toString(), QString("/H7/5HS9vDhXRYn88h7ksiZf3I4="));
    QCOMPARE(x.data(3, 1).toString(), QString("DsV+iExTk5eyjf+l6A2rQCBiEP0="));
    QCOMPARE(x.data(4, 1).toString(), QString("2TNJX+qEvph6dZglQGSPctRx8Zs="));

    QCOMPARE(x.data(3, 2).toString(), QString("TEDVVRARDAHLRSILDTVPDATVVSATDGTIVSFNAAAVRQFGYAEEEVIGQNLRILMPEPYRHEHDGYLQRYMATGEKRIIGIDRVVSGQRKDGSTFPMKLAVGEMRSGGERFFTG"));
}

void TestTableModel::fieldColumn()
{
    TableModel x;

    // ------------------------------------------------------------------------
    // Test: no columns should always return -1
    QCOMPARE(x.fieldColumn(""), -1);
    QCOMPARE(x.fieldColumn("missing"), -1);

    // ------------------------------------------------------------------------
    // Test: single field = id
    x.setSource(0, "amino_seqs", QStringList() << "id");
    QCOMPARE(x.fieldColumn("id"), 0);
    QCOMPARE(x.fieldColumn(""), -1);
    QCOMPARE(x.fieldColumn("missing"), -1);

    // ------------------------------------------------------------------------
    // Test: input of single non-id field, should result in id being prepended to front of list
    x.setSource(0, "amino_seqs", QStringList() << "sequence");
    QCOMPARE(x.fieldColumn("id"), 0);
    QCOMPARE(x.fieldColumn("sequence"), 1);
    QCOMPARE(x.fieldColumn(""), -1);
    QCOMPARE(x.fieldColumn("missing"), -1);

    // ------------------------------------------------------------------------
    // Test: input of two fields including id which is not first, should be moved to the first position
    x.setSource(0, "amino_seqs", QStringList() << "label" << "sequence" << "id");
    QCOMPARE(x.fieldColumn("id"), 0);
    QCOMPARE(x.fieldColumn("label"), 1);
    QCOMPARE(x.fieldColumn("sequence"), 2);
    QCOMPARE(x.fieldColumn(""), -1);
    QCOMPARE(x.fieldColumn("missing"), -1);

    // ------------------------------------------------------------------------
    // Test: resetting should make columns disappear
    x.setSource(0, "amino_seqs", QStringList());
    QCOMPARE(x.fieldColumn("id"), -1);
}

void TestTableModel::setFriendlyFieldNames()
{
    TableModel x;

    QSignalSpy spyFriendFieldNamesChanged(&x, SIGNAL(friendlyFieldNamesChanged(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: no columns should always empty qstring for all friendly field names
    for (int i=-3; i< 3; ++i)
        QVERIFY(x.friendlyFieldName(i).isEmpty());

    // ------------------------------------------------------------------------
    // Test: input of two fields including id which is not first; friendly field names by default should
    //       be the same as the fields names
    x.setSource(0, "amino_seqs", QStringList() << "label" << "sequence" << "id");
    QCOMPARE(x.friendlyFieldName(0), QString("id"));
    QCOMPARE(x.friendlyFieldName(1), QString("label"));
    QCOMPARE(x.friendlyFieldName(2), QString("sequence"));

    // ------------------------------------------------------------------------
    // Test: setFriendlyFieldNames without any should not change any
    x.setFriendlyFieldNames(QStringList());
    QCOMPARE(x.friendlyFieldName(0), QString("id"));
    QCOMPARE(x.friendlyFieldName(1), QString("label"));
    QCOMPARE(x.friendlyFieldName(2), QString("sequence"));

    // ------------------------------------------------------------------------
    // Test: setFriendlyFieldNames with varying number of friendly names
    QVERIFY(spyFriendFieldNamesChanged.isEmpty());
    x.setFriendlyFieldNames(QStringList() << "ID");
    QCOMPARE(x.friendlyFieldName(0), QString("ID"));
    QCOMPARE(x.friendlyFieldName(1), QString("label"));
    QCOMPARE(x.friendlyFieldName(2), QString("sequence"));
    QCOMPARE(spyFriendFieldNamesChanged.count(), 1);
    spyArguments = spyFriendFieldNamesChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 0);
    QCOMPARE(spyArguments.at(1).toInt(), 0);

    x.setFriendlyFieldNames(QStringList() << "ID" << "Name");
    QCOMPARE(x.friendlyFieldName(0), QString("ID"));
    QCOMPARE(x.friendlyFieldName(1), QString("Name"));
    QCOMPARE(x.friendlyFieldName(2), QString("sequence"));
    QCOMPARE(spyFriendFieldNamesChanged.count(), 1);
    spyArguments = spyFriendFieldNamesChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 0);
    QCOMPARE(spyArguments.at(1).toInt(), 1);

    x.setFriendlyFieldNames(QStringList() << "ID" << "" << "DATA");
    QCOMPARE(x.friendlyFieldName(0), QString("ID"));
    QCOMPARE(x.friendlyFieldName(1), QString(""));
    QCOMPARE(x.friendlyFieldName(2), QString("DATA"));
    QCOMPARE(spyFriendFieldNamesChanged.count(), 1);
    spyArguments = spyFriendFieldNamesChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 0);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

    // table model only has three fields, thus "last" should be ignored
    x.setFriendlyFieldNames(QStringList() << "ID" << "Name" << "DATA" << "last");
    QCOMPARE(x.friendlyFieldName(0), QString("ID"));
    QCOMPARE(x.friendlyFieldName(1), QString("Name"));
    QCOMPARE(x.friendlyFieldName(2), QString("DATA"));
    QCOMPARE(spyFriendFieldNamesChanged.count(), 1);
    spyArguments = spyFriendFieldNamesChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 0);
    QCOMPARE(spyArguments.at(1).toInt(), 2);

    // ------------------------------------------------------------------------
    // Test: friendly names should be default back to fields whenever setSource is called
    x.setSource(0, "amino_seqs", QStringList() << "one" << "two");
    QCOMPARE(x.friendlyFieldName(0), QString("id"));
    QCOMPARE(x.friendlyFieldName(1), QString("one"));
    QCOMPARE(x.friendlyFieldName(2), QString("two"));
}

void TestTableModel::setDataViaField()
{
    TableModel x;

    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: uninitialized model should return false QVariant every time for all row column variations
    QStringList randomFieldNames;
    randomFieldNames << "id" << "organism" << "source" << "label" << "start" << "stop";
    for (int i=-5; i< 5; ++i)
        for (int j=-5; j< 5; ++j)
            QCOMPARE(x.setData(i, randomFieldNames.at(randomInteger(0, randomFieldNames.count()-1)), "New value"), false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, empty table, no fields
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    x.setSource(dataSource, "", QStringList());
    for (int i=0; i< 5; ++i)
        QCOMPARE(x.setData(i, randomFieldNames.at(randomInteger(0, randomFieldNames.count()-1)), "New value"), false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, valid table, no fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList());
    for (int i=0; i< 5; ++i)
        QCOMPARE(x.setData(i, randomFieldNames.at(randomInteger(0, randomFieldNames.count()-1)), "New value"), false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, valid table, fields
    //       no rows, thus, all requests should return false
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "sequence");
    for (int i=0; i< 5; ++i)
        QCOMPARE(x.setData(i, randomFieldNames.at(randomInteger(0, randomFieldNames.count()-1)), "New value"), false);

    QVERIFY(spyDataChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Setup
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());

    // ------------------------------------------------------------------------
    // Test: valid opened data source, no data loaded
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "aseq_id" << "digest" << "sequence");
    for (int i=0; i< 5; ++i)
        QCOMPARE(x.setData(i, randomFieldNames.at(randomInteger(0, randomFieldNames.count()-1)), "New value"), false);

    QVERIFY(spyDataChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Test: loaded data
    x.load(QList<int>() << 2 << 4);
    QCOMPARE(x.rowCount(), 2);

    // Test: empty field name
    QCOMPARE(x.setData(2, QString(), "New value"), false);
    QCOMPARE(x.setData(2, "  ", "New value"), false);

    QSqlDatabase database = static_cast<SynchronousAdocDataSource *>(x.adocDataSource_)->database_;

    // Test: id of record not loaded
    QCOMPARE(x.setData(1, "aseq_id", 25), false);
    QSqlQuery query = database.exec(QString("SELECT aseq_id FROM %1 WHERE id = 1 AND aseq_id is not null").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(!query.next());

    // Test: id of loaded record, but invalid field
    QCOMPARE(x.setData(2, "missing", 25), false);
    QVERIFY(spyDataChanged.isEmpty());

    // Test: valid setData request
    QVERIFY(x.setData(2, "aseq_id", 25));
    QCOMPARE(x.data(2, "aseq_id").toInt(), 25);
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    spyDataChanged.clear();

    query = database.exec(QString("SELECT aseq_id FROM %1 WHERE id = 2").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());
    QCOMPARE(query.record().value(0).toInt(), 25);

    // Set it back to null
    QVERIFY(x.setData(2, "aseq_id", QVariant()));
    QVERIFY(x.data(2, "aseq_id").isValid() == false);
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    spyDataChanged.clear();

    query = database.exec(QString("SELECT aseq_id FROM %1 WHERE id = 2").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());
    QVERIFY(query.record().value(0).isNull());

    // Test: setting the id field is not allowed
    QCOMPARE(x.setData(2, "id", 4), false);
    QCOMPARE(x.data(2, "id").toInt(), 2);

    QCOMPARE(x.setData(2, "id", 100), false);
    QCOMPARE(x.data(2, "id").toInt(), 2);

    QVERIFY(spyDataChanged.isEmpty());

    query = database.exec(QString("SELECT count(*) FROM %1 WHERE id = 2").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());
    QCOMPARE(query.record().value(0).toInt(), 1);

    // Test: setData for another field
    QVERIFY(x.setData(4, "sequence", "ABC"));
    QCOMPARE(x.data(4, "sequence").toString(), QString("ABC"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    spyDataChanged.clear();

    query = database.exec(QString("SELECT sequence FROM %1 WHERE id = 4 and sequence = 'ABC'").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());

    // No need to test with invalid fields supplied by user because no records will ever be loaded -
    // the select query will fail.

    // Test: attempt to trigger setDataError by sending a valid command and forcefully closing the database connection first
    //       Should get two dataChanged signals, and one setDataError
    QSignalSpy spySetDataError(&x, SIGNAL(setDataError(int,int,QString)));
    database.close();
    QVERIFY(x.setData(4, "sequence", "DEF"));       // Should return true, but also should get setDataError signal and values should be reset
    QCOMPARE(x.data(4, "sequence").toString(), QString("ABC"));
    QCOMPARE(spyDataChanged.count(), 2);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    spyDataChanged.clear();

    QCOMPARE(spySetDataError.count(), 1);
    spyArguments = spySetDataError.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toString().isEmpty(), false);
}

void TestTableModel::setDataViaColumn()
{
    TableModel x;

    QSignalSpy spyDataChanged(&x, SIGNAL(dataChanged(int,int)));
    QVariantList spyArguments;

    // ------------------------------------------------------------------------
    // Test: uninitialized model should return false QVariant every time for all row column variations
    QStringList randomFieldNames;
    for (int i=-5; i< 5; ++i)
        for (int j=-5; j< 5; ++j)
            QCOMPARE(x.setData(i, j, "New value"), false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, empty table, no fields
    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    x.setSource(dataSource, "", QStringList());
    for (int i=0; i< 5; ++i)
        QCOMPARE(x.setData(i, randomInteger(0, 4), "New value"), false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, valid table, no fields
    x.setSource(dataSource, constants::kTableAstrings, QStringList());
    for (int i=0; i< 5; ++i)
        QCOMPARE(x.setData(i, randomInteger(0, 4), "New value"), false);

    // ------------------------------------------------------------------------
    // Test: valid, unopened data source, valid table, fields
    //       no rows, thus, all requests should return false
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "sequence");
    for (int i=0; i< 5; ++i)
        QCOMPARE(x.setData(i, randomInteger(0, 4), "New value"), false);

    QVERIFY(spyDataChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Setup
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());

    // ------------------------------------------------------------------------
    // Test: valid opened data source, no data loaded
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "aseq_id" << "digest" << "sequence");
    for (int i=0; i< 5; ++i)
        QCOMPARE(x.setData(i, randomInteger(0, 3), "New value"), false);

    QVERIFY(spyDataChanged.isEmpty());

    // ------------------------------------------------------------------------
    // Test: loaded data
    x.load(QList<int>() << 2 << 4);
    QCOMPARE(x.rowCount(), 2);

    // Test: negative and zero column
    QCOMPARE(x.setData(2, -1, "New value"), false);
    QCOMPARE(x.setData(2, 0, "New value"), false);

    QSqlDatabase database = static_cast<SynchronousAdocDataSource *>(x.adocDataSource_)->database_;

    // Test: id of record not loaded
    QCOMPARE(x.setData(1, 1, 25), false);
    QSqlQuery query = database.exec(QString("SELECT aseq_id FROM %1 WHERE id = 1 AND aseq_id is not null").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(!query.next());

    // Test: id of loaded record, but invalid field
    QCOMPARE(x.setData(2, 200, 25), false);
    QVERIFY(spyDataChanged.isEmpty());

    // Test: valid setData request
    QVERIFY(x.setData(2, 1, 25));
    QCOMPARE(x.data(2, 1).toInt(), 25);
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    spyDataChanged.clear();

    query = database.exec(QString("SELECT aseq_id FROM %1 WHERE id = 2").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());
    QCOMPARE(query.record().value(0).toInt(), 25);

    // Set it back to null
    QVERIFY(x.setData(2, 1, QVariant()));
    QVERIFY(x.data(2, 1).isValid() == false);
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 2);
    QCOMPARE(spyArguments.at(1).toInt(), 1);
    spyDataChanged.clear();

    query = database.exec(QString("SELECT aseq_id FROM %1 WHERE id = 2").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());
    QVERIFY(query.record().value(0).isNull());

    // Test: setting the id field is not allowed
    QCOMPARE(x.setData(2, 0, 4), false);
    QCOMPARE(x.data(2, 0).toInt(), 2);

    QCOMPARE(x.setData(2, 0, 100), false);
    QCOMPARE(x.data(2, 0).toInt(), 2);

    QVERIFY(spyDataChanged.isEmpty());

    query = database.exec(QString("SELECT count(*) FROM %1 WHERE id = 2").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());
    QCOMPARE(query.record().value(0).toInt(), 1);

    // Test: setData for another field
    QVERIFY(x.setData(4, 3, "ABC"));
    QCOMPARE(x.data(4, 3).toString(), QString("ABC"));
    QCOMPARE(spyDataChanged.count(), 1);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    spyDataChanged.clear();

    query = database.exec(QString("SELECT sequence FROM %1 WHERE id = 4 and sequence = 'ABC'").arg(constants::kTableAstrings));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());

    // No need to test with invalid fields supplied by user because no records will ever be loaded -
    // the select query will fail.

    // Test: attempt to trigger setDataError by sending a valid command and forcefully closing the database connection first
    //       Should get two dataChanged signals, and one setDataError
    QSignalSpy spySetDataError(&x, SIGNAL(setDataError(int,int,QString)));
    database.close();
    QVERIFY(x.setData(4, 3, "DEF"));       // Should return true, but also should get setDataError signal and values should be reset
    QCOMPARE(x.data(4, 3).toString(), QString("ABC"));
    QCOMPARE(spyDataChanged.count(), 2);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    spyArguments = spyDataChanged.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    spyDataChanged.clear();

    QCOMPARE(spySetDataError.count(), 1);
    spyArguments = spySetDataError.takeFirst();
    QCOMPARE(spyArguments.at(0).toInt(), 4);
    QCOMPARE(spyArguments.at(1).toInt(), 3);
    QCOMPARE(spyArguments.at(2).toString().isEmpty(), false);
}

void TestTableModel::rowCount()
{
    TableModel x;

    // Test: uninitialized table should not have any rows
    QCOMPARE(x.rowCount(), 0);

    // ------------------------------------------------------------------------
    // Setup
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    AbstractAdocDataSource *dataSource = new SynchronousAdocDataSource(this);
    dataSource->open(testDbFile);
    QVERIFY(dataSource->isOpen());
    x.setSource(dataSource, constants::kTableAstrings, QStringList() << "id" << "aseq_id" << "digest" << "sequence");

    // ------------------------------------------------------------------------
    // Load some records and check that the rowCount increases accordingly
    x.load(QList<int>() << 2 << 4);

    QCOMPARE(x.rowCount(), 2);

    x.load(QList<int>() << 1);
    QCOMPARE(x.rowCount(), 3);
}

QTEST_MAIN(TestTableModel)
#include "TestTableModel.moc"
