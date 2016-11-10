/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtSql/QSqlDriver>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>

#include <sqlite3.h>

#include "md5.h"

#include "exceptions/DatabaseError.h"
#include "exceptions/InvalidMpttNodeError.h"
#include "exceptions/RuntimeError.h"

#include "AdocDbDataSource.h"
#include "AdocTreeModel.h"
#include "AdocTreeNode.h"
#include "Adoc.h"
#include "DbAnonSeqFactory.h"
#include "DbSpec.h"


static void md5step(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    MD5Context *p;
    int i;
    if( argc<1 ) return;

    p = static_cast<MD5Context *>(sqlite3_aggregate_context(context, sizeof(*p)));

    if( p==0 )
        return;

    if( sqlite3_aggregate_count(context)==1 )
        MD5Init(p);

    for(i=0; i<argc; ++i)
    {
        if( argv[i] )
        {
            const unsigned char *x = sqlite3_value_text(argv[i]);
//            printf("I: %d, value: %s\n", i, x);
            MD5Update(p, x, strlen((const char *)x));
        }
    }
}

static void md5finalize(sqlite3_context *context)
{
    MD5Context *p;
    unsigned char digest[16];
    char zBuf[33];
    p = static_cast<MD5Context *>(sqlite3_aggregate_context(context, sizeof(*p)));
    MD5Final(digest,p);
    DigestToBase16(digest, zBuf);
    sqlite3_result_text(context, zBuf, strlen(zBuf), 0);
}


class TestAdoc : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Benchmarks
    void benchTreeInsertion();
    void benchTreeInsertionPrepared();
    void benchTreeReading();
    void benchTreeDigest();

    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void create();
    void open();
    void loadTreeFromDatabase();
    void save();
    void saveAs();

    void setModified();

    void anonSeqFactories();        // Also tests setAnonSeqFactory

private:

    bool areFilesIdentical(const QString &file1, const QString &file2);
    QByteArray digestFile(const QString &file);
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Benchmarks
void TestAdoc::benchTreeInsertion()
{
    QString db_file = "files/bench_insert.db";
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(db_file);
        QVERIFY(db.open());

        int n = 1000;

        QBENCHMARK {
            db.exec("BEGIN IMMEDIATE TRANSACTION");
            db.exec("create table data_tree (type text not null, foreign_table text, fk_id integer, label text, lft integer not null, rgt integer not null, "
                    "check(type IN ('root', 'project', 'group', 'seqamino', 'seqdna', 'seqrna', 'subseqamino', 'subseqdna', 'subseqrna', 'msaamino', 'msadna', 'msarna', 'primer')), "
                    "check(fk_id > 0), check(lft > 0), check(rgt > lft))");

            for (int i=0; i< n; ++i)
                db.exec("insert into data_tree (type, label, lft, rgt) values ('group', 'dummy', 3, 5)");

            db.exec("COMMIT");
            db.exec("DELETE FROM data_tree");
        }

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");

    QFile::remove(db_file);
}

void TestAdoc::benchTreeInsertionPrepared()
{
    QString db_file = "files/bench_insert.db";
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(db_file);
        QVERIFY(db.open());

        int n = 1000;

        QBENCHMARK {
            db.exec("BEGIN IMMEDIATE TRANSACTION");
            db.exec("create table data_tree (type text not null, foreign_table text, fk_id integer, label text, lft integer not null, rgt integer not null, "
                    "check(type IN ('root', 'project', 'group', 'seqamino', 'seqdna', 'seqrna', 'subseqamino', 'subseqdna', 'subseqrna', 'msaamino', 'msadna', 'msarna', 'primer')), "
                    "check(fk_id > 0), check(lft > 0), check(rgt > lft))");

            QSqlQuery insert(db);
            insert.prepare("insert into data_tree (type, label, lft, rgt) values (?, ?, ?, ?)");

            for (int i=0; i< n; ++i)
            {
                insert.bindValue(0, "group");
                insert.bindValue(1, "dummy");
                insert.bindValue(2, 3);
                insert.bindValue(3, 5);
                insert.exec();
            }

            db.exec("COMMIT");
            db.exec("DELETE FROM data_tree");
        }

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");

    QFile::remove(db_file);
}

void TestAdoc::benchTreeReading()
{
    QString db_file = "files/bench_read.db";
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(db_file);
        QVERIFY(db.open());

        int n = 1000;

        qDebug() << "Inserting" << n << "mptt records";
        db.exec("BEGIN IMMEDIATE TRANSACTION");
        db.exec("create table data_tree (type text not null, foreign_table text, fk_id integer, label text, lft integer not null, rgt integer not null, "
                "check(type IN ('root', 'project', 'group', 'seqamino', 'seqdna', 'seqrna', 'subseqamino', 'subseqdna', 'subseqrna', 'msaamino', 'msadna', 'msarna', 'primer')), "
                "check(fk_id > 0), check(lft > 0), check(rgt > lft))");

        for (int i=0; i< n; ++i)
            db.exec("insert into data_tree (type, label, lft, rgt) values ('group', 'dummy', 3, 5)");

        db.exec("COMMIT");

        qDebug() << "Done. Beginning benchmark";

        QBENCHMARK {
            db.exec("BEGIN IMMEDIATE TRANSACTION");

            QList<MpttNode *> mpttNodes;
            QSqlQuery select = db.exec("SELECT type, label, lft, rgt FROM data_tree ORDER BY lft");
            while (select.next())
                mpttNodes.append(new MpttNode(0, select.value(2).toInt(), select.value(3).toInt()));

            db.exec("COMMIT");

            qDeleteAll(mpttNodes);
            mpttNodes.clear();

            db.exec("DELETE FROM data_tree");
        }

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");
    QFile::remove(db_file);
}

void TestAdoc::benchTreeDigest()
{
    QString db_file = "files/bench_md5_digest.db";
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(db_file);
        QVERIFY(db.open());

        QVariant v = db.driver()->handle();
        if (v.isValid() && qstrcmp(v.typeName(), "sqlite3*")==0)
        {
            // v.data() returns a pointer to the handle
            sqlite3 *handle = *static_cast<sqlite3 **>(v.data());
            if (handle != 0) // check that it is not NULL
            {
                sqlite3_create_function(handle, "md5", -1, SQLITE_ANY, 0, 0, md5step, md5finalize);
            }
            else
                QVERIFY(0);
        }
        else
            QVERIFY(0);

        int n = 1000;

        qDebug() << "Inserting" << n << "mptt records";
        db.exec("BEGIN IMMEDIATE TRANSACTION");
        db.exec("create table data_tree (type text not null, foreign_table text, fk_id integer, label text, lft integer not null, rgt integer not null, "
                "check(type IN ('root', 'project', 'group', 'seqamino', 'seqdna', 'seqrna', 'subseqamino', 'subseqdna', 'subseqrna', 'msaamino', 'msadna', 'msarna', 'primer')), "
                "check(fk_id > 0), check(lft > 0), check(rgt > lft))");

        for (int i=0; i< n; ++i)
            db.exec("insert into data_tree (type, label, lft, rgt) values ('group', 'dummy', 3, 5)");

        db.exec("COMMIT");

        qDebug() << "Done. Beginning digest benchmark";

        QBENCHMARK {
            db.exec("BEGIN IMMEDIATE TRANSACTION");

            QSqlQuery select = db.exec("SELECT md5(type || label || lft || rgt) FROM data_tree");

            db.exec("COMMIT");

            db.exec("DELETE FROM data_tree");
        }

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");
    QFile::remove(db_file);
}



// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
void TestAdoc::constructor()
{
    Adoc adoc;

    // Ensure that it is in a valid default state
    QVERIFY(adoc.isModified() == false);
    QVERIFY(adoc.isTemporary() == false);
    QVERIFY(adoc.dataTreeModel() == 0);
    QVERIFY(adoc.dataSource().databaseReady() == false);
    QVERIFY(adoc.databaseFile().isNull());
    QVERIFY(adoc.close() == false);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestAdoc::create()
{
    DbSpec db_spec;
    Adoc adoc;

    try
    {
        // --------------------------
        // Test: spec is not loaded
        try
        {
            adoc.create("new_adoc.db", db_spec);
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
            QVERIFY(QFile::exists("new_adoc.db") == false);
            QVERIFY(adoc.isModified() == false);
            QVERIFY(adoc.isTemporary() == false);
            QVERIFY(adoc.dataTreeModel() == 0);
            QVERIFY(adoc.dataSource().databaseReady() == false);
            QVERIFY(adoc.databaseFile().isNull());
        }

        // --------------------------
        // Test: spec is logically invalid but loaded
        try
        {
            QVERIFY(db_spec.load("db_schema.xsd", "files/le_dup_table_names.xml"));
            adoc.create("new_adoc.db", db_spec);
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
            QVERIFY(QFile::exists("new_adoc.db") == false);
            QVERIFY(adoc.isModified() == false);
            QVERIFY(adoc.isTemporary() == false);
            QVERIFY(adoc.dataTreeModel() == 0);
            QVERIFY(adoc.dataSource().databaseReady() == false);
            QVERIFY(adoc.databaseFile().isNull());
        }

        QVERIFY(db_spec.load("db_schema.xsd", "db_spec.xml"));

        // --------------------------
        // Test: create over existing file
        try
        {
            adoc.create("files/preexisting_text_file", db_spec);
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
            QVERIFY(adoc.isModified() == false);
            QVERIFY(adoc.isTemporary() == false);
            QVERIFY(adoc.dataTreeModel() == 0);
            QVERIFY(adoc.dataSource().databaseReady() == false);
            QVERIFY(adoc.databaseFile().isNull());
        }

        // --------------------------
        // Test: create with file in directory that does not exist
        try
        {
            QVERIFY(!QFile::exists("missing_directory"));
            adoc.create("missing_directory/my_database_file.db", db_spec);
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);
        }


        // --------------------------
        // Test: completely invalid filename
        try
        {
            adoc.create("lkfasdf. fedk39%#%#$^ // \\lkj3@#$..db", db_spec);
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);
        }


        // --------------------------
        // Test: create database with valid name
        QString test_database_file = "files/test_create.db";
        if (QFile::exists(test_database_file))
            QFile::remove(test_database_file);
        adoc.create(test_database_file, db_spec);
        // Adoc should not be in a modified state because we created it with a permanent file name
        QVERIFY(adoc.isModified() == false);
        QVERIFY(adoc.isTemporary() == false);

        // No data should be loaded and thus the treeroot should still be null
        QVERIFY(adoc.dataTreeModel() == 0);
        QVERIFY(adoc.dataSource().databaseReady());
        QVERIFY(adoc.databaseFile() == test_database_file);

        // Check that the database tables were actually created
        QStringList tables = adoc.dataSource().database().tables();
        int nTables = tables.count();
        QVERIFY(nTables > 0);

        // --------------------------
        // Test: create database with another valid name on already opened adoc fails
        try
        {
            QString test_database_file2 = "files/test_create2.db";
            if (QFile::exists(test_database_file2))
                QFile::remove(test_database_file2);
            adoc.create(test_database_file2, db_spec);
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
            // Make sure that the existing database connection is working as expected
            QVERIFY(adoc.dataSource().databaseReady());
            tables = adoc.dataSource().database().tables();
            QCOMPARE(tables.count(), nTables);
        }

        // --------------------------
        // Test: same as above, except with NULL file
        try
        {
            adoc.create(QString(), db_spec);
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
            QVERIFY(adoc.dataSource().databaseReady());
            tables = adoc.dataSource().database().tables();
            QCOMPARE(tables.count(), nTables);
        }

        // --------------------------
        // Test: close
        QVERIFY(adoc.close());

        // state should be uninitialized after closing
        QVERIFY(adoc.isModified() == false);
        QVERIFY(adoc.dataTreeModel() == 0);
        QVERIFY(adoc.dataSource().databaseReady() == false);
        QVERIFY(adoc.databaseFile().isNull());

        // --------------------------
        // Test: close on already closed file should fail
        QVERIFY(adoc.close() == false);

        // Remove the test database file
        QFile::remove(test_database_file);

        // --------------------------
        // Test: temporary-file database
        adoc.create(QString(), db_spec);
        QString temp_database_file = adoc.databaseFile();
        QVERIFY(temp_database_file.isEmpty() == false);

        // Because it is not a permanent file, the modified state should be true
        QVERIFY(adoc.isModified());
        QVERIFY(adoc.dataTreeModel() == 0);
        QVERIFY(adoc.dataSource().databaseReady());

        // Check that tables were created
        tables = adoc.dataSource().database().tables();
        QVERIFY(tables.count() > 0);

        // Close it out and double check that the temporary file is gone
        QVERIFY(adoc.close());

        // Test: state should be uninitialized after closing
        QVERIFY(adoc.isModified() == false);
        QVERIFY(adoc.dataTreeModel() == 0);
        QVERIFY(adoc.dataSource().databaseReady() == false);
        QVERIFY(adoc.databaseFile().isNull());

        // Temporary database file should be deleted
        QVERIFY(QFile::exists(temp_database_file) == false);
    }
    catch (...) { QVERIFY(0); }
}

void TestAdoc::open()
{
    Adoc adoc;

    try
    {
        // ------------------------------------
        // Test: open null file should fail
        try
        {
            adoc.open("");
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
            QVERIFY(adoc.isModified() == false);
            QVERIFY(adoc.dataTreeModel() == 0);
            QVERIFY(adoc.dataSource().databaseReady() == false);
            QVERIFY(adoc.databaseFile().isNull());
        }


        // ------------------------------------
        // Test: open non-existent file
        try
        {
            QString non_existent_file = "files/file_does_not_exist";
            QVERIFY(QFile::exists(non_existent_file) == false);
            adoc.open(non_existent_file);
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(adoc.isModified() == false);
            QVERIFY(adoc.dataTreeModel() == 0);
            QVERIFY(adoc.dataSource().databaseReady() == false);
            QVERIFY(adoc.databaseFile().isNull());
        }


        // ------------------------------------
        // Test: open empty SQLite database
        QString basic_sqlite_file = "files/basic_sqlite.db";
        adoc.open(basic_sqlite_file);
        QVERIFY(adoc.isModified() == false);
        QVERIFY(adoc.dataTreeModel() == 0);
        QVERIFY(adoc.dataSource().databaseReady());
        QVERIFY(adoc.databaseFile() == basic_sqlite_file);
        QVERIFY(adoc.dataSource().database().tables().indexOf("tests") != -1);


        // ------------------------------------
        // Test: open another SQLite database without first closing original file
        QString basic_sqlite_file2 = "files/basic_sqlite2.db";
        try
        {
            adoc.open(basic_sqlite_file2);
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(adoc.dataSource().database().tables().indexOf("tests") != -1);
        }


        // ------------------------------------
        // Test: verify close works and that we can subsequently open basic_sqlite2.db
        QVERIFY(adoc.close());
        adoc.open(basic_sqlite_file2);
        QVERIFY(adoc.isModified() == false);
        QVERIFY(adoc.dataTreeModel() == 0);
        QVERIFY(adoc.dataSource().databaseReady());
        QVERIFY(adoc.databaseFile() == basic_sqlite_file2);
        QVERIFY(adoc.dataSource().database().tables().indexOf("names") != -1);
        QVERIFY(adoc.close());


        // ------------------------------------
        // Test: database connection locked against writes
        // o Should be able to read from it via another process
        adoc.open(basic_sqlite_file2);

        // --> Create another connection
        QSqlDatabase db2 = QSqlDatabase::addDatabase("QSQLITE", "test_db2");
        db2.setDatabaseName(basic_sqlite_file2);

        QVERIFY(db2.open());

        QSqlQuery s_names = db2.exec("SELECT id, first, last FROM names");
        QVERIFY(s_names.lastError().type() == false);
        QVERIFY(s_names.next());
        QVERIFY(s_names.value(0) == 1);
        QVERIFY(s_names.value(1) == "Luke");
        QVERIFY(s_names.value(2) == "Ulrich");
        s_names.finish();

        // o Should not be able to write via this connection
        QSqlQuery i_name = db2.exec("INSERT INTO names (id, first, last) values (10, 'Bob', 'Marley')");
        QVERIFY(i_name.lastError().type() != QSqlError::NoError);

        s_names = db2.exec("SELECT id, first, last FROM names WHERE id = 10");
        QVERIFY(s_names.lastError().type() == QSqlError::NoError);
        QVERIFY(s_names.next() == false);
        s_names.finish();

        db2.close();
    }
    catch (...) { QVERIFY(0); }

    QSqlDatabase::removeDatabase("test_db2");
}

void TestAdoc::loadTreeFromDatabase()
{
    Adoc adoc;

    try
    {
        // -------------------------------------
        // Test: loading tree from non-open adoc
        try
        {
            adoc.loadTreeFromDatabase();
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
            QVERIFY(adoc.dataTreeModel() == 0);
        }

        // -------------------------------------
        // Test: opened database with no activity or changes
        QString valid_database_file = "files/loadTreeTest.db";
        adoc.open(valid_database_file);

        // --> reset data_tree table to blank state
        adoc.dataSource().truncate("data_tree");
        adoc.dataSource().resetSqliteSequence("data_tree");
        QVERIFY(adoc.isModified() == false);

        // --> load tree from empty database should succeed and dataTreeModel
        adoc.loadTreeFromDatabase();
        QVERIFY(adoc.dataTreeModel());
        QVERIFY(adoc.dataTreeModel()->root());
        QCOMPARE(adoc.dataTreeModel()->root()->nodeType_, AdocTreeNode::RootType);
        QVERIFY(adoc.dataTreeModel()->root()->foreignTable_.isEmpty());
        QVERIFY(adoc.dataTreeModel()->root()->fkId_ == 0);
        QVERIFY(adoc.dataTreeModel()->root()->parent() == 0);
        QVERIFY(adoc.dataTreeModel()->root()->childCount() == 0);


        // -------------------------------------
        // Test: verify that second loading of tree from database fails because it is already loaded
        try
        {
            adoc.loadTreeFromDatabase();
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(adoc.dataTreeModel());
            QVERIFY(adoc.dataTreeModel()->root());
            QCOMPARE(adoc.dataTreeModel()->root()->nodeType_, AdocTreeNode::RootType);
        }
        QVERIFY(adoc.close());


        // -------------------------------------
        // Test: invalid MPTT tree
        adoc.open(valid_database_file);

        // --> reset data_tree table to blank state
        adoc.dataSource().truncate("data_tree");
        adoc.dataSource().resetSqliteSequence("data_tree");

        // --> create invalid tree of MPTT nodes; problem node is root rgt value of 3000
        QVERIFY(adoc.dataSource().database().exec("INSERT INTO data_tree (type, foreign_table, fk_id, label, lft, rgt) values ('root',     NULL,         NULL, 'root', 1, 3000)").lastError().type() == QSqlError::NoError);
        QVERIFY(adoc.dataSource().database().exec("INSERT INTO data_tree (type, foreign_table, fk_id, label, lft, rgt) values ('primer',   'primers',    10, 'A', 2, 3)").lastError().type() == QSqlError::NoError);
        QVERIFY(adoc.dataSource().database().exec("INSERT INTO data_tree (type, foreign_table, fk_id, label, lft, rgt) values ('group',    NULL,         NULL, 'B', 4, 7)").lastError().type() == QSqlError::NoError);
        QVERIFY(adoc.dataSource().database().exec("INSERT INTO data_tree (type, foreign_table, fk_id, label, lft, rgt) values ('seqamino', 'amino_seqs', 99, 'C', 5, 6)").lastError().type() == QSqlError::NoError);

        try
        {
            adoc.loadTreeFromDatabase();
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QCOMPARE(e.mpttNode()->right_, 3000);
            QVERIFY(adoc.dataTreeModel() == 0);
        }

        // -------------------------------------
        // Test: valid MPTT tree

        // --> fix invalid root node
        QVERIFY(adoc.dataSource().database().exec("UPDATE data_tree SET rgt = 8 where id = 1").lastError().type() == QSqlError::NoError);

        // Check that data loaded properly including the type and labels
        adoc.loadTreeFromDatabase();
        AdocTreeNode *root = adoc.dataTreeModel()->root();
        QVERIFY(root);
        QCOMPARE(root->nodeType_, AdocTreeNode::RootType);
        QVERIFY(root->foreignTable_.isEmpty());
        QCOMPARE(root->fkId_, 0);
        QCOMPARE(root->label_, QString("root"));
        QCOMPARE(root->childCount(), 2);

        AdocTreeNode *a = static_cast<AdocTreeNode *>(root->childAt(0));
        QCOMPARE(a->nodeType_, AdocTreeNode::PrimerType);
        QCOMPARE(a->foreignTable_, QString("primers"));
        QCOMPARE(a->fkId_, 10);
        QCOMPARE(a->label_, QString("A"));
        QVERIFY(a->parent() == root);
        QCOMPARE(a->childCount(), 0);

        AdocTreeNode *b = static_cast<AdocTreeNode *>(root->childAt(1));
        QCOMPARE(b->nodeType_, AdocTreeNode::GroupType);
        QVERIFY(b->foreignTable_.isEmpty());
        QCOMPARE(b->fkId_, 0);
        QCOMPARE(b->label_, QString("B"));
        QVERIFY(b->parent() == root);
        QCOMPARE(b->childCount(), 1);

        AdocTreeNode *c = static_cast<AdocTreeNode *>(b->childAt(0));
        QCOMPARE(c->nodeType_, AdocTreeNode::SeqAminoType);
        QCOMPARE(c->foreignTable_, QString("amino_seqs"));
        QCOMPARE(c->fkId_, 99);
        QCOMPARE(c->label_, QString("C"));
        QVERIFY(c->parent() == b);
        QCOMPARE(c->childCount(), 0);

        QVERIFY(adoc.close());
    }
    catch (...) { QVERIFY(0); }
}

void TestAdoc::save()
{
    Adoc adoc;

    try
    {
        // -------------------------------------
        // Test: attempt to save a non-open database
        try
        {
            adoc.save();
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
        }


        // -------------------------------------
        // Test: save opened database with no activity or changes
        QString valid_database_file = "files/simple_valid.db";
        adoc.open(valid_database_file);
        adoc.save();


        // -------------------------------------
        // Test: save overwrites data_tree in the database
        // o Build two trees that model before and after changes to the tree
        // o Then by comparing what happens in the database, we can validate the changes are
        //   indeed being saved as expected.
        QSqlDatabase db = adoc.dataSource().database();

        // --> Start with a fresh tree
        QVERIFY(db.exec("DELETE FROM data_tree").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("DELETE FROM sqlite_sequence WHERE name = 'data_tree'").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO data_tree (type, label, lft, rgt) values ('root', 'Rooter', 1, 4)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO data_tree (type, label, lft, rgt) values ('group', 'PAS', 2, 3)").lastError().type() == QSqlError::NoError);
        adoc.setModified(true);

        // --> Save should remove these nodes, because there are no nodes in memory
        adoc.save();
        QVERIFY(adoc.isModified() == false);

        // To make sure that we are outside of any previous transaction, close and re-open the database
        QVERIFY(adoc.close());
        adoc.open(valid_database_file);
        db = adoc.dataSource().database();

        QSqlQuery s_data_tree = db.exec("SELECT count(*) FROM data_tree");
        QVERIFY(s_data_tree.lastError().type() == QSqlError::NoError);
        QVERIFY(s_data_tree.next());
        QVERIFY(s_data_tree.value(0).toInt() == 0);


        // -------------------------------------
        // Test: start with fresh tree and check that in-memory tree replaces it
        adoc.loadTreeFromDatabase();

        AdocTreeNode *root = adoc.dataTreeModel()->root();
        QVERIFY(root);

        QVERIFY(db.exec("DELETE FROM data_tree").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("DELETE FROM sqlite_sequence WHERE name = 'data_tree'").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO data_tree (type, label, lft, rgt) values ('root', 'Rooter', 10, 40)").lastError().type() == QSqlError::NoError);

        root->appendChild(new AdocTreeNode(AdocTreeNode::SeqAminoType, "Divergent", "amino_seqs", 55));
        adoc.save();

        QSqlQuery s_count = db.exec("SELECT count(*) from data_tree");
        QVERIFY(s_count.lastError().type() == QSqlError::NoError);
        QVERIFY(s_count.next());
        QCOMPARE(s_count.value(0).toInt(), 2);

        s_data_tree = db.exec("SELECT type, foreign_table, fk_id, label, lft, rgt FROM data_tree");
        QVERIFY(s_data_tree.lastError().type() == QSqlError::NoError);
        QVERIFY(s_data_tree.next());
        QVERIFY(s_data_tree.value(0).toString() == "root");
        QVERIFY(s_data_tree.value(1).toString().isNull());
        QCOMPARE(s_data_tree.value(2).toInt(), 0);
        QCOMPARE(s_data_tree.value(4).toInt(), 1);
        QCOMPARE(s_data_tree.value(5).toInt(), 4);

        QVERIFY(s_data_tree.next());
        QVERIFY(s_data_tree.value(0).toString() == "seqamino");
        QVERIFY(s_data_tree.value(1).toString() == "amino_seqs");
        QCOMPARE(s_data_tree.value(2).toInt(), 55);
        QCOMPARE(s_data_tree.value(3).toString(), QString("Divergent"));
        QCOMPARE(s_data_tree.value(4).toInt(), 2);
        QCOMPARE(s_data_tree.value(5).toInt(), 3);
        s_data_tree.finish();


        // -------------------------------------
        // Test: check that new transaction is started after calling save
        adoc.save();

        QSqlDatabase db2 = QSqlDatabase::addDatabase("QSQLITE", "test_db2");
        db2.setDatabaseName(valid_database_file);

        QVERIFY(db2.open());

        // This query will work, but requires extra time due to the write lock on the database...
        QSqlQuery s_data_tree2 = db2.exec("SELECT count(*) FROM data_tree");
        QVERIFY(s_data_tree2.lastError().type() == QSqlError::NoError);
        QVERIFY(s_data_tree2.next());
        QCOMPARE(s_data_tree2.value(0).toInt(), 2);
        s_data_tree2.finish();

        // o Should not be able to write via this connection
        QSqlQuery i_data_tree = db2.exec("INSERT INTO data_tree (type, lft, rgt) values ('project', 50, 60)");
        QVERIFY(i_data_tree.lastError().type() != QSqlError::NoError);

        db2.close();
    }
    catch (...) { QVERIFY(0); }

    QSqlDatabase::removeDatabase("test_db2");
}

void TestAdoc::saveAs()
{
    Adoc adoc;

    try
    {
        // -------------------------------------
        // Test: attempt to saveAs with empty file and no open file
        try
        {
            adoc.saveAs("");
            QVERIFY(0);
        }
        catch (RuntimeError &e) { QVERIFY(1); }


        // -------------------------------------
        // Test: saveAs with non-empty file, but lacking open file
        try
        {
            adoc.saveAs("files/no_open_connection.db");
            QVERIFY(0);
        }
        catch (RuntimeError &e) { QVERIFY(1); }


        // -------------------------------------
        // Test: saveAs with empty file name and open file
        QString valid_database_file = "files/simple_valid.db";

        // Ensure that this file exists and has the data_tree table
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
            db.setDatabaseName(valid_database_file);
            QVERIFY(db.open());

            if (db.record(constants::kTableDataTree).isEmpty())
            {
                // Create the data tree table
                QString create_sql = QString("CREATE table %1 (id integer primary key autoincrement, type text, foreign_table text, fk_id integer, label text, lft integer not null, rgt integer not null)").arg(constants::kTableDataTree);
                QVERIFY(db.exec(create_sql).lastError().type() == QSqlError::NoError);
            }

            db.close();
        }
        QSqlDatabase::removeDatabase("test_db");

        adoc.open(valid_database_file);
        try
        {
            adoc.saveAs("");
            QVERIFY(0);
        }
        catch (RuntimeError &e) { QVERIFY(1); }
        catch (...)
        {
            QVERIFY(1);
        }


        // -------------------------------------
        // Test: saveAs to existing database file
        try
        {
            adoc.saveAs("files/preexisting_database.db");
            QVERIFY(0);
        }
        catch (RuntimeError &e) { QVERIFY(1); }


        // -------------------------------------
        // Test: saveAs to existing plain text file
        try
        {
            adoc.saveAs("files/preexisting_text_file");
            QVERIFY(0);
        }
        catch (RuntimeError &e) { QVERIFY(1); }


        // -------------------------------------
        // Test: saveAs with file in directory that does not exist
        try
        {
            QVERIFY(!QFile::exists("missing_directory"));
            adoc.saveAs("missing_directory/my_database_file.db");
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
        }


        // -------------------------------------
        // Test: saveAs with completely invalid filename
        try
        {
            adoc.saveAs("lkfasdf. fedk39%#%#$^ // \\lkj3@#$..db");
            QVERIFY(0);
        }
        catch (RuntimeError &e)
        {
            QVERIFY(1);
        }



        // --> Start with a fresh tree; how? loadTreeFromDatabase has not been called and thus the data tree is completely empty
        adoc.save();

        // -------------------------------------
        // Test: saveAs with no additional changes
        QString db_file2 = "files/simple_valid2.db";
        QFile::remove(db_file2);
        adoc.saveAs(db_file2);
        QVERIFY(adoc.isModified() == false);
        QVERIFY(adoc.databaseFile() == db_file2);
        QVERIFY(adoc.dataSource().database().databaseName() == db_file2);
        QVERIFY(adoc.dataSource().databaseReady());
        QVERIFY(QFile::exists(valid_database_file));
        QVERIFY(QFile::exists(db_file2));
        adoc.close();

        // Manually check that there are no rows inside either file
        QStringList files;
        files << valid_database_file << db_file2;
        foreach (QString file, files)
        {
            {
                QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
                db.setDatabaseName(file);
                QVERIFY(db.open());
                QSqlQuery s_data_tree_count = db.exec("SELECT count(*) from data_tree");
                QVERIFY(s_data_tree_count.next());
                QVERIFY(s_data_tree_count.value(0).toInt() == 0);
                db.close();
            }
            QSqlDatabase::removeDatabase("test_db");
        }


        // -------------------------------------
        // Test: saveAs with some changes
        adoc.open(valid_database_file);
        QSqlDatabase db = adoc.dataSource().database();

        // Create a table if it does not already exist
        QVERIFY(db.exec("CREATE TABLE projects (id integer not null primary key autoincrement, name text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO projects (name) VALUES ('Quorum')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO projects (name) VALUES ('MiST2')").lastError().type() == QSqlError::NoError);

        QVERIFY(db.exec("DELETE FROM data_tree").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("DELETE FROM sqlite_sequence WHERE name = 'data_tree'").lastError().type() == QSqlError::NoError);
        adoc.loadTreeFromDatabase();
        AdocTreeNode *root = adoc.dataTreeModel()->root();
        QVERIFY(root);
        root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Quorum-sensing"));
        adoc.setModified(true);

        QFile::remove(db_file2);
        adoc.saveAs(db_file2);
        QVERIFY(adoc.isModified() == false);
        QVERIFY(adoc.databaseFile() == db_file2);
        QVERIFY(adoc.dataSource().database().databaseName() == db_file2);
        QVERIFY(adoc.dataSource().databaseReady());
        QVERIFY(QFile::exists(valid_database_file));
        QVERIFY(QFile::exists(db_file2));

        // Subtest: Existing database contains the changes we made
        db = adoc.dataSource().database();
        QSqlQuery s_projects = db.exec("SELECT name FROM projects ORDER BY name");
        QVERIFY(s_projects.lastError().type() == QSqlError::NoError);
        QVERIFY(s_projects.next());
        QCOMPARE(s_projects.value(0).toString(), QString("MiST2"));
        QVERIFY(s_projects.next());
        QCOMPARE(s_projects.value(0).toString(), QString("Quorum"));
        s_projects.finish();

        QSqlQuery s_data_tree = db.exec("SELECT count(*) FROM data_tree");
        QVERIFY(s_data_tree.next());
        QCOMPARE(s_data_tree.value(0).toInt(), 2);
        s_data_tree.finish();
        adoc.close();

        // Subtest: old database does not contain these changes - in particular the projects table should not even be present
        adoc.open(valid_database_file);
        db = adoc.dataSource().database();
        QVERIFY(db.record("projects").isEmpty());

        s_data_tree = db.exec("SELECT count(*) FROM data_tree");
        QVERIFY(s_data_tree.next());
        QCOMPARE(s_data_tree.value(0).toInt(), 0);
        s_data_tree.finish();

        adoc.close();

        // Cleanup
        QFile::remove(db_file2);
    }
    catch (Exception &e)
    {
        qDebug() << e.what();
        QVERIFY(0);
    }
}

void TestAdoc::setModified()
{
    Adoc adoc;

    QVERIFY(adoc.isModified() == false);
    adoc.setModified(true);
    QVERIFY(adoc.isModified() == true);
    adoc.setModified(false);
    QVERIFY(adoc.isModified() == false);
}

void TestAdoc::anonSeqFactories()
{
    Adoc adoc;

    QVERIFY(adoc.anonSeqFactories().isEmpty());

    // Test: add a zero factory pointer
    adoc.setAnonSeqFactory(eAminoAlphabet, 0);
    QVERIFY(adoc.anonSeqFactories().isEmpty());

    // Test: add a valid factory
    DbAnonSeqFactory *aminoFactory = new DbAnonSeqFactory();
    adoc.setAnonSeqFactory(eAminoAlphabet, aminoFactory);
    QCOMPARE(adoc.anonSeqFactories().size(), 1);
    QVERIFY(adoc.anonSeqFactories().contains(eAminoAlphabet));
    QVERIFY(adoc.anonSeqFactories().value(eAminoAlphabet) == aminoFactory);
    aminoFactory = 0;

    // Test: add another valid facotyr over the current one
    DbAnonSeqFactory *aminoFactory2 = new DbAnonSeqFactory();
    adoc.setAnonSeqFactory(eAminoAlphabet, aminoFactory2);
    QCOMPARE(adoc.anonSeqFactories().size(), 1);
    QVERIFY(adoc.anonSeqFactories().contains(eAminoAlphabet));
    QVERIFY(adoc.anonSeqFactories().value(eAminoAlphabet) == aminoFactory2);
    aminoFactory2 = 0;

    // Test: zero out amino factory
    adoc.setAnonSeqFactory(eAminoAlphabet, 0);
    QCOMPARE(adoc.anonSeqFactories().size(), 0);

    // Test: add three factories and check that they return the right ones
    DbAnonSeqFactory *dnaFactory = new DbAnonSeqFactory();
    DbAnonSeqFactory *rnaFactory = new DbAnonSeqFactory();
    DbAnonSeqFactory *aminoFactory3 = new DbAnonSeqFactory();
    adoc.setAnonSeqFactory(eDnaAlphabet, dnaFactory);
    adoc.setAnonSeqFactory(eDnaAlphabet, rnaFactory);
    adoc.setAnonSeqFactory(eDnaAlphabet, aminoFactory3);
    QCOMPARE(adoc.anonSeqFactories().size(), 3);
    QVERIFY(adoc.anonSeqFactories().contains(eDnaAlphabet));
    QVERIFY(adoc.anonSeqFactories().value(eDnaAlphabet) == dnaFactory);
    QVERIFY(adoc.anonSeqFactories().contains(eRnaAlphabet));
    QVERIFY(adoc.anonSeqFactories().value(eRnaAlphabet) == rnaFactory);
    QVERIFY(adoc.anonSeqFactories().contains(eAminoAlphabet));
    QVERIFY(adoc.anonSeqFactories().value(eAminoAlphabet) == aminoFactory3);
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
bool TestAdoc::areFilesIdentical(const QString &file1, const QString &file2)
{
    if (!QFile::exists(file1) ||
        !QFile::exists(file2))
    {
        return false;
    }

    return digestFile(file1) == digestFile(file2);
}

QByteArray TestAdoc::digestFile(const QString &file)
{
    static QCryptographicHash hasher(QCryptographicHash::Sha1);

    hasher.reset();

    QFile in(file);
    if (in.open(QIODevice::ReadOnly))
    {
        while (!in.atEnd())
            hasher.addData(in.read(65536));
        in.close();
    }

    return hasher.result();
}

QTEST_MAIN(TestAdoc)
#include "TestAdoc.moc"
