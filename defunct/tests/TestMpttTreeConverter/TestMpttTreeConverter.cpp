/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "MpttTreeConverter.h"

#include "exceptions/InvalidMpttNodeError.h"

#include "MpttNode.h"
#include "TreeNode.h"

class TestMpttTreeConverter : public QObject
{
    Q_OBJECT

private slots:
    void toMpttList();
    void fromMpttList();

    // ------------------------------------------------------------------------------------------------
    // Benchmarks
   void benchMpttList();
};

void TestMpttTreeConverter::toMpttList()
{
    MpttTreeConverter converter;

    QList<MpttNode *> list;

    // Test: null/zero node
    list = converter.toMpttList(0);
    QVERIFY(list.count() == 0);

    // Test: just the root node
    TreeNode *root = new TreeNode();
    list = converter.toMpttList(root);
    QVERIFY(list.count() == 1);
    QVERIFY(list.at(0)->treeNode_ == root);
    QVERIFY(list.at(0)->left_ == 1);
    QVERIFY(list.at(0)->right_ == 2);

    // Test: root + child1
    qDeleteAll(list);
    list.clear();
    TreeNode *child1 = new TreeNode();
    root->appendChild(child1);
    list = converter.toMpttList(root);
    QVERIFY(list.count() == 2);
    QVERIFY(list.at(0)->treeNode_ == root);
    QVERIFY(list.at(0)->left_ == 1);
    QVERIFY(list.at(0)->right_ == 4);

    QVERIFY(list.at(1)->treeNode_ == child1);
    QVERIFY(list.at(1)->left_ == 2);
    QVERIFY(list.at(1)->right_ == 3);

    // Test: root, child1, and child12
    qDeleteAll(list);
    list.clear();
    TreeNode *child12 = new TreeNode();
    child1->appendChild(child12);
    list = converter.toMpttList(root);
    QVERIFY(list.count() == 3);
    QVERIFY(list.at(0)->treeNode_ == root);
    QVERIFY(list.at(0)->left_ == 1);
    QVERIFY(list.at(0)->right_ == 6);

    QVERIFY(list.at(1)->treeNode_ == child1);
    QVERIFY(list.at(1)->left_ == 2);
    QVERIFY(list.at(1)->right_ == 5);

    QVERIFY(list.at(2)->treeNode_ == child12);
    QVERIFY(list.at(2)->left_ == 3);
    QVERIFY(list.at(2)->right_ == 4);

    // Test: root, child1, cihld12, and child13
    qDeleteAll(list);
    list.clear();
    TreeNode *child13 = new TreeNode();
    child1->appendChild(child13);
    list = converter.toMpttList(root);
    QVERIFY(list.count() == 4);
    QVERIFY(list.at(0)->treeNode_ == root);
    QVERIFY(list.at(0)->left_ == 1);
    QVERIFY(list.at(0)->right_ == 8);

    QVERIFY(list.at(1)->treeNode_ == child1);
    QVERIFY(list.at(1)->left_ == 2);
    QVERIFY(list.at(1)->right_ == 7);

    QVERIFY(list.at(2)->treeNode_ == child12);
    QVERIFY(list.at(2)->left_ == 3);
    QVERIFY(list.at(2)->right_ == 4);

    QVERIFY(list.at(3)->treeNode_ == child13);
    QVERIFY(list.at(3)->left_ == 5);
    QVERIFY(list.at(3)->right_ == 6);

    // Test: root, child1, child12, child13, and child2
    qDeleteAll(list);
    list.clear();
    TreeNode *child2 = new TreeNode();
    root->appendChild(child2);
    list = converter.toMpttList(root);
    QVERIFY(list.count() == 5);
    QVERIFY(list.at(0)->treeNode_ == root);
    QVERIFY(list.at(0)->left_ == 1);
    QVERIFY(list.at(0)->right_ == 10);

    QVERIFY(list.at(1)->treeNode_ == child1);
    QVERIFY(list.at(1)->left_ == 2);
    QVERIFY(list.at(1)->right_ == 7);

    QVERIFY(list.at(2)->treeNode_ == child12);
    QVERIFY(list.at(2)->left_ == 3);
    QVERIFY(list.at(2)->right_ == 4);

    QVERIFY(list.at(3)->treeNode_ == child13);
    QVERIFY(list.at(3)->left_ == 5);
    QVERIFY(list.at(3)->right_ == 6);

    QVERIFY(list.at(4)->treeNode_ == child2);
    QVERIFY(list.at(4)->left_ == 8);
    QVERIFY(list.at(4)->right_ == 9);

    // Test: root and three children
    root->removeChildren();
    child1 = 0;
    child12 = 0;
    child13 = 0;
    child2 = 0;
    qDeleteAll(list);
    list.clear();
    TreeNode *one = new TreeNode();
    TreeNode *two = new TreeNode();
    TreeNode *three = new TreeNode();
    root->appendChild(one);
    root->appendChild(two);
    root->appendChild(three);
    list = converter.toMpttList(root);
    QVERIFY(list.count() == 4);
    QVERIFY(list.at(0)->treeNode_ == root);
    QVERIFY(list.at(0)->left_ == 1);
    QVERIFY(list.at(0)->right_ == 8);

    QVERIFY(list.at(1)->treeNode_ == one);
    QVERIFY(list.at(1)->left_ == 2);
    QVERIFY(list.at(1)->right_ == 3);

    QVERIFY(list.at(2)->treeNode_ == two);
    QVERIFY(list.at(2)->left_ == 4);
    QVERIFY(list.at(2)->right_ == 5);

    QVERIFY(list.at(3)->treeNode_ == three);
    QVERIFY(list.at(3)->left_ == 6);
    QVERIFY(list.at(3)->right_ == 7);

    delete root;
    root = 0;

    qDeleteAll(list);
    list.clear();
}


void TestMpttTreeConverter::fromMpttList()
{
    QList<MpttNode *> list;
    MpttTreeConverter converter;

    try
    {
        // ----------------------------------------
        // Test: empty list
        TreeNode *tree = converter.fromMpttList(list);
        QVERIFY(!tree);

        // ----------------------------------------
        // Test: root left != 1
        TreeNode *root = new TreeNode();
        MpttNode *rootMptt = new MpttNode(root, 2, 3);
        list.append(rootMptt);
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == rootMptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eBadLeftCode);
        }

        // ----------------------------------------
        // Test: invalid root right value
        rootMptt->left_ = 1;
        rootMptt->right_ = 3;
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == rootMptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eBadRightCode);
        }

        // Add child to check remaining cases

        // ----------------------------------------
        // Test: duplicate TreeNode pointer
        rootMptt->left_ = 1;
        rootMptt->right_ = 4;
        MpttNode *child1Mptt = new MpttNode(root, 2, 3);
        list.append(child1Mptt);
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eDuplicateTreeNodeCode);
        }

        // ----------------------------------------
        // Test: root node of mptt has pointer to parent treenode
        //       list only contains the child node but it has a parent
        TreeNode *child1 = new TreeNode();
        root->appendChild(child1);
        child1Mptt->treeNode_ = child1;
        child1Mptt->left_ = 1;
        child1Mptt->right_ = 2;
        list.clear();
        list.append(child1Mptt);
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            // child1Mptt is really functioning as a root node (in a sense)
            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eParentTreeNodeCode);
        }


        // ----------------------------------------
        // Test: children on root - in the previous test setup, we assigned child1Mptt as a child of root (TreeNode API)
        rootMptt->left_ = 1;
        rootMptt->right_ = 4;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 3;
        list.clear();
        list.append(rootMptt);
        list.append(child1Mptt);
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == rootMptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eHasChildrenCode);
        }

        // Cleanup from previous test
        root->removeChildren();
        child1 = 0;
        child1Mptt->treeNode_ = 0;


        // ----------------------------------------
        // Test: duplicate left value
        child1 = new TreeNode();
        child1Mptt->treeNode_ = child1;
        rootMptt->left_ = 1;
        rootMptt->right_ = 4;
        child1Mptt->left_ = 1;
        child1Mptt->right_ = 2;
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eDuplicateLeftCode);
        }


        // ----------------------------------------
        // Test: duplicate right value
        child1 = new TreeNode();
        child1Mptt->treeNode_ = child1;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 4;
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eDuplicateRightCode);
        }

        // ----------------------------------------
        // Test: ! left < right
        rootMptt->left_ = 1;
        rootMptt->right_ = 4;
        child1Mptt->left_ = 5;
        child1Mptt->right_ = 3;
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eBadLeftCode);
        }

        // ----------------------------------------
        // Test: even/odd inconsistency; must add another child node for this test
        rootMptt->right_ = 6;
        TreeNode *child2 = new TreeNode();
        MpttNode *child2Mptt = new MpttNode(child2, 4, 5);
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 4;
        list.append(child2Mptt);
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eEvenOddCode);
        }

        // ----------------------------------------
        // Test: unexpected left value
        rootMptt->left_ = 1;
        rootMptt->right_ = 8;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 3;
        child2Mptt->left_ = 5;
        child2Mptt->right_ = 6;
        TreeNode *child3 = new TreeNode();
        MpttNode *child3Mptt = new MpttNode(child3, 7, 7);
        list.append(child3Mptt);
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == child2Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eUnexpectedLeftCode);
        }

        // ----------------------------------------
        // Test: child left < parent left
        // Since we sort on the left value, I do not see how it is possible to build test case for child left < parent left
        // The commented test below, throws an error where the root value does not equal 1
    //    rootMptt->left_ = 1;
    //    rootMptt->right_ = 6;
    //    child1Mptt->left_ = 0;
    //    child1Mptt->right_ = 3;
    //    child2Mptt->left_ = 4;
    //    child2Mptt->right_ = 5;
    //    tree = converter.fromMpttList(list);
    //    QVERIFY(!tree);
    //    QVERIFY2(error.message() == "left must be greater than parent left", error.message().toAscii());


        // ----------------------------------------
        // Test: child right > parent right
        // To do this, we're going to break the naming convention of children and just use the three existing
        // children organized into the following tree -->  root: [ child1: [ child2 ], child3 ]
        rootMptt->left_ = 1;
        rootMptt->right_ = 8;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 5;
        child2Mptt->left_ = 3;
        child2Mptt->right_ = 6;  // <-- this is the problem child :)
        child3Mptt->left_ = 6;   // <-- This is also a problem child, but not tested because its left > child2's left
        child3Mptt->right_ = 7;

        list.clear();
        list.append(rootMptt);
        list.append(child1Mptt);
        list.append(child2Mptt);
        list.append(child3Mptt);
        try
        {
            converter.fromMpttList(list);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            QVERIFY(e.mpttNode() == child2Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eUnexpectedRightCode);
        }


        // Not sure how to build test case that passes the above tests and yet misses at least one left or right value...

        // ----------------------------------------
        // ----------------------------------------
        // ----------------------------------------
        // Done testing error cases for left and right. Begin checking real trees; since these are all expected to not
        // throw exceptions we simply utilize the outermost try/catch loop to ensure we behave appropriately
        root->removeChildren();
        child1 = 0;
        child2 = 0;
        child3 = 0;
        child1Mptt->treeNode_ = 0;
        child2Mptt->treeNode_ = 0;
        child3Mptt->treeNode_ = 0;
        list.clear();

        // ----------------------------------------
        // Test: just the root node
        rootMptt->left_ = 1;
        rootMptt->right_ = 2;
        list.append(rootMptt);
        tree = converter.fromMpttList(list);
        QVERIFY(tree);
        QVERIFY(tree == root);
        QVERIFY(tree->childCount() == 0);

        // Test: root, child1
        rootMptt->left_ = 1;
        rootMptt->right_ = 4;
        child1 = new TreeNode();
        child1Mptt->treeNode_ = child1;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 3;
        list.append(child1Mptt);
        tree = converter.fromMpttList(list);
        QVERIFY(tree);
        QVERIFY(tree == root);
        QVERIFY(tree->childCount() == 1);

        QVERIFY(tree->childAt(0) == child1);
        QVERIFY(tree->childAt(0)->childCount() == 0);
        QVERIFY(tree->childAt(0)->parent() == tree);

        // Must clear the parent/child relationships that were defined in order for the next test to succeed
        root->removeChildren(); // This will also remove all memory allocated to children
        child1 = 0;
        child1Mptt->treeNode_ = 0;

        // Test: root: [ child1: [ child11 ] ]
        rootMptt->left_ = 1;
        rootMptt->right_ = 6;
        child1 = new TreeNode();
        child1Mptt->treeNode_ = child1;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 5;
        TreeNode *child11 = new TreeNode();
        MpttNode *child11Mptt = new MpttNode(child11, 3, 4);
        list.append(child11Mptt);
        tree = converter.fromMpttList(list);
        QVERIFY(tree);
        QVERIFY(tree == root);
        QVERIFY2(tree->childCount() == 1, QString("ChildCount: %1").arg(tree->childCount()).toAscii());

        QVERIFY(tree->childAt(0) == child1);
        QVERIFY(tree->childAt(0)->childCount() == 1);
        QVERIFY(tree->childAt(0)->parent() == tree);

        QVERIFY(tree->childAt(0)->childAt(0) == child11);
        QVERIFY(tree->childAt(0)->childAt(0)->childCount() == 0);
        QVERIFY(tree->childAt(0)->childAt(0)->parent() == child1);

        // Must clear the parent/child relationships that were defined in order for the next test to succeed
        root->removeChildren(); // This will also remove all memory allocated to children
        child1 = 0;
        child1Mptt->treeNode_ = 0;
        child11 = 0;
        child11Mptt->treeNode_ = 0;


        // Test: root: [ child1: [ child11, child12 ] ]
        rootMptt->left_ = 1;
        rootMptt->right_ = 8;

        child1 = new TreeNode();
        child1Mptt->treeNode_ = child1;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 7;

        child11 = new TreeNode();
        child11Mptt->treeNode_ = child11;
        child11Mptt->left_ = 3;
        child11Mptt->right_ = 4;

        TreeNode *child12 = new TreeNode();
        MpttNode *child12Mptt = new MpttNode(child12, 5, 6);
        list.append(child12Mptt);

        tree = converter.fromMpttList(list);
        QVERIFY(tree);
        QVERIFY(tree == root);
        QVERIFY(tree->childCount() == 1);

        QVERIFY(tree->childAt(0) == child1);
        QVERIFY(tree->childAt(0)->childCount() == 2);
        QVERIFY(tree->childAt(0)->parent() == tree);

        QVERIFY(tree->childAt(0)->childAt(0) == child11);
        QVERIFY(tree->childAt(0)->childAt(0)->childCount() == 0);
        QVERIFY(tree->childAt(0)->childAt(0)->parent() == child1);

        QVERIFY(tree->childAt(0)->childAt(1) == child12);
        QVERIFY(tree->childAt(0)->childAt(1)->childCount() == 0);
        QVERIFY(tree->childAt(0)->childAt(1)->parent() == child1);

        // Must clear the parent/child relationships that were defined in order for the next test to succeed
        root->removeChildren(); // This will also remove all memory allocated to children
        child1 = 0;
        child1Mptt->treeNode_ = 0;
        child11 = 0;
        child11Mptt->treeNode_ = 0;
        child12 = 0;
        child12Mptt->treeNode_ = 0;


        // Test: root: [ child1: [ child11, child12 ], child2 ]
        rootMptt->left_ = 1;
        rootMptt->right_ = 10;

        child1 = new TreeNode();
        child1Mptt->treeNode_ = child1;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 7;

        child11 = new TreeNode();
        child11Mptt->treeNode_ = child11;
        child11Mptt->left_ = 3;
        child11Mptt->right_ = 4;

        child12 = new TreeNode();
        child12Mptt->treeNode_ = child12;
        child12Mptt->left_ = 5;
        child12Mptt->right_ = 6;

        child2 = new TreeNode();
        child2Mptt->treeNode_ = child2;
        child2Mptt->left_ = 8;
        child2Mptt->right_ = 9;
        list.clear();
        list.append(rootMptt);
        list.append(child1Mptt);
        list.append(child11Mptt);
        list.append(child12Mptt);
        list.append(child2Mptt);

        tree = converter.fromMpttList(list);
        QVERIFY(tree);
        QVERIFY(tree == root);
        QVERIFY(tree->childCount() == 2);

        QVERIFY(tree->childAt(0) == child1);
        QVERIFY(tree->childAt(0)->childCount() == 2);
        QVERIFY(tree->childAt(0)->parent() == tree);

        QVERIFY(tree->childAt(0)->childAt(0) == child11);
        QVERIFY(tree->childAt(0)->childAt(0)->childCount() == 0);
        QVERIFY(tree->childAt(0)->childAt(0)->parent() == child1);

        QVERIFY(tree->childAt(0)->childAt(1) == child12);
        QVERIFY(tree->childAt(0)->childAt(1)->childCount() == 0);
        QVERIFY(tree->childAt(0)->childAt(1)->parent() == child1);

        QVERIFY(tree->childAt(1) == child2);
        QVERIFY(tree->childAt(1)->childCount() == 0);
        QVERIFY(tree->childAt(1)->parent() == tree);

        // Must clear the parent/child relationships that were defined in order for the next test to succeed
        root->removeChildren(); // This will also remove all memory allocated to children
        child1 = 0;
        child1Mptt->treeNode_ = 0;
        child11 = 0;
        child11Mptt->treeNode_ = 0;
        child12 = 0;
        child12Mptt->treeNode_ = 0;
        child2 = 0;
        child2Mptt->treeNode_ = 0;


        // Test: 3 nodes directly under root
        root->removeChildren();
        list.clear();

        rootMptt->left_ = 1;
        rootMptt->right_ = 8;

        child1 = new TreeNode();
        child1Mptt->treeNode_ = child1;
        child1Mptt->left_ = 2;
        child1Mptt->right_ = 3;

        child2 = new TreeNode();
        child2Mptt->treeNode_ = child2;
        child2Mptt->left_ = 4;
        child2Mptt->right_ = 5;

        child3 = new TreeNode();
        child3Mptt->treeNode_ = child3;
        child3Mptt->left_ = 6;
        child3Mptt->right_ = 7;
        list.append(rootMptt);
        list.append(child1Mptt);
        list.append(child2Mptt);
        list.append(child3Mptt);
        tree = converter.fromMpttList(list);
        QVERIFY(tree);
        QVERIFY(tree == root);
        QVERIFY(tree->childCount() == 3);

        QVERIFY(tree->childAt(0) == child1);
        QVERIFY(tree->childAt(0)->childCount() == 0);
        QVERIFY(tree->childAt(0)->parent() == tree);

        QVERIFY(tree->childAt(1) == child2);
        QVERIFY(tree->childAt(1)->childCount() == 0);
        QVERIFY(tree->childAt(1)->parent() == tree);

        QVERIFY(tree->childAt(2) == child3);
        QVERIFY(tree->childAt(2)->childCount() == 0);
        QVERIFY(tree->childAt(2)->parent() == tree);

        list.clear();
        delete root;        root = 0;
        child1 = 0;
        child2 = 0;
        child3 = 0;
        delete child1Mptt;  child2Mptt = 0;
        delete child2Mptt;  child2Mptt = 0;
        delete child3Mptt;  child3Mptt = 0;
        delete rootMptt;    rootMptt = 0;
    }
    catch (...) { QVERIFY(0); }
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Benchmarks
void TestMpttTreeConverter::benchMpttList()
{
    int z = 1000000;
    qDebug() << "Testing the speed of" << z << "mptt nodes and deletions";
    QBENCHMARK {
        QList<MpttNode *> mpttNodes;
        for (int i=0; i< z; ++i)
            mpttNodes.append(new MpttNode(0, 3, 5));
        qDeleteAll(mpttNodes);
    }
}


QTEST_APPLESS_MAIN(TestMpttTreeConverter)
#include "TestMpttTreeConverter.moc"
