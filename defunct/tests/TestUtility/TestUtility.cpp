/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "utility.h"

class TestUtility : public QObject
{
    Q_OBJECT

private slots:
    void portableFileName_data();
    void portableFileName();
};

void TestUtility::portableFileName_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<bool>("expectedBool");

    QTest::newRow("Empty filename") << QString() << false;
    QTest::newRow("Filename with 1 space") << " " << false;
    QTest::newRow("Filename with 2 spaces") << "  " << false;
    QTest::newRow("Filename with 3 spaces") << "   " << false;
    QTest::newRow("Filename with 4 spaces") << "    " << false;

    QTest::newRow("First character may not be space: domains") << " domains" << false;

    QTest::newRow("First character may not be period: .") << "." << false;
    QTest::newRow("First character may not be period: .genome") << ".genome" << false;
    QTest::newRow("First character may not be period: .db") << ".db" << false;

    QTest::newRow("First character may not be hyphen: -") << "-" << false;
    QTest::newRow("First character may not be hyphen: -genome") << "-genome" << false;
    QTest::newRow("First character may not be hyphen: -db") << "-db" << false;

    QTest::newRow("Ends with space: 'domains '") << "domains " << false;
    QTest::newRow("First character period, ends with space: '.domains '") << ".domains " << false;
    QTest::newRow("First character hyphen, ends with space: '-domains '") << "-domains " << false;
    QTest::newRow("First character space, ends with space: ' domains '") << " domains " << false;
    QTest::newRow("First character space, second character period: ' .'") << " ." << false;
    QTest::newRow("First character period, second character space: '. '") << ". " << false;
    QTest::newRow("Two periods") << ".." << false;

    QTest::newRow("Slashes not allowed: /") << "/" << false;
    QTest::newRow("Slashes not allowed: \\") << "\\" << false;
    QTest::newRow("Slashes not allowed: \\domains") << "\\domains" << false;
    QTest::newRow("Slashes not allowed: /domains") << "/domains" << false;
    QTest::newRow("Slashes not allowed: ./domains") << "./domains" << false;
    QTest::newRow("Slashes not allowed: ./domains/") << "./domains/" << false;
    QTest::newRow("Slashes not allowed: domains/") << "domains/" << false;
    QTest::newRow("Slashes not allowed: /\\/") << "/\\/" << false;

    QString invalidCharacters = "`\"'~!@#$%^&*()+=[]{}|,<>;:";
    const QChar *x = invalidCharacters.constData();
    while (*x != '\0')
    {
        QTest::newRow(QString("Single invalid letter: %1").arg(*x).toAscii()) << QString(*x) << false;
        ++x;
    }

    QString validCharacters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_";
    x = validCharacters.constData();
    while (*x != '\0')
    {
        QTest::newRow(QString("Single valid letter: %1").arg(*x).toAscii()) << QString(*x) << true;
        ++x;
    }

    QTest::newRow("Period after underscore: _.") << "_." << true;
    QTest::newRow("Hyphen after underscore: _-") << "_-" << true;
    QTest::newRow("Internal spaces: 'pas domains'") << "pas domains" << true;

    QTest::newRow("Valid spot check: 'Quroum-sensing domains.db'") << "Quroum-sensing domains.db" << true;
    QTest::newRow("Valid spot check: 'Metagenomic data.xls'") << "Metagenomic data.xls" << true;
    QTest::newRow("Valid spot check: schlack-uniref90.11Jun10") << "schlack-uniref90.11Jun10" << true;
}

void TestUtility::portableFileName()
{
    QFETCH(QString, fileName);
    QFETCH(bool, expectedBool);

    QCOMPARE(::portableFileName(fileName), expectedBool);
}

QTEST_APPLESS_MAIN(TestUtility)
#include "TestUtility.moc"
