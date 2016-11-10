/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../TreeNode.h"
#include "../MpttNode.h"
#include "../global.h"

class TestMpttNode : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Global functions
    void freeMpttTreeNodes();
};

void TestMpttNode::constructor()
{
    MpttNode mpttNode;
    QVERIFY(mpttNode.treeNode_ == nullptr);
    QVERIFY(mpttNode.left_ == -1);
    QVERIFY(mpttNode.right_ == -1);

    TreeNode<bool> *root = new TreeNode<bool>(false);
    MpttNode mpttNode2(root, 1, 2);
    QVERIFY(mpttNode2.treeNode_ == root);
    QVERIFY(mpttNode2.left_ == 1);
    QVERIFY(mpttNode2.right_ == 2);

    delete root;
    root = 0;
}

void TestMpttNode::freeMpttTreeNodes()
{
    QVector<MpttNode> mpttNodes;
    mpttNodes << MpttNode() << MpttNode(new TreeNode<bool>(false));

    ::freeMpttTreeNodes(mpttNodes);
    QCOMPARE(mpttNodes.size(), 2);

    QVERIFY(mpttNodes.at(0).treeNode_ == nullptr);
    QVERIFY(mpttNodes.at(1).treeNode_ == nullptr);
}

QTEST_MAIN(TestMpttNode)
#include "TestMpttNode.moc"
