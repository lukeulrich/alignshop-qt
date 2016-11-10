/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtSql/QSqlError>
#include <QtTest/QtTest>

#include "../DbAminoSeqCrud.h"
#include "../../Entities/AminoSeq.h"
#include "../../MockDbSource.h"

#include "../../../Seq.h"
#include "../../../global.h"

// Note: Upon constructing a MockDbSource, the necessary SQL tables and sample data are created.
class TestDbAminoSeqCrud : public QObject
{
    Q_OBJECT

private slots:
    void erase();
    void read();
    void save_insert();
    void save_update();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestDbAminoSeqCrud::erase()
{
    MockDbSource source;
    DbAminoSeqCrud crud(&source);

    try
    {
        QSqlQuery query(source.database());
        QVERIFY(query.prepare("SELECT count(*) FROM amino_seqs"));
        QVERIFY(query.exec());
        QVERIFY(query.next());
        int nRows = query.value(0).toInt();

        QVector<AminoSeqPod> aseqPods = crud.read(QVector<int>() << 1 << 11);
        aseqPods[1].astring_ = new Astring(100, source.aseqs().last());
        QVector<AminoSeq *> aseqs(2, nullptr);
        aseqs[1] = new AminoSeq(aseqPods.at(1));

        crud.erase(aseqs);

        query.exec();
        query.next();
        QCOMPARE(query.value(0).toInt(), nRows - 1);

        QVERIFY(crud.read(QVector<int>() << 11).first().isNull());
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

void TestDbAminoSeqCrud::read()
{
    MockDbSource source;
    DbAminoSeqCrud crud(&source);

    try
    {
        // Test: empty amino seqs
        QVector<AminoSeqPod> aseqPods = crud.read(QVector<int>() << 0 << 999);
        QCOMPARE(aseqPods.size(), 2);
        QVERIFY(aseqPods.at(0).isNull());
        QVERIFY(aseqPods.at(1).isNull());

        // Test: read valid amino seqs
        aseqPods = crud.read(QVector<int>() << 11 << 12 << 13);
        QCOMPARE(aseqPods.size(), 3);
        QVERIFY(aseqPods.at(2).isNull());

        const AminoSeqPod *aseqPod = &aseqPods.at(0);
        QCOMPARE(aseqPod->id_, 11);
        QCOMPARE(aseqPod->astringId_, 3);
        QCOMPARE(aseqPod->start_, 1);
        QCOMPARE(aseqPod->stop_, 7);
        QCOMPARE(aseqPod->name_, QString("Beta"));
        QVERIFY(aseqPod->source_.isEmpty());
        QVERIFY(aseqPod->description_.isEmpty());
        QCOMPARE(aseqPod->notes_, QString("No notes"));

        aseqPod = &aseqPods.at(1);
        QCOMPARE(aseqPod->id_, 12);
        QCOMPARE(aseqPod->astringId_, 3);
        QCOMPARE(aseqPod->start_, 8);
        QCOMPARE(aseqPod->stop_, 14);
        QCOMPARE(aseqPod->name_, QString("Gamma"));
        QCOMPARE(aseqPod->source_, QString("Source"));
        QCOMPARE(aseqPod->description_, QString("last half"));
        QCOMPARE(aseqPod->notes_, QString("Notes"));
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

void TestDbAminoSeqCrud::save_insert()
{
    MockDbSource source;
    DbAminoSeqCrud crud(&source);
    QVector<Seq> seqs = source.aseqs();

    try
    {
        // Get number of sequences in table
        QSqlQuery query(source.database());
        QVERIFY(query.prepare("SELECT count(*) FROM amino_seqs"));
        QVERIFY(query.exec());
        QVERIFY(query.next());
        int nRows = query.value(0).toInt();

        Astring *astring = new Astring(2, seqs.at(1));
        AminoSeq *aseq = new AminoSeq(::newEntityId<AminoSeq>(), 1, 13, "New sequence", "Source", "description", "Notes", astring);
        QVERIFY(aseq->isNew());
        crud.save(QVector<AminoSeq *>() << aseq);
        QCOMPARE(aseq->isNew(), false);
        QCOMPARE(aseq->isDirty(), false);

        QVERIFY(query.exec());
        QVERIFY(query.next());
        QCOMPARE(query.value(0).toInt(), nRows + 1);
        ++nRows;

        // Check that it was inserted and the data was preserved
        AminoSeqPod inserted = crud.read(QVector<int>() << aseq->id()).first();
        QVERIFY(inserted.isNull() == false);
        QCOMPARE(inserted.id_, aseq->id());
        QCOMPARE(inserted.start_, aseq->ungappedSubseq().start());
        QCOMPARE(inserted.stop_, aseq->ungappedSubseq().stop());
        QCOMPARE(inserted.name_, aseq->name());
        QCOMPARE(inserted.source_, aseq->source());
        QCOMPARE(inserted.description_, aseq->description());
        QCOMPARE(inserted.notes_, aseq->notes());
        QCOMPARE(inserted.astringId_, astring->id());
        QVERIFY(inserted.astring_ == nullptr);

        // Test: inserting sequence that already exists should fail
        delete aseq;
        aseq = nullptr;
        delete astring;
        astring = nullptr;
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

void TestDbAminoSeqCrud::save_update()
{
    MockDbSource source;
    DbAminoSeqCrud crud(&source);

    try
    {
        // Retrieve a sequence of interest
        AminoSeqPod oldAseqPod = crud.read(QVector<int>() << 10).first();
        AminoSeqPod changedAseqPod = crud.read(QVector<int>() << 10).first();
        QVERIFY(oldAseqPod.isNull() == false);
        QVERIFY(changedAseqPod.isNull() == false);

        oldAseqPod.astring_ = new Astring(1, source.aseqs().first());
        AminoSeq *oldAseq = new AminoSeq(oldAseqPod);
        changedAseqPod.astring_ = new Astring(1, source.aseqs().first());
        AminoSeq *changedAseq = new AminoSeq(changedAseqPod);
        QVERIFY(oldAseq != nullptr);
        QVERIFY(changedAseq != nullptr);

        changedAseq->setStart(oldAseq->ungappedSubseq().start() + 1);
        changedAseq->setStop(oldAseq->ungappedSubseq().stop() - 1);
        changedAseq->setName("New name");
        changedAseq->setSource("New Source");
        changedAseq->setDescription("New description");
        changedAseq->setNotes("New notes");
        crud.save(QVector<AminoSeq *>() << changedAseq);

        AminoSeqPod updatedPod = crud.read(QVector<int>() << 10).first();
        QCOMPARE(updatedPod.id_, oldAseq->id());
        QCOMPARE(updatedPod.astringId_, oldAseq->abstractAnonSeq()->id());
        QCOMPARE(updatedPod.start_, changedAseq->ungappedSubseq().start());
        QCOMPARE(updatedPod.stop_, changedAseq->ungappedSubseq().stop());
        QCOMPARE(updatedPod.name_, changedAseq->name());
        QCOMPARE(updatedPod.source_, changedAseq->source());
        QCOMPARE(updatedPod.description_, changedAseq->description());
        QCOMPARE(updatedPod.notes_, changedAseq->notes());

        delete oldAseq;
        delete changedAseq;
        oldAseq = nullptr;
        changedAseq = nullptr;
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

QTEST_APPLESS_MAIN(TestDbAminoSeqCrud)
#include "TestDbAminoSeqCrud.moc"
