/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QTest>
#include <QtTest/QSignalSpy>

#include <QtCore/QFile>

#include "../../models/AdocTreeModel.h"
#include "../../models/AminoSliceModel.h"
#include "../../models/TableModel.h"
#include "../../models/RelatedTableModel.h"

#include "../../AdocDataSource.h"
#include "../../AdocTreeNode.h"

/**
  * Because SubseqSliceModel is another abstraction of the Slice model concept, it needs a concrete class to fully test its
  * implementation. Rather than create a mock class for this purpose, utilize TestAminoSliceModel for this purpose. Thus,
  * we are testing both SubseqSliceModel and AminoSliceModel here.
  */
class TestAminoSliceModel : public QObject
{
    Q_OBJECT

public:

private slots:

private:
    AdocDataSource dataSource_;
    AdocTreeNode *setup();
};

AdocTreeNode *TestAminoSliceModel::setup()
{
    const QString sourceTestDbFile = "../test_databases/adr_slice.db";

    // adr_slice.db contains the following data_tree:
    //
    // Root
    // |___ Amino (Group)
    //      |___ Asubseq1
    //      |___ Asubseq2
    //      |___ Group 1 (Group)
    // |___ Dna (Group)
    //      |___ Dsubseq1
    //      |___ Dsubseq2
    //      |___ Group 2 (Group)
    // |___ Rna (Group)
    //      |___ Rsubseq1
    //      |___ Rsubseq2
    //      |___ Group 3 (Group)
    // |___ Mix (Group)
    //      |___ Asubseq3
    //      |___ Asubseq4
    //      |___ Dsubseq3
    //      |___ Rsubseq3
    //      |___ Rsubseq4
    //      |___ Rsubseq5
    //      |___ Group 4 (Group)
    // |___ NoImmediateSubseqs (Group)
    //      |___ Group 5 (Group)
    //           |___ Asubseq5
    //           |___ Dsubseq4
    //           |___ Rsubseq6
    // |___ SplitSeq (Group)
    //      |___ AminoA (Group)
    //           |___ Asubseq6-1
    //           |___ Asubseq6-3
    //      |___ AminoB (Group)
    //           |___ Asubseq6-2
    //      |___ DnaA (Group)
    //           |___ Dsubseq5-1
    //           |___ Dsubseq5-3
    //      |___ DnaB (Group)
    //           |___ Dsubseq5-2
    //      |___ RnaA (Group)
    //           |___ Rsubseq7-1
    //           |___ Rsubseq7-3
    //      |___ RnaB (Group)
    //           |___ Rsubseq7-2

    if (!QFile::exists(sourceTestDbFile))
        return 0;

    // Make a local copy for testing purposes so that changes do not affect the master copy (overwrite any existing file)
    const QString testDbFile = "adr_slice-test.db";
    if (QFile::exists(testDbFile))
        QFile::remove(testDbFile);

    QFile::copy(sourceTestDbFile, testDbFile);

    if (dataSource_.isOpen())
        dataSource_.close();
    dataSource_.open(testDbFile);
    if (!dataSource_.isOpen())
        return 0;

    QSignalSpy spyDataTreeReady(&dataSource_, SIGNAL(dataTreeReady(AdocTreeNode *)));
    dataSource_.readDataTree();
    if (spyDataTreeReady.isEmpty())
        return 0;

    return qvariant_cast<AdocTreeNode *>(spyDataTreeReady.takeFirst().at(0));
}



QTEST_MAIN(TestAminoSliceModel)
#include "TestAminoSliceModel.moc"
