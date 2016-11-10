/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "DbDataSource.h"

// Exceptions
#include "exceptions/DatabaseError.h"
#include "exceptions/InvalidConnectionError.h"

class TestDbDataSource : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();     // Also tests connectionName

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setConnectionName();
    void checkTable();
    void database();
    void erase();
    void read();
    void readAll();
    void insert();
    void insertMany();
    void update();
    void updateMany();
    void truncate();

    void resetSqliteSequence();

    void savepoint();
    void rollbackToSavePoint();
    void releaseSavePoint();
};

void TestDbDataSource::constructor()
{
    // Test: empty constructor should work
    DbDataSource dbs;
    QVERIFY(dbs.connectionName().isEmpty());
    QVERIFY(dbs.database().isValid() == false);

    // Test: "" connection name should work
    DbDataSource dbs2("");
    QVERIFY(dbs2.connectionName().isEmpty());
    QVERIFY(dbs2.database().isValid() == false);

    // Test: invalid connection name in constructor
    try
    {
        DbDataSource dbs3("alignshop");
    }
    catch(InvalidConnectionError &e)
    {
        QCOMPARE(e.connectionName(), QString("alignshop"));
    }
    catch(...)
    {
        QVERIFY(0);
    }

    // Test: valid database connection
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        try
        {
            DbDataSource dbs4("test_db");
            QVERIFY(dbs4.database().isOpen());
        }
        catch(...)
        {
            QVERIFY(0);
        }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::setConnectionName()
{
    DbDataSource dbs;

    // ---------------------------------------------
    // Test: set connection name to invalid database
    try
    {
        dbs.setConnectionName("test_db");
        QVERIFY(0);
    }
    catch(InvalidConnectionError &e)
    {
        QCOMPARE(e.connectionName(), QString("test_db"));
        QVERIFY(dbs.connectionName().isEmpty());
    }
    catch(...) { QVERIFY(0); }


    // ---------------------------------------------
    // Test: set connection name to empty should work
    try
    {
        dbs.setConnectionName("");
    }
    catch(...) { QVERIFY(0); }


    // ---------------------------------------------
    // Test: set connection name to valid database
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        try
        {
            dbs.setConnectionName("test_db");
            QCOMPARE(dbs.connectionName(), QString("test_db"));
            QVERIFY(dbs.database().isOpen());

            // Go back to empty connection
            dbs.setConnectionName("");
        }
        catch(...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::checkTable()
{
    DbDataSource dbs;
    {
        try
        {
            dbs.checkTable("missing");
            QVERIFY(0);
        }
        catch (InvalidConnectionError &e) { QVERIFY(1); }

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        try
        {
            dbs.setConnectionName("test_db");

            // Test: valid database but empty table name
            try
            {
                dbs.checkTable("");
                QVERIFY(0);
            }
            catch (DatabaseError &e) { QVERIFY(1); }

            try
            {
                dbs.checkTable("missing");
                QVERIFY(0);
            }
            catch (DatabaseError &e) { QVERIFY(1); }

            // Test: valid present table name
            QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key autoincrement)").lastError().type() == QSqlError::NoError);

            dbs.checkTable("astrings");
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::database()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        try
        {
            DbDataSource dbs("test_db");
            QVERIFY(dbs.database().isValid());
            QVERIFY(dbs.database().isOpen());
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::erase()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key, sequence text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (30, 'GHI')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (40, 'JKL')").lastError().type() == QSqlError::NoError);

        // Ensure database is in correct state
        QSqlQuery query = db.exec("SELECT id, sequence FROM astrings WHERE id = 10");
        query.next();
        QCOMPARE(query.value(0).toInt(), 10);
        QCOMPARE(query.value(1).toString(), QString("ABC"));

        query = db.exec("SELECT id, sequence FROM astrings WHERE id = 20");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 20);
        QCOMPARE(query.value(1).toString(), QString("DEF"));

        try
        {
            DbDataSource dbs;

            // ---------------------------------------------
            // Test: erase without valid connection
            try
            {
                dbs.erase("astrings", 0);
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }


            // Setup the connection properly
            try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: erase invalid table name
            try
            {
                dbs.erase("dstrings", 0);
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }


            // ---------------------------------------------
            // Test: erase with calls that should not throw exception
            try
            {
                // Invalid id
                QCOMPARE(dbs.erase("astrings", QVariant()), 0);

                // Non-existent ids
                QCOMPARE(dbs.erase("astrings", 1), 0);
                QCOMPARE(dbs.erase("astrings", -1), 0);
                QCOMPARE(dbs.erase("astrings", 300), 0);

                // Valid ids
                QCOMPARE(dbs.erase("astrings", 10), 1);
                QCOMPARE(dbs.erase("astrings", 20), 1);
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 10 OR id = 20");
                QVERIFY(query.next() == false);

                // un-matching conditions
                QCOMPARE(dbs.erase("astrings", 30, "sequence = 'JKL'"), 0);
                QCOMPARE(dbs.erase("astrings", 40, "sequence = 'GHI'"), 0);

                // Matching conditions
                QCOMPARE(dbs.erase("astrings", 30, "sequence = 'GHI'"), 1);
                QCOMPARE(dbs.erase("astrings", 40, "sequence = 'JKL'"), 1);
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 30 OR id = 40");
                QVERIFY(query.next() == false);
            }
            catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: invalid conditions should throw DatabaseError
            try
            {
                dbs.erase("astrings", 30, "blakjdfe =d 3");
                QVERIFY(0);
            }
            catch (DatabaseError &)
            {
                QVERIFY(1);
            }
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::read()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key, sequence text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (30, 'GHI')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (40, 'JKL')").lastError().type() == QSqlError::NoError);

        DbDataSource dbs;

        try
        {
            DataRow row;

            // ---------------------------------------------
            // Test: read without valid connection
            try
            {
                dbs.read("astrings", 0);
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly
            try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: read from invalid table
            try
            {
                row = dbs.read("missing", 34);
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }


            // ---------------------------------------------
            // Test: read with calls that should not throw an exception
            try
            {
                // Test: valid table, invalid id
                row = dbs.read("astrings", QVariant());
                QVERIFY(row.isEmpty());
                QVERIFY(row.id_.isNull());

                // Test: valid table, non-existent id
                row = dbs.read("astrings", 100);
                QVERIFY(row.isEmpty());
                QVERIFY(row.id_.isNull());

                // Test: valid table, valid id
                row = dbs.read("astrings", 10);
                QVERIFY(row.isEmpty() == false);
                QCOMPARE(row.value("id").toInt(), 10);
                QCOMPARE(row.value("sequence").toString(), QString("ABC"));
                QCOMPARE(row.id_.toInt(), 10);

                // Test: Null for fields returns all fields
                row = dbs.read("astrings", 10, QStringList());
                QVERIFY(row.isEmpty() == false);
                QCOMPARE(row.value("id").toInt(), 10);
                QCOMPARE(row.value("sequence").toString(), QString("ABC"));
                QCOMPARE(row.id_.toInt(), 10);

                // Test: Define only a subset of fields
                row = dbs.read("astrings", 10, QStringList() << "sequence");
                QVERIFY(row.isEmpty() == false);
                QVERIFY(row.value("id").isValid() == false);
                QCOMPARE(row.value("sequence").toString(), QString("ABC"));
                QCOMPARE(row.id_.toInt(), 10);

                // ----------------------------------
                // Test: mismatching conditions
                row = dbs.read("astrings", 40, QStringList(), "sequence = 'ABC'");
                QVERIFY(row.isEmpty());
                QVERIFY(row.id_.isNull());

                // Test: matching conditions
                row = dbs.read("astrings", 30, QStringList(), "sequence = 'GHI'");
                QVERIFY(row.isEmpty() == false);
                QVERIFY(row.id_ == 30);
                QVERIFY(row.value("sequence").toString() == "GHI");
            }
            catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: extra field
            try
            {
                dbs.read("astrings", 20, QStringList() << "sequence" << "organism");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }

            // Test: single field that is not present in the database table
            try
            {
                dbs.read("astrings", 30, QStringList() << "organism");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::readAll()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key, sequence text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (30, 'GHI')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (40, 'JKL')").lastError().type() == QSqlError::NoError);

        DbDataSource dbs;

        try
        {
            QList<DataRow> rows;

            // ---------------------------------------------
            // Test: readAll without valid connection
            try
            {
                dbs.readAll("astrings");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly
            try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: readAll from invalid table
            try
            {
                dbs.readAll("invalid");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }


            // ---------------------------------------------
            // Test: read with calls that should not throw an exception
            try
            {
                // Test: valid table with defaults
                rows = dbs.readAll("astrings");
                QCOMPARE(rows.size(), 4);

                // Without an order by clause it's impossible to be sure of the order they are in. Thus we have to check for all values
                foreach (DataRow row, rows)
                {
                    QVERIFY(row.value("id") == 10 || row.value("id") == 20 || row.value("id") == 30 || row.value("id") == 40);
                    QVERIFY(row.value("sequence") == "ABC" || row.value("sequence") == "DEF" || row.value("sequence") == "GHI" || row.value("sequence") == "JKL");
                }

                // Same as above but with defaults and default ascending order by
                rows = dbs.readAll("astrings", QStringList(), "", OrderedField("id"));
                QCOMPARE(rows.size(), 4);
                QCOMPARE(rows.at(0).value("id").toInt(), 10);
                QCOMPARE(rows.at(1).value("id").toInt(), 20);
                QCOMPARE(rows.at(2).value("id").toInt(), 30);
                QCOMPARE(rows.at(3).value("id").toInt(), 40);

                QCOMPARE(rows.at(0).value("sequence").toString(), QString("ABC"));
                QCOMPARE(rows.at(1).value("sequence").toString(), QString("DEF"));
                QCOMPARE(rows.at(2).value("sequence").toString(), QString("GHI"));
                QCOMPARE(rows.at(3).value("sequence").toString(), QString("JKL"));

                // Test: all rows, but explicitly in ascending order
                rows = dbs.readAll("astrings", QStringList(), "", OrderedField("id", eSortAscending));
                QCOMPARE(rows.size(), 4);
                QCOMPARE(rows.at(0).value("id").toInt(), 10);
                QCOMPARE(rows.at(1).value("id").toInt(), 20);
                QCOMPARE(rows.at(2).value("id").toInt(), 30);
                QCOMPARE(rows.at(3).value("id").toInt(), 40);

                QCOMPARE(rows.at(0).value("sequence").toString(), QString("ABC"));
                QCOMPARE(rows.at(1).value("sequence").toString(), QString("DEF"));
                QCOMPARE(rows.at(2).value("sequence").toString(), QString("GHI"));
                QCOMPARE(rows.at(3).value("sequence").toString(), QString("JKL"));

                // Test: all rows, but in descending id order
                rows = dbs.readAll("astrings", QStringList(), "", OrderedField("id", eSortDescending));
                QCOMPARE(rows.size(), 4);
                QCOMPARE(rows.at(0).value("id").toInt(), 40);
                QCOMPARE(rows.at(1).value("id").toInt(), 30);
                QCOMPARE(rows.at(2).value("id").toInt(), 20);
                QCOMPARE(rows.at(3).value("id").toInt(), 10);

                QCOMPARE(rows.at(0).value("sequence").toString(), QString("JKL"));
                QCOMPARE(rows.at(1).value("sequence").toString(), QString("GHI"));
                QCOMPARE(rows.at(2).value("sequence").toString(), QString("DEF"));
                QCOMPARE(rows.at(3).value("sequence").toString(), QString("ABC"));

                // Test: conditions in the mix
                rows = dbs.readAll("astrings", QStringList(), "id IN (10, 40)", OrderedField("id", eSortAscending));
                QCOMPARE(rows.size(), 2);
                QCOMPARE(rows.at(0).value("id").toInt(), 10);
                QCOMPARE(rows.at(1).value("id").toInt(), 40);

                QCOMPARE(rows.at(0).value("sequence").toString(), QString("ABC"));
                QCOMPARE(rows.at(1).value("sequence").toString(), QString("JKL"));

                // Test: conditions plus field plus descending order
                rows = dbs.readAll("astrings", QStringList() << "sequence", "id IN (10, 40)", OrderedField("id", eSortDescending));
                QCOMPARE(rows.size(), 2);
                // Should not have any id field
                QVERIFY(rows.at(0).value("id").isValid() == false);
                QVERIFY(rows.at(1).value("id").isValid() == false);

                QCOMPARE(rows.at(0).value("sequence").toString(), QString("JKL"));
                QCOMPARE(rows.at(1).value("sequence").toString(), QString("ABC"));

                // Test: limit
                rows = dbs.readAll("astrings", QStringList(), "", OrderedField("id"), 2);
                QCOMPARE(rows.size(), 2);
                QCOMPARE(rows.at(0).value("id").toInt(), 10);
                QCOMPARE(rows.at(1).value("id").toInt(), 20);

                QCOMPARE(rows.at(0).value("sequence").toString(), QString("ABC"));
                QCOMPARE(rows.at(1).value("sequence").toString(), QString("DEF"));

                // Test: negative limit gets all the rows
                rows = dbs.readAll("astrings", QStringList(), "", OrderedField("id"), -34);
                QCOMPARE(rows.size(), 4);
                QCOMPARE(rows.at(0).value("id").toInt(), 10);
                QCOMPARE(rows.at(1).value("id").toInt(), 20);
                QCOMPARE(rows.at(2).value("id").toInt(), 30);
                QCOMPARE(rows.at(3).value("id").toInt(), 40);

                QCOMPARE(rows.at(0).value("sequence").toString(), QString("ABC"));
                QCOMPARE(rows.at(1).value("sequence").toString(), QString("DEF"));
                QCOMPARE(rows.at(2).value("sequence").toString(), QString("GHI"));
                QCOMPARE(rows.at(3).value("sequence").toString(), QString("JKL"));
            }
            catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: no common fields
            try
            {
                dbs.readAll("astrings", QStringList() << "organism");
            }
            catch (DatabaseError &) { QVERIFY(1); }
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::insert()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key, sequence text)").lastError().type() == QSqlError::NoError);

        DbDataSource dbs;

        try
        {
            DataRow blankDataRow;

            // ---------------------------------------------
            // Test: insert without valid connection
            try
            {
                dbs.insert(blankDataRow, "astrings");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly
            try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: insert into invalid table
            try
            {
                dbs.insert(blankDataRow, "invalid");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }


            // ---------------------------------------------
            // Test: insert calls that should not throw an exception
            try
            {
                // Test: inserts with no data should insert default values
                DataRow astring;
                QVERIFY(dbs.insert(astring, "astrings"));
                QSqlQuery query = db.exec("SELECT id, sequence FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QVERIFY(query.value(1).isNull());
                QCOMPARE(astring.id_.toInt(), 1);
                QVERIFY(astring.contains("id") == false);

                // Test: insert but only save id field
                db.exec("DELETE FROM astrings");
                DataRow astring2;
                astring2.setValue("id", 99);
                astring2.setValue("sequence", "ABC");
                QVERIFY(dbs.insert(astring2, "astrings", QStringList() << "id"));
                query = db.exec(QString("SELECT id, sequence FROM astrings"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 99);
                QVERIFY(query.value(1).isNull());
                QCOMPARE(astring2.id_.toInt(), 99);

                // Test: insert but only save the sequence field
                //       Have defined id field of 99, but we don't save this value in the insert method
                db.exec("DELETE FROM astrings");
                DataRow astring3;
                astring3.setValue("id", 99);
                astring3.setValue("sequence", "ABC");
                QVERIFY(dbs.insert(astring3, "astrings", QStringList() << "sequence"));
                query = db.exec(QString("SELECT id, sequence FROM astrings"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QVERIFY(query.value(1).toString() == "ABC");
                QCOMPARE(astring3.id_.toInt(), 1);

                // Test: insert without explicitly specifying fields
                DataRow astring4;
                astring4.setValue("id", 345);
                astring4.setValue("sequence", "MNO");
                db.exec("DELETE FROM astrings");
                QVERIFY(dbs.insert(astring4, "astrings"));
                query = db.exec(QString("SELECT id, sequence FROM astrings"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 345);
                QVERIFY(query.value(1).toString() == "MNO");
                QCOMPARE(astring4.id_.toInt(), 345);

                // Test: insert should fail with defined id
                DataRow astring5(900);
                QVERIFY(dbs.insert(astring5, "astrings") == false);

                // Test: explicitly specify the id field, but have it null in the dataRow
                db.exec("DELETE FROM astrings");
                DataRow astring6;
                astring6.setValue("id", QVariant());
                QVERIFY(dbs.insert(astring6, "astrings", QStringList() << "id"));
                QVERIFY(astring6.id_.toInt() == 1);
            }
            catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: empty data row, valid table, invalid field
            try
            {
                db.exec("DELETE FROM astrings");
                DataRow astring;
                dbs.insert(astring, "astrings", QStringList() << "organism");
                QVERIFY(0);
            }
            catch (DatabaseError &)
            {
                QVERIFY(1);
                QSqlQuery query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QVERIFY(query.value(0).toInt() == 0);
            }


            // ---------------------------------------------
            // Test: one field in common, one not => failure
            try
            {
                db.exec("DELETE FROM astrings");
                DataRow astring;
                dbs.insert(astring, "astrings", QStringList() << "sequence" << "organism");
                QVERIFY(0);
            }
            catch (DatabaseError &)
            {
                QVERIFY(1);
                QSqlQuery query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QVERIFY(query.value(0).toInt() == 0);
            }
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::insertMany()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key, sequence text)").lastError().type() == QSqlError::NoError);

        DbDataSource dbs;

        try
        {
            QList<DataRow> dataRows;

            // ---------------------------------------------
            // Test: insert without valid connection
            try
            {
                dbs.insert(dataRows, "astrings");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly
            try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: insert into invalid table
            try
            {
                dbs.insert(dataRows, "invalid");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }


            // ---------------------------------------------
            // Test: insert calls that should not throw an exception
            try
            {
                // Test: inserting empty list should return 0
                QVERIFY(dbs.insert(dataRows, "astrings") == 0);
                QVERIFY(dbs.insert(dataRows, "astrings", QStringList() << "id" << "sequence") == 0);
                QVERIFY(dbs.insert(dataRows, "astrings", QStringList()) == 0);

                QSqlQuery query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QVERIFY(query.value(0).toInt() == 0);

                // Test: inserts with no data should insert default values
                dataRows.append(DataRow());
                QVERIFY(dbs.insert(dataRows, "astrings"));
                query = db.exec("SELECT id, sequence FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QVERIFY(query.value(1).isNull());
                QCOMPARE(dataRows.at(0).id_.toInt(), 1);
                QVERIFY(dataRows.at(0).value("id").isNull());


                // -------------------------------------------------
                // Single insert tests repeated in list context
                // Test: insert but only save id field
                dataRows.clear();
                db.exec("DELETE FROM astrings");
                DataRow astring;
                astring.setValue("id", 99);
                astring.setValue("sequence", "ABC");
                dataRows.append(astring);
                QVERIFY(dbs.insert(dataRows, "astrings", QStringList() << "id") == 1);
                query = db.exec(QString("SELECT id, sequence FROM astrings"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 99);
                QVERIFY(query.value(1).isNull());
                QCOMPARE(dataRows.at(0).id_.toInt(), 99);

                // Test: insert but only save the sequence field
                //       Have defined id field of 99, but we don't save this value in the insert method
                dataRows.clear();
                db.exec("DELETE FROM astrings");
                DataRow astring2;
                astring2.setValue("id", 99);
                astring2.setValue("sequence", "ABC");
                dataRows.append(astring2);
                QVERIFY(dbs.insert(dataRows, "astrings", QStringList() << "sequence") == 1);
                query = db.exec(QString("SELECT id, sequence FROM astrings"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QVERIFY(query.value(1).toString() == "ABC");
                QCOMPARE(dataRows.at(0).id_.toInt(), 1);

                // Test: insert without explicitly specifying fields
                dataRows.clear();
                DataRow astring3;
                astring3.setValue("id", 345);
                astring3.setValue("sequence", "MNO");
                dataRows.append(astring3);
                db.exec("DELETE FROM astrings");
                QVERIFY(dbs.insert(dataRows, "astrings") == 1);
                query = db.exec(QString("SELECT id, sequence FROM astrings"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 345);
                QVERIFY(query.value(1).toString() == "MNO");
                QCOMPARE(dataRows.at(0).id_.toInt(), 345);

                // Test: insert should fail with defined id
                dataRows.clear();
                dataRows.append(DataRow(900));
                QVERIFY(dbs.insert(dataRows, "astrings") == 0);

                // Test: explicitly specify the id field, but have it null in the dataRow
                dataRows.clear();
                db.exec("DELETE FROM astrings");
                dataRows.append(DataRow());
                dataRows[0].setValue("id", QVariant());
                QVERIFY(dbs.insert(dataRows, "astrings", QStringList() << "id") == 1);
                QVERIFY(dataRows.at(0).id_.toInt() == 1);


                // ---------------------------------------------------------
                // Multiple entry list insert
                dataRows.clear();
                db.exec("DELETE FROM astrings");

                // Test: good one, then invalid one
                dataRows.append(DataRow());
                dataRows.append(DataRow(10));
                QVERIFY(dbs.insert(dataRows, "astrings") == 1);
                query = db.exec(QString("SELECT id FROM astrings"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QVERIFY(query.next() == false);

                // Check the values inside the dataRows array
                QVERIFY(dataRows.at(0).id_.toInt() == 1);


                // Test: invalid one, then good one
                dataRows.clear();
                db.exec("DELETE FROM astrings");
                dataRows.append(DataRow(10));
                dataRows.append(DataRow());
                QVERIFY(dbs.insert(dataRows, "astrings") == 0);
                query = db.exec(QString("SELECT id FROM astrings"));
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next() == false);
                QVERIFY(dataRows.at(1).id_.isNull());


                // Test: two good rows with data
                dataRows.clear();
                db.exec("DELETE FROM astrings");
                dataRows.append(DataRow());
                dataRows[0].setValue("sequence", "ABC");
                dataRows.append(DataRow());
                dataRows[1].setValue("sequence", "DEF");
                QVERIFY(dbs.insert(dataRows, "astrings") == 2);
                query = db.exec(QString("SELECT id, sequence FROM astrings ORDER BY id"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QCOMPARE(query.value(1).toString(), QString("ABC"));
                QVERIFY(dataRows.at(0).id_.toInt() == 1);
                QVERIFY(query.next());
                QCOMPARE(query.value(0).toInt(), 2);
                QCOMPARE(query.value(1).toString(), QString("DEF"));
                QVERIFY(dataRows.at(1).id_.toInt() == 2);
            }
            catch (...) { QVERIFY(0); }

            // ---------------------------------------------
            // Test: empty data row, invalid field
            try
            {
                db.exec("DELETE FROM astrings");
                dataRows.clear();
                dataRows.append(DataRow());
                dbs.insert(dataRows, "astrings", QStringList() << "organism");
                QVERIFY(0);
            }
            catch (DatabaseError &)
            {
                QVERIFY(1);
                QSqlQuery query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QVERIFY(query.value(0).toInt() == 0);
            }
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::update()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key, sequence text)").lastError().type() == QSqlError::NoError);

        DbDataSource dbs;

        try
        {
            // ---------------------------------------------
            // Test: update without valid connection
            try
            {
                DataRow astring;
                dbs.update(astring, "astrings");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly
            try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: update invalid table
            try
            {
                DataRow astring(1);
                dbs.update(astring, "invalid");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }


            // ---------------------------------------------
            // Test: update calls that should not throw an exception
            try
            {
                // Test: update without a defined id
                DataRow astring(1);
                astring.setValue("sequence", "ABC");
                QVERIFY(dbs.update(astring, "astrings") == false);
                QSqlQuery query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 0);

                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (1, 'ABC')").lastError().type() == QSqlError::NoError);

                // Test: attempt to save but only update the sequence field
                astring.setValue("sequence", "DEF");
                QVERIFY(dbs.update(astring, "astrings", QStringList() << "sequence"));
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 1");
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QCOMPARE(query.value(1).toString(), QString("DEF"));

                // Test: update the id
                QVERIFY(astring.id_ == 1);
                astring.setValue("id", 3);
                astring.setValue("sequence", "GHI");
                QVERIFY(dbs.update(astring, "astrings", QStringList() << "id"));
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 1");
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(!query.next());
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 3");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 3);
                QCOMPARE(query.value(1).toString(), QString("DEF"));
                QCOMPARE(astring.id_.toInt(), 3);

                // Test: update all default fields
                astring.setValue("id", 6);
                astring.setValue("sequence", "GHI");
                QVERIFY(dbs.update(astring, "astrings"));
                QVERIFY(astring.id_ == 6);
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 6");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 6);
                QCOMPARE(query.value(1).toString(), QString("GHI"));

                // Test: conditions on update
                // Get a clean slate
                QVERIFY(db.exec("DELETE FROM astrings").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (30, 'GHI')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (40, 'JKL')").lastError().type() == QSqlError::NoError);

                // Conditions which don't match a row
                DataRow astring3(30);
                astring3.setValue("sequence", "XYZ");
                QVERIFY(dbs.update(astring3, "astrings", QStringList(), "id != 30") == false);
                query = db.exec(QString("SELECT id, sequence FROM astrings WHERE id = 30"));
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(1).toString(), QString("GHI"));

                // Conditions which do match the row
                QVERIFY(dbs.update(astring3, "astrings", QStringList(), "sequence = 'GHI'"));
                query = db.exec(QString("SELECT id, sequence FROM astrings WHERE id = 30"));
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(1).toString(), QString("XYZ"));
            }
            catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: update with no valid fields should return false
            try
            {
                DataRow astring(10);
                astring.setValue("organism", "E. coli");
                dbs.update(astring, "astrings", QStringList() << "organism");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }


            // ---------------------------------------------
            // Test: update the id to a null value
            try
            {
                QVERIFY(db.exec("DELETE FROM astrings").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
                DataRow astring(10);
                astring.setValue("id", QVariant());
                dbs.update(astring, "astrings");
                QVERIFY(0);
            }
            catch (DatabaseError &)
            {
                QVERIFY(1);
                QSqlQuery query = db.exec("SELECT id, sequence FROM astrings WHERE id = 10");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 10);
            }
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::updateMany()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());
        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key, sequence text)").lastError().type() == QSqlError::NoError);

        DbDataSource dbs;

        try
        {
            QList<DataRow> dataRows;

            // ---------------------------------------------
            // Test: update without valid connection
            try
            {
                dbs.update(dataRows, "astrings");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly
            try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: update invalid table
            try
            {
                dbs.update(dataRows, "invalid");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }


            // ---------------------------------------------
            // Test: update calls that should not throw an exception
            try
            {
                // Test: updating empty list should return 0
                QVERIFY(dbs.update(dataRows, "astrings") == 0);
                QVERIFY(dbs.update(dataRows, "astrings", QStringList()) == 0);
                QSqlQuery query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QVERIFY(query.value(0).toInt() == 0);

                // -------------------------------------------------
                // Single update tests repeated in list context

                // Test: update without a defined id
                dataRows.clear();
                dataRows.append(DataRow());
                dataRows[0].setValue("sequence", "ABC");
                QVERIFY(dbs.update(dataRows, "astrings") == 0);
                query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 0);

                // Insert some dummy data
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (1, 'ABC')").lastError().type() == QSqlError::NoError);

                // Test: attempt to save but only update the sequence field
                dataRows[0].id_ = 1;
                dataRows[0].setValue("sequence", "DEF");
                QVERIFY(dbs.update(dataRows, "astrings", QStringList() << "sequence") == 1);
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 1");
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QCOMPARE(query.value(1).toString(), QString("DEF"));

                // Test: update the id
                QVERIFY(dataRows.at(0).id_ == 1);
                dataRows[0].setValue("id", 3);
                dataRows[0].setValue("sequence", "GHI");
                QVERIFY(dbs.update(dataRows, "astrings", QStringList() << "id") == 1);
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 1");
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(!query.next());
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 3");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 3);
                QCOMPARE(query.value(1).toString(), QString("DEF"));
                QCOMPARE(dataRows.at(0).id_.toInt(), 3);

                // Test: update all default fields
                dataRows[0].setValue("id", 6);
                dataRows[0].setValue("sequence", "GHI");
                QVERIFY(dbs.update(dataRows, "astrings") == 1);
                QVERIFY(dataRows.at(0).id_ == 6);
                query = db.exec("SELECT id, sequence FROM astrings WHERE id = 6");
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toInt(), 6);
                QCOMPARE(query.value(1).toString(), QString("GHI"));

                // Test: conditions on update
                // Get a clean slate
                db.exec("DELETE FROM astrings");
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (30, 'GHI')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (40, 'JKL')").lastError().type() == QSqlError::NoError);

                // Conditions which don't match a row
                dataRows.clear();
                dataRows.append(DataRow(30));
                dataRows[0].setValue("sequence", "XYZ");
                QVERIFY(dbs.update(dataRows, "astrings", QStringList(), "id != 30") == 0);
                query = db.exec(QString("SELECT id, sequence FROM astrings WHERE id = 30"));
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(1).toString(), QString("GHI"));

                // Conditions which do match the row
                QVERIFY(dbs.update(dataRows, "astrings", QStringList(), "sequence = 'GHI'") == 1);
                query = db.exec(QString("SELECT id, sequence FROM astrings WHERE id = 30"));
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(1).toString(), QString("XYZ"));


                // ---------------------------------------------------------
                // Multiple entry list update
                dataRows.clear();
                db.exec("DELETE FROM astrings");
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);

                // Test: good one, then invalid one
                dataRows.append(DataRow(10));
                dataRows[0].setValue("sequence", "XYZ");
                dataRows.append(DataRow());
                dataRows[1].setValue("sequence", "TUV");
                QVERIFY(dbs.update(dataRows, "astrings") == 1);
                query = db.exec(QString("SELECT id, sequence FROM astrings WHERE id = 10"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(1).toString(), QString("XYZ"));
                query = db.exec(QString("SELECT id, sequence FROM astrings WHERE id = 20"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(1).toString(), QString("DEF"));


                // Test: invalid one, then good one
                db.exec("DELETE FROM astrings");
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
                dataRows.clear();
                dataRows.append(DataRow());
                dataRows[0].setValue("sequence", "TUV");
                dataRows.append(DataRow(10));
                dataRows[1].setValue("sequence", "XYZ");

                QVERIFY(dbs.update(dataRows, "astrings") == 0);
                query = db.exec(QString("SELECT sequence FROM astrings WHERE id = 10"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toString(), QString("ABC"));
                query = db.exec(QString("SELECT sequence FROM astrings WHERE id = 20"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toString(), QString("DEF"));


                // Test: two good ones
                db.exec("DELETE FROM astrings");
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
                dataRows.clear();
                dataRows.append(DataRow(10));
                dataRows[0].setValue("sequence", "XYZ");
                dataRows.append(DataRow(20));
                dataRows[1].setValue("sequence", "TUV");
                QVERIFY(dbs.update(dataRows, "astrings") == 2);
                query = db.exec(QString("SELECT sequence FROM astrings WHERE id = 10"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toString(), QString("XYZ"));
                query = db.exec(QString("SELECT sequence FROM astrings WHERE id = 20"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toString(), QString("TUV"));


                // Test: two datarows with different fields
                db.exec("DELETE FROM astrings");
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
                dataRows.clear();
                dataRows.append(DataRow(10));
                dataRows[0].setValue("sequence", "XYZ");
                dataRows.append(DataRow(20));
                dataRows[1].setValue("id", 2);
                QCOMPARE(dbs.update(dataRows, "astrings"), 1);
                query = db.exec(QString("SELECT sequence FROM astrings WHERE id = 10"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toString(), QString("XYZ"));
                query = db.exec(QString("SELECT sequence FROM astrings WHERE id = 20"));
                QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
                QCOMPARE(query.value(0).toString(), QString("DEF"));

            }
            catch (...) { QVERIFY(0); }


            // ---------------------------------------------
            // Test: update invalid column
            try
            {
                db.exec("DELETE FROM astrings");
                QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (1, 'ABC')").lastError().type() == QSqlError::NoError);

                dataRows.clear();
                dataRows.append(DataRow(1));
                dataRows[0].setValue("sequence", "XYZ");
                dbs.update(dataRows, "astrings", QStringList() << "sequence" << "name");
                QVERIFY(0);
            }
            catch (DatabaseError &)
            {
                QVERIFY(1);
                QSqlQuery query = db.exec("SELECT id, sequence FROM astrings WHERE id = 1");
                QVERIFY(query.lastError().type() == QSqlError::NoError);
                QVERIFY(query.next());
                QCOMPARE(query.value(0).toInt(), 1);
                QCOMPARE(query.value(1).toString(), QString("ABC"));
            }


            // ---------------------------------------------
            // Test: update the id to a null value
            try
            {
                dataRows.clear();
                dataRows.append(DataRow(1));
                dataRows[0].setValue("id", QVariant());
                dbs.update(dataRows, "astrings");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }
        }
        catch (...) { QVERIFY(0); }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::truncate()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key, sequence text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (10, 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (20, 'DEF')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (30, 'GHI')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO astrings (id, sequence) VALUES (40, 'JKL')").lastError().type() == QSqlError::NoError);

        DbDataSource dbs;

        // ---------------------------------------------
        // Test: truncate without valid connection
        try
        {
            dbs.truncate("astrings");
            QVERIFY(0);
        }
        catch (InvalidConnectionError &) { QVERIFY(1); }
        catch (...) { QVERIFY(0); }

        // Setup the connection properly
        try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }


        // ---------------------------------------------
        // Test: update invalid table
        try
        {
            dbs.truncate("invalid");
            QVERIFY(0);
        }
        catch (DatabaseError &) { QVERIFY(1); }

        // Test: truncate blank table
        try
        {
            dbs.truncate("");
            QVERIFY(0);
        }
        catch (DatabaseError &e) { QVERIFY(1); }

        // ---------------------------------------------
        // Test: update calls that should not throw an exception
        try
        {
            try
            {
                dbs.truncate("astrings");
                QSqlQuery query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.next());
                QVERIFY(query.value(0).toInt() == 0);
            }
            catch (...) { QVERIFY(0); }

            // Test: should still work even though astrings table is empty
            try
            {
                dbs.truncate("astrings");
                QSqlQuery query = db.exec("SELECT count(*) FROM astrings");
                QVERIFY(query.next());
                QVERIFY(query.value(0).toInt() == 0);
            }
            catch (...) { QVERIFY(0); }
        }
        catch(...)
        {
            // Should not get here, but rather should have caught all errors previously
            QVERIFY(0);
        }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::resetSqliteSequence()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        QVERIFY(db.exec("CREATE TABLE astrings (id integer not null primary key autoincrement, sequence text)").lastError().type() == QSqlError::NoError);

        DbDataSource dbs;

        try
        {
            // ---------------------------------------------
            // Test: reset sequence without valid connection
            try
            {
                dbs.resetSqliteSequence("astrings");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly
            try { dbs.setConnectionName("test_db"); } catch (...) { QVERIFY(0); }

            // ---------------------------------------------
            // Test: reset sqlite sequence on invalid table
            try
            {
                dbs.resetSqliteSequence("invalid");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }

            // Test: reset sqlite sequence on empty table
            dbs.resetSqliteSequence("astrings");

            QVERIFY(db.exec("INSERT INTO astrings (sequence) VALUES ('ABC')").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("INSERT INTO astrings (sequence) VALUES ('DEF')").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("INSERT INTO astrings (sequence) VALUES ('GHI')").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("INSERT INTO astrings (sequence) VALUES ('JKL')").lastError().type() == QSqlError::NoError);
            dbs.truncate("astrings");

            // Verify that there is no data left in astrings
            QSqlQuery query = db.exec("SELECT count(*) from astrings");
            QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
            QCOMPARE(query.value(0).toInt(), 0);

            // Insert another astring and its id should be 5
            QVERIFY(db.exec("INSERT INTO astrings (sequence) VALUES ('MNO')").lastError().type() == QSqlError::NoError);
            query = db.exec("SELECT id from astrings");
            QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
            QCOMPARE(query.value(0).toInt(), 5);

            // Reset the sequence
            dbs.truncate("astrings");
            dbs.resetSqliteSequence("astrings");

            // Insert new sequence
            QVERIFY(db.exec("INSERT INTO astrings (sequence) VALUES ('PQR')").lastError().type() == QSqlError::NoError);
            query = db.exec("SELECT id from astrings WHERE id = 1");
            QVERIFY(query.lastError().type() == QSqlError::NoError && query.next());
            QCOMPARE(query.value(0).toInt(), 1);
        }
        catch(...)
        {
            // Should not get here, but rather should have caught all errors previously
            QVERIFY(0);
        }
    }
    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::savepoint()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        DbDataSource dbs;

        try
        {
            // ---------------------------------------------
            // Test: savepoint without valid connection
            try
            {
                dbs.savePoint("step1");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly
            dbs.setConnectionName("test_db");

            // ---------------------------------------------
            // Test: savepoint with invalid name
            try
            {
                dbs.savePoint("");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }
            catch(...)              { QVERIFY(0); }

            // ---------------------------------------------
            // Test: savepoint with valid names
            dbs.savePoint("step1");
            dbs.savePoint("step2");
            dbs.savePoint("step1");
            dbs.savePoint("  step3");
            dbs.savePoint("step4  ");
            dbs.savePoint("  step5  ");

            // Release all the savepoints
            QVERIFY(db.exec("RELEASE SAVEPOINT step1").lastError().type() == QSqlError::NoError);

            // ---------------------------------------------
            // Test: savepoint should actually create a savepoint; verify by inserting records and then rolling back, table should be empty
            QVERIFY(db.exec("create table genomes (name text not null primary key)").lastError().type() == QSqlError::NoError);

            dbs.savePoint("before_insert");
            QVERIFY(db.exec("INSERT INTO genomes values ('E. coli')").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("INSERT INTO genomes values ('B. subtilis')").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("ROLLBACK TO SAVEPOINT before_insert").lastError().type() == QSqlError::NoError);
            QSqlQuery s_count(db);
            QVERIFY(s_count.prepare("SELECT count(*) FROM genomes"));
            QVERIFY(s_count.exec());
            QVERIFY(s_count.next());
            QCOMPARE(s_count.value(0).toInt(), 0);
        }
        catch(...)  { QVERIFY(0); }
    }

    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::rollbackToSavePoint()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        DbDataSource dbs;

        try
        {
            // ---------------------------------------------
            // Test: rollback without valid connection
            try
            {
                dbs.rollbackToSavePoint("step1");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly and a valid savepoint
            dbs.setConnectionName("test_db");

            // ---------------------------------------------
            // Test: rollback with invalid name
            try
            {
                dbs.rollbackToSavePoint("");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }
            catch(...)              { QVERIFY(0); }

            // ---------------------------------------------
            // Test: rollback to non-existent savepoint
            try
            {
                dbs.rollbackToSavePoint("missing");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }
            catch(...)              { QVERIFY(0); }

            // ---------------------------------------------
            // Test: rollback to valid savepoint
            QVERIFY(db.exec("SAVEPOINT step1").lastError().type() == QSqlError::NoError);
            dbs.rollbackToSavePoint("step1");

            QVERIFY(db.exec("SAVEPOINT step2").lastError().type() == QSqlError::NoError);
            dbs.rollbackToSavePoint("  step2");

            QVERIFY(db.exec("SAVEPOINT step3").lastError().type() == QSqlError::NoError);
            dbs.rollbackToSavePoint("step3  ");

            QVERIFY(db.exec("SAVEPOINT step4").lastError().type() == QSqlError::NoError);
            dbs.rollbackToSavePoint("  step4  ");

            // Test: multiple rollbacks to the same savepoint should not affect throw any errors
            dbs.rollbackToSavePoint("step4");
            dbs.rollbackToSavePoint("step4");

            // Test: rollback to previous rollback should invalidate intermediate savepoints
            dbs.rollbackToSavePoint("step2");
            try
            {
                dbs.rollbackToSavePoint("step3");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }

            // Release all the savepoints
            QVERIFY(db.exec("RELEASE SAVEPOINT step1").lastError().type() == QSqlError::NoError);

            // Test: rolling back to savepoint step1 should fail
            try
            {
                dbs.rollbackToSavePoint("step1");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }

            // ---------------------------------------------
            // Test: rollback should rollback SQL statements; verify by inserting records and then rolling back, table should be empty
            QVERIFY(db.exec("create table genomes (name text not null primary key)").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("SAVEPOINT before_insert").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("INSERT INTO genomes values ('E. coli')").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("INSERT INTO genomes values ('B. subtilis')").lastError().type() == QSqlError::NoError);
            dbs.rollbackToSavePoint("before_insert");
            QSqlQuery s_count(db);
            QVERIFY(s_count.prepare("SELECT count(*) FROM genomes"));
            QVERIFY(s_count.exec());
            QVERIFY(s_count.next());
            QCOMPARE(s_count.value(0).toInt(), 0);
        }
        catch(...)  { QVERIFY(0); }
    }

    QSqlDatabase::removeDatabase("test_db");
}

void TestDbDataSource::releaseSavePoint()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        DbDataSource dbs;

        try
        {
            // ---------------------------------------------
            // Test: release without valid connection
            try
            {
                dbs.releaseSavePoint("step1");
                QVERIFY(0);
            }
            catch (InvalidConnectionError &) { QVERIFY(1); }
            catch (...) { QVERIFY(0); }

            // Setup the connection properly and a valid savepoint
            dbs.setConnectionName("test_db");

            // ---------------------------------------------
            // Test: release with invalid name
            try
            {
                dbs.releaseSavePoint("");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }
            catch(...)              { QVERIFY(0); }

            // ---------------------------------------------
            // Test: release to non-existent savepoint
            try
            {
                dbs.releaseSavePoint("missing");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }
            catch(...)              { QVERIFY(0); }

            // ---------------------------------------------
            // Test: release to valid savepoint
            QVERIFY(db.exec("SAVEPOINT step1").lastError().type() == QSqlError::NoError);
            dbs.releaseSavePoint("step1");

            QVERIFY(db.exec("SAVEPOINT step2").lastError().type() == QSqlError::NoError);
            dbs.releaseSavePoint("  step2");

            QVERIFY(db.exec("SAVEPOINT step3").lastError().type() == QSqlError::NoError);
            dbs.releaseSavePoint("step3  ");

            QVERIFY(db.exec("SAVEPOINT step4").lastError().type() == QSqlError::NoError);
            dbs.releaseSavePoint("  step4  ");

            // Test: release with previously released savepoint should throw database error
            try
            {
                dbs.releaseSavePoint("step4");
                QVERIFY(0);
            }
            catch(DatabaseError &) { QVERIFY(1); }

            // Test: release to savepoint before other savepoints should remove these savepoints
            QVERIFY(db.exec("SAVEPOINT alpha").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("SAVEPOINT beta").lastError().type() == QSqlError::NoError);
            dbs.releaseSavePoint("alpha");
            try
            {
                dbs.releaseSavePoint("beta");
                QVERIFY(0);
            }
            catch (DatabaseError &) { QVERIFY(1); }

            // ---------------------------------------------
            // Test: release should retain all SQL commands; verify by inserting records and then releasing, table should not be empty
            QVERIFY(db.exec("create table genomes (name text not null primary key)").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("SAVEPOINT before_insert").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("INSERT INTO genomes values ('E. coli')").lastError().type() == QSqlError::NoError);
            QVERIFY(db.exec("INSERT INTO genomes values ('B. subtilis')").lastError().type() == QSqlError::NoError);
            dbs.releaseSavePoint("before_insert");
            QSqlQuery s_count(db);
            QVERIFY(s_count.prepare("SELECT count(*) FROM genomes"));
            QVERIFY(s_count.exec());
            QVERIFY(s_count.next());
            QCOMPARE(s_count.value(0).toInt(), 2);
        }
        catch(...)  { QVERIFY(0); }
    }

    QSqlDatabase::removeDatabase("test_db");
}

QTEST_MAIN(TestDbDataSource)
#include "TestDbDataSource.moc"
