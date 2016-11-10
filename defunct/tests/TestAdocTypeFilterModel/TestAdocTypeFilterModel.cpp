/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include "AdocTypeFilterModel.h"

class TestAdocTypeFilterModel : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void acceptNodeTypes();         // Also tests setAcceptNodeTypes
    void isFiltered();
};


// ------------------------------------------------------------------------------------------------
// Constructors
void TestAdocTypeFilterModel::constructor()
{
    AdocTypeFilterModel model;

    QObject *qObject = new QObject();
    new AdocTypeFilterModel(qObject);
    delete qObject;
}

void TestAdocTypeFilterModel::acceptNodeTypes()
{
    AdocTypeFilterModel model;

    // Test: default model should have no types defined
    QVERIFY(model.acceptNodeTypes().isEmpty());

    // Test: give it some node types
    model.setAcceptNodeTypes(QList<AdocTreeNode::NodeType>() << AdocTreeNode::RootType << AdocTreeNode::ProjectType);
    QVERIFY(model.acceptNodeTypes().count() == 2);
    QCOMPARE(model.acceptNodeTypes().at(0), AdocTreeNode::RootType);
    QCOMPARE(model.acceptNodeTypes().at(1), AdocTreeNode::ProjectType);

    // Test: clear it out
    model.setAcceptNodeTypes(QList<AdocTreeNode::NodeType>());
    QVERIFY(model.acceptNodeTypes().isEmpty());
}

void TestAdocTypeFilterModel::isFiltered()
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

    AdocTypeFilterModel *filter = new AdocTypeFilterModel(this);
    filter->setSourceModel(tree);

    // At this point, everything is filtered because we have not set any acceptNodeTypes
    QVERIFY(filter->rowCount() == 0);

    filter->setAcceptNodeTypes(QList<AdocTreeNode::NodeType>() << AdocTreeNode::RootType << AdocTreeNode::ProjectType << AdocTreeNode::GroupType);

    // Test: verify that only RootType, ProjectType, GroupType, and Msa{X}Type nodes are visible in the proxy
    QVERIFY2(filter->rowCount() == 3, QString("%1").arg(filter->rowCount()).toAscii());
    QModelIndex alpha_index = filter->index(0, 0);
    QVERIFY(filter->data(alpha_index) == "Alpha");
    QVERIFY(filter->rowCount(alpha_index) == 0);

    QModelIndex beta_index = filter->index(1, 0);
    QVERIFY(filter->data(beta_index) == "Beta");
    QVERIFY(filter->rowCount(beta_index) == 0);

    QModelIndex gamma_index = filter->index(2, 0);
    QVERIFY(filter->data(gamma_index) == "Gamma");
    QVERIFY(filter->rowCount(gamma_index) == 1);
    QVERIFY(filter->rowCount(filter->index(0, 0, gamma_index)) == 0);
}

QTEST_MAIN(TestAdocTypeFilterModel)
#include "TestAdocTypeFilterModel.moc"
