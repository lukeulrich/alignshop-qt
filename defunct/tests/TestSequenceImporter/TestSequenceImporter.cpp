/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "SequenceImporter.h"

#include "exceptions/DatabaseError.h"
#include "models/AdocTreeModel.h"

#include "AminoString.h"
#include "DbAnonSeqFactory.h"
#include "ParsedBioString.h"

#include "global.h"

class TestSequenceImporter : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void setAdocTreeModel();
    void setAnonSeqFactories();

    void importSequences();
    void importAlignment();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
void TestSequenceImporter::constructor()
{
    SequenceImporter importer;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
void TestSequenceImporter::setAdocTreeModel()
{
    SequenceImporter importer;

    importer.setAdocTreeModel(0);

    AdocTreeModel *treeModel = new AdocTreeModel();
    importer.setAdocTreeModel(treeModel);

    delete treeModel;
    treeModel = 0;
}

void TestSequenceImporter::setAnonSeqFactories()
{
    SequenceImporter importer;

    importer.setAnonSeqFactories(QHash<Alphabet, AnonSeqFactory *>());
}

void TestSequenceImporter::importSequences()
{
    SequenceImporter imp;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Test: create the astring table
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableAstrings + QString(" (id integer not null primary key autoincrement, digest text not null, sequence text not null)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDstrings + QString(" (id integer not null primary key autoincrement, digest text not null, sequence text not null)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableRstrings + QString(" (id integer not null primary key autoincrement, digest text not null, sequence text not null)")).lastError().type() == QSqlError::NoError);
    }

    QHash<Alphabet, AnonSeqFactory *> factories_;
    DbAnonSeqFactory *aminoAnonSeqFactory = new DbAnonSeqFactory();
    QVERIFY(aminoAnonSeqFactory->setDataSource("test_db", "astrings"));
    factories_[eAminoAlphabet] = aminoAnonSeqFactory;

    // Test: no tree model, no factory, empty list
    QVERIFY(imp.importSequences(QList<ParsedBioString>(), eAminoAlphabet, QModelIndex()) == false);

    // Test: tree model, no factory, empty list
    AdocTreeModel *treeModel = new AdocTreeModel();
    imp.setAdocTreeModel(treeModel);
    QVERIFY(imp.importSequences(QList<ParsedBioString>(), eAminoAlphabet, QModelIndex()) == false);

    // Test: no tree model, factory, empty list
    imp.setAdocTreeModel(0);
    imp.setAnonSeqFactories(factories_);
    QVERIFY(imp.importSequences(QList<ParsedBioString>(), eAminoAlphabet, QModelIndex()) == false);

    // Test: tree model, factory, empty list
    imp.setAdocTreeModel(treeModel);
    QVERIFY(imp.importSequences(QList<ParsedBioString>(), eRnaAlphabet, QModelIndex()) == false);

    // Test: valid parsed bio string but with no factory for the Dna Alphabet
    ParsedBioString pbs("ATCG", "Dna string", true, true);
    QVERIFY(imp.importSequences(QList<ParsedBioString>() << pbs, eDnaAlphabet, QModelIndex()) == false);

    QList<ParsedBioString> pbss;
    pbss.append(ParsedBioString("AT-CG", "Dna1", true, true));
    pbss.append(ParsedBioString("ATCG--", "Dna1.1", false, true));
    pbss.append(ParsedBioString("TCG-A", "Dna2", true, false));

    DbAnonSeqFactory *dnaAnonSeqFactory = new DbAnonSeqFactory();
    QVERIFY(dnaAnonSeqFactory->setDataSource("test_db", "dstrings"));
    factories_[eDnaAlphabet] = dnaAnonSeqFactory;
    imp.setAnonSeqFactories(factories_);

    treeModel->adocDbDataSource_.setConnectionName("test_db");

    try
    {
        // -----------------------------------
        // Test: try with missing tables
        try
        {
            imp.importSequences(pbss, eDnaAlphabet, QModelIndex());
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);
        }


        // -----------------------------------
        // Create the dna_seqs table
        QSqlDatabase db = QSqlDatabase::database("test_db");
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDnaSeqs + QString(" (id integer not null primary key autoincrement, "
                                                                                      "  dstring_id integer not null,"
                                                                                      "  label text,"
                                                                                      "  organism text not null)")).lastError().type() == QSqlError::NoError);
        try
        {
            imp.importSequences(pbss, eDnaAlphabet, QModelIndex());
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);

            // Check that no sequences were inserted
            QSqlQuery s_seqs = db.exec(QString("SELECT count(*) FROM %1").arg(constants::kTableDnaSeqs));
            QVERIFY(s_seqs.lastError().type() == QSqlError::NoError);
            QVERIFY(s_seqs.next());
            QVERIFY(s_seqs.value(0).toInt() == 0);
        }

        // -----------------------------------
        // Test: all necessary tables present and accounted for

        // Create the subseqs table
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDnaSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                         "  dna_seq_id integer not null,"
                                                                                         "  label text,"
                                                                                         "  start integer not null,"
                                                                                         "  stop integer not null,"
                                                                                         "  sequence text not null)")).lastError().type() == QSqlError::NoError);

        // -----------------------------------
        // Test: should not import unless specify the fields to import. By default, DbDataSource will attempt to
        //       insert all fields if none are specified. Thus, the not null constraint on dna_seqs.organism should
        //       cause the insert to fail. Only do this for the Dna seqs and subseqs table (not the amino and rna tests
        //       below).
        try
        {
            imp.importSequences(pbss, eDnaAlphabet, QModelIndex());
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);
        }

        // Remove the not null column
        QVERIFY(db.exec(QString("DROP TABLE %1").arg(constants::kTableDnaSeqs)).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDnaSeqs + QString(" (id integer not null primary key autoincrement, "
                                                                                      "  dstring_id integer not null,"
                                                                                      "  label text,"
                                                                                      "  organism text)")).lastError().type() == QSqlError::NoError);

        try
        {
            imp.importSequences(pbss, eDnaAlphabet, QModelIndex());

            // Verify that they were loaded into the database
            QSqlQuery s_dstrings = db.exec(QString("SELECT id, sequence FROM %1 ORDER BY id").arg(constants::kTableDstrings));
            QVERIFY(s_dstrings.lastError().type() == QSqlError::NoError);
            QVERIFY(s_dstrings.next());
            QCOMPARE(s_dstrings.value(0).toInt(), 1);
            QCOMPARE(s_dstrings.value(1).toString(), QString("ATCG"));
            QVERIFY(s_dstrings.next());
            QCOMPARE(s_dstrings.value(0).toInt(), 2);
            QCOMPARE(s_dstrings.value(1).toString(), QString("TCGA"));
            QVERIFY(!s_dstrings.next());

            // Verify the dna_seqs and dna_subseqs table
            QSqlQuery s_dnaseqs = db.exec(QString("SELECT id, dstring_id, label FROM %1 ORDER BY id").arg(constants::kTableDnaSeqs));
            QVERIFY(s_dnaseqs.lastError().type() == QSqlError::NoError);

            QSqlQuery s_dnasubseqs(db);
            QVERIFY(s_dnasubseqs.prepare(QString("SELECT start, stop, sequence FROM %1 WHERE dna_seq_id = ?").arg(constants::kTableDnaSubseqs)));

            // Take the first dna seq
            QVERIFY(s_dnaseqs.next());
            QCOMPARE(s_dnaseqs.value(0).toInt(), 1);    // id == 1
            QCOMPARE(s_dnaseqs.value(1).toInt(), 1);    // dstring_id == 1
            QCOMPARE(s_dnaseqs.value(2).toString(), QString("Dna1"));

            s_dnasubseqs.bindValue(0, 1);   // dna_seq_id = 1
            QVERIFY(s_dnasubseqs.exec());
            QVERIFY(s_dnasubseqs.next());
            QCOMPARE(s_dnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_dnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_dnasubseqs.value(2).toString(), QString("AT-CG"));

            // Next dna seq
            QVERIFY(s_dnaseqs.next());
            QCOMPARE(s_dnaseqs.value(0).toInt(), 2);    // id == 2
            QCOMPARE(s_dnaseqs.value(1).toInt(), 1);    // dstring_id == 1 because same sequence as the first one
            QCOMPARE(s_dnaseqs.value(2).toString(), QString("Dna1.1"));

            s_dnasubseqs.bindValue(0, 2);   // dna_seq_id = 2
            QVERIFY(s_dnasubseqs.exec());
            QVERIFY(s_dnasubseqs.next());
            QCOMPARE(s_dnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_dnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_dnasubseqs.value(2).toString(), QString("ATCG--"));

            // Last dna seq
            QVERIFY(s_dnaseqs.next());
            QCOMPARE(s_dnaseqs.value(0).toInt(), 3);    // id == 3
            QCOMPARE(s_dnaseqs.value(1).toInt(), 2);    // dstring_id == 2
            QCOMPARE(s_dnaseqs.value(2).toString(), QString("Dna2"));
            QVERIFY(!s_dnaseqs.next());

            s_dnasubseqs.bindValue(0, 3);   // dna_seq_id = 2
            QVERIFY(s_dnasubseqs.exec());
            QVERIFY(s_dnasubseqs.next());
            QCOMPARE(s_dnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_dnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_dnasubseqs.value(2).toString(), QString("TCG-A"));

            // Verify that the three subseqs were placed beneath the root index
            QCOMPARE(treeModel->rowCount(), 3);
            AdocTreeNode *node = treeModel->data(treeModel->index(0, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqDnaType);
            QCOMPARE(node->label_, QString("Dna1"));
            QCOMPARE(node->fkId_, 1);

            node = treeModel->data(treeModel->index(1, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqDnaType);
            QCOMPARE(node->label_, QString("Dna1.1"));
            QCOMPARE(node->fkId_, 2);

            node = treeModel->data(treeModel->index(2, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqDnaType);
            QCOMPARE(node->label_, QString("Dna2"));
            QCOMPARE(node->fkId_, 3);
        }
        catch (DatabaseError &e) { QVERIFY(0); }


        // ---------------------------------------------------
        // Same tests as above except with Amino acid sequences
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableAminoSeqs + QString(" (id integer not null primary key autoincrement, "
                                                                                      "  astring_id integer not null,"
                                                                                      "  label text,"
                                                                                      "  organism text)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableAminoSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                         "  amino_seq_id integer not null,"
                                                                                         "  label text,"
                                                                                         "  start integer not null,"
                                                                                         "  stop integer not null,"
                                                                                         "  sequence text not null)")).lastError().type() == QSqlError::NoError);

        pbss.clear();
        pbss.append(ParsedBioString("VYVF--DEKGR", "PAS-A", true, true));
        pbss.append(ParsedBioString("--VVFF", "PAS-B", false, true));
        pbss.append(ParsedBioString("VVFF--", "PAS-C", true, false));

        try
        {
            imp.importSequences(pbss, eAminoAlphabet, QModelIndex());

            // Verify that they were loaded into the database
            QSqlQuery s_astrings = db.exec(QString("SELECT id, sequence FROM %1 ORDER BY id").arg(constants::kTableAstrings));
            QVERIFY(s_astrings.lastError().type() == QSqlError::NoError);
            QVERIFY(s_astrings.next());
            QCOMPARE(s_astrings.value(0).toInt(), 1);
            QCOMPARE(s_astrings.value(1).toString(), QString("VYVFDEKGR"));
            QVERIFY(s_astrings.next());
            QCOMPARE(s_astrings.value(0).toInt(), 2);
            QCOMPARE(s_astrings.value(1).toString(), QString("VVFF"));
            QVERIFY(!s_astrings.next());

            // Verify the amino_seqs and amino_subseqs table
            QSqlQuery s_aminoseqs = db.exec(QString("SELECT id, astring_id, label FROM %1 ORDER BY id").arg(constants::kTableAminoSeqs));
            QVERIFY(s_aminoseqs.lastError().type() == QSqlError::NoError);

            QSqlQuery s_aminosubseqs(db);
            QVERIFY(s_aminosubseqs.prepare(QString("SELECT start, stop, sequence FROM %1 WHERE amino_seq_id = ?").arg(constants::kTableAminoSubseqs)));

            // Take the first amino seq
            QVERIFY(s_aminoseqs.next());
            QCOMPARE(s_aminoseqs.value(0).toInt(), 1);    // id == 1
            QCOMPARE(s_aminoseqs.value(1).toInt(), 1);    // astring_id == 1
            QCOMPARE(s_aminoseqs.value(2).toString(), QString("PAS-A"));

            s_aminosubseqs.bindValue(0, 1);   // amino_seq_id = 1
            QVERIFY(s_aminosubseqs.exec());
            QVERIFY(s_aminosubseqs.next());
            QCOMPARE(s_aminosubseqs.value(0).toInt(), 1);
            QCOMPARE(s_aminosubseqs.value(1).toInt(), 9);
            QCOMPARE(s_aminosubseqs.value(2).toString(), QString("VYVF--DEKGR"));

            // Next amino seq
            QVERIFY(s_aminoseqs.next());
            QCOMPARE(s_aminoseqs.value(0).toInt(), 2);    // id == 2
            QCOMPARE(s_aminoseqs.value(1).toInt(), 2);
            QCOMPARE(s_aminoseqs.value(2).toString(), QString("PAS-B"));

            s_aminosubseqs.bindValue(0, 2);   // amino_seq_id = 2
            QVERIFY(s_aminosubseqs.exec());
            QVERIFY(s_aminosubseqs.next());
            QCOMPARE(s_aminosubseqs.value(0).toInt(), 1);
            QCOMPARE(s_aminosubseqs.value(1).toInt(), 4);
            QCOMPARE(s_aminosubseqs.value(2).toString(), QString("--VVFF"));

            // Last amino seq
            QVERIFY(s_aminoseqs.next());
            QCOMPARE(s_aminoseqs.value(0).toInt(), 3);    // id == 3
            QCOMPARE(s_aminoseqs.value(1).toInt(), 2);    // astring_id == 2
            QCOMPARE(s_aminoseqs.value(2).toString(), QString("PAS-C"));
            QVERIFY(!s_aminoseqs.next());

            s_aminosubseqs.bindValue(0, 3);   // amino_seq_id = 2
            QVERIFY(s_aminosubseqs.exec());
            QVERIFY(s_aminosubseqs.next());
            QCOMPARE(s_aminosubseqs.value(0).toInt(), 1);
            QCOMPARE(s_aminosubseqs.value(1).toInt(), 4);
            QCOMPARE(s_aminosubseqs.value(2).toString(), QString("VVFF--"));

            // Verify that the three subseqs were placed beneath the root index
            QCOMPARE(treeModel->rowCount(), 6);
            AdocTreeNode *node = treeModel->data(treeModel->index(3, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqAminoType);
            QCOMPARE(node->label_, QString("PAS-A"));
            QCOMPARE(node->fkId_, 1);

            node = treeModel->data(treeModel->index(4, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqAminoType);
            QCOMPARE(node->label_, QString("PAS-B"));
            QCOMPARE(node->fkId_, 2);

            node = treeModel->data(treeModel->index(5, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqAminoType);
            QCOMPARE(node->label_, QString("PAS-C"));
            QCOMPARE(node->fkId_, 3);
        }
        catch (DatabaseError &e) { QVERIFY(0); }


        // ---------------------------------------------------
        // Same tests as above except with Rna sequences
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableRnaSeqs + QString(" (id integer not null primary key autoincrement, "
                                                                                      "  rstring_id integer not null,"
                                                                                      "  label text,"
                                                                                      "  organism text)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableRnaSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                         "  rna_seq_id integer not null,"
                                                                                         "  label text,"
                                                                                         "  start integer not null,"
                                                                                         "  stop integer not null,"
                                                                                         "  sequence text not null)")).lastError().type() == QSqlError::NoError);

        pbss.clear();
        pbss.append(ParsedBioString("AU-CG", "Rna1", true, true));
        pbss.append(ParsedBioString("AUCG--", "Rna1.1", false, true));
        pbss.append(ParsedBioString("UCG-A", "Rna2", true, false));

        DbAnonSeqFactory *rnaAnonSeqFactory = new DbAnonSeqFactory();
        QVERIFY(rnaAnonSeqFactory->setDataSource("test_db", "rstrings"));
        factories_[eRnaAlphabet] = rnaAnonSeqFactory;
        imp.setAnonSeqFactories(factories_);

        try
        {
            imp.importSequences(pbss, eRnaAlphabet, QModelIndex());

            // Verify that they were loaded into the database
            QSqlQuery s_rstrings = db.exec(QString("SELECT id, sequence FROM %1 ORDER BY id").arg(constants::kTableRstrings));
            QVERIFY(s_rstrings.lastError().type() == QSqlError::NoError);
            QVERIFY(s_rstrings.next());
            QCOMPARE(s_rstrings.value(0).toInt(), 1);
            QCOMPARE(s_rstrings.value(1).toString(), QString("AUCG"));
            QVERIFY(s_rstrings.next());
            QCOMPARE(s_rstrings.value(0).toInt(), 2);
            QCOMPARE(s_rstrings.value(1).toString(), QString("UCGA"));
            QVERIFY(!s_rstrings.next());

            // Verify the rna_seqs and rna_subseqs table
            QSqlQuery s_rnaseqs = db.exec(QString("SELECT id, rstring_id, label FROM %1 ORDER BY id").arg(constants::kTableRnaSeqs));
            QVERIFY(s_rnaseqs.lastError().type() == QSqlError::NoError);

            QSqlQuery s_rnasubseqs(db);
            QVERIFY(s_rnasubseqs.prepare(QString("SELECT start, stop, sequence FROM %1 WHERE rna_seq_id = ?").arg(constants::kTableRnaSubseqs)));

            // Take the first rna seq
            QVERIFY(s_rnaseqs.next());
            QCOMPARE(s_rnaseqs.value(0).toInt(), 1);    // id == 1
            QCOMPARE(s_rnaseqs.value(1).toInt(), 1);    // rstring_id == 1
            QCOMPARE(s_rnaseqs.value(2).toString(), QString("Rna1"));

            s_rnasubseqs.bindValue(0, 1);   // rna_seq_id = 1
            QVERIFY(s_rnasubseqs.exec());
            QVERIFY(s_rnasubseqs.next());
            QCOMPARE(s_rnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_rnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_rnasubseqs.value(2).toString(), QString("AU-CG"));

            // Next rna seq
            QVERIFY(s_rnaseqs.next());
            QCOMPARE(s_rnaseqs.value(0).toInt(), 2);    // id == 2
            QCOMPARE(s_rnaseqs.value(1).toInt(), 1);    // rstring_id == 1 because same sequence as the first one
            QCOMPARE(s_rnaseqs.value(2).toString(), QString("Rna1.1"));

            s_rnasubseqs.bindValue(0, 2);   // rna_seq_id = 2
            QVERIFY(s_rnasubseqs.exec());
            QVERIFY(s_rnasubseqs.next());
            QCOMPARE(s_rnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_rnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_rnasubseqs.value(2).toString(), QString("AUCG--"));

            // Last rna seq
            QVERIFY(s_rnaseqs.next());
            QCOMPARE(s_rnaseqs.value(0).toInt(), 3);    // id == 3
            QCOMPARE(s_rnaseqs.value(1).toInt(), 2);    // dstring_id == 2
            QCOMPARE(s_rnaseqs.value(2).toString(), QString("Rna2"));
            QVERIFY(!s_rnaseqs.next());

            s_rnasubseqs.bindValue(0, 3);   // rna_seq_id = 2
            QVERIFY(s_rnasubseqs.exec());
            QVERIFY(s_rnasubseqs.next());
            QCOMPARE(s_rnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_rnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_rnasubseqs.value(2).toString(), QString("UCG-A"));

            // Verify that the three subseqs were placed beneath the root index
            QCOMPARE(treeModel->rowCount(), 9);
            AdocTreeNode *node = treeModel->data(treeModel->index(6, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqRnaType);
            QCOMPARE(node->label_, QString("Rna1"));
            QCOMPARE(node->fkId_, 1);

            node = treeModel->data(treeModel->index(7, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqRnaType);
            QCOMPARE(node->label_, QString("Rna1.1"));
            QCOMPARE(node->fkId_, 2);

            node = treeModel->data(treeModel->index(8, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::SubseqRnaType);
            QCOMPARE(node->label_, QString("Rna2"));
            QCOMPARE(node->fkId_, 3);
        }
        catch (DatabaseError &e) { QVERIFY(0); }
    }
    catch (...) { QVERIFY(0); }

    QSqlDatabase::removeDatabase("test_db");
}

void TestSequenceImporter::importAlignment()
{
    SequenceImporter imp;

    QVERIFY(imp.importAlignment("", QList<ParsedBioString>(), eAminoAlphabet, QModelIndex()) == false);

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "test_db");
        db.setDatabaseName(":memory:");
        QVERIFY(db.open());

        // Test: create the astring table
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableAstrings + QString(" (id integer not null primary key autoincrement, digest text not null, sequence text not null)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDstrings + QString(" (id integer not null primary key autoincrement, digest text not null, sequence text not null)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableRstrings + QString(" (id integer not null primary key autoincrement, digest text not null, sequence text not null)")).lastError().type() == QSqlError::NoError);
    }

    QHash<Alphabet, AnonSeqFactory *> factories_;
    DbAnonSeqFactory *aminoAnonSeqFactory = new DbAnonSeqFactory();
    QVERIFY(aminoAnonSeqFactory->setDataSource("test_db", "astrings"));
    factories_[eAminoAlphabet] = aminoAnonSeqFactory;

    // Test: no tree model, no factory, empty list
    QVERIFY(imp.importAlignment("dummy", QList<ParsedBioString>(), eAminoAlphabet, QModelIndex()) == false);

    // Test: tree model, no factory, empty list
    AdocTreeModel *treeModel = new AdocTreeModel();
    imp.setAdocTreeModel(treeModel);
    QVERIFY(imp.importAlignment("dummy", QList<ParsedBioString>(), eAminoAlphabet, QModelIndex()) == false);

    // Test: no tree model, factory, empty list
    imp.setAdocTreeModel(0);
    imp.setAnonSeqFactories(factories_);
    QVERIFY(imp.importAlignment("dummy", QList<ParsedBioString>(), eAminoAlphabet, QModelIndex()) == false);

    // Test: tree model, factory, empty list
    imp.setAdocTreeModel(treeModel);
    QVERIFY(imp.importAlignment("dummy", QList<ParsedBioString>(), eRnaAlphabet, QModelIndex()) == false);

    // Test: valid parsed bio string but with no factory for the Dna Alphabet
    ParsedBioString pbs("ATCG", "Dna string1", true, true);
    ParsedBioString pbs2("AT-G", "Dna string2", true, true);
    QVERIFY(imp.importAlignment("dummy", QList<ParsedBioString>() << pbs << pbs2, eDnaAlphabet, QModelIndex()) == false);

    QList<ParsedBioString> pbss;
    pbss.append(ParsedBioString("AT-CG", "Dna1", true, true));
    pbss.append(ParsedBioString("ATCG-", "Dna1.1", false, true));
    pbss.append(ParsedBioString("TCG-A", "Dna2", true, false));

    DbAnonSeqFactory *dnaAnonSeqFactory = new DbAnonSeqFactory();
    QVERIFY(dnaAnonSeqFactory->setDataSource("test_db", "dstrings"));
    factories_[eDnaAlphabet] = dnaAnonSeqFactory;
    imp.setAnonSeqFactories(factories_);

    treeModel->adocDbDataSource_.setConnectionName("test_db");

    try
    {
        // -----------------------------------
        // Test: try with missing tables
        try
        {
            imp.importAlignment("dummy", pbss, eDnaAlphabet, QModelIndex());
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);
        }


        // -----------------------------------
        // Create the dna_seqs table
        QSqlDatabase db = QSqlDatabase::database("test_db");
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDnaSeqs + QString(" (id integer not null primary key autoincrement, "
                                                                                      "  dstring_id integer not null,"
                                                                                      "  label text,"
                                                                                      "  organism text)")).lastError().type() == QSqlError::NoError);
        try
        {
            imp.importAlignment("dummy", pbss, eDnaAlphabet, QModelIndex());
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);

            // Check that no sequences were inserted
            QSqlQuery s_seqs = db.exec(QString("SELECT count(*) FROM %1").arg(constants::kTableDnaSeqs));
            QVERIFY(s_seqs.lastError().type() == QSqlError::NoError);
            QVERIFY(s_seqs.next());
            QVERIFY(s_seqs.value(0).toInt() == 0);
        }

        // Create the subseqs table
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDnaSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                         "  dna_seq_id integer not null,"
                                                                                         "  label text,"
                                                                                         "  start integer not null,"
                                                                                         "  stop integer not null,"
                                                                                         "  sequence text not null)")).lastError().type() == QSqlError::NoError);
        try
        {
            imp.importAlignment("dummy", pbss, eDnaAlphabet, QModelIndex());
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);

            // Check that no sequences were inserted
            QSqlQuery s_seqs = db.exec(QString("SELECT count(*) FROM %1").arg(constants::kTableDnaSubseqs));
            QVERIFY(s_seqs.lastError().type() == QSqlError::NoError);
            QVERIFY(s_seqs.next());
            QVERIFY(s_seqs.value(0).toInt() == 0);
        }

        // Create the msa table
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDnaMsas + QString(" (id integer not null primary key autoincrement,"
                                                                                      "  name text not null,"
                                                                                      "  description text)")).lastError().type() == QSqlError::NoError);
        try
        {
            imp.importAlignment("dummy", pbss, eDnaAlphabet, QModelIndex());
            QVERIFY(0);
        }
        catch (DatabaseError &e)
        {
            QVERIFY(1);

            // Check that no sequences were inserted
            QSqlQuery s_seqs = db.exec(QString("SELECT count(*) FROM %1").arg(constants::kTableDnaMsas));
            QVERIFY(s_seqs.lastError().type() == QSqlError::NoError);
            QVERIFY(s_seqs.next());
            QVERIFY(s_seqs.value(0).toInt() == 0);
        }

        // -----------------------------------
        // Test: all necessary tables present and accounted for
        // Create the msa subseqs table
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableDnaMsaSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                            "  dna_msa_id integer not null,"
                                                                                            "  dna_subseq_id integer not null)")).lastError().type() == QSqlError::NoError);

        // Test: only one sequence should not be allowed for import
        QVERIFY(imp.importAlignment("dummy", QList<ParsedBioString>() << pbs, eDnaAlphabet, QModelIndex()) == false);

        try
        {
            // Test: alignment with some of the sequences having different lengths should fail
            pbss[1].bioString_ = "ATCG----";
            QVERIFY(imp.importAlignment("dummy", pbss, eDnaAlphabet, QModelIndex()) == false);
            QSqlQuery s_dstrings(db);
            QVERIFY(s_dstrings.prepare(QString("SELECT id, sequence FROM %1 ORDER BY id").arg(constants::kTableDstrings)));
            QVERIFY(s_dstrings.exec());
            QVERIFY(s_dstrings.next() == false);

            // Test: now import a valid alignment
            pbss[1].bioString_ = "ATCG-";
            QVERIFY(imp.importAlignment("dummy-dna-msa", pbss, eDnaAlphabet, QModelIndex()));

            // Verify that they were loaded into the database
            QVERIFY(s_dstrings.exec());
            QVERIFY(s_dstrings.next());
            QCOMPARE(s_dstrings.value(0).toInt(), 1);
            QCOMPARE(s_dstrings.value(1).toString(), QString("ATCG"));
            QVERIFY(s_dstrings.next());
            QCOMPARE(s_dstrings.value(0).toInt(), 2);
            QCOMPARE(s_dstrings.value(1).toString(), QString("TCGA"));
            QVERIFY(!s_dstrings.next());

            // Verify the dna_seqs and dna_subseqs table
            QSqlQuery s_dnaseqs = db.exec(QString("SELECT id, dstring_id, label FROM %1 ORDER BY id").arg(constants::kTableDnaSeqs));
            QVERIFY(s_dnaseqs.lastError().type() == QSqlError::NoError);

            QSqlQuery s_dnasubseqs(db);
            QVERIFY(s_dnasubseqs.prepare(QString("SELECT start, stop, sequence FROM %1 WHERE dna_seq_id = ?").arg(constants::kTableDnaSubseqs)));

            // Take the first dna seq
            QVERIFY(s_dnaseqs.next());
            QCOMPARE(s_dnaseqs.value(0).toInt(), 1);    // id == 1
            QCOMPARE(s_dnaseqs.value(1).toInt(), 1);    // dstring_id == 1
            QCOMPARE(s_dnaseqs.value(2).toString(), QString("Dna1"));

            s_dnasubseqs.bindValue(0, 1);   // dna_seq_id = 1
            QVERIFY(s_dnasubseqs.exec());
            QVERIFY(s_dnasubseqs.next());
            QCOMPARE(s_dnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_dnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_dnasubseqs.value(2).toString(), QString("AT-CG"));

            // Next dna seq
            QVERIFY(s_dnaseqs.next());
            QCOMPARE(s_dnaseqs.value(0).toInt(), 2);    // id == 2
            QCOMPARE(s_dnaseqs.value(1).toInt(), 1);    // dstring_id == 1 because same sequence as the first one
            QCOMPARE(s_dnaseqs.value(2).toString(), QString("Dna1.1"));

            s_dnasubseqs.bindValue(0, 2);   // dna_seq_id = 2
            QVERIFY(s_dnasubseqs.exec());
            QVERIFY(s_dnasubseqs.next());
            QCOMPARE(s_dnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_dnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_dnasubseqs.value(2).toString(), QString("ATCG-"));

            // Last dna seq
            QVERIFY(s_dnaseqs.next());
            QCOMPARE(s_dnaseqs.value(0).toInt(), 3);    // id == 3
            QCOMPARE(s_dnaseqs.value(1).toInt(), 2);    // dstring_id == 2
            QCOMPARE(s_dnaseqs.value(2).toString(), QString("Dna2"));
            QVERIFY(!s_dnaseqs.next());

            s_dnasubseqs.bindValue(0, 3);   // dna_seq_id = 2
            QVERIFY(s_dnasubseqs.exec());
            QVERIFY(s_dnasubseqs.next());
            QCOMPARE(s_dnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_dnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_dnasubseqs.value(2).toString(), QString("TCG-A"));

            // --------
            // Verify the dna_msas and dna_msas_subseqs tables
            QSqlQuery s_dnamsas = db.exec(QString("SELECT id FROM %1 ORDER BY id").arg(constants::kTableDnaMsas));
            QVERIFY(s_dnamsas.lastError().type() == QSqlError::NoError);

            QSqlQuery s_dnamsas_subseqs(db);
            QVERIFY(s_dnamsas_subseqs.prepare(QString("SELECT dna_subseq_id FROM %1 WHERE dna_msa_id = ? and dna_subseq_id = ?").arg(constants::kTableDnaMsaSubseqs)));

            QVERIFY(s_dnamsas.next());
            QCOMPARE(s_dnamsas.value(0).toInt(), 1);    // id == 1
            QVERIFY(!s_dnamsas.next());

            // Now check that the subseqs are in place
            s_dnamsas_subseqs.bindValue(0, 1);
            s_dnamsas_subseqs.bindValue(1, 1);
            QVERIFY(s_dnamsas_subseqs.exec());
            QVERIFY(s_dnamsas_subseqs.next());      // This verifies that the record is present
            s_dnamsas_subseqs.bindValue(0, 1);
            s_dnamsas_subseqs.bindValue(1, 2);
            QVERIFY(s_dnamsas_subseqs.exec());
            QVERIFY(s_dnamsas_subseqs.next());      // This verifies that the record is present
            s_dnamsas_subseqs.bindValue(0, 1);
            s_dnamsas_subseqs.bindValue(1, 3);
            QVERIFY(s_dnamsas_subseqs.exec());
            QVERIFY(s_dnamsas_subseqs.next());      // This verifies that the record is present


            // Verify that the three subseqs were placed beneath the root index
            QCOMPARE(treeModel->rowCount(), 1);
            AdocTreeNode *node = treeModel->data(treeModel->index(0, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::MsaDnaType);
            QCOMPARE(node->label_, QString("dummy-dna-msa"));
            QCOMPARE(node->fkId_, 1);
        }
        catch (DatabaseError &e) { QVERIFY(0); }



        // ---------------------------------------------------
        // Virtually identical to the above tests except for amino acid sequences
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableAminoSeqs + QString(" (id integer not null primary key autoincrement, "
                                                                                      "  astring_id integer not null,"
                                                                                      "  label text,"
                                                                                      "  organism text)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableAminoSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                         "  amino_seq_id integer not null,"
                                                                                         "  label text,"
                                                                                         "  start integer not null,"
                                                                                         "  stop integer not null,"
                                                                                         "  sequence text not null)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableAminoMsas + QString(" (id integer not null primary key autoincrement,"
                                                                                        "  name text not null,"
                                                                                        "  description text)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableAminoMsaSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                              "  amino_msa_id integer not null,"
                                                                                              "  amino_subseq_id integer not null)")).lastError().type() == QSqlError::NoError);

        pbss.clear();
        pbss.append(ParsedBioString("VYVF--DEKGR", "PAS-A", true, true));
        pbss.append(ParsedBioString("--VVFF", "PAS-B", false, true));
        pbss.append(ParsedBioString("VVFF--", "PAS-C", true, false));

        try
        {
            // Test: alignment with differing length of sequences
            QVERIFY(imp.importAlignment("dummy", pbss, eAminoAlphabet, QModelIndex()) == false);
            QSqlQuery s_astrings(db);
            QVERIFY(s_astrings.prepare(QString("SELECT id, sequence FROM %1 ORDER BY id").arg(constants::kTableAstrings)));
            QVERIFY(s_astrings.exec());
            QVERIFY(s_astrings.next() == false);

            pbss[0].bioString_ = "-VYVF-";

            // Verify that they were loaded into the database
            QVERIFY(imp.importAlignment("dummy-amino-msa", pbss, eAminoAlphabet, QModelIndex()));
            QVERIFY(s_astrings.exec());
            QVERIFY(s_astrings.next());
            QCOMPARE(s_astrings.value(0).toInt(), 1);
            QCOMPARE(s_astrings.value(1).toString(), QString("VYVF"));
            QVERIFY(s_astrings.next());
            QCOMPARE(s_astrings.value(0).toInt(), 2);
            QCOMPARE(s_astrings.value(1).toString(), QString("VVFF"));
            QVERIFY(!s_astrings.next());

            // Verify the amino_seqs and amino_subseqs table
            QSqlQuery s_aminoseqs = db.exec(QString("SELECT id, astring_id, label FROM %1 ORDER BY id").arg(constants::kTableAminoSeqs));
            QVERIFY(s_aminoseqs.lastError().type() == QSqlError::NoError);

            QSqlQuery s_aminosubseqs(db);
            QVERIFY(s_aminosubseqs.prepare(QString("SELECT start, stop, sequence FROM %1 WHERE amino_seq_id = ?").arg(constants::kTableAminoSubseqs)));

            // Take the first amino seq
            QVERIFY(s_aminoseqs.next());
            QCOMPARE(s_aminoseqs.value(0).toInt(), 1);    // id == 1
            QCOMPARE(s_aminoseqs.value(1).toInt(), 1);    // astring_id == 1
            QCOMPARE(s_aminoseqs.value(2).toString(), QString("PAS-A"));

            s_aminosubseqs.bindValue(0, 1);   // amino_seq_id = 1
            QVERIFY(s_aminosubseqs.exec());
            QVERIFY(s_aminosubseqs.next());
            QCOMPARE(s_aminosubseqs.value(0).toInt(), 1);
            QCOMPARE(s_aminosubseqs.value(1).toInt(), 4);
            QCOMPARE(s_aminosubseqs.value(2).toString(), QString("-VYVF-"));

            // Next amino seq
            QVERIFY(s_aminoseqs.next());
            QCOMPARE(s_aminoseqs.value(0).toInt(), 2);    // id == 2
            QCOMPARE(s_aminoseqs.value(1).toInt(), 2);
            QCOMPARE(s_aminoseqs.value(2).toString(), QString("PAS-B"));

            s_aminosubseqs.bindValue(0, 2);   // amino_seq_id = 2
            QVERIFY(s_aminosubseqs.exec());
            QVERIFY(s_aminosubseqs.next());
            QCOMPARE(s_aminosubseqs.value(0).toInt(), 1);
            QCOMPARE(s_aminosubseqs.value(1).toInt(), 4);
            QCOMPARE(s_aminosubseqs.value(2).toString(), QString("--VVFF"));

            // Last amino seq
            QVERIFY(s_aminoseqs.next());
            QCOMPARE(s_aminoseqs.value(0).toInt(), 3);    // id == 3
            QCOMPARE(s_aminoseqs.value(1).toInt(), 2);    // astring_id == 2
            QCOMPARE(s_aminoseqs.value(2).toString(), QString("PAS-C"));
            QVERIFY(!s_aminoseqs.next());

            s_aminosubseqs.bindValue(0, 3);   // amino_seq_id = 2
            QVERIFY(s_aminosubseqs.exec());
            QVERIFY(s_aminosubseqs.next());
            QCOMPARE(s_aminosubseqs.value(0).toInt(), 1);
            QCOMPARE(s_aminosubseqs.value(1).toInt(), 4);
            QCOMPARE(s_aminosubseqs.value(2).toString(), QString("VVFF--"));

            // --------
            // Verify the amino_msas and amino_msas_subseqs tables
            QSqlQuery s_aminomsas = db.exec(QString("SELECT id FROM %1 ORDER BY id").arg(constants::kTableAminoMsas));
            QVERIFY(s_aminomsas.lastError().type() == QSqlError::NoError);

            QSqlQuery s_aminomsas_subseqs(db);
            QVERIFY(s_aminomsas_subseqs.prepare(QString("SELECT amino_subseq_id FROM %1 WHERE amino_msa_id = ? and amino_subseq_id = ?").arg(constants::kTableAminoMsaSubseqs)));

            QVERIFY(s_aminomsas.next());
            QCOMPARE(s_aminomsas.value(0).toInt(), 1);    // id == 1
            QVERIFY(!s_aminomsas.next());

            // Now check that the subseqs are in place
            s_aminomsas_subseqs.bindValue(0, 1);
            s_aminomsas_subseqs.bindValue(1, 1);
            QVERIFY(s_aminomsas_subseqs.exec());
            QVERIFY(s_aminomsas_subseqs.next());      // This verifies that the record is present
            s_aminomsas_subseqs.bindValue(0, 1);
            s_aminomsas_subseqs.bindValue(1, 2);
            QVERIFY(s_aminomsas_subseqs.exec());
            QVERIFY(s_aminomsas_subseqs.next());      // This verifies that the record is present
            s_aminomsas_subseqs.bindValue(0, 1);
            s_aminomsas_subseqs.bindValue(1, 3);
            QVERIFY(s_aminomsas_subseqs.exec());
            QVERIFY(s_aminomsas_subseqs.next());      // This verifies that the record is present

            // Verify that the three subseqs were placed beneath the root index
            QCOMPARE(treeModel->rowCount(), 2);
            AdocTreeNode *node = treeModel->data(treeModel->index(1, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::MsaAminoType);
            QCOMPARE(node->label_, QString("dummy-amino-msa"));
            QCOMPARE(node->fkId_, 1);
        }
        catch (DatabaseError &e) { QVERIFY(0); }


        // ---------------------------------------------------
        // Same tests as above except with Rna sequences
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableRnaSeqs + QString(" (id integer not null primary key autoincrement, "
                                                                                      "  rstring_id integer not null,"
                                                                                      "  label text,"
                                                                                      "  organism text)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableRnaSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                         "  rna_seq_id integer not null,"
                                                                                         "  label text,"
                                                                                         "  start integer not null,"
                                                                                         "  stop integer not null,"
                                                                                         "  sequence text not null)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableRnaMsas + QString(" (id integer not null primary key autoincrement,"
                                                                                      "  name text not null,"
                                                                                      "  description text)")).lastError().type() == QSqlError::NoError);
        QVERIFY(db.exec(QString("CREATE TABLE ") + constants::kTableRnaMsaSubseqs + QString(" (id integer not null primary key autoincrement,"
                                                                                            "  rna_msa_id integer not null,"
                                                                                            "  rna_subseq_id integer not null)")).lastError().type() == QSqlError::NoError);

        pbss.clear();
        pbss.append(ParsedBioString("AU-CG", "Rna1", true, true));
        pbss.append(ParsedBioString("AUCG--", "Rna1.1", false, true));
        pbss.append(ParsedBioString("UCG-A", "Rna2", true, false));

        DbAnonSeqFactory *rnaAnonSeqFactory = new DbAnonSeqFactory();
        QVERIFY(rnaAnonSeqFactory->setDataSource("test_db", "rstrings"));
        factories_[eRnaAlphabet] = rnaAnonSeqFactory;
        imp.setAnonSeqFactories(factories_);

        try
        {
            // Test: invalid sequence length of alignment
            QVERIFY(imp.importAlignment("dummy", pbss, eRnaAlphabet, QModelIndex()) == false);
            QSqlQuery s_rstrings(db);
            QVERIFY(s_rstrings.prepare(QString("SELECT id, sequence FROM %1 ORDER BY id").arg(constants::kTableRstrings)));
            QVERIFY(s_rstrings.exec());
            QVERIFY(!s_rstrings.next());

            // Test: valid sequence length
            pbss[1].bioString_ = "AUCG-";
            QVERIFY(imp.importAlignment("dummy-rna-msa", pbss, eRnaAlphabet, QModelIndex()));

            // Verify that they were loaded into the database
            QVERIFY(s_rstrings.exec());
            QVERIFY(s_rstrings.next());
            QCOMPARE(s_rstrings.value(0).toInt(), 1);
            QCOMPARE(s_rstrings.value(1).toString(), QString("AUCG"));
            QVERIFY(s_rstrings.next());
            QCOMPARE(s_rstrings.value(0).toInt(), 2);
            QCOMPARE(s_rstrings.value(1).toString(), QString("UCGA"));
            QVERIFY(!s_rstrings.next());

            // Verify the rna_seqs and rna_subseqs table
            QSqlQuery s_rnaseqs = db.exec(QString("SELECT id, rstring_id, label FROM %1 ORDER BY id").arg(constants::kTableRnaSeqs));
            QVERIFY(s_rnaseqs.lastError().type() == QSqlError::NoError);

            QSqlQuery s_rnasubseqs(db);
            QVERIFY(s_rnasubseqs.prepare(QString("SELECT start, stop, sequence FROM %1 WHERE rna_seq_id = ?").arg(constants::kTableRnaSubseqs)));

            // Take the first rna seq
            QVERIFY(s_rnaseqs.next());
            QCOMPARE(s_rnaseqs.value(0).toInt(), 1);    // id == 1
            QCOMPARE(s_rnaseqs.value(1).toInt(), 1);    // rstring_id == 1
            QCOMPARE(s_rnaseqs.value(2).toString(), QString("Rna1"));

            s_rnasubseqs.bindValue(0, 1);   // rna_seq_id = 1
            QVERIFY(s_rnasubseqs.exec());
            QVERIFY(s_rnasubseqs.next());
            QCOMPARE(s_rnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_rnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_rnasubseqs.value(2).toString(), QString("AU-CG"));

            // Next rna seq
            QVERIFY(s_rnaseqs.next());
            QCOMPARE(s_rnaseqs.value(0).toInt(), 2);    // id == 2
            QCOMPARE(s_rnaseqs.value(1).toInt(), 1);    // rstring_id == 1 because same sequence as the first one
            QCOMPARE(s_rnaseqs.value(2).toString(), QString("Rna1.1"));

            s_rnasubseqs.bindValue(0, 2);   // rna_seq_id = 2
            QVERIFY(s_rnasubseqs.exec());
            QVERIFY(s_rnasubseqs.next());
            QCOMPARE(s_rnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_rnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_rnasubseqs.value(2).toString(), QString("AUCG-"));

            // Last rna seq
            QVERIFY(s_rnaseqs.next());
            QCOMPARE(s_rnaseqs.value(0).toInt(), 3);    // id == 3
            QCOMPARE(s_rnaseqs.value(1).toInt(), 2);    // dstring_id == 2
            QCOMPARE(s_rnaseqs.value(2).toString(), QString("Rna2"));
            QVERIFY(!s_rnaseqs.next());

            s_rnasubseqs.bindValue(0, 3);   // rna_seq_id = 2
            QVERIFY(s_rnasubseqs.exec());
            QVERIFY(s_rnasubseqs.next());
            QCOMPARE(s_rnasubseqs.value(0).toInt(), 1);
            QCOMPARE(s_rnasubseqs.value(1).toInt(), 4);
            QCOMPARE(s_rnasubseqs.value(2).toString(), QString("UCG-A"));


            // --------
            // Verify the rna_msas and rna_msas_subseqs tables
            QSqlQuery s_rnamsas = db.exec(QString("SELECT id FROM %1 ORDER BY id").arg(constants::kTableRnaMsas));
            QVERIFY(s_rnamsas.lastError().type() == QSqlError::NoError);

            QSqlQuery s_rnamsas_subseqs(db);
            QVERIFY(s_rnamsas_subseqs.prepare(QString("SELECT rna_subseq_id FROM %1 WHERE rna_msa_id = ? and rna_subseq_id = ?").arg(constants::kTableRnaMsaSubseqs)));

            QVERIFY(s_rnamsas.next());
            QCOMPARE(s_rnamsas.value(0).toInt(), 1);    // id == 1
            QVERIFY(!s_rnamsas.next());

            // Now check that the subseqs are in place
            s_rnamsas_subseqs.bindValue(0, 1);
            s_rnamsas_subseqs.bindValue(1, 1);
            QVERIFY(s_rnamsas_subseqs.exec());
            QVERIFY(s_rnamsas_subseqs.next());      // This verifies that the record is present
            s_rnamsas_subseqs.bindValue(0, 1);
            s_rnamsas_subseqs.bindValue(1, 2);
            QVERIFY(s_rnamsas_subseqs.exec());
            QVERIFY(s_rnamsas_subseqs.next());      // This verifies that the record is present
            s_rnamsas_subseqs.bindValue(0, 1);
            s_rnamsas_subseqs.bindValue(1, 3);
            QVERIFY(s_rnamsas_subseqs.exec());
            QVERIFY(s_rnamsas_subseqs.next());      // This verifies that the record is present

            // Verify that the three subseqs were placed beneath the root index
            QCOMPARE(treeModel->rowCount(), 3);
            AdocTreeNode *node = treeModel->data(treeModel->index(2, 0), AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>();
            QVERIFY(node->nodeType_ == AdocTreeNode::MsaRnaType);
            QCOMPARE(node->label_, QString("dummy-rna-msa"));
            QCOMPARE(node->fkId_, 1);
        }
        catch (DatabaseError &e) { QVERIFY(0); }
    }
    catch (...) { QVERIFY(0); }

    QSqlDatabase::removeDatabase("test_db");
}

QTEST_MAIN(TestSequenceImporter)
#include "TestSequenceImporter.moc"
