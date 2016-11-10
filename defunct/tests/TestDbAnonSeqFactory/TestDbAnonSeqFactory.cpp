/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>

#include <typeinfo>

#include "SynchronousAdocDataSource.h"
#include "DbAnonSeqFactory.h"

#include "AminoString.h"
#include "DnaString.h"

#include <QDebug>

class TestDbAnonSeqFactory : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setSourceTable();  // Also tests tableName
    void setDataSource();
    void ready();
    void fetchById();
    void fetchByBioString();
    void add();
    void removeById();
    void removeByBioString();
    void size();
};

void TestDbAnonSeqFactory::setSourceTable()
{
    DbAnonSeqFactory f;

    // Test: default table name should be empty
    QVERIFY(f.sourceTable().isEmpty());

    f.setSourceTable("aseqs");
    QCOMPARE(f.sourceTable(), QString("aseqs"));

    f.setSourceTable("  dseqs");
    QCOMPARE(f.sourceTable(), QString("dseqs"));

    f.setSourceTable("  rseqs  \n");
    QCOMPARE(f.sourceTable(), QString("rseqs"));

    f.setSourceTable("");
    QVERIFY(f.sourceTable().isEmpty());

    f.setSourceTable("  ");
    QVERIFY(f.sourceTable().isEmpty());

    f.setSourceTable("astrings");
    QCOMPARE(f.ready(), false);
}

void TestDbAnonSeqFactory::setDataSource()
{
    DbAnonSeqFactory f(QCryptographicHash::Sha1);

    QVERIFY(f.adocDataSource() == 0);

    SynchronousAdocDataSource *ds = new SynchronousAdocDataSource(this);
    f.setAdocDataSource(ds);

    QVERIFY(f.adocDataSource() == ds);

    f.setAdocDataSource(0);
    QVERIFY(f.adocDataSource() == 0);

    QVERIFY(f.ready() == false);
}

void TestDbAnonSeqFactory::ready()
{
    DbAnonSeqFactory f;
    QVERIFY(f.ready() == false);

    SynchronousAdocDataSource *ds = new SynchronousAdocDataSource(this);

    f.setSourceTable("astrings");
    f.setAdocDataSource(ds);

    QVERIFY(f.ready());

    f.setSourceTable(QString());
    QVERIFY(f.ready() == false);

    f.setSourceTable("dstrings");
    QVERIFY(f.ready());

    f.setAdocDataSource(0);
    QVERIFY(f.ready() == false);

    f.setAdocDataSource(ds);
    QVERIFY(f.ready());
}

void TestDbAnonSeqFactory::fetchById()
{
    DbAnonSeqFactory f(QCryptographicHash::Sha1);

    // Test: uninitialized factory
    QVERIFY(f.fetch("").isValid() == false);
    QVERIFY(f.fetch("ABC").isValid() == false);

    QString testDbFile = "TestDbAnonSeqFactory.db";
    {
        if (QFile::exists(testDbFile))
            QVERIFY(QFile::remove(testDbFile));

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(testDbFile);
        db.open();

        QVERIFY(db.exec("CREATE TABLE aseqs (id integer primary key autoincrement, digest text, sequence text)").lastError().type() == QSqlError::NoError);

        // Insert a few sequences for testing purposes; must use SHA1 hashes
        QVERIFY(db.exec("INSERT INTO aseqs values (10, 'PAG9uybzWLqyfyZ5JKosmgP8/bg=', 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO aseqs values (20, '9b+DWcnbyv6F4ii2rh8EVFcYg+E=', 'MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI')").lastError().type() == QSqlError::NoError);

        QVERIFY(db.exec("CREATE TABLE dseqs (id integer primary key autoincrement, digest text, sequence text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO dseqs values (50, '4xb4QOWFhDsKbhLyg1Z/DqS091A=', 'ATG')").lastError().type() == QSqlError::NoError);
        db.close();

        SynchronousAdocDataSource *ds = new SynchronousAdocDataSource(this);
        f.setSourceTable("aseqs");
        f.setAdocDataSource(ds);
        QVERIFY(f.ready());

        // Test skipped because it throws assert: fetching by valid id should not work until AdocDataSource has been opened
//        AnonSeq a = f.fetch(10);
//        QVERIFY(a.isValid() == false);

        ds->open(testDbFile);
        QVERIFY(ds->isOpen());

        // Test: fetch for sequence that does not exist
        QVERIFY(f.fetch(1).isValid() == false);
        QVERIFY(f.fetch(0).isValid() == false);
        QVERIFY(f.fetch(-1).isValid() == false);
        QVERIFY(f.fetch(30).isValid() == false);

        // Test: no prototype set, fetch actual record
        AnonSeq a = f.fetch(10);
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 10);
        QVERIFY(a.bioString() == "ABC");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("BioString"));

        a = f.fetch(20);
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 20);
        QVERIFY(a.bioString() == "MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("BioString"));

        // Test: prototype is set, make sure that fetched AnonSeq produces expected derived bioString prototype
        f.setBioStringPrototype(new AminoString());
        a = f.fetch(10);
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 10);
        QVERIFY(a.bioString() == "ABC");
        QVERIFY2(QString(typeid(a.bioString()).name()).contains("AminoString"), QString("%1").arg(typeid(a.bioString()).name()).toAscii());

        f.setSourceTable("dseqs");
        QVERIFY(f.fetch(10).isValid() == false);

        f.setBioStringPrototype(new DnaString());
        a = f.fetch(50);
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 50);
        QVERIFY(a.bioString() == "ATG");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("DnaString"));

        ds->close();
    }

    QSqlDatabase::removeDatabase("test_db");

    QFile::remove(testDbFile);
}

void TestDbAnonSeqFactory::fetchByBioString()
{
    DbAnonSeqFactory f(QCryptographicHash::Sha1);

    // Test: uninitialized factory
    QVERIFY(f.fetch("").isValid() == false);
    QVERIFY(f.fetch("ABC").isValid() == false);

    QString testDbFile = "TestDbAnonSeqFactory.db";
    {
        if (QFile::exists(testDbFile))
            QVERIFY(QFile::remove(testDbFile));

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(testDbFile);
        db.open();

        QVERIFY2(db.exec("CREATE TABLE aseqs (id integer primary key autoincrement, digest text, sequence text)").lastError().type() == QSqlError::NoError, db.lastError().text().toAscii());

        // Insert a few sequences for testing purposes; must use SHA1 hashes
        QVERIFY(db.exec("INSERT INTO aseqs values (10, 'PAG9uybzWLqyfyZ5JKosmgP8/bg=', 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO aseqs values (20, '9b+DWcnbyv6F4ii2rh8EVFcYg+E=', 'MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI')").lastError().type() == QSqlError::NoError);

        SynchronousAdocDataSource *ds = new SynchronousAdocDataSource(this);
        f.setSourceTable("aseqs");
        f.setAdocDataSource(ds);
        QVERIFY(f.ready());

        ds->open(testDbFile);
        QVERIFY(ds->isOpen());

        // Test skipped because it throws assert: before adoc data source is opened, should return false
//        AnonSeq a = f.fetch("ABC");
//        QVERIFY(a.isValid() == false);

        // No prototype is set so all AnonSeq BioString's should be of the BioString type

        // Test: fetch for empty biostrings
        QVERIFY(f.fetch(QString()).isValid() == false);
        QVERIFY(f.fetch("---").isValid() == false);
        QVERIFY(f.fetch("A is happy").isValid() == false);

        // Test: biostring that is perfect match
        AnonSeq a = f.fetch("ABC");
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 10);
        QVERIFY(a.bioString() == "ABC");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("BioString"));

        // Test: biostring that is perfect match after reduction and normalization
        a = f.fetch("a---b---c");
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 10);
        QVERIFY(a.bioString() == "ABC");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("BioString"));

        // Test: another perfect match
        a = f.fetch("MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI");
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 20);
        QVERIFY(a.bioString() == "MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("BioString"));

        // Test: perfect match after reduction
        a = f.fetch("MKTI LVTGGA GYIGS     HTVVE-----   LLAA.....    E Y N    \n\nLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLG\r\tWKI");
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 20);
        QVERIFY(a.bioString() == "MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("BioString"));

        // Test: same previous four tests except with a prototype set
        f.setBioStringPrototype(new AminoString());

        // Test: biostring that is perfect match
        a = f.fetch("ABC");
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 10);
        QVERIFY(a.bioString() == "ABC");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("AminoString"));

        // Test: biostring that is perfect match after reduction and normalization
        a = f.fetch("a---b---c");
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 10);
        QVERIFY(a.bioString() == "ABC");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("AminoString"));

        // Test: another perfect match
        a = f.fetch("MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI");
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 20);
        QVERIFY(a.bioString() == "MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("AminoString"));

        // Test: perfect match after reduction
        a = f.fetch("MKTI LVTGGA GYIGS     HTVVE-----   LLAA.....    E Y N    \n\nLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLG\r\tWKI");
        QVERIFY(a.isValid());
        QVERIFY(a.id() == 20);
        QVERIFY(a.bioString() == "MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI");
        QVERIFY(QString(typeid(a.bioString()).name()).contains("AminoString"));

        // Change the data source and try with different table
        QVERIFY(db.exec("CREATE TABLE dseqs (id integer primary key autoincrement, digest text, sequence text)").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO dseqs values (50, '4xb4QOWFhDsKbhLyg1Z/DqS091A=', 'ATG')").lastError().type() == QSqlError::NoError);
        f.setSourceTable("dseqs");
        f.setBioStringPrototype(new DnaString());
        QVERIFY(f.fetch("ABC").isValid() == false);
        AnonSeq d = f.fetch("ATG");
        QVERIFY(d.isValid());
        QVERIFY(d.id() == 50);
        QVERIFY(d.bioString() == "ATG");
        QVERIFY(QString(typeid(d.bioString()).name()).contains("DnaString"));

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");

    QFile::remove(testDbFile);
}

void TestDbAnonSeqFactory::add()
{
    DbAnonSeqFactory f(QCryptographicHash::Sha1);

    // Test: attempt to add BioString before factory is initialized with a proper data source
    QVERIFY(f.add(BioString("")).isValid() == false);
    QVERIFY(f.add(BioString("  ")).isValid() == false);
    QVERIFY(f.add(BioString("ABC")).isValid() == false);

    QString testDbFile = "TestDbAnonSeqFactory.db";
    {
        if (QFile::exists(testDbFile))
            QVERIFY(QFile::remove(testDbFile));

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(testDbFile);
        db.open();

        QVERIFY(db.exec("CREATE TABLE aseqs (id integer primary key autoincrement, digest text, sequence text)").lastError().type() == QSqlError::NoError);

        SynchronousAdocDataSource *ds = new SynchronousAdocDataSource(this);
        f.setSourceTable("aseqs");
        f.setAdocDataSource(ds);
        QVERIFY(f.ready());

        // Test skipped because it throws assert: adding before data source is opened should fail
//        QVERIFY(f.add(BioString("ABC")).isValid() == false);

        ds->open(testDbFile);
        QVERIFY(ds->isOpen());

        // Test: add empty biostring
        QVERIFY(f.add(BioString()).isValid() == false);

        // Test: add biostring that when reduced is empty
        QVERIFY(f.add(BioString("   ")).isValid() == false);
        QVERIFY(f.add(BioString(" -- .. -- ")).isValid() == false);

        // Test: non-empty biostring
        AnonSeq aseq = f.add(BioString("ABC"));
        QVERIFY(aseq.isValid());
        QVERIFY(aseq.bioString() == "ABC");

        // Check that the record was added to the database
        QSqlQuery q = db.exec("SELECT id, sequence FROM aseqs");
        QVERIFY(q.next());
        QVERIFY(q.value(0).toInt() == aseq.id());
        QVERIFY(q.value(1).toString() == aseq.bioString().sequence());
        q.finish();     // Necessary to relinquish lock on database

        // Test: adding a record with the same sequence, should get an anonseq with the same id
        AnonSeq aseq2 = f.add(BioString("--AB--C"));
        QVERIFY(aseq2.isValid());
        QVERIFY(aseq2.bioString() == "ABC");
        QVERIFY(aseq2.id() == aseq.id());

        // Test: derived biostring type
        AnonSeq aseq3 = f.add(AminoString("---PASSED---"));
        QVERIFY(aseq3.isValid());
        QVERIFY(aseq3.bioString() == "PASSED");
        QVERIFY(aseq3.id() != aseq2.id());

        // Check that the record was added to the database
        q = db.exec("SELECT id, sequence FROM aseqs ORDER BY id desc limit 1");
        QVERIFY(q.next());
        QVERIFY(q.value(0).toInt() == aseq3.id());
        QVERIFY(q.value(1).toString() == aseq3.bioString().sequence());
        q.finish();

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");

    QFile::remove(testDbFile);
}

void TestDbAnonSeqFactory::removeById()
{
    DbAnonSeqFactory f(QCryptographicHash::Sha1);

    // Test: uninitialized factory
    QVERIFY(f.remove(-1) == 0);
    QVERIFY(f.remove(0) == 0);
    QVERIFY(f.remove(1) == 0);
    QVERIFY(f.remove(30) == 0);

    QString testDbFile = "TestDbAnonSeqFactory.db";
    {
        if (QFile::exists(testDbFile))
            QVERIFY(QFile::remove(testDbFile));

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(testDbFile);
        db.open();

        QVERIFY2(db.exec("CREATE TABLE aseqs (id integer primary key autoincrement, digest text, sequence text)").lastError().type() == QSqlError::NoError, db.lastError().text().toAscii());

        // Insert a few sequences for testing purposes; must use SHA1 hashes
        QVERIFY(db.exec("INSERT INTO aseqs values (10, 'PAG9uybzWLqyfyZ5JKosmgP8/bg=', 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO aseqs values (20, '9b+DWcnbyv6F4ii2rh8EVFcYg+E=', 'MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI')").lastError().type() == QSqlError::NoError);

        SynchronousAdocDataSource *ds = new SynchronousAdocDataSource(this);
        f.setSourceTable("aseqs");
        f.setAdocDataSource(ds);
        ds->open(testDbFile);
        QVERIFY(ds->isOpen());
        QVERIFY(f.ready());

        // Test: remove with id that does not exist
        QVERIFY(f.remove(-1) == 0);
        QVERIFY(f.remove(0) == 0);
        QVERIFY(f.remove(1) == 0);
        QVERIFY(f.remove(30) == 0);

        // Test: remove with valid id
        QVERIFY(f.remove(10) == 1);

        // Check that it is gone in the database
        QSqlQuery q = db.exec("SELECT id, sequence FROM aseqs WHERE id = 10");
        QVERIFY(q.next() == false);

        // Test: remove with another valid id
        QVERIFY(f.remove(20));
        q = db.exec("SELECT id, sequence FROM aseqs WHERE id = 20");
        QVERIFY(q.next() == false);

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");

    QFile::remove(testDbFile);
}

void TestDbAnonSeqFactory::removeByBioString()
{
    DbAnonSeqFactory f(QCryptographicHash::Sha1);

    // Test: uninitialized factory
    QVERIFY(f.remove("") == 0);
    QVERIFY(f.remove("ABC") == 0);
    QVERIFY(f.remove("--ab--c-  ") == 0);

    QString testDbFile = "TestDbAnonSeqFactory.db";
    {
        if (QFile::exists(testDbFile))
            QVERIFY(QFile::remove(testDbFile));

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(testDbFile);
        db.open();

        QVERIFY2(db.exec("CREATE TABLE aseqs (id integer primary key autoincrement, digest text, sequence text)").lastError().type() == QSqlError::NoError, db.lastError().text().toAscii());

        SynchronousAdocDataSource *ds = new SynchronousAdocDataSource(this);
        f.setSourceTable("aseqs");
        f.setAdocDataSource(ds);
        QVERIFY(f.ready());
        ds->open(testDbFile);
        QVERIFY(ds->isOpen());

        // Insert a few sequences for testing purposes; must use SHA1 hashes
        QVERIFY(db.exec("INSERT INTO aseqs values (10, 'PAG9uybzWLqyfyZ5JKosmgP8/bg=', 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO aseqs values (20, '9b+DWcnbyv6F4ii2rh8EVFcYg+E=', 'MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI')").lastError().type() == QSqlError::NoError);

        // Test: remove with BioString that does not exist
        QVERIFY(f.remove("") == 0);
        QVERIFY(f.remove("asdf") == 0);
        QVERIFY(f.remove("---DOES NOT exist ---") == 0);

        // Test: remove with valid BioString
        QVERIFY(f.remove("ABC") == 1);

        // Check that it is gone in the database
        QSqlQuery q = db.exec("SELECT id, sequence FROM aseqs WHERE id = 10");
        QVERIFY(q.next() == false);

        // Test: remove with another valid id, but also using a non-reduced form
        QVERIFY(f.remove("MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSN....SSR------VALERVEKISGRSFHFYQADIR DIYSLN\rQIFTDH\nSID\nAVIHF AGLKAVSE PSYL GWKI") == 1);
        q = db.exec("SELECT id, sequence FROM aseqs WHERE id = 20");
        QVERIFY(q.next() == false);

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");

    QFile::remove(testDbFile);
}

void TestDbAnonSeqFactory::size()
{
    DbAnonSeqFactory f(QCryptographicHash::Sha1);

    // Test: uninitialized factory
    QVERIFY(f.size() == 0);

    QString testDbFile = "TestDbAnonSeqFactory.db";
    {
        if (QFile::exists(testDbFile))
            QVERIFY(QFile::remove(testDbFile));

        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(testDbFile);
        db.open();

        // Insert a few sequences for testing purposes; must use SHA1 hashes
        QVERIFY2(db.exec("CREATE TABLE aseqs (id integer primary key autoincrement, digest text, sequence text)").lastError().type() == QSqlError::NoError, db.lastError().text().toAscii());

        SynchronousAdocDataSource *ds = new SynchronousAdocDataSource(this);
        f.setSourceTable("aseqs");
        f.setAdocDataSource(ds);
        ds->open(testDbFile);
        QVERIFY(ds->isOpen());
        QVERIFY(f.ready());

        QVERIFY(f.size() == 0);

        QVERIFY(db.exec("INSERT INTO aseqs values (10, 'PAG9uybzWLqyfyZ5JKosmgP8/bg=', 'ABC')").lastError().type() == QSqlError::NoError);
        QVERIFY(f.size() == 1);

        QVERIFY(db.exec("INSERT INTO aseqs values (20, '9b+DWcnbyv6F4ii2rh8EVFcYg+E=', 'MKTILVTGGAGYIGSHTVVELLAAEYNLVIVDNLSNSSRVALERVEKISGRSFHFYQADIRDIYSLNQIFTDHSIDAVIHFAGLKAVSEPSYLGWKI')").lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec("INSERT INTO aseqs values (30, 'Mzr/KYas2FUX6rV2RNFSjeHj3O4=', 'MKTILV')").lastError().type() == QSqlError::NoError);
        QVERIFY(f.size() == 3);

        QVERIFY(db.exec("DELETE FROM aseqs WHERE id = 10").lastError().type() == QSqlError::NoError);
        QVERIFY(f.size() == 2);

        QVERIFY(db.exec("DELETE FROM aseqs").lastError().type() == QSqlError::NoError);
        QVERIFY(f.size() == 0);

        db.close();
    }

    QSqlDatabase::removeDatabase("test_db");
}

QTEST_MAIN(TestDbAnonSeqFactory)
#include "TestDbAnonSeqFactory.moc"


