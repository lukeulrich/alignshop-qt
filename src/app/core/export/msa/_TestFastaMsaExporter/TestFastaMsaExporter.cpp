/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QBuffer>
#include <QtCore/QIODevice>

#include "../../../Entities/AminoMsa.h"
#include "../../../Entities/AminoSeq.h"
#include "../../../Entities/Astring.h"
#include "../../../ObservableMsa.h"
#include "../FastaMsaExporter.h"

class TestFastaMsaExporter : public QObject
{
    Q_OBJECT

private slots:
    void exportMsa();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestFastaMsaExporter::exportMsa()
{
    FastaMsaExporter x;

    QBuffer buffer;
    buffer.open(QIODevice::WriteOnly);

    // ---------------------------
    // Test: Empty msa
    AminoMsa aminoMsa(4, "pas", "PAS domains", QString());

    try
    {
        x.exportMsa(aminoMsa, buffer);
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // ---------------------------
    // Test: non-null ObservableMsa, but no sequences
    ObservableMsa *msa = new ObservableMsa(eAminoGrammar);
    aminoMsa.setMsa(msa);
    try
    {
        buffer.close();
        buffer.setData(QByteArray());
        buffer.open(QIODevice::WriteOnly);
        x.exportMsa(aminoMsa, buffer);
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // ---------------------------
    // Test: one sequence - but without any sequence entity
    Seq seq1("PKKLRQQD", eAminoGrammar);
    Subseq *subseq1 = new Subseq(seq1);
    msa->append(subseq1);

    try
    {
        buffer.close();
        buffer.setData(QByteArray());
        buffer.open(QIODevice::WriteOnly);
        x.exportMsa(aminoMsa, buffer);
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // ---------------------------
    // Test: one sequence with sequence entity
    msa->clear();
    subseq1 = new Subseq(seq1);
    AstringSPtr astring1(new Astring(1, seq1));
    AminoSeqSPtr aminoSeq1(AminoSeq::createEntity("ABQ11269.1", astring1));
    subseq1->seqEntity_ = aminoSeq1;
    msa->append(subseq1);

    try
    {
        buffer.close();
        buffer.setData(QByteArray());
        buffer.open(QIODevice::WriteOnly);
        x.exportMsa(aminoMsa, buffer);
        QCOMPARE(buffer.data(), QByteArray(">ABQ11269.1\nPKKLRQQD\n"));
    }
    catch (...)
    {
        QVERIFY(0);
    }

    // ---------------------------
    // Test: two sequences one without a sequence entity
    msa->clear();
    subseq1 = new Subseq(seq1);
    Seq seq2("PPELK", eAminoGrammar);
    Subseq *subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("PPELK---"));
    subseq1->seqEntity_ = aminoSeq1;
    QVERIFY(msa->append(subseq1));
    QVERIFY(msa->append(subseq2));
    try
    {
        buffer.close();
        buffer.setData(QByteArray());
        buffer.open(QIODevice::WriteOnly);
        x.exportMsa(aminoMsa, buffer);
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }

    // ---------------------------
    // Test: two sequences both with sequence entities
    msa->clear();
    subseq1 = new Subseq(seq1);
    subseq2 = new Subseq(seq2);
    QVERIFY(subseq2->setBioString("PPELK---"));
    subseq1->seqEntity_ = aminoSeq1;
    AstringSPtr astring2(new Astring(2, seq2));
    AminoSeqSPtr aminoSeq2(AminoSeq::createEntity("YP_001135808.1", astring2));
    subseq2->seqEntity_ = aminoSeq2;
    QVERIFY(msa->append(subseq1));
    QVERIFY(msa->append(subseq2));

    try
    {
        buffer.close();
        buffer.setData(QByteArray());
        buffer.open(QIODevice::WriteOnly);
        x.exportMsa(aminoMsa, buffer);
        QCOMPARE(buffer.data(), QByteArray(">ABQ11269.1\n"
                                           "PKKLRQQD\n"
                                           ">YP_001135808.1\n"
                                           "PPELK---\n"));
    }
    catch (...)
    {
        QVERIFY(0);
    }
}


QTEST_APPLESS_MAIN(TestFastaMsaExporter)
#include "TestFastaMsaExporter.moc"
