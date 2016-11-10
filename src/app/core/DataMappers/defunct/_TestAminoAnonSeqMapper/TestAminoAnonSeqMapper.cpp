/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../AminoAnonSeqMapper.h"
#include "../../DataSources/MockAdocSource.h"
#include "../../Entities/AminoAnonSeq.h"
#include "../../global.h"

class TestAminoAnonSeqMapper : public QObject
{
    Q_OBJECT

private slots:
    void erase();
    void find();
    void findByDigest();
    void save_insert();
    void save_update();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAminoAnonSeqMapper::erase()
{
    MockAdocSource source;
    AminoAnonSeqMapper x(&source);

    // Fetch and erase one
    AminoAnonSeq *aseq = x.findOne(6);
    QVERIFY(aseq);
    QVERIFY(x.eraseOne(aseq));
    QCOMPARE(aseq->id(), 0);
    delete aseq;
    aseq = 0;

    aseq = x.findOne(6);
    QVERIFY(aseq == nullptr);

    // Fetch and erase many
    QVector<AminoAnonSeq *> aseqs = x.find(QVector<int>() << 5 << 7);
    QVERIFY(aseqs.at(0));
    QVERIFY(aseqs.at(1));
    aseqs << AminoAnonSeq::create(Seq("GCGMNLST", eAminoGrammar));
    QVERIFY(aseqs.at(2));
    QVERIFY(aseqs.at(2)->isNew());

    QVERIFY(x.erase(aseqs));
    QCOMPARE(aseqs.at(0)->id(), 0);
    QCOMPARE(aseqs.at(1)->id(), 0);
    QVERIFY(aseqs.at(2)->id() < 0);
    QVERIFY(aseqs.at(2)->isNew());
    qDeleteAll(aseqs);
    aseqs.clear();
    aseqs = x.find(QVector<int>() << 5 << 7);
    QVERIFY(aseqs.at(0) == nullptr);
    QVERIFY(aseqs.at(1) == nullptr);
}

void TestAminoAnonSeqMapper::find()
{
    MockAdocSource source;
    AminoAnonSeqMapper x(&source);

    // Test: non-existent amino anon seq
    AminoAnonSeq *aseq = x.findOne(9999);
    QVERIFY(aseq == nullptr);

    aseq = x.findOne(6);
    QVERIFY(aseq);
    QCOMPARE(aseq->id(), 6);
    QCOMPARE(aseq->seq_, Seq("ABCDEF", eAminoGrammar));
    QVERIFY(aseq->coils().isEmpty());
    QCOMPARE(aseq->segs().size(), 2);
    QCOMPARE(aseq->segs().at(0), Seg(60, ClosedIntRange(1, 3)));
    QCOMPARE(aseq->segs().at(1), Seg(61, ClosedIntRange(4, 6)));
    delete aseq;
    aseq = nullptr;

    QVector<AminoAnonSeq *> aseqs = x.find(QVector<int>() << 4 << 5 << 7);
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
    qDeleteAll(aseqs);
    aseqs.clear();
}

void TestAminoAnonSeqMapper::findByDigest()
{
    MockAdocSource source;
    AminoAnonSeqMapper x(&source);

    // Test: non-existent amino anon seq
    AminoAnonSeq *aseq = x.findOneByDigest("-------");
    QVERIFY(aseq == nullptr);

    // Test: check for digest
    Seq seq6("ABCDEF", eAminoGrammar);
    aseq = x.findOneByDigest(seq6.digest());
    QVERIFY(aseq);
    QCOMPARE(aseq->id(), 6);
    QCOMPARE(aseq->seq_, seq6);
    QVERIFY(aseq->coils().isEmpty());
    QCOMPARE(aseq->segs().size(), 2);
    QCOMPARE(aseq->segs().at(0), Seg(60, ClosedIntRange(1, 3)));
    QCOMPARE(aseq->segs().at(1), Seg(61, ClosedIntRange(4, 6)));
    delete aseq;
    aseq = nullptr;

    Seq seq5("ABCDEFGHIJKLMNOPQRSTUVWXYZ", eAminoGrammar);
    Seq seq7("GHIJKLMNOP", eAminoGrammar);
    QVector<AminoAnonSeq *> aseqs = x.findByDigests(QVector<QByteArray>() << "" << seq5.digest() << seq7.digest());
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
    qDeleteAll(aseqs);
    aseqs.clear();
}

void TestAminoAnonSeqMapper::save_insert()
{
    MockAdocSource source;
    AminoAnonSeqMapper x(&source);

    // Test: insert new astring with its annotation
    Seq seq("ABCDEFG", eAminoGrammar);
    AminoAnonSeq *aseq = AminoAnonSeq::create(seq);
    QVector<Coil> testCoils;
    testCoils << Coil(ClosedIntRange(1, 2)) << Coil(ClosedIntRange(5, 6));
    aseq->setCoils(testCoils);
    QVector<Seg> testSegs;
    testSegs << Seg(ClosedIntRange(3, 4));
    aseq->setSegs(testSegs);

    QVERIFY(aseq->isNew());
    QVERIFY(x.saveOne(aseq));
    QCOMPARE(aseq->isNew(), false);

    // Check that it was inserted and the data was preserved
    AminoAnonSeq *inserted = x.findOne(aseq->id());
    QVERIFY(inserted);
    QCOMPARE(inserted->seq_, Seq("ABCDEFG", eAminoGrammar));
    QCOMPARE(inserted->coils(), testCoils);
    QCOMPARE(inserted->segs(), testSegs);

    // Test: inserting sequence that already exists should fail
    delete aseq;
    aseq = nullptr;
    aseq = AminoAnonSeq::create(seq);
    QVERIFY(x.saveOne(aseq) == false);
    delete aseq;
    aseq = nullptr;
}

void TestAminoAnonSeqMapper::save_update()
{
    MockAdocSource source;
    AminoAnonSeqMapper x(&source);

    // Test: retrieve a sequence of interest
    AminoAnonSeq *aseq = x.findOne(7);
    QVector<Coil> originalCoils = aseq->coils();
    QVector<Seg> originalSegs = aseq->segs();

    aseq->setCoils(QVector<Coil>());    // Remove all coils
    aseq->addSeg(ClosedIntRange(3, 4));

    QVERIFY(x.saveOne(aseq));
    delete aseq;
    aseq = x.findOne(7);

    QVector<Coil> newCoils = aseq->coils();
    QVector<Seg> newSegs = aseq->segs();

    QVERIFY(aseq->coils().isEmpty());
    QCOMPARE(aseq->segs().size(), 2);
    if (aseq->segs().at(0).begin() == 8)
        QCOMPARE(aseq->segs().at(1).location(), ClosedIntRange(3, 4));
    else
        QCOMPARE(aseq->segs().at(0).location(), ClosedIntRange(3, 4));
}

QTEST_APPLESS_MAIN(TestAminoAnonSeqMapper)
#include "TestAminoAnonSeqMapper.moc"
