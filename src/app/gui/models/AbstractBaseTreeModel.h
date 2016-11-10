/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#ifndef ABSTRACTBASETREEMODEL_H
#define ABSTRACTBASETREEMODEL_H

#include <QtCore/QAbstractItemModel>
#include "../../core/global.h"
#include "../../core/macros.h"

#include <QtDebug>

/**
  * AbstractBaseTreeModel provides a basic implementation of some of the key methods required for modeling tree
  * structures composed of TreeNode.
  *
  * Specifically, this includes the following:
  * -- Reimplemented public methods
  *    o index
  *    o parent
  *    o removeRows
  *    o rowCount
  *
  * -- Helper public methods
  *    o indexFromNode
  *    o nodeFromIndex
  *
  * While the root_ data member is defined and initialized (to null) in this class, allocation and deallocation is the
  * responsibility of subclasses.
  */
template<typename TreeNodeT>
class AbstractBaseTreeModel : public QAbstractItemModel
{
public:
    // ------------------------------------------------------------------------------------------------
    // Constructor
    AbstractBaseTreeModel(QObject *parent = nullptr);


    // ------------------------------------------------------------------------------------------------
    // Reimplmented public methods
    virtual QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex &child) const;
    virtual bool removeRows(int row, int count, const QModelIndex &parent);
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;


    // ------------------------------------------------------------------------------------------------
    // Helper methods
    QModelIndex indexFromNode(TreeNodeT *treeNode, int column = 0) const;
    QModelIndex indexFromNode(const TreeNodeT *treeNode, int column = 0) const;
    TreeNodeT *nodeFromIndex(const QModelIndex &index) const;


protected:
    TreeNodeT *root_;
};


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param parent [QObject *]
  */
template<typename TreeNodeT>
inline
AbstractBaseTreeModel<TreeNodeT>::AbstractBaseTreeModel(QObject *parent)
    : QAbstractItemModel(parent),
      root_(nullptr)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns QModelIndex
  */
template<typename TreeNodeT>
inline
QModelIndex AbstractBaseTreeModel<TreeNodeT>::index(int row, int column, const QModelIndex &parent) const
{
    TreeNodeT *parentTreeNode = nodeFromIndex(parent);
    if (parentTreeNode == nullptr)
        return QModelIndex();

    // Check for valid row and column values
    if (row < 0 ||
        row >= parentTreeNode->childCount() ||
        column < 0 ||
        column >= columnCount(parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, parentTreeNode->childAt(row));
}

/**
  * @param child [const QModelIndex &]
  * @returns QModelIndex
  */
template<typename TreeNodeT>
inline
QModelIndex AbstractBaseTreeModel<TreeNodeT>::parent(const QModelIndex &child) const
{
    TreeNodeT *childNode = nodeFromIndex(child);
    if (childNode == nullptr)
        return QModelIndex();

    TreeNodeT *parentNode = childNode->parent();
    if (parentNode == root_ || parentNode == nullptr)
        return QModelIndex();

    if (parentNode->parent() == nullptr)
        return QModelIndex();

    return createIndex(parentNode->row(), 0, parentNode);
}

/**
  * @param row [int]
  * @param count [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
template<typename TreeNodeT>
inline
bool AbstractBaseTreeModel<TreeNodeT>::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count < 0)
        return false;

    TreeNodeT *parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr)
        return false;

    ASSERT(row >= 0 && row < parentNode->childCount());
    ASSERT(row + count <= parentNode->childCount());

    if (count == 0)
        return true;

    beginRemoveRows(parent, row, row + count - 1);
    parentNode->removeChildren(row, count);
    endRemoveRows();

    return true;
}

/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
template<typename TreeNodeT>
inline
int AbstractBaseTreeModel<TreeNodeT>::rowCount(const QModelIndex &parent) const
{
    TreeNodeT *parentNode = nodeFromIndex(parent);
    if (parentNode != nullptr)
        return parentNode->childCount();

    return 0;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Helper methods
/**
  * @param treeNode [TreeNode<TreeNodeT> *]
  * @param column [int]
  * @returns QModelIndex
  */
template<typename TreeNodeT>
inline
QModelIndex AbstractBaseTreeModel<TreeNodeT>::indexFromNode(TreeNodeT *treeNode, int column) const
{
    // Good debug check, although invalid index is returned in release mode
    ASSERT(treeNode == nullptr || treeNode == root_ || treeNode->isDescendantOf(root_));

    if (treeNode == nullptr || treeNode == root_ || !treeNode->isDescendantOf(root_))
        return QModelIndex();

    if (column != 0)
    {
        // Check the column
        QModelIndex parentIndex = createIndex(treeNode->parent()->row(), 0, treeNode->parent());
        if (column < 0 || column >= columnCount(parentIndex))
            return QModelIndex();
    }

    return createIndex(treeNode->row(), column, treeNode);
}

/**
  * @param treeNode [const TreeNode<TreeNodeT> *]
  * @param column [int]
  * @returns QModelIndex
  */
template<typename TreeNodeT>
inline
QModelIndex AbstractBaseTreeModel<TreeNodeT>::indexFromNode(const TreeNodeT *treeNode, int column) const
{
    return indexFromNode(const_cast<TreeNodeT *>(treeNode), column);
}

/**
  * @param index [const QModelIndex &]
  * @returns AdocTreeNode *
  */
template<typename TreeNodeT>
inline
TreeNodeT *AbstractBaseTreeModel<TreeNodeT>::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        if (index.model() != this)
        {
            qDebug() << Q_FUNC_INFO << "Wrong index passed to model";
            return nullptr;
        }

        return static_cast<TreeNodeT *>(index.internalPointer());
    }

    return root_;
}

#endif // ABSTRACTBASETREEMODEL_H
