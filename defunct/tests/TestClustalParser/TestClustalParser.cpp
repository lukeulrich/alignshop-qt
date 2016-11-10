/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QFile>
#include <QList>
#include <QTextStream>
#include <QtTest/QtTest>

#include "ClustalParser.h"
#include "SimpleSeq.h"
#include "SimpleSeqParser.h"
#include "ParseError.h"

class TestClustalParser : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructorEmpty();
    void constructorCopy();

    // ------------------------------------------------------------------------------------------------
    // Assignment operators
    void assignOther();

    // ------------------------------------------------------------------------------------------------
    // Public methods

    // Invalidity tests
    void readAll_empty();
    void readAll_clustal_header();
    void readAll_clustal_header_part2();
    void readAll_nosequences();
    void readAll_min_two_sequences();
    void readAll_unequal_sequence_blocks();
    void readAll_distinct_ids_in_blocks();
    void readAll_misordered();
    void readAll_diflen_intra_block();
    void readAll_malformed();
//    void readAll_inconsistent_spacing();

    // Make sure we are reading the data properly
    void readAll();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestClustalParser::constructorEmpty()
{
    ClustalParser cp;
}

void TestClustalParser::constructorCopy()
{
    ClustalParser cp1;
    ClustalParser cp2(cp1);
}

void TestClustalParser::assignOther()
{
    ClustalParser cp1;
    ClustalParser cp2;

    cp1 = cp2;
}

void TestClustalParser::readAll_empty()
{
    ClustalParser cp;
    QStringList files;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    // Test: Completely empty or whitespace input - should throw ParseError
    files.append("empty");                                  // Completely empty file
    files.append("whitespace");                             // File containing just whitespace

    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);
        QVERIFY(cp.readAll(stream, sseqs, pe) == false);
        QVERIFY(pe.message() == "empty file");

        file.close();
    }
}


void TestClustalParser::readAll_clustal_header()
{
    ClustalParser cp;
    QStringList files;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    // Test: missing CLUSTAL header line
    files.append("clustal_header_missing.aln");          // Valid alignment without Clustal header line
    files.append("clustal_header_mispelled.aln");        // Valid alignment but mispelled CLUSTAL in header line
    files.append("clustal_header_wrong_place.aln");      // Clustal header line in between alignment blocks
    files.append("clustal_header_at_eof.aln");           // Clustal header line at the very end of alignment

    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);

        QVERIFY2(cp.readAll(stream, sseqs, pe) == false, QString("readAll did not throw missing clustal header error. File: %1").arg(files.at(i)).toAscii());
        QVERIFY2(pe.message() == "missing or invalid CLUSTAL header line", QString("readAll gave unexpected error. File: %1").arg(files.at(i)).toAscii());

        file.close();
    }
}

void TestClustalParser::readAll_clustal_header_part2()
{
    ClustalParser cp;
    QStringList files;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    // Test: missing CLUSTAL header line
    files.append("clustal_header_without_empty_line.aln");
    files.append("clustal_header_in_identifier.aln");    // Clustal word in sequence identifier but no header line

    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);

        QVERIFY2(cp.readAll(stream, sseqs, pe) == false, QString("readAll did not throw blank line error. File: %1").arg(files.at(i)).toAscii());
        QVERIFY2(pe.message() == "blank line must immediately follow the CLUSTAL header line", QString("readAll gave unexpected error. File: %1").arg(files.at(i)).toAscii());

        file.close();
    }
}

void TestClustalParser::readAll_nosequences()
{
    ClustalParser cp;
    QString input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n";
    QList<SimpleSeq> sseqs;
    ParseError pe;

    QTextStream stream(&input);

    QVERIFY2(cp.readAll(stream, sseqs, pe) == false, "readAll did not throw no sequence error");
    QVERIFY(pe.message() == "no sequences found");
}

void TestClustalParser::readAll_min_two_sequences()
{
    ClustalParser cp;
    QString input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n1\tABC\n\n1\tD-F\n";
    QList<SimpleSeq> sseqs;
    ParseError pe;

    QTextStream stream(&input);

    QVERIFY2(cp.readAll(stream, sseqs, pe) == false, "readAll did not throw minimum two sequences error");
    QVERIFY(pe.message() == "alignment must have more than one sequence");
}

void TestClustalParser::readAll_unequal_sequence_blocks()
{
    ClustalParser cp;
    QStringList files;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    // Test: Different number of sequences between distinct alignment blocks
    // Two-blocks
    files.append("unequal_seqblock_2-3.aln");       // First with 2 sequences, second with 3 sequences
    files.append("unequal_seqblock_3-2.aln");       // First with 3 sequences, second with 2 sequences

    // Three blocks
    files.append("unequal_seqblock_1-3-3.aln");
    files.append("unequal_seqblock_3-2-3.aln");
    files.append("unequal_seqblock_3-2-3b.aln");
    files.append("unequal_seqblock_3-3-2.aln");

    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);

        QVERIFY2(cp.readAll(stream, sseqs, pe) == false, QString("readAll did not throw unequal sequence block error. File: %1").arg(files.at(i)).toAscii());
        QVERIFY(pe.message() == "unequal number of sequences between blocks");

        file.close();
    }
}

void TestClustalParser::readAll_distinct_ids_in_blocks()
{
    ClustalParser cp;
    QStringList files;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    // Test: Same number of sequences in each block, but some have distinct identifiers (which implies that others
    //       in previous blocks are missing from this block)
    files.append("distinct_ids_no_common.aln");
    files.append("distinct_ids_1_of_3.aln");
    files.append("distinct_ids_1_of_3b.aln");
    files.append("distinct_ids_1_of_3c.aln");
    files.append("distinct_ids_2_of_3.aln");
    files.append("distinct_ids_2_of_3b.aln");
    files.append("distinct_ids_2_of_3c.aln");

    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);

        QVERIFY2(cp.readAll(stream, sseqs, pe) == false, QString("readAll did not throw distinct id error. File: %1").arg(files.at(i)).toAscii());
        QVERIFY(pe.message() == "found sequence identifiers in current block that are distinct from previous block(s)");

        file.close();
    }
}

void TestClustalParser::readAll_misordered()
{
    ClustalParser cp;
    QStringList files;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    files.append("misordered_1.aln");
    files.append("misordered_2.aln");
    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);

        QVERIFY2(cp.readAll(stream, sseqs, pe) == false, QString("readAll did not throw misordered sequence error. File: %1").arg(files.at(i)).toAscii());
        QVERIFY(pe.message() == "sequence identifiers ordered differently from previous blocks");

        file.close();
    }
}

void TestClustalParser::readAll_diflen_intra_block()
{
    ClustalParser cp;
    QStringList files;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    // Test: Single-block, different alignment lengths
    files.append("diflen_1block_1seq.aln");
    files.append("diflen_1block_1seq_b.aln");
    files.append("diflen_1block_1seq_c.aln");

    files.append("diflen_1block_2seq.aln");
    files.append("diflen_1block_2seq_b.aln");
    files.append("diflen_1block_2seq_c.aln");

    // Test: multi-block, different alignment lengths
    files.append("diflen_2block_1seq.aln");
    files.append("diflen_2block_1seq_b.aln");
    files.append("diflen_2block_1seq_c.aln");

    files.append("diflen_2block_2seq.aln");
    files.append("diflen_2block_2seq_b.aln");
    files.append("diflen_2block_2seq_c.aln");

    files.append("diflen_2block_1seq_alt.aln");
    files.append("diflen_2block_1seq_alt_b.aln");
    files.append("diflen_2block_1seq_alt_c.aln");

    files.append("diflen_2block_2seq_alt.aln");
    files.append("diflen_2block_2seq_alt_b.aln");
    files.append("diflen_2block_2seq_alt_c.aln");

    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);

        QVERIFY2(cp.readAll(stream, sseqs, pe) == false, QString("readAll did not throw alignment length error. File: %1").arg(files.at(i)).toAscii());
        QVERIFY(pe.message() == "alignments within block do not all have the same length");

        file.close();
    }
}

/*
void TestClustalParser::readAll_inconsistent_spacing()
{
    ClustalParser cp;
    QStringList files;

    files.append("inconsistent_spacing_1.aln");
    files.append("inconsistent_spacing_2.aln");
    files.append("inconsistent_spacing_3.aln");
    files.append("inconsistent_spacing_4.aln");
    files.append("inconsistent_spacing_5.aln");
    files.append("inconsistent_spacing_6.aln");

    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);

        try
        {
            cp.readAll(stream);
            QVERIFY2(0, QString("readAll did not throw inconsistent spacing exception. File: %1").arg(files.at(i)).toAscii());
        }
        catch (ParserException &e)
        {
            QVERIFY(QString(e.what()) == "inconsistent spacing between sequence identifier and start of alignment");
//            QVERIFY(e.columnNumber() == -1);
//            QVERIFY(e.lineNumber() == -1);
//            QVERIFY(e.line().isEmpty());
        }

        file.close();
    }
}
*/

void TestClustalParser::readAll_malformed()
{
    ClustalParser cp;
    QStringList files;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    files.append("malformed_1.aln");
    files.append("malformed_2.aln");
    files.append("malformed_3.aln");
    files.append("malformed_4.aln");
    files.append("malformed_5.aln");
    files.append("malformed_6.aln");

    for (int i=0, z=files.size(); i<z; ++i)
    {
        QFile file(QString("files/") + files.at(i));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            QVERIFY2(0, QString("Unable to open file %1 for testing").arg(files.at(i)).toAscii());
            continue;
        }

        QTextStream stream(&file);

        QVERIFY2(cp.readAll(stream, sseqs, pe) == false, QString("readAll did not throw malformed error. File: %1").arg(files.at(i)).toAscii());
        QVERIFY(pe.message() == "malformed alignment line");

        file.close();
    }
}

void TestClustalParser::readAll()
{
    ClustalParser cp;
    QList<SimpleSeq> sseqs;
    ParseError pe;

    // ------------------------------------------------------------------------
    // Test: normal alignment, single block, no numbers, tabs
    QString input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
                    "1\tABC\n"
                    "2\tA-C\n"
                    "3\t--C\n";
    QTextStream stream(&input);

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABC");
    QVERIFY2(sseqs.at(1).header() == "2", QString("Result: %1").arg(sseqs.at(1).header()).toAscii());
    QVERIFY(sseqs.at(1).sequence() == "A-C");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--C");

    // ------------------------------------------------------------------------
    // Test: Normal alignment spaces instead of tabs
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   ABC\n"
            "2   A-C\n"
            "3   --C\n";
    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABC");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--C");

    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks, spaces instead of tabs
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   ABC\n"
            "2   A-C\n"
            "3   --C\n"
            "\n"
            "1   DEF\n"
            "2   -E-\n"
            "3   D--\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABCDEF");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C-E-");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--CD--");

    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks, spaces in one block, tabs in the other
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1\tABC\n"
            "2\tA-C\n"
            "3\t--C\n"
            "\n"
            "1   DEF\n"
            "2   -E-\n"
            "3   D--\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABCDEF");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C-E-");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--CD--");

    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks, arbitrary, unusual spacing between identifier and alignment
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1\t ABC\n"
            "2\t   A-C\n"
            "3 --C\n"
            "\n"
            "1       DEF\n"
            "2  \t \t -E-\n"
            "3           D--\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABCDEF");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C-E-");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--CD--");

    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks, spaces within alignment
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   A BC \n"
            "2   A-  C\n"
            "3  --C\n"
            "\n"
            "1   D E F\n"
            "2   -E -\n"
            "3   D- -\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABCDEF");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C-E-");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--CD--");

    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks, numbers at end of alignment
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   ABC 234\n"
            "2   A-C 012\n"
            "3   --C 678\n"
            "\n"
            "1   DEF 9\n"
            "2   -E-\n"
            "3   D-- 00030203401\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABCDEF");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C-E-");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--CD--");

    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks, spaces inside alignment and numbers at end of line
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   A BC 12\n"
            "2   A-  C 34\n"
            "3   --C  56\n"
            "\n"
            "1     DEF 78\n"
            "2   -E-\n"
            "3   D  -  - 90\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABCDEF");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C-E-");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--CD--");


    // ------------------------------------------------------------------------
    // Test: Normal alignment, 5 blocks
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
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
            "3   P\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABCDEF---JKLMNOP");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C-E-GH-J--M-O-");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "--CD--G-I--LMNOP");

    // ------------------------------------------------------------------------
    // Test: Normal alignment, 4 blocks, single letter per block
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
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
            "3   P\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "AD-JP");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-GJ-");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "-DG-P");


    // ------------------------------------------------------------------------
    // Test: Normal alignment, 1 block
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   A\n"
            "2   A\n"
            "3   -\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "A");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "-");

    // ------------------------------------------------------------------------
    // Test: Normal alignment, 1 block, no newline at end of file
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   A\n"
            "2   A\n"
            "3   -";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "A");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "-");


    // ------------------------------------------------------------------------
    // Test: Normal alignment, 1 block, fancy identifiers, many newlines at end
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1234_345-234_Paer   A\n"
            "567_12-200_Ecol   B\n"
            "890_456-323_Rleg   -\n\n\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1234_345-234_Paer");
    QVERIFY(sseqs.at(0).sequence() == "A");
    QVERIFY(sseqs.at(1).header() == "567_12-200_Ecol");
    QVERIFY(sseqs.at(1).sequence() == "B");
    QVERIFY(sseqs.at(2).header() == "890_456-323_Rleg");
    QVERIFY(sseqs.at(2).sequence() == "-");


    // ------------------------------------------------------------------------
    // Test: Normal alignment, 1 block, with consensus line
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   ABC---DEF\n"
            "2   A-C----EF\n"
            "3   AB----DE-\n"
            "    *:   .: *\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABC---DEF");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C----EF");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "AB----DE-");


    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks separated by consensus line and newline
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   ABC---DEF\n"
            "2   A-C----EF\n"
            "3   AB----DE-\n"
            "    *:   .: *\n\n"
            "1   GH-\n"
            "2   G-I\n"
            "3   GHI\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABC---DEFGH-");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C----EFG-I");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "AB----DE-GHI");


    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks separated by consensus line and extra line
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   ABC---DEF\n"
            "2   A-C----EF\n"
            "3   AB----DE-\n"
            "    *:   .: *\n\n"
            "1   GH-\n"
            "2   G-I\n"
            "3   GHI\n"
            "    *:.\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABC---DEFGH-");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C----EFG-I");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "AB----DE-GHI");


    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks separated by many newlines and whitespace
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   ABC---DEF\n"
            "2   A-C----EF\n"
            "3   AB----DE-\n"
            "     \n    \n\n\n  \n"
            "1   GH-\n"
            "2   G-I\n"
            "3   GHI\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 3);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABC---DEFGH-");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "A-C----EFG-I");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "AB----DE-GHI");


    // ------------------------------------------------------------------------
    // Test: Normal alignment, 2 blocks with duplicate identifiers
    stream.seek(0);
    input = "CLUSTAL W(1.83) - multiple sequence alignment\n\n"
            "1   ABC---DEF\n"
            "1   ABC-----F\n"
            "2   A-C----EF\n"
            "3   AB----DE-\n"
            "\n"
            "1   GH-\n"
            "1   G--\n"
            "2   G-I\n"
            "3   GHI\n";

    QVERIFY(cp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 4);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABC---DEFGH-");
    QVERIFY(sseqs.at(1).header() == "1");
    QVERIFY(sseqs.at(1).sequence() == "ABC-----FG--");
    QVERIFY(sseqs.at(2).header() == "2");
    QVERIFY(sseqs.at(2).sequence() == "A-C----EFG-I");
    QVERIFY(sseqs.at(3).header() == "3");
    QVERIFY(sseqs.at(3).sequence() == "AB----DE-GHI");
}

QTEST_MAIN(TestClustalParser)
#include "TestClustalParser.moc"
