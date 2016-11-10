/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QSharedPointer>

#include "DataFormatDetector.h"
#include "FastaFormatInspector.h"
#include "ClustalFormatInspector.h"

class TestDataFormatDetector : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Public methods
    void dataFormats();     // Also tests setDataFormats
    void formatFromFileExtension();
    void formatFromString();
    void formatFromFile();
    void unknownFormat();
};

void TestDataFormatDetector::dataFormats()
{
    DataFormatDetector detector;

    // Test: default should have no data formats defined
    QVERIFY(detector.dataFormats().isEmpty());

    // Test: should be able to add a data format and get it back
    QList<DataFormat *> formats;
    formats << new DataFormat(eFastaType, "Fasta");

    detector.setDataFormats(formats);

    QVERIFY(detector.dataFormats().size() == 1);
    QVERIFY(detector.dataFormats().at(0) == formats.at(0));

    // Test: clear out formats via passing empty list
    detector.setDataFormats(QList<DataFormat *>());
    QVERIFY(detector.dataFormats().isEmpty());
    formats.clear();

    // Test: set multiple data formats should be taken
    formats << new DataFormat(eFastaType, "Fasta")
            << new DataFormat(eClustalType, "Clustal");
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
        QCOMPARE(detector.formatFromFileExtension(extension)->type(), eUnknownFormatType);

    // Test: check that registered types are returned appropriately
    QList<DataFormat *> formats;
    formats << new DataFormat(eFastaType, "Fasta", QStringList() << "faa" << "fa" << "fnt")
            << new DataFormat(eClustalType, "Clustal", QStringList() << "aln");

    // Test: all registered types
    detector.setDataFormats(formats);
    QCOMPARE(detector.formatFromFileExtension("faa")->type(), eFastaType);
    QCOMPARE(detector.formatFromFileExtension("fa")->type(), eFastaType);
    QCOMPARE(detector.formatFromFileExtension("fnt")->type(), eFastaType);
    QCOMPARE(detector.formatFromFileExtension("aln")->type(), eClustalType);

    // Test: unregistered extensions
    QCOMPARE(detector.formatFromFileExtension("ph")->type(), eUnknownFormatType);
    QCOMPARE(detector.formatFromFileExtension("")->type(), eUnknownFormatType);

    // Test: if duplicate extensions are registered should return type of first containing DataFormat
    formats.clear();
    formats << new DataFormat(eFastaType, "Fasta", QStringList() << "faa" << "fa" << "fnt")
            << new DataFormat(eClustalType, "Clustal", QStringList() << "aln" << "faa");
    QCOMPARE(detector.formatFromFileExtension("faa")->type(), eFastaType);
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
        QCOMPARE(detector.formatFromString(data_string)->type(), eUnknownFormatType);

    // Test: should receive same result if there are no valid AbstractDataFormatInspectors
    QList<DataFormat *> formats;
    formats << new DataFormat(eFastaType, "Fasta", QStringList() << "faa" << "fa" << "fnt")
            << new DataFormat(eClustalType, "Clustal", QStringList() << "aln");
    detector.setDataFormats(formats);

    foreach (QString data_string, data_strings)
        QCOMPARE(detector.formatFromString(data_string)->type(), eUnknownFormatType);

    // Test: same as above, except with valid inspectors
    formats.at(0)->setInspector(new FastaFormatInspector());
    formats.at(1)->setInspector(new ClustalFormatInspector());

    QCOMPARE(detector.formatFromString(">test sequence\nATGC\n")->type(), eFastaType);
    QCOMPARE(detector.formatFromString("CLUSTAL\n\nTest1  AT-G\nTest2  ATCG\n")->type(), eClustalType);

    // Test: junk data with valid inspectors
    QCOMPARE(detector.formatFromString("blah blah missing invalid\n\ntype")->type(), eUnknownFormatType);

    // Test: Should return first matching inspector result
    formats.at(1)->setInspector(new FastaFormatInspector());
    QCOMPARE(detector.formatFromString(">test sequence\nATGC\n")->type(), eFastaType);
}

void TestDataFormatDetector::formatFromFile()
{
    QStringList invalid_data_file_list;
    invalid_data_file_list << "invalid"
                           << "invalid.fa"
                           << "invalid.aln"
                           << "invalid.txt";

    QStringList valid_fasta_file_list;
    valid_fasta_file_list << "valid_fasta"
                          << "valid_fasta.fa"
                          << "valid_fasta.aln"
                          << "valid_fasta.txt";

    QStringList valid_clustal_file_list;
    valid_clustal_file_list << "valid_clustal"
                            << "valid_clustal.aln"
                            << "valid_clustal.fa"
                            << "valid_clustal.txt";

    DataFormatDetector detector;

    // Test: no matter the file, without any valid inspectors, should always return unknown format
    QFile file("files/empty");
    QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
    QVERIFY(file.isOpen() == false);

    file.setFileName("files/empty.fa");
    QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
    QVERIFY(file.isOpen() == false);

    file.setFileName("files/empty.aln");
    QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
    QVERIFY(file.isOpen() == false);

    file.setFileName("files/empty.txt");
    QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
    QVERIFY(file.isOpen() == false);

    foreach (QString filename, invalid_data_file_list)
    {
        file.setFileName("files/" + filename);
        QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
        QVERIFY(file.isOpen() == false);
    }
    foreach (QString filename, valid_fasta_file_list)
    {
        file.setFileName("files/" + filename);
        QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
        QVERIFY(file.isOpen() == false);
    }
    foreach (QString filename, valid_clustal_file_list)
    {
        file.setFileName("files/" + filename);
        QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
        QVERIFY(file.isOpen() == false);
    }

    // Test: now with valid inspectors
    QList<DataFormat *> formats;
    formats << new DataFormat(eFastaType, "Fasta", QStringList() << "faa" << "fa" << "fnt", new FastaFormatInspector())
            << new DataFormat(eClustalType, "Clustal", QStringList() << "aln", new ClustalFormatInspector());
    detector.setDataFormats(formats);

    // Test: empty files - since we have valid inspectors, should result in specific types for those that have registered file extensions
    file.setFileName("files/empty");
    QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
    QVERIFY(file.isOpen() == false);

    file.setFileName("files/empty.fa");
    QCOMPARE(detector.formatFromFile(file)->type(), eFastaType);
    QVERIFY(file.isOpen() == false);

    file.setFileName("files/empty.aln");
    QCOMPARE(detector.formatFromFile(file)->type(), eClustalType);
    QVERIFY(file.isOpen() == false);

    file.setFileName("files/empty.txt");
    QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
    QVERIFY(file.isOpen() == false);

    // Test: invalid files - since we have valid inspectors, should result in specific types for those that have registered file extensions
    file.setFileName("files/invalid");
    QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
    QVERIFY(file.isOpen() == false);
    file.setFileName("files/invalid.fa");
    QCOMPARE(detector.formatFromFile(file)->type(), eFastaType);
    QVERIFY(file.isOpen() == false);
    file.setFileName("files/invalid.aln");
    QCOMPARE(detector.formatFromFile(file)->type(), eClustalType);
    QVERIFY(file.isOpen() == false);
    file.setFileName("files/invalid.txt");
    QCOMPARE(detector.formatFromFile(file)->type(), eUnknownFormatType);
    QVERIFY(file.isOpen() == false);

    // Test: valid fasta files (some with wrong extensions)
    foreach (QString filestring, valid_fasta_file_list)
    {
        file.setFileName("files/" + filestring);
        QCOMPARE(detector.formatFromFile(file)->type(), eFastaType);
        QVERIFY(file.isOpen() == false);
    }

    // Test: valid clustal files (some with wrong extensions)
    foreach (QString filestring, valid_clustal_file_list)
    {
        file.setFileName("files/" + filestring);
        QCOMPARE(detector.formatFromFile(file)->type(), eClustalType);
        QVERIFY(file.isOpen() == false);
    }
}

void TestDataFormatDetector::unknownFormat()
{
    DataFormatDetector detector;

    QVERIFY(detector.unknownFormat() != 0);
    QVERIFY(detector.unknownFormat()->type() == eUnknownFormatType);
}

QTEST_MAIN(TestDataFormatDetector)
#include "TestDataFormatDetector.moc"
