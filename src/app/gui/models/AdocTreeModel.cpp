/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QColor>
#include <QtGui/QIcon>
#include <QtGui/QUndoStack>

#include "AdocTreeModel.h"
#include "CustomRoles.h"

#include "../Commands/InsertAdocTreeNodesCommand.h"
#include "../Commands/MoveAdocTreeNodesCommand.h"
#include "../Commands/RemoveAdocTreeNodesCommand.h"
#include "../Commands/SetGroupLabelCommand.h"

#include "../util/ModelIndexRange.h"

#include "../../core/global.h"
#include "../../core/macros.h"

#include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
AdocTreeModel::AdocTreeModel(QObject *parent)
    : AbstractBaseTreeModel<AdocTreeNode>(parent), undoStack_(nullptr)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int AdocTreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

/**
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant AdocTreeModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        ASSERT(index.column() == 0);

        AdocTreeNode *node = nodeFromIndex(index);
        if (node == nullptr)
            return QVariant();

        switch (role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            return node->label_;
        case Qt::DecorationRole:
            if (node->nodeType_ == eGroupNode)
                return QIcon(":/aliases/images/icons/folder");
            break;
        case CustomRoles::kIsCutRole:
            return cutOrCopyIndices_.contains(index);

        default:
            return QVariant();
        }
    }

    return QVariant();
}

/**
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags AdocTreeModel::flags(const QModelIndex &index) const
{
    AdocTreeNode *node = nodeFromIndex(index);
    if (node == nullptr)
        return Qt::NoItemFlags;
    // Apparently, modeltest indicates that the only flag options available for the root node are:
    // nothing or Qt::ItemIsDropEnabled
    else if (node->nodeType_ == eRootNode)
        return Qt::ItemIsDropEnabled;

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
    if (node->nodeType_ == eGroupNode)
        flags |= Qt::ItemIsEditable | Qt::ItemIsDropEnabled;

    return flags;
}

/**
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  * @returns bool
  */
bool AdocTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    AdocTreeNode *node = nodeFromIndex(index);
    if (node == nullptr)
        return false;

    if (node->nodeType_ != eGroupNode)
        return false;

    if (undoStack_ != nullptr)
    {
        undoStack_->push(new SetGroupLabelCommand(this, node, value));
        return true;
    }

    return setDataPrivate(index, value);
}

/**
  * @param data [const QMimeData *]
  * @param action [Qt::DropAction]
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int /* row */, int /* column */, const QModelIndex &parent)
{
    if (action == Qt::IgnoreAction)
        return true;

    if (action != Qt::MoveAction)
        return false;

    const AdocTreeMimeData *adocTreeMimeData = static_cast<const AdocTreeMimeData *>(data);
    if (!adocTreeMimeData->hasFormat("application/x-alignshop"))
        return false;

    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr)
        return false;

    ASSERT(parentNode->nodeType_ == eGroupNode || parentNode->nodeType_ == eRootNode);

    moveRows(adocTreeMimeData->indices(), parentNode);

    // If we return true, then the model will call removeRows - we don't want that!
    return false;
}

/**
  * @param indices [const QModelIndexList &]
  * @returns QMimeData *
  */
QMimeData *AdocTreeModel::mimeData(const QModelIndexList &indices) const
{
    if (indices.isEmpty())
        return nullptr;

    // For now, assume that the indices are all valid
    AdocTreeMimeData *adocTreeMimeData = new AdocTreeMimeData(indices);
    adocTreeMimeData->setData("application/x-alignshop", QByteArray());
    return adocTreeMimeData;
}

/**
  * @returns QStringList
  */
QStringList AdocTreeModel::mimeTypes() const
{
    return QStringList() << "application/x-alignshop";
}

/**
  * @returns Qt::DropActions
  */
Qt::DropActions AdocTreeModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

/**
  * @returns AdocTreeNode *
  */
AdocTreeNode *AdocTreeModel::root() const
{
    return root_;
}

/**
  * @param root [AdocTreeNode *]
  */
void AdocTreeModel::setRoot(AdocTreeNode *root)
{
    beginResetModel();
    root_ = root;
    cutOrCopyIndices_.clear();
    endResetModel();
}

/**
  * @param node [AdocTreeNode *]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeModel::appendRow(AdocTreeNode *node, const QModelIndex &parent)
{
    ASSERT(node != nullptr);

    static AdocTreeNodeVector vector(1);
    vector[0] = node;
    return appendRows(vector, parent);
}

/**
  * @param nodes [const AdocTreeNodeVector &]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeModel::appendRows(const AdocTreeNodeVector &nodes, const QModelIndex &parent)
{
    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr)
        return false;

    if (nodes.size() > 0)
    {
        if (undoStack_ != nullptr)
            undoStack_->push(new InsertAdocTreeNodesCommand(this, nodes, parentNode));
        else    // Append them in the normal manner
            addRows(nodes, parentNode);
    }

    return true;
}

/**
  * All indicies must have the same parent
  *
  * @param indices [const QModelIndexList &]
  */
void AdocTreeModel::cutRows(const QModelIndexList &indices)
{
    clearCutCopyRows();

    if (indices.isEmpty())
        return;

    ASSERT(indices.first().model() == this);

#ifdef QT_DEBUG
    // Ensure that they each have the same parent and all have column zero
    foreach (const QModelIndex &index, indices)
    {
        ASSERT(index.parent() == indices.first().parent());
        ASSERT(index.column() == 0);
    }
#endif

    cutOrCopyIndices_.resize(indices.size());
    for (int i=0, z=indices.size(); i<z; ++i)
    {
        cutOrCopyIndices_[i] = indices.at(i);
        emit dataChanged(indices.at(i), indices.at(i));
    }
}

bool AdocTreeModel::hasCutRows() const
{
    return cutOrCopyIndices_.size() > 0;
}

/**
  * All move row commands should be issued using this interface rather than constructing new MoveAdocTreeNodesCommand
  * instances elsewhere.
  *
  * @param srcRow [int]
  * @param count [int]
  * @param srcParent [const QModelIndex &]
  * @param dstParent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeModel::moveRows(int srcRow, int count, const QModelIndex &srcParent, const QModelIndex &dstParent)
{
    ASSERT(srcRow >= 0);
    if (srcParent == dstParent)
        return true;

    if (count == 0)
        return true;

    AdocTreeNode *srcParentNode = nodeFromIndex(srcParent);
    AdocTreeNode *dstParentNode = nodeFromIndex(dstParent);
    if (srcParentNode == nullptr || dstParentNode == nullptr)
        return false;

    if (dstParentNode->nodeType_ != eRootNode && dstParentNode->nodeType_ != eGroupNode)
        return false;

    if (dstParentNode->isEqualOrDescendantOfAny(srcParentNode, srcRow, count))
        return false;

    ASSERT(srcRow >= 0 && srcRow < srcParentNode->childCount());
    ASSERT(srcRow + count <= srcParentNode->childCount());

    if (undoStack_ != nullptr)
    {
        undoStack_->push(new MoveAdocTreeNodesCommand(this, srcRow, count, srcParentNode, dstParentNode));
        return true;
    }

    moveRows(srcRow, count, srcParentNode, dstParentNode);
    return true;
}

/**
  * @param groupName [const QString &]
  * @param parent [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex AdocTreeModel::newGroup(const QString &groupName, const QModelIndex &parent)
{
#ifdef QT_DEBUG
    AdocTreeNode *parentNode = nodeFromIndex(parent);
    ASSERT(parentNode != nullptr);
    // TODO: Wrap this in a more extensible fashion
    ASSERT(parentNode->nodeType_ == eRootNode || parentNode->nodeType_ == eGroupNode);
#endif

    AdocTreeNode *groupNode = new AdocTreeNode(eGroupNode, groupName);
    if (appendRow(groupNode, parent))
        return index(rowCount(parent) - 1, 0, parent);

    // Otherwise, failed to append the group to the model
    delete groupNode;
    groupNode = nullptr;

    return QModelIndex();
}

/**
  * We can assume that cutOrCopyIndices_ all have the same parent
  */
void AdocTreeModel::paste(const QModelIndex &index)
{
    ASSERT(index.isValid() == false || index.model() == this);
    if (cutOrCopyIndices_.isEmpty())
        return;

    AdocTreeNode *dstParentNode = nodeFromIndex(index);
    if (dstParentNode == nullptr)
        return;
    ASSERT(dstParentNode->nodeType_ == eRootNode || dstParentNode->nodeType_ == eGroupNode);

    // Purge invalid indices from the cutOrCopyIndices list before issuing the move command
    // Rationale: because cutOrCopyIndices_ consists of QPersistentModelIndex'es it is possible for them to become
    // invalid depending on the user's action. One specific scenario where this might happen:
    // o User imports some sequences
    // o User cuts some of these newly imported sequences
    // o User undoes the import
    // Result: previously cut indices are now no longer valid
    QVector<QPersistentModelIndex>::Iterator it = cutOrCopyIndices_.begin();
    while (it != cutOrCopyIndices_.end())
    {
        if ((*it).isValid())
            ++it;
        else
            it = cutOrCopyIndices_.erase(it);
    }

    if (moveRows(cutOrCopyIndices_, dstParentNode))
        cutOrCopyIndices_.clear();
}

/**
  * Currently requires that all indices have the same parent; however, I do not think this is necessary.
  *
  * @param indices [const QModelIndexList &]
  * @returns bool
  */
bool AdocTreeModel::removeRows(const QModelIndexList &indices)
{
    if (indices.isEmpty())
        return true;

#ifdef QT_DEBUG
    // Ensure that they each have the same parent and all have column zero
    foreach (const QModelIndex &index, indices)
    {
        ASSERT(index.model() == this);
        ASSERT_X(index.parent() == indices.first().parent(), "All indices in list must have the same parent");
        ASSERT(index.column() == 0);
    }
#endif

    QVector<ModelIndexRange> indexRanges = ::convertIndicesToRanges<QModelIndexList>(indices);
    if (indexRanges.size() == 1)
        return removeRows(indexRanges.at(0).start_, indexRanges.at(0).count_, indexRanges.at(0).parentIndex_);

    // We are dealing with multiple index ranges - we want to group these into a single undo command
    AdocTreeNode *srcParentNode = nodeFromIndex(indexRanges.first().parentIndex_);
    ASSERT(srcParentNode != nullptr);
    if (undoStack_ != nullptr)
    {
        // We deal with the undo stack here (rather than simply calling removeRows) for grouping multiple index ranges
        // into a single undo command.
        QUndoCommand *masterCommand = new QUndoCommand(QString("Removing %1 row(s)").arg(indices.size()));

        // IMPORTANT!! It is vital to remove these in reverse order otherwise, all but the first remove command will
        //                            ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^  !!
        // reference the correct nodes.
        for (int i=indexRanges.size()-1; i>= 0; --i)
        {
            const ModelIndexRange &range = indexRanges.at(i);
            new RemoveAdocTreeNodesCommand(this, range.start_, range.count_, srcParentNode, masterCommand);
        }
        undoStack_->push(masterCommand);

        // ISSUE?
        return true;
    }

    // Else, there is no undo stack, perform each of these remove commands in sequence
    bool success = true;
    foreach (const ModelIndexRange &range, indexRanges)
    {
        success = removeRows(range.start_, range.count_, range.parentIndex_);
        if (!success)
            break;
    }
    return success;
}

/**
  * @param row [int]
  * @param count [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (undoStack_ != nullptr)
    {
        ASSERT(count > 0);

        AdocTreeNode *parentNode = nodeFromIndex(parent);
        if (parentNode == nullptr)
            return false;

        undoStack_->push(new RemoveAdocTreeNodesCommand(this, row, count, parentNode));
        return true;    // ISSUE ??
    }

    return AbstractBaseTreeModel<AdocTreeNode>::removeRows(row, count, parent);
}

/**
  * @param undoStack [QUndoStack *]
  */
void AdocTreeModel::setUndoStack(QUndoStack *undoStack)
{
    undoStack_ = undoStack;
}

/**
  * @returns QUndoStack *
  */
QUndoStack *AdocTreeModel::undoStack() const
{
    return undoStack_;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
void AdocTreeModel::clearCutCopyRows()
{
    QVector<QPersistentModelIndex> temp = cutOrCopyIndices_;
    cutOrCopyIndices_.clear();
    foreach (const QModelIndex &index, temp)
        emit dataChanged(index, index);
}

/**
  * @param index [const QModelIndex &]
  * @param entity [IEntitySPtr &]
  * @returns bool
  */
bool AdocTreeModel::replaceTransientEntity(const QModelIndex &index, IEntitySPtr &entity)
{
    AdocTreeNode *node = nodeFromIndex(index);
    if (node == nullptr)
        return false;

    if (node->nodeType_ != eTransientTaskNode)
        return false;

    if (entity == nullptr)
        return false;

    const IEntitySPtr &oldEntity = node->entity();
    if (oldEntity == entity)
        return true;

    // At this point, the only reference to entity is ourselves; it needs to be placed into a repository somehow. This
    // is handled by the AdocTreeNodeEraserSerivce.
    node->setEntity(entity);
    emit transientEntityReplaced(index, entity, oldEntity);

    return true;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @returns bool
  */
bool AdocTreeModel::setDataPrivate(const QModelIndex &index, const QVariant &value)
{
    AdocTreeNode *node = nodeFromIndex(index);
    if (node == nullptr)
        return false;

    // Only allow editing of the labels for Group nodes
    if (node->nodeType_ != eGroupNode)
        return false;

    QString valueAsString = value.toString();
    if (node->label_ != valueAsString)
    {
        node->label_ = valueAsString;
        emit dataChanged(index, index);
    }

    return true;
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * Returns the integer position of the location where the nodes were inserted.
  *
  * @param nodes [const AdocTreeNodeVector &]
  * @param parentNode [AdocTreeNode *]]
  * @returns int
  */
int AdocTreeModel::addRows(const AdocTreeNodeVector &nodes, AdocTreeNode *parentNode)
{
    ASSERT(parentNode != nullptr);
    ASSERT(nodes.isEmpty() == false);

    emit nodesAboutToBeAdded(nodes);

    // No check is done to verify that these nodes are not already present in the tree
    int row = parentNode->childCount();

    beginInsertRows(indexFromNode(parentNode), row, row + nodes.size() - 1);
    parentNode->appendChildren(nodes);
    endInsertRows();

    return row;
}

/**
  * @param srcRow [int]
  * @param count [int]
  * @param srcParentNode [AdocTreeNode *]
  * @param dstParentNode [AdocTreeNode *]
  * @returns int
  */
int AdocTreeModel::moveRows(int srcRow, int count, AdocTreeNode *srcParentNode, AdocTreeNode *dstParentNode)
{
    ASSERT(srcRow >= 0 && srcRow + count - 1 < srcParentNode->childCount());

    int dstRow = dstParentNode->childCount();
    beginMoveRows(indexFromNode(srcParentNode), srcRow, srcRow + count - 1, indexFromNode(dstParentNode), dstRow);
    AdocTreeNodeVector nodes = srcParentNode->takeChildren(srcRow, count);
    dstParentNode->appendChildren(nodes);
    endMoveRows();

    return dstRow;
}

/**
  * @param modelIndexList [const QModelIndexList &]
  * @param dstParentNode [AdocTreeNode *]
  * @returns bool
  */
bool AdocTreeModel::moveRows(const QModelIndexList &modelIndexList, AdocTreeNode *dstParentNode)
{
    ASSERT(dstParentNode != nullptr);
    if (modelIndexList.isEmpty())
        return true;

    // 1) Check the descendancy status and 2) if the drop target is the root node, make sure all indices are
    //    are groups (e.g. no entities allowed under root node)
    foreach (const QModelIndex &index, modelIndexList)
    {
        AdocTreeNode *node = nodeFromIndex(index);
        if (dstParentNode == node || dstParentNode->isDescendantOf(node))
            return false;

        if (dstParentNode->nodeType_ == eRootNode && nodeFromIndex(index)->nodeType_ != eGroupNode)
            return false;
    }

    QVector<ModelIndexRange> indexRanges = ::convertIndicesToRanges<QModelIndexList>(modelIndexList);
    AdocTreeNode *srcParentNode = nodeFromIndex(indexRanges.first().parentIndex_);
    ASSERT(srcParentNode != nullptr);
    if (srcParentNode == dstParentNode)
        return false;

    if (undoStack_ != nullptr)
    {
        // We deal with the undo stack here (rather than simply calling moveRows) for grouping multiple index ranges
        // into a single undo command.
        QUndoCommand *masterCommand = new QUndoCommand("Moving rows");

        // IMPORTANT!! It is vital to move these in reverse order otherwise, the indices will no longer work properly
        //                            ^^^^^^^^^^^^^^^^^^^^^^^^^^^  !!
        for (int i=indexRanges.size()-1; i>= 0; --i)
        {
            const ModelIndexRange &range = indexRanges.at(i);
            new MoveAdocTreeNodesCommand(this, range.start_, range.count_, srcParentNode, dstParentNode, masterCommand);
        }
        undoStack_->push(masterCommand);
    }
    else
    {
        foreach (const ModelIndexRange &range, indexRanges)
            moveRows(range.start_, range.count_, srcParentNode, dstParentNode);
    }

    return true;
}

/**
  * @param modelIndexList [const QVector<QPersistentModelIndex> &]
  * @param dstParentNode [AdocTreeNode *]
  * @returns bool
  */
bool AdocTreeModel::moveRows(const QVector<QPersistentModelIndex> &persistentModelIndexList, AdocTreeNode *dstParentNode)
{
    QModelIndexList modelIndexList;
    modelIndexList.reserve(persistentModelIndexList.size());
    foreach (const QPersistentModelIndex &index, persistentModelIndexList)
        modelIndexList << index;

    return moveRows(modelIndexList, dstParentNode);
}

/**
  * @param row [int]
  * @param count [int]
  * @param parent [const QModelIndex &]
  * @returns AdocTreeNodeVector
  * @see removeRows()
  */
AdocTreeNodeVector AdocTreeModel::takeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0)
        return AdocTreeNodeVector();

    AdocTreeNode *parentNode = nodeFromIndex(parent);
    if (parentNode == nullptr)
        return AdocTreeNodeVector();

    ASSERT(row >= 0 && row < parentNode->childCount());
    ASSERT(row + count <= parentNode->childCount());

    beginRemoveRows(parent, row, row + count - 1);
    AdocTreeNodeVector removedNodes = parentNode->takeChildren(row, count);
    endRemoveRows();

    return removedNodes;
}
