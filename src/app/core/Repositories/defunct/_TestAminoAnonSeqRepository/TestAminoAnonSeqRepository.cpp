/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../AminoAnonSeqRepository.h"
#include "../../DataSources/MockAdocSource.h"
#include "../../DataMappers/AminoAnonSeqMapper.h"

class TestAminoAnonSeqRepository : public QObject
{
    Q_OBJECT

private slots:
    void add();
    void erase();   // Also tests unerase
    void find();    // Also tests unfind
    void findBySeq();
    void findBySeqOrCreate();
    void save_insert();
    void save_update();
    void saveAll();
};

// Note: It is not necessary to manually free the memory because this is automatically handled by the repository!!
//       Thus, no delete's are present in the code below

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAminoAnonSeqRepository::add()
{
    MockAdocSource source;
    AminoAnonSeqMapper mapper(&source);
    AminoAnonSeqRepository repo(&mapper);

    // Test: adding null pointers
    QCOMPARE(repo.addOne(nullptr, false), false);
    QVERIFY(repo.addOne(nullptr, true));

    // Test: manually add pointer
    AminoAnonSeq *aseq1 = new AminoAnonSeq(10, Seq("ABCDEF", eAminoGrammar));
    QCOMPARE(repo.addOne(aseq1, false), true);
    QVERIFY(repo.findOne(10) == aseq1);
    repo.unfindOne(aseq1);  // To release the count from the add method
    repo.unfindOne(aseq1);  // To release the count via the find method

    // Test: attempt to add the same sequence again
    QCOMPARE(repo.addOne(aseq1, false), false);

    // Test: add another entity with different address, but same id
    AminoAnonSeq *aseq2 = new AminoAnonSeq(10, Seq("GHIJKL", eAminoGrammar));
    QVERIFY(aseq1 != aseq2);
    QCOMPARE(repo.addOne(aseq2, false), false);
    QVERIFY(repo.findOne(10) == aseq1);
    repo.unfindOne(aseq1);

    // Test: add negative ids
    AminoAnonSeq *aseq3 = AminoAnonSeq::create(Seq("GHIJKLM", eAminoGrammar));
    QVERIFY(repo.addOne(aseq3, false));
    QVERIFY(repo.findOne(aseq3->id()) == aseq3);
    repo.unfindOne(aseq3);
    repo.unfindOne(aseq3);
}

void TestAminoAnonSeqRepository::erase()
{
    MockAdocSource source;
    AminoAnonSeqMapper mapper(&source);
    AminoAnonSeqRepository repo(&mapper);

    // Test: erase null should return false
    QCOMPARE(repo.eraseOne(nullptr), false);

    // Test: erase multiples - one valid and one null; should fail and all valid entries remain
    QVector<AminoAnonSeq *> aseqs = repo.find(QVector<int>() << 6 << 7);
    QCOMPARE(aseqs.at(0)->id(), 6);
    QCOMPARE(aseqs.at(1)->id(), 7);
    repo.unfind(aseqs);
    QCOMPARE(repo.erase(QVector<AminoAnonSeq *>() << aseqs.at(0) << nullptr), false);
    QVERIFY(repo.findOne(6) == aseqs.at(0));
    QVERIFY(repo.findOne(7) == aseqs.at(1));
    // At this point, the above find commands have the reference count set to 1 for both aseqs.

    // Test: erase multiple valid entries
    QVERIFY(repo.erase(aseqs));
    QVERIFY(repo.findOne(6) == nullptr);
    QVERIFY(repo.findOne(7) == nullptr);
    // Now both aseqs have been soft erased. Test that we can still unfind them even in their erased state
    repo.unfind(aseqs);

    // Test: attempt un-erase with one valid and one null
    QCOMPARE(repo.unerase(QVector<AminoAnonSeq *>() << aseqs.at(1) << nullptr), false);
    QVERIFY(repo.findOne(7) == nullptr);

    // Test: attempt un-erase of a single valid unit
    QVERIFY(repo.uneraseOne(aseqs.at(1)));
    QVERIFY(repo.findOne(7) == aseqs.at(1));
    QVERIFY(repo.findOne(6) == nullptr);
    QVERIFY(repo.uneraseOne(aseqs.at(0)));
    QVERIFY(repo.findOne(6) == aseqs.at(0));
    repo.unfind(aseqs);

    // Test: erase and un-erase of entries
    QVERIFY(repo.erase(aseqs));
    QVERIFY(repo.unerase(aseqs));
    QVERIFY(repo.findOne(6) == aseqs.at(0));
    QVERIFY(repo.findOne(7) == aseqs.at(1));
    repo.unfind(aseqs);
}

void TestAminoAnonSeqRepository::find()
{
    MockAdocSource source;
    AminoAnonSeqMapper mapper(&source);
    AminoAnonSeqRepository repo(&mapper);

    // Test: non-existent amino anon seq
    AminoAnonSeq *aseq = repo.findOne(9999);
    QVERIFY(aseq == nullptr);

    aseq = repo.findOne(6);
    QVERIFY(aseq);
    QCOMPARE(aseq->id(), 6);
    QCOMPARE(aseq->seq_, Seq("ABCDEF", eAminoGrammar));
    QVERIFY(aseq->coils().isEmpty());
    QCOMPARE(aseq->segs().size(), 2);
    QCOMPARE(aseq->segs().at(0), Seg(60, ClosedIntRange(1, 3)));
    QCOMPARE(aseq->segs().at(1), Seg(61, ClosedIntRange(4, 6)));

    // Test: check that we get the same pointer from the same call
    QVERIFY(aseq == repo.findOne(6));
    repo.unfindOne(aseq);
    repo.unfindOne(aseq);

    QVector<AminoAnonSeq *> aseqs = repo.find(QVector<int>() << 4 << 5 << 7);
    QCOMPARE(aseqs.size(), 3);
    QVERIFY(aseqs.at(0) == nullptr);
    QCOMPARE(aseqs.at(1)->id(), 5);
    QCOMPARE(aseqs.at(1)->seq_, Seq("ABCDEFGHIJKLMNOPQRSTUVWXYZ", eAminoGrammar));
    QCOMPARE(aseqs.at(1)->coils().size(), 2);
    QCOMPARE(aseqs.at(1)->coils().at(0), Coil(50, ClosedIntRange(1, 10)));
    QCOMPARE(aseqs.at(1)->coils().at(1), Coil(51, ClosedIntRange(21, 24)));
    QVERIFY(aseqs.at(1)->segs().isEmpty());

    QCOMPARE(aseqs.at(2)->id(), 7);
    QCOMPARE(aseqs.at(2)->seq_, Seq("GHIJKLMNOP", eAminoGrammar));
    QCOMPARE(aseqs.at(2)->coils().size(), 1);
    QCOMPARE(aseqs.at(2)->coils().at(0), Coil(70, ClosedIntRange(3, 8)));
    QCOMPARE(aseqs.at(2)->segs().size(), 1);
    QCOMPARE(aseqs.at(2)->segs().at(0), Seg(70, ClosedIntRange(8, 10)));

    repo.unfind(aseqs);
}

void TestAminoAnonSeqRepository::findBySeq()
{
    MockAdocSource source;
    AminoAnonSeqMapper mapper(&source);
    AminoAnonSeqRepository repo(&mapper);

    // Test: attempt to find a non-existent sequence
    AminoAnonSeq *aseq = repo.findBySeq(Seq("Nonexisting", eAminoGrammar));
    QVERIFY(aseq == nullptr);

    // Test: attempt to find a real sequence in the mock data source
    Seq seq("ABCDEFGHIJKLMNOPQRSTUVWXYZ", eAminoGrammar);
    aseq = repo.findBySeq(seq);
    QVERIFY(aseq);
    QCOMPARE(aseq->id(), 5);
    QCOMPARE(aseq->seq_, seq);
    QCOMPARE(aseq->coils().size(), 2);
    QCOMPARE(aseq->coils().at(0), Coil(50, ClosedIntRange(1, 10)));
    QCOMPARE(aseq->coils().at(1), Coil(51, ClosedIntRange(21, 24)));
    QVERIFY(aseq->segs().isEmpty());
    repo.unfindOne(aseq);
}

void TestAminoAnonSeqRepository::findBySeqOrCreate()
{
    MockAdocSource source;
    AminoAnonSeqMapper mapper(&source);
    AminoAnonSeqRepository repo(&mapper);

    // Test: attempt to find a real sequence in the mock data source
    Seq seq("ABCDEFGHIJKLMNOPQRSTUVWXYZ", eAminoGrammar);
    AminoAnonSeq *aseq = repo.findBySeqOrCreate(seq);
    QVERIFY(aseq);
    QVERIFY(aseq->isNew() == false);
    QCOMPARE(aseq->id(), 5);
    QCOMPARE(aseq->seq_, seq);
    QCOMPARE(aseq->coils().size(), 2);
    QCOMPARE(aseq->coils().at(0), Coil(50, ClosedIntRange(1, 10)));
    QCOMPARE(aseq->coils().at(1), Coil(51, ClosedIntRange(21, 24)));
    QVERIFY(aseq->segs().isEmpty());
    repo.unfindOne(aseq);

    // Test: creation of an AminoAnonSeq
    Seq newSeq("GRHIPPLMNQST", eAminoGrammar);
    QVERIFY(repo.findBySeq(newSeq) == nullptr); // Verify it does not exist
    AminoAnonSeq *newAseq = repo.findBySeqOrCreate(newSeq);
    QVERIFY(newAseq->isNew());
    QCOMPARE(newAseq->seq_, newSeq);
}

void TestAminoAnonSeqRepository::save_insert()
{
    MockAdocSource source;
    AminoAnonSeqMapper mapper(&source);
    AminoAnonSeqRepository repo(&mapper);

    // Test: insert new astring with its annotation
    Seq seq("ABCDEFG", eAminoGrammar);
    AminoAnonSeq *aseq = AminoAnonSeq::create(seq);
    QVector<Coil> testCoils;
    testCoils << Coil(ClosedIntRange(1, 2)) << Coil(ClosedIntRange(5, 6));
    aseq->setCoils(testCoils);
    QVector<Seg> testSegs;
    testSegs << Seg(ClosedIntRange(3, 4));
    aseq->setSegs(testSegs);

    QVERIFY(repo.addOne(aseq, false));
    repo.unfindOne(aseq);
    QVERIFY(repo.saveOne(aseq));
    QCOMPARE(aseq->isNew(), false);

    {
        AminoAnonSeqRepository repo2(&mapper);
        AminoAnonSeq *aseq_test = repo2.findOne(aseq->id());
        QVERIFY(aseq_test);
        QVERIFY(aseq_test != aseq);
        QCOMPARE(aseq_test->seq_, aseq->seq_);
        QCOMPARE(aseq_test->coils(), aseq->coils());
        QCOMPARE(aseq_test->segs(), aseq->segs());
        repo2.unfindOne(aseq_test);
    }
}

void TestAminoAnonSeqRepository::save_update()
{
    MockAdocSource source;
    AminoAnonSeqMapper mapper(&source);
    AminoAnonSeqRepository repo(&mapper);

    // Test: retrieve a sequence of interest
    AminoAnonSeq *aseq = repo.findOne(7);
    QVector<Coil> originalCoils = aseq->coils();
    QVector<Seg> originalSegs = aseq->segs();
    aseq->setCoils(QVector<Coil>());    // Remove all coils
    aseq->addSeg(ClosedIntRange(3, 4));
    repo.unfindOne(aseq);

    QVERIFY(repo.saveOne(aseq));
    {
        AminoAnonSeqRepository repo2(&mapper);
        AminoAnonSeq *aseq_test = repo2.findOne(7);
        QVERIFY(aseq_test);
        QVERIFY(aseq_test != aseq);
        QCOMPARE(aseq_test->seq_, aseq->seq_);
        QCOMPARE(aseq_test->coils(), aseq->coils());
        QCOMPARE(aseq_test->segs(), aseq->segs());
        repo2.unfindOne(aseq_test);
    }
}

void TestAminoAnonSeqRepository::saveAll()
{
    MockAdocSource source;
    AminoAnonSeqMapper mapper(&source);
    AminoAnonSeqRepository repo(&mapper);

    // Test: find some sequences, tweak them, and then save
    {
        QVector<AminoAnonSeq *> aseqs = repo.find(QVector<int>() << 5 << 6 << 7);
        aseqs.at(0)->setCoils(QVector<Coil>());
        aseqs.at(0)->addSeg(ClosedIntRange(1, 5));
        aseqs.at(0)->addSeg(ClosedIntRange(8, 15));

        aseqs.at(2)->setCoils(QVector<Coil>());
        aseqs.at(2)->setSegs(QVector<Seg>());

        QVERIFY(repo.eraseOne(aseqs.at(1)));
        repo.unfind(aseqs);

        repo.saveAll();
    }

    QVector<AminoAnonSeq *> aseqs = repo.find(QVector<int>() << 5 << 6 << 7);
    QVERIFY(aseqs.at(1) == nullptr);
    QVERIFY(aseqs.at(0)->coils().isEmpty());
    QCOMPARE(aseqs.at(0)->segs(), QVector<Seg>() << ClosedIntRange(1, 5) << ClosedIntRange(8, 15));
    QVERIFY(aseqs.at(2)->coils().isEmpty());
    QVERIFY(aseqs.at(2)->segs().isEmpty());
}

QTEST_APPLESS_MAIN(TestAminoAnonSeqRepository)
#include "TestAminoAnonSeqRepository.moc"
