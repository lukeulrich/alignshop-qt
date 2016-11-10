/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QSharedPointer>

#include "FastaFormatInspector.h"

class TestFastaFormatInspector : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void inspect();
};

void TestFastaFormatInspector::inspect()
{
    FastaFormatInspector inspector;

    // Test: null string
    QCOMPARE(inspector.inspect(QString()), eUnknownFormatType);

    // Test: empty string
    QCOMPARE(inspector.inspect(""), eUnknownFormatType);

    // Test: several empty lines
    QCOMPARE(inspector.inspect("\n\n\n"), eUnknownFormatType);

    // Test: clustal header line
    QCOMPARE(inspector.inspect("CLUSTAL W(1.83) - multiple sequence alignment\n"), eUnknownFormatType);

    // Test: first character is not >
    QCOMPARE(inspector.inspect("\n\n    \n\t\r\n\nWow\n"), eUnknownFormatType);

    // Test: first non-whitespace character is > but does not occur immediately after newline
    QCOMPARE(inspector.inspect("  >"), eUnknownFormatType);
    QCOMPARE(inspector.inspect("\n\n >"), eUnknownFormatType);
    QCOMPARE(inspector.inspect("\r\n\t>\nATG\n"), eUnknownFormatType);

    // Test: valid fasta header
    QCOMPARE(inspector.inspect(">test sequence"), eFastaType);
    QCOMPARE(inspector.inspect(">test sequence\n"), eFastaType);

    // Test: valid fasta header and sequence(s)
    QCOMPARE(inspector.inspect(">test sequence\nATGC\n"), eFastaType);
    QCOMPARE(inspector.inspect(">test sequence\nATGC\n>test sequence2\nATGC"), eFastaType);
}


QTEST_MAIN(TestFastaFormatInspector)
#include "TestFastaFormatInspector.moc"
