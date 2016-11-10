/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtCore/QStringList>
#include <QtCore/QTimer>
#include <QtGui/QUndoStack>

#include "AbstractMultiEntityTableModel.h"

#include "AdocTreeModel.h"
#include "ColumnAdapters/IColumnAdapter.h"
#include "../Commands/RemoveAdocTreeNodesCommand.h"
#include "../../core/Repositories/IRepository.h"
#include "../../core/util/QVariantLessGreaterThan.h"
#include "../../core/constants.h"
#include "../../core/macros.h"
#include "../../core/misc.h"
#include "../gui_misc.h"

#include <QtDebug>



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Comparison methods for sorting purposes
bool groupsLessThan(const AdocTreeNode *groupA, const AdocTreeNode *groupB);
bool groupsGreaterThan(const AdocTreeNode *groupA, const AdocTreeNode *groupB);
class MultiEntityLessThanPrivate
{
public:
    MultiEntityLessThanPrivate(AbstractMultiEntityTableModel *model, int column)
        : model_(model), column_(column)
    {
    }

    bool operator()(const IEntitySPtr &a, const IEntitySPtr &b) const
    {
        return QVariantLessThan(model_->dataForEntity(a, column_), model_->dataForEntity(b, column_));
    }

private:
    AbstractMultiEntityTableModel *model_;
    int column_;
};

class MultiEntityGreaterThanPrivate
{
public:
    MultiEntityGreaterThanPrivate(AbstractMultiEntityTableModel *model, int column)
        : model_(model), column_(column)
    {
    }

    bool operator()(const IEntitySPtr &a, const IEntitySPtr &b) const
    {
        return QVariantLessThan(model_->dataForEntity(b, column_), model_->dataForEntity(a, column_));
    }

private:
    AbstractMultiEntityTableModel *model_;
    int column_;
};


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant AbstractMultiEntityTableModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() == false)
        return QVariant();

    ASSERT(index.column() < columnCount());

    if (!isGroupIndex(index))
        return dataForEntity(entityFromIndex(index), index.column(), role);

    if (index.column() == groupLabelColumn())
        return adocTreeModel_->data(mapToTree(index), role);

    return QVariant();
}

/**
  * @param entity [const IEntitySPtr &]
  * @param column [int]
  * @param role [int]
  * @returns QVariant
  */
QVariant AbstractMultiEntityTableModel::dataForEntity(const IEntitySPtr &entity, int column, int role) const
{
    ASSERT(entity);
    ASSERT(column >= 0 && column < columnCount());

    if (!entityTypeAdapterHash_.contains(entity->type()))
        return QVariant();

    const EntityAdapterSpecification &specification = entityTypeAdapterHash_.value(entity->type());
    ASSERT(specification.columnAdapter_ != nullptr);

    // TODO: ??
    // Let the column adapter deal with what data to return for an invalid column
    //
    // Replace the following few lines with:
    // return specification.columnAdapter_->data(entity, specification.mapToAdapter(index.column()));
    int adapterColumn = specification.mapToAdapter(column);
    if (adapterColumn == constants::kInvalidColumn)
        return QVariant();

    return specification.columnAdapter_->data(entity, adapterColumn, role);
}

/**
  * @returns bool
  */
bool AbstractMultiEntityTableModel::dynamicSort() const
{
    return sortParams_.dynamic_;
}

/**
  * @param index [const QModelIndex &]
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags AbstractMultiEntityTableModel::flags(const QModelIndex &index) const
{
    // An invalid index is passed in whenever layoutAboutToBeChanged and layoutChanged is called
    if (index.isValid() == false)
        return Qt::NoItemFlags;

    if (!isGroupIndex(index))
    {
        const IEntitySPtr &entity = entityFromIndex(index);
        ASSERT(entityTypeAdapterHash_.contains(entity->type()));

        const EntityAdapterSpecification &specification = entityTypeAdapterHash_.value(entity->type());
        ASSERT(specification.columnAdapter_ != nullptr);

        int adapterColumn = specification.mapToAdapter(index.column());
        if (adapterColumn == constants::kInvalidColumn)
            return QAbstractItemModel::flags(index);

        return specification.columnAdapter_->flags(adapterColumn) | Qt::ItemIsDragEnabled;
    }

    // index is a group index
    if (index.column() == groupLabelColumn())
        return adocTreeModel_->flags(mapToTree(index));

    // Permit nothing to be done with the remaining columns
    return Qt::NoItemFlags;
}

/**
  * @param index [const QModelIndex &]
  * @returns bool
  */
bool AbstractMultiEntityTableModel::isGroupIndex(const QModelIndex &index) const
{
    // Enabling pasting to the root index
    if (!index.isValid())
        return true;

    ASSERT(index.model() == this);

    return index.row() < groups_.size();
}

/**
  * @param row [int]
  * @returns QModelIndex
  */
QModelIndex AbstractMultiEntityTableModel::mapToTree(int row) const
{
    ASSERT(row >= 0 && row < rowCount());

    if (row >= groups_.size())
    {
        // More entities by default, so this is the nominal test case
        ASSERT(entityNodeHash_.contains(entityFromRow(row)));
        return adocTreeModel_->indexFromNode(entityNodeHash_.value(entityFromRow(row)));
    }

    // We have a group node
    return adocTreeModel_->indexFromNode(groups_.at(row));
}

/**
  * @param index [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex AbstractMultiEntityTableModel::mapToTree(const QModelIndex &tableIndex) const
{
    if (tableIndex.isValid() == false)
        return rootIndex_;

    ASSERT(tableIndex.model() == this);
    return mapToTree(tableIndex.row());
}

/**
  * @param tableSelection [const QItemSelection &]
  * @returns QItemSelection
  */
QItemSelection AbstractMultiEntityTableModel::mapSelectionToTree(const QItemSelection &tableSelection) const
{
    QItemSelection treeSelection;

    QVector<int> rows = reduceToUniqueRows(tableSelection.indexes());
    foreach (int row, rows)
    {
        QModelIndex treeIndex = mapToTree(row);
        treeSelection.select(treeIndex, treeIndex);
    }

    return treeSelection;
}

/**
  * @param index [const QModelIndex &]
  * @returns AdocTreeNode *
  */
AdocTreeNode *AbstractMultiEntityTableModel::nodeFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        ASSERT(index.model() == this);

        if (isGroupIndex(index))
            return groups_.at(index.row());

        const IEntitySPtr &entity = entityFromIndex(index);
        ASSERT(entityNodeHash_.contains(entity));
        return entityNodeHash_.value(entity);
    }

    return root_;
}

/**
  *
  *
  * @param row [int]
  * @param count [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AbstractMultiEntityTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
#ifdef QT_DEBUG
    ASSERT(parent.isValid() == false);
    ASSERT(row >= 0 && row < rowCount());
    ASSERT(row + count - 1 < rowCount());
#else
    Q_UNUSED(parent);
#endif

    if (count == 0)
        return true;

    QVector<int> treeRows;
    for (int j=0, i=row; j<count; ++j, ++i)
        treeRows << mapToTree(i).row();

    // It is necessary to remove these rows in reverse order otherwise the tree rows for the all but the first removal
    //                    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ <-- IMPORTANT!!
    // will be irrelevant.
    QVector<QPair<int, int> > treeRowRanges = ::convertIntVectorToRanges(treeRows);
    if (undoStack_ != nullptr)
    {
        // Use a macro to make this a single undo/redo command
        QUndoCommand *masterRemoveCommand = new QUndoCommand(QString("Removing %1 rows from table view").arg(treeRows.size()));
        for (int i=treeRowRanges.size()-1; i>= 0; --i)
        {
            new RemoveAdocTreeNodesCommand(adocTreeModel_,
                                           treeRowRanges.at(i).first,
                                           treeRowRanges.at(i).second - treeRowRanges.at(i).first + 1,
                                           root_,
                                           masterRemoveCommand);
        }
        undoStack_->push(masterRemoveCommand);

        // ISSUE: We really don't know if it was successful because it is wrapped in Command objects
        return true;
    }
    else
    {
        bool allRowsRemoved = true;

        // No undo stack is provided, do each one individually
        for (int i=treeRowRanges.size()-1; i>= 0; --i)
        {
            if (!adocTreeModel_->removeRows(treeRowRanges.at(i).first,
                                            treeRowRanges.at(i).second - treeRowRanges.at(i).first + 1,
                                            rootIndex_))
            {
                allRowsRemoved = false;
            }
        }

        return allRowsRemoved;
    }
}

/**
  * @returns AdocTreeNode *
  */
AdocTreeNode *AbstractMultiEntityTableModel::root() const
{
    return root_;
}

/**
  * @returns QModelIndex
  */
QModelIndex AbstractMultiEntityTableModel::rootIndex() const
{
    return rootIndex_;
}

/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int AbstractMultiEntityTableModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() == false)
        return groups_.size() + entities_.size();

    return 0;
}

/**
  * @param entityType [int]
  * @param entityAdapterSpecification [const EntityAdapterSpecification &]
  */
void AbstractMultiEntityTableModel::setAdapterSpecification(int entityType, const EntityAdapterSpecification &entityAdapterSpecification)
{
    ASSERT(entityAdapterSpecification.columnAdapter_ != nullptr);

    if (entityTypeAdapterHash_.contains(entityType))
    {
        IColumnAdapter *oldAdapter = entityTypeAdapterHash_.value(entityType).columnAdapter_;
        disconnect(oldAdapter, SIGNAL(dataChanged(IEntitySPtr,int)), this, SLOT(onEntityDataChanged(IEntitySPtr,int)));
        // No need to remove the old specification since it will be replaced with the new entityAdapterSpecification
    }

    entityTypeAdapterHash_.insert(entityType, entityAdapterSpecification);

    connect(entityAdapterSpecification.columnAdapter_,
            SIGNAL(dataChanged(IEntitySPtr,int)),
            SLOT(onEntityDataChanged(IEntitySPtr,int)));
}

/**
  * @param dynamicSort [bool]
  */
void AbstractMultiEntityTableModel::setDynamicSort(bool dynamicSort)
{
    sortParams_.dynamic_ = dynamicSort;
    if (sortParams_.dynamic_)
        sort();
}

/**
  * @param adocTreeModel [AdocTreeModel *]
  */
void AbstractMultiEntityTableModel::setSourceTreeModel(AdocTreeModel *adocTreeModel)
{
    beginResetModel();

    if (adocTreeModel_)
    {
        disconnect(adocTreeModel_, SIGNAL(modelReset()), this, SLOT(onTreeModelReset()));
        disconnect(adocTreeModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(onTreeModelDataChanged(QModelIndex,QModelIndex)));
        disconnect(adocTreeModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(onTreeModelRowsInserted(QModelIndex,int,int)));
        disconnect(adocTreeModel_, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(onTreeModelRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(adocTreeModel_, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), this, SLOT(onTreeModelRowsMoved(QModelIndex,int,int,QModelIndex,int)));
        disconnect(adocTreeModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(onTreeModelRowsAboutToBeRemoved(QModelIndex,int,int)));
        disconnect(adocTreeModel_, SIGNAL(transientEntityReplaced(QModelIndex,IEntitySPtr,IEntitySPtr)), this, SLOT(onTreeModelTransientEntityReplaced(QModelIndex,IEntitySPtr,IEntitySPtr)));
    }

    adocTreeModel_ = adocTreeModel;
    resetVariables();

    if (adocTreeModel_ != nullptr)
    {
        connect(adocTreeModel_, SIGNAL(modelReset()), SLOT(onTreeModelReset()));
        connect(adocTreeModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(onTreeModelDataChanged(QModelIndex,QModelIndex)));
        connect(adocTreeModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(onTreeModelRowsInserted(QModelIndex,int,int)));
        connect(adocTreeModel_, SIGNAL(rowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)), SLOT(onTreeModelRowsAboutToBeMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(adocTreeModel_, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), SLOT(onTreeModelRowsMoved(QModelIndex,int,int,QModelIndex,int)));
        connect(adocTreeModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), SLOT(onTreeModelRowsAboutToBeRemoved(QModelIndex,int,int)));
        connect(adocTreeModel_, SIGNAL(transientEntityReplaced(QModelIndex,IEntitySPtr,IEntitySPtr)), SLOT(onTreeModelTransientEntityReplaced(QModelIndex,IEntitySPtr,IEntitySPtr)));
    }

    endResetModel();
}

/**
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  * @returns bool
  */
bool AbstractMultiEntityTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role != Qt::EditRole)
        return false;

    if (!isGroupIndex(index))
    {
        const IEntitySPtr &entity = entityFromIndex(index);
        ASSERT(entityTypeAdapterHash_.contains(entity->type()));

        const EntityAdapterSpecification &specification = entityTypeAdapterHash_.value(entity->type());
        ASSERT(specification.columnAdapter_ != nullptr);

        int adapterColumn = specification.mapToAdapter(index.column());
        if (adapterColumn == constants::kInvalidColumn)
            return false;

        return specification.columnAdapter_->setData(entity, adapterColumn, value);
    }

    // Dealing with a group index
    if (index.column() == groupLabelColumn())
    {
        // Map back to the original tree model
        AdocTreeNode *groupNode = groups_.at(index.row());
        return adocTreeModel_->setData(adocTreeModel_->indexFromNode(groupNode), value, role);
    }

    return false;
}

/**
  * @param undoStack [QUndoStack *]
  */
void AbstractMultiEntityTableModel::setUndoStack(QUndoStack *undoStack)
{
    undoStack_ = undoStack;
}

/**
  * Convenience method for calling sort with the stored sort paramters.
  */
void AbstractMultiEntityTableModel::sort()
{
    sort(sortParams_.column_, sortParams_.order_);
}

/**
  * @param column [int]
  * @param order [Qt::SortOrder]
  */
void AbstractMultiEntityTableModel::sort(int column, Qt::SortOrder order)
{
    ASSERT(column >= 0 && column < columnCount());

    emit layoutAboutToBeChanged();

    QVector<PersistentIndexData> persistentIndexData = mapToPersistentIndexData(persistentIndexList());

    // Always sort the groups because their labels might have changed even if the sort column is not the group column
    if (order == Qt::AscendingOrder)
    {
        qStableSort(groups_.begin(), groups_.end(), groupsLessThan);
        qStableSort(entities_.begin(), entities_.end(), MultiEntityLessThanPrivate(this, column));
    }
    else
    {
        qStableSort(groups_.begin(), groups_.end(), groupsGreaterThan);
        qStableSort(entities_.begin(), entities_.end(), MultiEntityGreaterThanPrivate(this, column));
    }

    changePersistentIndexList(persistentIndexList(), mapToModelIndices(persistentIndexData));

    emit layoutChanged();

    sortParams_.column_ = column;
    sortParams_.order_ = order;
}

/**
  * @returns QUndoStack *
  */
QUndoStack *AbstractMultiEntityTableModel::undoStack() const
{
    return undoStack_;
}


/**
  * @param data [const QMimeData *]
  * @param action [Qt::DropAction]
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool AbstractMultiEntityTableModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    // Special case: parent is invalid - should map to the rootIndex of this table model, not the tree root index
    // necessarily
    QModelIndex parentTreeIndex;
    if (parent.isValid())
        parentTreeIndex = mapToTree(parent);
    else
        parentTreeIndex = rootIndex_;
    return adocTreeModel_->dropMimeData(data, action, row, column, parentTreeIndex);
}

/**
  * @param indices [const QModelIndexList &]
  * @returns QMimeData *
  */
QMimeData *AbstractMultiEntityTableModel::mimeData(const QModelIndexList &indices) const
{
    if (indices.isEmpty())
        return nullptr;

    ASSERT(indices.first().model() == this);

    QModelIndexList treeModelIndexList;
    treeModelIndexList.reserve(indices.size());
    QVector<int> rows = reduceToUniqueRows(indices);
    foreach (int row, rows)
        treeModelIndexList << mapToTree(row);

    // For now, assume that the indices are all valid
    AdocTreeMimeData *adocTreeMimeData = new AdocTreeMimeData(treeModelIndexList);
    adocTreeMimeData->setData("application/x-alignshop", QByteArray());
    return adocTreeMimeData;
}

/**
  * @returns QStringList
  */
QStringList AbstractMultiEntityTableModel::mimeTypes() const
{
    return QStringList() << "application/x-alignshop";
}

/**
  * @returns Qt::DropActions
  */
Qt::DropActions AbstractMultiEntityTableModel::supportedDropActions() const
{
    return Qt::MoveAction;
}



// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public slots
/**
  * Stub virtual method
  */
void AbstractMultiEntityTableModel::refreshEntityIds()
{
}

/**
  * If dynamicSort is true, and we are immediately adding rows, wait to sort until after the reset has been completed.
  * Otherwise, an assertion fails when updating the persistent indexes.
  *
  * @param rootIndex [const QModelIndex &]
  * @see processLoadRequest()
  */
void AbstractMultiEntityTableModel::setRoot(const QModelIndex &rootIndex)
{
    ASSERT(adocTreeModel_ != nullptr);
    ASSERT(rootIndex.isValid() == false || rootIndex.model() == adocTreeModel_);

    if (rootIndex.isValid() && rootIndex_ == rootIndex)
        return;

    beginResetModel();
    resetVariables();

    bool sortNow = false;
    root_ = nullptr;
    rootIndex_ = rootIndex;
    if (rootIndex_.isValid())       // Do not currently permit setting the root to the actual tree root node
    {
        AdocTreeNode *newRoot = adocTreeModel_->nodeFromIndex(rootIndex);
        root_ = newRoot;
        ASSERT(root_ != nullptr);

        if (root_->loaded_)
        {
            // This node has been loaded from the data source before, load all the nodes
            findAddNodes(extractAcceptableNodes(newRoot));
            if (sortParams_.dynamic_)
                sortNow = true;
        }
        else
        {
            // Otherwise, batch process the loading so that the UI does not appear to block
            QHash<AdocNodeType, AdocTreeNodeVector> newData = extractAcceptableNodes(newRoot);
            if (newData.isEmpty() == false)
            {
                loadRequestManager_.reset(new LoadRequestManager(newData));
                loadTimer_->start();  // Calls processLoadRequest() repeatedly until there is no more data chunks
                                      // to be loaded
            }
        }
    }

    endResetModel();

    if (sortNow)
        sort();

    emit rootChanged(rootIndex_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param parent [QObject *]
  */
AbstractMultiEntityTableModel::AbstractMultiEntityTableModel(QObject *parent) :
    QAbstractTableModel(parent),
    adocTreeModel_(nullptr),
    root_(nullptr),
    undoStack_(nullptr)
{
    sortParams_.dynamic_ = false;
    sortParams_.column_ = 0;
    sortParams_.order_ = Qt::AscendingOrder;

    loadTimer_ = new QTimer(this);
    loadTimer_->setInterval(0);
    connect(loadTimer_, SIGNAL(timeout()), SLOT(processLoadRequest()));

    loadRequestManager_.reset(new LoadRequestManager());
}

AbstractMultiEntityTableModel::~AbstractMultiEntityTableModel()
{
    // Properly clean-up our mess. This includes dereferencing any leftover entities
    resetVariables();
}

/**
  * @param nodeType [int]
  * @returns bool
  */
bool AbstractMultiEntityTableModel::acceptNodeType(int nodeType) const
{
    return nodeType == eGroupNode || entityTypeAdapterHash_.contains(nodeType);
}

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  */
void AbstractMultiEntityTableModel::onTreeModelReset()
{
    setRoot(QModelIndex());
    if (sortParams_.dynamic_)
        sort();
}

/**
  * @param topLeft [const QModelIndex &]
  * @param bottomRight [const QModelIndex &]
  */
void AbstractMultiEntityTableModel::onTreeModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_UNUSED(bottomRight);

    ASSERT(topLeft.isValid());
    ASSERT(topLeft.model() == adocTreeModel_);
    ASSERT(topLeft == bottomRight);
    ASSERT(adocTreeModel_ != nullptr);

    // Ignore all changes outside of the currently modeled node
    if (topLeft.parent() != rootIndex_)
        return;

    // Usually group node, but entity node if it has been "cut"
    QModelIndex tableIndex = mapFromTree(topLeft);
    if (tableIndex.isValid() == false)
        return;

    emit dataChanged(tableIndex, tableIndex);

    // The only data that can be changed by the tree model is the group labels, so if it is not a group node,
    // then do not bother re-sorting the table.
    if (isGroupIndex(tableIndex) && sortParams_.dynamic_)
        sort();
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void AbstractMultiEntityTableModel::onTreeModelRowsInserted(const QModelIndex &parent, int start, int end)
{
    ASSERT(parent.isValid() == false || parent.model() == adocTreeModel_);
    if (root_ == nullptr || parent != rootIndex_)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(parent);
    QHash<AdocNodeType, AdocTreeNodeVector> acceptedNodes = extractAcceptableNodes(parentNode, start, end);
    if (acceptedNodes.isEmpty())
        return;

    // Direct method for immediately loading all new rows from the root adoc tree model
    // Perform this update automatically if reasonably number of nodes to add
    if (sumAcceptableNodes(acceptedNodes) <= 1000)
    {
        findAddNodes(acceptedNodes);
        if (dynamicSort())
            sort();
    }
    else
    {
        // ISSUE: (minor)
        // Technically at this point, the root_ node is only partly loaded. Although it may claim to be be loaded
#ifdef QT_DEBUG
        if (root_->loaded_)
            qWarning("[Warning] New tree model rows available for AbstractMultiSeqTableModel, yet loaded is true");
#endif

        // Alternative method for performing this via the load request manager
        ASSERT(loadRequestManager_.isNull() == false);
        loadRequestManager_->addBatch(acceptedNodes);
        loadTimer_->start();
    }
}

/**
  * Two cases to deal with:
  * 1) Are rows moving into our space?
  * 2) Are rows being moved out of our space?  <-- Handled in this method
  *
  * This method strictly deals with the second case - rows moving out of our space
  *
  * @param srcParentIndex [const QModelIndex &]
  * @param srcStart [int]
  * @param srcEnd [int]
  * @param dstParentIndex [const QModelIndex &]
  * @param dstRow [int]
  * @see onTreeModelRowsMoved()
  */
void AbstractMultiEntityTableModel::onTreeModelRowsAboutToBeMoved(const QModelIndex &srcParentIndex, int srcStart, int srcEnd, const QModelIndex & /* dstParentIndex */, int /* dstRow */)
{
//    ASSERT(srcParentIndex.isValid() == false || srcParentIndex.model() == adocTreeModel_);
//    ASSERT(dstParentIndex.isValid() == false || dstParentIndex.model() == adocTreeModel_);
//    ASSERT(srcStart >= 0 && srcStart <= srcEnd);

//    AdocTreeNode *srcParentNode = adocTreeModel_->nodeFromIndex(srcParentIndex);
//    AdocTreeNode *dstParentNode = adocTreeModel_->nodeFromIndex(dstParentIndex);
//    ASSERT(srcParentNode != dstParentNode);
//    ASSERT(srcEnd < srcParentNode->childCount());

    // Case 0: We do not have an established root, disregard all signals
    if (root_ == nullptr)
        return;

    // Case 2: Rows moving out of this space
    if (srcParentIndex == rootIndex_)
        onTreeModelRowsAboutToBeRemoved(srcParentIndex, srcStart, srcEnd);
}

/**
  * Two cases to deal with:
  * 1) Are rows moving into our space?         <-- Handled in this method
  * 2) Are rows being moved out of our space?
  *
  * This method strictly deals with the first case - rows moving into our space
  *
  * @param srcParentIndex [const QModelIndex &]
  * @param srcStart [int]
  * @param srcEnd [int]
  * @param dstParentIndex [const QModelIndex &]
  * @param dstRow [int]
  * @see onTreeModelRowsAboutToBeMoved()
  */
void AbstractMultiEntityTableModel::onTreeModelRowsMoved(const QModelIndex & /* srcParentIndex */, int srcStart, int srcEnd, const QModelIndex &dstParentIndex, int dstRow)
{
    // Case 0: We do not have an established root, disregard all signals
    if (root_ == nullptr)
        return;

    // Case 1: Rows moving into to this space
    if (dstParentIndex == rootIndex_)
        onTreeModelRowsInserted(dstParentIndex, dstRow, dstRow + srcEnd - srcStart);
}

/**
  * @param parent [const QModelIndex &]
  * @param start [int]
  * @param end [int]
  */
void AbstractMultiEntityTableModel::onTreeModelRowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    ASSERT(parent.isValid() == false || parent.model() == adocTreeModel_);
    if (root_ == nullptr)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(parent);

    // Case 1: parentNode == root
    if (root_ == parentNode)
    {
        QVector<int> groupRows;
        QVector<int> entityRows;

        for (int i=start; i<= end; ++i)
        {
            AdocTreeNode *child = parentNode->childAt(i);

            // ---------------------------------
            // Handle removal of all group nodes
            if (child->nodeType_ == eGroupNode)
            {
                int groupIndex = groups_.indexOf(child);
                if (groupIndex != -1)
                    groupRows << groupIndex;

                continue;
            }

            // -------------------------------------------------
            // Now check for any entities that should be removed
            QVector<IEntitySPtr>::Iterator it = entities_.begin();
            for (; it != entities_.constEnd(); ++it)
            {
                const IEntitySPtr &entity = *it;
                if (child->entityId() == entity->id() &&
                    child->nodeType_ == entity->type())
                {
                    entityRows << groups_.size() + (it - entities_.begin());

                    ASSERT(entityNodeHash_.contains(entity));
                    entityNodeHash_.remove(entity);
                    break;
                }
            }
        }

        // Optimized to remove from the model in batches rather than calling begin/end per row
        QVector<QPair<int, int> > ranges = ::convertIntVectorToRanges(entityRows);
        for (int i=ranges.size()-1; i>=0; --i)
        {
            beginRemoveRows(QModelIndex(), ranges.at(i).first, ranges.at(i).second);
            entities_.remove(ranges.at(i).first - groups_.size(), ranges.at(i).second - ranges.at(i).first + 1);
            endRemoveRows();
        }

        ranges = ::convertIntVectorToRanges(groupRows);
        for (int i=ranges.size()-1; i>=0; --i)
        {
            beginRemoveRows(QModelIndex(), ranges.at(i).first, ranges.at(i).second);
            groups_.remove(ranges.at(i).first, ranges.at(i).second - ranges.at(i).first + 1);
            endRemoveRows();
        }

        return;
    }

    // Case 2: root is a descendant of one of the rows being removed
    for (int i=start; i<=end; ++i)
    {
        if (root_->isDescendantOf(parentNode->childAt(i)))
        {
            // The whole shabang is being removed
            beginResetModel();
            resetVariables();
            endResetModel();
            return;
        }
    }

    // Case 3: Removal is outside the context of the node currently being modeled - do nothing
}

/**
  * Remove the old entity from the relevant repository.
  *
  * @param index [const QModelIndex &]
  * @param newEntity [IEntitySPtr &]
  * @param oldEntity [IEntitySPtr &]
  */
void AbstractMultiEntityTableModel::onTreeModelTransientEntityReplaced(const QModelIndex &treeIndex, const IEntitySPtr &newEntity, const IEntitySPtr &oldEntity)
{
    // Ignore all changes outside of the currently modeled node
    if (treeIndex.parent() != rootIndex_)
        return;

    ASSERT(oldEntity != nullptr);
    ASSERT(newEntity != nullptr);

    // Is the old entity present?
    int entityIndex = entities_.indexOf(oldEntity);
    if (entityIndex == -1)
        return;

    ASSERT(entityNodeHash_.contains(oldEntity));
    ASSERT(entityNodeHash_.value(oldEntity) == adocTreeModel_->nodeFromIndex(treeIndex));

    // Check that newEntity is a valid entity to display
    int entityModelRow = mapEntityRowToModelRow(entityIndex);
    AdocTreeNode *node = entityNodeHash_.value(oldEntity);
    entityNodeHash_.remove(oldEntity);
    ASSERT(node != nullptr);
    if (!acceptNodeType(node->nodeType_))
    {
        // Remove the row from ourselves and return; do not use the removeRows() method of this class because that maps
        // all nodes to the treemodel and utilizes the undo stack. Remove manually here because they simply should not
        // be shown at this point.
        beginRemoveRows(QModelIndex(), entityModelRow, entityModelRow);
        entities_.remove(entityIndex, 1);
        endRemoveRows();
        return;
    }

    // Increase the reference count for this entity
    ASSERT(entityTypeAdapterHash_.contains(newEntity->type()));
    ASSERT(entityTypeAdapterHash_.value(newEntity->type()).repository_ != nullptr);
    entityTypeAdapterHash_.value(newEntity->type()).repository_->find(newEntity->id());

    // ISSUE: should we sort with the new data?
    // Swap out the entities and refresh this row
    entities_[entityIndex] = newEntity;
    entityNodeHash_[newEntity] = node;
    for (int i=0, z=columnCount(); i<z; ++i)
    {
        QModelIndex cellIndex = index(entityModelRow, i);
        emit dataChanged(cellIndex, cellIndex);
    }
}

/**
  * @param entity [IEntitySPtr &]
  * @param column [int]
  */
void AbstractMultiEntityTableModel::onEntityDataChanged(const IEntitySPtr &entity, int entityColumn)
{
    ASSERT(entity != nullptr);
    ASSERT(entityColumn >= 0);

    int entityRow = entities_.indexOf(entity);
    if (entityRow == -1)
        return;

    ASSERT(entityTypeAdapterHash_.contains(entity->type()));
    const EntityAdapterSpecification &specification = entityTypeAdapterHash_.value(entity->type());
    int modelColumn = specification.mapToModel(entityColumn);
    if (modelColumn != constants::kInvalidColumn)
    {
        QModelIndex changedIndex = QAbstractTableModel::index(groups_.size() + entityRow, modelColumn);
        emit dataChanged(changedIndex, changedIndex);

        // TODO: Only update via the dynamic sort if the actual column changed is the same as the sort column
        if (sortParams_.dynamic_ && mapEntityColumn(entity->type(), entityColumn) == sortParams_.column_)
            sort();
    }
}

/**
  */
void AbstractMultiEntityTableModel::processLoadRequest()
{
//    qDebug() << Q_FUNC_INFO << loadRequestManager_->isDone();

    ASSERT(loadRequestManager_.isNull() == false);
    ASSERT(loadRequestManager_->isDone() == false);

    QVector<LoadRequestChunk> loadRequestChunkVector = loadRequestManager_->nextBatch();
    ASSERT(loadRequestChunkVector.isEmpty() == false);
    foreach (const LoadRequestChunk &loadRequestChunk, loadRequestChunkVector)
    {
        ASSERT(loadRequestChunk.nodeType_ != eUndefinedNode);

        if (loadRequestChunk.nodeType_ != eGroupNode)
            findAddNodes(loadRequestChunk.nodeType_, loadRequestChunk.nodeVector_, loadRequestChunk.start_, loadRequestChunk.end_);
        else
            loadingContainer_.groups_ << loadRequestChunk.nodeVector_;
    }

    if (loadRequestManager_->isDone())
    {
        loadTimer_->stop();
        int nNewGroups = loadingContainer_.groups_.size();
        if (nNewGroups > 0)
        {
            beginInsertRows(QModelIndex(), groups_.size(), groups_.size() + nNewGroups - 1);
            groups_ << loadingContainer_.groups_;
            loadingContainer_.groups_.clear();
            endInsertRows();
        }

        int nNewEntities = loadingContainer_.entities_.size();
        if (nNewEntities > 0)
        {
            beginInsertRows(QModelIndex(), rowCount(), rowCount() + nNewEntities - 1);
            entities_ << loadingContainer_.entities_;
            loadingContainer_.entities_.clear();
            QHash<IEntitySPtr, AdocTreeNode *>::ConstIterator it = loadingContainer_.entityNodeHash_.constBegin();
            for (; it != loadingContainer_.entityNodeHash_.constEnd(); ++it)
                entityNodeHash_.insert(it.key(), it.value());
            loadingContainer_.entityNodeHash_.clear();
            endInsertRows();
        }

        if (dynamicSort() && nNewGroups + nNewEntities > 0)
            sort();

        // Cache that this node has been loaded
        ASSERT(root_ != nullptr);           // Given the signaling nature of this, it might be possible that this is
                                            // null... ISSUE?
        root_->loaded_ = true;

        /*
        if (loadingContainer_.groups_.size() + loadingContainer_.entities_.size() > 0)
        {
            beginInsertRows(QModelIndex(), rowCount(), rowCount() + loadingContainer_.groups_.size() + loadingContainer_.entities_.size() - 1);
            groups_ << loadingContainer_.groups_;
            loadingContainer_.groups_.clear();
            entities_ << loadingContainer_.entities_;
            loadingContainer_.entities_.clear();
            QHash<IEntitySPtr &, AdocTreeNode *>::ConstIterator it = loadingContainer_.entityNodeHash_.constBegin();
            for (; it != loadingContainer_.entityNodeHash_.constEnd(); ++it)
                entityNodeHash_.insert(it.key(), it.value());
            loadingContainer_.entityNodeHash_.clear();
            endInsertRows();

            if (dynamicSort())
                sort();

            // Cache that this node has been loaded
            ASSERT(root_ != nullptr);           // Given the signaling nature of this, it might be possible that this is
                                                // null... ISSUE?
            root_->data_.loaded_ = true;
        }
        */
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param index [const QModelIndex &index]
  * @returns IEntitySPtr
  */
IEntitySPtr AbstractMultiEntityTableModel::entityFromIndex(const QModelIndex &index) const
{
    ASSERT(isGroupIndex(index) == false);
    ASSERT(entities_.size() > 0);
    ASSERT(index.model() == this);
    ASSERT(index.isValid());

    const IEntitySPtr &entity = entities_.at(index.row() - groups_.size());
    ASSERT(entity);

    return entity;
}

/**
  * @param row [int]
  * @returns IEntitySPtr
  */
IEntitySPtr AbstractMultiEntityTableModel::entityFromRow(int row) const
{
    ASSERT(row >= groups_.size());
    ASSERT(row < rowCount());

    const IEntitySPtr &entity = entities_.at(row - groups_.size());
    ASSERT(entity);

    return entity;
}

/**
  * @param parent [const AdocTreeNode *]
  * @returns QHash<AdocNodeType, AdocTreeNodeVector>
  */
QHash<AdocNodeType, AdocTreeNodeVector> AbstractMultiEntityTableModel::extractAcceptableNodes(const AdocTreeNode *parent) const
{
    if (parent->childCount() > 0)
        return extractAcceptableNodes(parent, 0, parent->childCount() - 1);

    return QHash<AdocNodeType, AdocTreeNodeVector>();
}

/**
  * @param parent [const AdocTreeNode *]
  * @param start [int]
  * @param end [int]
  * @returns QHash<AdocNodeType, AdocTreeNodeVector>
  */
QHash<AdocNodeType, AdocTreeNodeVector> AbstractMultiEntityTableModel::extractAcceptableNodes(const AdocTreeNode *parent, int start, int end) const
{
    ASSERT(parent != nullptr);
    ASSERT(start >= 0 && start <= end);
    ASSERT(end < parent->childCount());

    QHash<AdocNodeType, AdocTreeNodeVector> acceptableNodeHash; // {node type -> [node *] }
    for (int i=start; i<=end; ++i)
    {
        AdocTreeNode *child = parent->childAt(i);
        if (acceptNodeType(child->nodeType_))
            acceptableNodeHash[child->nodeType_] << child;
    }
    return acceptableNodeHash;
}

/**
  * Completely finds and adds all the entities referenced in entityNodeHash
  *
  * @param entityNodeHash [const QHash<AdocNodeType, AdocTreeNodeVector> &]
  */
void AbstractMultiEntityTableModel::findAddNodes(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash)
{
    AdocTreeNodeVector newGroups;
    QVector<IEntitySPtr> newEntities;

    QHash<AdocNodeType, AdocTreeNodeVector>::ConstIterator it = entityNodeHash.constBegin();
    for (; it != entityNodeHash.constEnd(); ++it)
    {
        // it.key() = node type
        // it.value() = AdocTreeNodeVector
        if (it.key() == eGroupNode)
        {
            newGroups << it.value();
            continue;
        }

        ASSERT(it.key() != eRootNode);

        // Must be entity node
        IRepository *repository = repositoryForNodeType(it.key());
        ASSERT(repository != nullptr);
        const AdocTreeNodeVector &adocTreeNodeVector = it.value();
        QVector<int> entityIds;
        entityIds.reserve(adocTreeNodeVector.size());
        for (int i=0, z=adocTreeNodeVector.size(); i<z; ++i)
            entityIds << adocTreeNodeVector.at(i)->entityId();

        // ISSUE: If an entry is present in the tree, but not the data source, then a null pointer will be present
        //        here. As soon as it is attempted to be read from (e.g. as requested from a view), the program will
        //        crash.
        QVector<IEntitySPtr> entities = repository->find(entityIds);
        ASSERT(entities.size() == adocTreeNodeVector.size());
        for (int i=0, z=entities.size(); i<z; ++i)
        {
            ASSERT(entities.at(i));
            entityNodeHash_.insert(entities.at(i), adocTreeNodeVector.at(i));
        }
        newEntities << entities;
    }

    if (newGroups.size() > 0)
    {
        beginInsertRows(QModelIndex(), groups_.size(), groups_.size() + newGroups.size() - 1);
        groups_ << newGroups;
        endInsertRows();
    }

    if (newEntities.size() > 0)
    {
        beginInsertRows(QModelIndex(), rowCount(), rowCount() + newEntities.size() - 1);
        entities_ << newEntities;
        endInsertRows();
    }
}

/**
  * Performs a partial request for data from the data source.
  *
  * @param adocNodeType [AdocNodeType]
  * @param adocTreeNodeVector [const AdocTreeNodeVector &]
  * @param start [int]
  * @param end [int]
  */
void AbstractMultiEntityTableModel::findAddNodes(AdocNodeType adocNodeType, const AdocTreeNodeVector &adocTreeNodeVector, int start, int end)
{
    // Groups should be handled outside of this method for performance reasons (no lookup is necessary for them)
    ASSERT(adocNodeType != eRootNode && adocNodeType != eGroupNode);

    // Must be entity node
    IRepository *repository = repositoryForNodeType(adocNodeType);
    ASSERT(repository != nullptr);
    QVector<int> entityIds;
    entityIds.reserve(end - start + 1);
    for (int i=start; i <= end; ++i)
        entityIds << adocTreeNodeVector.at(i)->entityId();

    // ISSUE: If an entry is present in the tree, but not the data source, then a null pointer will be present
    //        here. As soon as it is attempted to be read from (e.g. as requested from a view), the program will
    //        crash. Assertion added.
    //
    // Another possibility: the database structure has been manipulated such that the prepared queries failed. In such
    // a case, the program will continue to here; however, all the entities will be null.
    QVector<IEntitySPtr> newEntities = repository->find(entityIds);
    ASSERT(newEntities.size() == end - start + 1);
    for (int i=0, z=newEntities.size(); i<z; ++i)
    {
        ASSERT(newEntities.at(i) != nullptr);
        loadingContainer_.entityNodeHash_.insert(newEntities.at(i), adocTreeNodeVector.at(start + i));
    }
    loadingContainer_.entities_ << newEntities;
}

/**
  * @param entity [IEntitySPtr &]
  * @param column [int]
  * @returns QModelIndex
  */
QModelIndex AbstractMultiEntityTableModel::indexFromEntity(const IEntitySPtr &entity, int column) const
{
    if (entity != nullptr)
    {
        int entityRow = entities_.indexOf(entity);
        if (entityRow == -1)
            return QModelIndex();

        return index(mapEntityRowToModelRow(entityRow), column);
    }

    return QModelIndex();
}

/**
  * @param groupNode [AdocTreeNode *]
  * @param column [int]
  * @returns QModelIndex
  */
QModelIndex AbstractMultiEntityTableModel::indexFromGroupNode(AdocTreeNode *groupNode, int column) const
{
    if (groupNode != nullptr)
    {
        int groupRow = groups_.indexOf(groupNode);
        if (groupRow == -1)
            return QModelIndex();

        return index(groupRow, column);
    }

    return QModelIndex();
}

/**
  * @param entityType [int]
  * @param entityColumn [int]
  * @returns int
  */
int AbstractMultiEntityTableModel::mapEntityColumn(int entityType, int entityColumn) const
{
    if (!entityTypeAdapterHash_.contains(entityType))
        return constants::kInvalidColumn;

    return entityTypeAdapterHash_.value(entityType).mapToModel(entityColumn);
}

/**
  * @param entityRow [int]
  * @returns int
  */
int AbstractMultiEntityTableModel::mapEntityRowToModelRow(int entityRow) const
{
    return groups_.size() + entityRow;
}

/**
  * @param treeIndex [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex AbstractMultiEntityTableModel::mapFromTree(const QModelIndex &treeIndex) const
{
    if (treeIndex.isValid() == false)
        return QModelIndex();

    ASSERT(treeIndex.model() == adocTreeModel_);
    AdocTreeNode *node = adocTreeModel_->nodeFromIndex(treeIndex);
    ASSERT(node != nullptr);
    if (node->nodeType_ == eGroupNode)
    {
        // Check if the group is present here
        int i = groups_.indexOf(node);
        if (i != -1)
            return index(i, groupLabelColumn());
    }
    else
    {
        // Check if this is entity is present in our list
        for (int i=0, z=entities_.size(); i<z; ++i)
        {
            if (entities_.at(i)->id() == node->entityId() &&
                entities_.at(i)->type() == node->nodeType_)
            {
                return index(groups_.size() + i, groupLabelColumn());
            }
        }
    }

    return QModelIndex();
}

/**
  * Produces an equivalent vector of PersistentIndexData from modelIndexList.
  *
  * @param modelIndexList [const QModelIndexList &]
  * @returns QVector<AbstractMultiEntityTableModel::PersistentIndexData>
  * @see mapToModelIndices(), rowDataPointer(), sort()
  */
QVector<AbstractMultiEntityTableModel::PersistentIndexData>
AbstractMultiEntityTableModel::mapToPersistentIndexData(const QModelIndexList &modelIndexList) const
{
    QVector<PersistentIndexData> dataVector;
    dataVector.reserve(modelIndexList.size());
    foreach (const QModelIndex &index, modelIndexList)
    {
        if (!isGroupIndex(index))
            dataVector << PersistentIndexData(index.column(), entityFromIndex(index));
        else
            dataVector << PersistentIndexData(index.column(), nodeFromIndex(index));
    }

    return dataVector;
}

/**
  * Presumably the data has been rearranged somehow (e.g. via sort) and needs to be remapped back to the new indices
  * from the persistent index data.
  *
  * @param persistentIndexData [const QVector<PersistentIndexData> &]
  * @returns QModelIndexList
  * @see mapToPeristentIndexData()
  */
QModelIndexList
AbstractMultiEntityTableModel::mapToModelIndices(const QVector<PersistentIndexData> &persistentIndexData) const
{
    QModelIndexList indices;
    indices.reserve(persistentIndexData.size());
    for (int i=0, z=persistentIndexData.size(); i<z; ++i)
    {
        const PersistentIndexData &pid = persistentIndexData.at(i);
        ASSERT_X(pid.entity() || pid.node() != nullptr, "Null pointer not allowed in persistent index data");
        if (pid.isEntity_)
            indices << indexFromEntity(pid.entity(), pid.column_);
        else
            indices << indexFromGroupNode(pid.node(), pid.column_);
    }

    return indices;
}

/**
  * @param nodeType [AdocNodeType]
  * @returns IRepository *
  */
IRepository *AbstractMultiEntityTableModel::repositoryForNodeType(AdocNodeType nodeType) const
{
    if (!entityTypeAdapterHash_.contains(nodeType))
        return nullptr;

    ASSERT(entityTypeAdapterHash_.value(nodeType).repository_ != nullptr);

    return entityTypeAdapterHash_.value(nodeType).repository_;
}

/**
  */
void AbstractMultiEntityTableModel::resetVariables()
{
    groups_.clear();
    entities_.clear();
    entityNodeHash_.clear();
    root_ = nullptr;
    rootIndex_ = QModelIndex();

    loadTimer_->stop();
    if (loadRequestManager_.isNull() == false)
        loadRequestManager_->clear();
    loadingContainer_.clear();
}

/**
  * @param entityNodeHash [const QHash<AdocNodeType, AdocTreeNodeVector> &]
  * @returns int
  */
int AbstractMultiEntityTableModel::sumAcceptableNodes(const QHash<AdocNodeType, AdocTreeNodeVector> &entityNodeHash) const
{
    int sum = 0;
    QHash<AdocNodeType, AdocTreeNodeVector>::ConstIterator it = entityNodeHash.constBegin();
    for (; it != entityNodeHash.constEnd(); ++it)
        sum += it.value().size();

    return sum;
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructor for EntityAdapterSpecification
/**
  * @param nModelColumns [int]
  * @param repository [IRepository *]
  * @param columnAdapter [IColumnAdapter *]
  */
AbstractMultiEntityTableModel::EntityAdapterSpecification::EntityAdapterSpecification(int nModelColumns, IRepository *repository, IColumnAdapter *columnAdapter)
    : repository_(repository), columnAdapter_(columnAdapter), modelColumnToAdapterColumn_(nModelColumns, constants::kInvalidColumn)
{
    if (nModelColumns == 0)
        qWarning("[Warning] EntityAdapterSpecification created with zero model columns; mapping will not work as expected\n");
}

/**
  * @param modelColumn [int]
  * @param adapterColumn [int]
  */
void AbstractMultiEntityTableModel::EntityAdapterSpecification::setMapping(int modelColumn, int adapterColumn)
{
    ASSERT(modelColumn >= 0 && modelColumn < modelColumnToAdapterColumn_.size());
    modelColumnToAdapterColumn_[modelColumn] = adapterColumn;
    adapterColumnToModelColumn_.insert(adapterColumn, modelColumn);
}

/**
  * @param adapterColumn [int]
  */
int AbstractMultiEntityTableModel::EntityAdapterSpecification::mapToModel(int adapterColumn) const
{
    if (adapterColumnToModelColumn_.contains(adapterColumn))
        return adapterColumnToModelColumn_.value(adapterColumn);

    return constants::kInvalidColumn;
}

/**
  * @param modelColumn [int]
  */
int AbstractMultiEntityTableModel::EntityAdapterSpecification::mapToAdapter(int modelColumn) const
{
    ASSERT(modelColumn >= 0 && modelColumn < modelColumnToAdapterColumn_.size());

    return modelColumnToAdapterColumn_.at(modelColumn);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Comparison methods for sorting purposes
/**
  * @param groupA [const AdocTreeNode *]
  * @param groupB [const AdocTreeNode *]
  * @returns bool
  */
bool groupsLessThan(const AdocTreeNode *groupA, const AdocTreeNode *groupB)
{
    ASSERT(groupA != nullptr && groupB != nullptr);
    ASSERT(groupA->nodeType_ == eGroupNode && groupB->nodeType_ == eGroupNode);

    return groupA->label_ < groupB->label_;
}

/**
  * @param groupA [const AdocTreeNode *]
  * @param groupB [const AdocTreeNode *]
  * @returns bool
  */
bool groupsGreaterThan(const AdocTreeNode *groupA, const AdocTreeNode *groupB)
{
    ASSERT(groupA != nullptr && groupB != nullptr);
    ASSERT(groupA->nodeType_ == eGroupNode && groupB->nodeType_ == eGroupNode);

    return groupsLessThan(groupB, groupA);
}
