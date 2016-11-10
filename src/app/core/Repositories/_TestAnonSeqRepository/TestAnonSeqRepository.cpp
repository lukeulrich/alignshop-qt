/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../AnonSeqRepository.h"
#include "../../DataMappers/AnonSeqMapper.h"
#include "../../DataSources/SqliteAdocSource.h"
#include "../../Entities/Astring.h"

class TestAnonSeqRepository : public QObject
{
    Q_OBJECT

private slots:
    void construction();
    void findBySeq();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestAnonSeqRepository::construction()
{
    SqliteAdocSource source;
    AnonSeqMapper<Astring> astringMapper(&source);
    AnonSeqRepository<Astring> repo(&astringMapper);
}

void TestAnonSeqRepository::findBySeq()
{
    SqliteAdocSource source;
    AnonSeqMapper<Astring> astringMapper(&source);

    QFile::remove("test.db");
    QVERIFY(source.createAndOpen("test.db"));

    // Test: insertion and save through the repository
    Seq seq1("ABCDEF", eAminoGrammar);
    {
        Astring *astring = new Astring(::newEntityId<Astring>(), seq1);
        astring->addCoil(Coil(ClosedIntRange(1, 4)));
        AnonSeqRepository<Astring> repo(&astringMapper);
        QVERIFY(repo.addOne(astring, false));
        repo.saveAll();

        astring = nullptr;  // Repo takes care of deleting pointer
    }

    // Test: fetch sequence not already loaded in the repository
    {
        AnonSeqRepository<Astring> repo(&astringMapper);
        Astring *inserted = repo.findBySeq(seq1);
        QVERIFY(inserted != nullptr);
        QCOMPARE(inserted->seq_, seq1);
        QCOMPARE(inserted->coils().size(), 1);
        QCOMPARE(inserted->coils().at(0), Coil(ClosedIntRange(1, 4)));
        inserted = nullptr;
    }

    QFile::remove("test.db");
}

QTEST_APPLESS_MAIN(TestAnonSeqRepository)
#include "TestAnonSeqRepository.moc"
