/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "models/AdocAminoFilterModel.h"

class TestAdocAminoFilterModel : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Verify filtering functionality
    void isFiltered();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestAdocAminoFilterModel::constructor()
{
    AdocAminoFilterModel model;

    QObject *qObject = new QObject();
    new AdocAminoFilterModel(qObject);
    delete qObject;
}

void TestAdocAminoFilterModel::isFiltered()
{
    AdocTreeModel *tree = new AdocTreeModel(this);

    // Create the following tree for testing purposes
    // Root (RootType)
    // |___ Alpha (ProjectType)
    //      |___ s1 (SeqAminoType)
    //      |___ s2 (SeqDnaType)
    //      |___ s3 (SeqRnaType)
    //      |___ ss1 (SubseqAminoType)
    //      |___ ss2 (SubseqDnaType)
    //      |___ ss3 (SubseqRnaType)
    // |___ Beta (ProjectType)
    //      |___ msa1 (MsaAminoType)
    //      |___ msa2 (MsaDnaType)
    //      |___ msa3 (MsaRnaType)
    // |___ Gamma (ProjectType)
    //      |___ g1 (GroupType)
    //           |___ p1 (PrimerType)

    QSharedPointer<AdocTreeNode> root = tree->root().toStrongRef();
    AdocTreeNode *alpha = new AdocTreeNode(AdocTreeNode::ProjectType, "Alpha");
    root->appendChild(alpha);
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SeqAminoType, "s1"));
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SeqDnaType, "s2"));
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SeqRnaType, "s3"));
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "ss1"));
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SubseqDnaType, "ss2"));
    alpha->appendChild(new AdocTreeNode(AdocTreeNode::SubseqRnaType, "ss3"));

    AdocTreeNode *beta = new AdocTreeNode(AdocTreeNode::ProjectType, "Beta");
    root->appendChild(beta);
    beta->appendChild(new AdocTreeNode(AdocTreeNode::MsaAminoType, "msa1"));
    beta->appendChild(new AdocTreeNode(AdocTreeNode::MsaDnaType, "msa2"));
    beta->appendChild(new AdocTreeNode(AdocTreeNode::MsaRnaType, "msa3"));

    AdocTreeNode *gamma = new AdocTreeNode(AdocTreeNode::ProjectType, "Gamma");
    root->appendChild(gamma);
    AdocTreeNode *g1 = new AdocTreeNode(AdocTreeNode::GroupType, "g1");
    gamma->appendChild(g1);
    g1->appendChild(new AdocTreeNode(AdocTreeNode::PrimerType, "p1"));

    AdocAminoFilterModel *filter = new AdocAminoFilterModel(this);
    filter->setSourceModel(tree);

    QModelIndex alpha_index = filter->index(0, 0);
    QVERIFY(filter->data(alpha_index) == "Alpha");
    QVERIFY(filter->rowCount(alpha_index) == 2);
    QVERIFY(filter->data(alpha_index.child(0, 0)) == "s1");
    QVERIFY(filter->data(alpha_index.child(1, 0)) == "ss1");

    QModelIndex beta_index = filter->index(1, 0);
    QVERIFY(filter->data(beta_index) == "Beta");
    QVERIFY(filter->rowCount(beta_index) == 3);

    QModelIndex gamma_index = filter->index(2, 0);
    QVERIFY(filter->data(gamma_index) == "Gamma");
    QVERIFY(filter->rowCount(gamma_index) == 1);
    QVERIFY(filter->rowCount(filter->index(0, 0, gamma_index)) == 0);
}

QTEST_MAIN(TestAdocAminoFilterModel)
#include "TestAdocAminoFilterModel.moc"
