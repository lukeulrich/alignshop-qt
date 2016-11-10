/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QTest>

#include "models/ParsedBioStringTableModel.h"

#include <QDebug>

class TestParsedBioStringTableModel : public QObject
{
    Q_OBJECT

private slots:
    void parsedBioString_struct();

    void parsedBioStrings();            // Also tests setParsedBioStrings()
    void clear();

    void columnCount();
    void data();
    void flags();
    void headerData();
    void rowCount();
    void setData();
};

void TestParsedBioStringTableModel::parsedBioString_struct()
{
    // Test: default constructor of parsed BioString should properly initialize its members
    ParsedBioString pbs("ABC");
    QCOMPARE(pbs.bioString_.sequence(), QString("ABC"));
    QVERIFY(pbs.header_.isEmpty());
    QVERIFY(pbs.valid_ == false);
    QVERIFY(pbs.checked_ == false);

    ParsedBioString pbs2("DEF", "Dehydrogenase");
    QCOMPARE(pbs2.bioString_.sequence(), QString("DEF"));
    QCOMPARE(pbs2.header_, QString("Dehydrogenase"));
    QVERIFY(pbs2.valid_ == false);
    QVERIFY(pbs2.checked_ == false);

    ParsedBioString pbs3("GHI", "PAS domain", true);
    QCOMPARE(pbs3.bioString_.sequence(), QString("GHI"));
    QCOMPARE(pbs3.header_, QString("PAS domain"));
    QVERIFY(pbs3.valid_);
    QVERIFY(pbs3.checked_ == false);

    ParsedBioString pbs4("JKL", "Quorum sensing", false, true);
    QCOMPARE(pbs4.bioString_.sequence(), QString("JKL"));
    QCOMPARE(pbs4.header_, QString("Quorum sensing"));
    QVERIFY(pbs4.valid_ == false);
    QVERIFY(pbs4.checked_ == true);
}

void TestParsedBioStringTableModel::parsedBioStrings()
{
    ParsedBioStringTableModel model;
    QVERIFY(model.parsedBioStrings().isEmpty());

    model.setParsedBioStrings(QList<ParsedBioString>() << ParsedBioString("ABC") << ParsedBioString("DEF"));
    QCOMPARE(model.parsedBioStrings().count(), 2);
    QCOMPARE(model.parsedBioStrings().at(0).bioString_.sequence(), QString("ABC"));
    QCOMPARE(model.parsedBioStrings().at(1).bioString_.sequence(), QString("DEF"));
}

void TestParsedBioStringTableModel::clear()
{
    ParsedBioStringTableModel model;
    model.setParsedBioStrings(QList<ParsedBioString>() << ParsedBioString("ABC") << ParsedBioString("DEF"));
    QCOMPARE(model.parsedBioStrings().count(), 2);
    model.clear();
    QVERIFY(model.parsedBioStrings().isEmpty());
}

void TestParsedBioStringTableModel::columnCount()
{
    ParsedBioStringTableModel model;
    QCOMPARE(model.columnCount(), 3);
}

void TestParsedBioStringTableModel::data()
{
    ParsedBioStringTableModel model;
    QList<ParsedBioString> pbss;
    pbss.append(ParsedBioString("ABC", "LuxR", true));
    pbss.append(ParsedBioString("DEF", "PAS domain", false));
    pbss.append(ParsedBioString("GHI", "Metagenomics", false, true));
    model.setParsedBioStrings(pbss);

    // Test: first column, check state role
    QVERIFY(model.data(model.index(0, ParsedBioStringTableModel::kLabelColumn), Qt::CheckStateRole) == Qt::Unchecked);
    QVERIFY(model.data(model.index(1, ParsedBioStringTableModel::kLabelColumn), Qt::CheckStateRole) == Qt::Unchecked);
    QVERIFY(model.data(model.index(2, ParsedBioStringTableModel::kLabelColumn), Qt::CheckStateRole) == Qt::Unchecked);  // Special case, setParsedBioStrings should have set checked to false because it's valid state is set to false

    // Test: first column, checkbox, display role
    QCOMPARE(model.data(model.index(0, ParsedBioStringTableModel::kLabelColumn)).toString(), QString("LuxR"));
    QCOMPARE(model.data(model.index(1, ParsedBioStringTableModel::kLabelColumn)).toString(), QString("PAS domain"));
    QCOMPARE(model.data(model.index(2, ParsedBioStringTableModel::kLabelColumn)).toString(), QString("Metagenomics"));

    // Test: second column, integer, display role
    QCOMPARE(model.data(model.index(0, ParsedBioStringTableModel::kSequenceColumn)).toString(), QString("ABC"));
    QCOMPARE(model.data(model.index(1, ParsedBioStringTableModel::kSequenceColumn)).toString(), QString("DEF"));
    QCOMPARE(model.data(model.index(2, ParsedBioStringTableModel::kSequenceColumn)).toString(), QString("GHI"));

    // Test: third column, valid should be yes or no
    QCOMPARE(model.data(model.index(0, ParsedBioStringTableModel::kValidColumn)).toString(), QString("Yes"));
    QCOMPARE(model.data(model.index(1, ParsedBioStringTableModel::kValidColumn)).toString(), QString("No"));
    QCOMPARE(model.data(model.index(2, ParsedBioStringTableModel::kValidColumn)).toString(), QString("No"));
}

void TestParsedBioStringTableModel::flags()
{
    ParsedBioStringTableModel model;

    QList<ParsedBioString> pbss;
    pbss.append(ParsedBioString("ABC", "LuxR", true));
    pbss.append(ParsedBioString("DEF", "PAS domain", false));
    model.setParsedBioStrings(pbss);

//    qDebug() << model.flags(model.index(0, 0));                             // 49
//    qDebug() << (model.flags(model.index(0, 0)) & Qt::ItemIsEnabled);       // 32
//    qDebug() << (model.flags(model.index(0, 1)) & Qt::ItemIsEditable);      // 0

    // Test: first column, checkbox
    QVERIFY((model.flags(model.index(0, ParsedBioStringTableModel::kLabelColumn)) & Qt::ItemIsUserCheckable) == Qt::ItemIsUserCheckable);
    QVERIFY((model.flags(model.index(1, ParsedBioStringTableModel::kLabelColumn)) & Qt::ItemIsUserCheckable) == Qt::ItemIsUserCheckable);

    // Test: first column, label
    QVERIFY((model.flags(model.index(0, ParsedBioStringTableModel::kLabelColumn)) & Qt::ItemIsEnabled) == Qt::ItemIsEnabled);
    QVERIFY((model.flags(model.index(0, ParsedBioStringTableModel::kLabelColumn)) & Qt::ItemIsEditable) == Qt::ItemIsEditable);
    QVERIFY((model.flags(model.index(1, ParsedBioStringTableModel::kLabelColumn)) & Qt::ItemIsEnabled) == 0);       // Because it is not valid
    QVERIFY((model.flags(model.index(1, ParsedBioStringTableModel::kLabelColumn)) & Qt::ItemIsEditable) == Qt::ItemIsEditable);

    // Other columns should not be editable
    QVERIFY((model.flags(model.index(0, ParsedBioStringTableModel::kSequenceColumn)) & Qt::ItemIsEditable) == 0);
    QVERIFY((model.flags(model.index(1, ParsedBioStringTableModel::kSequenceColumn)) & Qt::ItemIsEditable) == 0);

    QVERIFY((model.flags(model.index(0, ParsedBioStringTableModel::kValidColumn)) & Qt::ItemIsEditable) == 0);
    QVERIFY((model.flags(model.index(1, ParsedBioStringTableModel::kValidColumn)) & Qt::ItemIsEditable) == 0);
}

void TestParsedBioStringTableModel::headerData()
{
    ParsedBioStringTableModel model;

    // Test: check horizontal headers
    QCOMPARE(model.headerData(ParsedBioStringTableModel::kLabelColumn, Qt::Horizontal).toString(), QString("Label"));
    QCOMPARE(model.headerData(ParsedBioStringTableModel::kSequenceColumn, Qt::Horizontal).toString(), QString("Sequence"));
    QCOMPARE(model.headerData(ParsedBioStringTableModel::kValidColumn, Qt::Horizontal).toString(), QString("Valid"));

    // Test: vertical headers - should be 1-based numbers
    QList<ParsedBioString> pbss;
    pbss.append(ParsedBioString("ABC", "LuxR", true));
    pbss.append(ParsedBioString("DEF", "PAS domain", false));
    model.setParsedBioStrings(pbss);

    QCOMPARE(model.headerData(0, Qt::Vertical).toInt(), 1);
    QCOMPARE(model.headerData(1, Qt::Vertical).toInt(), 2);
}

void TestParsedBioStringTableModel::rowCount()
{
    ParsedBioStringTableModel model;
    QCOMPARE(model.rowCount(), 0);
    model.setParsedBioStrings(QList<ParsedBioString>() << ParsedBioString("ABC") << ParsedBioString("DEF"));
    QCOMPARE(model.rowCount(), 2);
    model.clear();
    QCOMPARE(model.rowCount(), 0);
}

void TestParsedBioStringTableModel::setData()
{
    ParsedBioStringTableModel model;
    QList<ParsedBioString> pbss;
    pbss.append(ParsedBioString("ABC", "LuxR", true));
    pbss.append(ParsedBioString("DEF", "PAS domain", false));
    model.setParsedBioStrings(pbss);

    QModelIndex row = model.index(0, ParsedBioStringTableModel::kLabelColumn);
    QVERIFY(row.isValid());
    QVERIFY(model.setData(row, "Quorum sensor"));
    QCOMPARE(model.data(row).toString(), QString("Quorum sensor"));

    // Test: valid biostring, uncheck the checkbox
    QVERIFY(model.setData(row, false, Qt::CheckStateRole));
    QVERIFY(model.parsedBioStrings().at(0).checked_ == false);
    QCOMPARE(model.data(row, Qt::CheckStateRole).toBool(), false);

    // Test: valid biostring, check the checkbox
    QVERIFY(model.setData(row, true, Qt::CheckStateRole));
    QVERIFY(model.parsedBioStrings().at(0).checked_);
    QCOMPARE(model.data(row, Qt::CheckStateRole).toBool(), true);

    // Test: invalid biostring, check the checkbox should fail
    row = model.index(1, ParsedBioStringTableModel::kLabelColumn);
    QVERIFY(model.setData(row, true, Qt::CheckStateRole) == false);
    QVERIFY(model.data(row, Qt::CheckStateRole).toBool() == false);

    // Test: invalid biostring, unchecking the checkbox should work
    QVERIFY(model.setData(row, false, Qt::CheckStateRole) == true);
    QVERIFY(model.data(row, Qt::CheckStateRole).toBool() == false);

    // ----------------
    // Test: label should not be able to be changed for both invalid and valid biostrings
    row = model.index(0, ParsedBioStringTableModel::kLabelColumn);
    QVERIFY(model.setData(row, "ABC"));
    QCOMPARE(model.data(row).toString(), QString("ABC"));

    row = model.index(1, ParsedBioStringTableModel::kLabelColumn);
    QVERIFY(model.setData(row, "XYZ"));
    QCOMPARE(model.data(row).toString(), QString("XYZ"));

    // ----------------
    // Test: sequence column is not editable
    row = model.index(0, ParsedBioStringTableModel::kSequenceColumn);
    QVERIFY(model.setData(row, "SEQUENCE") == false);
    QCOMPARE(model.data(row).toString(), QString("ABC"));

    // ----------------
    // Test: valid column
    row = model.index(0, ParsedBioStringTableModel::kValidColumn);
    QVERIFY(model.setData(row, false));
    QCOMPARE(model.data(row).toString(), QString("No"));

    QVERIFY(model.setData(row, true));
    QCOMPARE(model.data(row).toString(), QString("Yes"));

    // Try the same data should still return true
    QVERIFY(model.setData(row, true));
}

QTEST_MAIN(TestParsedBioStringTableModel)
#include "TestParsedBioStringTableModel.moc"
