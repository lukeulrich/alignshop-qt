/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QSharedPointer>
#include <QtCore/QTextStream>

#include "AbstractDataFormatInspector.h"
#include "DataFormat.h"
#include "ParseError.h"
#include "SimpleSeq.h"

/**
  * To completely test the DataFormat class, we must also test the inspector functions, Because
  * AbstractDataFormatInspector contains a pure-virtual function, we must define a concrete implementation
  * of this class that defines these methods so we can create an instance for testing purposes.
  */
class MockDataFormatInspector : public AbstractDataFormatInspector
{
public:
    DataFormatType inspect(const QString &buffer = QString()) const
    {
        Q_UNUSED(buffer);

        // Simply return a value since this is a mock object
        return eUnknownFormatType;
    }
};

class MockSimpleSeqParser : public SimpleSeqParser
{
public:
    bool readAll(QTextStream &source, QList<SimpleSeq> &listSimpleSeq, ParseError &parseError)
    {
        Q_UNUSED(source);
        Q_UNUSED(listSimpleSeq);
        Q_UNUSED(parseError);

        return true;
    }
};


class TestDataFormat : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void fileExtensions();  // Also tests setFileExtensions
    void type();            // Also tests setFormat
    void inspector();       // Also tests setInspector
    void name();            // Also tests setName
    void parser();          // Also tests setParser
    void hasFileExtension();

    void nameFilter();

    // ------------------------------------------------------------------------------------------------
    // Static methods
    void nameFilters();
};

void TestDataFormat::constructor()
{
    // Test: all the different variants of constructing a DataFormat instance
    DataFormat f1;
    DataFormat f2(eClustalType);
    DataFormat f3(eClustalType, "Clustal");
    DataFormat f4(eClustalType, "Clustal", QStringList() << "aln");
    DataFormat f5(eClustalType, "Clustal", QStringList() << "aln", new MockDataFormatInspector());
}

void TestDataFormat::fileExtensions()
{
    // Test: no fileExtensions by default
    DataFormat f1;
    QVERIFY(f1.fileExtensions().isEmpty());

    // Test: pass empty fileExtensions
    f1.setFileExtensions(QStringList() << "" << "  " << QString());
    QVERIFY(f1.fileExtensions().count() == 0);

    // Test: one extension
    f1.setFileExtensions(QStringList() << "aln");
    QVERIFY(f1.fileExtensions().count() == 1);
    QCOMPARE(f1.fileExtensions().at(0), QString("aln"));

    // Test: multiple fileExtensions
    f1.setFileExtensions(QStringList() << "aln" << "clustal" << "clu");
    QVERIFY(f1.fileExtensions().count() == 3);
    QCOMPARE(f1.fileExtensions().at(0), QString("aln"));
    QCOMPARE(f1.fileExtensions().at(1), QString("clustal"));
    QCOMPARE(f1.fileExtensions().at(2), QString("clu"));

    // Test: clearing the fileExtensions
    f1.setFileExtensions(QStringList());
    QVERIFY(f1.fileExtensions().isEmpty());

    // Test: duplicate fileExtensions
    f1.setFileExtensions(QStringList() << "aln" << "aln");
    QVERIFY(f1.fileExtensions().count() == 1);
    QCOMPARE(f1.fileExtensions().at(0), QString("aln"));

    // Test: mixture of empty and valid fileExtensions
    f1.setFileExtensions(QStringList() << "" << "aln" << " " << "clustal");
    QVERIFY(f1.fileExtensions().count() == 2);
    QCOMPARE(f1.fileExtensions().at(0), QString("aln"));
    QCOMPARE(f1.fileExtensions().at(1), QString("clustal"));

    // Test: mixture of empty, duplicate valid fileExtensions
    f1.setFileExtensions(QStringList() << "" << "aln" << "aln" << " " << "clustal");
    QVERIFY(f1.fileExtensions().count() == 2);
    QCOMPARE(f1.fileExtensions().at(0), QString("aln"));
    QCOMPARE(f1.fileExtensions().at(1), QString("clustal"));

    // Test: fileExtensions defined via the constructor
    DataFormat f2(eClustalType, "Clustal", QStringList() << "clustal" << "aln");
    QVERIFY(f2.fileExtensions().count() == 2);
    QCOMPARE(f2.fileExtensions().at(0), QString("clustal"));
    QCOMPARE(f2.fileExtensions().at(1), QString("aln"));

    // Test: fileExtensions with duplicates and empty values sent via the constructor
    DataFormat f3(eClustalType, "Clustal", QStringList() << "" << "   " << "clustal" << "clustal" << "aln");
    QVERIFY(f3.fileExtensions().count() == 2);
    QCOMPARE(f3.fileExtensions().at(0), QString("clustal"));
    QCOMPARE(f3.fileExtensions().at(1), QString("aln"));
}

void TestDataFormat::type()
{
    // Test: default uninitialized file type should be UnknownFormat
    DataFormat f1;
    QCOMPARE(f1.type(), eUnknownFormatType);

    // Test: file type via constructor
    DataFormat f2(eFastaType);
    QCOMPARE(f2.type(), eFastaType);

    // Test: setting the file type
    DataFormat f3;
    f3.setType(eClustalType);
    QCOMPARE(f3.type(), eClustalType);
}

void TestDataFormat::inspector()
{
    // Test: default inspector should be null and invalid
    DataFormat f1;
    QVERIFY(f1.inspector() == 0);

    // Test: valid inspector set from constructor
    MockDataFormatInspector *inspector = new MockDataFormatInspector();
    DataFormat f2(eClustalType, "Clustal", QStringList(), inspector);
    QVERIFY(f2.inspector() == inspector);

    // Test: setting the inspector
    MockDataFormatInspector *inspector2 = new MockDataFormatInspector();
    DataFormat f3;
    f3.setInspector(inspector2);
    QVERIFY(f3.inspector() == inspector2);

    // Test: clearing the inspector
    f3.setInspector(0);
    QVERIFY(f3.inspector() == 0);
}

void TestDataFormat::name()
{
    // Test: default uninitialized name should be empty
    DataFormat f1;
    QVERIFY(f1.name().isEmpty());

    // Test: name via constructor
    DataFormat f2(eFastaType, "Fasta");
    QCOMPARE(f2.name(), QString("Fasta"));

    // Test: setting the name
    DataFormat f3;
    f3.setName("fasta");
    QCOMPARE(f3.name(), QString("fasta"));
}

void TestDataFormat::parser()
{
    // Test: default parser should be null and invalid
    DataFormat f1;
    QVERIFY(f1.parser() == 0);

    // Test: valid inspector set from constructor
    MockSimpleSeqParser *parser = new MockSimpleSeqParser();
    DataFormat f2(eClustalType, "Clustal", QStringList(), 0, parser);
    QVERIFY(f2.parser() == parser);

    // Test: setting the parser
    MockSimpleSeqParser *parser2 = new MockSimpleSeqParser();
    DataFormat f3;
    f3.setParser(parser2);
    QVERIFY(f3.parser() == parser2);

    // Test: clearing the inspector
    f3.setParser(0);
    QVERIFY(f3.parser() == 0);
}

void TestDataFormat::hasFileExtension()
{
    DataFormat f1;

    // Test: verify that empty input fails with empty extension list
    QVERIFY(f1.hasFileExtension("") == false);
    QVERIFY(f1.hasFileExtension("    ") == false);
    QVERIFY(f1.hasFileExtension(QString()) == false);

    // Test: verify that empty input fails with non-empty extension list
    f1.setFileExtensions(QStringList() << "aln" << "clustal");
    QVERIFY(f1.hasFileExtension("") == false);
    QVERIFY(f1.hasFileExtension("    ") == false);
    QVERIFY(f1.hasFileExtension(QString()) == false);

    // Test: verify that exact matches are found
    QVERIFY(f1.hasFileExtension("aln"));
    QVERIFY(f1.hasFileExtension("clustal"));

    // Test: verify that non-exact matches are not found
    QVERIFY(f1.hasFileExtension("missing") == false);
    QVERIFY(f1.hasFileExtension("aln2") == false);

    // Test: verify that lookup is case-insensitive
    QVERIFY(f1.hasFileExtension("ALN"));
    QVERIFY(f1.hasFileExtension("Aln"));
    QVERIFY(f1.hasFileExtension("aLn"));
    QVERIFY(f1.hasFileExtension("ClUsTaL"));

    // Test: verify that non-exact matches including those prefixed with periods do not work
    QVERIFY(f1.hasFileExtension(".aln") == false);
    QVERIFY(f1.hasFileExtension(".clustal") == false);
}

void TestDataFormat::nameFilter()
{
    DataFormat f1;

    // Test: since no fileExtensions have been provided and the name is invalid, should return an empty string
    QVERIFY(f1.nameFilter().isEmpty());

    // Test: name without extension
    f1.setName("Clustal");
    QVERIFY(f1.nameFilter().isEmpty());

    // Test: extension without name
    f1.setName(QString());
    f1.setFileExtensions(QStringList() << "aln" << "clustal");
    QCOMPARE(f1.nameFilter(), QString("(*.aln *.clustal)"));

    // Test: name with just whitespace
    f1.setName("  ");
    f1.setFileExtensions(QStringList() << "aln" << "clustal");
    QCOMPARE(f1.nameFilter(), QString("   (*.aln *.clustal)")); // Note we have three leading spaces, because even though the name is just whitespace, it is not empty

    // Test: name with valid characters
    f1.setName("Clustal");
    f1.setFileExtensions(QStringList() << "aln" << "clustal");
    QCOMPARE(f1.nameFilter(), QString("Clustal (*.aln *.clustal)"));

    // Test: single extension list
    f1.setFileExtensions(QStringList() << "aln");
    QCOMPARE(f1.nameFilter(), QString("Clustal (*.aln)"));
}

void TestDataFormat::nameFilters()
{
    QList<DataFormat *> types;

    // Test: no file types present, should return empty stringlist
    QVERIFY(DataFormat::nameFilters(types).count() == 0);

    // Test: one file type in the list
    types.append(new DataFormat());
    types[0]->setFileExtensions(QStringList() << "aln" << "clustal");

    QStringList filter_list = DataFormat::nameFilters(types);
    QVERIFY(filter_list.count() == 1);
    QCOMPARE(filter_list.at(0), QString("(*.aln *.clustal)"));

    // Test: multiple types in the list
    qDeleteAll(types);
    types.clear();
    types.append(new DataFormat());
    types[0]->setName("Fasta");
    types[0]->setFileExtensions(QStringList() << "fasta" << "faa" << "fnt");

    types.append(new DataFormat());
    types[1]->setName("JSON");
    types[1]->setFileExtensions(QStringList() << "js");

    filter_list = DataFormat::nameFilters(types);
    QVERIFY(filter_list.count() == 2);
    QCOMPARE(filter_list.at(0), QString("Fasta (*.fasta *.faa *.fnt)"));
    QCOMPARE(filter_list.at(1), QString("JSON (*.js)"));

    qDeleteAll(types);
    types.clear();
}

QTEST_MAIN(TestDataFormat)
#include "TestDataFormat.moc"
