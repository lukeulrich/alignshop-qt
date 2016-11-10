/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtSql/QSqlField>

#include "AnonSeq.h"
#include "BioString.h"
#include "DataRow.h"
#include "RichSubseq.h"

class TestRichSubseq : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorBasic();

    // ------------------------------------------------------------------------------------------------
    // Public members
    void annotation();
};

void TestRichSubseq::constructorBasic()
{
    AnonSeq anonSeq(1, BioString("ABC...DEF"));

    RichSubseq rs1(anonSeq);
    RichSubseq rs2(anonSeq, 1);

    DataRow info(100);
    info.append(QSqlField("name", QVariant::String));
    info.setValue("name", "LuxR");

    RichSubseq rs3(anonSeq, 10, info);

    QCOMPARE(rs3.annotation_.value("name").toString(), QString("LuxR"));
}

void TestRichSubseq::annotation()
{
    AnonSeq anonSeq(1, BioString("ABC...DEF"));
    RichSubseq rs1(anonSeq);

    rs1.annotation_.append(QSqlField("name", QVariant::String));
    rs1.annotation_.setValue("name", "LuxR");
    QCOMPARE(rs1.annotation_.value("name").toString(), QString("LuxR"));
}

QTEST_MAIN(TestRichSubseq)
#include "TestRichSubseq.moc"
