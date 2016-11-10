/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QTest>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "DbRowCacheManager.h"

#include <QtDebug>

class TestDbRowCacheManager : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Test methods
    void constructor();        // Also checks source table
    void fetch();
    void clear();
    void setBaseSize();        // Also checks baseSize
    void remove();
    void setOverflowSize();    // Also checks overflowSize
    void capacity();
};

void TestDbRowCacheManager::constructor()
{
    DbRowCacheManager mgr(DbDataSource(), "invalid");

    // ----------------
    // Public interface
    // Test: baseSize and overflow size should be zero
    QCOMPARE(mgr.sourceTable(), QString("invalid"));
    QCOMPARE(mgr.baseSize(), 0);
    QCOMPARE(mgr.overflowSize(), 0);
    QCOMPARE(mgr.size(), 0);

    // -------------
    // Private state
    QCOMPARE(mgr.sourceTable_, QString("invalid"));
    QCOMPARE(mgr.baseSize_, 0);
    QVERIFY(mgr.cachedDataRows_.isEmpty());
    QVERIFY(mgr.cachedTime_.isEmpty());
    QCOMPARE(mgr.counter_, static_cast<unsigned long>(0));
    QCOMPARE(mgr.overflowSize_, 0);
}

void TestDbRowCacheManager::fetch()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE seqs (id integer not null primary key, name text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (10, 'NifL')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (20, 'FAD')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (30, 'PAS')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (40, 'RR')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (50, 'HK_CA')").lastError().type() == QSqlError::NoError);

        try
        {
            DbRowCacheManager mgr(DbDataSource("test_db"), "seqs");

            // Test: fetch non-existent record, should not add to cache obviously
            DataRow row = mgr.fetch(1);
            QVERIFY(row.isEmpty());
            QVERIFY(row.count() == 0);
            QCOMPARE(mgr.size(), 0);
            QVERIFY(mgr.cachedDataRows_.isEmpty());

            // Test: fetch existent record, but it should not be cached because the baseSize is still zero
            row = mgr.fetch(10);
            QCOMPARE(row.value("id").toInt(), 10);
            QCOMPARE(row.value("name").toString(), QString("NifL"));
            QCOMPARE(row.id_.toInt(), 10);
            QCOMPARE(mgr.size(), 0);
            QVERIFY(mgr.cachedDataRows_.isEmpty());
            QVERIFY(mgr.cachedTime_.isEmpty());
            QCOMPARE(mgr.counter_, static_cast<unsigned long>(0));

            // Test: add to base size and ensure it is cached
            mgr.setBaseSize(1);
            QCOMPARE(mgr.size(), 0);
            row = mgr.fetch(10);
            QCOMPARE(mgr.size(), 1);
            QCOMPARE(mgr.cachedDataRows_.size(), 1);
            QVERIFY(mgr.cachedDataRows_.contains(10));
            QCOMPARE(mgr.cachedTime_.size(), 1);
            QVERIFY(mgr.cachedTime_.contains(10));
            QCOMPARE(mgr.counter_, static_cast<unsigned long>(1));

            // Test: fetch another record and ensure that cache size remains at one
            row = mgr.fetch(20);
            QCOMPARE(row.value("id").toInt(), 20);
            QCOMPARE(row.value("name").toString(), QString("FAD"));
            QCOMPARE(row.id_.toInt(), 20);
            QCOMPARE(mgr.size(), 1);

            // Check its internal state
            QCOMPARE(mgr.cachedDataRows_.size(), 1);
            QVERIFY(mgr.cachedDataRows_.contains(20));
            QCOMPARE(mgr.cachedTime_.size(), 1);
            QVERIFY(mgr.cachedTime_.contains(20));
            QCOMPARE(mgr.counter_, static_cast<unsigned long>(2));

            // Test: fetch should kick out the oldest accessed sequence
            mgr.fetch(10);  // Now the only record in the cache is 10, because basesize is still 1

            // Allow up to four rows
            mgr.setBaseSize(4);
            mgr.fetch(20);
            mgr.fetch(30);
            mgr.fetch(40);
            mgr.fetch(50);

            QCOMPARE(mgr.size(), 4);
            QVERIFY(mgr.cachedDataRows_.contains(10) == false); // Should have kicked out 10 because it was the oldest
            QVERIFY(mgr.cachedDataRows_.contains(20));
            QVERIFY(mgr.cachedDataRows_.contains(30));
            QVERIFY(mgr.cachedDataRows_.contains(40));
            QVERIFY(mgr.cachedDataRows_.contains(50));

            QVERIFY(mgr.cachedTime_.contains(10) == false);
            QVERIFY(mgr.cachedTime_.contains(20));
            QVERIFY(mgr.cachedTime_.contains(30));
            QVERIFY(mgr.cachedTime_.contains(40));
            QVERIFY(mgr.cachedTime_.contains(50));

            // Check that the cache times are appropriately positioned
            QVERIFY(mgr.cachedTime_[50] > mgr.cachedTime_[40]);
            QVERIFY(mgr.cachedTime_[40] > mgr.cachedTime_[30]);
            QVERIFY(mgr.cachedTime_[30] > mgr.cachedTime_[20]);

            // Test: add 10 back in and 20 should be kicked out
            mgr.fetch(10);
            QCOMPARE(mgr.size(), 4);
            QVERIFY(mgr.cachedDataRows_.contains(20) == false); // Should have kicked out 20 because it was the oldest
            QVERIFY(mgr.cachedDataRows_.contains(30));
            QVERIFY(mgr.cachedDataRows_.contains(40));
            QVERIFY(mgr.cachedDataRows_.contains(50));
            QVERIFY(mgr.cachedDataRows_.contains(10));

            QVERIFY(mgr.cachedTime_.contains(20) == false);
            QVERIFY(mgr.cachedTime_.contains(30));
            QVERIFY(mgr.cachedTime_.contains(40));
            QVERIFY(mgr.cachedTime_.contains(50));
            QVERIFY(mgr.cachedTime_.contains(10));

            // Check that the cache times are appropriately positioned
            QVERIFY(mgr.cachedTime_[10] > mgr.cachedTime_[50]);
            QVERIFY(mgr.cachedTime_[50] > mgr.cachedTime_[40]);
            QVERIFY(mgr.cachedTime_[40] > mgr.cachedTime_[30]);

            // Test: at this point, 30 is the oldest, let's fetch it to make it the most recent, then
            //       insert 20, which should kick out 40
            mgr.fetch(30);
            mgr.fetch(20);
            mgr.fetch(20);
            mgr.fetch(20);
            QCOMPARE(mgr.size(), 4);
            QVERIFY(mgr.cachedDataRows_.contains(40) == false); // Should have kicked out 40 because it was the oldest
            QVERIFY(mgr.cachedDataRows_.contains(50));
            QVERIFY(mgr.cachedDataRows_.contains(10));
            QVERIFY(mgr.cachedDataRows_.contains(30));
            QVERIFY(mgr.cachedDataRows_.contains(20));

            QVERIFY(mgr.cachedTime_.contains(40) == false);
            QVERIFY(mgr.cachedTime_.contains(50));
            QVERIFY(mgr.cachedTime_.contains(10));
            QVERIFY(mgr.cachedTime_.contains(30));
            QVERIFY(mgr.cachedTime_.contains(20));

            // Check that the cache times are appropriately positioned
            QVERIFY(mgr.cachedTime_[20] > mgr.cachedTime_[30]);
            QVERIFY(mgr.cachedTime_[30] > mgr.cachedTime_[10]);
            QVERIFY(mgr.cachedTime_[10] > mgr.cachedTime_[50]);

            // Test: check overflow
            mgr.clear();
            mgr.setBaseSize(3);
            mgr.setOverflowSize(0);
            mgr.counter_ = ULONG_MAX - 2;
            mgr.fetch(10);
            QCOMPARE(mgr.cachedTime_[10], ULONG_MAX - 1);
            mgr.fetch(20);
            QCOMPARE(mgr.cachedTime_[20], ULONG_MAX);

            // Another fetch, should push us over the edge
            mgr.fetch(30);
            QCOMPARE(mgr.cachedTime_[10], static_cast<unsigned long>(1));
            QCOMPARE(mgr.cachedTime_[20], static_cast<unsigned long>(2));
            QCOMPARE(mgr.cachedTime_[30], static_cast<unsigned long>(3));
        }
        catch (...)
        {
            QVERIFY(0);
        }
    }

    QSqlDatabase::removeDatabase("test_db");
}

void TestDbRowCacheManager::clear()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE seqs (id integer not null primary key, name text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (10, 'NifL')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (20, 'FAD')").lastError().type() == QSqlError::NoError);

        try
        {
            DbRowCacheManager mgr(DbDataSource("test_db"), "seqs");
            mgr.setBaseSize(2);

            // Test: clear should remove the entries from the cache
            mgr.fetch(10);
            mgr.fetch(20);
            QCOMPARE(mgr.size(), 2);
            QCOMPARE(mgr.cachedDataRows_.size(), 2);
            QCOMPARE(mgr.cachedTime_.size(), 2);
            mgr.clear();
            QCOMPARE(mgr.size(), 0);

            QVERIFY(mgr.cachedDataRows_.isEmpty());
            QVERIFY(mgr.cachedTime_.isEmpty());
            QCOMPARE(mgr.counter_, static_cast<unsigned long>(0));
        }
        catch (...)
        {
            QVERIFY(0);
        }
    }

    QSqlDatabase::removeDatabase("test_db");
}

void TestDbRowCacheManager::setBaseSize()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE seqs (id integer not null primary key, name text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (10, 'NifL')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (20, 'FAD')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (30, 'PAS')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (40, 'RR')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (50, 'HK_CA')").lastError().type() == QSqlError::NoError);

        try
        {
            DbRowCacheManager mgr(DbDataSource("test_db"), "seqs");
            mgr.setBaseSize(2);
            QCOMPARE(mgr.baseSize(), 2);

            // Test: size should be limited by the base size no matter how many are called
            mgr.fetch(10);
            mgr.fetch(20);
            QCOMPARE(mgr.size(), 2);

            mgr.fetch(30);
            mgr.fetch(40);
            mgr.fetch(50);
            QCOMPARE(mgr.size(), 2);

            // Test: expand the base size should not change the size
            mgr.setBaseSize(5);
            QCOMPARE(mgr.size(), 2);

            // Should have 40 and 50 in the cache
            QVERIFY(mgr.cachedDataRows_.contains(40));
            QVERIFY(mgr.cachedDataRows_.contains(50));
            QVERIFY(mgr.cachedTime_.contains(40));
            QVERIFY(mgr.cachedTime_.contains(50));
            QVERIFY(mgr.cachedTime_[50] >= mgr.cachedTime_[40]);

            // Test: but reducing the base size should and it should remove the oldest accessed first
            mgr.setBaseSize(1);
            QCOMPARE(mgr.size(), 1);
            QCOMPARE(mgr.baseSize_, 1);
            QVERIFY(mgr.cachedDataRows_.contains(50));
            QVERIFY(mgr.cachedTime_.contains(50));

            // Test: same thing but with more sequences
            mgr.setBaseSize(5);
            mgr.fetch(10);
            mgr.fetch(20);
            mgr.fetch(30);
            mgr.fetch(40);
            mgr.fetch(50);

            QCOMPARE(mgr.cachedDataRows_.size(), 5);
            QCOMPARE(mgr.cachedTime_.size(), 5);

            mgr.setBaseSize(4);
            QVERIFY(mgr.cachedDataRows_.contains(10) == false);
            QVERIFY(mgr.cachedTime_.contains(10) == false);
            mgr.setBaseSize(2);
            QCOMPARE(mgr.cachedDataRows_.size(), 2);
            QVERIFY(mgr.cachedDataRows_.contains(40));
            QVERIFY(mgr.cachedDataRows_.contains(50));
            QVERIFY(mgr.cachedTime_.contains(40));
            QVERIFY(mgr.cachedTime_.contains(50));

            // Test: set the base size to 0
            mgr.setBaseSize(0);
            QVERIFY(mgr.cachedDataRows_.isEmpty());
            QVERIFY(mgr.cachedTime_.isEmpty());

            mgr.fetch(10);
            QVERIFY(mgr.cachedDataRows_.isEmpty());
            QVERIFY(mgr.cachedTime_.isEmpty());
        }
        catch (...)
        {
            QVERIFY(0);
        }
    }

    QSqlDatabase::removeDatabase("test_db");
}

void TestDbRowCacheManager::remove()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE seqs (id integer not null primary key, name text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (10, 'NifL')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (20, 'FAD')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (30, 'PAS')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (40, 'RR')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (50, 'HK_CA')").lastError().type() == QSqlError::NoError);

        try
        {
            DbRowCacheManager mgr(DbDataSource("test_db"), "seqs");
            mgr.setBaseSize(2);
            QCOMPARE(mgr.baseSize(), 2);

            // Add some DataRows to the internal cache
            mgr.fetch(10);
            mgr.fetch(20);
            QCOMPARE(mgr.size(), 2);
            QCOMPARE(mgr.cachedDataRows_.size(), 2);
            QCOMPARE(mgr.cachedTime_.size(), 2);
            QVERIFY(mgr.cachedDataRows_.contains(10));
            QVERIFY(mgr.cachedDataRows_.contains(20));
            QVERIFY(mgr.cachedTime_.contains(10));
            QVERIFY(mgr.cachedTime_.contains(20));

            // Test: remove id that does not exist should do nothing
            mgr.remove(-100);
            mgr.remove(-1);
            mgr.remove(0);
            mgr.remove(1);
            mgr.remove(25);
            QCOMPARE(mgr.size(), 2);
            QCOMPARE(mgr.cachedDataRows_.size(), 2);
            QCOMPARE(mgr.cachedTime_.size(), 2);
            QVERIFY(mgr.cachedDataRows_.contains(10));
            QVERIFY(mgr.cachedDataRows_.contains(20));
            QVERIFY(mgr.cachedTime_.contains(10));
            QVERIFY(mgr.cachedTime_.contains(20));

            // Test: remove should remove from the internal cache
            mgr.remove(20);
            QCOMPARE(mgr.size(), 1);
            QCOMPARE(mgr.cachedDataRows_.size(), 1);
            QCOMPARE(mgr.cachedTime_.size(), 1);
            QVERIFY(mgr.cachedDataRows_.contains(10));
            QVERIFY(mgr.cachedDataRows_.contains(20) == false);
            QVERIFY(mgr.cachedTime_.contains(10));
            QVERIFY(mgr.cachedTime_.contains(20) == false);

            mgr.remove(10);
            QVERIFY(mgr.size() == 0);
            QVERIFY(mgr.cachedDataRows_.isEmpty());
            QVERIFY(mgr.cachedTime_.isEmpty());
        }
        catch (...)
        {
            QVERIFY(0);
        }
    }

    QSqlDatabase::removeDatabase("test_db");
}

void TestDbRowCacheManager::setOverflowSize()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Create some dummy table data
        QVERIFY(db.exec("CREATE TABLE seqs (id integer not null primary key, name text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (10, 'NifL')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (20, 'FAD')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (30, 'PAS')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (40, 'RR')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO seqs (id, name) VALUES (50, 'HK_CA')").lastError().type() == QSqlError::NoError);

        try
        {
            DbRowCacheManager mgr(DbDataSource("test_db"), "seqs");

            // Test: basesize of zero and overflow size of 2
            mgr.setOverflowSize(2);
            QCOMPARE(mgr.overflowSize(), 2);
            QCOMPARE(mgr.overflowSize_, 2);

            mgr.fetch(10);
            QVERIFY(mgr.fetch(20).isEmpty() == false);
            QCOMPARE(mgr.size(), 2);

            // Test: overflow should not exceed if we fetch an existing sequence
            mgr.fetch(20);
            QCOMPARE(mgr.size(), 2);

            // Test: overflow should not exceed if we fetch a non-existent sequence
            mgr.fetch(-340);
            QCOMPARE(mgr.size(), 2);

            // Test: once overflow has exceeded should reduce to basesize - 1 (or zero minimum) and
            //       add currently requested sequence
            mgr.fetch(30);
            QCOMPARE(mgr.size(), 1);
            QVERIFY(mgr.cachedDataRows_.contains(30));
            QVERIFY(mgr.cachedTime_.contains(30));

            // --------------------------------------
            // Test: effect of overflowsize on reducing the cached rows
            mgr.clear();
            mgr.setOverflowSize(3);
            mgr.fetch(10);
            mgr.fetch(20);
            mgr.fetch(30);

            QCOMPARE(mgr.size(), 3);
            mgr.setOverflowSize(2);
            QCOMPARE(mgr.size(), 2);
            QVERIFY(mgr.cachedDataRows_.contains(10) == false);
            QVERIFY(mgr.cachedTime_.contains(10) == false);

            // ------------------------------------------------
            // Test: positive base size and positive overflow size
            mgr.clear();
            mgr.setBaseSize(2);
            mgr.setOverflowSize(2);
            QVERIFY(mgr.size() == 0);

            mgr.fetch(10);
            mgr.fetch(20);
            mgr.fetch(30);
            mgr.fetch(40);

            QCOMPARE(mgr.size(), 4);
            QCOMPARE(mgr.cachedDataRows_.size(), 4);
            QCOMPARE(mgr.cachedTime_.size(), 4);

            // Fetching a fifth sequence should exceed the overflow limit and cause the
            // cache to be reduced to base size
            mgr.fetch(50);
            QCOMPARE(mgr.baseSize(), 2);
            QCOMPARE(mgr.size(), mgr.baseSize());
            QVERIFY(mgr.cachedDataRows_.contains(40));
            QVERIFY(mgr.cachedTime_.contains(40));
            QVERIFY(mgr.cachedDataRows_.contains(50));
            QVERIFY(mgr.cachedTime_.contains(50));

            mgr.fetch(10);
            mgr.fetch(20);
            QCOMPARE(mgr.size(), 4);
            QVERIFY(mgr.cachedDataRows_.contains(40));
            QVERIFY(mgr.cachedDataRows_.contains(50));
            QVERIFY(mgr.cachedDataRows_.contains(10));
            QVERIFY(mgr.cachedDataRows_.contains(20));
            QVERIFY(mgr.cachedTime_.contains(40));
            QVERIFY(mgr.cachedTime_.contains(50));
            QVERIFY(mgr.cachedTime_.contains(10));
            QVERIFY(mgr.cachedTime_.contains(20));

            // ------------------------------------------------
            // Test: basesize of zero, overflow of 2, then set basesize to 2, should permit up to four sequences in cache before
            //       reducing to basesize
            mgr.clear();
            mgr.setBaseSize(0);
            mgr.setOverflowSize(2);
            mgr.fetch(10);
            mgr.fetch(20);

            QCOMPARE(mgr.size(), 2);
            QVERIFY(mgr.cachedDataRows_.contains(10));
            QVERIFY(mgr.cachedDataRows_.contains(20));
            QVERIFY(mgr.cachedTime_.contains(10));
            QVERIFY(mgr.cachedTime_.contains(20));

            mgr.setBaseSize(2);
            mgr.fetch(30);
            mgr.fetch(40);
            QCOMPARE(mgr.size(), 4);
            QVERIFY(mgr.cachedDataRows_.contains(10));
            QVERIFY(mgr.cachedDataRows_.contains(20));
            QVERIFY(mgr.cachedDataRows_.contains(30));
            QVERIFY(mgr.cachedDataRows_.contains(40));
            QVERIFY(mgr.cachedTime_.contains(10));
            QVERIFY(mgr.cachedTime_.contains(20));
            QVERIFY(mgr.cachedTime_.contains(30));
            QVERIFY(mgr.cachedTime_.contains(40));

            mgr.setBaseSize(2);
            QCOMPARE(mgr.baseSize(), 2);
        }
        catch (...)
        {
            QVERIFY(0);
        }
    }

    QSqlDatabase::removeDatabase("test_db");
}

void TestDbRowCacheManager::capacity()
{
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        DbRowCacheManager mgr(DbDataSource("test_db"), "invalid");

        QCOMPARE(mgr.capacity(), 0);
        mgr.setBaseSize(5);
        QCOMPARE(mgr.capacity(), 5);
        mgr.setBaseSize(10);
        QCOMPARE(mgr.capacity(), 10);

        mgr.setOverflowSize(5);
        QCOMPARE(mgr.capacity(), 15);
    }

    QSqlDatabase::removeDatabase("test_db");
}

QTEST_MAIN(TestDbRowCacheManager)
#include "TestDbRowCacheManager.moc"
