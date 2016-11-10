/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../Seq.h"

class TestSeq : public QObject
{
    Q_OBJECT

private slots:
    void constructor_BioString();
    void constructor_strGram();
    void constructor_byteGram();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestSeq::constructor_BioString()
{
    Seq seq(BioString("ABC...DEF---", eRnaGrammar));
    QCOMPARE(seq.grammar(), eRnaGrammar);
    QCOMPARE(seq.constData(), "ABCDEF");

    Seq seq2(BioString("-1-2-3-", eAminoGrammar));
    QCOMPARE(seq2.grammar(), eAminoGrammar);
    QCOMPARE(seq2.constData(), "123");
}

void TestSeq::constructor_strGram()
{
    Seq seq("--AB--DE..", eAminoGrammar);
    QCOMPARE(seq.grammar(), eAminoGrammar);
    QCOMPARE(seq.constData(), "ABDE");

    Seq seq2("-1-2-3-", eUnknownGrammar);
    QCOMPARE(seq2.grammar(), eUnknownGrammar);
    QCOMPARE(seq2.constData(), "123");

    const char *seq_str = "ABCD";
    Seq seq3(seq_str);
    QCOMPARE(seq3.grammar(), eUnknownGrammar);
    QCOMPARE(seq3.constData(), seq_str);
}

void TestSeq::constructor_byteGram()
{
    Seq seq(QByteArray("--AB--DE.."), eAminoGrammar);
    QCOMPARE(seq.grammar(), eAminoGrammar);
    QCOMPARE(seq.constData(), "ABDE");

    Seq seq2(QByteArray("-1-2-3-"), eUnknownGrammar);
    QCOMPARE(seq2.grammar(), eUnknownGrammar);
    QCOMPARE(seq2.constData(), "123");
}

QTEST_APPLESS_MAIN(TestSeq)
#include "TestSeq.moc"
