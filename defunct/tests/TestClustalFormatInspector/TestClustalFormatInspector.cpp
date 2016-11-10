/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QSharedPointer>

#include "ClustalFormatInspector.h"

class TestClustalFormatInspector : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void inspect();
};

void TestClustalFormatInspector::inspect()
{
    ClustalFormatInspector inspector;

    // Test: null string
    QCOMPARE(inspector.inspect(QString()), eUnknownFormatType);

    // Test: empty string
    QCOMPARE(inspector.inspect(""), eUnknownFormatType);

    // Test: several empty lines
    QCOMPARE(inspector.inspect("\n\n\n"), eUnknownFormatType);

    // Test: fasta/clustal mixture line
    QCOMPARE(inspector.inspect(">CLUSTAL - multiple sequence alignment\n"), eUnknownFormatType);

    // Test: first character is not CLUSTAL
    QCOMPARE(inspector.inspect("\n\n    \n\t\r\n\nWow\n"), eUnknownFormatType);
    QCOMPARE(inspector.inspect("CLUSTA multiple sequence alignment"), eUnknownFormatType);

    // Test: first non-whitespace string is CLUSTAL but does not occur immediately after newline
    QCOMPARE(inspector.inspect("  CLUSTAL"), eUnknownFormatType);
    QCOMPARE(inspector.inspect("\n\n CLUSTAL"), eUnknownFormatType);
    QCOMPARE(inspector.inspect("\r\n\tCLUSTAL"), eUnknownFormatType);

    // Test: valid clustal header
    QCOMPARE(inspector.inspect("CLUSTAL"), eClustalType);
    QCOMPARE(inspector.inspect("CLUSTAL W(1.83) - multiple sequence alignment"), eClustalType);

    // Test: valid clustal header and alignment data
    QCOMPARE(inspector.inspect("CLUSTAL\n\nTest1  AT-G\nTest2  ATCG\n"), eClustalType);
    QCOMPARE(inspector.inspect("\n\n   \nCLUSTAL\n\nTest1  AT-G\nTest2  ATCG\n"), eClustalType);
}


QTEST_MAIN(TestClustalFormatInspector)
#include "TestClustalFormatInspector.moc"
