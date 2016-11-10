/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>

#include "TreeNode.h"

class MockTreeNode : public TreeNode<MockTreeNode>
{
};

enum Names
{
    Luke = 0,
    Megan,
    Paul,
    Caleb
};

class TestTreeNode : public QObject
{
    Q_OBJECT

public:
    TestTreeNode()
    {
        isolate_ = false;
    }

private slots:
    // ------------------------------------------------------------------------------------------------
    // Constructors
    void constructor();
//    void copyConstructor();

    // ------------------------------------------------------------------------------------------------
    // Public methods
    void childAt();
    void childrenBetween();
    void childCount();
    void hasChildren();
    void isDescendantOf();
    void isEqualOrDescendantOfAny();
    void isLeaf();
    void isRoot();
    void nextAscendant();
    void nextSibling();
    void parent();
    void row();
    void removeChildAt();
    void removeChildren();
    void removeChildrenRange();
    void root();
    void appendChild();
    void appendChildren();
    void insertChildAt();
    void takeChildren();
    void takeChildrenRange();

    // ------------------------------------------------------------------------------------------------
    // Iterators
    void iterator();

private:
    bool isolate_;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
void TestTreeNode::constructor()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    QVERIFY(root->parent() == 0);

    delete root;
}

/*
void TestTreeNode::copyConstructor()
{
    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child3 = new MockTreeNode;

    root->appendChild(child);
    root->appendChild(child2);
    root->appendChild(child3);

    MockTreeNode *rootCopy = new MockTreeNode(*root);
    QVERIFY(rootCopy->childCount() == 0);
    QVERIFY(rootCopy->parent() == 0);

    MockTreeNode *childCopy = new MockTreeNode(*child);
    QVERIFY(childCopy->childCount() == 0);
    QVERIFY(childCopy->parent() == 0);

    delete root;
    root = 0;
    child = 0;
    child2 = 0;
    child3 = 0;

    delete rootCopy;
    rootCopy = 0;
    delete childCopy;
    childCopy = 0;
}
*/

void TestTreeNode::childAt()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child3 = new MockTreeNode;

    root->appendChild(child);
    root->appendChild(child2);
    root->appendChild(child3);

    QVERIFY(root->childAt(0) == child);
    QVERIFY(root->childAt(1) == child2);
    QVERIFY(root->childAt(2) == child3);

    delete root;
    root = 0;
    child = 0;
    child2 = 0;
    child3 = 0;
}

void TestTreeNode::childrenBetween()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child3 = new MockTreeNode;

    root->appendChild(child);
    root->appendChild(child2);
    root->appendChild(child3);

    QVector<MockTreeNode *> result;
    result = root->childrenBetween(0, 0);
    QCOMPARE(result.count(), 1);
    QCOMPARE(result.at(0), child);

    result = root->childrenBetween(1, 2);
    QCOMPARE(result.count(), 2);
    QCOMPARE(result.at(0), child2);
    QCOMPARE(result.at(1), child3);

    delete root;
    root = 0;
    child = 0;
    child2 = 0;
    child3 = 0;
}

void TestTreeNode::childCount()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child3 = new MockTreeNode;

    root->appendChild(child);
    root->appendChild(child2);
    root->appendChild(child3);

    QVERIFY(root->childCount() == 3);
    QVERIFY(child->childCount() == 0);
    QVERIFY(child2->childCount() == 0);
    QVERIFY(child3->childCount() == 0);

    delete root;
    root = 0;
    child = 0;
    child2 = 0;
    child3 = 0;
}

void TestTreeNode::hasChildren()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child21 = new MockTreeNode;
    MockTreeNode *child211 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child11);
    child1->appendChild(child12);
    root->appendChild(child2);
    child2->appendChild(child21);
    child21->appendChild(child211);

    QVERIFY(root->hasChildren());
    QVERIFY(child1->hasChildren());
    QVERIFY(!child11->hasChildren());
    QVERIFY(!child12->hasChildren());
    QVERIFY(child2->hasChildren());
    QVERIFY(child21->hasChildren());
    QVERIFY(!child211->hasChildren());

    delete root;
}

void TestTreeNode::isDescendantOf()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child21 = new MockTreeNode;
    MockTreeNode *child211 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child11);
    child1->appendChild(child12);
    root->appendChild(child2);
    child2->appendChild(child21);
    child21->appendChild(child211);

    // None can be descendant of zero
    QVERIFY(root->isDescendantOf(0) == false);
    QVERIFY(child1->isDescendantOf(0) == false);
    QVERIFY(child11->isDescendantOf(0) == false);
    QVERIFY(child12->isDescendantOf(0) == false);
    QVERIFY(child2->isDescendantOf(0) == false);
    QVERIFY(child21->isDescendantOf(0) == false);
    QVERIFY(child211->isDescendantOf(0) == false);

    // Test: isolated node
    MockTreeNode *soloNode = new MockTreeNode;
    QVERIFY(root->isDescendantOf(soloNode) == false);
    QVERIFY(child1->isDescendantOf(soloNode) == false);
    QVERIFY(child11->isDescendantOf(soloNode) == false);
    QVERIFY(child12->isDescendantOf(soloNode) == false);
    QVERIFY(child2->isDescendantOf(soloNode) == false);
    QVERIFY(child21->isDescendantOf(soloNode) == false);
    QVERIFY(child211->isDescendantOf(soloNode) == false);
    delete soloNode;
    soloNode = 0;

    // None can be a descendant of itself
    QVERIFY(root->isDescendantOf(root) == false);
    QVERIFY(child1->isDescendantOf(child1) == false);
    QVERIFY(child11->isDescendantOf(child11) == false);
    QVERIFY(child12->isDescendantOf(child12) == false);
    QVERIFY(child2->isDescendantOf(child2) == false);
    QVERIFY(child21->isDescendantOf(child21) == false);
    QVERIFY(child211->isDescendantOf(child211) == false);

    // Root
    QVERIFY(root->isDescendantOf(child1) == false);
    QVERIFY(root->isDescendantOf(child11) == false);
    QVERIFY(root->isDescendantOf(child12) == false);
    QVERIFY(root->isDescendantOf(child2) == false);
    QVERIFY(root->isDescendantOf(child21) == false);
    QVERIFY(root->isDescendantOf(child211) == false);

    // Child1
    QVERIFY(child1->isDescendantOf(root));
    QVERIFY(child1->isDescendantOf(child11) == false);
    QVERIFY(child1->isDescendantOf(child12) == false);
    QVERIFY(child1->isDescendantOf(child2) == false);
    QVERIFY(child1->isDescendantOf(child21) == false);
    QVERIFY(child1->isDescendantOf(child211) == false);

    // Child11
    QVERIFY(child11->isDescendantOf(root));
    QVERIFY(child11->isDescendantOf(child1));
    QVERIFY(child11->isDescendantOf(child12) == false);
    QVERIFY(child11->isDescendantOf(child2) == false);
    QVERIFY(child11->isDescendantOf(child21) == false);
    QVERIFY(child11->isDescendantOf(child211) == false);

    // Child12
    QVERIFY(child12->isDescendantOf(root));
    QVERIFY(child12->isDescendantOf(child1));
    QVERIFY(child12->isDescendantOf(child11) == false);
    QVERIFY(child12->isDescendantOf(child2) == false);
    QVERIFY(child12->isDescendantOf(child21) == false);
    QVERIFY(child12->isDescendantOf(child211) == false);

    // Child2
    QVERIFY(child2->isDescendantOf(root));
    QVERIFY(child2->isDescendantOf(child1) == false);
    QVERIFY(child2->isDescendantOf(child11) == false);
    QVERIFY(child2->isDescendantOf(child12) == false);
    QVERIFY(child2->isDescendantOf(child21) == false);
    QVERIFY(child2->isDescendantOf(child211) == false);

    // Child21
    QVERIFY(child21->isDescendantOf(root));
    QVERIFY(child21->isDescendantOf(child1) == false);
    QVERIFY(child21->isDescendantOf(child11) == false);
    QVERIFY(child21->isDescendantOf(child12) == false);
    QVERIFY(child21->isDescendantOf(child2));
    QVERIFY(child21->isDescendantOf(child211) == false);

    // Child211
    QVERIFY(child211->isDescendantOf(root));
    QVERIFY(child211->isDescendantOf(child1) == false);
    QVERIFY(child211->isDescendantOf(child11) == false);
    QVERIFY(child211->isDescendantOf(child12) == false);
    QVERIFY(child211->isDescendantOf(child2));
    QVERIFY(child211->isDescendantOf(child21));

    delete root;
    root = 0;
    child1 = 0;
    child11 = 0;
    child12 = 0;
    child2 = 0;
    child21 = 0;
    child211 = 0;
}

void TestTreeNode::isEqualOrDescendantOfAny()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child21 = new MockTreeNode;
    MockTreeNode *child211 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child11);
    child1->appendChild(child12);
    root->appendChild(child2);
    child2->appendChild(child21);
    child21->appendChild(child211);

    // None can be descendant of zero
    QVERIFY(root->isEqualOrDescendantOfAny(0, 0, 1) == false);
    QVERIFY(child1->isEqualOrDescendantOfAny(0, 0, 1) == false);
    QVERIFY(child11->isEqualOrDescendantOfAny(0, 0, 1) == false);
    QVERIFY(child12->isEqualOrDescendantOfAny(0, 0, 1) == false);
    QVERIFY(child2->isEqualOrDescendantOfAny(0, 0, 1) == false);
    QVERIFY(child21->isEqualOrDescendantOfAny(0, 0, 1) == false);
    QVERIFY(child211->isEqualOrDescendantOfAny(0, 0, 1) == false);

    // None can be a descendant of itself
    QVERIFY(root->isEqualOrDescendantOfAny(root, 0, 1) == false);
    QVERIFY(child1->isEqualOrDescendantOfAny(child1, 0, 1) == false);
    QVERIFY(child2->isEqualOrDescendantOfAny(child2, 0, 1) == false);
    QVERIFY(child21->isEqualOrDescendantOfAny(child21, 0, 1) == false);

    // Root - never a descendant of any other node
    QVERIFY(root->isEqualOrDescendantOfAny(child1, 0, 1) == false);
    QVERIFY(root->isEqualOrDescendantOfAny(child2, 0, 1) == false);
    QVERIFY(root->isEqualOrDescendantOfAny(child21, 0, 1) == false);

    // Child1
    QVERIFY(child1->isEqualOrDescendantOfAny(root, 0, 1));
    QVERIFY(child1->isEqualOrDescendantOfAny(root, 1, 1) == false);
    QVERIFY(child1->isEqualOrDescendantOfAny(child2, 0, 1) == false);
    QVERIFY(child1->isEqualOrDescendantOfAny(child21, 0, 1) == false);

    // Child11
    QVERIFY(child11->isEqualOrDescendantOfAny(root, 0, 2));
    QVERIFY(child11->isEqualOrDescendantOfAny(root, 0, 1));
    QVERIFY(child11->isEqualOrDescendantOfAny(root, 1, 1) == false);
    QVERIFY(child11->isEqualOrDescendantOfAny(child1, 0, 2));
    QVERIFY(child11->isEqualOrDescendantOfAny(child1, 0, 1));
    QVERIFY(child11->isEqualOrDescendantOfAny(child1, 1, 1) == false);
    QVERIFY(child11->isEqualOrDescendantOfAny(child2, 0, 1) == false);
    QVERIFY(child11->isEqualOrDescendantOfAny(child21, 0, 1) == false);

    // Child12
    QVERIFY(child12->isEqualOrDescendantOfAny(root, 0, 2));
    QVERIFY(child12->isEqualOrDescendantOfAny(root, 0, 1));
    QVERIFY(child12->isEqualOrDescendantOfAny(root, 1, 1) == false);
    QVERIFY(child12->isEqualOrDescendantOfAny(child1, 0, 2));
    QVERIFY(child12->isEqualOrDescendantOfAny(child1, 0, 1) == false);
    QVERIFY(child12->isEqualOrDescendantOfAny(child1, 1, 1));
    QVERIFY(child12->isEqualOrDescendantOfAny(child2, 0, 1) == false);
    QVERIFY(child12->isEqualOrDescendantOfAny(child21, 0, 1) == false);

    // Child2
    QVERIFY(child2->isEqualOrDescendantOfAny(root, 0, 2));
    QVERIFY(child2->isEqualOrDescendantOfAny(root, 0, 1) == false);
    QVERIFY(child2->isEqualOrDescendantOfAny(root, 1, 1));
    QVERIFY(child2->isEqualOrDescendantOfAny(child1, 0, 2) == false);
    QVERIFY(child2->isEqualOrDescendantOfAny(child2, 0, 1) == false);
    QVERIFY(child2->isEqualOrDescendantOfAny(child21, 0, 1) == false);

    // Child21
    QVERIFY(child21->isEqualOrDescendantOfAny(root, 0, 2));
    QVERIFY(child21->isEqualOrDescendantOfAny(root, 0, 1) == false);
    QVERIFY(child21->isEqualOrDescendantOfAny(root, 1, 1));
    QVERIFY(child21->isEqualOrDescendantOfAny(child1, 0, 2) == false);
    QVERIFY(child21->isEqualOrDescendantOfAny(child2, 0, 1));

    // Child211
    QVERIFY(child211->isEqualOrDescendantOfAny(root, 0, 2));
    QVERIFY(child211->isEqualOrDescendantOfAny(root, 0, 1) == false);
    QVERIFY(child211->isEqualOrDescendantOfAny(root, 1, 1));
    QVERIFY(child211->isEqualOrDescendantOfAny(child1, 0, 2) == false);
    QVERIFY(child211->isEqualOrDescendantOfAny(child2, 0, 1));
    QVERIFY(child211->isEqualOrDescendantOfAny(child21, 0, 1));

    delete root;
    root = 0;
    child1 = 0;
    child11 = 0;
    child12 = 0;
    child2 = 0;
    child21 = 0;
    child211 = 0;
}

void TestTreeNode::isLeaf()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    QVERIFY(root->isLeaf());
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    root->appendChild(child1);
    QVERIFY(root->isLeaf() == false);
    QVERIFY(child1->isLeaf());
    child1->appendChild(child11);
    QVERIFY(child11->isLeaf());

    delete root;
    root = nullptr;
    child1 = nullptr;
    child11 =  nullptr;
}

void TestTreeNode::isRoot()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    QVERIFY(root->isRoot());
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    root->appendChild(child1);
    QVERIFY(child1->isRoot() == false);
    child1->appendChild(child11);
    QVERIFY(child11->isRoot() == false);

    delete root;
    root = nullptr;
    child1 = nullptr;
    child11 =  nullptr;
}

void TestTreeNode::nextAscendant()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child21 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child11);
    child1->appendChild(child12);
    root->appendChild(child2);
    child2->appendChild(child21);

    QVERIFY(root->nextAscendant() == nullptr);
    QVERIFY(child1->nextAscendant() == child2);
    QVERIFY(child11->nextAscendant() == child12);
    QVERIFY(child12->nextAscendant() == child2);
    QVERIFY(child2->nextAscendant() == nullptr);
    QVERIFY(child21->nextAscendant() == nullptr);

    delete root;
    child1 = 0;
    child11 = 0;
    child12 = 0;
    child2 = 0;
    child21 = 0;
}

void TestTreeNode::nextSibling()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child21 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child11);
    child1->appendChild(child12);
    root->appendChild(child2);
    child2->appendChild(child21);

    QVERIFY(root->nextSibling() == nullptr);
    QVERIFY(child1->nextSibling() == child2);
    QVERIFY(child11->nextSibling() == child12);
    QVERIFY(child12->nextSibling() == nullptr);
    QVERIFY(child2->nextSibling() == nullptr);
    QVERIFY(child21->nextSibling() == nullptr);

    delete root;
    child1 = 0;
    child11 = 0;
    child12 = 0;
    child2 = 0;
    child21 = 0;
}

void TestTreeNode::parent()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child13 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child12);
    child1->appendChild(child13);

    QVERIFY(root->parent() == 0);
    QVERIFY(child1->parent() == root);
    QVERIFY(child12->parent() == child1);
    QVERIFY(child13->parent() == child1);

    delete root;
    root = 0;
    child1 = 0;
    child12 = 0;
    child13 = 0;
}

void TestTreeNode::row()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child13 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child12);
    child1->appendChild(child13);

    QVERIFY(root->row() == 0);
    QVERIFY(child1->row() == 0);
    QVERIFY(child12->row() == 0);
    QVERIFY(child13->row() == 1);

    delete root;
    root = 0;
    child1 = 0;
    child12 = 0;
    child13 = 0;
}

void TestTreeNode::removeChildAt()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child13 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child12);
    child1->appendChild(child13);
    root->appendChild(child2);

    QVERIFY(root->childCount() == 2);
    root->removeChildAt(1);
    QVERIFY(root->childCount() == 1);

    QVERIFY(child1->childCount() == 2);
    child1->removeChildAt(child12->row());
    QVERIFY(child1->childCount() == 1);
    QVERIFY(child1->childAt(0) == child13);

    root->removeChildAt(child1->row());
    QVERIFY(root->childCount() == 0);

    delete root;
    root = 0;
    child1 = 0;
    child12 = 0;
    child13 = 0;
    child2 = 0;
}

void TestTreeNode::removeChildren()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child13 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child12);
    child1->appendChild(child13);
    root->appendChild(child2);

    QVERIFY(child1->childCount() == 2);
    child1->removeChildren();
    QVERIFY(child1->childCount() == 0);

    root->removeChildren();
    QVERIFY(root->childCount() == 0);

    delete root;
    root = 0;
    child1 = 0;
    child12 = 0;
    child13 = 0;
    child2 = 0;
}

void TestTreeNode::removeChildrenRange()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child3 = new MockTreeNode;
    MockTreeNode *child4 = new MockTreeNode;

    root->appendChild(child1);
    root->appendChild(child2);
    root->appendChild(child3);
    root->appendChild(child4);

    QVERIFY(root->childCount() == 4);
    root->removeChildren(0, 1);
    child1 = 0;
    QVERIFY(root->childCount() == 3);
    QVERIFY(root->childAt(0) == child2);
    QVERIFY(root->childAt(1) == child3);
    QVERIFY(root->childAt(2) == child4);
    child3 = 0;
    child4 = 0;

    root->removeChildren(1, 2);
    QVERIFY(root->childCount() == 1);
    QVERIFY(root->childAt(0) == child2);

    delete root;
    root = 0;
    child2 = 0;
}

void TestTreeNode::root()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child21 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child11);
    child1->appendChild(child12);
    root->appendChild(child2);
    child2->appendChild(child21);

    QVERIFY(root->root() == root);
    QVERIFY(child1->root() == root);
    QVERIFY(child11->root() == root);
    QVERIFY(child12->root() == root);
    QVERIFY(child2->root() == root);
    QVERIFY(child21->root() == root);

    delete root;
    child1 = 0;
    child11 = 0;
    child12 = 0;
    child2 = 0;
    child21 = 0;
}

void TestTreeNode::appendChild()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    root->appendChild(child1);
    QVERIFY(root->childCount() == 1);
    QVERIFY(root->childAt(0) == child1);
    QVERIFY(child1->parent() == root);

    delete root;
    root = 0;
    child1 = 0;
}

void TestTreeNode::appendChildren()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child3 = new MockTreeNode;
    MockTreeNode *child4 = new MockTreeNode;

    QVector<MockTreeNode *> children;

    // Test: appending empty list;
    root->appendChildren(children);
    QCOMPARE(root->childCount(), 0);

    // Test: append empty list to list with one child
    root->appendChild(child1);
    root->appendChildren(children);
    QCOMPARE(root->childCount(), 1);

    // Test: append list with one child
    children << child2;
    root->appendChildren(children);
    QCOMPARE(root->childCount(), 2);
    QCOMPARE(root->childAt(0), child1);
    QCOMPARE(root->childAt(1), child2);

    // Test: append multiple children
    children.clear();
    children << child3 << child4;
    root->appendChildren(children);
    QCOMPARE(root->childCount(), 4);
    QCOMPARE(root->childAt(0), child1);
    QCOMPARE(root->childAt(1), child2);
    QCOMPARE(root->childAt(2), child3);
    QCOMPARE(root->childAt(3), child4);

    delete root;
    root = 0;
    child1 = 0;
    child2 = 0;
    child3 = 0;
    child4 = 0;
}

void TestTreeNode::insertChildAt()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;

    // Test: insert at first position with empty list
    MockTreeNode *child1 = new MockTreeNode;
    root->insertChildAt(0, child1);

    QVERIFY(root->childCount() == 1);
    QVERIFY(root->childAt(0) == child1);

    // Test: insert at first position with non-empty list
    MockTreeNode *child2 = new MockTreeNode;
    root->insertChildAt(0, child2);
    QVERIFY(root->childCount() == 2);
    QVERIFY(root->childAt(0) == child2);
    QVERIFY(root->childAt(1) == child1);

    // Test: insert at end of list
    MockTreeNode *child3 = new MockTreeNode;
    root->insertChildAt(2, child3);
    QVERIFY(root->childCount() == 3);
    QVERIFY(root->childAt(0) == child2);
    QVERIFY(root->childAt(1) == child1);
    QVERIFY(root->childAt(2) == child3);

    // Test: insert into middle of list
    MockTreeNode *child4 = new MockTreeNode;
    root->insertChildAt(1, child4);
    QVERIFY(root->childCount() == 4);
    QVERIFY(root->childAt(0) == child2);
    QVERIFY(root->childAt(1) == child4);
    QVERIFY(root->childAt(2) == child1);
    QVERIFY(root->childAt(3) == child3);

    // Test: verify that each child has its parent pointer set properly
    QVERIFY(root->childAt(0)->parent() == root);
    QVERIFY(root->childAt(1)->parent() == root);
    QVERIFY(root->childAt(2)->parent() == root);
    QVERIFY(root->childAt(3)->parent() == root);

    delete root;
    root = 0;
    child1 = 0;
    child2 = 0;
    child3 = 0;
    child4 = 0;
}

void TestTreeNode::takeChildren()
{
    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child21 = new MockTreeNode;
    MockTreeNode *child211 = new MockTreeNode;

    // Test: taking children of isolated nodes should return empty lists
    QVERIFY(root->takeChildren().isEmpty());
    QVERIFY(child1->takeChildren().isEmpty());
    QVERIFY(child11->takeChildren().isEmpty());
    QVERIFY(child12->takeChildren().isEmpty());
    QVERIFY(child2->takeChildren().isEmpty());
    QVERIFY(child21->takeChildren().isEmpty());
    QVERIFY(child211->takeChildren().isEmpty());

    // Setup: build the tree
    root->appendChild(child1);
    child1->appendChild(child11);
    child1->appendChild(child12);
    root->appendChild(child2);
    child2->appendChild(child21);
    child21->appendChild(child211);

    // Test: taking children of leaf nodes should return empty lists
    QVERIFY(child11->takeChildren().isEmpty());
    QVERIFY(child12->takeChildren().isEmpty());
    QVERIFY(child211->takeChildren().isEmpty());
    QCOMPARE(child1->childCount(), 2);
    QCOMPARE(child21->childCount(), 1);

    // Test: take children off the root
    QVector<MockTreeNode *> rootChildren = root->takeChildren();
    QCOMPARE(root->childCount(), 0);
    QCOMPARE(rootChildren.at(0), child1);
    QCOMPARE(rootChildren.at(1), child2);
    QVERIFY(rootChildren.at(0)->parent() == 0);
    QVERIFY(rootChildren.at(1)->parent() == 0);

    // Add them back on for further testing
    root->appendChild(rootChildren.at(0));
    root->appendChild(rootChildren.at(1));
    rootChildren.clear();
    QCOMPARE(root->childCount(), 2);
    QVERIFY(child1->parent() == root);
    QVERIFY(child2->parent() == root);

    // Test: taking children of child1
    QVector<MockTreeNode *> child1Children = child1->takeChildren();
    QCOMPARE(child1->childCount(), 0);
    QCOMPARE(child1Children.at(0), child11);
    QCOMPARE(child1Children.at(1), child12);
    QVERIFY(child1Children.at(0)->parent() == 0);
    QVERIFY(child1Children.at(1)->parent() == 0);

    qDeleteAll(child1Children);
    child1Children.clear();

    delete root;
    root = 0;
    child1 = 0;
    child11 = 0;
    child12 = 0;
    child2 = 0;
    child21 = 0;
    child211 = 0;
}

void TestTreeNode::takeChildrenRange()
{
    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child3 = new MockTreeNode;
    MockTreeNode *child4 = new MockTreeNode;

    // Setup: build the tree
    root->appendChild(child1);
    root->appendChild(child2);
    root->appendChild(child3);
    root->appendChild(child4);

    QVERIFY(root->childCount() == 4);
    QVERIFY(root->takeChildren(1, 0).isEmpty());

    QVector<MockTreeNode *> nodes = root->takeChildren(0, 1);
    QCOMPARE(nodes.size(), 1);
    QCOMPARE(root->childCount(), 3);
    QVERIFY(nodes.at(0) == child1);
    delete child1;
    child1 = nullptr;

    nodes = root->takeChildren(1, 2);
    QCOMPARE(nodes.size(), 2);
    QCOMPARE(root->childCount(), 1);
    QVERIFY(nodes.at(0) == child3);
    delete child3;
    child3 = nullptr;
    QVERIFY(nodes.at(1) == child4);
    delete child4;
    child4 = nullptr;

    delete root;
    root = 0;
    child2 = 0;
}

void TestTreeNode::iterator()
{
    if (isolate_)
        return;

    MockTreeNode *root = new MockTreeNode;
    MockTreeNode *child1 = new MockTreeNode;
    MockTreeNode *child11 = new MockTreeNode;
    MockTreeNode *child12 = new MockTreeNode;
    MockTreeNode *child2 = new MockTreeNode;
    MockTreeNode *child21 = new MockTreeNode;

    root->appendChild(child1);
    child1->appendChild(child11);
    child1->appendChild(child12);
    root->appendChild(child2);
    child2->appendChild(child21);

    // Test: iterator beginning at root
    MockTreeNode::ConstIterator it = root;
    QVERIFY(it == root);
    ++it;
    QVERIFY(it == child1);
    ++it;
    QVERIFY(it == child11);
    ++it;
    QVERIFY(it == child12);
    ++it;
    QVERIFY(it == child2);
    ++it;
    QVERIFY(it == child21);
    ++it;
    QVERIFY(it == nullptr);

    // Test: iterator beginning at child11
    it = child11;
    QVERIFY(it == child11);
    ++it;
    QVERIFY(it == child12);
    ++it;
    QVERIFY(it == child2);
    ++it;
    QVERIFY(it == child21);
    ++it;
    QVERIFY(it == nullptr);

    // Test: iterator beginning at child12 and using post-increment operator
    it = child12;
    QVERIFY(it == child12);
    QVERIFY(it++ == child12);
    QVERIFY(it == child2);

    QVERIFY(it++ == child2);
    QVERIFY(it == child21);

    QVERIFY(it++ == child21);
    QVERIFY(it == nullptr);

    delete root;
    child1 = 0;
    child11 = 0;
    child12 = 0;
    child2 = 0;
    child21 = 0;
}

QTEST_MAIN(TestTreeNode)
#include "TestTreeNode.moc"
