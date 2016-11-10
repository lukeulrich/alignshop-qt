/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtSql/QSqlField>
#include <QtSql/QSqlRecord>

#include "DataRow.h"

#include <QtCore/QDebug>

class TestDataRow : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();     // Also tests id, tableName()

    void setValue();
    void setValueInt();
    void fieldNames();
    void remove();

    void copyConstructor();
};

void TestDataRow::constructor()
{
    DataRow dr1;
    QVERIFY(dr1.id_.isNull());

    DataRow dr2(34);
    QCOMPARE(dr2.id_, QVariant(34));
    QVERIFY(dr2.isEmpty());
}

void TestDataRow::setValue()
{
    DataRow dr;
    dr.setValue("id", 35);
    dr.setValue("company", "Agile Genomics, LLC");

    QCOMPARE(dr.value("id").toInt(), 35);
    QCOMPARE(dr.value("company").toString(), QString("Agile Genomics, LLC"));

    dr.setValue("id", 40);
    QCOMPARE(dr.count(), 2);
}

void TestDataRow::setValueInt()
{
    DataRow dr;
    dr.append(QSqlField("id"));
    dr.append(QSqlField("company"));

    QVERIFY(dr.value("id").isNull());
    QVERIFY(dr.value("company").isNull());

    dr.setValue(0, 35);
    dr.setValue(1, "Agile Genomics, LLC");

    QCOMPARE(dr.value("id").toInt(), 35);
    QCOMPARE(dr.value("company").toString(), QString("Agile Genomics, LLC"));

    dr.setValue(0, 40);
    QCOMPARE(dr.count(), 2);

    dr.setValue(2, 50);
    QCOMPARE(dr.count(), 2);
}

void TestDataRow::fieldNames()
{
    DataRow dr;

    dr.setValue("id", 35);
    QVERIFY(dr.fieldNames().count() == 1);
    QCOMPARE(dr.fieldNames().at(0), QString("id"));

    dr.setValue("company", "Agile Genomics, LLC");

    QVERIFY(dr.fieldNames().count() == 2);
    QVERIFY(dr.fieldNames().contains("id"));
    QVERIFY(dr.fieldNames().contains("company"));
}

void TestDataRow::remove()
{
    DataRow dr;

    dr.append(QString("id"));
    dr.remove("id");
    QVERIFY(dr.count() == 0);
}

void TestDataRow::copyConstructor()
{
    QSqlRecord sqlRecord;
    sqlRecord.append(QString("id"));
    sqlRecord.append(QString("company"));
    sqlRecord.setValue("id", 1);
    sqlRecord.setValue("company", "Agile Genomics, LLC");

    DataRow dr(sqlRecord);

    QCOMPARE(dr.value("id").toInt(), 1);
    QCOMPARE(dr.value("company").toString(), QString("Agile Genomics, LLC"));
    QVERIFY(dr.id_.isNull());

    DataRow dr2(30, sqlRecord);
    QCOMPARE(dr2.value("id").toInt(), 1);
    QCOMPARE(dr2.value("company").toString(), QString("Agile Genomics, LLC"));
    QCOMPARE(dr2.id_.toInt(), 30);
}

QTEST_MAIN(TestDataRow)
#include "TestDataRow.moc"
