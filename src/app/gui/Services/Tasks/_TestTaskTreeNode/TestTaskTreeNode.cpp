/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QScopedPointer>
#include <QtTest/QtTest>

#include "../TaskTreeNode.h"
#include "../Task.h"

class TestTaskTreeNode : public QObject
{
    Q_OBJECT

private slots:
    void anyTestActive();
    void allTasksOver();
    void contains();
    void firstLeafWithStatus();
    void hasTaskWithStatus();
    void rootTaskNode();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestTaskTreeNode::anyTestActive()
{
    QScopedPointer<TaskTreeNode> root(new TaskTreeNode(new Task(Ag::Group, "Root")));
    TaskTreeNode *child1(new TaskTreeNode(new Task(Ag::Leaf, "Child1")));
    TaskTreeNode *child2(new TaskTreeNode(new Task(Ag::Group, "Child2")));
    TaskTreeNode *child21(new TaskTreeNode(new Task(Ag::Leaf, "Child21")));
    TaskTreeNode *child22(new TaskTreeNode(new Task(Ag::Leaf, "Child22")));
    TaskTreeNode *child23(new TaskTreeNode(new Task(Ag::Group, "Child23")));

    root->appendChild(child1);
    root->appendChild(child2);
    child2->appendChild(child21);
    child2->appendChild(child22);
    child2->appendChild(child23);

    QVERIFY(!root->anyTaskActive());

    child22->task()->setStatus(Ag::Running);
    QVERIFY(root->anyTaskActive());
    child22->task()->setStatus(Ag::NotStarted);
    QVERIFY(!root->anyTaskActive());

    child21->task()->setStatus(Ag::Starting);
    QVERIFY(root->anyTaskActive());
    child21->task()->setStatus(Ag::NotStarted);
    QVERIFY(!root->anyTaskActive());

    child1->task()->setStatus(Ag::Paused);
    QVERIFY(root->anyTaskActive());
    child1->task()->setStatus(Ag::NotStarted);
    QVERIFY(!root->anyTaskActive());

    QVERIFY(!child23->anyTaskActive());
    child23->task()->setStatus(Ag::Running);
    QVERIFY(!child23->anyTaskActive());
}

void TestTaskTreeNode::allTasksOver()
{
    QScopedPointer<TaskTreeNode> root(new TaskTreeNode(new Task(Ag::Group, "Root")));
    TaskTreeNode *child1(new TaskTreeNode(new Task(Ag::Leaf, "Child1")));
    TaskTreeNode *child2(new TaskTreeNode(new Task(Ag::Group, "Child2")));
    TaskTreeNode *child21(new TaskTreeNode(new Task(Ag::Leaf, "Child21")));
    TaskTreeNode *child22(new TaskTreeNode(new Task(Ag::Leaf, "Child22")));
    TaskTreeNode *child23(new TaskTreeNode(new Task(Ag::Group, "Child23")));

    root->appendChild(child1);
    root->appendChild(child2);
    child2->appendChild(child21);
    child2->appendChild(child22);
    child2->appendChild(child23);

    // All leaf tasks start off as NotStarted
    QVERIFY(!root->allTasksOver());
    QVERIFY(!child1->allTasksOver());

    child1->task()->setStatus(Ag::Finished);
    QVERIFY(!root->allTasksOver());
    child21->task()->setStatus(Ag::Error);
    QVERIFY(!root->allTasksOver());
    child22->task()->setStatus(Ag::Killed);
    QVERIFY(root->allTasksOver());

    // Test: setting a group status to running does not necessarily make sense, but it should not change the all
    // tasks over method result
    child2->task()->setStatus(Ag::Running);
    QVERIFY(root->allTasksOver());
}

void TestTaskTreeNode::contains()
{
    QScopedPointer<TaskTreeNode> root(new TaskTreeNode(new Task(Ag::Group, "Root")));
    TaskTreeNode *child1(new TaskTreeNode(new Task(Ag::Leaf, "Child1")));
    TaskTreeNode *child2(new TaskTreeNode(new Task(Ag::Group, "Child2")));
    TaskTreeNode *child21(new TaskTreeNode(new Task(Ag::Leaf, "Child21")));
    TaskTreeNode *child22(new TaskTreeNode(new Task(Ag::Leaf, "Child22")));
    TaskTreeNode *child23(new TaskTreeNode(new Task(Ag::Group, "Child23")));

    root->appendChild(child1);
    root->appendChild(child2);
    child2->appendChild(child21);
    child2->appendChild(child22);
    child2->appendChild(child23);

    QVERIFY(root->contains(root->task()));
    QVERIFY(root->contains(child1->task()));
    QVERIFY(root->contains(child2->task()));
    QVERIFY(root->contains(child21->task()));
    QVERIFY(root->contains(child22->task()));
    QVERIFY(root->contains(child23->task()));

    QVERIFY(!child1->contains(root->task()));
    QVERIFY(!child1->contains(child2->task()));
    QVERIFY(!child1->contains(child21->task()));
    QVERIFY(!child1->contains(child22->task()));
    QVERIFY(!child1->contains(child23->task()));

    QVERIFY(!child2->contains(root->task()));
    QVERIFY(!child2->contains(child1->task()));
    QVERIFY(child2->contains(child2->task()));
    QVERIFY(child2->contains(child21->task()));
    QVERIFY(child2->contains(child22->task()));
    QVERIFY(child2->contains(child23->task()));
}

void TestTaskTreeNode::firstLeafWithStatus()
{
    QScopedPointer<TaskTreeNode> root(new TaskTreeNode(new Task(Ag::Group, "Root")));
    TaskTreeNode *child1(new TaskTreeNode(new Task(Ag::Leaf, "Child1")));
    TaskTreeNode *child2(new TaskTreeNode(new Task(Ag::Group, "Child2")));
    TaskTreeNode *child21(new TaskTreeNode(new Task(Ag::Leaf, "Child21")));
    TaskTreeNode *child22(new TaskTreeNode(new Task(Ag::Leaf, "Child22")));
    TaskTreeNode *child23(new TaskTreeNode(new Task(Ag::Group, "Child23")));

    root->appendChild(child1);
    root->appendChild(child2);
    child2->appendChild(child21);
    child2->appendChild(child22);
    child2->appendChild(child23);

    QVERIFY(root->firstLeafWithStatus(Ag::NotStarted) == child1);
    QVERIFY(child2->firstLeafWithStatus(Ag::NotStarted) == child21);
    child21->task()->setStatus(Ag::Running);
    QVERIFY(child2->firstLeafWithStatus(Ag::NotStarted) == child22);
    child22->task()->setStatus(Ag::Finished);
    QVERIFY(child2->firstLeafWithStatus(Ag::NotStarted) == nullptr);

    QVERIFY(root->firstLeafWithStatus(Ag::NotApplicable) == nullptr);

    QVERIFY(root->firstLeafWithStatus(Ag::Finished) == child22);
    child21->task()->setStatus(Ag::Finished);
    QVERIFY(root->firstLeafWithStatus(Ag::Finished) == child21);
}

void TestTaskTreeNode::hasTaskWithStatus()
{
    QScopedPointer<TaskTreeNode> root(new TaskTreeNode(new Task(Ag::Group, "Root")));
    TaskTreeNode *child1(new TaskTreeNode(new Task(Ag::Leaf, "Child1")));
    TaskTreeNode *child2(new TaskTreeNode(new Task(Ag::Group, "Child2")));
    TaskTreeNode *child21(new TaskTreeNode(new Task(Ag::Leaf, "Child21")));
    TaskTreeNode *child22(new TaskTreeNode(new Task(Ag::Leaf, "Child22")));
    TaskTreeNode *child23(new TaskTreeNode(new Task(Ag::Group, "Child23")));

    root->appendChild(child1);
    root->appendChild(child2);
    child2->appendChild(child21);
    child2->appendChild(child22);
    child2->appendChild(child23);

    QVERIFY(root->hasTaskWithStatus(Ag::NotStarted));
    QVERIFY(child2->hasTaskWithStatus(Ag::NotStarted));
    child21->task()->setStatus(Ag::Running);
    QVERIFY(child2->hasTaskWithStatus(Ag::NotStarted));
    child22->task()->setStatus(Ag::Finished);
    QVERIFY(child2->hasTaskWithStatus(Ag::NotStarted) == false);

    QVERIFY(root->hasTaskWithStatus(Ag::NotApplicable) == false);

    QVERIFY(root->hasTaskWithStatus(Ag::Finished));
    child21->task()->setStatus(Ag::Finished);
    QVERIFY(root->hasTaskWithStatus(Ag::Finished));
}

void TestTaskTreeNode::rootTaskNode()
{
    QScopedPointer<TaskTreeNode> root(new TaskTreeNode(new Task(Ag::Group, "Root")));
    TaskTreeNode *child1(new TaskTreeNode(new Task(Ag::Leaf, "Child1")));
    TaskTreeNode *child2(new TaskTreeNode(new Task(Ag::Group, "Child2")));
    TaskTreeNode *child21(new TaskTreeNode(new Task(Ag::Leaf, "Child21")));
    TaskTreeNode *child22(new TaskTreeNode(new Task(Ag::Leaf, "Child22")));
    TaskTreeNode *child23(new TaskTreeNode(new Task(Ag::Group, "Child23")));

    root->appendChild(child1);
    root->appendChild(child2);
    child2->appendChild(child21);
    child2->appendChild(child22);
    child2->appendChild(child23);

    QVERIFY(root->rootTaskNode() == root.data());
    QVERIFY(child1->rootTaskNode() == root.data());
    QVERIFY(child2->rootTaskNode() == root.data());
    QVERIFY(child21->rootTaskNode() == root.data());
    QVERIFY(child22->rootTaskNode() == root.data());
    QVERIFY(child23->rootTaskNode() == root.data());
}

QTEST_MAIN(TestTaskTreeNode)
#include "TestTaskTreeNode.moc"
