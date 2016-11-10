/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../../DataSources/SqliteAdocSource.h"
#include "../GenericEntityMapper.h"
#include "../../Entities/Astring.h"

class TestGenericEntityMapper : public QObject
{
    Q_OBJECT

private slots:
    void construction();
    void insertion();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestGenericEntityMapper::construction()
{
    SqliteAdocSource source;
    GenericEntityMapper<Astring, AstringPod> astringMapper(&source);
}

void TestGenericEntityMapper::insertion()
{
    if (QFile::exists("test.db"))
        QFile::remove("test.db");

    SqliteAdocSource source;
    source.createAndOpen("test.db");
    QVERIFY(source.isOpen());

    GenericEntityMapper<Astring, AstringPod> astringMapper(&source);

    // Test saving an astring
    Astring *astring = new Astring(::newEntityId<Astring>(), Seq("ABCDEF", eAminoGrammar));
    QVERIFY(astringMapper.saveOne(astring));

    int insertedId = astring->id();
    delete astring;

    astring = astringMapper.findOne(insertedId);
    QCOMPARE(astring->seq_.asByteArray(), QByteArray("ABCDEF"));

    source.close();
    QFile::remove("test.db");
}


QTEST_APPLESS_MAIN(TestGenericEntityMapper)
#include "TestGenericEntityMapper.moc"
