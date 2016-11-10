/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtSql/QSqlError>

#include <QtTest/QtTest>

#include "../../Entities/Astring.h"
#include "../DbAstringCrud.h"
#include "../../MockDbSource.h"

#include "../../../Seq.h"
#include "../../../global.h"

// Note: Upon constructing a MockDbSource, the necessary SQL tables and sample data are created.
class TestDbAstringCrud : public QObject
{
    Q_OBJECT

private slots:
    // Not testing erase, because erasing of astrings will not occur via a CRUD interface
    void read();
    void readByDigests();
    void save_insert();
    void save_update();
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestDbAstringCrud::read()
{
    MockDbSource source;
    DbAstringCrud crud(&source);
    QVector<Seq> seqs_ = source.aseqs();

    try
    {
        QVector<AstringPod> astringPods = crud.read(QVector<int>() << 8 << 1 << 3 << 10);
        QCOMPARE(astringPods.size(), 4);
        QVERIFY(astringPods.at(0).isNull());
        QVERIFY(astringPods.at(3).isNull());

        const AstringPod &astringPod = astringPods.at(1);
        QCOMPARE(astringPod.id_, 1);
        QCOMPARE(astringPod.seq_, seqs_.at(0));

        // Check the coils
        QVector<Coil> coils;
        coils << Coil(ClosedIntRange(1, 3));
        QVector<Seg> segs;
        segs << Seg(ClosedIntRange(4, 6));
        QCOMPARE(astringPod.coils_, coils);
        QCOMPARE(astringPod.segs_, segs);
        coils.clear();
        segs.clear();

        const AstringPod &astringPod2 = astringPods.at(2);
        QCOMPARE(astringPod2.id_, 3);
        QCOMPARE(astringPod2.seq_, seqs_.at(2));
        segs << Seg(ClosedIntRange(1, 6)) << Seg(ClosedIntRange(9, 12));
        QVERIFY(astringPod2.coils_.isEmpty());
        QCOMPARE(astringPod2.segs_, segs);
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

void TestDbAstringCrud::readByDigests()
{
    MockDbSource source;
    DbAstringCrud crud(&source);
    QVector<Seq> seqs_ = source.aseqs();

    try
    {
        // Test: Attempt to read sequences that don't exist
        QVector<AstringPod> astringPods = crud.readByDigests(QVector<QByteArray>() << "blargh" << "digest");
        QCOMPARE(astringPods.size(), 2);
        QVERIFY(astringPods.at(0).isNull());
        QVERIFY(astringPods.at(1).isNull());

        // Test: attempt to read a valid sequence
        AstringPod astringPod = crud.readByDigests(QVector<QByteArray>() << seqs_.at(0).digest()).first();
        QVERIFY(astringPod.isNull() == false);
        QCOMPARE(astringPod.id_, 1);
        QCOMPARE(astringPod.seq_, seqs_.at(0));

        // Test: attempt to read mixed group
        astringPods = crud.readByDigests(QVector<QByteArray>() << seqs_.at(1).digest() << "invalid" << seqs_.at(2).digest());
        QCOMPARE(astringPods.size(), 3);
        QVERIFY(astringPods.at(1).isNull());
        QCOMPARE(astringPods.at(0).id_, 2);
        QCOMPARE(astringPods.at(0).seq_, seqs_.at(1));
        QCOMPARE(astringPods.at(2).id_, 3);
        QCOMPARE(astringPods.at(2).seq_, seqs_.at(2));
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

void TestDbAstringCrud::save_insert()
{
    MockDbSource source;
    DbAstringCrud crud(&source);
    QVector<Seq> seqs_ = source.aseqs();

    try
    {
        Seq seq("RVRQGEGGA", eAminoGrammar);
        Astring *astring = new Astring(::newEntityId<Astring>(), seq);
        QVector<Coil> testCoils;
        testCoils << Coil(ClosedIntRange(1, 2)) << Coil(ClosedIntRange(5, 6));
        astring->setCoils(testCoils);
        QVector<Seg> testSegs;
        testSegs << Seg(ClosedIntRange(3, 4));
        astring->setSegs(testSegs);

        QVERIFY(astring->isNew());
        crud.save(QVector<Astring *>() << astring);
        QCOMPARE(astring->isNew(), false);
        QCOMPARE(astring->isDirty(), false);
        QVERIFY(astring->id() > 3);

        // Check that it was inserted and the data was preserved
        AstringPod inserted = crud.read(QVector<int>() << astring->id()).first();
        QVERIFY(inserted.isNull() == false);
        QCOMPARE(inserted.seq_, seq);
        QCOMPARE(inserted.coils_, testCoils);
        QCOMPARE(inserted.segs_, testSegs);

        // Test: inserting sequence that already exists should fail
        delete astring;
        astring = nullptr;
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

void TestDbAstringCrud::save_update()
{
    MockDbSource source;
    DbAstringCrud crud(&source);

    try
    {
        // Retrieve a sequence of interest
        AstringPod astringPod = crud.read(QVector<int>() << 1).first();
        Astring *astring = new Astring(astringPod);
        QVERIFY(astring);
        QVERIFY(astring->coils().isEmpty() == false);
        QVERIFY(astring->segs().isEmpty() == false);

        astring->setCoils(QVector<Coil>());    // Remove all coils
        astring->addSeg(ClosedIntRange(2, 5));

        crud.save(QVector<Astring *>() << astring);
        delete astring;
        astring = nullptr;

        astringPod = crud.read(QVector<int>() << 1).first();
        astring = new Astring(astringPod);
        QVERIFY(astring->coils().isEmpty());
        QCOMPARE(astring->segs().size(), 2);
        if (astring->segs().at(0).begin() == 2)
            QCOMPARE(astring->segs().at(0).location(), ClosedIntRange(2, 5));
        else
            QCOMPARE(astring->segs().at(1).location(), ClosedIntRange(2, 5));

        delete astring;
        astring = nullptr;
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

QTEST_APPLESS_MAIN(TestDbAstringCrud)
#include "TestDbAstringCrud.moc"
