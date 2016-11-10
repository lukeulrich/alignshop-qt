/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../PrimerPairFinder.h"
#include "../PrimerSearchParameters.h"

class TestPrimerPairFinder : public QObject
{
    Q_OBJECT

private slots:
    void findPrimerPairs();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPrimerPairFinder::findPrimerPairs()
{
    PrimerSearchParameters psp;

    // 918 long
    BioString dnaString("ATGAATATTCGTGATCTTGAGTACCTGGTGGCATTGGCTGAACACCGCCATTTTCGGCGTGCGGCAGATTCCTGCCACGTTAGCCAGCCGACGCTTAGCGGGCAAATTCGTAAGCTGGAAGATGAGTTGGGCGTGATGTTGCTGGAGCGGACCAGCCGTAAAGTGTTGTTCACCCAGGCGGGAATGCTGCTGGTGGATCAGGCGCGTACCGTGCTGCGTGAGGTGAAAGTCCTTAAAGAGATGGCAAGCCAGCAGGGCGAGACGATGTCCGGACCGCTGCATATTGGTTTGATTCCCACGGTTGGACCGTACCTGCTACCGCATATTATCCCGATGCTGCACCAGACCTTTCCAAAGCTGGAAATGTATCTGCATGAAGCACAGACCCACCAGTTACTGGCGCAACTGGACAGTGGCAAACTCGATTGCGTGATCCTCGCGCTGGTGAAAGAGAGCGAAGCATTCATTGAAGTGCCGTTGTTTGATGAGCCAATGTTGCTGGCTATCTATGAAGATCACCCGTGGGCGAACCGCGAATGCGTACCGATGGCCGATCTGGCAGGGGAAAAACTGCTGATGCTGGAAGATGGTCACTGTTTGCGCGATCAGGCAATGGGTTTCTGCTTTGAAGCCGGGGCGGATGAAGATACACACTTCCGCGCGACCAGCCTGGAGACACTGCGTAACATGGTGGCGGCAGGTAGCGGGATCACTTTACTGCCTGCGCTGGCTGTGCCGCCGGAGCGCAAACGCGATGGGGTTGTTTATTTGCCGTGTATTAAGCCGGAACCTCGCCGCACGATTGGCCTGGTTTATCGTCCTGGCTCACCGCTGCGCAGCCGCTATGAGCAGCTGGCAGAGGCCATCCGCGCAAGAATGGATGGCCATTTCGATAAAGTGTTAAAACAGGCGGTTTAA", eDnaGrammar);

    // 888 -> 918
    psp.ampliconLengthRange_.begin_ = 700 - 30;
    psp.ampliconLengthRange_.end_ = 700;

    PrimerPairFinder primerPairFinder;

    QVector<PrimerPair> primerPairs = primerPairFinder.findPrimerPairs(dnaString, ClosedIntRange(118, 818), psp);
    qDebug() << primerPairs.size();
}


QTEST_APPLESS_MAIN(TestPrimerPairFinder)
#include "TestPrimerPairFinder.moc"
