/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QSqlError>

#include "CrudSqlRecord.h"

// TODO: test case sensitivity of table names and columns

class TestCrudSqlRecord : public QObject
{
    Q_OBJECT

public:
    TestCrudSqlRecord(QObject *parent = 0) : QObject(parent)
    {
        dbName_ = "memdb";
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorBasic();        // Also tests dbTable() and queryFields()
    void setDbTable();              // Also tests dbTable() and queryFields()

    // CrudSqlRecord specific field modifying functions - only testing the presence/absence of fields in the
    // associated table per the documentation ;)
    void append();
    void insert();
    void replace();
    void remove();

    void defaultFields();
    void hasData();

    // CRUD specific tests
    void read();
    void erase();
    void save_insert();
    void save_update();

    // ------------------------------------------------------------------------------------------------
    // Public methods
private:
    QString dbName_;
    QSqlDatabase cleanDatabase();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestCrudSqlRecord::constructorBasic()
{
    QSqlDatabase db = cleanDatabase();

    // Test: invalid dbTable
    CrudSqlRecord alpha(DbTable("", QSqlDatabase()));
    QVERIFY(alpha.count() == 0);
    QVERIFY(alpha.queryFields().isEmpty());
    QVERIFY(alpha.id_.isValid() == false);
    QVERIFY(alpha.hasData() == false);

    // Test: non-empty table, but invalid database
    CrudSqlRecord beta(DbTable("projects", QSqlDatabase()));
    QVERIFY(beta.count() == 0);
    QVERIFY(beta.queryFields().isEmpty());
    QVERIFY(beta.id_.isValid() == false);
    QVERIFY(beta.hasData() == false);

    // Test: valid dbTable and database
    CrudSqlRecord gamma(DbTable("projects", db));
    QVERIFY(gamma.count() == 3);
    QVERIFY(gamma.contains("name"));
    QVERIFY(gamma.value(0).isNull());
    QVERIFY(gamma.value(1).isNull());
    QVERIFY(gamma.value(2).isNull());
    QVERIFY2(gamma.queryFields().count() == 3, QString("%1").arg(gamma.queryFields().count()).toAscii());
    QVERIFY(gamma.id_.isValid() == false);
}

void TestCrudSqlRecord::setDbTable()
{
    QSqlDatabase db = cleanDatabase();

    // Test: invalid table, valid database
    CrudSqlRecord alpha(DbTable("", QSqlDatabase()));
    QVERIFY(alpha.dbTable().isEmpty());
    QVERIFY(alpha.isEmpty());

    alpha.id_ = 50;
    alpha.setDbTable(DbTable("projects", QSqlDatabase()));
    QVERIFY(alpha.dbTable().isEmpty());
    QVERIFY(alpha.isEmpty());
    QVERIFY(alpha.id_.isValid() == false);

    // Test: valid table, valid database
    alpha.id_ = 75;
    alpha.setDbTable(DbTable("projects", db));
    QVERIFY(alpha.dbTable().count() > 0);
    QVERIFY(alpha.count() == 3);
    QVERIFY(alpha.contains("name"));
    QVERIFY(alpha.value(0).isNull());
    QVERIFY(alpha.value(1).isNull());
    QVERIFY(alpha.value(2).isNull());
    QVERIFY(alpha.id_.isValid() == false);
    QVERIFY(alpha.dbTable().tableName() == "projects");

    // Test: Reset to invalid table (which must be created anew, because it is not possible to migrate from valid dbTable
    //       to invalid state (i.e. table.setTable("", QSqlDatabase()) will fail).
    // -- Also check that the id_ is reset
    alpha.id_ = 100;
    alpha.setDbTable(DbTable("", QSqlDatabase()));
    QVERIFY(alpha.dbTable().isEmpty());
    QVERIFY(alpha.isEmpty());
    QVERIFY(alpha.id_.isValid() == false);

    // Test: from valid to valid
    alpha.setDbTable(DbTable("projects", db));
    alpha.id_ = 125;
    alpha.setDbTable(DbTable("aseqs", db));
    QVERIFY(alpha.dbTable().count() == 2);
    QVERIFY(alpha.value(0).isNull());
    QVERIFY(alpha.value(1).isNull());
    QVERIFY(alpha.id_.isValid() == false);
    QVERIFY(alpha.dbTable().tableName() == "aseqs");
}

void TestCrudSqlRecord::append()
{
    QSqlDatabase db = cleanDatabase();

    CrudSqlRecord record(DbTable("projects", db));

    record.clear();
    QVERIFY(record.dbTable().tableName() == "projects");

    // Test: append invalid column
    QVERIFY(record.append("column") == false);
    QVERIFY(record.isEmpty());

    // Test: append valid column
    QVERIFY(record.append("id"));
    QVERIFY(record.count() == 1);
    QVERIFY(record.fieldName(0) == "id");

    // Test: append duplicate column name
    QVERIFY(record.append("id") == false);
    QVERIFY(record.count() == 1);
    QVERIFY(record.fieldName(0) == "id");

    // Test: append the other columns
    QVERIFY(record.append("name"));
    QVERIFY(record.count() == 2);
    QVERIFY(record.fieldName(1) == "name");

    QVERIFY(record.append("created"));
    QVERIFY(record.count() == 3);
    QVERIFY(record.fieldName(2) == "created");

    // Test: append another invalid column
    QVERIFY(record.append("missing") == false);
}

void TestCrudSqlRecord::insert()
{
    QSqlDatabase db = cleanDatabase();

    CrudSqlRecord record(DbTable("projects", db));

    record.clear();

    // Test: insert invalid column
    QVERIFY(record.insert(0, "missing") == false);
    QVERIFY(record.isEmpty());

    // Test: insert valid column
    QVERIFY(record.insert(0, "name"));
    QVERIFY(record.count() == 1);
    QVERIFY(record.fieldName(0) == "name");

    // Test: insert duplicate valid column
    QVERIFY(record.insert(0, "name") == false);
    QVERIFY(record.count() == 1);
    QVERIFY(record.fieldName(0) == "name");

    // Test: insert another column
    QVERIFY(record.insert(0, "id"));
    QVERIFY(record.count() == 2);
    QVERIFY(record.fieldName(0) == "id");
}

void TestCrudSqlRecord::replace()
{
    QSqlDatabase db = cleanDatabase();

    CrudSqlRecord record(DbTable("projects", db));

    QVERIFY(record.count() > 0);

    // Test: replace with invalid column
    QVERIFY(record.replace(0, "invalid_column") == false);
    QVERIFY(record.count() == 3);
    QVERIFY(record.fieldName(0) == "id");

    record.remove(0);

    // Test: replace with valid column
    QVERIFY(record.replace(0, "id"));
    QVERIFY(record.count() == 2);
    QVERIFY(record.fieldName(0) == "id");
    QVERIFY(record.fieldName(1) == "created");

    // Test: replacing valid column with itself should work
    QVERIFY(record.replace(0, "id"));
    QVERIFY(record.count() == 2);
    QVERIFY(record.fieldName(0) == "id");
    QVERIFY(record.fieldName(1) == "created");
}

void TestCrudSqlRecord::remove()
{
    QSqlDatabase db = cleanDatabase();
    CrudSqlRecord record(DbTable("projects", db));

    // Test: remove missing field
    QVERIFY(record.remove("missing") == false);
    QVERIFY(record.count() == 3);

    // Test: remove valid field
    QVERIFY(record.remove("created"));
    QVERIFY(record.contains("created") == false);
    QVERIFY(record.count() == 2);
}

void TestCrudSqlRecord::defaultFields()
{
    QSqlDatabase db = cleanDatabase();

    CrudSqlRecord record(DbTable("projects", db));

    // Remove all the fields
    record.clear();

    // Reset them to the basic set
    record.defaultFields();

    QVERIFY(record.count() == 3);
    QVERIFY(record.fieldName(0) == "id");
    QVERIFY(record.fieldName(1) == "name");
    QVERIFY(record.fieldName(2) == "created");
}

void TestCrudSqlRecord::hasData()
{
    QSqlDatabase db = cleanDatabase();

    CrudSqlRecord record(DbTable("projects", db));

    // Test: clean record should not have data
    QVERIFY(record.hasData() == false);

    // Test: add piece of data
    record.setValue("name", "New project");
    QVERIFY(record.hasData());

    // Test: changing database table should clear out any data
    record.setDbTable(DbTable("aseqs", db));
    QVERIFY(record.hasData() == false);

    // Test: remove field that originally has data
    record.setDbTable(DbTable("projects", db));
    record.setValue("name", "New project");
    QVERIFY(record.hasData());
    record.remove("name");
    QVERIFY(record.hasData() == false);
}

void TestCrudSqlRecord::read()
{
    QSqlDatabase db = cleanDatabase();

    CrudSqlRecord record(DbTable("", QSqlDatabase()));

    // Test: read from invalid table
    QVERIFY(record.isEmpty());
    QVERIFY(record.read() == -1);
    QVERIFY(record.read(100) == -1);

    // --> empty table
    record.setDbTable(DbTable("projects", db));

    // Test: read with invalid id and id_
    QVERIFY(record.read() == -1);

    // Test: read from empty table
    record.id_ = 100;
    QVERIFY(record.read() == 0);
    QVERIFY(record.id_ == 100);
    QVERIFY(record.read(250) == 0);
    QVERIFY(record.hasData() == false);
    QVERIFY(record.id_ == 250);

    // Test: read valid row
    db.exec("INSERT INTO projects (id, name, created) values (375, 'Agile', 'today')");
    record.id_ = 375;
    QVERIFY(record.read() == 1);
    QVERIFY(record.hasData());
    QVERIFY(record.value(0) == 375);
    QVERIFY(record.value(1) == "Agile");
    QVERIFY(record.value(2) == "today");

    // Test: read with id passed as argument
    db.exec("INSERT INTO projects (id, name, created) values (975, 'Genomics', 'yesterday')");
    QVERIFY(record.read(975) == 1);
    QVERIFY(record.id_ == 975);
    QVERIFY(record.hasData());
    QVERIFY(record.value(0) == 975);
    QVERIFY(record.value(1) == "Genomics");
    QVERIFY(record.value(2) == "yesterday");

    // Test: back to reading invalid record
    QVERIFY(record.read(10) == 0);
    QVERIFY(record.hasData() == false);
    QVERIFY(record.id_ == 10);

    // Test: primary key is not in the select list, should still set the id_
    record.remove("id");
    QVERIFY(record.read(975));
    QVERIFY(record.id_ == 975);
    QVERIFY(record.hasData());
    QVERIFY(record.value(0) == "Genomics");
    QVERIFY(record.value(1) == "yesterday");
}

void TestCrudSqlRecord::erase()
{
    QSqlDatabase db = cleanDatabase();

    CrudSqlRecord record(DbTable("", QSqlDatabase()));

    // Test: delete from invalid table
    record.id_ = 30;
    QVERIFY(record.erase() == -1);
    QVERIFY(record.id_.isNull());

    QVERIFY(record.erase(100) == -1);
    QVERIFY(record.id_.isValid() == false);

    // --> empty table
    record.setDbTable(DbTable("projects", db));

    // Test: erase with invalid id and id_
    QVERIFY(record.erase() == -1);

    // Test: erase with "dangling" id
    QVERIFY(record.erase(34) == 0);
    QVERIFY(record.id_.isValid() == false);

    // Test: erase with "dangling" id_
    record.id_ = 34;
    QVERIFY(record.erase() == 0);
    QVERIFY(record.id_.isValid() == false);

    // Test: all stored values should be cleared regardless of any rows deleted
    record.setValue("name", "New project");
    QVERIFY(record.erase(100) == 0);
    QVERIFY2(record.value("name").isNull(), QString("%1").arg(record.value("name").toString()).toAscii());

    // Test: delete "dangling" id from non-empty table
    db.exec("INSERT INTO projects (id, name, created) values (375, 'Agile', 'today')");
    db.exec("INSERT INTO projects (id, name, created) values (975, 'Genomics', 'yesterday')");

    QVERIFY(record.erase(200) == 0);

    // --> Check that rows are still present
    QSqlQuery s_rows(db);
    s_rows.prepare("SELECT count(*) FROM projects");
    QVERIFY(s_rows.exec());
    QVERIFY(s_rows.next());
    QVERIFY(s_rows.value(0) == 2);

    // Test: delete with valid id
    QVERIFY(record.erase(375) == 1);
    QVERIFY(record.id_.isValid() == false);

    // --> Make sure that it was deleted from the database
    QVERIFY(s_rows.exec());
    QVERIFY(s_rows.next());
    QVERIFY(s_rows.value(0) == 1);

    // Test: delete with valid id_
    record.id_ = 975;
    QVERIFY(record.erase() == 1);
    QVERIFY(record.id_.isValid() == false);

    // --> Make sure that it was deleted from the database
    QVERIFY(s_rows.exec());
    QVERIFY(s_rows.next());
    QVERIFY(s_rows.value(0) == 0);
}

void TestCrudSqlRecord::save_insert()
{
    QSqlDatabase db = cleanDatabase();
    CrudSqlRecord record(DbTable("", QSqlDatabase()));

    // Test: Insert with empty values on invalid table
    QVERIFY(record.save() == -1);

    record.setDbTable(DbTable("projects", db));

    // Test: Insert with completely empty record
    QVERIFY(record.save() == -2);

    // Test: Insert with all required fields present
    record.setValue("name", "New project");
    QVERIFY(record.save() == 1);
    QVERIFY2(record.id_.toInt() == 1, QString("%1").arg(record.id_.toString()).toAscii());

    QSqlQuery s_rows(db);
    s_rows.prepare("SELECT count(*) FROM projects");
    QVERIFY(s_rows.exec());
    QVERIFY(s_rows.next());
    QVERIFY(s_rows.value(0) == 1);

    QSqlQuery query = db.exec("SELECT * FROM projects WHERE id = 1");
    QVERIFY(query.next());
    QVERIFY(query.value(0).toInt() == 1);
    QVERIFY(query.value(1).toString() == "New project");
    QVERIFY(query.value(2).isNull());

    // Test: insert with specific id and all fields present
    record.setValue("id", 5);
    record.setValue("name", "Second project");
    record.setValue("created", "Very recently");
    record.id_.clear();
    QVERIFY(record.save() == 1);
    QVERIFY(record.id_.toInt() == 5);

    QVERIFY(s_rows.exec());
    QVERIFY(s_rows.next());
    QVERIFY(s_rows.value(0) == 2);

    query = db.exec("SELECT * FROM projects WHERE id = 5");
    QVERIFY(query.next());
    QVERIFY(query.value(0).toInt() == 5);
    QVERIFY(query.value(1).toString() == "Second project");
    QVERIFY(query.value(2).toString() == "Very recently");

    // Test: insert with pre-existing id
    record.id_.clear();
    QVERIFY(record.save() == -2);

    // Test: insert without primary key field present
    record.remove("id");
    record.setValue("name", "Response_reg");
    record.setNull("created");
    record.id_.clear();
    QVERIFY(record.save() == 1);
    QVERIFY(record.id_.isNull() == false);
    query = db.exec("SELECT id, name, created FROM projects WHERE id = " + record.id_.toString());
    QVERIFY(query.next());
    QVERIFY(query.value(0).toInt() == record.id_.toInt());
    QVERIFY(query.value(1).toString() == "Response_reg");
    QVERIFY(query.value(2).isNull());

    // Test: insert on table without auto-generated primary key field
    db.exec("CREATE TABLE databases (name text not null primary key, description text)");
    CrudSqlRecord record2(DbTable("databases", db));
    QVERIFY(record2.dbTable().primaryKeyField() == "name");
    QVERIFY(record2.save() == -2);

    record2.setValue("name", "NR");
    QVERIFY(record2.save() == 1);
    QVERIFY(record2.id_ == "NR");

    // Test: insert with subset of fields present
    record2.remove("description");
    record2.setValue("name", "PDB");
    record2.id_.clear();
    QVERIFY(record2.save() == 1);
    QVERIFY(record2.id_ == "PDB");

    // Test: insert on table with no fields specified in the record
    db.exec("CREATE TABLE amounts (id integer primary key autoincrement, value text)");
    CrudSqlRecord record3(DbTable("amounts", db));
    record3.clear();
    QVERIFY(record3.save() == 1);
    QVERIFY(record3.id_ == 1);
}

void TestCrudSqlRecord::save_update()
{
    QSqlDatabase db = cleanDatabase();
    QSqlQuery select(db);
    select.prepare("SELECT id, name, created FROM projects WHERE id = ?");
    select.setForwardOnly(true);

    CrudSqlRecord record(DbTable("", QSqlDatabase()));

    // Test: update with empty data on invalid table
    record.id_ = 10;
    QVERIFY(record.save() == -1);
    QVERIFY(record.id_ == 10);

    record.setDbTable(DbTable("projects", db));
    // Note: record has all the fields contained in the projects table and they are initially set to NULL

    // Test: update on non-existent record with missing required field; this will attempt to set all fields to NULL
    //       where pk = 10. Since no rows exist in this table with that identifier, should return 0 fields affected
    record.id_ = 10;            // Since we set a new database table, its previous value would have been cleared
    QVERIFY(record.save() == 0);

    // Test: update on non-existent record with required field present in data
    record.setValue("name", "Old project");
    QVERIFY(record.save() == 0);

    // Test: update single field (name) on valid record
    QVERIFY2(db.exec("INSERT INTO projects (id, name, created) values (10, 'New project', 'Recently')").lastError().type() == QSqlError::NoError, db.lastError().text().toAscii());

    // record::name still equals "Old project"
    record.remove("id");
    record.remove("created");
    QVERIFY(record.save() == 1);
    QVERIFY(record.id_ == 10);

    select.bindValue(0, 10);    // Select all fields with id = 10
    QVERIFY(select.exec());
    QVERIFY(select.next());
    QVERIFY(select.value(0) == 10);
    QVERIFY(select.value(1) == "Old project");
    QVERIFY(select.value(2) == "Recently");

    // Test: update two fields on valid record
    record.clear();
    record.append("name");
    record.append("created");
    record.setValue("name", "PAS domains");
    record.setValue("created", "Never");
    QVERIFY(record.save() == 1);
    QVERIFY(record.id_ == 10);

    QVERIFY(select.exec());
    QVERIFY(select.next());
    QVERIFY(select.value(0) == 10);
    QVERIFY(select.value(1) == "PAS domains");
    QVERIFY(select.value(2) == "Never");

    // Test: update all fields including the primary key
    record.append("id");
    record.setValue("id", 20);
    record.setValue("name", "Quorum-sensing");
    record.setValue("created", "7 April 2010");
    QVERIFY(record.save() == 1);
    QVERIFY(record.id_ == 20);

    select.bindValue(0, 20);
    QVERIFY(select.exec());
    QVERIFY(select.next());
    QVERIFY(select.value(0) == 20);
    QVERIFY(select.value(1) == "Quorum-sensing");
    QVERIFY(select.value(2) == "7 April 2010");

    // Test: update field to invalid state should fail
    record.setNull("name");
    QVERIFY(record.save() == -2);
    QVERIFY(record.id_ == 20);

    // Ensure that nothing has changed
    select.bindValue(0, 20);
    QVERIFY(select.exec());
    QVERIFY(select.next());
    QVERIFY(select.value(0) == 20);
    QVERIFY(select.value(1) == "Quorum-sensing");
    QVERIFY(select.value(2) == "7 April 2010");

    // Test: update only with one field selected
    record.clear();
    record.append("name");
    record.setValue("name", "Chemotaxis");
    QVERIFY(record.save() == 1);
    QVERIFY(record.id_ == 20);

    QVERIFY(select.exec());
    QVERIFY(select.next());
    QVERIFY(select.value(0) == 20);
    QVERIFY(select.value(1) == "Chemotaxis");
    QVERIFY(select.value(2) == "7 April 2010");

    // Test: update the primary key field only
    record.remove("name");
    record.append("id");
    record.setValue("id", 30);

    QVERIFY(record.save() == 1);
    QVERIFY(record.id_ == 30);

    select.bindValue(0, 30);
    QVERIFY(select.exec());
    QVERIFY(select.next());
    QVERIFY(select.value(0) == 30);
    QVERIFY(select.value(1) == "Chemotaxis");
    QVERIFY(select.value(2) == "7 April 2010");

    // Test: update without any fields
    record.clear();
    QVERIFY(record.save() == -1);

    // Ensure nothing changed
    QVERIFY(select.exec());
    QVERIFY(select.next());
    QVERIFY(select.value(0) == 30);
    QVERIFY(select.value(1) == "Chemotaxis");
    QVERIFY(select.value(2) == "7 April 2010");
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
QSqlDatabase TestCrudSqlRecord::cleanDatabase()
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


QTEST_MAIN(TestCrudSqlRecord)
#include "TestCrudSqlRecord.moc"
