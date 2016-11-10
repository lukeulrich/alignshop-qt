/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../FastaParser.h"
#include "../../PODs/SequenceParseResultPod.h"

class TestFastaParser : public QObject
{
    Q_OBJECT

private slots:
    void isCompatibleString();
    void parseStream_data();
    void parseStream();
};

typedef QVector<SimpleSeqPod> SimpleSeqPodVector;

Q_DECLARE_METATYPE(TriBool);
Q_DECLARE_METATYPE(SimpleSeqPodVector);

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestFastaParser::isCompatibleString()
{
    FastaParser fp;

    // Test: null string
    QCOMPARE(fp.isCompatibleString(QString()), false);

    // Test: empty string
    QCOMPARE(fp.isCompatibleString(""), false);

    // Test: several empty lines
    QCOMPARE(fp.isCompatibleString("\n\n\n"), false);

    // Test: clustal header line
    QCOMPARE(fp.isCompatibleString("CLUSTAL W(1.83) - multiple sequence alignment\n"), false);

    // Test: first character is not >
    QCOMPARE(fp.isCompatibleString("\n\n    \n\t\r\n\nWow\n"), false);

    // Test: first non-whitespace character is > but does not occur immediately after newline
    QCOMPARE(fp.isCompatibleString("  >"), false);
    QCOMPARE(fp.isCompatibleString("\n\n >"), false);
    QCOMPARE(fp.isCompatibleString("\r\n\t>\nATG\n"), false);

    // Test: valid fasta header
    QCOMPARE(fp.isCompatibleString(">test sequence"), true);
    QCOMPARE(fp.isCompatibleString(">test sequence\n"), true);

    // Test: valid fasta header and sequence(s)
    QCOMPARE(fp.isCompatibleString(">test sequence\nATGC\n"), true);
    QCOMPARE(fp.isCompatibleString(">test sequence\nATGC\n>test sequence2\nATGC"), true);
}

void TestFastaParser::parseStream_data()
{
    QTest::addColumn<QString>("sample");
    QTest::addColumn<TriBool>("isParseSuccessful");
    QTest::addColumn<SimpleSeqPodVector>("simpleSeqPods");

    QTest::newRow("Empty text")                                     << QString() << eTrue << SimpleSeqPodVector();
    QTest::newRow("Whitespace before the > symbol, no sequence")    << "  >My header" << eFalse << SimpleSeqPodVector();
    QTest::newRow("Only the > symbol")                              << ">" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod());
    QTest::newRow("Junk before > symbol")                           << "a>" << eFalse << SimpleSeqPodVector();
    QTest::newRow("Leading whitespace before valid sequence")       << "\t\r\v\f\n \n>Header\nABC" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header", "ABC"));
    QTest::newRow("Pure header")                                    << ">Header" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header"));
    QTest::newRow("Pure header containing additional > symbols")    << ">>Header>text" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod(">Header>text"));
    QTest::newRow("Pure header, leading and terminal whitespace")   << ">  Header  " << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header"));
    QTest::newRow("Header and newline, but no sequence data")       << ">Header\n" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header"));
    QTest::newRow("No terminal newline after sequence")             << ">Header\nABC" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header", "ABC"));
    QTest::newRow("Terminal newline after sequence")                << ">Header\nABC\n" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header", "ABC"));
    QTest::newRow("> embedded in sequence")                         << ">Header\nABC\nDEF>\n >GHI" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header", "ABCDEF>>GHI"));
    QTest::newRow(">\\n>")                                          << ">\n>" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod() << SimpleSeqPod());
    QTest::newRow(">\\n>\\n")                                       << ">\n>\n" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod() << SimpleSeqPod());
    QTest::newRow(">\\n>\\n>\\n>\\n")                               << ">\n>\n>\n>\n" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod() << SimpleSeqPod() << SimpleSeqPod() << SimpleSeqPod());
    QTest::newRow("Multiple sequences without terminal newline")    << ">123\nABC\n>456\nDEF" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("123", "ABC") << SimpleSeqPod("456", "DEF"));
    QTest::newRow("Multiple sequences with terminal newline")       << ">123\nABC\n>456\nDEF" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("123", "ABC") << SimpleSeqPod("456", "DEF"));
    QTest::newRow("Empty header and valid sequence; vice versa")    << ">\nABC\n>456\n" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("", "ABC") << SimpleSeqPod("456"));
    QTest::newRow("Multiple terminal newline in sequence")          << ">123\nABC\n>456\n\n" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("123", "ABC") << SimpleSeqPod("456"));
    QTest::newRow("Embedded whitespace in sequence")                << ">Header\n\nA\tB\fC\rD\vE  F\n>Header2" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header", "ABCDEF") << SimpleSeqPod("Header2"));
    QTest::newRow("Several normal sequences")                       << ">1\nA\n>2\nB\n>3\nC\n>4\nD\n>5\nE\n" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("1", "A") << SimpleSeqPod("2", "B") << SimpleSeqPod("3", "C") << SimpleSeqPod("4", "D") << SimpleSeqPod("5", "E"));
    QTest::newRow("Multi-line sequence data")                       << ">Header\nABC\nDEF\nGHI\n>Header2\nJKL\n" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("Header", "ABCDEFGHI") << SimpleSeqPod("Header2", "JKL"));
    QTest::newRow("Two sequences with empty sequence")              << ">ABC\n\n>" << eTrue << (SimpleSeqPodVector() << SimpleSeqPod("ABC") << SimpleSeqPod());
}

void TestFastaParser::parseStream()
{
    QFETCH(QString, sample);
    QFETCH(TriBool, isParseSuccessful);
    QFETCH(SimpleSeqPodVector, simpleSeqPods);

    QTextStream stream(&sample, QIODevice::ReadOnly);

    FastaParser fp;
    SequenceParseResultPod resultPod;

    try
    {
        resultPod = fp.parseStream(stream, sample.size());
        QCOMPARE(isParseSuccessful, eTrue);
        QCOMPARE(resultPod.grammar_, eUnknownGrammar);
        QCOMPARE(resultPod.isAlignment_, eUnknown);
        QCOMPARE(resultPod.simpleSeqPods_, simpleSeqPods);
    }
    catch(...)
    {
        QCOMPARE(isParseSuccessful, eFalse);
    }
}


QTEST_APPLESS_MAIN(TestFastaParser)
#include "TestFastaParser.moc"
