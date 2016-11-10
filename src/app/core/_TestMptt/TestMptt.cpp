/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "../Mptt.h"
#include "../ValueTreeNode.h"
#include "../exceptions/InvalidMpttNodeError.h"
#include "../global.h"

typedef ValueTreeNode<bool> BoolNode;

class MockNode : public ValueTreeNode<bool>
{
public:
    MockNode(const bool value)
        : ValueTreeNode<bool>(value)
    {}
};

class TestMptt : public QObject
{
    Q_OBJECT

private slots:
    void toMpttVector();
    void fromMpttVector();
};

void TestMptt::toMpttVector()
{
    QVector<MpttNode<BoolNode> > mpttNodes;

    // Test: null/zero node
    mpttNodes = Mptt::toMpttVector(static_cast<BoolNode *>(nullptr));
    QVERIFY(mpttNodes.count() == 0);

    // Test: just the root node
    BoolNode *root = new BoolNode(false);
    mpttNodes = Mptt::toMpttVector(root);
    QVERIFY(mpttNodes.count() == 1);
    QVERIFY(mpttNodes.at(0).treeNode_ == root);
    QCOMPARE(mpttNodes.at(0).left_, 1);
    QCOMPARE(mpttNodes.at(0).right_, 2);

    // Test: root + child1
    mpttNodes.clear();
    BoolNode *child1 = new BoolNode(false);
    root->appendChild(child1);
    mpttNodes = Mptt::toMpttVector(root);
    QVERIFY(mpttNodes.count() == 2);
    QVERIFY(mpttNodes.at(0).treeNode_ == root);
    QCOMPARE(mpttNodes.at(0).left_, 1);
    QCOMPARE(mpttNodes.at(0).right_, 4);

    QVERIFY(mpttNodes.at(1).treeNode_ == child1);
    QCOMPARE(mpttNodes.at(1).left_, 2);
    QCOMPARE(mpttNodes.at(1).right_, 3);

    // Test: root, child1, and child12
    mpttNodes.clear();
    BoolNode *child12 = new BoolNode(false);
    child1->appendChild(child12);
    mpttNodes = Mptt::toMpttVector(root);
    QVERIFY(mpttNodes.count() == 3);
    QVERIFY(mpttNodes.at(0).treeNode_ == root);
    QCOMPARE(mpttNodes.at(0).left_, 1);
    QCOMPARE(mpttNodes.at(0).right_, 6);

    QVERIFY(mpttNodes.at(1).treeNode_ == child1);
    QCOMPARE(mpttNodes.at(1).left_, 2);
    QCOMPARE(mpttNodes.at(1).right_, 5);

    QVERIFY(mpttNodes.at(2).treeNode_ == child12);
    QCOMPARE(mpttNodes.at(2).left_, 3);
    QCOMPARE(mpttNodes.at(2).right_, 4);

    // Test: root, child1, cihld12, and child13
    mpttNodes.clear();
    BoolNode *child13 = new BoolNode(false);
    child1->appendChild(child13);
    mpttNodes = Mptt::toMpttVector(root);
    QVERIFY(mpttNodes.count() == 4);
    QVERIFY(mpttNodes.at(0).treeNode_ == root);
    QCOMPARE(mpttNodes.at(0).left_, 1);
    QCOMPARE(mpttNodes.at(0).right_, 8);

    QVERIFY(mpttNodes.at(1).treeNode_ == child1);
    QCOMPARE(mpttNodes.at(1).left_, 2);
    QCOMPARE(mpttNodes.at(1).right_, 7);

    QVERIFY(mpttNodes.at(2).treeNode_ == child12);
    QCOMPARE(mpttNodes.at(2).left_, 3);
    QCOMPARE(mpttNodes.at(2).right_, 4);

    QVERIFY(mpttNodes.at(3).treeNode_ == child13);
    QCOMPARE(mpttNodes.at(3).left_, 5);
    QCOMPARE(mpttNodes.at(3).right_, 6);

    // Test: root, child1, child12, child13, and child2
    mpttNodes.clear();
    BoolNode *child2 = new BoolNode(false);
    root->appendChild(child2);
    mpttNodes = Mptt::toMpttVector(root);
    QVERIFY(mpttNodes.count() == 5);
    QVERIFY(mpttNodes.at(0).treeNode_ == root);
    QCOMPARE(mpttNodes.at(0).left_, 1);
    QCOMPARE(mpttNodes.at(0).right_, 10);

    QVERIFY(mpttNodes.at(1).treeNode_ == child1);
    QCOMPARE(mpttNodes.at(1).left_, 2);
    QCOMPARE(mpttNodes.at(1).right_, 7);

    QVERIFY(mpttNodes.at(2).treeNode_ == child12);
    QCOMPARE(mpttNodes.at(2).left_, 3);
    QCOMPARE(mpttNodes.at(2).right_, 4);

    QVERIFY(mpttNodes.at(3).treeNode_ == child13);
    QCOMPARE(mpttNodes.at(3).left_, 5);
    QCOMPARE(mpttNodes.at(3).right_, 6);

    QVERIFY(mpttNodes.at(4).treeNode_ == child2);
    QCOMPARE(mpttNodes.at(4).left_, 8);
    QCOMPARE(mpttNodes.at(4).right_, 9);

    // Test: root and three children
    root->removeChildren();
    child1 = nullptr;
    child12 = nullptr;
    child13 = nullptr;
    child2 = nullptr;
    mpttNodes.clear();
    BoolNode *one = new BoolNode(false);
    BoolNode *two = new BoolNode(false);
    BoolNode *three = new BoolNode(false);
    root->appendChild(one);
    root->appendChild(two);
    root->appendChild(three);
    mpttNodes = Mptt::toMpttVector(root);
    QVERIFY(mpttNodes.count() == 4);
    QVERIFY(mpttNodes.at(0).treeNode_ == root);
    QCOMPARE(mpttNodes.at(0).left_, 1);
    QCOMPARE(mpttNodes.at(0).right_, 8);

    QVERIFY(mpttNodes.at(1).treeNode_ == one);
    QCOMPARE(mpttNodes.at(1).left_, 2);
    QCOMPARE(mpttNodes.at(1).right_, 3);

    QVERIFY(mpttNodes.at(2).treeNode_ == two);
    QCOMPARE(mpttNodes.at(2).left_, 4);
    QCOMPARE(mpttNodes.at(2).right_, 5);

    QVERIFY(mpttNodes.at(3).treeNode_ == three);
    QCOMPARE(mpttNodes.at(3).left_, 6);
    QCOMPARE(mpttNodes.at(3).right_, 7);

    delete root;
    root = nullptr;
    mpttNodes.clear();

    MockNode *mockRoot = new MockNode(false);
    mockRoot->appendChild(new MockNode(true));
    mockRoot->appendChild(new MockNode(false));
    QVector<MpttNode<MockNode> > mockMpttNodes;
    mockMpttNodes = Mptt::toMpttVector(mockRoot);

    delete mockRoot;
}


void TestMptt::fromMpttVector()
{
    QVector<MpttNode<BoolNode> > mpttNodes;

    try
    {
        // ----------------------------------------
        // Test: empty list
        BoolNode *tree = Mptt::fromMpttVector(mpttNodes);
        QVERIFY(tree == nullptr);

        // ----------------------------------------
        // Test: root left != 1
        BoolNode *root = new BoolNode(false);
        MpttNode<BoolNode> rootMptt(root, 2, 3);
        mpttNodes.append(rootMptt);
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == rootMptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eBadLeftCode);
        }

        // ----------------------------------------
        // Test: invalid root right value
        rootMptt.left_ = 1;
        rootMptt.right_ = 3;
        mpttNodes.clear();
        mpttNodes << rootMptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == rootMptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eBadRightCode);
        }

        // Add child to check remaining cases

        // ----------------------------------------
        // Test: duplicate BoolNode pointer
        rootMptt.left_ = 1;
        rootMptt.right_ = 4;
        MpttNode<BoolNode> child1Mptt(root, 2, 3);
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eDuplicateTreeNodeCode);
        }

        // ----------------------------------------
        // Test: root node of mptt has pointer to parent treenode
        //       list only contains the child node but it has a parent
        BoolNode *child1 = new BoolNode(false);
        root->appendChild(child1);
        child1Mptt.treeNode_ = child1;
        child1Mptt.left_ = 1;
        child1Mptt.right_ = 2;
        mpttNodes.clear();
        mpttNodes << child1Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
            // child1Mptt is really functioning as a root node (in a sense)
//            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eParentTreeNodeCode);
        }


        // ----------------------------------------
        // Test: children on root - in the previous test setup, we assigned child1Mptt as a child of root (BoolNode API)
        rootMptt.left_ = 1;
        rootMptt.right_ = 4;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 3;
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == rootMptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eHasChildrenCode);
        }

        // Cleanup from previous test
        root->removeChildren();
        child1 = nullptr;
        child1Mptt.treeNode_ = nullptr;


        // ----------------------------------------
        // Test: duplicate left value
        child1 = new BoolNode(false);
        child1Mptt.treeNode_ = child1;
        rootMptt.left_ = 1;
        rootMptt.right_ = 4;
        child1Mptt.left_ = 1;
        child1Mptt.right_ = 2;
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eDuplicateLeftCode);
        }


        // ----------------------------------------
        // Test: duplicate right value
        child1 = new BoolNode(false);
        child1Mptt.treeNode_ = child1;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 4;
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eDuplicateRightCode);
        }

        // ----------------------------------------
        // Test: ! left < right
        rootMptt.left_ = 1;
        rootMptt.right_ = 4;
        child1Mptt.left_ = 5;
        child1Mptt.right_ = 3;
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eBadLeftCode);
        }

        // ----------------------------------------
        // Test: even/odd inconsistency; must add another child node for this test
        rootMptt.right_ = 6;
        BoolNode *child2 = new BoolNode(false);
        MpttNode<BoolNode> child2Mptt(child2, 4, 5);
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 4;
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt << child2Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == child1Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eEvenOddCode);
        }

        // ----------------------------------------
        // Test: unexpected left value
        rootMptt.left_ = 1;
        rootMptt.right_ = 8;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 3;
        child2Mptt.left_ = 5;
        child2Mptt.right_ = 6;
        BoolNode *child3 = new BoolNode(false);
        MpttNode<BoolNode> child3Mptt(child3, 7, 7);
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt << child2Mptt << child3Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == child2Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eUnexpectedLeftCode);
        }

        // ----------------------------------------
        // Test: child left < parent left
        // Since we sort on the left value, I do not see how it is possible to build test case for child left < parent left
        // The commented test below, throws an error where the root value does not equal 1
    //    rootMptt.left_ = 1;
    //    rootMptt.right_ = 6;
    //    child1Mptt.left_ = 0;
    //    child1Mptt.right_ = 3;
    //    child2Mptt.left_ = 4;
    //    child2Mptt.right_ = 5;
    //    tree = Mptt::fromMpttVector(list);
    //    QVERIFY(!tree);
    //    QVERIFY2(error.message() == "left must be greater than parent left", error.message().toAscii());


        // ----------------------------------------
        // Test: child right > parent right
        // To do this, we're going to break the naming convention of children and just use the three existing
        // children organized into the following tree -->  root: [ child1: [ child2 ], child3 ]
        rootMptt.left_ = 1;
        rootMptt.right_ = 8;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 5;
        child2Mptt.left_ = 3;
        child2Mptt.right_ = 6;  // <-- this is the problem child :)
        child3Mptt.left_ = 6;   // <-- This is also a problem child, but not tested because its left > child2's left
        child3Mptt.right_ = 7;

        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt << child2Mptt << child3Mptt;
        try
        {
            Mptt::fromMpttVector(mpttNodes);
            QVERIFY(0);
        }
        catch (InvalidMpttNodeError &e)
        {
//            QVERIFY(e.mpttNode() == child2Mptt);
            QVERIFY(e.errorNumber() == InvalidMpttNodeError::eUnexpectedRightCode);
        }


        // Not sure how to build test case that passes the above tests and yet misses at least one left or right value...

        // ----------------------------------------
        // ----------------------------------------
        // ----------------------------------------
        // Done testing error cases for left and right. Begin checking real trees; since these are all expected to not
        // throw exceptions we simply utilize the outermost try/catch loop to ensure we behave appropriately
        root->removeChildren();
        child1 = nullptr;
        child2 = nullptr;
        child3 = nullptr;
        child1Mptt.treeNode_ = nullptr;
        child2Mptt.treeNode_ = nullptr;
        child3Mptt.treeNode_ = nullptr;

        // ----------------------------------------
        // Test: just the root node
        rootMptt.left_ = 1;
        rootMptt.right_ = 2;
        mpttNodes.clear();
        mpttNodes << rootMptt;
        tree = Mptt::fromMpttVector(mpttNodes);
        QVERIFY(tree);
        QVERIFY(tree == root);
        QVERIFY(tree->childCount() == 0);

        // Test: root, child1
        rootMptt.left_ = 1;
        rootMptt.right_ = 4;
        child1 = new BoolNode(false);
        child1Mptt.treeNode_ = child1;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 3;
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt;
        tree = Mptt::fromMpttVector(mpttNodes);
        QVERIFY(tree);
        QVERIFY(tree == root);
        QVERIFY(tree->childCount() == 1);

        QVERIFY(tree->childAt(0) == child1);
        QVERIFY(tree->childAt(0)->childCount() == 0);
        QVERIFY(tree->childAt(0)->parent() == tree);

        // Must clear the parent/child relationships that were defined in order for the next test to succeed
        root->removeChildren(); // This will also remove all memory allocated to children
        child1 = nullptr;
        child1Mptt.treeNode_ = nullptr;

        // Test: root: [ child1: [ child11 ] ]
        rootMptt.left_ = 1;
        rootMptt.right_ = 6;
        child1 = new BoolNode(false);
        child1Mptt.treeNode_ = child1;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 5;
        BoolNode *child11 = new BoolNode(false);
        MpttNode<BoolNode> child11Mptt(child11, 3, 4);
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt << child11Mptt;
        tree = Mptt::fromMpttVector(mpttNodes);
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
        child1 = nullptr;
        child1Mptt.treeNode_ = nullptr;
        child11 = nullptr;
        child11Mptt.treeNode_ = nullptr;


        // Test: root: [ child1: [ child11, child12 ] ]
        rootMptt.left_ = 1;
        rootMptt.right_ = 8;

        child1 = new BoolNode(false);
        child1Mptt.treeNode_ = child1;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 7;

        child11 = new BoolNode(false);
        child11Mptt.treeNode_ = child11;
        child11Mptt.left_ = 3;
        child11Mptt.right_ = 4;

        BoolNode *child12 = new BoolNode(false);
        MpttNode<BoolNode> child12Mptt(child12, 5, 6);
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt << child11Mptt << child12Mptt;

        tree = Mptt::fromMpttVector(mpttNodes);
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
        child1 = nullptr;
        child1Mptt.treeNode_ = nullptr;
        child11 = nullptr;
        child11Mptt.treeNode_ = nullptr;
        child12 = nullptr;
        child12Mptt.treeNode_ = nullptr;


        // Test: root: [ child1: [ child11, child12 ], child2 ]
        rootMptt.left_ = 1;
        rootMptt.right_ = 10;

        child1 = new BoolNode(false);
        child1Mptt.treeNode_ = child1;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 7;

        child11 = new BoolNode(false);
        child11Mptt.treeNode_ = child11;
        child11Mptt.left_ = 3;
        child11Mptt.right_ = 4;

        child12 = new BoolNode(false);
        child12Mptt.treeNode_ = child12;
        child12Mptt.left_ = 5;
        child12Mptt.right_ = 6;

        child2 = new BoolNode(false);
        child2Mptt.treeNode_ = child2;
        child2Mptt.left_ = 8;
        child2Mptt.right_ = 9;
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt << child11Mptt << child12Mptt << child2Mptt;

        tree = Mptt::fromMpttVector(mpttNodes);
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
        child1 = nullptr;
        child1Mptt.treeNode_ = nullptr;
        child11 = nullptr;
        child11Mptt.treeNode_ = nullptr;
        child12 = nullptr;
        child12Mptt.treeNode_ = nullptr;
        child2 = nullptr;
        child2Mptt.treeNode_ = nullptr;


        // Test: 3 nodes directly under root
        root->removeChildren();

        rootMptt.left_ = 1;
        rootMptt.right_ = 8;

        child1 = new BoolNode(false);
        child1Mptt.treeNode_ = child1;
        child1Mptt.left_ = 2;
        child1Mptt.right_ = 3;

        child2 = new BoolNode(false);
        child2Mptt.treeNode_ = child2;
        child2Mptt.left_ = 4;
        child2Mptt.right_ = 5;

        child3 = new BoolNode(false);
        child3Mptt.treeNode_ = child3;
        child3Mptt.left_ = 6;
        child3Mptt.right_ = 7;
        mpttNodes.clear();
        mpttNodes << rootMptt << child1Mptt << child2Mptt << child3Mptt;
        tree = Mptt::fromMpttVector(mpttNodes);
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

        mpttNodes.clear();
        delete root;        root = nullptr;
        child1 = nullptr;
        child2 = nullptr;
        child3 = nullptr;
    }
    catch (...) { QVERIFY(0); }
}


QTEST_APPLESS_MAIN(TestMptt)
#include "TestMptt.moc"
