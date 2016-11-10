/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "DbTable.h"

#include <QSqlError>

// TODO: test case sensitivity of table names and columns

class TestDbTable : public QObject
{
    Q_OBJECT

public:
    TestDbTable(QObject *parent = 0) : QObject(parent)
    {
        dbName_ = "memdb";
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorBasic();  // Also tests database(), primaryKeyField(), and tableName()
    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setPrimaryKeyField();
    void setTable();

private:
    QSqlDatabase cleanDatabase();

    QString dbName_;
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestDbTable::constructorBasic()
{
    // Put in code block to avoid stupid database warning that connection is still in use
    {
        QSqlDatabase db = cleanDatabase();

        // Test: empty table name and empty database
        DbTable table1("", QSqlDatabase());
        QVERIFY(table1.count() == 0);
        QVERIFY(table1.primaryKeyField().isEmpty());
        QVERIFY(table1.tableName().isEmpty());
        QVERIFY(table1.database().isValid() == false);

        // Test: empty table name and valid database
        DbTable table2(QString(), db);
        QVERIFY(table2.count() == 0);
        QVERIFY(table2.primaryKeyField().isEmpty());
        QVERIFY(table2.tableName().isEmpty());
        QVERIFY(table2.database().isValid() == false);

        // Test: table name and empty database
        DbTable table3("projects", QSqlDatabase());
        QVERIFY(table3.count() == 0);
        QVERIFY(table3.primaryKeyField().isEmpty());
        QVERIFY(table3.tableName().isEmpty());
        QVERIFY(table3.database().isValid() == false);

        // Test: invalid table name and valid database
        DbTable table4("missing", db);
        QVERIFY(table4.count() == 0);
        QVERIFY(table4.primaryKeyField().isEmpty());
        QVERIFY(table4.tableName().isEmpty());
        QVERIFY(table4.database().isValid() == false);

        // Test: valid table name and closed database
        db.close();
        DbTable table5("projects", db);
        QVERIFY(table5.count() == 0);
        QVERIFY(table5.primaryKeyField().isEmpty());
        QVERIFY(table5.tableName().isEmpty());
        QVERIFY(table5.database().isValid() == false);
    }

    // Test: valid table name and valid database
    QSqlDatabase db = cleanDatabase();

    DbTable table6("projects", db);
    QVERIFY2(table6.count() == 3, QString("%1").arg(table6.count()).toAscii());
    QVERIFY2(table6.primaryKeyField() == "id", table6.primaryKeyField().toAscii());
    QVERIFY(table6.tableName() == "projects");
    QVERIFY(table6.database().isValid());
    QVERIFY(table6.indexOf("id") != -1);
    QVERIFY(table6.indexOf("name") != -1);
    QVERIFY(table6.indexOf("created") != -1);

    // Test: composite primary key should utilize the first field or id
    QVERIFY(db.exec("CREATE TABLE composite (aseq_id integer, start integer, primary key(aseq_id, start))").lastError().type() == QSqlError::NoError);
    DbTable table7("composite", db);
    QVERIFY(table7.count() == 2);
    QVERIFY(table7.primaryKeyField() == "aseq_id");
    QVERIFY(table7.tableName() == "composite");

    // Test: table without primary key but with field id should have id as the primary key
    QVERIFY(db.exec("CREATE TABLE nopk (id integer, start integer)").lastError().type() == QSqlError::NoError);
    DbTable table8("nopk", db);
    QVERIFY(table8.count() == 2);
    QVERIFY(table8.primaryKeyField() == "id");
    QVERIFY(table8.tableName()  == "nopk");

    // Test: this should occur even if the id field is not the first one listed
    QVERIFY(db.exec("CREATE TABLE nopk2 (start integer, id integer)").lastError().type() == QSqlError::NoError);
    DbTable table9("nopk2", db);
    QVERIFY(table9.count() == 2);
    QVERIFY(table9.primaryKeyField() == "id");
    QVERIFY(table9.tableName()  == "nopk2");

    // Test: table without primary key or a field named id, should have the first named field as the primary key
    // NOTE: Qt does not guarantee the order of the fields will correspond to their order in the database. For now, we are
    //       assuming this is the case because empirical tests show that this is the case.
    QVERIFY(db.exec("CREATE TABLE nopk3 (aseq_id integer, start integer)").lastError().type() == QSqlError::NoError);
    DbTable table10("nopk3", db);
    QVERIFY2(table10.count() == 2, QString("%1").arg(table10.count()).toAscii());
    QVERIFY(table10.primaryKeyField() == "aseq_id");
    QVERIFY(table10.tableName()  == "nopk3");
}

void TestDbTable::constructorCopy()
{
    QSqlDatabase db = cleanDatabase();
    DbTable table("projects", db);

    // Test: copy the DbTable
    DbTable table2(table);
    QVERIFY(table2.count() == 3);
    QVERIFY(table2.primaryKeyField() == "id");
    QVERIFY(table2.tableName() == "projects");

    QVERIFY(table2.indexOf("id") != -1);
    QVERIFY(table2.indexOf("name") != -1);
    QVERIFY(table2.indexOf("created") != -1);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestDbTable::setPrimaryKeyField()
{
    QSqlDatabase db = cleanDatabase();

    DbTable table("projects", db);

    // Test: invalid field name
    QVERIFY(table.setPrimaryKeyField("missing") == false);
    QVERIFY(table.primaryKeyField() == "id");

    // Test: valid alternative field name
    QVERIFY(table.setPrimaryKeyField("name"));
    QVERIFY(table.primaryKeyField() == "name");
}


// Existing configuration to new configuration
// Bad configuration to new configuration

void TestDbTable::setTable()
{
    QSqlDatabase db = cleanDatabase();

    DbTable table("missing", QSqlDatabase());

    // Test: empty table name, empty database
    QVERIFY(table.setTable("") == false);
    QVERIFY(table.isEmpty());
    QVERIFY(table.database().isValid() == false);
    QVERIFY(table.tableName().isEmpty());

    // Test: non-empty table name, invalid database
    QVERIFY(table.setTable("aseqs") == false);
    QVERIFY(table.isEmpty());
    QVERIFY(table.database().isValid() == false);
    QVERIFY(table.tableName().isEmpty());

    // Test: empty table name, valid database
    QVERIFY(table.setTable("", db) == false);
    QVERIFY(table.isEmpty());
    QVERIFY(table.database().isValid() == false);
    QVERIFY(table.tableName().isEmpty());

    // Test: invalid table name, valid database
    QVERIFY(table.setTable("missing", db) == false);
    QVERIFY(table.isEmpty());
    QVERIFY(table.database().isValid() == false);
    QVERIFY(table.tableName().isEmpty());

    // Test: valid table name, valid database
    QVERIFY(table.setTable("aseqs", db));
    QVERIFY(table.count() == 2);    // Current table should be unchanged
    QVERIFY(table.contains("sequence"));
    QVERIFY(table.contains("aseq_id"));
    QVERIFY(table.tableName() == "aseqs");

    // Test: migrate from valid, non Empty configuration to invalid empty configuration, should not update
    DbTable table2("projects", db);

    // Test: empty table name
    QVERIFY(table2.setTable("") == false);
    QVERIFY(table2.database().isValid());
    QVERIFY(table2.tableName() == "projects");
    QVERIFY(table2.count() == 3);

    // Test: invalid table name
    QVERIFY(table2.setTable("missing") == false);
    QVERIFY(table2.database().isValid());
    QVERIFY(table2.tableName() == "projects");
    QVERIFY(table2.count() == 3);

    // Test: valid table name
    QVERIFY(table2.setTable("aseqs"));
    QVERIFY(table2.database().isValid());
    QVERIFY(table2.tableName() == "aseqs");
    QVERIFY(table2.count() == 2);

    // Test: valid table name, invalid database
    QSqlDatabase bad_db = QSqlDatabase::addDatabase("QSQLITE", "bad_db");
    QVERIFY(table2.setTable("projects", bad_db) == false);
    QVERIFY(table2.database().isValid());
    QVERIFY(table2.tableName() == "aseqs");
    QVERIFY(table2.count() == 2);

    // Test: same table name, different database with different fields
    QSqlDatabase db2 = QSqlDatabase::addDatabase("QSQLITE", "db2");
    db2.setDatabaseName(":memory:");
    QVERIFY(db2.open());
    QVERIFY(db2.exec("CREATE TABLE projects (label text not null primary key, cost text not null, amount integer, expires text)").lastError().type() == QSqlError::NoError);

    QVERIFY(table2.setTable("projects", db2));
    QVERIFY(table2.tableName() == "projects");
    QVERIFY(table2.count() == 4);
    QVERIFY(table2.contains("label"));
    QVERIFY(table2.contains("cost"));
    QVERIFY(table2.contains("amount"));
    QVERIFY(table2.contains("expires"));

    db2.close();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
QSqlDatabase TestDbTable::cleanDatabase()
{
    if (QSqlDatabase::database(dbName_).isOpen())
    {
        QSqlDatabase::database(dbName_).close();
        QSqlDatabase::removeDatabase(dbName_);
    }

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", dbName_);
    db.setDatabaseName(":memory:");
    db.open();
    db.exec("CREATE TABLE projects (id integer primary key autoincrement, name text not null, created text)");
    db.exec("CREATE TABLE aseqs (aseq_id integer primary key autoincrement, sequence text not null)");

    return db;
}

QTEST_MAIN(TestDbTable)
#include "TestDbTable.moc"
