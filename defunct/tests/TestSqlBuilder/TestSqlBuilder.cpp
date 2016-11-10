/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "SqlBuilder.h"

#include <QtCore/QDebug>

class TestSqlBuilder : public QObject
{
    Q_OBJECT

private slots:
    void preparedInsertSql_data();
    void preparedInsertSql();

    void preparedUpdateSql_data();
    void preparedUpdateSql();
};

void TestSqlBuilder::preparedInsertSql_data()
{
    QTest::addColumn<QString>("tableName");
    QTest::addColumn<QStringList>("fields");
    QTest::addColumn<QString>("result");

    // Test: invalid cases
    QTest::newRow("empty input")  << "" << QStringList() << "";
    QTest::newRow("empty table")  << "" << (QStringList() << "id" << "sequence") << "";
    QTest::newRow("empty fields") << "astrings" << QStringList() << "";
    QTest::newRow("blank table")  << "  " << (QStringList() << "id") << "";
    QTest::newRow("blank field")  << "astrings" << (QStringList() << "  ") << "";
    QTest::newRow("mixed fields") << "astrings" << (QStringList() << "id" << "  ") << "";

    // Test: valid cases
    QTest::newRow("single field") << "names" << (QStringList() << "first") << "INSERT INTO names(first) VALUES (?)";
    QTest::newRow("two fields")   << "astrings" << (QStringList() << "sequence" << "digest") << "INSERT INTO astrings(sequence, digest) VALUES (?, ?)";
    QTest::newRow("four fields")  << "organisms" << (QStringList() << "id" << "genus" << "species" << "google_rank") << "INSERT INTO organisms(id, genus, species, google_rank) VALUES (?, ?, ?, ?)";
    QTest::newRow("space table")  << "  bacteria" << (QStringList() << "pathogen") << "INSERT INTO bacteria(pathogen) VALUES (?)";
    QTest::newRow("space table2") << "bacteria  " << (QStringList() << "pathogen") << "INSERT INTO bacteria(pathogen) VALUES (?)";
    QTest::newRow("space table3")  << "  bacteria  " << (QStringList() << "pathogen") << "INSERT INTO bacteria(pathogen) VALUES (?)";
}

void TestSqlBuilder::preparedInsertSql()
{
    QFETCH(QString, tableName);
    QFETCH(QStringList, fields);
    QFETCH(QString, result);

    // Test: static method
    QCOMPARE(SqlBuilder::preparedInsertSql(tableName, fields), result);

    // Test: object method
    SqlBuilder sqlBuilder;
    QCOMPARE(sqlBuilder.preparedInsertSql(tableName, fields), result);
}

void TestSqlBuilder::preparedUpdateSql_data()
{
    QTest::addColumn<QString>("tableName");
    QTest::addColumn<QStringList>("fields");
    QTest::addColumn<QString>("conditions");
    QTest::addColumn<QString>("result");

    // Test: invalid cases
    QTest::newRow("empty input")  << "" << QStringList() << "" << "";
    QTest::newRow("empty table")  << "" << (QStringList() << "sequence") << "id = ?" << "";
    QTest::newRow("empty fields") << "astrings" << QStringList() << "WHERE id = ?" << "";
    QTest::newRow("blank table")  << "  " << (QStringList() << "sequence") << "id = ?" << "";
    QTest::newRow("blank table2") << "  " << (QStringList() << "sequence") << "" << "";
    QTest::newRow("blank field")  << "astrings" << (QStringList() << "  ") << "id = ?" << "";
    QTest::newRow("blank field2") << "astrings" << (QStringList() << "  ") << "" << "";
    QTest::newRow("mixed fields") << "astrings" << (QStringList() << "sequence" << " " << "digest") << "" << "";

    // Test: valid cases
    QTest::newRow("single field") << "names" << (QStringList() << "first") << "id = ?" << "UPDATE names SET first = ? WHERE id = ?";
    QTest::newRow("two fields")   << "astrings" << (QStringList() << "sequence" << "digest") << "id < 50" << "UPDATE astrings SET sequence = ?, digest = ? WHERE id < 50";
    QTest::newRow("four fields")  << "organisms" << (QStringList() << "id" << "genus" << "species" << "google_rank") << "id = ? AND genus != 'Escherichia'" << "UPDATE organisms SET id = ?, genus = ?, species = ?, google_rank = ? WHERE id = ? AND genus != 'Escherichia'";
    QTest::newRow("no condition") << "names" << (QStringList() << "first") << "" << "UPDATE names SET first = ?";
    QTest::newRow("space table")  << "  bacteria" << (QStringList() << "pathogen") << "" << "UPDATE bacteria SET pathogen = ?";
    QTest::newRow("space table2") << "bacteria  " << (QStringList() << "pathogen") << "id = ?" << "UPDATE bacteria SET pathogen = ? WHERE id = ?";
    QTest::newRow("space table3") << "  bacteria  " << (QStringList() << "pathogen") << "dormant = true" << "UPDATE bacteria SET pathogen = ? WHERE dormant = true";
}

void TestSqlBuilder::preparedUpdateSql()
{
    QFETCH(QString, tableName);
    QFETCH(QStringList, fields);
    QFETCH(QString, conditions);
    QFETCH(QString, result);

    // Test: static method
    QCOMPARE(SqlBuilder::preparedUpdateSql(tableName, fields, conditions), result);

    // Test: object method
    SqlBuilder sqlBuilder;
    QCOMPARE(sqlBuilder.preparedUpdateSql(tableName, fields, conditions), result);
}

QTEST_MAIN(TestSqlBuilder)
#include "TestSqlBuilder.moc"
