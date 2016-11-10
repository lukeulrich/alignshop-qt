/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtSql/QSqlField>

#include "AminoString.h"
#include "AnonSeq.h"
#include "DnaString.h"
#include "Msa.h"
#include "RichMsa.h"
#include "RichSubseq.h"

class TestRichMsa: public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorBasic();

    // ------------------------------------------------------------------------------------------------
    // Public members
    void annotation();
    void operator_paren();
    void at();
    void append();
    void insert();
    void prepend();
};

void TestRichMsa::constructorBasic()
{
    RichMsa richMsa1(eUnknownAlphabet, 10);
    RichMsa richMsa2(eAminoAlphabet, 20);
    RichMsa richMsa3(eDnaAlphabet, 30);
    RichMsa richMsa4(eRnaAlphabet, 40);

    QVERIFY(richMsa1.annotation_.isEmpty());
    QVERIFY(richMsa2.annotation_.isEmpty());
    QVERIFY(richMsa3.annotation_.isEmpty());
    QVERIFY(richMsa4.annotation_.isEmpty());
}

void TestRichMsa::annotation()
{
    RichMsa richMsa(eAminoAlphabet, 10);

    richMsa.annotation_.append(QSqlField("name", QVariant::String));
    richMsa.annotation_.setValue("name", "LuxR");
    QCOMPARE(richMsa.annotation_.value("name").toString(), QString("LuxR"));
}

void TestRichMsa::operator_paren()
{
    AnonSeq anonSeq(1, AminoString("ABC...DEF"));
    RichSubseq *rs1 = new RichSubseq(anonSeq);

    AnonSeq anonSeq2(2, AminoString("GHI---JKL"));
    RichSubseq *rs2 = new RichSubseq(anonSeq2);

    AnonSeq anonSeq3(3, AminoString("MNO---QRS"));
    RichSubseq *rs3 = new RichSubseq(anonSeq3);

    RichMsa richMsa(eAminoAlphabet, 1);

    QVERIFY(richMsa.append(rs1));
    QVERIFY(richMsa.append(rs3));
    QVERIFY(richMsa.append(rs2));

    QCOMPARE(richMsa(1), rs1);
    QCOMPARE(richMsa(2), rs3);
    QCOMPARE(richMsa(3), rs2);
}

void TestRichMsa::at()
{
    AnonSeq anonSeq(1, AminoString("ABC...DEF"));
    RichSubseq *rs1 = new RichSubseq(anonSeq);

    AnonSeq anonSeq2(2, AminoString("GHI---JKL"));
    RichSubseq *rs2 = new RichSubseq(anonSeq2);

    AnonSeq anonSeq3(3, AminoString("MNO---QRS"));
    RichSubseq *rs3 = new RichSubseq(anonSeq3);

    RichMsa richMsa(eAminoAlphabet, 1);

    QVERIFY(richMsa.append(rs1));
    QVERIFY(richMsa.append(rs3));
    QVERIFY(richMsa.append(rs2));

    QCOMPARE(richMsa.at(1), rs1);
    QCOMPARE(richMsa.at(2), rs3);
    QCOMPARE(richMsa.at(3), rs2);

    QCOMPARE(richMsa.at(-3), rs1);
    QCOMPARE(richMsa.at(-2), rs3);
    QCOMPARE(richMsa.at(-1), rs2);
}

void TestRichMsa::append()
{
    AnonSeq anonSeq(1, AminoString("ABC...DEF"));
    Subseq subseq1(anonSeq);
    RichSubseq *rs1 = new RichSubseq(anonSeq);

    AnonSeq anonSeq2(2, AminoString("GHI---JKL"));
    Subseq subseq2(anonSeq2);
    RichSubseq *rs2 = new RichSubseq(anonSeq2);

    AnonSeq anonSeq3(3, AminoString("MNO---QRS"));
    Subseq subseq3(anonSeq3);
    RichSubseq *rs3 = new RichSubseq(anonSeq3);

    rs1->annotation_.append(QSqlField("name", QVariant::String));
    rs1->annotation_.setValue("source", "E. coli");

    RichMsa richMsa(eAminoAlphabet, 1);

    // Test: append rich subseq
    QVERIFY(richMsa.append(rs1));
    QCOMPARE(richMsa.subseqCount(), 1);
    QCOMPARE(richMsa(1), rs1);

    // Test: append another rich subseq
    QVERIFY(richMsa.append(rs2));
    QCOMPARE(richMsa.subseqCount(), 2);
    QCOMPARE(richMsa(2), rs2);

    // Test: appending rich subseqs from base class pointer
    Msa *msa = &richMsa;
    QVERIFY(msa->append(rs3) == false);
    QCOMPARE(richMsa.subseqCount(), 2);

    // Test: appending normal subseq via base class pointer returns false
    QVERIFY(msa->append(&subseq1) == false);
    QVERIFY(msa->append(&subseq2) == false);
    QVERIFY(msa->append(&subseq3) == false);
    QCOMPARE(richMsa.subseqCount(), 2);
}

void TestRichMsa::insert()
{
    AnonSeq anonSeq(1, AminoString("ABC...DEF"));
    Subseq subseq1(anonSeq);
    RichSubseq *rs1 = new RichSubseq(anonSeq);

    AnonSeq anonSeq2(2, AminoString("GHI---JKL"));
    Subseq subseq2(anonSeq2);
    RichSubseq *rs2 = new RichSubseq(anonSeq2);

    AnonSeq anonSeq3(3, AminoString("MNO---QRS"));
    Subseq subseq3(anonSeq3);
    RichSubseq *rs3 = new RichSubseq(anonSeq3);

    AnonSeq anonSeq4(4, AminoString("TUVWXY"));
    RichSubseq *rs4 = new RichSubseq(anonSeq4);

    AnonSeq anonSeq5(5, AminoString("ABCXYZ"));
    RichSubseq *rs5 = new RichSubseq(anonSeq5);

    RichMsa richMsa(eAminoAlphabet, 1);

    // Test: insert rich subseq into empty list
    QVERIFY(richMsa.insert(1, rs1));
    QCOMPARE(richMsa.subseqCount(), 1);
    QCOMPARE(richMsa(1), rs1);

    // Test: insert rich subseq at beginning
    QVERIFY(richMsa.insert(1, rs2));
    QCOMPARE(richMsa.subseqCount(), 2);
    QCOMPARE(richMsa(1), rs2);

    // Test: insert rich subseq in middle of list
    QVERIFY(richMsa.insert(2, rs3));
    QCOMPARE(richMsa.subseqCount(), 3);
    QCOMPARE(richMsa(1), rs2);
    QCOMPARE(richMsa(2), rs3);
    QCOMPARE(richMsa(3), rs1);

    // Test: insert rich subseq at end of list
    QVERIFY(richMsa.insert(4, rs4));
    QCOMPARE(richMsa.subseqCount(), 4);
    QCOMPARE(richMsa(1), rs2);
    QCOMPARE(richMsa(2), rs3);
    QCOMPARE(richMsa(3), rs1);
    QCOMPARE(richMsa(4), rs4);

    // Test: appending rich subseqs from base class pointer
    Msa *msa = &richMsa;
    QVERIFY(msa->insert(1, rs5) == false);
    QCOMPARE(richMsa.subseqCount(), 4);

    // Test: appending normal subseq via base class pointer returns false
    QVERIFY(msa->insert(1, &subseq1) == false);
    QVERIFY(msa->insert(2, &subseq2) == false);
    QVERIFY(msa->insert(3, &subseq3) == false);
    QCOMPARE(richMsa.subseqCount(), 4);
    QCOMPARE(richMsa(1), rs2);
    QCOMPARE(richMsa(2), rs3);
    QCOMPARE(richMsa(3), rs1);
    QCOMPARE(richMsa(4), rs4);
}

void TestRichMsa::prepend()
{
    AnonSeq anonSeq(1, DnaString("ACG...TG"));
    Subseq subseq1(anonSeq);
    RichSubseq *rs1 = new RichSubseq(anonSeq);

    AnonSeq anonSeq2(2, DnaString("ACC...TG"));
    Subseq subseq2(anonSeq2);
    RichSubseq *rs2 = new RichSubseq(anonSeq2);

    AnonSeq anonSeq3(3, AminoString("GGC---AA"));
    Subseq subseq3(anonSeq3);
    RichSubseq *rs3 = new RichSubseq(anonSeq3);

    RichMsa richMsa(eDnaAlphabet, 1);

    // Test: prepend rich subseq
    QVERIFY(richMsa.prepend(rs1));
    QCOMPARE(richMsa.subseqCount(), 1);

    // Test: prepend another rich subseq
    QVERIFY(richMsa.prepend(rs2));
    QCOMPARE(richMsa.subseqCount(), 2);

    // Test: prepending rich subseqs from base class pointer
    Msa *msa = &richMsa;
    QVERIFY(msa->prepend(rs3) == false);
    QCOMPARE(richMsa.subseqCount(), 2);

    // Test: prepending normal subseq via base class pointer returns false
    QVERIFY(msa->prepend(&subseq1) == false);
    QVERIFY(msa->prepend(&subseq2) == false);
    QVERIFY(msa->prepend(&subseq3) == false);
    QCOMPARE(richMsa.subseqCount(), 2);
}

QTEST_MAIN(TestRichMsa)
#include "TestRichMsa.moc"
