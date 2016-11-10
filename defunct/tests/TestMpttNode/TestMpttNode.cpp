/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "MpttNode.h"
#include "TreeNode.h"

class TestMpttNode : public QObject
{
    Q_OBJECT

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    void constructor();

    // ------------------------------------------------------------------------------------------------
    // Global functions
    void freeMpttList();
};

void TestMpttNode::constructor()
{
    MpttNode mpttNode;
    QVERIFY(mpttNode.treeNode_ == 0);
    QVERIFY(mpttNode.left_ == -1);
    QVERIFY(mpttNode.right_ == -1);

    TreeNode *root = new TreeNode();
    MpttNode mpttNode2(root, 1, 2);
    QVERIFY(mpttNode2.treeNode_ == root);
    QVERIFY(mpttNode2.left_ == 1);
    QVERIFY(mpttNode2.right_ == 2);

    delete root;
    root = 0;
}

void TestMpttNode::freeMpttList()
{
    QList<MpttNode *> mpttNodes;
    mpttNodes.append(new MpttNode());
    mpttNodes.append(new MpttNode(new TreeNode()));

    ::freeMpttList(mpttNodes);

    QVERIFY(mpttNodes.isEmpty());
}

QTEST_MAIN(TestMpttNode)
#include "TestMpttNode.moc"
