/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "AdocTreeModel.h"

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>


class TestAdocTreeModel : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void root();
    void setRoot();
    void index();
    void nodeFromIndex();
    void indexFromNode();
    void columnCount();
    void rowCount();
    void parent();
    void data();
    void setData();

    void appendRow();
    void insertRow();
    void removeRows();

    void isValidChildType();
    void newGroup();

    void getIdsByNodeType();


    // Untested:
    // void flags()
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestAdocTreeModel::constructor()
{
    AdocTreeModel model;

    QObject *qObject = new QObject();
    new AdocTreeModel(qObject);
    delete qObject;
}

void TestAdocTreeModel::root()
{
    AdocTreeModel model;

    AdocTreeNode *root = model.root();
    QVERIFY(root);
    QVERIFY(root->childCount() == 0);
    QVERIFY(root->parent() == 0);
}

void TestAdocTreeModel::setRoot()
{
    AdocTreeModel model;

    AdocTreeNode *old_root = model.root();

    // Test: set invalid root
    QVERIFY(model.setRoot(0) == false);
    QVERIFY(model.root() == old_root);

    // Test: set to new valid root
    AdocTreeNode *new_root = new AdocTreeNode(AdocTreeNode::RootType, "root node");
    QVERIFY(model.setRoot(new_root));
    QVERIFY(model.root() == new_root);
}

void TestAdocTreeModel::index()
{
    AdocTreeModel model;

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    AdocTreeNode *root = model.root();
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Alpha"));
    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::GroupType, "Beta");
    root->appendChild(beta);
    beta->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Gamma"));

    QModelIndex root_index; // Root index in a model is simply an invalid QModelIndex

    // Test: root child indices 0 and 1 should be valid, others invalid
    QModelIndex alpha_index = model.index(0, 0, root_index);
    QVERIFY(alpha_index.isValid());

    QModelIndex beta_index = model.index(1, 0, root_index);
    QVERIFY(beta_index.isValid());

    QVERIFY(model.index(2, 0, root_index).isValid() == false);

    // Test: Alpha has no child indices
    QVERIFY(model.index(0, 0, alpha_index).isValid() == false);

    // Test: Beta has one child
    QVERIFY(model.index(0, 0, beta_index).isValid());
    QVERIFY(model.index(0, 0, beta_index) == beta_index.child(0, 0));

    QVERIFY(model.index(1, 0, beta_index).isValid() == false);
}

void TestAdocTreeModel::nodeFromIndex()
{
    AdocTreeModel model;

    // Test: AdocTreeNode *root == nodeFromIndex(root_index)
    QVERIFY(model.root() == model.nodeFromIndex(QModelIndex()));

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    AdocTreeNode *root = model.root();
    AdocTreeNode *alpha = new AdocTreeNode(AdocTreeNode::GroupType, "Alpha");
    root->appendChild(alpha);
    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::GroupType, "Beta");
    root->appendChild(beta);
    AdocTreeNode *gamma = new AdocTreeNode(AdocTreeNode::GroupType, "Gamma");
    beta->appendChild(gamma);

    QVERIFY(model.nodeFromIndex(model.index(0, 0, QModelIndex())) == alpha);
    QModelIndex beta_index = model.index(1, 0, QModelIndex());
    QVERIFY(model.nodeFromIndex(beta_index) == beta);
    QVERIFY(model.nodeFromIndex(model.index(0, 0, beta_index)) == gamma);

    // Test: If we pass an index from a different model, it should return null.
    AdocTreeModel model2;
    model2.root()->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Group"));
    QVERIFY(model.nodeFromIndex(model2.index(0, 0)) == 0);
}

void TestAdocTreeModel::indexFromNode()
{
    AdocTreeModel model;

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    AdocTreeNode *root = model.root();
    AdocTreeNode *alpha = new AdocTreeNode(AdocTreeNode::GroupType, "Alpha");
    root->appendChild(alpha);
    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::GroupType, "Beta");
    root->appendChild(beta);
    AdocTreeNode *gamma = new AdocTreeNode(AdocTreeNode::GroupType, "Gamma");
    beta->appendChild(gamma);

    QVERIFY(model.indexFromNode(root) == QModelIndex());
    QVERIFY(model.indexFromNode(alpha) == model.index(0, 0));
    QVERIFY(model.indexFromNode(beta) == model.index(1, 0));
    QVERIFY(model.indexFromNode(gamma) == model.index(0, 0, model.index(1, 0)));
}

void TestAdocTreeModel::columnCount()
{
    AdocTreeModel model;

    // Test: column count for root node should be 1
    QVERIFY(model.columnCount() == 1);      // Default should be root
    QVERIFY(model.columnCount(QModelIndex()) == 1);

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    AdocTreeNode *root = model.root();
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Alpha"));
    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::GroupType, "Beta");
    root->appendChild(beta);
    beta->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Gamma"));

    // Test: column count should be 1 for every model index
    QVERIFY(model.columnCount(model.index(0, 0, QModelIndex())) == 1);
    QModelIndex beta_index = model.index(1, 0, QModelIndex());
    QVERIFY(model.columnCount(beta_index) == 1);
    QVERIFY(model.columnCount(model.index(0, 0, beta_index)) == 1);
}

void TestAdocTreeModel::rowCount()
{
    AdocTreeModel model;

    // Test: column count for root node should be 1
    QVERIFY(model.rowCount() == 0);     // default should be root
    QVERIFY(model.rowCount(QModelIndex()) == 0);

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    AdocTreeNode *root = model.root();
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Alpha"));
    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::GroupType, "Beta");
    root->appendChild(beta);
    beta->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Gamma"));

    // Test: column count should be 1 for every model index
    QVERIFY(model.rowCount(model.index(0, 0, QModelIndex())) == 0);
    QModelIndex beta_index = model.index(1, 0, QModelIndex());
    QVERIFY(model.rowCount(beta_index) == 1);
    QVERIFY(model.rowCount(model.index(0, 0, beta_index)) == 0);
}

void TestAdocTreeModel::parent()
{
    AdocTreeModel model;

    // Test: column count for root node should be 1
    QVERIFY(model.rowCount(QModelIndex()) == 0);

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    AdocTreeNode *root = model.root();
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Alpha"));
    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::GroupType, "Beta");
    root->appendChild(beta);
    beta->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Gamma"));

    QModelIndex root_index = QModelIndex();
    QModelIndex alpha_index = model.index(0, 0, root_index);
    QModelIndex beta_index = model.index(1, 0, root_index);
    QModelIndex gamma_index = model.index(0, 0, beta_index);

    QVERIFY(model.parent(root_index).isValid() == false);
    QVERIFY(model.parent(alpha_index) == root_index);
    QVERIFY(model.parent(beta_index) == root_index);
    QVERIFY(model.parent(gamma_index) == beta_index);
}

void TestAdocTreeModel::data()
{
    AdocTreeModel model;

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    AdocTreeNode *root = model.root();
    AdocTreeNode *alpha = new AdocTreeNode(AdocTreeNode::GroupType, "Alpha");
    root->appendChild(alpha);
    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::GroupType, "Beta");
    root->appendChild(beta);
    AdocTreeNode *gamma = new AdocTreeNode(AdocTreeNode::GroupType, "Gamma");
    beta->appendChild(gamma);

    QModelIndex root_index = QModelIndex();
    QModelIndex alpha_index = model.index(0, 0, root_index);
    QModelIndex beta_index = model.index(1, 0, root_index);
    QModelIndex gamma_index = model.index(0, 0, beta_index);

    // Test: Qt::DisplayRole
    QVERIFY(model.data(root_index, Qt::DisplayRole).isNull());
    QVERIFY(model.data(alpha_index, Qt::DisplayRole).toString() == "Alpha");
    QVERIFY(model.data(beta_index, Qt::DisplayRole).toString() == "Beta");
    QVERIFY(model.data(gamma_index, Qt::DisplayRole).toString() == "Gamma");

    // Test: AdocTreeModel::NodeTypeRole
    QCOMPARE(model.data(root_index, AdocTreeModel::NodeTypeRole).value<AdocTreeNode::NodeType>(), AdocTreeNode::RootType);
    QCOMPARE(model.data(alpha_index, AdocTreeModel::NodeTypeRole).value<AdocTreeNode::NodeType>(), AdocTreeNode::GroupType);
    QCOMPARE(model.data(beta_index, AdocTreeModel::NodeTypeRole).value<AdocTreeNode::NodeType>(), AdocTreeNode::GroupType);
    QCOMPARE(model.data(gamma_index, AdocTreeModel::NodeTypeRole).value<AdocTreeNode::NodeType>(), AdocTreeNode::GroupType);

    // Test: AdocTreeModel::AdocTreeNodeRole
    QVERIFY(model.data(root_index, AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>() == root);
    QVERIFY(model.data(alpha_index, AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>() == alpha);
    QVERIFY(model.data(beta_index, AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>() == beta);
    QVERIFY(model.data(gamma_index, AdocTreeModel::AdocTreeNodeRole).value<AdocTreeNode *>() == gamma);
}

void TestAdocTreeModel::setData()
{
    AdocTreeModel model;

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha

    AdocTreeNode *root = model.root();
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Alpha"));

    QModelIndex root_index = QModelIndex();
    QModelIndex alpha_index = model.index(0, 0, root_index);

    // Test: unable to setData on the root node
    QVERIFY(model.setData(root_index, "Rootage", Qt::EditRole) == false);

    // Test: setData on the alpha node
    QVERIFY(model.data(alpha_index, Qt::DisplayRole).toString() == "Alpha");
    QVERIFY(model.setData(alpha_index, "ahpla", Qt::EditRole));
    QVERIFY(model.data(alpha_index, Qt::DisplayRole).toString() == "ahpla");
}

void TestAdocTreeModel::appendRow()
{
    AdocTreeModel model;

    // Test: create the following tree
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    // Test: alpha
    QVERIFY(model.appendRow(new AdocTreeNode(AdocTreeNode::GroupType, "Alpha"), QModelIndex()));
    QVERIFY(model.rowCount() == 1);
    QVERIFY(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Alpha");

    // Test: beta
    QVERIFY(model.appendRow(new AdocTreeNode(AdocTreeNode::GroupType, "Beta")));
    QVERIFY(model.rowCount() == 2);
    QVERIFY(model.data(model.index(1, 0), Qt::DisplayRole).toString() == "Beta");

    // Test: gamma
    QModelIndex beta_index = model.index(1, 0);
    QVERIFY(model.rowCount(beta_index) == 0);
    QVERIFY(model.appendRow(new AdocTreeNode(AdocTreeNode::GroupType, "Gamma"), beta_index));
    QVERIFY(model.rowCount(beta_index) == 1);
    QVERIFY(model.data(model.index(0, 0, beta_index), Qt::DisplayRole).toString() == "Gamma");
}

void TestAdocTreeModel::insertRow()
{
    AdocTreeModel model;

    // Test: create the following tree
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma
    // |___ Zeta
    // |___ Delta

    // Test: insert into empty list
    QVERIFY(model.insertRow(0, new AdocTreeNode(AdocTreeNode::GroupType, "Beta")));
    QVERIFY(model.rowCount() == 1);
    QVERIFY(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Beta");

    // Test: insert at beginning of list
    QVERIFY(model.insertRow(0, new AdocTreeNode(AdocTreeNode::GroupType, "Alpha"), QModelIndex()));
    QVERIFY(model.rowCount() == 2);
    QVERIFY(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Alpha");
    QVERIFY(model.data(model.index(1, 0), Qt::DisplayRole).toString() == "Beta");

    // Test: insert at end of list
    QVERIFY(model.insertRow(2, new AdocTreeNode(AdocTreeNode::GroupType, "Delta")));
    QVERIFY(model.rowCount() == 3);
    QVERIFY(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Alpha");
    QVERIFY(model.data(model.index(1, 0), Qt::DisplayRole).toString() == "Beta");
    QVERIFY(model.data(model.index(2, 0), Qt::DisplayRole).toString() == "Delta");

    // Test: insert into middle of list
    QVERIFY(model.insertRow(2, new AdocTreeNode(AdocTreeNode::GroupType, "Zeta"), QModelIndex()));
    QVERIFY(model.rowCount() == 4);
    QVERIFY(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Alpha");
    QVERIFY(model.data(model.index(1, 0), Qt::DisplayRole).toString() == "Beta");
    QVERIFY(model.data(model.index(2, 0), Qt::DisplayRole).toString() == "Zeta");
    QVERIFY(model.data(model.index(3, 0), Qt::DisplayRole).toString() == "Delta");

    // Test: insert on child node
    QVERIFY(model.insertRow(0, new AdocTreeNode(AdocTreeNode::GroupType, "Gamma"), model.index(1, 0)));
    QVERIFY(model.rowCount() == 4);
    QVERIFY(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Alpha");
    QVERIFY(model.data(model.index(1, 0), Qt::DisplayRole).toString() == "Beta");
    QVERIFY(model.data(model.index(2, 0), Qt::DisplayRole).toString() == "Zeta");
    QVERIFY(model.data(model.index(3, 0), Qt::DisplayRole).toString() == "Delta");

    QVERIFY(model.rowCount(model.index(1, 0)) == 1);
    QVERIFY(model.data(model.index(0, 0, model.index(1, 0)), Qt::DisplayRole).toString() == "Gamma");
}

void TestAdocTreeModel::removeRows()
{
    AdocTreeModel model;

    // Setup: create the following tree for testing purposes
    // Root
    // |___ Alpha
    // |___ Beta
    //      |___ Gamma

    AdocTreeNode *root = model.root();
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Alpha"));
    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::GroupType, "Beta");
    root->appendChild(beta);
    beta->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "Gamma"));

    // Test: remove 0 rows from the root
    QVERIFY(model.removeRows(0, 0));
    QVERIFY(model.rowCount() == 2);

    // Test: remove 0 rows from specific non-root
    QVERIFY(model.removeRows(0, 0, model.index(1, 0)));
    QVERIFY(model.rowCount(model.index(1, 0)) == 1);

    // Test: remove 1 row from root - alpha
    QVERIFY(model.removeRows(0, 1));
    QVERIFY(model.rowCount() == 1);
    QVERIFY2(model.data(model.index(0, 0), Qt::DisplayRole).toString() == "Beta", model.data(model.index(0, 0), Qt::DisplayRole).toString().toAscii());

    // Test: remove 1 row from specific non-root
    QVERIFY(model.removeRows(0, 1, model.index(0, 0)));
    QVERIFY(model.rowCount(model.index(0, 0)) == 0);

    // Test: remove 2 rows
    model.appendRow(new AdocTreeNode(AdocTreeNode::GroupType, "Delta"));
    QVERIFY(model.removeRows(0, 2));
    QVERIFY(model.rowCount() == 0);

    // Test: removeRows with noDeleteOnRemove configured
    //
    // [18 November 2010] Do not remember what the noDeleteOnRemove was for. I think it has something
    //                    to do with not erasing records from the data source when a node is deleted...
    /*
    AdocTreeNode *zeta = new AdocTreeNode(AdocTreeNode::GroupType, "Zeta");
    model.appendRow(zeta);
    QCOMPARE(model.rowCount(), 1);
    model.noDeleteOnRemove_ << zeta;
    model.removeRow(0, QModelIndex());
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.noDeleteOnRemove_.isEmpty());
    QCOMPARE(zeta->label_, QString("Zeta"));

    // At this point, the root tree node still exists, but does not have any children.
    // zeta also exists and is valid because we added it to the noDeleteOnRemove_ set
    // before calling remove rows. The only issue is that zeta still has its parent
    // pointer set to root. Thus, deleting the node without first setting the parent
    // to zero will cause an assertion failure because the root node will node find
    // zeta in its child list.
    zeta->parent_ = 0;
    delete zeta;
    zeta = 0;
    */
}

void TestAdocTreeModel::isValidChildType()
{
    AdocTreeModel model;

    // Test: Undefined, root, and all subseqs are not allowed under the root node
    QVERIFY(model.isValidChildType(AdocTreeNode::UndefinedType, QModelIndex()) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::RootType, QModelIndex()) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqAminoType, QModelIndex()) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqDnaType, QModelIndex()) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqRnaType, QModelIndex()) == false);

    QVERIFY(model.isValidChildType(AdocTreeNode::GroupType, QModelIndex()));
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqAminoType, QModelIndex()));
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqDnaType, QModelIndex()));
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqRnaType, QModelIndex()));
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaAminoType, QModelIndex()));
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaDnaType, QModelIndex()));
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaRnaType, QModelIndex()));
    QVERIFY(model.isValidChildType(AdocTreeNode::PrimerType, QModelIndex()));

    // Add a group under root
    model.appendRow(new AdocTreeNode(AdocTreeNode::GroupType, "Group A"));
    QModelIndex groupIndex = model.index(0, 0);

    // Test: Undefined, root, and all subseqs are not allowed under a group
    QVERIFY(model.isValidChildType(AdocTreeNode::UndefinedType, groupIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::RootType, groupIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqAminoType, groupIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqDnaType, groupIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqRnaType, groupIndex) == false);

    QVERIFY(model.isValidChildType(AdocTreeNode::GroupType, groupIndex));
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqAminoType, groupIndex));
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqDnaType, groupIndex));
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqRnaType, groupIndex));
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaAminoType, groupIndex));
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaDnaType, groupIndex));
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaRnaType, groupIndex));
    QVERIFY(model.isValidChildType(AdocTreeNode::PrimerType, groupIndex));

    // Add a seq Amino under root
    model.appendRow(new AdocTreeNode(AdocTreeNode::SeqAminoType, "Amino seq"));
    QModelIndex seqAminoIndex = model.index(1, 0);

    // Test: Only SubseqAmino allowed under seqamino
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqAminoType, seqAminoIndex));

    QVERIFY(model.isValidChildType(AdocTreeNode::UndefinedType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::RootType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::GroupType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqAminoType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqDnaType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqRnaType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqDnaType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqRnaType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaAminoType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaDnaType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaRnaType, seqAminoIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::PrimerType, seqAminoIndex) == false);

    // Add a seq DNA under project
    model.appendRow(new AdocTreeNode(AdocTreeNode::SeqDnaType, "Dna seq"));
    QModelIndex seqDnaIndex = model.index(2, 0);

    // Test: Only SubseqDna allowed under seqdna
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqDnaType, seqDnaIndex));

    QVERIFY(model.isValidChildType(AdocTreeNode::UndefinedType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::RootType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::GroupType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqAminoType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqDnaType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqRnaType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqAminoType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqRnaType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaAminoType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaDnaType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaRnaType, seqDnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::PrimerType, seqDnaIndex) == false);

    // Add a seq RNA under project
    model.appendRow(new AdocTreeNode(AdocTreeNode::SeqRnaType, "Rna seq"));
    QModelIndex seqRnaIndex = model.index(3, 0);

    // Test: Only SubseqRna allowed under seqrna
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqRnaType, seqRnaIndex));

    QVERIFY(model.isValidChildType(AdocTreeNode::UndefinedType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::RootType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::GroupType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqAminoType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqRnaType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SeqRnaType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqAminoType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::SubseqDnaType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaAminoType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaRnaType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::MsaRnaType, seqRnaIndex) == false);
    QVERIFY(model.isValidChildType(AdocTreeNode::PrimerType, seqRnaIndex) == false);
}

void TestAdocTreeModel::newGroup()
{
    AdocTreeModel model;

    // Test: adding group to root should not work, because it is not a valid child descendant
    QVERIFY(model.newGroup("Group A", QModelIndex()).isValid());
    QVERIFY(model.rowCount() == 1);

    model.appendRow(new AdocTreeNode(AdocTreeNode::GroupType, "Group B"));

    // Test: Add new group beneath a group
    QVERIFY(model.newGroup("Group B", model.index(0, 0)).isValid());
    QVERIFY(model.rowCount(model.index(0, 0)) == 1);
    QCOMPARE(model.data(model.index(0, 0).child(0, 0), Qt::DisplayRole).toString(), QString("Group B"));
}

void TestAdocTreeModel::getIdsByNodeType()
{
    AdocTreeModel model;

    // Test: empty list of nodes
    QVERIFY(model.getIdsByNodeType(QList<QModelIndex>()).isEmpty());

    // Test: empty tree should return empty hash
    QVERIFY(model.getIdsByNodeType(QList<QModelIndex>() << QModelIndex()).isEmpty());

    // Setup basic test tree
    // Root
    // |___ Domains (Group)
    //      |___ Alpha (Group)
    //           |___ Amino [id = 1] (Subseq Amino)
    //           |___ Dna [id = 2] (Subseq DNA)
    //           |___ Rna [id = 3] (Subseq RNA)
    //      |___ Primer [id = 4] (Primer)
    //      |___ Amino2 [id = 5] (Subseq Amino)
    // |___ Rna2 [id = 6] (Subseq RNA)
    // |___ MsaAmino [id = 7]
    //      |___ Amino3 [id = 8]
    //      |___ Amino4 [id = 9]
    // |___ Delta (group)
    //     |___ MsaRna [id = 10]
    //          |___ Rna3 [id = 11]
    //          |___ Rna4 [id = 12]
    //     |___ MsaDna [id = 13]
    //          |___ Dna2 [id = 14]
    //          |___ Dna3 [id = 15]

    AdocTreeNode *domains = new AdocTreeNode(AdocTreeNode::GroupType, "Domains");
    AdocTreeNode *alpha = new AdocTreeNode(AdocTreeNode::GroupType, "Alpha");
    AdocTreeNode *amino = new AdocTreeNode(AdocTreeNode::SubseqAminoType, "Amino", 1);
    AdocTreeNode *dna = new AdocTreeNode(AdocTreeNode::SubseqDnaType, "Dna", 2);
    AdocTreeNode *rna = new AdocTreeNode(AdocTreeNode::SubseqRnaType, "Rna", 3);
    AdocTreeNode *primer = new AdocTreeNode(AdocTreeNode::PrimerType, "Primer", 4);
    AdocTreeNode *amino2 = new AdocTreeNode(AdocTreeNode::SubseqAminoType, "Amino2", 5);
    AdocTreeNode *rna2 = new AdocTreeNode(AdocTreeNode::SubseqRnaType, "Rna2", 6);
    AdocTreeNode *msaAmino = new AdocTreeNode(AdocTreeNode::MsaAminoType, "MsaAmino", 7);
    AdocTreeNode *amino3 = new AdocTreeNode(AdocTreeNode::SubseqAminoType, "Amino3", 8);
    AdocTreeNode *amino4 = new AdocTreeNode(AdocTreeNode::SubseqAminoType, "Amino4", 9);
    AdocTreeNode *delta = new AdocTreeNode(AdocTreeNode::GroupType, "Delta");
    AdocTreeNode *msaRna = new AdocTreeNode(AdocTreeNode::MsaRnaType, "MsaRna", 10);
    AdocTreeNode *rna3 = new AdocTreeNode(AdocTreeNode::SubseqRnaType, "Rna3", 11);
    AdocTreeNode *rna4 = new AdocTreeNode(AdocTreeNode::SubseqRnaType, "Rna4", 12);
    AdocTreeNode *msaDna = new AdocTreeNode(AdocTreeNode::MsaDnaType, "MsaDna", 13);
    AdocTreeNode *dna2 = new AdocTreeNode(AdocTreeNode::SubseqDnaType, "Dna2", 14);
    AdocTreeNode *dna3 = new AdocTreeNode(AdocTreeNode::SubseqDnaType, "Dna3", 15);

    model.appendRow(domains);
    domains->appendChild(alpha);
    alpha->appendChild(amino);
    alpha->appendChild(dna);
    alpha->appendChild(rna);
    domains->appendChild(primer);
    domains->appendChild(amino2);

    model.appendRow(rna2);

    model.appendRow(msaAmino);
    msaAmino->appendChild(amino3);
    msaAmino->appendChild(amino4);

    model.appendRow(delta);
    delta->appendChild(msaRna);
    msaRna->appendChild(rna3);
    msaRna->appendChild(rna4);
    delta->appendChild(msaDna);
    msaDna->appendChild(dna2);
    msaDna->appendChild(dna3);

    QModelIndex domainsIndex = model.index(0, 0);
    QModelIndex alphaIndex = domainsIndex.child(0, 0);
    QModelIndex aminoIndex = alphaIndex.child(0, 0);
    QModelIndex dnaIndex = alphaIndex.child(1, 0);
    QModelIndex rnaIndex = alphaIndex.child(2, 0);
    QModelIndex primerIndex = domainsIndex.child(1, 0);
    QModelIndex amino2Index = domainsIndex.child(2, 0);

    QModelIndex rna2Index = model.index(1, 0);

    QModelIndex msaAminoIndex = model.index(2, 0);
    QModelIndex amino3Index = msaAminoIndex.child(0, 0);
    QModelIndex amino4Index = msaAminoIndex.child(1, 0);

    QModelIndex deltaIndex = model.index(3, 0);
    QModelIndex msaRnaIndex = deltaIndex.child(0, 0);
    QModelIndex rna3Index = msaRnaIndex.child(0, 0);
    QModelIndex rna4Index = msaRnaIndex.child(1, 0);
    QModelIndex msaDnaIndex = deltaIndex.child(1, 0);
    QModelIndex dna2Index = msaDnaIndex.child(0, 0);
    QModelIndex dna3Index = msaDnaIndex.child(1, 0);

    // ---------------------------------------------------
    // Suite: single parent in the list
    // Test: root
    QHash<AdocTreeNode::NodeType, QStringList> result;
    result = model.getIdsByNodeType(QList<QModelIndex>() << QModelIndex());
    QCOMPARE(result.count(), 7);
    QVERIFY(result.contains(AdocTreeNode::SubseqAminoType));
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QVERIFY(result.contains(AdocTreeNode::PrimerType));
    QVERIFY(result.contains(AdocTreeNode::MsaAminoType));
    QVERIFY(result.contains(AdocTreeNode::MsaRnaType));
    QVERIFY(result.contains(AdocTreeNode::MsaDnaType));

    QCOMPARE(result[AdocTreeNode::SubseqAminoType].join(","), QString("1,5,8,9"));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("2,14,15"));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("3,6,11,12"));
    QCOMPARE(result[AdocTreeNode::PrimerType].join(","), QString("4"));
    QCOMPARE(result[AdocTreeNode::MsaAminoType].join(","), QString("7"));
    QCOMPARE(result[AdocTreeNode::MsaRnaType].join(","), QString("10"));
    QCOMPARE(result[AdocTreeNode::MsaDnaType].join(","), QString("13"));

    // Test: Domains
    result = model.getIdsByNodeType(QList<QModelIndex>() << domainsIndex);
    QCOMPARE(result.count(), 4);
    QVERIFY(result.contains(AdocTreeNode::SubseqAminoType));
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QVERIFY(result.contains(AdocTreeNode::PrimerType));

    QCOMPARE(result[AdocTreeNode::SubseqAminoType].join(","), QString("1,5"));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("2"));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("3"));
    QCOMPARE(result[AdocTreeNode::PrimerType].join(","), QString("4"));

    // Test: Alpha
    result = model.getIdsByNodeType(QList<QModelIndex>() << alphaIndex);
    QCOMPARE(result.count(), 3);
    QVERIFY(result.contains(AdocTreeNode::SubseqAminoType));
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));

    QCOMPARE(result[AdocTreeNode::SubseqAminoType].join(","), QString("1"));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("2"));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("3"));

    // Test: Amino
    result = model.getIdsByNodeType(QList<QModelIndex>() << aminoIndex);
    QCOMPARE(result.count(), 1);
    QVERIFY(result.contains(AdocTreeNode::SubseqAminoType));
    QCOMPARE(result[AdocTreeNode::SubseqAminoType].join(","), QString("1"));

    // Test: Dna
    result = model.getIdsByNodeType(QList<QModelIndex>() << dnaIndex);
    QCOMPARE(result.count(), 1);
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("2"));

    // Test: Rna
    result = model.getIdsByNodeType(QList<QModelIndex>() << rnaIndex);
    QCOMPARE(result.count(), 1);
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("3"));

    // Test: Primer
    result = model.getIdsByNodeType(QList<QModelIndex>() << primerIndex);
    QCOMPARE(result.count(), 1);
    QVERIFY(result.contains(AdocTreeNode::PrimerType));
    QCOMPARE(result[AdocTreeNode::PrimerType].join(","), QString("4"));

    // Test: Amino2
    result = model.getIdsByNodeType(QList<QModelIndex>() << amino2Index);
    QCOMPARE(result.count(), 1);
    QVERIFY(result.contains(AdocTreeNode::SubseqAminoType));
    QCOMPARE(result[AdocTreeNode::SubseqAminoType].join(","), QString("5"));

    // Test: Rna2
    result = model.getIdsByNodeType(QList<QModelIndex>() << rna2Index);
    QCOMPARE(result.count(), 1);
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("6"));

    // Test: MsaAmino
    result = model.getIdsByNodeType(QList<QModelIndex>() << msaAminoIndex);
    QCOMPARE(result.count(), 2);
    QVERIFY(result.contains(AdocTreeNode::MsaAminoType));
    QVERIFY(result.contains(AdocTreeNode::SubseqAminoType));

    QCOMPARE(result[AdocTreeNode::MsaAminoType].join(","), QString("7"));
    QCOMPARE(result[AdocTreeNode::SubseqAminoType].join(","), QString("8,9"));

    // Test: delta
    result = model.getIdsByNodeType(QList<QModelIndex>() << deltaIndex);
    QCOMPARE(result.count(), 4);
    QVERIFY(result.contains(AdocTreeNode::MsaRnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QVERIFY(result.contains(AdocTreeNode::MsaDnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));

    QCOMPARE(result[AdocTreeNode::MsaRnaType].join(","), QString("10"));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("11,12"));
    QCOMPARE(result[AdocTreeNode::MsaDnaType].join(","), QString("13"));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("14,15"));

    // Test: MsaRna
    result = model.getIdsByNodeType(QList<QModelIndex>() << msaRnaIndex);
    QCOMPARE(result.count(), 2);
    QVERIFY(result.contains(AdocTreeNode::MsaRnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));

    QCOMPARE(result[AdocTreeNode::MsaRnaType].join(","), QString("10"));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("11,12"));

    // Test: MsaDna
    result = model.getIdsByNodeType(QList<QModelIndex>() << msaDnaIndex);
    QCOMPARE(result.count(), 2);
    QVERIFY(result.contains(AdocTreeNode::MsaDnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));

    QCOMPARE(result[AdocTreeNode::MsaDnaType].join(","), QString("13"));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("14,15"));

    // Test: Index from different model
    AdocTreeModel model2;
    model2.appendRow(new AdocTreeNode(AdocTreeNode::GroupType, "Group"));
    QVERIFY(model.getIdsByNodeType(QList<QModelIndex>() << model2.index(0, 0)).isEmpty());

    // ---------------------------------------------------
    // Suite: multiple parents in list
    // Test: alpha + MsaDna
    result = model.getIdsByNodeType(QList<QModelIndex>() << alphaIndex << msaDnaIndex);
    QCOMPARE(result.count(), 4);
    QVERIFY(result.contains(AdocTreeNode::SubseqAminoType));
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QVERIFY(result.contains(AdocTreeNode::MsaDnaType));

    QCOMPARE(result[AdocTreeNode::SubseqAminoType].join(","), QString("1"));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("2,14,15"));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("3"));
    QCOMPARE(result[AdocTreeNode::MsaDnaType].join(","), QString("13"));

    // Test: Rna2 + Rna + Rna4
    result = model.getIdsByNodeType(QList<QModelIndex>() << rna2Index << rnaIndex << rna4Index);
    QCOMPARE(result.count(), 1);
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("6,3,12"));

    // Test: delta + primer
    result = model.getIdsByNodeType(QList<QModelIndex>() << deltaIndex << primerIndex);
    QCOMPARE(result.count(), 5);
    QVERIFY(result.contains(AdocTreeNode::MsaRnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QVERIFY(result.contains(AdocTreeNode::MsaDnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));
    QVERIFY(result.contains(AdocTreeNode::PrimerType));

    QCOMPARE(result[AdocTreeNode::MsaRnaType].join(","), QString("10"));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("11,12"));
    QCOMPARE(result[AdocTreeNode::MsaDnaType].join(","), QString("13"));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("14,15"));
    QCOMPARE(result[AdocTreeNode::PrimerType].join(","), QString("4"));

    // ---------------------------------------------------
    // Test: duplicate nodes
    result = model.getIdsByNodeType(QList<QModelIndex>() << QModelIndex() << QModelIndex());
    QCOMPARE(result.count(), 7);
    QVERIFY(result.contains(AdocTreeNode::SubseqAminoType));
    QVERIFY(result.contains(AdocTreeNode::SubseqDnaType));
    QVERIFY(result.contains(AdocTreeNode::SubseqRnaType));
    QVERIFY(result.contains(AdocTreeNode::PrimerType));
    QVERIFY(result.contains(AdocTreeNode::MsaAminoType));
    QVERIFY(result.contains(AdocTreeNode::MsaRnaType));
    QVERIFY(result.contains(AdocTreeNode::MsaDnaType));

    QCOMPARE(result[AdocTreeNode::SubseqAminoType].join(","), QString("1,5,8,9,1,5,8,9"));
    QCOMPARE(result[AdocTreeNode::SubseqDnaType].join(","), QString("2,14,15,2,14,15"));
    QCOMPARE(result[AdocTreeNode::SubseqRnaType].join(","), QString("3,6,11,12,3,6,11,12"));
    QCOMPARE(result[AdocTreeNode::PrimerType].join(","), QString("4,4"));
    QCOMPARE(result[AdocTreeNode::MsaAminoType].join(","), QString("7,7"));
    QCOMPARE(result[AdocTreeNode::MsaRnaType].join(","), QString("10,10"));
    QCOMPARE(result[AdocTreeNode::MsaDnaType].join(","), QString("13,13"));
}

QTEST_MAIN(TestAdocTreeModel)
#include "TestAdocTreeModel.moc"
