/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef MOCKMODEL_H
#define MOCKMODEL_H

#include <QtGui/QAbstractProxyModel>

#include "TreeNode.h"

class MockNode : public TreeNode
{
public:
    MockNode(int id, QString name) : TreeNode(), id_(id), name_(name)
    {    }

    int id_;
    QString name_;
};

class MockModel : public QAbstractItemModel
{
public:
    MockModel(QObject * parent = 0) : QAbstractItemModel(parent), root_(0)
    {
    }

    ~MockModel()
    {
        if (root_)
            delete root_;
    }

    // Loads a sample data set for testing purposes
    // Root
    // |___ Group
    // |    |___ One
    // |    |___ Two
    // |    |___ Another group
    // |         |___ Five
    // |___ Three
    // |___ Four
    // |___ Lab members
    //      |___ Luke
    //      |___ Megan
    void loadTreeSetOne()
    {
        if (root_)
        {
            delete root_;
            root_ = 0;
        }

        root_ = new MockNode(0, "Root");
        MockNode *group = new MockNode(-1, "Group");
        root_->appendChild(group);
        MockNode *x = new MockNode(1, "One");
        MockNode *y = new MockNode(2, "Two");
        group->appendChild(x);
        group->appendChild(y);

        MockNode *group2 = new MockNode(-1, "Another group");
        group->appendChild(group2);
        MockNode *x2 = new MockNode(5, "Five");
        group2->appendChild(x2);

        root_->appendChild(new MockNode(3, "Three"));
        root_->appendChild(new MockNode(4, "Four"));

        MockNode *group3 = new MockNode(-1, "Lab members");
        root_->appendChild(group3);
        MockNode *luke = new MockNode(6, "Luke");
        group3->appendChild(luke);
        MockNode *megan = new MockNode(7, "Megan");
        group3->appendChild(megan);
    }

    // Appends mockNode as a child beneath parent
    void appendChild(MockNode *mockNode, const QModelIndex &parent = QModelIndex())
    {
        MockNode *parentNode = nodeFromIndex(parent);
        if (!parentNode)
            return;

        int row = parentNode->childCount();

        beginInsertRows(parent, row, row);
        parentNode->appendChild(mockNode);
        endInsertRows();
    }

    // Inserts mockNode at position row beneath parent
    bool insertRow(int row, MockNode *mockNode, const QModelIndex &parent)
    {
        Q_ASSERT_X(mockNode, "MockModel::appendRow", "adocTreeNode must not be empty");

        MockNode *parentNode = nodeFromIndex(parent);
        if (!parentNode)
            return false;

        Q_ASSERT_X(row >= 0, "MockModel::insertRow", QString("row out of range, must be >= 0 (invalid row value: %1)").arg(row).toAscii());
        Q_ASSERT_X(row <= parentNode->childCount(), "MockModel::insertRow", QString("row out of range, must be < parentNode->childCount() (%1) (invalid row value: %2)").arg(parentNode->childCount()).arg(row).toAscii());

        beginInsertRows(parent, row, row);
        parentNode->insertChildAt(row, mockNode);
        endInsertRows();

        return true;
    }

    // Inserts newRows at position row beneath parent
    bool insertRows(int row, QList<MockNode *> newRows, const QModelIndex &parent)
    {
        MockNode *parentNode = nodeFromIndex(parent);
        if (!parentNode)
            return false;

        Q_ASSERT_X(row >= 0, "MockModel::insertRows", QString("row out of range, must be >= 0 (invalid row value: %1)").arg(row).toAscii());
        Q_ASSERT_X(row <= parentNode->childCount(), "MockModel::insertRows", QString("row out of range, must be < parentNode->childCount() (%1) (invalid row value: %2)").arg(parentNode->childCount()).arg(row).toAscii());

        beginInsertRows(parent, row, row + newRows.count() - 1);
        for (int i=0; i< newRows.count(); ++i)
            parentNode->insertChildAt(i + row, newRows.at(i));
        endInsertRows();

        return true;

    }

    // Removes count rows starting at row from beneath parent
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex())
    {
        MockNode *parentNode = nodeFromIndex(parent);
        if (!parentNode)
            return false;

        Q_ASSERT_X(parentNode->childCount() > 0, "MockModel::removeRows",
                   "parentNode does not have any children to remove");

        Q_ASSERT_X(row >= 0, "MockModel::removeRows",
                   QString("row out of range, must be >= 0 (invalid row value: %1)").arg(row).toAscii());

        Q_ASSERT_X(row < parentNode->childCount(), "MockModel::removeRows",
                   QString("row out of range, must be < parentNode->childCount (%1) (invalid row value: %2)").arg(parentNode->childCount()).arg(row).toAscii());

        Q_ASSERT_X(count >= 0, "MockModel::removeRows",
                   QString("count must be at least 0 (invalid count value: %1)").arg(count).toAscii());

        Q_ASSERT_X(row + count <= parentNode->childCount(), "MockModel::removeRows",
                   QString("row (%1) + count (%2) must be <= parentNode.childCount (%3)").arg(row).arg(count).arg(parentNode->childCount()).toAscii());

        // In debug, count will always be greater than or equal to 0; however, in release mode when the asserts are disabled,
        // do nothing if count is negative
        if (count < 0)
            return false;

        // In debug mode, parentNode->childCount() will always be a positive number or throw an assertion; however, in release mode
        //   with asserts turned off, it is possible to call this function with a parent that has no children. Ignore these requests
        if (parentNode->childCount() == 0)
            return false;

        if (count)
        {
            beginRemoveRows(parent, row, row + count - 1);
            for (int i=row, z= row + count; i<z; ++i)
            {
                MockNode *node = dynamic_cast<MockNode *>(parentNode->childAt(row));
                Q_ASSERT_X(node, "MockModel::removeRows",
                           QString("Null pointer obtained for childAt %1").arg(row).toAscii());

                parentNode->removeChildAt(row);
            }
            endRemoveRows();
        }

        return true;
    }

    /* Moving rows is not implemented because it can be achieved by a remove followed by an insert; however, this skeletal code
       remains should I change my mind and want to do it in the future.
    bool moveRows(const QModelIndex &sourceParent, int sourceStart, int sourceEnd, const QModelIndex &destinationParent, int destinationRow)
    {
        Q_ASSERT(sourceStart >= 0);
        Q_ASSERT(sourceEnd >= sourceStart);
        Q_ASSERT(sourceEnd < rowCount(sourceParent));
        Q_ASSERT(destinationRow >= 0);
        Q_ASSERT(destinationRow <= rowCount(destinationParent));
        Q_ASSERT(sourceParent.model() == destinationParent.model());

        // Check source
        MockNode *sourceParentNode = nodeFromIndex(sourceParent);
        if (!sourceParentNode)
            return false;

        // Check destination
        MockNode *destinationParentNode = nodeFromIndex(destinationParent);
        if (!destinationParentNode)
            return false;

        // Check for no change cases, return true?
        if (sourceParent == destinationParent && destinationRow >= sourceStart && destinationRow <= sourceEnd)
            return true;

        return true;
    }
    */

    MockNode *nodeFromIndex(const QModelIndex &index) const
    {
        if (index.isValid())
            return static_cast<MockNode *>(index.internalPointer());
        else
            return root_;
    }

    // Only returns one column for now
    int columnCount(const QModelIndex & /* parent */) const
    {
        return 1;
    }

    int rowCount(const QModelIndex &parent = QModelIndex()) const
    {
        MockNode *parentNode = nodeFromIndex(parent);
        if (parentNode)
            return parentNode->childCount();

        return 0;
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const
    {
        MockNode *parentNode = nodeFromIndex(parent);
        if (!parentNode)
            return QModelIndex();

        return createIndex(row, column, parentNode->childAt(row));
    }

    QModelIndex parent(const QModelIndex &index) const
    {
        TreeNode *childNode = nodeFromIndex(index);
        if (!childNode)
            return QModelIndex();

        TreeNode *parentNode = childNode->parent();
        if (!parentNode || parentNode == root_)
            return QModelIndex();

        if (!parentNode->parent())
            return QModelIndex();

        return createIndex(parentNode->row(), 0, parentNode);
    }

    // Depending on the data type, can return multiple columns as needed
    QVariant data(const QModelIndex &index, int role) const
    {
        if (role != Qt::DisplayRole)
            return QVariant();

        MockNode *node = nodeFromIndex(index);
        if (!node)
            return QVariant();

        switch (index.column())
        {
        case 0:
            return node->id_;
        case 1:
            return node->name_;
        default:
            return QVariant();
        }
    }

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole)
    {
        if (index.isValid() && (index.column() == 0 || index.column() == 1) && role == Qt::EditRole)
        {
            MockNode *node = nodeFromIndex(index);
            switch (index.column())
            {
            case 0:
                node->id_ = value.toInt();
                break;
            case 1:
                node->name_ = value.toString();
                break;
            default:
                return false;
            }

            emit dataChanged(index, index);
            return true;
        }

        return false;
    }

    // Dummy function to test that the reset signal get sent properly
    void doReset()
    {
        beginResetModel();
        endResetModel();
    }

private:
    MockNode *root_;
};

#endif
