/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "../AbstractDbSource.h"
#include "../MockDbSource.h"

class TestAbstractDbSource : public QObject
{
    Q_OBJECT

private slots:
    void testMockDbSource();
    void getPreparedQuery();
    void clearPreparedQueries();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAbstractDbSource::testMockDbSource()
{
    MockDbSource source;
    QVERIFY(source.database().isOpen());

    // Attempt to create a table
    QSqlQuery query = source.database().exec("CREATE TABLE tests (id integer not null primary key, name text not null)");
    QVERIFY(query.lastError().type() == QSqlError::NoError);
}

void TestAbstractDbSource::getPreparedQuery()
{
    MockDbSource source;

    // Create a dummy table
    QSqlQuery create = source.database().exec("CREATE TABLE seqs (id integer not null primary key, sequence text not null)");
    QVERIFY(create.lastError().type() == QSqlError::NoError);

    try
    {
        // Test: insert a sequence using a prepared query
        QSqlQuery query = source.getPreparedQuery("insertSeq",
                                                  "INSERT INTO seqs (sequence) VALUES (?)");

        query.bindValue(0, "ABCDEF");
        QVERIFY(query.exec());
        int firstId = query.lastInsertId().toInt();
        QVERIFY(firstId > 0);

        // Test: Check that the prepared query with the same name is returned
        query = source.getPreparedQuery("insertSeq", "");

        // Test: reuse the query
        query.bindValue(0, "GHIJKL");
        QVERIFY(query.exec());
        QVERIFY(query.lastInsertId().toInt() > firstId);

        // Test: attempt to fetch the records using another prepared query
        QSqlQuery select = source.getPreparedQuery("selectCount",
                                                   "SELECT count(*) from seqs");

        QVERIFY(select.exec());
        QVERIFY(select.next());
        QCOMPARE(select.value(0).toInt(), 2);
    }
    catch(...)
    {
        QVERIFY(0);
    }

    // Ensure that bad queries fail
    try
    {
        source.getPreparedQuery("bad query",
                                "INSERT NTO seqs (sequences) VALUES (34)");
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }
}

void TestAbstractDbSource::clearPreparedQueries()
{
    MockDbSource source;

    // Create a dummy table
    QSqlQuery create = source.database().exec("CREATE TABLE seqs (id integer not null)");
    QVERIFY(create.lastError().type() == QSqlError::NoError);

    try
    {
        // Test: insert a sequence using a prepared query
        QSqlQuery query = source.getPreparedQuery("testName",
                                                  "INSERT INTO seqs (id) VALUES (1)");
        query = source.getPreparedQuery("testName",
                                        "INSERT INTO seqs(id) VALUES (10)");

        QVERIFY(query.exec());

        // Test: check that one was inserted
        query = source.database().exec("SELECT * FROM seqs WHERE id = 1");
        QVERIFY(query.next());
        query = source.database().exec("SELECT * FROM seqs WHERE id = 10");
        QVERIFY(query.next() == false);

        source.database().exec("DELETE FROM seqs");
        query = source.database().exec("SELECT count(*) FROM seqs");
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), 0);

        // Test: clear the queries and retry the second query
        source.clearPreparedQueries();
        query = source.getPreparedQuery("testName",
                                        "INSERT INTO seqs(id) VALUES (10)");
        QVERIFY(query.exec());

        query = source.database().exec("SELECT * FROM seqs WHERE id = 1");
        QVERIFY(query.next() == false);
        query = source.database().exec("SELECT * FROM seqs WHERE id = 10");
        QVERIFY(query.next());
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

QTEST_APPLESS_MAIN(TestAbstractDbSource)
#include "TestAbstractDbSource.moc"
