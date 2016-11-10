/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtCore/QScopedPointer>

#include "../../../../core/PointerTreeNode.h"
#include "../TaskTreeFunctions.h"
#include "../Task.h"

using namespace TaskTreeFunctions;

class TestTaskTreeFunctions : public QObject
{
    Q_OBJECT

private slots:
    void hasActiveTask();
    void hasUnfinishedTask();
    void contains();
    void firstNodeWithStatus();
    void hasChildWithStatus();
    void findTask();
};

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Actual test functions
void TestTaskTreeFunctions::hasActiveTask()
{
    TaskTreeNode *root(new TaskTreeNode(new Task(Ag::Group, "Root")));
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

    QVERIFY(!TaskTreeFunctions::hasActiveTask(root));

    child22->data_->setStatus(Ag::Running);
    QVERIFY(TaskTreeFunctions::hasActiveTask(root));
    child22->data_->setStatus(Ag::NotStarted);
    QVERIFY(!TaskTreeFunctions::hasActiveTask(root));

    child21->data_->setStatus(Ag::Starting);
    QVERIFY(TaskTreeFunctions::hasActiveTask(root));
    child21->data_->setStatus(Ag::NotStarted);
    QVERIFY(!TaskTreeFunctions::hasActiveTask(root));

    child1->data_->setStatus(Ag::Paused);
    QVERIFY(TaskTreeFunctions::hasActiveTask(root));
    child1->data_->setStatus(Ag::NotStarted);
    QVERIFY(!TaskTreeFunctions::hasActiveTask(root));

    QVERIFY(!TaskTreeFunctions::hasActiveTask(child23));
    child23->data_->setStatus(Ag::Running);
    QVERIFY(!TaskTreeFunctions::hasActiveTask(child23));

    delete root;
}

void TestTaskTreeFunctions::hasUnfinishedTask()
{
    TaskTreeNode *root(new TaskTreeNode(new Task(Ag::Group, "Root")));
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
    QVERIFY(TaskTreeFunctions::hasUnfinishedTask(root));
    QVERIFY(TaskTreeFunctions::hasUnfinishedTask(child1));

    child1->data_->setStatus(Ag::Finished);
    QVERIFY(TaskTreeFunctions::hasUnfinishedTask(root));
    child21->data_->setStatus(Ag::Error);
    QVERIFY(TaskTreeFunctions::hasUnfinishedTask(root));
    child22->data_->setStatus(Ag::Killed);
    QVERIFY(!TaskTreeFunctions::hasUnfinishedTask(root));

    // Test: setting a group status to running does not necessarily make sense, but it should not change the all
    // tasks over method result
    child2->data_->setStatus(Ag::Running);
    QVERIFY(!TaskTreeFunctions::hasUnfinishedTask(root));

    delete root;
}

void TestTaskTreeFunctions::contains()
{
    TaskTreeNode *root(new TaskTreeNode(new Task(Ag::Group, "Root")));
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

    QVERIFY(TaskTreeFunctions::contains(root, root->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(root, child1->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(root, child2->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(root, child21->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(root, child22->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(root, child23->data_.data()));

    QVERIFY(!TaskTreeFunctions::contains(child1, root->data_.data()));
    QVERIFY(!TaskTreeFunctions::contains(child1, child2->data_.data()));
    QVERIFY(!TaskTreeFunctions::contains(child1, child21->data_.data()));
    QVERIFY(!TaskTreeFunctions::contains(child1, child22->data_.data()));
    QVERIFY(!TaskTreeFunctions::contains(child1, child23->data_.data()));

    QVERIFY(!TaskTreeFunctions::contains(child2, root->data_.data()));
    QVERIFY(!TaskTreeFunctions::contains(child2, child1->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(child2, child2->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(child2, child21->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(child2, child22->data_.data()));
    QVERIFY(TaskTreeFunctions::contains(child2, child23->data_.data()));

    delete root;
}

void TestTaskTreeFunctions::firstNodeWithStatus()
{
    TaskTreeNode *root(new TaskTreeNode(new Task(Ag::Group, "Root")));
    TaskTreeNode *child1(new TaskTreeNode(new Task(Ag::Leaf, "Child1")));
    TaskTreeNode *child2(new TaskTreeNode(new Task(Ag::Group, "Child2")));
    TaskTreeNode *child21(new TaskTreeNode(new Task(Ag::Leaf, "Child21"))); // *
    TaskTreeNode *child22(new TaskTreeNode(new Task(Ag::Leaf, "Child22"))); // *
    TaskTreeNode *child23(new TaskTreeNode(new Task(Ag::Group, "Child23")));

    root->appendChild(child1);
    root->appendChild(child2);
    child2->appendChild(child21);
    child2->appendChild(child22);
    child2->appendChild(child23);

    QVERIFY(TaskTreeFunctions::firstNodeWithStatus(root, Ag::NotStarted) == child1);
    QVERIFY(TaskTreeFunctions::firstNodeWithStatus(child2, Ag::NotStarted) == child21);
    child21->data_->setStatus(Ag::Running);
    QVERIFY(TaskTreeFunctions::firstNodeWithStatus(child2, Ag::NotStarted) == child22);
    child22->data_->setStatus(Ag::Finished);
    QVERIFY(TaskTreeFunctions::firstNodeWithStatus(child2, Ag::NotStarted) == nullptr);

    QVERIFY(TaskTreeFunctions::firstNodeWithStatus(root, Ag::NotApplicable) == nullptr);

    QVERIFY(TaskTreeFunctions::firstNodeWithStatus(root, Ag::Finished) == child22);
    child21->data_->setStatus(Ag::Finished);
    QVERIFY(TaskTreeFunctions::firstNodeWithStatus(root, Ag::Finished) == child21);

    delete root;
}

void TestTaskTreeFunctions::hasChildWithStatus()
{
}

void TestTaskTreeFunctions::findTask()
{
}



QTEST_MAIN(TestTaskTreeFunctions)
#include "TestTaskTreeFunctions.moc"
