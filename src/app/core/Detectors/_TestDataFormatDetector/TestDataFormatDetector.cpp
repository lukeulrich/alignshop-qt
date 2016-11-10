/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../DataFormatDetector.h"
#include "../../Parsers/FastaParser.h"
#include "../../Parsers/ClustalParser.h"

class TestDataFormatDetector : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void dataFormats();     // Also tests setDataFormats
    void formatFromFileExtension();
    void formatFromString();
    void formatFromFile_data();
    void formatFromFile();
};

Q_DECLARE_METATYPE(DataFormat);

void TestDataFormatDetector::dataFormats()
{
    DataFormatDetector detector;

    // Test: default should have no data formats defined
    QVERIFY(detector.dataFormats().isEmpty());

    // Test: should be able to add a data format and get it back
    QVector<DataFormat> formats;
    formats << DataFormat(eFastaFormat, "Fasta");

    detector.setDataFormats(formats);

    QVERIFY(detector.dataFormats().size() == 1);
    QVERIFY(detector.dataFormats().at(0) == formats.at(0));

    // Test: clear out formats via passing empty list
    detector.setDataFormats(QVector<DataFormat>());
    QVERIFY(detector.dataFormats().isEmpty());
    formats.clear();

    // Test: set multiple data formats should be taken
    formats << DataFormat(eFastaFormat, "Fasta")
            <<  DataFormat(eClustalFormat, "Clustal");
    detector.setDataFormats(formats);
    QVERIFY(detector.dataFormats().size() == 2);
    QVERIFY(detector.dataFormats().at(0) == formats.at(0));
    QVERIFY(detector.dataFormats().at(1) == formats.at(1));
}

void TestDataFormatDetector::formatFromFileExtension()
{
    DataFormatDetector detector;

    // Test: default state should return unknown format regardless
    QStringList extensions;
    extensions << "aln" << "fasta" << "bob" << "xls" << "sh" << "" << "none";
    foreach (QString extension, extensions)
        QCOMPARE(detector.formatFromFileExtension(extension), DataFormat());

    // Test: check that registered types are returned appropriately
    QVector<DataFormat> formats;
    formats << DataFormat(eFastaFormat, "Fasta", QStringList() << "faa" << "fa" << "fnt")
            << DataFormat(eClustalFormat, "Clustal", QStringList() << "aln");

    // Test: all registered types
    detector.setDataFormats(formats);
    QCOMPARE(detector.formatFromFileExtension("faa").type(), eFastaFormat);
    QCOMPARE(detector.formatFromFileExtension("fa").type(), eFastaFormat);
    QCOMPARE(detector.formatFromFileExtension("fnt").type(), eFastaFormat);
    QCOMPARE(detector.formatFromFileExtension("aln").type(), eClustalFormat);

    // Test: unregistered extensions
    QCOMPARE(detector.formatFromFileExtension("ph"), DataFormat());
    QCOMPARE(detector.formatFromFileExtension(""), DataFormat());

    // Test: if duplicate extensions are registered should return type of first containing DataFormat
    formats.clear();
    formats << DataFormat(eFastaFormat, "Fasta", QStringList() << "faa" << "fa" << "fnt")
            << DataFormat(eClustalFormat, "Clustal", QStringList() << "aln" << "faa");
    QCOMPARE(detector.formatFromFileExtension("faa").type(), eFastaFormat);
}

void TestDataFormatDetector::formatFromString()
{
    DataFormatDetector detector;

    // Test: default state, shouldn't matter the input, should always return unknown type
    QStringList data_strings;
    data_strings << ">1\nACDEF\n>2\nDEF\n"
                 << ">1...."
                 << ""
                 << "CLUSTAL";
    foreach (QString data_string, data_strings)
        QCOMPARE(detector.formatFromString(data_string), DataFormat());

    // Test: should receive same result if there are no valid AbstractDataFormatInspectors
    QVector<DataFormat> formats;
    formats << DataFormat(eFastaFormat, "Fasta", QStringList() << "faa" << "fa" << "fnt")
            << DataFormat(eClustalFormat, "Clustal", QStringList() << "aln");
    detector.setDataFormats(formats);

    foreach (QString data_string, data_strings)
        QCOMPARE(detector.formatFromString(data_string), DataFormat());

    // Test: same as above, except with valid inspectors
    formats[0].setParser(new FastaParser(this));
    formats[1].setParser(new ClustalParser(this));
    detector.setDataFormats(formats);

    QCOMPARE(detector.formatFromString(">test sequence\nATGC\n").type(), eFastaFormat);
    QCOMPARE(detector.formatFromString("CLUSTAL\n\nTest1  AT-G\nTest2  ATCG\n").type(), eClustalFormat);

    // Test: junk data with valid inspectors
    QCOMPARE(detector.formatFromString("blah blah missing invalid\n\ntype"), DataFormat());

    // Test: Should return first matching inspector result
    formats[1].setParser(new FastaParser(this));
    QCOMPARE(detector.formatFromString(">test sequence\nATGC\n").type(), eFastaFormat);
}

void TestDataFormatDetector::formatFromFile_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<DataFormat>("fastaFormat");
    QTest::addColumn<DataFormat>("clustalFormat");
    QTest::addColumn<DataFormat>("dataFormat1");
    QTest::addColumn<DataFormat>("dataFormat2");

    DataFormat fastaFormat(eFastaFormat, "Fasta", QStringList() << "faa" << "fa" << "fnt", new FastaParser(this));
    DataFormat clustalFormat(eClustalFormat, "Clustal", QStringList() << "aln", new ClustalParser(this));

    QTest::newRow("empty") << "empty" << fastaFormat << clustalFormat << DataFormat() << DataFormat();
    QTest::newRow("empty.fa") << "empty.fa" << fastaFormat << clustalFormat << DataFormat() << fastaFormat;
    QTest::newRow("empty.aln") << "empty.aln" << fastaFormat << clustalFormat << DataFormat() << clustalFormat;
    QTest::newRow("empty.txt") << "empty.txt" << fastaFormat << clustalFormat << DataFormat() << DataFormat();

    QTest::newRow("invalid") << "invalid" << fastaFormat << clustalFormat << DataFormat() << DataFormat();
    QTest::newRow("invalid.fa") << "invalid.fa" << fastaFormat << clustalFormat << DataFormat() << fastaFormat;
    QTest::newRow("invalid.aln") << "invalid.aln" << fastaFormat << clustalFormat << DataFormat() << clustalFormat;
    QTest::newRow("invalid.txt") << "invalid.txt" << fastaFormat << clustalFormat << DataFormat() << DataFormat();

    QTest::newRow("valid_fasta") << "valid_fasta" << fastaFormat << clustalFormat << DataFormat() << fastaFormat;
    QTest::newRow("valid_fasta.fa") << "valid_fasta.fa" << fastaFormat << clustalFormat << DataFormat() << fastaFormat;
    QTest::newRow("valid_fasta.aln") << "valid_fasta.aln" << fastaFormat << clustalFormat << DataFormat() << fastaFormat;
    QTest::newRow("valid_fasta.txt") << "valid_fasta.txt" << fastaFormat << clustalFormat << DataFormat() << fastaFormat;

    QTest::newRow("valid_clustal") << "valid_clustal" << fastaFormat << clustalFormat << DataFormat() << clustalFormat;
    QTest::newRow("valid_clustal.fa") << "valid_clustal.fa" << fastaFormat << clustalFormat << DataFormat() << clustalFormat;
    QTest::newRow("valid_clustal.aln") << "valid_clustal.aln" << fastaFormat << clustalFormat << DataFormat() << clustalFormat;
    QTest::newRow("valid_clustal.txt") << "valid_clustal.txt" << fastaFormat << clustalFormat << DataFormat() << clustalFormat;
}

void TestDataFormatDetector::formatFromFile()
{
    QFETCH(QString, fileName);
    QFETCH(DataFormat, fastaFormat);
    QFETCH(DataFormat, clustalFormat);
    QFETCH(DataFormat, dataFormat1);
    QFETCH(DataFormat, dataFormat2);

    QFile file("files/" + fileName);
    QVERIFY(file.exists());

    DataFormatDetector detector;
    QCOMPARE(detector.formatFromFile(file), dataFormat1);
    QVERIFY(file.isOpen() == false);

    detector.setDataFormats(QVector<DataFormat>() << fastaFormat << clustalFormat);
    QCOMPARE(detector.formatFromFile(file), dataFormat2);
    QVERIFY(file.isOpen() == false);
}

QTEST_APPLESS_MAIN(TestDataFormatDetector)
#include "TestDataFormatDetector.moc"
