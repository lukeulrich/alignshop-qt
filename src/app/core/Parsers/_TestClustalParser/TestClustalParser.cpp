/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../ClustalParser.h"
#include "../../PODs/SequenceParseResultPod.h"

class TestClustalParser : public QObject
{
    Q_OBJECT

private slots:
    void isCompatibleString();

    void parseFile_withErrors_data();
    void parseFile_withErrors();

    void parseStream_withErrors_data();
    void parseStream_withErrors();

    void parseStream_data();
    void parseStream();

    void benchNoRegex();
    void benchRegex();
};

typedef QVector<SimpleSeqPod> SimpleSeqPodVector;

Q_DECLARE_METATYPE(SimpleSeqPodVector);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestClustalParser::isCompatibleString()
{
    ClustalParser cp;

    // Test: null string
    QCOMPARE(cp.isCompatibleString(QString()), false);

    // Test: empty string
    QCOMPARE(cp.isCompatibleString(""), false);

    // Test: several empty lines
    QCOMPARE(cp.isCompatibleString("\n\n\n"), false);

    // Test: fasta/clustal mixture line
    QCOMPARE(cp.isCompatibleString(">CLUSTAL - multiple sequence alignment\n"), false);

    // Test: first character is not CLUSTAL
    QCOMPARE(cp.isCompatibleString("\n\n    \n\t\r\n\nWow\n"), false);
    QCOMPARE(cp.isCompatibleString("CLUSTA multiple sequence alignment"), false);

    // Test: first non-whitespace string is CLUSTAL but does not occur immediately after newline
    QCOMPARE(cp.isCompatibleString("  CLUSTAL"), false);
    QCOMPARE(cp.isCompatibleString("\n\n CLUSTAL"), false);
    QCOMPARE(cp.isCompatibleString("\r\n\tCLUSTAL"), false);

    // Test: valid clustal header
    QCOMPARE(cp.isCompatibleString("CLUSTAL"), true);
    QCOMPARE(cp.isCompatibleString("CLUSTAL W(1.83) - multiple sequence alignment"), true);

    // Test: valid clustal header and alignment data
    QCOMPARE(cp.isCompatibleString("CLUSTAL\n\nTest1  AT-G\nTest2  ATCG\n"), true);
    QCOMPARE(cp.isCompatibleString("\n\n   \nCLUSTAL\n\nTest1  AT-G\nTest2  ATCG\n"), true);
}

void TestClustalParser::parseFile_withErrors_data()
{
    QTest::addColumn<QString>("clustalFile");

    QTest::newRow("Empty file") << "empty";
    QTest::newRow("Pure whitespace") << "whitespace";

    // Test: CLUSTAL header line problems
    QTest::newRow("Valid alignment without CLUSTAL header line") << "clustal_header_missing.aln";
    QTest::newRow("Valid alignment, mispelled CLUSTAL tag") << "clustal_header_mispelled.aln";
    QTest::newRow("CLUSTAL tag in between alignment blocks") << "clustal_header_wrong_place.aln";
    QTest::newRow("CLUSTAL tag at end of alignment") << "clustal_header_at_eof.aln";
    QTest::newRow("No empty line following CLUSTAL tag") << "clustal_header_without_empty_line.aln";
    QTest::newRow("CLUSTAL tag in identifier") << "clustal_header_in_identifier.aln";

    // Test: Different number of sequences between distinct alignment blocks
    // Two-blocks
    QTest::newRow("Unequal sequence blocks, 2 then 3") << "unequal_seqblock_2-3.aln";
    QTest::newRow("Unequal sequence blocks, 3 then 2") << "unequal_seqblock_3-2.aln";

    // Three blocks
    QTest::newRow("Unequal sequence blocks, 1 - 3 - 3") << "unequal_seqblock_1-3-3.aln";
    QTest::newRow("Unequal sequence blocks, 3 - 2 - 3") << "unequal_seqblock_3-2-3.aln";
    QTest::newRow("Unequal sequence blocks, 3 - 2 - 3b") << "unequal_seqblock_3-2-3b.aln";
    QTest::newRow("Unequal sequence blocks, 3 - 3 - 2") << "unequal_seqblock_3-3-2.aln";

    // Test: Same number of sequences in each block, but some have distinct identifiers (which implies that others
    //       in previous blocks are missing from this block)
    QTest::newRow("No common ids between blocks") << "distinct_ids_no_common.aln";
    QTest::newRow("Distinct ids 1 of 3") << "distinct_ids_1_of_3.aln";
    QTest::newRow("Distinct ids 1 of 3b") << "distinct_ids_1_of_3b.aln";
    QTest::newRow("Distinct ids 1 of 3c") << "distinct_ids_1_of_3c.aln";
    QTest::newRow("Distinct ids 2 of 3") << "distinct_ids_2_of_3.aln";
    QTest::newRow("Distinct ids 2 of 3b") << "distinct_ids_2_of_3b.aln";
    QTest::newRow("Distinct ids 2 of 3c") << "distinct_ids_2_of_3c.aln";

    // Test: misordered ids
    QTest::newRow("Misordered ids between blocks 1-2-3 :: 2-1-3") << "misordered_1.aln";
    QTest::newRow("Misordered ids between blocks 1-2-3 :: 3-2-1") << "misordered_2.aln";

    // Test: Single-block, different sequence lengths lengths
    QTest::newRow("Single block, different sequence lengths: 1 sequence") << "diflen_1block_1seq.aln";
    QTest::newRow("Single block, different sequence lengths: 1 sequence (part b)") << "diflen_1block_1seq_b.aln";
    QTest::newRow("Single block, different sequence lengths: 1 sequence (part c)") << "diflen_1block_1seq_c.aln";

    QTest::newRow("Single block, different sequence lengths: 2 sequences") << "diflen_1block_2seq.aln";
    QTest::newRow("Single block, different sequence lengths: 2 sequences (part b)") << "diflen_1block_2seq_b.aln";
    QTest::newRow("Single block, different sequence lengths: 2 sequences (part c)") << "diflen_1block_2seq_c.aln";

    // Test: multi-block, different alignment lengths
    QTest::newRow("Multiple block, different sequence lengths: 1 sequence") << "diflen_2block_1seq.aln";
    QTest::newRow("Multiple block, different sequence lengths: 1 sequence (part b)") << "diflen_2block_1seq_b.aln";
    QTest::newRow("Multiple block, different sequence lengths: 1 sequence (part c)") << "diflen_2block_1seq_c.aln";

    QTest::newRow("Multiple block, different sequence lengths: 2 sequences") << "diflen_2block_2seq.aln";
    QTest::newRow("Multiple block, different sequence lengths: 2 sequences (part b)") << "diflen_2block_2seq_b.aln";
    QTest::newRow("Multiple block, different sequence lengths: 2 sequences (part c)") << "diflen_2block_2seq_c.aln";

    QTest::newRow("Multiple block, different sequence lengths: 1 sequence [alternate]") << "diflen_2block_1seq_alt.aln";
    QTest::newRow("Multiple block, different sequence lengths: 1 sequence [alternate] (part b)") << "diflen_2block_1seq_alt_b.aln";
    QTest::newRow("Multiple block, different sequence lengths: 1 sequence [alternate] (part c)") << "diflen_2block_1seq_alt_c.aln";

    QTest::newRow("Multiple block, different sequence lengths: 2 sequences [alternate]") << "diflen_2block_2seq_alt.aln";
    QTest::newRow("Multiple block, different sequence lengths: 2 sequences [alternate] (part b)") << "diflen_2block_2seq_alt_b.aln";
    QTest::newRow("Multiple block, different sequence lengths: 2 sequences [alternate] (part c)") << "diflen_2block_2seq_alt_c.aln";

    // Test: malformed data files
    QTest::newRow("Malformed - junk data insertion - part 1") << "malformed_1.aln";
    QTest::newRow("Malformed - junk data insertion - part 2") << "malformed_2.aln";
    QTest::newRow("Malformed - junk data insertion - part 3") << "malformed_3.aln";
    QTest::newRow("Malformed - junk data insertion - part 4") << "malformed_4.aln";
    QTest::newRow("Malformed - junk data insertion - part 5") << "malformed_5.aln";
    QTest::newRow("Malformed - junk data insertion - part 6") << "malformed_6.aln";
}

void TestClustalParser::parseFile_withErrors()
{
    QFETCH(QString, clustalFile);
    clustalFile.prepend("files/");
    QVERIFY(QFile::exists(clustalFile));

    ClustalParser cp;

    try
    {
        SequenceParseResultPod resultPod = cp.parseFile(clustalFile);
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }
}

void TestClustalParser::parseStream_withErrors_data()
{
    QTest::addColumn<QString>("input");

    QTest::newRow("no sequences") << "CLUSTAL W(1.83) - multiple sequence alignment\n\n";
    QTest::newRow("Only one sequence") << "CLUSTAL W(1.83) - multiple sequence alignment\n\n1\tABC\n\n1\tD-F\n";
}

void TestClustalParser::parseStream_withErrors()
{
    QFETCH(QString, input);

    QVERIFY(input.isEmpty() == false);
    QTextStream inputStream(&input);

    ClustalParser cp;
    try
    {
        SequenceParseResultPod resultPod = cp.parseStream(inputStream, input.size());
        QVERIFY(0);
    }
    catch (...)
    {
        QVERIFY(1);
    }
}

void TestClustalParser::parseStream_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<SimpleSeqPodVector>("simpleSeqPods");

    QTest::newRow("Single block, no numbers or tabs")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1\tABC\n"
               "2\tA-C\n"
               "3\t--C\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABC")
                << SimpleSeqPod("2", "A-C")
                << SimpleSeqPod("3", "--C"));

    QTest::newRow("Spaces instead of tabs")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC\n"
               "2   A-C\n"
               "3   --C\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABC")
                << SimpleSeqPod("2", "A-C")
                << SimpleSeqPod("3", "--C"));

    QTest::newRow("2 blocks, spaces instead of tabs")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC\n"
               "2   A-C\n"
               "3   --C\n"
               "\n"
               "1   DEF\n"
               "2   -E-\n"
               "3   D--\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABCDEF")
                << SimpleSeqPod("2", "A-C-E-")
                << SimpleSeqPod("3", "--CD--"));

    QTest::newRow("2 blocks, spaces in one block, tabs in the other")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1\tABC\n"
               "2\tA-C\n"
               "3\t--C\n"
               "\n"
               "1   DEF\n"
               "2   -E-\n"
               "3   D--\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABCDEF")
                << SimpleSeqPod("2", "A-C-E-")
                << SimpleSeqPod("3", "--CD--"));

    QTest::newRow("2 blocks, arbitrary, unusual spacing between identifier and alignment")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1\t ABC\n"
               "2\t   A-C\n"
               "3 --C\n"
               "\n"
               "1       DEF\n"
               "2  \t \t -E-\n"
               "3           D--\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABCDEF")
                << SimpleSeqPod("2", "A-C-E-")
                << SimpleSeqPod("3", "--CD--"));

    QTest::newRow("2 blocks, spaces within alignment")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   A BC \n"
               "2   A-  C\n"
               "3  --C\n"
               "\n"
               "1   D E F\n"
               "2   -E -\n"
               "3   D- -\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABCDEF")
                << SimpleSeqPod("2", "A-C-E-")
                << SimpleSeqPod("3", "--CD--"));

    QTest::newRow("2 blocks, numbers at end of alignment")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC 234\n"
               "2   A-C 012\n"
               "3   --C 678\n"
               "\n"
               "1   DEF 9\n"
               "2   -E-\n"
               "3   D-- 00030203401\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABCDEF")
                << SimpleSeqPod("2", "A-C-E-")
                << SimpleSeqPod("3", "--CD--"));

    QTest::newRow("2 blocks, spaces inside alignment and numbers at end of alignment")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   A BC 12\n"
               "2   A-  C 34\n"
               "3   --C  56\n"
               "\n"
               "1     DEF 78\n"
               "2   -E-\n"
               "3   D  -  - 90\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABCDEF")
                << SimpleSeqPod("2", "A-C-E-")
                << SimpleSeqPod("3", "--CD--"));

    QTest::newRow("5 blocks")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC\n"
               "2   A-C\n"
               "3   --C\n"
               "\n"
               "1   DEF\n"
               "2   -E-\n"
               "3   D--\n"
               "\n"
               "1   ---\n"
               "2   GH-\n"
               "3   G-I\n"
               "\n"
               "1   JKLMNO\n"
               "2   J--M-O\n"
               "3   --LMNO\n"
               "\n"
               "1   P\n"
               "2   -\n"
               "3   P\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABCDEF---JKLMNOP")
                << SimpleSeqPod("2", "A-C-E-GH-J--M-O-")
                << SimpleSeqPod("3", "--CD--G-I--LMNOP"));

    QTest::newRow("4 blocks, single letter per block")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   A\n"
               "2   A\n"
               "3   -\n"
               "\n"
               "1   D\n"
               "2   -\n"
               "3   D\n"
               "\n"
               "1   -\n"
               "2   G\n"
               "3   G\n"
               "\n"
               "1   J\n"
               "2   J\n"
               "3   -\n"
               "\n"
               "1   P\n"
               "2   -\n"
               "3   P\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "AD-JP")
                << SimpleSeqPod("2", "A-GJ-")
                << SimpleSeqPod("3", "-DG-P"));

    QTest::newRow("1 block")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   A\n"
               "2   A\n"
               "3   -\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "A")
                << SimpleSeqPod("2", "A")
                << SimpleSeqPod("3", "-"));

    QTest::newRow("1 block, no newline at end of file")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   A\n"
               "2   A\n"
               "3   -"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "A")
                << SimpleSeqPod("2", "A")
                << SimpleSeqPod("3", "-"));

    QTest::newRow("1 block, fancy ids, many newlines at end of file")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1234_345-234_Paer   A\n"
               "567_12-200_Ecol   B\n"
               "890_456-323_Rleg   -\n\n\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1234_345-234_Paer", "A")
                << SimpleSeqPod("567_12-200_Ecol", "B")
                << SimpleSeqPod("890_456-323_Rleg", "-"));

    QTest::newRow("1 block with consensus line")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC---DEF\n"
               "2   A-C----EF\n"
               "3   AB----DE-\n"
               "    *:   .: *\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABC---DEF")
                << SimpleSeqPod("2", "A-C----EF")
                << SimpleSeqPod("3", "AB----DE-"));

    QTest::newRow("2 blocks separated by consensus line and newline")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC---DEF\n"
               "2   A-C----EF\n"
               "3   AB----DE-\n"
               "    *:   .: *\n\n"
               "1   GH-\n"
               "2   G-I\n"
               "3   GHI\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABC---DEFGH-")
                << SimpleSeqPod("2", "A-C----EFG-I")
                << SimpleSeqPod("3", "AB----DE-GHI"));

    QTest::newRow("2 blocks both with consensus lines")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC---DEF\n"
               "2   A-C----EF\n"
               "3   AB----DE-\n"
               "    *:   .: *\n\n"
               "1   GH-\n"
               "2   G-I\n"
               "3   GHI\n"
               "    *:.\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABC---DEFGH-")
                << SimpleSeqPod("2", "A-C----EFG-I")
                << SimpleSeqPod("3", "AB----DE-GHI"));

    QTest::newRow("2 blocks separated by many newlines and whitespace")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC---DEF\n"
               "2   A-C----EF\n"
               "3   AB----DE-\n"
               "     \n    \n\n\n  \n"
               "1   GH-\n"
               "2   G-I\n"
               "3   GHI\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABC---DEFGH-")
                << SimpleSeqPod("2", "A-C----EFG-I")
                << SimpleSeqPod("3", "AB----DE-GHI"));

    QTest::newRow("2 blocks with duplicate identifiers")
            << "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
               "1   ABC---DEF\n"
               "1   ABC-----F\n"
               "2   A-C----EF\n"
               "3   AB----DE-\n"
               "\n"
               "1   GH-\n"
               "1   G--\n"
               "2   G-I\n"
               "3   GHI\n"
            << (SimpleSeqPodVector()
                << SimpleSeqPod("1", "ABC---DEFGH-")
                << SimpleSeqPod("1", "ABC-----FG--")
                << SimpleSeqPod("2", "A-C----EFG-I")
                << SimpleSeqPod("3", "AB----DE-GHI"));
}

void TestClustalParser::parseStream()
{
    QFETCH(QString, input);
    QFETCH(SimpleSeqPodVector, simpleSeqPods);

    QVERIFY(input.isEmpty() == false);
    QTextStream inputStream(&input);

    ClustalParser cp;
    try
    {
        SequenceParseResultPod resultPod = cp.parseStream(inputStream, input.size());
        QCOMPARE(resultPod.grammar_, eUnknownGrammar);
        QCOMPARE(resultPod.isAlignment_, eTrue);
        QCOMPARE(resultPod.simpleSeqPods_, simpleSeqPods);
    }
    catch (...)
    {
        QVERIFY(0);
    }
}

void TestClustalParser::benchNoRegex()
{
    QString filename = "files/1_tarhs.aln";
    ClustalParser parser;

    QBENCHMARK {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QTextStream input(&file);

        parser.parseSimpleSeqPodsNoRegex(input, file.size());

        file.close();
    }
}

void TestClustalParser::benchRegex()
{
    QString filename = "files/1_tarhs.aln";
    ClustalParser parser;

    QBENCHMARK {
        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        QTextStream input(&file);

        parser.parseSimpleSeqPodsRegex(input, file.size());

        file.close();
    }
}

QTEST_APPLESS_MAIN(TestClustalParser)
#include "TestClustalParser.moc"
