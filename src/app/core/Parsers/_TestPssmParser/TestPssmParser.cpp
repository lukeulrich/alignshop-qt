/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QFile>
#include <QtTest/QtTest>

#include "../PssmParser.h"
#include "../../PODs/Pssm.h"

class TestPssmParser : public QObject
{
    Q_OBJECT

private slots:
    void parseFileInvalid_data();
    void parseFileInvalid();

    void parseFile();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestPssmParser::parseFileInvalid_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("empty filename") << "";
    QTest::newRow("directory") << ".";
    QTest::newRow("missing file") << "non-existent-file";
    QTest::newRow("file size of zero") << "empty";
    QTest::newRow("file with just whitespace") << "whitespace";
    QTest::newRow("No header line") << "pssm-no_header";
    QTest::newRow("No scale factor") << "pssm-no_scale_factor";
    QTest::newRow("Zero scale factor") << "pssm-zero_scale_factor";
    QTest::newRow("PSSM with 6 score columns") << "pssm-6_columns";
    QTest::newRow("PSSM without any rows") << "pssm-no_rows";
    QTest::newRow("PSSM with one row missing a score column") << "pssm-row_without_column";
    QTest::newRow("Invalid position") << "pssm-non_linear_position";
    QTest::newRow("Word instead of score") << "pssm-word_score";
    QTest::newRow("Duplicate column labels") << "pssm-duplicate_column_labels";
}

void TestPssmParser::parseFileInvalid()
{
    QFETCH(QString, fileName);

    fileName = "files/" + fileName;

    if (fileName != "files/non-existent-file")
        QVERIFY(QFile::exists(fileName));

    PssmParser x;
    try
    {
        x.parseFile(fileName);
        QVERIFY(0);
    }
    catch(...)
    {
        QVERIFY(1);
    }
}

void TestPssmParser::parseFile()
{
    PssmParser x;

    try
    {
        Pssm pssm = x.parseFile("files/ag_pssm");

        QCOMPARE(pssm.positScaleFactor_, 200.);
        QVector<char> expectedMapping;
        expectedMapping << 'A' << 'R' << 'N' << 'D' << 'C' << 'Q' << 'E' << 'G' << 'H' << 'I' << 'L' << 'K' << 'M'
                        << 'F' << 'P' << 'S' << 'T' << 'W' << 'Y' << 'V';
        QCOMPARE(pssm.mapping_, expectedMapping);

        QCOMPARE(pssm.isEmpty(), false);
        QCOMPARE(pssm.rows_.size(), 104);

        // Check the first and last rows
        QVector<int> firstLineScores;
        firstLineScores << -211 << -305 << -476 << -674 << -316 << -98 << -442 << -591 << -345 << 240 << 428 << -302
                        << 1213 << -4 << -547 << -330 << -152 << -317 << -224 << 144;

        for (int i=0; i< firstLineScores.size(); ++i)
            QCOMPARE(firstLineScores.at(i), pssm.rows_.at(0).scores_[i]);

        QVector<int> lastLineScores;
        lastLineScores << -353 << -339 << -416 << -613 << -481 << -284 << -404 << -608 << 339 << -266 << -212 << -364
                       << -199 << 588 << -584 << -337 << -321 << 431 << 1319 << -242;

        for (int i=0; i< lastLineScores.size(); ++i)
            QCOMPARE(lastLineScores.at(i), pssm.rows_.last().scores_[i]);
    }
    catch(...)
    {
        QVERIFY(0);
    }
}

QTEST_APPLESS_MAIN(TestPssmParser)
#include "TestPssmParser.moc"
