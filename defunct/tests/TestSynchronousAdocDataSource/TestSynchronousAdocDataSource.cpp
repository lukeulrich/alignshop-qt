/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtTest/QSignalSpy>

#include <QtCore/QHash>
#include <QtCore/QStringBuilder>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "AdocTreeNode.h"
#include "AminoString.h"
#include "DataRow.h"
#include "DbAnonSeqFactory.h"
#include "DnaString.h"
#include "Msa.h"
#include "RnaString.h"
#include "SynchronousAdocDataSource.h"

/**
  * Used for testing select calls
  */
struct SelectResponse
{
    int tag_;
    QString error_;
    QList<DataRow> dataRows_;

    SelectResponse()
    {
        tag_ = 0;
    }

    void clear()
    {
        tag_ = 0;
        error_ = QString();
        dataRows_.clear();
    }

    bool isEmpty() const
    {
        return tag_ == 0 && dataRows_.isEmpty() && error_.isEmpty();
    }
};

/**
  * Used for testing update calls
  */
struct UpdateResponse
{
    int tag_;
    QString error_;
    DataRow dataRow_;
    bool affected_;

    UpdateResponse()
    {
        tag_ = 0;
        affected_ = false;
    }

    void clear()
    {
        tag_ = 0;
        error_ = QString();
        affected_ = false;
        dataRow_.clear();
    }

    bool isEmpty() const
    {
        return tag_ == 0 && dataRow_.isEmpty() && error_.isEmpty() && affected_ == false;
    }
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
class TestSynchronousAdocDataSource : public QObject
{
    Q_OBJECT

public:
    TestSynchronousAdocDataSource()
    {
        qRegisterMetaType<AdocTreeNode *>("AdocTreeNode *");
        qRegisterMetaType<Alphabet>("Alphabet");
        qRegisterMetaType<QList<DataRow> >("QList<DataRow>");
        qRegisterMetaType<Msa *>("Msa *");
    }

public slots:
    void __selectError(const QString &error, int tag);
    void __selectReady(const QList<DataRow> &dataRows, int tag);

    void __updateDone(const DataRow &dataRow, bool rowAffected, int tag);
    void __updateError(const QString &error, int tag);

private slots:
    void getDatabase();
    void open();        // Also tests: isOpen(), sourceFile()
    void create();
    void close();
    void readDataTree();
    void saveAs();

    void select();
    void update();

    void setAnonSeqFactories();
    void readMsa();

    // -----------------------------------------------------------
    // -----------------------------------------------------------
    // Obsolete
    /*
    void readSeqs();
    void readSubseqs();
    */
    // -----------------------------------------------------------
    // -----------------------------------------------------------


private:
    //! Creates a sqlite database having the name filename
    bool createDatabase(const QString &filename, const QStringList &queries = QStringList());

    // The following variables are used for testing the returned values of select queries
    SelectResponse selectResponse_;
    UpdateResponse updateResponse_;
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
bool TestSynchronousAdocDataSource::createDatabase(const QString &file, const QStringList &queries)
{
    if (QFile::exists(file))
        QFile::remove(file);

    bool retval = true;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", __FUNCTION__);
        db.setDatabaseName(file);
        if (db.open())
        {
            QSqlQuery query(db);
            foreach (const QString &sql, queries)
            {
                if (!query.prepare(sql))
                {
                    retval = false;
                    break;
                }

                if (!query.exec())
                {
                    retval = false;
                    break;
                }
            }

            db.close();
        }
        else
            retval = false;
    }

    QSqlDatabase::removeDatabase(__FUNCTION__);

    return retval;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Test slots
void TestSynchronousAdocDataSource::getDatabase()
{
    SynchronousAdocDataSource x;

    QVERIFY(x.getDatabase().isOpen() == false);

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    const QString testDbFile = "adoc_db_data_source-test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    x.open(testDbFile);
    QVERIFY(x.isOpen());

    QVERIFY(x.getDatabase().isOpen());
    QVERIFY(x.getDatabase().tables().count() > 0);
    x.close();

    QFile::remove(testDbFile);
}

/**
  * Not testing that after not finding an available connection name after 10 tries (file-1, file-2, .. file-10)
  * it throws an openError :)
  */
void TestSynchronousAdocDataSource::open()
{
    SynchronousAdocDataSource x;

    QVERIFY(x.isOpen() == false);

    QSignalSpy spyOpened(&x, SIGNAL(opened()));
    QSignalSpy spyOpenError(&x, SIGNAL(openError(QString,QString)));

    // ------------------
    // Test: empty string
    x.open("");
    QVERIFY(x.isOpen() == false);
    QVERIFY(QSqlDatabase::connectionNames().isEmpty());

    QVERIFY(spyOpened.isEmpty());
    QCOMPARE(spyOpenError.count(), 1);
    QCOMPARE(spyOpenError.takeFirst().at(0).toString(), QString(""));
    spyOpenError.clear();

    // ------------------------------
    // Test: file that does not exist
    QString nonExistentFile = "non-existent-file";
    QVERIFY(QFile::exists(nonExistentFile) == false);
    x.open(nonExistentFile);
    QVERIFY(x.isOpen() == false);
    QVERIFY(QSqlDatabase::connectionNames().isEmpty());

    QVERIFY(spyOpened.isEmpty());
    QCOMPARE(spyOpenError.count(), 1);
    QCOMPARE(spyOpenError.takeFirst().at(0).toString(), nonExistentFile);
    spyOpenError.clear();

    // ----------------------------
    // Test: file of zero file size
    QString emptyFile = "files/empty_file";
    QVERIFY(QFile::exists(emptyFile));
    QFile fileObj(emptyFile);
    QVERIFY(fileObj.size() == 0);
    x.open(emptyFile);
    QVERIFY(x.isOpen() == false);
    QVERIFY(QSqlDatabase::connectionNames().isEmpty());

    QVERIFY(spyOpened.isEmpty());
    QCOMPARE(spyOpenError.count(), 1);
    QCOMPARE(spyOpenError.takeFirst().at(0).toString(), emptyFile);
    spyOpenError.clear();

    // ---------------------
    // Test: non-sqlite file
    QString makeFile = "files/Makefile";
    QVERIFY(QFile::exists(makeFile));
    x.open(makeFile);
    QVERIFY(x.isOpen() == false);
    QVERIFY(QSqlDatabase::connectionNames().isEmpty());

    QVERIFY(spyOpened.isEmpty());
    QCOMPARE(spyOpenError.count(), 1);
    QCOMPARE(spyOpenError.takeFirst().at(0).toString(), makeFile);
    spyOpenError.clear();

    // ------------------------
    // Test: actual sqlite file - should succeed
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    const QString testDbFile = "adoc_db_data_source-test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    x.open(testDbFile);
    QVERIFY(x.isOpen());
    QCOMPARE(x.sourceFile(), testDbFile);
    QCOMPARE(QSqlDatabase::connectionNames().count(), 1);

    QCOMPARE(spyOpened.count(), 1);
    QCOMPARE(spyOpenError.count(), 0);
    spyOpened.clear();

    // -----------------------------------------------------------
    // Test: attempt to open the same file without first closing it
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    QCOMPARE(x.sourceFile(), testDbFile);
    QCOMPARE(QSqlDatabase::connectionNames().count(), 1);

    QVERIFY(spyOpened.isEmpty());
    QCOMPARE(spyOpenError.count(), 1);
    QCOMPARE(spyOpenError.takeFirst().at(0).toString(), testDbFile);
    spyOpenError.clear();

    // ------------------------------------------------------------------
    // Test: attempt to open another file without first closing open file
    const QString testDbFile2 = "adoc_db_data_source-test2.db";
    if (QFile::exists(testDbFile2))
        QFile::remove(testDbFile2);
    QFile::copy(sourceTestDbFile, testDbFile2);
    x.open(testDbFile2);
    QVERIFY(x.isOpen());
    QCOMPARE(x.sourceFile(), testDbFile);
    QCOMPARE(QSqlDatabase::connectionNames().count(), 1);

    QVERIFY(spyOpened.isEmpty());
    QCOMPARE(spyOpenError.count(), 1);
    QCOMPARE(spyOpenError.takeFirst().at(0).toString(), testDbFile2);
    spyOpenError.clear();

    QFile::remove(testDbFile);
    QFile::remove(testDbFile2);
}

void TestSynchronousAdocDataSource::create()
{
    SynchronousAdocDataSource x;

    QSignalSpy spyCreateError(&x, SIGNAL(createError(QString,QString)));
    QSignalSpy spyCreateDone(&x, SIGNAL(createDone(QString)));
    QVariantList spyArguments;

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // ------------------------------------------------------------------------
    // Two passes, the first with a closed data source, and the second time it is open;
    // results should be the same
    for (int i=0; i<2; ++i)
    {
        if (i == 1)
        {
            // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
            if (QFile::exists(testDbFile))
                QFile::remove(testDbFile);
            QFile::copy(sourceTestDbFile, testDbFile);

            x.open(testDbFile);
            QVERIFY(x.isOpen());
        }

        // Test: empty file name
        x.create("");
        QVERIFY(spyCreateDone.isEmpty());
        QCOMPARE(spyCreateError.count(), 1);
        spyArguments = spyCreateError.takeFirst();
        QVERIFY(spyArguments.at(0).toString().isEmpty());
        QVERIFY(spyArguments.at(1).toString().isEmpty() == false);
        spyCreateError.clear();

        // Test: create with file that already exists
        QString existingFile = "files/emptyExistingFile.db";
        QVERIFY(QFile::exists(existingFile));
        x.create(existingFile);
        QVERIFY(spyCreateDone.isEmpty());
        QCOMPARE(spyCreateError.count(), 1);
        spyArguments = spyCreateError.takeFirst();
        QCOMPARE(spyArguments.at(0).toString(), existingFile);
        QVERIFY(spyArguments.at(1).toString().isEmpty() == false);
        spyCreateError.clear();

        // Test: create with valid, non-existing file
        QString testCreateDbFile = "testCreate.db";
        if (QFile::exists(testCreateDbFile))
            QFile::remove(testCreateDbFile);
        x.create(testCreateDbFile);
        QVERIFY(spyCreateError.isEmpty());
        QCOMPARE(spyCreateDone.count(), 1);
        spyArguments = spyCreateDone.takeFirst();
        QCOMPARE(spyArguments.at(0).toString(), testCreateDbFile);
        QVERIFY(QFile::exists(testCreateDbFile));

        // Test: valid create with one query
        QFile::remove(testCreateDbFile);
        QStringList ddl;
        ddl << "CREATE TABLE astrings (id integer primary key, sequence text not null)";
        x.create(testCreateDbFile, ddl);
        QVERIFY(spyCreateError.isEmpty());
        QCOMPARE(spyCreateDone.count(), 1);
        spyArguments = spyCreateDone.takeFirst();
        QCOMPARE(spyArguments.at(0).toString(), testCreateDbFile);
        QVERIFY(QFile::exists(testCreateDbFile));

        // Check that the database ddl was initiated
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-create");
            db.setDatabaseName(testCreateDbFile);
            QVERIFY(db.open());
            QCOMPARE(db.tables().count(), 1);
            QVERIFY(db.tables().contains("astrings"));
            db.close();
        }
        QSqlDatabase::removeDatabase("test-create");

        // Test: valid create with multiple queries
        QFile::remove(testCreateDbFile);
        ddl << "CREATE TABLE data_tree (id integer primary key, label text, fk_id integer, lft integer not null, rgt integer not null)";
        ddl << "INSERT INTO astrings(sequence) values ('ABC')";
        ddl << "INSERT INTO data_tree (label, fk_id, lft, rgt) values ('Root', 10, 20, 30)";
        x.create(testCreateDbFile, ddl);
        QVERIFY(spyCreateError.isEmpty());
        QCOMPARE(spyCreateDone.count(), 1);
        spyArguments = spyCreateDone.takeFirst();
        QCOMPARE(spyArguments.at(0).toString(), testCreateDbFile);
        QVERIFY(QFile::exists(testCreateDbFile));

        // Check that the database ddl was initiated
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test-create");
            db.setDatabaseName(testCreateDbFile);
            QVERIFY(db.open());
            QCOMPARE(db.tables().count(), 2);
            QVERIFY(db.tables().contains("astrings"));
            QVERIFY(db.tables().contains("data_tree"));

            QSqlQuery query = db.exec("SELECT label, fk_id, lft, rgt FROM data_tree");
            QVERIFY(query.next());
            QCOMPARE(query.value(0).toString(), QString("Root"));
            QCOMPARE(query.value(1).toInt(), 10);
            QCOMPARE(query.value(2).toInt(), 20);
            QCOMPARE(query.value(3).toInt(), 30);

            query = db.exec("SELECT sequence FROM astrings");
            QVERIFY(query.next());
            QCOMPARE(query.value(0).toString(), QString("ABC"));

            // On windows vista, calling db.close somehow retains a lock on testCreateDbFile making
            // it impossible to remove the file for other use.
            // See log.txt, 29 Oct 2010 for details
            //
            // db.close();
        }
        QSqlDatabase::removeDatabase("test-create");

        // Test: create with invalid statement at end
        QVERIFY(QFile::remove(testCreateDbFile));
        ddl << "INSERT INTO missing_table (label, fk_id, lft, rgt) values ('Root', 10, 20, 30)";
        x.create(testCreateDbFile, ddl);
        QVERIFY(spyCreateDone.isEmpty());
        QCOMPARE(spyCreateError.count(), 1);
        spyArguments = spyCreateError.takeFirst();
        QCOMPARE(spyArguments.at(0).toString(), testCreateDbFile);
        QVERIFY(QFile::exists(testCreateDbFile) == false);
    }

    x.close();
    QFile::remove(testDbFile);
}

void TestSynchronousAdocDataSource::close()
{
    SynchronousAdocDataSource x;

    QSignalSpy spyClosed(&x, SIGNAL(closed()));

    // Test: close when not opened, should not emit closed signal
    QVERIFY(x.isOpen() == false);
    QVERIFY(spyClosed.isEmpty());
    x.close();
    QVERIFY(spyClosed.isEmpty());

    // Setup
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    // Test: close file that is actually open
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    QCOMPARE(x.sourceFile(), testDbFile);

    // Verify that the connection name is freed and can reuse the -1 suffix
    QVERIFY(QSqlDatabase::database(testDbFile + "-1", false).isValid());
    x.close();
    QVERIFY(x.isOpen() == false);
    QVERIFY(x.sourceFile().isEmpty());
    QVERIFY(QSqlDatabase::database(testDbFile + "-1", false).isValid() == false);
    QCOMPARE(spyClosed.count(), 1);
    spyClosed.clear();

    // Verify that we can open it again
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    QCOMPARE(x.sourceFile(), testDbFile);
    x.close();
    QCOMPARE(spyClosed.count(), 1);
    spyClosed.clear();
    QVERIFY(x.isOpen() == false);
    QVERIFY(x.sourceFile().isEmpty());

    QFile::remove(testDbFile);

    // ------------------------------------------------------------------------
    // Test: on deletion, the closed signal should be emitted only if it is open!
    SynchronousAdocDataSource *y = new SynchronousAdocDataSource();
    QSignalSpy spyClosed2(y, SIGNAL(closed()));
    delete y;
    y = 0;
    QVERIFY(spyClosed2.isEmpty());

    y = new SynchronousAdocDataSource();
    QSignalSpy spyClosed3(y, SIGNAL(closed()));
    y->open(sourceTestDbFile);
    QVERIFY(y->isOpen());
    delete y;
    y = 0;
    QCOMPARE(spyClosed3.count(), 1);
}

void TestSynchronousAdocDataSource::readDataTree()
{
    QString testDbFile = "files/readDataTree.db";
    SynchronousAdocDataSource x;

    QSignalSpy spyDataTreeReady(&x, SIGNAL(dataTreeReady(AdocTreeNode *)));
    QSignalSpy spyDataTreeError(&x, SIGNAL(dataTreeError(QString)));

    // ------------------------------------------------------------------------
    // Test: data tree when data source has not been opened
    x.readDataTree();
    QVERIFY(spyDataTreeReady.isEmpty());
    QCOMPARE(spyDataTreeError.count(), 1);
    spyDataTreeError.clear();

    // ------------------------------------------------------------------------
    // Test: data tree with invalid table structure
    QVERIFY(createDatabase(testDbFile,
                           QStringList() << "CREATE table bad_data_tree (id integer primary key, name text not null)"));
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    x.readDataTree();
    QVERIFY(spyDataTreeReady.isEmpty());
    QCOMPARE(spyDataTreeError.count(), 1);
    spyDataTreeError.clear();
    x.close();

    // ------------------------------------------------------------------------
    // Test: valid data tree table but no rows
    QString dataTreeSql = QString("CREATE table %1 (id integer primary key autoincrement,"
                                  "type text not null,"
                                  "fk_id integer,"
                                  "label text,"
                                  "lft integer not null,"
                                  "rgt integer not null)").arg(constants::kTableDataTree);
    QVERIFY(createDatabase(testDbFile, QStringList() << dataTreeSql));
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    x.readDataTree();
    QCOMPARE(spyDataTreeReady.count(), 1);
    AdocTreeNode *root = qvariant_cast<AdocTreeNode *>(spyDataTreeReady.takeFirst().at(0));
    QVERIFY(root);
    QCOMPARE(root->nodeType_, AdocTreeNode::RootType);
    QCOMPARE(root->childCount(), 0);
    QVERIFY(root->parent() == 0);
    QCOMPARE(root->fkId_, 0);
    QCOMPARE(spyDataTreeError.count(), 0);
    spyDataTreeReady.clear();
    x.close();
    delete root;
    root = 0;

    // ------------------------------------------------------------------------
    // Test: valid data tree table but invalid mptt state
    QStringList invalidMpttData;
    invalidMpttData << QString("INSERT INTO %1 (type, fk_id, label, lft, rgt) values ('root',     NULL, 'root', 1, 3000)").arg(constants::kTableDataTree)
                    << QString("INSERT INTO %1 (type, fk_id, label, lft, rgt) values ('primer',   10,   'A',    2, 3)").arg(constants::kTableDataTree)
                    << QString("INSERT INTO %1 (type, fk_id, label, lft, rgt) values ('group',    NULL, 'B',    4, 7)").arg(constants::kTableDataTree)
                    << QString("INSERT INTO %1 (type, fk_id, label, lft, rgt) values ('seqamino', 99,   'C',    5, 6)").arg(constants::kTableDataTree);
    QVERIFY(createDatabase(testDbFile, QStringList() << dataTreeSql << invalidMpttData));
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    x.readDataTree();
    QVERIFY(spyDataTreeReady.isEmpty());
    QCOMPARE(spyDataTreeError.count(), 1);
    spyDataTreeError.clear();
    x.close();

    // ------------------------------------------------------------------------
    // Test: valid data tree table but valid mptt state
    QStringList validMpttData;
    validMpttData << QString("INSERT INTO %1 (type, fk_id, label, lft, rgt) values ('root',     NULL, 'root', 1, 8)").arg(constants::kTableDataTree)
                  << QString("INSERT INTO %1 (type, fk_id, label, lft, rgt) values ('primer',   10,   'A',    2, 3)").arg(constants::kTableDataTree)
                  << QString("INSERT INTO %1 (type, fk_id, label, lft, rgt) values ('group',    NULL, 'B',    4, 7)").arg(constants::kTableDataTree)
                  << QString("INSERT INTO %1 (type, fk_id, label, lft, rgt) values ('seqamino', 99,   'C',    5, 6)").arg(constants::kTableDataTree);
    QVERIFY(createDatabase(testDbFile, QStringList() << dataTreeSql << validMpttData));
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    x.readDataTree();
    QCOMPARE(spyDataTreeReady.count(), 1);

    root = qvariant_cast<AdocTreeNode *>(spyDataTreeReady.takeFirst().at(0));
    QVERIFY(root);
    QCOMPARE(root->nodeType_, AdocTreeNode::RootType);
    QCOMPARE(root->fkId_, 0);
    QCOMPARE(root->label_, QString("root"));
    QCOMPARE(root->childCount(), 2);

    AdocTreeNode *a = static_cast<AdocTreeNode *>(root->childAt(0));
    QCOMPARE(a->nodeType_, AdocTreeNode::PrimerType);
    QCOMPARE(a->fkId_, 10);
    QCOMPARE(a->label_, QString("A"));
    QVERIFY(a->parent() == root);
    QCOMPARE(a->childCount(), 0);

    AdocTreeNode *b = static_cast<AdocTreeNode *>(root->childAt(1));
    QCOMPARE(b->nodeType_, AdocTreeNode::GroupType);
    QCOMPARE(b->fkId_, 0);
    QCOMPARE(b->label_, QString("B"));
    QVERIFY(b->parent() == root);
    QCOMPARE(b->childCount(), 1);

    AdocTreeNode *c = static_cast<AdocTreeNode *>(b->childAt(0));
    QCOMPARE(c->nodeType_, AdocTreeNode::SeqAminoType);
    QCOMPARE(c->fkId_, 99);
    QCOMPARE(c->label_, QString("C"));
    QVERIFY(c->parent() == b);
    QCOMPARE(c->childCount(), 0);
    QCOMPARE(spyDataTreeError.count(), 0);
    spyDataTreeReady.clear();
    x.close();

    // Maintain pointer sanity
    delete root;
    root = 0;
    a = 0;
    b = 0;
    c = 0;

    QFile::remove(testDbFile);
}

void TestSynchronousAdocDataSource::saveAs()
{
    SynchronousAdocDataSource x;

    QSignalSpy spySaveAsDone(&x, SIGNAL(saveAsDone(QString)));
    QSignalSpy spySaveAsError(&x, SIGNAL(saveAsError(QString,QString)));
    QVariantList spyArguments;

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    // ------------------------------------------------------------------------
    // Test: attempt to saveAs with empty file and no open file
    x.saveAs("");
    QVERIFY(spySaveAsDone.isEmpty());
    QCOMPARE(spySaveAsError.count(), 1);
    spyArguments = spySaveAsError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty());
    QVERIFY(spyArguments.at(1).toString().isEmpty() == false);
    spySaveAsError.clear();

    // ------------------------------------------------------------------------
    // Test: saveAs with non-empty file, but lacking open file
    QString testSaveAsFile = "testSaveAsTarget.db";
    x.saveAs(testSaveAsFile);
    QVERIFY(spySaveAsDone.isEmpty());
    QCOMPARE(spySaveAsError.count(), 1);
    spyArguments = spySaveAsError.takeFirst();
    QCOMPARE(spyArguments.at(0).toString(), testSaveAsFile);
    QVERIFY(spyArguments.at(1).toString().isEmpty() == false);
    spySaveAsError.clear();
    QVERIFY(QFile::exists(testSaveAsFile) == false);

    // ------------------------------------------------------------------------
    // Test: saveAs with empty file name and open file
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    x.saveAs("");
    QCOMPARE(x.sourceFile(), testDbFile);
    QVERIFY(spySaveAsDone.isEmpty());
    QCOMPARE(spySaveAsError.count(), 1);
    spyArguments = spySaveAsError.takeFirst();
    QVERIFY(spyArguments.at(0).toString().isEmpty());
    QVERIFY(spyArguments.at(1).toString().isEmpty() == false);
    spySaveAsError.clear();

    // ------------------------------------------------------------------------
    // Context: x is still open
    // Test: saveAs to existing database file, existing text file, invalid filename
    QStringList fileNames;
    fileNames << "files/preexisting_database.db"
              << "files/preexisting_text_file"
              << "lkfasdf. fedk39%#%#$^ // \\lkj3@#$..db";
    foreach (const QString &fileName, fileNames)
    {
        x.saveAs(fileName);
        QCOMPARE(x.sourceFile(), testDbFile);
        QVERIFY(spySaveAsDone.isEmpty());
        QCOMPARE(spySaveAsError.count(), 1);
        spyArguments = spySaveAsError.takeFirst();
        QCOMPARE(spyArguments.at(0).toString(), fileName);
        QVERIFY(spyArguments.at(1).toString().isEmpty() == false);
        spySaveAsError.clear();
    }

    // ------------------------------------------------------------------------
    // Test: saveAs with file in directory that does not exist
    QVERIFY(QFile::exists("missing_directory") == false);
    x.saveAs("missing_directory/my_database_file.db");
    QCOMPARE(x.sourceFile(), testDbFile);
    QVERIFY(spySaveAsDone.isEmpty());
    QCOMPARE(spySaveAsError.count(), 1);
    spyArguments = spySaveAsError.takeFirst();
    QCOMPARE(spyArguments.at(0).toString(), QString("missing_directory/my_database_file.db"));
    QVERIFY(spyArguments.at(1).toString().isEmpty() == false);
    spySaveAsError.clear();

    // ------------------------------------------------------------------------
    // Test: saveAs with no additional changes
    QString db_file2 = "files/simple_valid2.db";
    QFile::remove(db_file2);
    x.saveAs(db_file2);
    QVERIFY(x.isOpen());
    QCOMPARE(x.sourceFile(), db_file2);
    QVERIFY(spySaveAsError.isEmpty());
    QCOMPARE(spySaveAsDone.count(), 1);
    spyArguments = spySaveAsDone.takeFirst();
    QCOMPARE(spyArguments.at(0).toString(), db_file2);
    spySaveAsDone.clear();
    x.close();

    // Check for the proper files on the file system
    QVERIFY(QFile::exists(testDbFile));
    QVERIFY(QFile::exists(db_file2));

    // Manually check that there is data inside both files
    QStringList tableLists[2];
    QHash<QString, int> rowCounts[2];
    QStringList files;
    files << testDbFile << db_file2;
    for (int i=0; i<2; ++i)
    {
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
            db.setDatabaseName(files.at(i));
            QVERIFY(db.open());
            tableLists[i] = db.tables();

            foreach (const QString &tableName, tableLists[i])
            {
                QSqlQuery s_row_count = db.exec(QString("SELECT count(*) FROM %1").arg(tableName));
                QVERIFY(s_row_count.next());
                rowCounts[i].insert(tableName, s_row_count.value(0).toInt());
            }
            db.close();
        }
        QSqlDatabase::removeDatabase("test_db");
    }

    QVERIFY(tableLists[0] == tableLists[1]);
    QVERIFY(rowCounts[0] == rowCounts[1]);

    QFile::remove(db_file2);

    // ------------------------------------------------------------------------
    // Test: saveAs with some changes
    x.open(testDbFile);
    QVERIFY(x.isOpen());
    QCOMPARE(x.sourceFile(), testDbFile);

    // Simulate a transaction by utilizing our friend status to access the database
    QVERIFY(x.database_.transaction());
    QVERIFY(x.database_.exec("DELETE FROM data_tree").lastError().type() == QSqlError::NoError);

    x.saveAs(db_file2);
    QVERIFY(x.isOpen());
    QCOMPARE(x.sourceFile(), db_file2);
    QVERIFY(spySaveAsError.isEmpty());
    QCOMPARE(spySaveAsDone.count(), 1);
    spyArguments = spySaveAsDone.takeFirst();
    QCOMPARE(spyArguments.at(0).toString(), db_file2);
    spySaveAsDone.clear();
    x.close();

    // Check for the proper files on the file system
    QVERIFY(QFile::exists(testDbFile));
    QVERIFY(QFile::exists(db_file2));

    // Manually check that there is data inside both files
    tableLists[0].clear();
    tableLists[1].clear();
    rowCounts[0].clear();
    rowCounts[1].clear();
    files.clear();
    files << testDbFile << db_file2;
    for (int i=0; i<2; ++i)
    {
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
            db.setDatabaseName(files.at(i));
            QVERIFY(db.open());
            tableLists[i] = db.tables();

            foreach (const QString &tableName, tableLists[i])
            {
                QSqlQuery s_row_count = db.exec(QString("SELECT count(*) FROM %1").arg(tableName));
                QVERIFY(s_row_count.next());
                rowCounts[i].insert(tableName, s_row_count.value(0).toInt());
            }
            db.close();
        }
        QSqlDatabase::removeDatabase("test_db");
    }

    // Number of tables should not have changed
    QVERIFY(tableLists[0] == tableLists[1]);

    // Check that data_tree has none in db_file2 but all of them in testDbFile
    QVERIFY(rowCounts[0].contains("data_tree"));
    QCOMPARE(rowCounts[0]["data_tree"], 10);

    QVERIFY(rowCounts[1].contains("data_tree"));
    QCOMPARE(rowCounts[1]["data_tree"], 0);

    rowCounts[0].remove("data_tree");
    rowCounts[1].remove("data_tree");

    QVERIFY(rowCounts[0] == rowCounts[1]);

    QFile::remove(db_file2);
}

void TestSynchronousAdocDataSource::select()
{
    SynchronousAdocDataSource x;

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    // ------------------------------------------------------------------------
    // Suite: unopened data source
    QVERIFY(x.isOpen() == false);

    // Test: empty sql
    selectResponse_.clear();
    x.select("", QList<QVariant>() << 1 << 2, this, "__selectReady", "__selectError");
    QVERIFY(selectResponse_.error_.isEmpty() == false);
    QVERIFY(selectResponse_.dataRows_.isEmpty());
    QCOMPARE(selectResponse_.tag_, 0);

    // Test: valid sql, no ids
    selectResponse_.clear();
    x.select(QString("SELECT id, lft, rgt FROM %1").arg(constants::kTableDataTree), QList<QVariant>(), this, "__selectReady", "__selectError", 10);
    QVERIFY(selectResponse_.error_.isEmpty() == false);
    QVERIFY(selectResponse_.dataRows_.isEmpty());
    QCOMPARE(selectResponse_.tag_, 10);

    // Test: valid sql, ids
    x.select(QString("SELECT digest FROM %1").arg(constants::kTableAstrings), QList<QVariant>() << 2 << 3, this, "__selectReady", "__selectError", 50);
    QVERIFY(selectResponse_.error_.isEmpty() == false);
    QVERIFY(selectResponse_.dataRows_.isEmpty());
    QCOMPARE(selectResponse_.tag_, 50);

    // ------------------------------------------------------------------------
    // Suite: opened data source
    x.open(testDbFile);
    QVERIFY(x.isOpen());

    // Clear any select response
    selectResponse_.clear();

    // ------------------------------------------------------------------------
    // Test: empty SQL
    x.select("", QList<QVariant>() << 1 << 2, this, "__selectReady", "__selectError");
    QVERIFY(selectResponse_.error_.isEmpty() == false);
    QVERIFY(selectResponse_.dataRows_.isEmpty());
    QCOMPARE(selectResponse_.tag_, 0);

    // ------------------------------------------------------------------------
    // Test: valid sql, no ids, tag
    selectResponse_.clear();
    x.select(QString("SELECT id, lft, rgt FROM %1 WHERE id = ?").arg(constants::kTableDataTree), QList<QVariant>(), this, "__selectReady", "__selectError", 10);
    QVERIFY(selectResponse_.error_.isEmpty() == false);
    QVERIFY(selectResponse_.dataRows_.isEmpty());
    QCOMPARE(selectResponse_.tag_, 10);

    // ------------------------------------------------------------------------
    // Test: invalid sql, ids, tag
    selectResponse_.clear();
    x.select("SELECT id, lft, rgt FROM missing_table WHERE id = ?", QList<QVariant>() << 1, this, "__selectReady", "__selectError", 20);
    QVERIFY(selectResponse_.error_.isEmpty() == false);
    QVERIFY(selectResponse_.dataRows_.isEmpty());
    QCOMPARE(selectResponse_.tag_, 20);

    // ------------------------------------------------------------------------
    // Test: valid sql, ids that do not match any records, tag
    selectResponse_.clear();
    x.select(QString("SELECT id, lft, rgt FROM %1 WHERE id = ?").arg(constants::kTableDataTree), QList<QVariant>() << -100, this, "__selectReady", "__selectError", 30);
    QCOMPARE(selectResponse_.error_, QString());
    QVERIFY(selectResponse_.dataRows_.isEmpty());
    QCOMPARE(selectResponse_.tag_, 30);

    // ------------------------------------------------------------------------
    // Test: valid sql, ids that partially match (some do, some do not) records, tag
    selectResponse_.clear();
    x.select(QString("SELECT id, lft, rgt, label FROM %1 WHERE id = ?").arg(constants::kTableDataTree), QList<QVariant>() << -100 << 5, this, "__selectReady", "__selectError", -40);
    QCOMPARE(selectResponse_.error_, QString());
    QCOMPARE(selectResponse_.tag_, -40);
    QCOMPARE(selectResponse_.dataRows_.count(), 1);
    QCOMPARE(selectResponse_.dataRows_.at(0).count(), 4);
    QCOMPARE(selectResponse_.dataRows_.at(0).id_.toInt(), 5);
    QCOMPARE(selectResponse_.dataRows_.at(0).value(0).toInt(), 5);
    QCOMPARE(selectResponse_.dataRows_.at(0).value(1).toInt(), 8);
    QCOMPARE(selectResponse_.dataRows_.at(0).value(2).toInt(), 13);
    QCOMPARE(selectResponse_.dataRows_.at(0).value(3).toString(), QString("Delta"));

    // ------------------------------------------------------------------------
    // Test: alternate table
    selectResponse_.clear();
    x.select(QString("SELECT digest FROM %1 WHERE id = ?").arg(constants::kTableAstrings), QList<QVariant>() << 2 << 3, this, "__selectReady", "__selectError", 50);
    QCOMPARE(selectResponse_.error_, QString());
    QCOMPARE(selectResponse_.tag_, 50);
    QCOMPARE(selectResponse_.dataRows_.count(), 2);
    QCOMPARE(selectResponse_.dataRows_.at(0).count(), 1);
    QVERIFY(selectResponse_.dataRows_.at(0).id_.isNull());
    QCOMPARE(selectResponse_.dataRows_.at(0).value(0).toString(), QString("/H7/5HS9vDhXRYn88h7ksiZf3I4="));
    QVERIFY(selectResponse_.dataRows_.at(1).id_.isNull());
    QCOMPARE(selectResponse_.dataRows_.at(1).value(0).toString(), QString("DsV+iExTk5eyjf+l6A2rQCBiEP0="));

    // ------------------------------------------------------------------------
    // Suite: instances in which nothing is done by select
    // Test: receiver is not defined
    selectResponse_.clear();
    selectResponse_.tag_ = 1000;
    x.select(QString("SELECT digest FROM %1 WHERE id = ?").arg(constants::kTableAstrings), QList<QVariant>() << 2 << 3, NULL, "__selectReady", "__selectError", -1000);
    QCOMPARE(selectResponse_.error_, QString());
    QCOMPARE(selectResponse_.tag_, 1000);
    QCOMPARE(selectResponse_.dataRows_.count(), 0);

    // Test: method, errorMethod are not defined
    x.select(QString("SELECT digest FROM %1 WHERE id = ?").arg(constants::kTableAstrings), QList<QVariant>() << 2 << 3, this, NULL, NULL, -1000);
    QCOMPARE(selectResponse_.error_, QString());
    QCOMPARE(selectResponse_.tag_, 1000);
    QCOMPARE(selectResponse_.dataRows_.count(), 0);

    // ------------------------------------------------------------------------
    // Test: only method is defined, error case
    selectResponse_.clear();
    x.select("SELECT digest FROM missing_table WHERE id = ?", QList<QVariant>() << 2 << 3, this, "__selectReady", NULL, -1000);
    QCOMPARE(selectResponse_.error_, QString());
    QCOMPARE(selectResponse_.tag_, 0);
    QCOMPARE(selectResponse_.dataRows_.count(), 0);

    // ------------------------------------------------------------------------
    // Test: only method is defined, success case
    selectResponse_.clear();
    x.select(QString("SELECT digest FROM %1 WHERE id = ?").arg(constants::kTableAstrings), QList<QVariant>() << 2 << 3, this, "__selectReady", NULL, 2000);
    QCOMPARE(selectResponse_.error_, QString());
    QCOMPARE(selectResponse_.tag_, 2000);
    QCOMPARE(selectResponse_.dataRows_.count(), 2);
    QCOMPARE(selectResponse_.dataRows_.at(0).count(), 1);
    QVERIFY(selectResponse_.dataRows_.at(0).id_.isNull());
    QCOMPARE(selectResponse_.dataRows_.at(0).value(0).toString(), QString("/H7/5HS9vDhXRYn88h7ksiZf3I4="));
    QVERIFY(selectResponse_.dataRows_.at(1).id_.isNull());
    QCOMPARE(selectResponse_.dataRows_.at(1).value(0).toString(), QString("DsV+iExTk5eyjf+l6A2rQCBiEP0="));

    // ------------------------------------------------------------------------
    // Test: only errorMethod is defined, error case
    selectResponse_.clear();
    x.select("SELECT digest FROM missing_table WHERE id = ?", QList<QVariant>() << 2, this, NULL, "__selectError", -3000);
    QVERIFY(selectResponse_.error_.isEmpty() == false);
    QCOMPARE(selectResponse_.tag_, -3000);
    QCOMPARE(selectResponse_.dataRows_.count(), 0);

    // ------------------------------------------------------------------------
    // Test: only errorMethod is defined, success case
    selectResponse_.clear();
    x.select(QString("SELECT digest FROM %1 WHERE id = ?").arg(constants::kTableAstrings), QList<QVariant>() << 2 << 3, this, NULL, "__selectError", 4000);
    QCOMPARE(selectResponse_.error_, QString());
    QCOMPARE(selectResponse_.tag_, 0);
    QCOMPARE(selectResponse_.dataRows_.count(), 0);
}

void TestSynchronousAdocDataSource::__selectError(const QString &error, int tag)
{
    selectResponse_.error_ = error;
    selectResponse_.tag_ = tag;
}

void TestSynchronousAdocDataSource::__selectReady(const QList<DataRow> &dataRows, int tag)
{
    selectResponse_.dataRows_ = dataRows;
    selectResponse_.tag_ = tag;
}

void TestSynchronousAdocDataSource::update()
{
    SynchronousAdocDataSource x;

    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    // ------------------------------------------------------------------------
    // Suite: unopened data source
    QVERIFY(x.isOpen() == false);

    // Test: empty DataRow without any id defined
    DataRow astring;
    updateResponse_.clear();
    x.update(astring, constants::kTableAstrings, QStringList(), this, "__updateDone", "__updateError");
    QVERIFY(updateResponse_.error_.isEmpty() == false);
    QVERIFY(updateResponse_.dataRow_.isEmpty());
    QVERIFY(updateResponse_.affected_ == false);
    QCOMPARE(updateResponse_.tag_, 0);

    astring.id_ = 1;
    updateResponse_.clear();
    x.update(astring, constants::kTableAstrings, QStringList() << "field1", this, "__updateDone", "__updateError");
    QVERIFY(updateResponse_.error_.isEmpty() == false);
    QVERIFY(updateResponse_.affected_ == false);
    QVERIFY(updateResponse_.dataRow_.isEmpty());
    QCOMPARE(updateResponse_.tag_, 0);

    // ------------------------------------------------------------------------
    // Suite: opened data source
    x.open(testDbFile);
    QVERIFY(x.isOpen());

    // ------------------------------------------------------------------------
    // Test: empty data row, invalid table, no fields
    updateResponse_.clear();
    {
        DataRow dataRow;
        x.update(dataRow, "invalid", QStringList(), this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty() == false);
        QVERIFY(updateResponse_.affected_ == false);
        QVERIFY(updateResponse_.dataRow_.isEmpty());
        QCOMPARE(updateResponse_.tag_, 0);
    }

    // Test: data row with an id, invalid table, no fields, tag
    updateResponse_.clear();
    {
        DataRow dataRow(5);
        x.update(dataRow, "invalid", QStringList(), this, "__updateDone", "__updateError", 5);
        QVERIFY(updateResponse_.error_.isEmpty() == false);
        QVERIFY(updateResponse_.affected_ == false);
        QVERIFY(updateResponse_.dataRow_.isEmpty());
        QCOMPARE(updateResponse_.tag_, 5);
    }

    // Test: data row with id, valid table, no fields
    updateResponse_.clear();
    {
        DataRow dataRow(5);
        x.update(dataRow, constants::kTableAstrings, QStringList(), this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty() == false);
        QVERIFY(updateResponse_.dataRow_.isEmpty());
        QCOMPARE(updateResponse_.tag_, 0);
    }

    // Test: data row with id, valid table, invalid field
    updateResponse_.clear();
    {
        DataRow dataRow(1);
        dataRow.setValue("missing", "DEF");
        x.update(dataRow, constants::kTableAstrings, QStringList(), this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty() == false);
        QVERIFY(updateResponse_.affected_ == false);
        QVERIFY(updateResponse_.dataRow_.isEmpty());
        QCOMPARE(updateResponse_.tag_, 0);
    }

    // Test: same as above, but without id; however, referencing valid field
    updateResponse_.clear();
    {
        DataRow dataRow;
        dataRow.setValue("sequence", "DEF");
        x.update(dataRow, constants::kTableAstrings, QStringList(), this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty() == false);
        QVERIFY(updateResponse_.affected_ == false);
        QVERIFY(updateResponse_.dataRow_.isEmpty());
        QCOMPARE(updateResponse_.tag_, 0);
    }

    // Test: update with non-empty id that does not reference actual row; however, all other parameters are fine
    updateResponse_.clear();
    {
        // Verify that there is no row with an id of 100 in astrings
        int id = 100;
        QSqlQuery query = x.database_.exec(QString("SELECT * FROM %1 WHERE id = %2").arg(constants::kTableAstrings).arg(id));
        QVERIFY(query.lastError().type() == QSqlError::NoError);
        QVERIFY(query.next() == false);

        DataRow dataRow(id);
        dataRow.setValue("sequence", "DEF");
        x.update(dataRow, constants::kTableAstrings, QStringList(), this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty());
        QVERIFY(updateResponse_.affected_ == false);
        QCOMPARE(updateResponse_.dataRow_.value("sequence").toString(), QString("DEF"));
        QCOMPARE(updateResponse_.tag_, 0);
    }

    // Check that there is an astring record of 1
    QSqlQuery query = x.database_.exec(QString("SELECT * FROM %1 WHERE id = %2").arg(constants::kTableAstrings).arg(1));
    QVERIFY(query.lastError().type() == QSqlError::NoError);
    QVERIFY(query.next());
    QSqlRecord result = query.record();

    // Test: update that specifies the wrong fields should fail
    updateResponse_.clear();
    {
        QVERIFY(result.value("sequence").toString() != QString("DEF"));

        DataRow dataRow(1);
        dataRow.setValue("sequence", "DEF");

        x.update(dataRow, constants::kTableAstrings, QStringList() << "invalid", this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty() == false);
        QVERIFY(updateResponse_.affected_ == false);
        QVERIFY(updateResponse_.dataRow_.isEmpty());
        QCOMPARE(updateResponse_.tag_, 0);
    }

    // Test: update with empty tablename should fail
    updateResponse_.clear();
    {
        DataRow dataRow(1);
        dataRow.setValue("sequence", "XYZ");
        x.update(dataRow, QString(), QStringList(), this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty() == false);
        QVERIFY(updateResponse_.affected_ == false);
        QVERIFY(updateResponse_.dataRow_.isEmpty());
        QCOMPARE(updateResponse_.tag_, 0);
    }

    // Test: update with valid data
    updateResponse_.clear();
    {
        DataRow dataRow(1);
        dataRow.setValue("sequence", "DEF");

        x.update(dataRow, constants::kTableAstrings, QStringList(), this, "__updateDone", "__updateError", 100);
        QVERIFY(updateResponse_.error_.isEmpty());
        QVERIFY(updateResponse_.affected_);
        QCOMPARE(updateResponse_.dataRow_, dataRow);
        QCOMPARE(updateResponse_.tag_, 100);

        // Check that the database was updated
        query = x.database_.exec(QString("SELECT * FROM %1 WHERE id = %2").arg(constants::kTableAstrings).arg(1));
        QVERIFY(query.lastError().type() == QSqlError::NoError);
        QVERIFY(query.next());
        QCOMPARE(query.record().value("sequence").toString(), QString("DEF"));
    }

    // Test: update should work even if response methods are not defined
    updateResponse_.clear();
    {
        DataRow dataRow(1);
        dataRow.setValue("sequence", "GHI");

        x.update(dataRow, constants::kTableAstrings);
        QVERIFY(updateResponse_.error_.isEmpty());
        QVERIFY(updateResponse_.affected_ == false);
        QVERIFY(updateResponse_.dataRow_.isEmpty());   // Because we did not configure the response handlers
        QCOMPARE(updateResponse_.tag_, 0);

        // Check that the database was updated
        query = x.database_.exec(QString("SELECT * FROM %1 WHERE id = %2").arg(constants::kTableAstrings).arg(1));
        QVERIFY(query.lastError().type() == QSqlError::NoError);
        QVERIFY(query.next());
        QCOMPARE(query.record().value("sequence").toString(), QString("GHI"));
    }

    // Test: update multiple fields simultaneously
    updateResponse_.clear();
    {
        DataRow dataRow(2);
        dataRow.setValue("digest", "__blank__");
        dataRow.setValue("sequence", "JKL");

        x.update(dataRow, constants::kTableAstrings, QStringList(), this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty());
        QVERIFY(updateResponse_.affected_);
        QCOMPARE(updateResponse_.dataRow_, dataRow);
        QCOMPARE(updateResponse_.tag_, 0);

        // Check that the database was updated
        query = x.database_.exec(QString("SELECT * FROM %1 WHERE id = %2").arg(constants::kTableAstrings).arg(dataRow.id_.toInt()));
        QVERIFY(query.lastError().type() == QSqlError::NoError);
        QVERIFY(query.next());
        QCOMPARE(query.record().value("sequence").toString(), QString("JKL"));
        QCOMPARE(query.record().value("digest").toString(), QString("__blank__"));
    }

    // Test: have multiple fields set, but only update those specified
    updateResponse_.clear();
    {
        DataRow dataRow(2);
        dataRow.setValue("digest", "__base64__");
        dataRow.setValue("sequence", "MNO");

        x.update(dataRow, constants::kTableAstrings, QStringList() << "digest", this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty());
        QVERIFY(updateResponse_.affected_);
        QCOMPARE(updateResponse_.dataRow_, dataRow);
        QCOMPARE(updateResponse_.tag_, 0);

        // Check that the database was updated
        query = x.database_.exec(QString("SELECT * FROM %1 WHERE id = %2").arg(constants::kTableAstrings).arg(dataRow.id_.toInt()));
        QVERIFY(query.lastError().type() == QSqlError::NoError);
        QVERIFY(query.next());
        QCOMPARE(query.record().value("sequence").toString(), QString("JKL"));
        QCOMPARE(query.record().value("digest").toString(), QString("__base64__"));
    }

    // Test: update id, should come back changed
    updateResponse_.clear();
    {
        DataRow dataRow(2);
        dataRow.setValue("id", 99);

        x.update(dataRow, constants::kTableAstrings, QStringList(), this, "__updateDone", "__updateError");
        QVERIFY(updateResponse_.error_.isEmpty());
        QVERIFY(updateResponse_.affected_);
        QCOMPARE(updateResponse_.dataRow_.id_.toInt(), 99);
        QCOMPARE(updateResponse_.tag_, 0);

        // Check that the database was updated
        query = x.database_.exec(QString("SELECT * FROM %1 WHERE id = %2").arg(constants::kTableAstrings).arg(99));
        QVERIFY(query.lastError().type() == QSqlError::NoError);
        QVERIFY(query.next());
        QCOMPARE(query.record().value("sequence").toString(), QString("JKL"));
        QCOMPARE(query.record().value("digest").toString(), QString("__base64__"));
    }

    // Test: successful update should work even without response handlers
    updateResponse_.clear();
    {
        DataRow dataRow(99);
        dataRow.setValue("id", 2);
        dataRow.setValue("sequence", "MNO");

        x.update(dataRow, constants::kTableAstrings);
        // Check that the database was updated
        query = x.database_.exec(QString("SELECT * FROM %1 WHERE id = %2").arg(constants::kTableAstrings).arg(2));
        QVERIFY(query.lastError().type() == QSqlError::NoError);
        QVERIFY(query.next());
        QCOMPARE(query.record().value("sequence").toString(), QString("MNO"));
    }
}

void TestSynchronousAdocDataSource::__updateDone(const DataRow &dataRow, bool rowAffected, int tag)
{
    updateResponse_.affected_ = rowAffected;
    updateResponse_.dataRow_ = dataRow;
    updateResponse_.tag_ = tag;
}

void TestSynchronousAdocDataSource::__updateError(const QString &error, int tag)
{
    updateResponse_.error_ = error;
    updateResponse_.tag_ = tag;
}

void TestSynchronousAdocDataSource::setAnonSeqFactories()
{
    SynchronousAdocDataSource *x = new SynchronousAdocDataSource(this);

    QVERIFY(x->anonSeqFactories().isEmpty());

    // Test: add a zero factory pointer
    x->setAnonSeqFactory(eAminoAlphabet, 0);
    QVERIFY(x->anonSeqFactories().isEmpty());

    // Test: add a valid factory
    DbAnonSeqFactory *aminoFactory = new DbAnonSeqFactory();
    x->setAnonSeqFactory(eAminoAlphabet, aminoFactory);
    QCOMPARE(x->anonSeqFactories().size(), 1);
    QVERIFY(x->anonSeqFactories().contains(eAminoAlphabet));
    QVERIFY(x->anonSeqFactories().value(eAminoAlphabet) == aminoFactory);
    QVERIFY(x->anonSeqFactories().value(eAminoAlphabet)->adocDataSource() == x);

    // Test: add another valid factory over the current one
    aminoFactory = 0;
    DbAnonSeqFactory *aminoFactory2 = new DbAnonSeqFactory();
    x->setAnonSeqFactory(eAminoAlphabet, aminoFactory2);
    QCOMPARE(x->anonSeqFactories().size(), 1);
    QVERIFY(x->anonSeqFactories().contains(eAminoAlphabet));
    QVERIFY(x->anonSeqFactories().value(eAminoAlphabet) == aminoFactory2);
    QVERIFY(x->anonSeqFactories().value(eAminoAlphabet)->adocDataSource() == x);

    // Test: zero out amino factory
    aminoFactory2 = 0;
    x->setAnonSeqFactory(eAminoAlphabet, 0);
    QCOMPARE(x->anonSeqFactories().size(), 0);

    // Test: add three factories and check that they return the right ones
    DbAnonSeqFactory *dnaFactory = new DbAnonSeqFactory();
    DbAnonSeqFactory *rnaFactory = new DbAnonSeqFactory();
    DbAnonSeqFactory *aminoFactory3 = new DbAnonSeqFactory();
    x->setAnonSeqFactory(eDnaAlphabet, dnaFactory);
    x->setAnonSeqFactory(eRnaAlphabet, rnaFactory);
    x->setAnonSeqFactory(eAminoAlphabet, aminoFactory3);
    QCOMPARE(x->anonSeqFactories().size(), 3);
    QVERIFY(x->anonSeqFactories().contains(eDnaAlphabet));
    QVERIFY(x->anonSeqFactories().value(eDnaAlphabet) == dnaFactory);
    QVERIFY(x->anonSeqFactories().value(eDnaAlphabet)->adocDataSource() == x);
    QVERIFY(x->anonSeqFactories().contains(eRnaAlphabet));
    QVERIFY(x->anonSeqFactories().value(eRnaAlphabet) == rnaFactory);
    QVERIFY(x->anonSeqFactories().value(eRnaAlphabet)->adocDataSource() == x);
    QVERIFY(x->anonSeqFactories().contains(eAminoAlphabet));
    QVERIFY(x->anonSeqFactories().value(eAminoAlphabet) == aminoFactory3);
    QVERIFY(x->anonSeqFactories().value(eAminoAlphabet)->adocDataSource() == x);
}

void TestSynchronousAdocDataSource::readMsa()
{
    SynchronousAdocDataSource *x = new SynchronousAdocDataSource(this);

    QSignalSpy spyMsaReady(x, SIGNAL(msaReady(Msa*,int)));
    QSignalSpy spyMsaError(x, SIGNAL(msaError(QString,int)));

    // ------------------------------------------------------------------------
    // Test: data source not open, tag passing
    QList<Alphabet> alphabets;
    alphabets << eUnknownAlphabet << eAminoAlphabet << eDnaAlphabet << eRnaAlphabet;
    for (int i=0; i< 4; ++i)
    {
        for (int j=0; j< 4; ++j)
        {
            x->readMsa(i, alphabets.at(j), i+j);
            QVERIFY(spyMsaReady.isEmpty());
            QCOMPARE(spyMsaError.count(), 1);
            QCOMPARE(spyMsaError.takeFirst().at(1).toInt(), i+j);
        }
    }

    // ------------------------------------------------------------------------
    // Setup: copy over test database
    const QString sourceTestDbFile = "../test_databases/synchronous_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    const QString testDbFile = "synchronous_data_source-test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    x->open(testDbFile);
    QVERIFY(x->isOpen());

    // ------------------------------------------------------------------------
    // Test: opened data source, no anon seq factories, tag passing
    for (int i=0; i< 4; ++i)
    {
        for (int j=0; j< 4; ++j)
        {
            x->readMsa(i, alphabets.at(j), i+j);
            QVERIFY(spyMsaReady.isEmpty());
            QCOMPARE(spyMsaError.count(), 1);
            QCOMPARE(spyMsaError.takeFirst().at(1).toInt(), i+j);
        }
    }

    // ------------------------------------------------------------------------
    // Setup: valid anon seq factories
    DbAnonSeqFactory *aminoAnonSeqFactory = new DbAnonSeqFactory(QCryptographicHash::Sha1, constants::kTableAstrings);
    aminoAnonSeqFactory->setBioStringPrototype(new AminoString);
    DbAnonSeqFactory *dnaAnonSeqFactory = new DbAnonSeqFactory(QCryptographicHash::Sha1, constants::kTableDstrings);
    dnaAnonSeqFactory->setBioStringPrototype(new DnaString);
    DbAnonSeqFactory *rnaAnonSeqFactory = new DbAnonSeqFactory(QCryptographicHash::Sha1, constants::kTableRstrings);
    rnaAnonSeqFactory->setBioStringPrototype(new RnaString);

    x->setAnonSeqFactory(eAminoAlphabet, aminoAnonSeqFactory);
    x->setAnonSeqFactory(eDnaAlphabet, dnaAnonSeqFactory);
    x->setAnonSeqFactory(eRnaAlphabet, rnaAnonSeqFactory);

    // ------------------------------------------------------------------------
    // Test: eUnknownAlphabet still returns error
    x->readMsa(1, eUnknownAlphabet, 99);
    QVERIFY(spyMsaReady.isEmpty());
    QCOMPARE(spyMsaError.count(), 1);
    QCOMPARE(spyMsaError.takeFirst().at(1).toInt(), 99);

    // ------------------------------------------------------------------------
    // Test: opened data source, no record exists for the given id
    for (int i=0; i< alphabets.size(); ++i)
    {
        x->readMsa(1000, alphabets.at(i), 1000+i);
        QVERIFY(spyMsaReady.isEmpty());
        QCOMPARE(spyMsaError.count(), 1);
        QCOMPARE(spyMsaError.takeFirst().at(1).toInt(), 1000+i);
    }

    // ------------------------------------------------------------------------
    // Test: Read valid amino msa and check that it's position is properly loaded
    x->readMsa(1, eAminoAlphabet);
    QVERIFY(spyMsaError.isEmpty());
    QCOMPARE(spyMsaReady.count(), 1);
    Msa *msa = qvariant_cast<Msa *>(spyMsaReady.takeFirst().at(0));
    QCOMPARE(msa->id(), 1);
    QCOMPARE(msa->name(), QString("pas"));
    QCOMPARE(msa->alphabet(), eAminoAlphabet);
    QCOMPARE(msa->subseqCount(), 3);
    QCOMPARE(msa->at(1)->id(), 3);
    QCOMPARE(msa->at(1)->start(), 1);
    QCOMPARE(msa->at(1)->stop(), 118);
    QCOMPARE(msa->at(1)->bioString().alphabet(), eAminoAlphabet);
    QCOMPARE(msa->at(1)->bioString().sequence(), QString("TEDVVRARDAHLRSILDTVPDATVVSATDGTIVSFNAAAVRQFGYAEEEVIGQNLRILMPEPYRHEHDGYLQRYMATGEKRIIGIDRVVSGQRKDGSTFPMKLAVGEMRSGGERFFTG-------------------"));

    QCOMPARE(msa->at(2)->id(), 4);
    QCOMPARE(msa->at(2)->start(), 1);
    QCOMPARE(msa->at(2)->stop(), 114);
    QCOMPARE(msa->at(2)->bioString().alphabet(), eAminoAlphabet);
    QCOMPARE(msa->at(2)->bioString().sequence(), QString("--------GIFFPALEQNMMGAVLINEND-EVMFFNPAAEKLWGYKREEVIGNNIDMLIPRDLRPAHPEYIRHNREGGKARVEGMSRELQLEKKDGSKIWTRFALSKVSAEGKVYYLALVRDA--------------"));

    QCOMPARE(msa->at(3)->id(), 2);
    QCOMPARE(msa->at(3)->start(), 1);
    QCOMPARE(msa->at(3)->stop(), 119);
    QCOMPARE(msa->at(3)->bioString().alphabet(), eAminoAlphabet);
    QCOMPARE(msa->at(3)->bioString().sequence(), QString("------------------IPDAMIVIDGHGIIQLFSTAAERLFGWSELEAIGQNVNILMPEPDRSRHDSYISRYRTTSDPHIIGIGRIVTGKRRDGTTFPMHLSIGEMQSGGEPYFTGFVRDLTEHQQTQARLQELQ"));
    delete msa;
    msa = 0;

    // ------------------------------------------------------------------------
    // Test: amino msa without any subseqs
    x->readMsa(2, eAminoAlphabet);
    QVERIFY(spyMsaError.isEmpty());
    QCOMPARE(spyMsaReady.count(), 1);
    msa = qvariant_cast<Msa *>(spyMsaReady.takeFirst().at(0));
    QCOMPARE(msa->id(), 2);
    QCOMPARE(msa->name(), QString("LuxR"));
    QCOMPARE(msa->alphabet(), eAminoAlphabet);
    QCOMPARE(msa->subseqCount(), 0);
    delete msa;
    msa = 0;

    // ------------------------------------------------------------------------
    // Test: Read valid dna msa and check that it's position is properly loaded
    x->readMsa(1, eDnaAlphabet);
    QVERIFY(spyMsaError.isEmpty());
    QCOMPARE(spyMsaReady.count(), 1);
    msa = qvariant_cast<Msa *>(spyMsaReady.takeFirst().at(0));
    QCOMPARE(msa->id(), 1);
    QCOMPARE(msa->name(), QString("pas_dna"));
    QCOMPARE(msa->alphabet(), eDnaAlphabet);
    QCOMPARE(msa->subseqCount(), 3);
    QCOMPARE(msa->at(1)->id(), 4);
    QCOMPARE(msa->at(1)->start(), 1);
    QCOMPARE(msa->at(1)->stop(), 342);
    QCOMPARE(msa->at(1)->bioString().alphabet(), eDnaAlphabet);
    QCOMPARE(msa->at(1)->bioString().sequence(), QString("------------------------CATTTTTTTCCCCGCCCTTGAGCAAAATATGATGGGTGCGGTGTTAATTAACGAAAATGATGAAGTGAT---GTTTTTCAACCCCGCCGCAGAGAAGCTCTGGGGATACAAACGTGAAGAAGTCATTGGCAATAACATTGATATGCTGATTCCGCGGGATTTGCGTCCTGCGCATCCTGAATACATTCGTCACAACCGTGAAGGCGGTAAAGCGCGTGTTGAGGGGATGAGTCGGGAGCTGCAGCTGGAGAAAAAAGACGGCAGTAAAATCTGGACCCGTTTTGCGCTATCGAAAGTGAGCGCCGAGGGGAAAGTTTATTACCTGGCGCTGGTACGGGATGCCAG------------------------------------------"));

    QCOMPARE(msa->at(2)->id(), 3);
    QCOMPARE(msa->at(2)->start(), 1);
    QCOMPARE(msa->at(2)->stop(), 354);
    QCOMPARE(msa->at(2)->bioString().alphabet(), eDnaAlphabet);
    QCOMPARE(msa->at(2)->bioString().sequence(), QString("CGAGGACGTCGTAAGGGCCCGCGACGCTCATCTGAGATCCATACTGGATACTGTTCCGGACGCTACAGTGGTCAGCGCTACCGACGGCACAATCGTGTCCTTCAACGCCGCGGCCGTCCGGCAGTTCGGATACGCGGAGGAGGAGGTCATCGGCCAGAACCTGCGCATATTGATGCCGGAACCCTACCGCCACGAACACGACGGATATCTGCAGCGCTACATGGCAACCGGGGAAAAGCGCATCATCGGTATCGATCGCGTTGTCTCGGGGCAGCGGAAGGATGGATCGACGTTTCCGATGAAGCTCGCCGTGGGGGAGATGCGGTCGGGCGGCGAGAGGTTCTTCACGGGCTT---------------------------------------------------------"));

    QCOMPARE(msa->at(3)->id(), 2);
    QCOMPARE(msa->at(3)->start(), 1);
    QCOMPARE(msa->at(3)->stop(), 357);
    QCOMPARE(msa->at(3)->bioString().alphabet(), eDnaAlphabet);
    QCOMPARE(msa->at(3)->bioString().sequence(), QString("------------------------------------------------------TCCCGACGCCATGATCGTCATCGACGGCCACGGCATCATCCAGCTGTTCAGCACCGCCGCCGAGCGCCTGTTCGGCTGGTCCGAGCTCGAGGCGATCGGCCAGAACGTCAACATCCTGATGCCGGAGCCCGACCGCTCCCGGCATGACAGCTACATTTCGCGTTACCGCACCACGAGCGATCCCCACATCATCGGCATCGGGCGCATCGTGACCGGCAAGCGCCGCGACGGCACCACCTTCCCGATGCACCTGTCGATCGGCGAGATGCAGTCCGGCGGCGAGCCCTATTTTACCGGTTTCGTCCGCGATCTCACCGAGCACCAGCAGACCCAGGCGCGTCTCCAGGAACTGCAATC"));
    delete msa;
    msa = 0;

    // ------------------------------------------------------------------------
    // Test: Read valid rna msa and check that it's position is properly loaded
    x->readMsa(1, eRnaAlphabet);
    QVERIFY(spyMsaError.isEmpty());
    QCOMPARE(spyMsaReady.count(), 1);
    msa = qvariant_cast<Msa *>(spyMsaReady.takeFirst().at(0));
    QCOMPARE(msa->id(), 1);
    QCOMPARE(msa->name(), QString("pas_rna"));
    QCOMPARE(msa->alphabet(), eRnaAlphabet);
    QCOMPARE(msa->subseqCount(), 3);
    QCOMPARE(msa->at(1)->id(), 2);
    QCOMPARE(msa->at(1)->start(), 1);
    QCOMPARE(msa->at(1)->stop(), 357);
    QCOMPARE(msa->at(1)->bioString().alphabet(), eRnaAlphabet);
    QCOMPARE(msa->at(1)->bioString().sequence(), QString("------------------------------------------------------UCCCGACGCCAUGAUCGUCAUCGACGGCCACGGCAUCAUCCAGCUGUUCAGCACCGCCGCCGAGCGCCUGUUCGGCUGGUCCGAGCUCGAGGCGAUCGGCCAGAACGUCAACAUCCUGAUGCCGGAGCCCGACCGCUCCCGGCAUGACAGCUACAUUUCGCGUUACCGCACCACGAGCGAUCCCCACAUCAUCGGCAUCGGGCGCAUCGUGACCGGCAAGCGCCGCGACGGCACCACCUUCCCGAUGCACCUGUCGAUCGGCGAGAUGCAGUCCGGCGGCGAGCCCUAUUUUACCGGUUUCGUCCGCGAUCUCACCGAGCACCAGCAGACCCAGGCGCGUCUCCAGGAACUGCAAUC"));

    QCOMPARE(msa->at(2)->id(), 3);
    QCOMPARE(msa->at(2)->start(), 1);
    QCOMPARE(msa->at(2)->stop(), 354);
    QCOMPARE(msa->at(2)->bioString().alphabet(), eRnaAlphabet);
    QCOMPARE(msa->at(2)->bioString().sequence(), QString("CGAGGACGUCGUAAGGGCCCGCGACGCUCAUCUGAGAUCCAUACUGGAUACUGUUCCGGACGCUACAGUGGUCAGCGCUACCGACGGCACAAUCGUGUCCUUCAACGCCGCGGCCGUCCGGCAGUUCGGAUACGCGGAGGAGGAGGUCAUCGGCCAGAACCUGCGCAUAUUGAUGCCGGAACCCUACCGCCACGAACACGACGGAUAUCUGCAGCGCUACAUGGCAACCGGGGAAAAGCGCAUCAUCGGUAUCGAUCGCGUUGUCUCGGGGCAGCGGAAGGAUGGAUCGACGUUUCCGAUGAAGCUCGCCGUGGGGGAGAUGCGGUCGGGCGGCGAGAGGUUCUUCACGGGCUU---------------------------------------------------------"));

    QCOMPARE(msa->at(3)->id(), 4);
    QCOMPARE(msa->at(3)->start(), 1);
    QCOMPARE(msa->at(3)->stop(), 342);
    QCOMPARE(msa->at(3)->bioString().alphabet(), eRnaAlphabet);
    QCOMPARE(msa->at(3)->bioString().sequence(), QString("------------------------CAUUUUUUUCCCCGCCCUUGAGCAAAAUAUGAUGGGUGCGGUGUUAAUUAACGAAAAUGAUGAAGUGAU---GUUUUUCAACCCCGCCGCAGAGAAGCUCUGGGGAUACAAACGUGAAGAAGUCAUUGGCAAUAACAUUGAUAUGCUGAUUCCGCGGGAUUUGCGUCCUGCGCAUCCUGAAUACAUUCGUCACAACCGUGAAGGCGGUAAAGCGCGUGUUGAGGGGAUGAGUCGGGAGCUGCAGCUGGAGAAAAAAGACGGCAGUAAAAUCUGGACCCGUUUUGCGCUAUCGAAAGUGAGCGCCGAGGGGAAAGUUUAUUACCUGGCGCUGGUACGGGAUGCCAG------------------------------------------"));
    delete msa;
    msa = 0;
}


// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------
// Obsolete
/*
void TestSynchronousAdocDataSource::readSeqs()
{
    SynchronousAdocDataSource x;

    QSignalSpy spySeqsError(&x, SIGNAL(seqsError(Alphabet,QString,int)));
    QSignalSpy spySeqsReady(&x, SIGNAL(seqsReady(Alphabet,QList<DataRow>,int)));
    QList<QVariant> spyArguments;

    // Setup
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    // ------------------------------------------------------------------------
    // Suite: cases which return error signal
    // Test: Not open, all alphabets, empty ids, default tag
    QList<Alphabet> alphabets;
    alphabets << eAminoAlphabet << eDnaAlphabet << eRnaAlphabet << eUnknownAlphabet;
    foreach (Alphabet alphabet, alphabets)
    {
        x.readSeqs(alphabet, QList<int>());
        QVERIFY(spySeqsReady.isEmpty());
        QCOMPARE(spySeqsError.count(), 1);
        spyArguments = spySeqsError.takeFirst();
        QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), alphabet);
        QVERIFY(spyArguments.at(1).isNull() == false);
        QCOMPARE(spyArguments.at(2).toInt(), 0);
        spySeqsError.clear();
    }

    // Test: Not open, all alphabets, id list, various tags
    int tag = 1;
    foreach (Alphabet alphabet, alphabets)
    {
        x.readSeqs(alphabet, QList<int>() << 1 << 2 << 3 << -1 << -2 << 0 << 100, tag);
        QVERIFY(spySeqsReady.isEmpty());
        QCOMPARE(spySeqsError.count(), 1);
        spyArguments = spySeqsError.takeFirst();
        QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), alphabet);
        QVERIFY(spyArguments.at(1).isNull() == false);
        QCOMPARE(spyArguments.at(2).toInt(), tag);
        spySeqsError.clear();
        ++tag;
    }

    // Remaining tests deal with open database connection
    x.open(sourceTestDbFile);
    QVERIFY(x.isOpen());

    // Test: open, unknown alphabet, empty ids, default tag
    x.readSeqs(eUnknownAlphabet, QList<int>());
    QVERIFY(spySeqsReady.isEmpty());
    QCOMPARE(spySeqsError.count(), 1);
    spyArguments = spySeqsError.takeFirst();
    QVERIFY(spyArguments.at(0).toInt() == eUnknownAlphabet);
    QVERIFY(spyArguments.at(1).isNull() == false);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spySeqsError.clear();

    // Test: open, unknown alphabet, id list, non-default tag
    x.readSeqs(eUnknownAlphabet, QList<int>() << 1 << 2 << 3 << -1 << -2 << 0 << 100, -5);
    QVERIFY(spySeqsReady.isEmpty());
    QCOMPARE(spySeqsError.count(), 1);
    spyArguments = spySeqsError.takeFirst();
    QVERIFY(spyArguments.at(0).toInt() == eUnknownAlphabet);
    QVERIFY(spyArguments.at(1).isNull() == false);
    QCOMPARE(spyArguments.at(2).toInt(), -5);
    spySeqsError.clear();

    // ------------------------------------------------------------------------
    // Suite: cases which return success signal, but empty result list
    // Test: all valid alphabets, empty id list, default tag
    alphabets.clear();
    alphabets << eAminoAlphabet << eDnaAlphabet << eRnaAlphabet;
    foreach (Alphabet alphabet, alphabets)
    {
        x.readSeqs(alphabet, QList<int>());
        QVERIFY(spySeqsError.isEmpty());
        QCOMPARE(spySeqsReady.count(), 1);
        spyArguments = spySeqsReady.takeFirst();
        QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), alphabet);
        QVERIFY(qvariant_cast<QList<DataRow> >(spyArguments.at(1)).isEmpty());
        QCOMPARE(spyArguments.at(2).toInt(), 0);
        spySeqsReady.clear();
    }

    // Test: all valid alphabets, id list that does not match any database records, various tag
    foreach (Alphabet alphabet, alphabets)
    {
        x.readSeqs(alphabet, QList<int>() << -5 << -10 << 0 << 10 << 1000, tag);
        QVERIFY(spySeqsError.isEmpty());
        QCOMPARE(spySeqsReady.count(), 1);
        spyArguments = spySeqsReady.takeFirst();
        QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), alphabet);
        QVERIFY(qvariant_cast<QList<DataRow> >(spyArguments.at(1)).isEmpty());
        QCOMPARE(spyArguments.at(2).toInt(), tag);
        spySeqsReady.clear();
        ++tag;
    }

    // ------------------------------------------------------------------------
    // Suite: cases which return success signal, and non empty result list
    QList<DataRow> results;

    // Test: amino alphabets, single record, default tag
    x.readSeqs(eAminoAlphabet, QList<int>() << 1);
    QVERIFY(spySeqsError.isEmpty());
    QCOMPARE(spySeqsReady.count(), 1);
    spyArguments = spySeqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eAminoAlphabet);
    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 1);
    QCOMPARE(results.at(0).value("id").toInt(), 1);
    QCOMPARE(results.at(0).value("astring_id").toInt(), 1);
    QCOMPARE(results.at(0).value("label").toString(), QString("Asubseq_400-511"));
    QVERIFY(results.at(0).value("source").isNull());
    QVERIFY(results.at(0).value("protein").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:18:49"));
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spySeqsReady.clear();

    // Test: dna alphabet, two matching records, arbitrary tag
    x.readSeqs(eDnaAlphabet, QList<int>() << 2 << 3, 9999);
    QVERIFY(spySeqsError.isEmpty());
    QCOMPARE(spySeqsReady.count(), 1);
    spyArguments = spySeqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eDnaAlphabet);

    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 2);
    QCOMPARE(results.at(0).value("id").toInt(), 2);
    QCOMPARE(results.at(0).value("dstring_id").toInt(), 2);
    QCOMPARE(results.at(0).value("label").toString(), QString("bll2760"));
    QVERIFY(results.at(0).value("source").isNull());
    QVERIFY(results.at(0).value("gene").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:19:22"));

    QCOMPARE(results.at(1).value("id").toInt(), 3);
    QCOMPARE(results.at(1).value("dstring_id").toInt(), 3);
    QCOMPARE(results.at(1).value("label").toString(), QString("SMa1229"));
    QVERIFY(results.at(1).value("source").isNull());
    QVERIFY(results.at(1).value("gene").isNull());
    QCOMPARE(results.at(1).value("created").toString(), QString("2010-09-23 14:19:22"));

    QCOMPARE(spyArguments.at(2).toInt(), 9999);
    spySeqsReady.clear();

    // Test: rna alphabet, matching and non-matching record, arbitrary tag
    x.readSeqs(eRnaAlphabet, QList<int>() << 4 << 88, -9999);
    QVERIFY(spySeqsError.isEmpty());
    QCOMPARE(spySeqsReady.count(), 1);
    spyArguments = spySeqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eRnaAlphabet);

    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 1);
    QCOMPARE(results.at(0).value("id").toInt(), 4);
    QCOMPARE(results.at(0).value("rstring_id").toInt(), 4);
    QCOMPARE(results.at(0).value("label").toString(), QString("E4"));
    QVERIFY(results.at(0).value("source").isNull());
    QVERIFY(results.at(0).value("gene").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:19:41"));

    QCOMPARE(spyArguments.at(2).toInt(), -9999);
    spySeqsReady.clear();

    x.close();
}

void TestSynchronousAdocDataSource::readSubseqs()
{
    SynchronousAdocDataSource x;

    QSignalSpy spySeqsError(&x, SIGNAL(seqsError(Alphabet,QString,int)));
    QSignalSpy spySeqsReady(&x, SIGNAL(seqsReady(Alphabet,QList<DataRow>,int)));
    QSignalSpy spySubseqsError(&x, SIGNAL(subseqsError(Alphabet,QString,int)));
    QSignalSpy spySubseqsReady(&x, SIGNAL(subseqsReady(Alphabet,QList<DataRow>,int)));
    QList<QVariant> spyArguments;

    // Setup
    const QString sourceTestDbFile = "../test_databases/adoc_db_data_source.db";
    QVERIFY(QFile::exists(sourceTestDbFile));
    const QString testDbFile = "adoc_db_data_source-test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);
    QFile::copy(sourceTestDbFile, testDbFile);

    // ------------------------------------------------------------------------
    // Suite: cases which return error signal
    // Test: Not open, all alphabets, empty ids, default tag
    QList<Alphabet> alphabets;
    alphabets << eAminoAlphabet << eDnaAlphabet << eRnaAlphabet << eUnknownAlphabet;
    foreach (Alphabet alphabet, alphabets)
    {
        x.readSubseqs(alphabet, QList<int>());
        QVERIFY(spySeqsReady.isEmpty());
        QVERIFY(spySeqsError.isEmpty());
        QVERIFY(spySubseqsReady.isEmpty());
        QCOMPARE(spySubseqsError.count(), 1);
        spyArguments = spySubseqsError.takeFirst();
        QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), alphabet);
        QVERIFY(spyArguments.at(1).isNull() == false);
        QCOMPARE(spyArguments.at(2).toInt(), 0);
        spySubseqsError.clear();
    }

    // Test: Not open, all alphabets, id list, various tags
    int tag = 1;
    foreach (Alphabet alphabet, alphabets)
    {
        x.readSubseqs(alphabet, QList<int>() << 1 << 2 << 3 << -1 << -2 << 0 << 100, tag);
        QVERIFY(spySeqsReady.isEmpty());
        QVERIFY(spySeqsError.isEmpty());
        QVERIFY(spySubseqsReady.isEmpty());
        QCOMPARE(spySubseqsError.count(), 1);
        spyArguments = spySubseqsError.takeFirst();
        QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), alphabet);
        QVERIFY(spyArguments.at(1).isNull() == false);
        QCOMPARE(spyArguments.at(2).toInt(), tag);
        ++tag;
    }

    // Remaining tests deal with open database connection
    x.open(sourceTestDbFile);
    QVERIFY(x.isOpen());

    // Test: open, unknown alphabet, empty ids, default tag
    x.readSubseqs(eUnknownAlphabet, QList<int>());
    QVERIFY(spySeqsReady.isEmpty());
    QVERIFY(spySeqsError.isEmpty());
    QVERIFY(spySubseqsReady.isEmpty());
    QCOMPARE(spySubseqsError.count(), 1);
    spyArguments = spySubseqsError.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eUnknownAlphabet);
    QVERIFY(spyArguments.at(1).isNull() == false);
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spySubseqsError.clear();

    // Test: open, unknown alphabet, id list, non-default tag
    x.readSubseqs(eUnknownAlphabet, QList<int>() << 1 << 2 << 3 << -1 << -2 << 0 << 100, -5);
    QVERIFY(spySeqsReady.isEmpty());
    QVERIFY(spySeqsError.isEmpty());
    QVERIFY(spySubseqsReady.isEmpty());
    QCOMPARE(spySubseqsError.count(), 1);
    spyArguments = spySubseqsError.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eUnknownAlphabet);
    QVERIFY(spyArguments.at(1).isNull() == false);
    QCOMPARE(spyArguments.at(2).toInt(), -5);
    spySubseqsError.clear();

    // ------------------------------------------------------------------------
    // Suite: cases which return success signal, but empty result list
    // Test: all valid alphabets, empty id list, default tag
    alphabets.clear();
    alphabets << eAminoAlphabet << eDnaAlphabet << eRnaAlphabet;
    foreach (Alphabet alphabet, alphabets)
    {
        x.readSubseqs(alphabet, QList<int>());
        QVERIFY(spySeqsReady.isEmpty());
        QVERIFY(spySeqsError.isEmpty());
        QVERIFY(spySubseqsError.isEmpty());
        QCOMPARE(spySubseqsReady.count(), 1);
        spyArguments = spySubseqsReady.takeFirst();
        QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), alphabet);
        QVERIFY(qvariant_cast<QList<DataRow> >(spyArguments.at(1)).isEmpty());
        QCOMPARE(spyArguments.at(2).toInt(), 0);
        spySubseqsReady.clear();
    }

    // Test: all valid alphabets, id list that does not match any database records, various tag
    foreach (Alphabet alphabet, alphabets)
    {
        x.readSubseqs(alphabet, QList<int>() << -5 << -10 << 0 << 10 << 1000, tag);
        QVERIFY(spySeqsError.isEmpty());
        QVERIFY(spySubseqsError.isEmpty());
        QVERIFY(spySeqsReady.isEmpty());
        QCOMPARE(spySubseqsReady.count(), 1);
        spyArguments = spySubseqsReady.takeFirst();
        QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), alphabet);
        QVERIFY(qvariant_cast<QList<DataRow> >(spyArguments.at(1)).isEmpty());
        QCOMPARE(spyArguments.at(2).toInt(), tag);
        spySubseqsReady.clear();
        ++tag;
    }

    // ------------------------------------------------------------------------
    // Suite: cases which return success signal, and non empty result list
    QList<DataRow> results;

    // Test: amino alphabets, single record, default tag || should trigger amino_seq id of 1 is fetched along with the actual subseq
    x.readSubseqs(eAminoAlphabet, QList<int>() << 1);
    QVERIFY(spySeqsError.isEmpty());
    QVERIFY(spySubseqsError.isEmpty());
    QCOMPARE(spySeqsReady.count(), 1);
    spyArguments = spySeqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eAminoAlphabet);
    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 1);
    QCOMPARE(results.at(0).value("id").toInt(), 1);
    QCOMPARE(results.at(0).value("astring_id").toInt(), 1);
    QCOMPARE(results.at(0).value("label").toString(), QString("Asubseq_400-511"));
    QVERIFY(results.at(0).value("source").isNull());
    QVERIFY(results.at(0).value("protein").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:18:49"));
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spySeqsReady.clear();

    QCOMPARE(spySubseqsReady.count(), 1);
    spyArguments = spySubseqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eAminoAlphabet);
    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 1);
    QCOMPARE(results.at(0).value("id").toInt(), 1);
    QCOMPARE(results.at(0).value("amino_seq_id").toInt(), 1);
    QCOMPARE(results.at(0).value("label").toString(), QString("Asubseq_400-511_1-112"));
    QCOMPARE(results.at(0).value("start").toInt(), 1);
    QCOMPARE(results.at(0).value("stop").toInt(), 112);
    QCOMPARE(results.at(0).value("sequence").toString(), QString("SRLEALFENSPDMIDVLDADGTICEVNQRFCAELGYDESEVLGRSIWEFDLMFDAEDVQTQLSGFSVDERRKFEGLYERRDGSTMSVEVHLLRFNLEGEDRFLAISRDITER"));
    QVERIFY(results.at(0).value("notes").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:18:49"));
    QCOMPARE(spyArguments.at(2).toInt(), 0);
    spySubseqsReady.clear();


    // Test: dna alphabet, two matching records, arbitrary tag
    x.readSubseqs(eDnaAlphabet, QList<int>() << 2 << 3, 9999);
    QVERIFY(spySeqsError.isEmpty());
    QVERIFY(spySubseqsError.isEmpty());
    QCOMPARE(spySeqsReady.count(), 1);
    spyArguments = spySeqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eDnaAlphabet);

    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 2);
    QCOMPARE(results.at(0).value("id").toInt(), 2);
    QCOMPARE(results.at(0).value("dstring_id").toInt(), 2);
    QCOMPARE(results.at(0).value("label").toString(), QString("bll2760"));
    QVERIFY(results.at(0).value("source").isNull());
    QVERIFY(results.at(0).value("gene").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:19:22"));

    QCOMPARE(results.at(1).value("id").toInt(), 3);
    QCOMPARE(results.at(1).value("dstring_id").toInt(), 3);
    QCOMPARE(results.at(1).value("label").toString(), QString("SMa1229"));
    QVERIFY(results.at(1).value("source").isNull());
    QVERIFY(results.at(1).value("gene").isNull());
    QCOMPARE(results.at(1).value("created").toString(), QString("2010-09-23 14:19:22"));

    QCOMPARE(spyArguments.at(2).toInt(), 9999);
    spySeqsReady.clear();

    // And confirm that the subseq data was fetched
    QCOMPARE(spySubseqsReady.count(), 1);
    spyArguments = spySubseqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eDnaAlphabet);
    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 2);
    QCOMPARE(results.at(0).value("id").toInt(), 2);
    QCOMPARE(results.at(0).value("dna_seq_id").toInt(), 2);
    QCOMPARE(results.at(0).value("label").toString(), QString("bll2760_1-357"));
    QCOMPARE(results.at(0).value("start").toInt(), 1);
    QCOMPARE(results.at(0).value("stop").toInt(), 357);
    QCOMPARE(results.at(0).value("sequence").toString(), QString("------------------------------------------------------TCCCGACGCCATGATCGTCATCGACGGCCACGGCATCATCCAGCTGTTCAGCACCGCCGCCGAGCGCCTGTTCGGCTGGTCCGAGCTCGAGGCGATCGGCCAGAACGTCAACATCCTGATGCCGGAGCCCGACCGCTCCCGGCATGACAGCTACATTTCGCGTTACCGCACCACGAGCGATCCCCACATCATCGGCATCGGGCGCATCGTGACCGGCAAGCGCCGCGACGGCACCACCTTCCCGATGCACCTGTCGATCGGCGAGATGCAGTCCGGCGGCGAGCCCTATTTTACCGGTTTCGTCCGCGATCTCACCGAGCACCAGCAGACCCAGGCGCGTCTCCAGGAACTGCAATC"));
    QVERIFY(results.at(0).value("notes").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:19:22"));

    QCOMPARE(results.at(1).value("id").toInt(), 3);
    QCOMPARE(results.at(1).value("dna_seq_id").toInt(), 3);
    QCOMPARE(results.at(1).value("label").toString(), QString("SMa1229_1-354"));
    QCOMPARE(results.at(1).value("start").toInt(), 1);
    QCOMPARE(results.at(1).value("stop").toInt(), 354);
    QCOMPARE(results.at(1).value("sequence").toString(), QString("CGAGGACGTCGTAAGGGCCCGCGACGCTCATCTGAGATCCATACTGGATACTGTTCCGGACGCTACAGTGGTCAGCGCTACCGACGGCACAATCGTGTCCTTCAACGCCGCGGCCGTCCGGCAGTTCGGATACGCGGAGGAGGAGGTCATCGGCCAGAACCTGCGCATATTGATGCCGGAACCCTACCGCCACGAACACGACGGATATCTGCAGCGCTACATGGCAACCGGGGAAAAGCGCATCATCGGTATCGATCGCGTTGTCTCGGGGCAGCGGAAGGATGGATCGACGTTTCCGATGAAGCTCGCCGTGGGGGAGATGCGGTCGGGCGGCGAGAGGTTCTTCACGGGCTT---------------------------------------------------------"));
    QVERIFY(results.at(1).value("notes").isNull());
    QCOMPARE(results.at(1).value("created").toString(), QString("2010-09-23 14:19:22"));

    QCOMPARE(spyArguments.at(2).toInt(), 9999);
    spySubseqsReady.clear();


    // Test: rna alphabet, matching and non-matching record, arbitrary tag
    x.readSubseqs(eRnaAlphabet, QList<int>() << 4 << 88, -9999);
    QVERIFY(spySeqsError.isEmpty());
    QCOMPARE(spySeqsReady.count(), 1);
    spyArguments = spySeqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eRnaAlphabet);

    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 1);
    QCOMPARE(results.at(0).value("id").toInt(), 4);
    QCOMPARE(results.at(0).value("rstring_id").toInt(), 4);
    QCOMPARE(results.at(0).value("label").toString(), QString("E4"));
    QVERIFY(results.at(0).value("source").isNull());
    QVERIFY(results.at(0).value("gene").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:19:41"));

    QCOMPARE(spyArguments.at(2).toInt(), -9999);
    spySeqsReady.clear();


    // And confirm that the subseq data was fetched
    QCOMPARE(spySubseqsReady.count(), 1);
    spyArguments = spySubseqsReady.takeFirst();
    QCOMPARE(qvariant_cast<Alphabet>(spyArguments.at(0)), eRnaAlphabet);
    results = qvariant_cast<QList<DataRow> >(spyArguments.at(1));
    QCOMPARE(results.count(), 1);
    QCOMPARE(results.at(0).value("id").toInt(), 4);
    QCOMPARE(results.at(0).value("rna_seq_id").toInt(), 4);
    QCOMPARE(results.at(0).value("label").toString(), QString("E4_1-342"));
    QCOMPARE(results.at(0).value("start").toInt(), 1);
    QCOMPARE(results.at(0).value("stop").toInt(), 342);
    QCOMPARE(results.at(0).value("sequence").toString(), QString("------------------------CAUUUUUUUCCCCGCCCUUGAGCAAAAUAUGAUGGGUGCGGUGUUAAUUAACGAAAAUGAUGAAGUGAU---GUUUUUCAACCCCGCCGCAGAGAAGCUCUGGGGAUACAAACGUGAAGAAGUCAUUGGCAAUAACAUUGAUAUGCUGAUUCCGCGGGAUUUGCGUCCUGCGCAUCCUGAAUACAUUCGUCACAACCGUGAAGGCGGUAAAGCGCGUGUUGAGGGGAUGAGUCGGGAGCUGCAGCUGGAGAAAAAAGACGGCAGUAAAAUCUGGACCCGUUUUGCGCUAUCGAAAGUGAGCGCCGAGGGGAAAGUUUAUUACCUGGCGCUGGUACGGGAUGCCAG------------------------------------------"));
    QVERIFY(results.at(0).value("notes").isNull());
    QCOMPARE(results.at(0).value("created").toString(), QString("2010-09-23 14:19:41"));

    QCOMPARE(spyArguments.at(2).toInt(), -9999);
    spySubseqsReady.clear();
}
*/
// --------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------


QTEST_MAIN(TestSynchronousAdocDataSource)
#include "TestSynchronousAdocDataSource.moc"
