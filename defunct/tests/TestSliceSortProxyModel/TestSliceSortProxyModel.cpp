/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include <QtTest/QTest>

#include <QtSql/QSqlField>

#include "../../models/AdocTreeModel.h"

#include "../AdocTreeNode.h"
#include "../DataRow.h"

#include "../../models/SliceProxyModel.h"
#include "../../models/SliceSortProxyModel.h"

#include <QtDebug>

/**
  * Creates a 2 column mock slice model that consists of all the children of the parent
  * AdocTreeNode. The first column is the nodes label. Only non-group nodes have a non-NULL
  * value for the second column and this is an integer which starts at 1 and is incremented
  * with each non-group slice row that is added.
  */
class MockSliceProxyModel : public SliceProxyModel
{
    Q_OBJECT

public:
    MockSliceProxyModel(QObject *parent = 0) : SliceProxyModel(parent)
    {
    }

    int columnCount(const QModelIndex &parent = QModelIndex()) const
    {
        Q_UNUSED(parent);

        return 2;
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const
    {
        if (!index.isValid() || index.column() >= columnCount() || index.row() >= rowCount())
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            AdocTreeNode *node = slice_.sourceNodes_.at(index.row());
            if (index.column() == 0)
                return node->label_;

            if (node->nodeType_ == AdocTreeNode::GroupType)
                return QVariant(QVariant::Bool);
        }

        return QVariant();
    }
};

class TestSliceSortProxyModel : public QObject
{
    Q_OBJECT

private slots:
    void setSourceModel();
    void sort();
};

void TestSliceSortProxyModel::setSourceModel()
{
    MockSliceProxyModel *mockSliceProxyModel = new MockSliceProxyModel(this);
    SliceSortProxyModel *sortModel = new SliceSortProxyModel(this);
    sortModel->setSourceModel(0);

    QVERIFY(sortModel->sourceModel() == 0);
    sortModel->setSourceModel(mockSliceProxyModel);
    QVERIFY(sortModel->sourceModel() == mockSliceProxyModel);
    sortModel->setSourceModel(0);
    QVERIFY(sortModel->sourceModel() == 0);
}


void TestSliceSortProxyModel::sort()
{
    SliceSortProxyModel *sortModel = new SliceSortProxyModel(this);
    MockSliceProxyModel *mockSliceProxyModel = new MockSliceProxyModel(this);
    AdocTreeModel *treeModel = new AdocTreeModel(this);

    // Setup
    mockSliceProxyModel->setTreeModel(treeModel);
    sortModel->setSourceModel(mockSliceProxyModel);

    // -------------------------------------
    // Test: one group, one item already sorted (meaning, the group appears first)
    AdocTreeNode *root = treeModel->root();
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupA"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "A"));
    mockSliceProxyModel->setSourceParent(QModelIndex());

    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));
    sortModel->sort(0, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));
    sortModel->sort(0, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));
    sortModel->sort(1, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));
    sortModel->sort(1, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));

    // -------------------------------------
    // Test: one group, one item not sorted (the group occurs second in the tree)
    root->removeChildren(); // Hackish way to remove children
    mockSliceProxyModel->clear();       // Clear to reset the proxy model, since we are removing children outside of the tree model interface
    root->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "A"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupA"));
    mockSliceProxyModel->setSourceParent(QModelIndex());
    QCOMPARE(sortModel->rowCount(), 2);

    // No matter the sort, should place the group first and the non-group second
    sortModel->sort(0, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));
    sortModel->sort(0, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));
    sortModel->sort(1, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));
    sortModel->sort(1, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("A"));

    // -------------------------------------
    // Test: two groups, one non-group item
    root->removeChildren();
    mockSliceProxyModel->clear();       // Clear to reset the proxy model, since we are removing children outside of the tree model interface
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupB"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "A"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupA"));
    mockSliceProxyModel->setSourceParent(QModelIndex());
    QCOMPARE(sortModel->rowCount(), 3);

    sortModel->sort(0, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("A"));
    sortModel->sort(0, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("A"));
    sortModel->sort(1, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("A"));
    sortModel->sort(1, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("A"));

    // -------------------------------------
    // Test: two groups, two non-groups
    root->removeChildren();
    mockSliceProxyModel->clear();       // Clear to reset the proxy model, since we are removing children outside of the tree model interface
    root->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "B"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "A"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupB"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupA"));
    mockSliceProxyModel->setSourceParent(QModelIndex());

    sortModel->sort(0, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("A"));
    QCOMPARE(sortModel->index(3, 0).data().toString(), QString("B"));
    sortModel->sort(0, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("B"));
    QCOMPARE(sortModel->index(3, 0).data().toString(), QString("A"));
    sortModel->sort(1, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("A"));
    QCOMPARE(sortModel->index(3, 0).data().toString(), QString("B"));
    sortModel->sort(1, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("B"));
    QCOMPARE(sortModel->index(3, 0).data().toString(), QString("A"));

    // -------------------------------------
    // Test: three groups, three non-groups
    root->removeChildren();
    mockSliceProxyModel->clear();       // Clear to reset the proxy model, since we are removing children outside of the tree model interface
    root->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "C"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupB"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "A"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupA"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::SubseqAminoType, "B"));
    root->appendChild(new AdocTreeNode(AdocTreeNode::GroupType, "GroupC"));
    mockSliceProxyModel->setSourceParent(QModelIndex());

    sortModel->sort(0, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("GroupC"));
    QCOMPARE(sortModel->index(3, 0).data().toString(), QString("A"));
    QCOMPARE(sortModel->index(4, 0).data().toString(), QString("B"));
    QCOMPARE(sortModel->index(5, 0).data().toString(), QString("C"));
    sortModel->sort(0, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupC"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(3, 0).data().toString(), QString("C"));
    QCOMPARE(sortModel->index(4, 0).data().toString(), QString("B"));
    QCOMPARE(sortModel->index(5, 0).data().toString(), QString("A"));
    sortModel->sort(1, Qt::AscendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupC"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(3, 0).data().toString(), QString("A"));
    QCOMPARE(sortModel->index(4, 0).data().toString(), QString("B"));
    QCOMPARE(sortModel->index(5, 0).data().toString(), QString("C"));
    sortModel->sort(1, Qt::DescendingOrder);
    QCOMPARE(sortModel->index(0, 0).data().toString(), QString("GroupC"));
    QCOMPARE(sortModel->index(1, 0).data().toString(), QString("GroupB"));
    QCOMPARE(sortModel->index(2, 0).data().toString(), QString("GroupA"));
    QCOMPARE(sortModel->index(3, 0).data().toString(), QString("C"));
    QCOMPARE(sortModel->index(4, 0).data().toString(), QString("B"));
    QCOMPARE(sortModel->index(5, 0).data().toString(), QString("A"));
}

QTEST_MAIN(TestSliceSortProxyModel)
#include "TestSliceSortProxyModel.moc"
