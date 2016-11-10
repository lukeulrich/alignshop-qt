/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../AnonSeqMapper.h"
#include "../../DataSources/SqliteAdocSource.h"
#include "../../Entities/Astring.h"

class TestAstringMapper : public QObject
{
    Q_OBJECT

private slots:
    void construction();
    void findByDigests();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAstringMapper::construction()
{
    SqliteAdocSource source;
    AnonSeqMapper<Astring, AstringPod> astringMapper(&source);
}

void TestAstringMapper::findByDigests()
{
    if (QFile::exists("test.db"))
        QFile::remove("test.db");

    SqliteAdocSource source;
    QVERIFY(source.createAndOpen("test.db"));

    AnonSeqMapper<Astring, AstringPod> astringMapper(&source);

    Seq seq1("ABCDEF", eAminoGrammar);
    Astring *astring = new Astring(::newEntityId<Astring>(), seq1);
    astring->addCoil(Coil(ClosedIntRange(1, 4)));
    QVERIFY(astringMapper.saveOne(astring));
    delete astring;

    Seq seq2("GHIJKL", eAminoGrammar);
    astring = new Astring(::newEntityId<Astring>(), seq2);
    astring->addSeg(Seg(ClosedIntRange(3, 6)));
    QVERIFY(astringMapper.saveOne(astring));
    delete astring;

    astring = astringMapper.findOneByDigest(seq1.digest());
    QVERIFY(astring != nullptr);
    QCOMPARE(astring->seq_, seq1);
    QCOMPARE(astring->coils().size(), 1);
    QCOMPARE(astring->coils().at(0), Coil(ClosedIntRange(1, 4)));
    delete astring;

    astring = astringMapper.findOneByDigest(seq2.digest());
    QVERIFY(astring != nullptr);
    QCOMPARE(astring->seq_, seq2);
    QCOMPARE(astring->segs().size(), 1);
    QCOMPARE(astring->segs().at(0), Seg(ClosedIntRange(3, 6)));
    delete astring;

    source.close();

    QFile::remove("test.db");
}

QTEST_APPLESS_MAIN(TestAstringMapper)
#include "TestAstringMapper.moc"
