/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QList>
#include <QtTest/QtTest>

#include "FastaParser.h"
#include "SimpleSeq.h"
#include "ParseError.h"

class TestFastaParser : public QObject
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
    void readAll();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestFastaParser::constructorEmpty()
{
    FastaParser fp;
}

void TestFastaParser::constructorCopy()
{
    FastaParser fp1;
    FastaParser fp2(fp1);
}

void TestFastaParser::assignOther()
{
    FastaParser fp1;
    FastaParser fp2;

    fp1 = fp2;
}

void TestFastaParser::readAll()
{
    QString sample = "";
    QTextStream stream(&sample, QIODevice::ReadOnly);

    ParseError pe;
    FastaParser fp;

    // Test: Read completely empty text
    QList<SimpleSeq> sseqs;

    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 0);

    // Test: Whitespace between newline character and first > symbol
    stream.seek(0);
    sample = "  >My header";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 0);

    // Test: Fasta text with just the > symbol
    stream.seek(0);
    sample = ">";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header().isEmpty());
    QVERIFY2(sseqs.at(0).sequence().isEmpty(), QString("Result: %1").arg(sseqs.at(0).sequence()).toAscii());

    // Test: junk before newline character and first > symbol
    stream.seek(0);
    sample = " abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()>";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 0);

    // Test: junk before newline character and first > symbol and then the first newline > combination
    stream.seek(0);
    sample = " abcdefghijklmnopqrstuvwxyz1234567890!@#$%^&*()>\n>";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header().isEmpty());
    QVERIFY(sseqs.at(0).sequence().isEmpty());

    // Test: Fasta text with just header and no newline
    stream.seek(0);
    sample = ">My header";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY2(sseqs.at(0).header() == "My header", QString("Result: %1").arg(sseqs.at(0).header()).toAscii());
    QVERIFY(sseqs.at(0).sequence().isEmpty());

    // Test: Fasta text with just header (which includes other > symbols) and no newline
    stream.seek(0);
    sample = ">>ABC>def";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header() == ">ABC>def");
    QVERIFY(sseqs.at(0).sequence().isEmpty());

    // Test: Fasta text with just header, terminal whitespaces and no newline
    stream.seek(0);
    sample = ">My header  \t\r";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header() == "My header  \t\r");
    QVERIFY(sseqs.at(0).sequence().isEmpty());

    // Test: Fasta text with header and newline, but no sequence data
    stream.seek(0);
    sample = ">My header\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header() == "My header");
    QVERIFY(sseqs.at(0).sequence().isEmpty());

    // Test: Fasta text with header, terminal whitespace, and newline, but no sequence data
    stream.seek(0);
    sample = ">My header\t \r\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header() == "My header\t \r");
    QVERIFY(sseqs.at(0).sequence().isEmpty());

    // Test: Fasta text with header, newline and sequence data but no newline terminated sequence (EOF simulation with
    //       no trailing newline)
    stream.seek(0);
    sample = ">My header\nABC";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header() == "My header");
    QVERIFY(sseqs.at(0).sequence() == "ABC");

    // Test: Fasta text with header, newline, sequence, and sequence newline
    stream.seek(0);
    sample = ">My header\nABC\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header() == "My header");
    QVERIFY2(sseqs.at(0).sequence() == "ABC", QString("Result: %1").arg(sseqs.at(0).sequence()).toAscii());

    // Test: Fasta text with header, newline, sequence, and sequence newline, and > symbol but not at beginning of line
    stream.seek(0);
    sample = ">My header\nABC\nDEF>\n >GHI";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header() == "My header");
    QVERIFY2(sseqs.at(0).sequence() == "ABC\nDEF>\n >GHI", QString("Result: %1").arg(sseqs.at(0).sequence()).toAscii());

    // Tests: Fasta text with multiple empty sequences and headers
    stream.seek(0);
    sample = ">\n>";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header().isEmpty());
    QVERIFY(sseqs.at(0).sequence().isEmpty());
    QVERIFY(sseqs.at(1).header().isEmpty());
    QVERIFY(sseqs.at(1).sequence().isEmpty());

    stream.seek(0);
    sample = ">\n>\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header().isEmpty());
    QVERIFY(sseqs.at(0).sequence().isEmpty());
    QVERIFY(sseqs.at(1).header().isEmpty());
    QVERIFY(sseqs.at(1).sequence().isEmpty());

    stream.seek(0);
    sample = ">\n>\n>\n>\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 4);
    QVERIFY(sseqs.at(0).header().isEmpty());
    QVERIFY(sseqs.at(0).sequence().isEmpty());
    QVERIFY(sseqs.at(1).header().isEmpty());
    QVERIFY(sseqs.at(1).sequence().isEmpty());
    QVERIFY(sseqs.at(2).header().isEmpty());
    QVERIFY(sseqs.at(2).sequence().isEmpty());
    QVERIFY(sseqs.at(3).header().isEmpty());
    QVERIFY(sseqs.at(3).sequence().isEmpty());

    // Test: Multiple normal sequences without terminal newline
    stream.seek(0);
    sample = ">123\nABC\n>456\nDEF";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header() == "123");
    QVERIFY(sseqs.at(0).sequence() == "ABC");
    QVERIFY(sseqs.at(1).header() == "456");
    QVERIFY(sseqs.at(1).sequence() == "DEF");

    // Test: same as above except with terminal newline
    stream.seek(0);
    sample = ">123\nABC\n>456\nDEF\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header() == "123");
    QVERIFY(sseqs.at(0).sequence() == "ABC");
    QVERIFY(sseqs.at(1).header() == "456");
    QVERIFY(sseqs.at(1).sequence() == "DEF");

    // Test: Empty header but valid sequence
    stream.seek(0);
    sample = ">\nABC\n>456\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header().isEmpty());
    QVERIFY(sseqs.at(0).sequence() == "ABC");
    QVERIFY(sseqs.at(1).header() == "456");
    QVERIFY(sseqs.at(1).sequence().isEmpty());

    // Test: multi-terminal newline in sequence
    stream.seek(0);
    sample = ">123\nABC\n>456\n\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header() == "123");
    QVERIFY(sseqs.at(0).sequence() == "ABC");
    QVERIFY(sseqs.at(1).header() == "456");
    QVERIFY(sseqs.at(1).sequence().isEmpty());

    stream.seek(0);
    sample = ">123\nABC\n>456\n\n\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header() == "123");
    QVERIFY(sseqs.at(0).sequence() == "ABC");
    QVERIFY(sseqs.at(1).header() == "456");
    QVERIFY(sseqs.at(1).sequence() == "\n");

    stream.seek(0);
    sample = ">123\nABC\n>456\n\n\n\n\n\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header() == "123");
    QVERIFY(sseqs.at(0).sequence() == "ABC");
    QVERIFY(sseqs.at(1).header() == "456");
    QVERIFY(sseqs.at(1).sequence() == "\n\n\n\n");

    // Test: several normal sequences
    stream.seek(0);
    sample = ">1\nA\n>2\nB\n>3\nC\n>4\nD\n>5\nE\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 5);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "A");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "B");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "C");
    QVERIFY(sseqs.at(3).header() == "4");
    QVERIFY(sseqs.at(3).sequence() == "D");
    QVERIFY(sseqs.at(4).header() == "5");
    QVERIFY(sseqs.at(4).sequence() == "E");

    // Test: same as above except with leading junk
    stream.seek(0);
    sample = "#this is some junk\nstuff at\ntop\n123434 of line\n>1\nA\n>2\nB\n>3\nC\n>4\nD\n>5\nE\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 5);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "A");
    QVERIFY(sseqs.at(1).header() == "2");
    QVERIFY(sseqs.at(1).sequence() == "B");
    QVERIFY(sseqs.at(2).header() == "3");
    QVERIFY(sseqs.at(2).sequence() == "C");
    QVERIFY(sseqs.at(3).header() == "4");
    QVERIFY(sseqs.at(3).sequence() == "D");
    QVERIFY(sseqs.at(4).header() == "5");
    QVERIFY(sseqs.at(4).sequence() == "E");

    // Test: multi-line sequence data
    stream.seek(0);
    sample = ">1\nABC\nDEF\n  GHI--..456\n";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 1);
    QVERIFY(sseqs.at(0).header() == "1");
    QVERIFY(sseqs.at(0).sequence() == "ABC\nDEF\n  GHI--..456");

    // Test: two sequence with empty sequence
    stream.seek(0);
    sample = ">ABC\n\n>";
    QVERIFY(fp.readAll(stream, sseqs, pe));
    QVERIFY(sseqs.count() == 2);
    QVERIFY(sseqs.at(0).header() == "ABC");
    QVERIFY(sseqs.at(0).sequence().isEmpty());
}


QTEST_MAIN(TestFastaParser)
#include "TestFastaParser.moc"

