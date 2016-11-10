/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SliceModel.h"

#include <QtCore/QStringList>

#include "models/AdocTreeModel.h"
#include "AdocTreeNode.h"
#include "DataRow.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * Sets the reference count to 0.
  */
Slice::Slice() : refCount_(0)
{
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
SliceModel::SliceModel(QObject *parent) : QAbstractItemModel(parent)
{
    adocTreeModel_ = 0;
}

/**
  */
SliceModel::~SliceModel()
{
    clear();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * Utilizes the pure virtual methods, primaryColumn() and isEditableColumn(), along with index checking
  * to determine the appropriate flags for the given index. A zero result is returned for all top level
  * indices. If the index is a group node for the primaryColumn, then simply return the flags determined
  * by the AdocTreeModel class. Otherwise, if it is not a group item, enable it and if isEditableColumn,
  * add the editable flag.
  *
  * @param index [const QModelIndex &]
  * @see primaryColumn(), isEditableColumn()
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags SliceModel::flags(const QModelIndex &index) const
{
    // getParentSlice(...) ensures that index is valid and belongs to this model.
    // Thus, if slice is valid, then we can trust that the index row and column reference
    // a valid slice index.
    Slice *slice = getParentSlice(index);
    if (!slice)
        return 0;

    AdocTreeNode *node = slice->sourceNodes_.at(index.row());
    if (index.column() == primaryColumn())
        return adocTreeModel_->indexFromNode(node).flags();

    // This condition occurs when there are artificial indices created for a column (e.g. associated with a group
    // row or msa row) that is not the primary column.
    if (node->nodeType_ == AdocTreeNode::GroupType
        || node->nodeType_ == AdocTreeNode::MsaAminoType)
        return 0;

    // By default, all items are enabled
    Qt::ItemFlags flags = Qt::ItemIsEnabled;

    // But not all columns may be edited
    if (isEditableColumn(index.column()))
        flags |= Qt::ItemIsEditable;

    return flags;
}

/**
  * Excluding the tree root, there are only two levels of items that can have valid indices: the top
  * level AdocTreeNode pointers, and the second-level slice items. No valid indices will be returned
  * if a valid tree source model has not yet been defined. By definition, top-level items may only
  * have one column and slice items may have sliceColumnCount() columns (see columnCount()).
  *
  * Case 1: top level items (mappedNodes_)
  *         >> parent is invalid
  *         >> row must be between 0 and mappedNodes_.count()
  *
  * Case 2: slice items (sliceHash_)
  *         >> parent is valid
  *         >> row must be between 0 and slice->rows_.count()
  *
  * Slice item indices store the parent tree node pointer in their internalPointer. This provides a
  * mechanism for returning the proper QModelIndex from the parent function.
  *
  * Additionally, even though some slice rows may only have one column of data, due to the nature of
  * the Qt Model/View framework, it is necessary to create indices for all columns. Other model methods
  * are responsible for handling requests for these indices as needed (e.g. flags, data, setData).
  *
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns QModelIndex
  * @see columnCount()
  */
QModelIndex SliceModel::index(int row, int column, const QModelIndex &parent) const
{
    // Must have a source tree model
    if (!adocTreeModel_)
        return QModelIndex();

    // Must have a minimum row and valid column number
    if (row < 0 || column < 0 || column >= columnCount(parent))
        return QModelIndex();

    // -----------------------
    // Case 1: Top level items (mappedNodes)
    if (!parent.isValid())
    {
        if (row >= mappedNodes_.count())
            return QModelIndex();

        // Because the parent of a top-level item is the root, there is no need to save its
        // pointer in the QModelIndex internalPointer variable
        return createIndex(row, column);
    }

    // Parent is a valid QModelIndex, check that it originated from this model
    if (parent.model() != this)
    {
        qWarning("SliceModel::index - parent index does not belong to this model");
        return QModelIndex();
    }

    // -------------------
    // Case 2: Slice items
    Q_ASSERT_X(!parent.parent().isValid(), "SliceModel::index", "parent's parent must be an invalid index");
    if (parent.row() >= mappedNodes_.count())
        return QModelIndex();
    AdocTreeNode *parentNode = mappedNodes_.at(parent.row());
    Q_ASSERT_X(parentNode, "SliceModel::index", "mapped parent node must not be null");

    // Determine if the row is within a valid range. Could call rowCount(parent); however, that would
    // incur additional validation. Simpler to directly look at the slice rows itself
    Q_ASSERT_X(sliceHash_.contains(parentNode), "SliceModel::index", "sliceHash_ must contain the parentNode");
    Slice *slice = sliceHash_[parentNode];
    if (row >= slice->rows_.count())
        return QModelIndex();

    return createIndex(row, column, parentNode);
}

/**
  * Because SliceModel contains a simple, two-tiered tree structure, there are two possible cases that require
  * the parent model index: top-level items and slice items. Top-level items by definition are children of the
  * root index (QModelIndex()). Slice items will have the parent tree node stored in their internalPointer which
  * can be used to map to its top-level index. This operation should have taken place before this method is
  * called. All other indices should return QModelIndex().
  *
  * @param child [const QModelIndex &]
  * @returns QModelIndex
  * @see index()
  */
QModelIndex SliceModel::parent(const QModelIndex &child) const
{
    if (child.isValid() && child.model() != this)
    {
        qWarning("SliceModel::parent - child index does not belong to this model");
        return QModelIndex();
    }

    // Child is slice item if it has a non-zero internal pointer
    if (child.internalPointer())
    {
        AdocTreeNode *parentNode = static_cast<AdocTreeNode *>(child.internalPointer());
        Q_ASSERT_X(parentNode, "SliceModel::parent", "parentNode stored in internalPointer must not be null");
        int row = mappedNodes_.indexOf(parentNode);
        Q_ASSERT_X(row != -1, "SliceModel::parent", QString("Unable to find parentNode %1 in mappedNodes_").arg(parentNode->label_).toAscii());
        if (row != -1)
            // The column value is zero because this is a top-level item
            return createIndex(row, 0);
    }

    // child is either a top level item or invalid
    return QModelIndex();
}

/**
  * This method provides for removing slice item rows and slice item rows only. Removing top-level items
  * corresponding to mapped source tree nodes is accomplished via the releaseSlice method.
  *
  * While functional in its own right, this is primarily to support drag and drop operations from the
  * various QAbstractItemView's that may be attached to this model.
  *
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @see releaseSlice()
  */
bool SliceModel::removeRows(int row, int count, const QModelIndex &parent)
{
    // Do not permit removal of rows from the root
    if (!parent.isValid())
        return false;

    // Double check that this is a top-level item
    if (parent.parent().isValid())
        return false;

    // Check that parent is associated with this model
    if (parent.model() != this)
    {
        qWarning("SliceModel::removeRows- parent index does not belong to this model");
        return false;
    }

    // Check that parent is in a valid range
    Q_ASSERT_X(parent.row() < mappedNodes_.count(), "SliceModel::removeRows", "parent row is out of range");
    if (parent.row() >= mappedNodes_.count())    // Release mode guard
        return false;

    AdocTreeNode *parentNode = mappedNodes_.at(parent.row());
    if (!sliceHash_.contains(parentNode))
        return false;

    Slice *slice = sliceHash_[parentNode];
    Q_ASSERT_X(row >= 0 && row < parentNode->childCount(), "SliceModel::removeRows", "row out of range");
    Q_ASSERT_X(count >= 0, "SliceModel::removeRows", "count parameter must be positive");
    Q_ASSERT_X(row + count <= parentNode->childCount(), "SliceModel::removeRows", "row + count must be less than number of children");
    if (row < 0 || count < 0 || row + count > parentNode->childCount())
        return false;

    if (count == 0)
        return true;

    QModelIndex sourceParentIndex = adocTreeModel_->indexFromNode(parentNode);

    // Removal of nodes is carried one by one beginning with the largest source tree row position and
    // ending with the smallest source tree row index. Because the order within sourceRowIndices_ does
    // not necessarily follow the source tree order, it is required to first sort them in ascending order.
    // If nodes that occurred earlier in the tree (smaller position) were removed before those that occur
    // later (larger position value), the tree would be corrupted.
    QList<int> orderedPositions = slice->sourceRowIndices_.mid(row, count);
    qStableSort(orderedPositions);

    // OPTIMIZATION: remove contiguous blocks when possible rather than removing one at a time
    for (int i=orderedPositions.count() - 1; i >= 0; --i)
        adocTreeModel_->removeRows(orderedPositions.at(i), 1, sourceParentIndex);

    return true;
}

/**
  * Similar to the other reimplemented methods, the rowCount must be supplied for the root and top-level parent
  * indices. All other cases will return zero. Because this is a standard tree structure, only indices in column
  * zero may have a non-zero value.
  *
  * @param parent [const QModelIndex &]
  * @returns int
  */
int SliceModel::rowCount(const QModelIndex &parent) const
{
    // -----------------------
    // Case 1: Top level item: parent is the root
    if (!parent.isValid())
        return mappedNodes_.count();

    // -----------------------
    // Model compatibility check
    if (parent.model() != this)
    {
        qWarning("SliceModel::rowCount - parent index does not belong to this model");
        return 0;
    }

    // -----------------------
    // Case 2: slice items

    // Only indices in the first column may have a non-zero rowCount
    if (parent.column() != 0)
        return 0;

    // Only consider top-level items because all second-tier rows are not allowed to have any children
    if (parent.parent().isValid())
        return 0;

    // parent must be within range
    if (parent.row() >= mappedNodes_.count())
        return 0;

    AdocTreeNode *parentNode = mappedNodes_.at(parent.row());
    Q_ASSERT_X(parentNode, "SliceModel::rowCount", "mapped parent node must not be null");
    Q_ASSERT_X(sliceHash_.contains(parentNode), "SliceModel::rowCount", "sliceHash_ must contain the parentNode");
    if (!sliceHash_.contains(parentNode))        // Release mode guard
        return 0;

    return sliceHash_[parentNode]->rows_.count();
}


// ---------------------
// ---------------------
// Drag and drop methods
/**
  * By definition, a SliceModel may only accept mime data if the column is the primaryColumn().
  * Otherwise, the source tree model result with the corresponding parameters is returned.
  *
  * @param data [const QMimeData *]
  * @param action [Qt::DropAction]
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  * @see primaryColumn()
  */
bool SliceModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::dropMimeData", "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return false;

    // Only consider drops onto parent items that are in the primary column
    if (parent.column() != primaryColumn())
        return false;

    Slice *slice = getParentSlice(parent);
    if (!slice)
        return false;

    QModelIndex sourceTreeParent = adocTreeModel_->indexFromNode(slice->sourceNodes_.at(parent.row()));
    return adocTreeModel_->dropMimeData(data, action, row, column, sourceTreeParent);
}

/**
  * Because this is a public function, it is possible that indexes contains indices that are not
  * "correct" - they may not belong to this model, may reference invalid columns, etc. Returns 0
  * if an error or invalid state is encountered when mapping any index to its source tree equivalent.
  * In other words, all indexes must be valid to expect a non-zero result.
  *
  * Indexes with differing parents are allowed.
  *
  * @param indexes [const QModelIndexList &]
  * @returns QMimeData *
  * @see dropMimeData()
  */
QMimeData *SliceModel::mimeData(const QModelIndexList &indexes) const
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::mimeData", "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return 0;

    // Transform indexes into the source tree model indexes
    QModelIndexList sourceIndices;
    foreach (const QModelIndex &index, indexes)
    {
        Slice *slice = getParentSlice(index);
        if (!slice)
        {
            qWarning("SliceModel::mimeData - invalid index (row: %d, column %d, data: %s", index.row(), index.column(), index.data().toString().toAscii().constData());
            return 0;
        }

        if (index.column() != primaryColumn())
        {
            qWarning("SliceModel::mimeData - index column (column: %d, data: %s) does not correspond to the primary column; skipping", index.column(), index.data().toString().toAscii().constData());
            return 0;
        }

        sourceIndices << adocTreeModel_->indexFromNode(slice->sourceNodes_.at(index.row()));
    }

    return adocTreeModel_->mimeData(sourceIndices);
}

/**
  * @returns QStringList
  */
QStringList SliceModel::mimeTypes() const
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::mimeTypes", "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return QStringList();

    return adocTreeModel_->mimeTypes();
}

/**
  * @returns Qt::DropActions
  */
Qt::DropActions SliceModel::supportedDragActions() const
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::supportedDragActions", "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return 0;

    return adocTreeModel_->supportedDragActions();
}

/**
  * @returns Qt::DropActions
  */
Qt::DropActions SliceModel::supportedDropActions() const
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::supportedDropActions", "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return 0;

    return adocTreeModel_->supportedDropActions();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns AdocTreeModel *
  * @see setTreeModel()
  */
AdocTreeModel *SliceModel::adocTreeModel() const
{
    return adocTreeModel_;
}

/**
  * Public method for clearing all loaded slice data and issuing a model reset.
  *
  * @see ~SliceModel(), clearStructures()
  */
void SliceModel::clear()
{
    beginResetModel();
    clearStructures();
    endResetModel();
}

/**
  * @returns int
  */
int SliceModel::defaultSortColumn() const
{
    return 0;
}

/**
  * Parent must have been previously mapped via loadSlice() or an invalid QModelIndex will be returned.
  *
  * @param parent [AdocTreeNode *]
  * @returns QModelIndex
  * @see loadSlice()
  */
QModelIndex SliceModel::indexFromNode(AdocTreeNode *parent) const
{
    Q_ASSERT_X(parent, "SliceModel::indexFromNode", "parent must not be null");
    if (!parent)    // Release mode guard
        return QModelIndex();

    int row = mappedNodes_.indexOf(parent);
    if (row == -1)
        return QModelIndex();

    return createIndex(row, 0);
}

/**
  * @param index [const QModelIndex &]
  * @returns bool
  */
bool SliceModel::isGroupItem(const QModelIndex &index) const
{
    Slice *slice = getParentSlice(index);
    return slice && slice->sourceNodes_.at(index.row())->nodeType_ == AdocTreeNode::GroupType;
}

/**
  * If not already loaded, loads the 2D slice associated with parent via the virtual delegate createSlice.
  * The slice reference count is incremented each time this method is called with parent.
  *
  * Can create a slice for any node including the root node.
  *
  * @param sourceParentIndex [const QModelIndex &]
  * @see createSlice(), releaseSlice()
  */
void SliceModel::loadSlice(const QModelIndex &sourceParentIndex)
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::loadSlice", "Missing valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    if (sourceParentIndex.isValid()
        && adocTreeModel_ != sourceParentIndex.model())    // Model compatibility check
    {
        qWarning("SliceModel::loadSlice - sourceParentIndex does not belong to the associated adocTreeModel_");
        return;
    }

    AdocTreeNode *parent = adocTreeModel_->nodeFromIndex(sourceParentIndex);
    Q_ASSERT_X(parent, "SliceModel::loadSlice", "parent pointer must not be null");
    if (!parent)    // Release mode guard
        return;

    if (!sliceHash_.contains(parent))
    {
        beginInsertRows(QModelIndex(), mappedNodes_.count(), mappedNodes_.count());
        Slice *newSlice = createSlice(parent);
        sliceHash_.insert(parent, newSlice);

        // Update the nodeHash for the reverse mapping
        nodeHash_.insert(newSlice, parent);

        // Insert in the top level, the actual order does not matter
        mappedNodes_.append(parent);
        endInsertRows();

        // -----------------------------------------------------
        // Call the virtual "signal" for inserting the new slice
        sliceInserted(newSlice);
    }

    ++sliceHash_[parent]->refCount_;
}

/**
  * Two cases because of the two-tiered structure of the SliceModel:
  * 1) Top-level items
  * 2) Slice rows
  *
  * The mechanism involves retrieving the corresponding AdocTreeNode pointer and then using
  * the source tree model's, indexFromNode() method to fetch the appropriate source tree index.
  *
  * @param proxyIndex [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex SliceModel::mapToSource(const QModelIndex &proxyIndex) const
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::mapToSource", "Missing valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return QModelIndex();

    if (!proxyIndex.isValid())
        return QModelIndex();

    if (proxyIndex.model() != this)    // Model compatibility check
    {
        qWarning("SliceModel::mapToSource - proxyIndex does not belong to this model");
        return QModelIndex();
    }

    // Safe to assume if the above conditions pass, that proxyIndex is a validly created index
    // because the only mechanism for creating indices for this model is the index function. And
    // the index function constrains this process to only validly referencable indices.
    //
    // What about stale indices that the user has stored?
    // -> Should only have to check the upper row-count boundary for such indices
    if (proxyIndex.internalPointer())
    {
        Slice *slice = getParentSlice(proxyIndex);
        if (!slice)
            return QModelIndex();

        Q_ASSERT_X(proxyIndex.row() < slice->rows_.count(), "SliceModel::mapToSource", "proxyIndex out of range");
        if (proxyIndex.row() >= slice->rows_.count())
            return QModelIndex();

        return adocTreeModel_->indexFromNode(slice->sourceNodes_.at(proxyIndex.row()));
    }

    Q_ASSERT_X(proxyIndex.row() < rowCount(), "SliceModel::mapToSource", "proxyIndex out of range");
    if (proxyIndex.row() >= rowCount())
        return QModelIndex();

    return adocTreeModel_->indexFromNode(mappedNodes_.at(proxyIndex.row()));
}

/**
  * Concurrent with the need to produce 2D slices, it is also useful to provide a mechanism for
  * releasing slice memory when it is no longer needed. releaseSlice() decrements the reference
  * count for a slice associated with a valid parent AdocTreeNode. If the reference count
  * reaches zero, the slice memory is released.
  *
  * Technically, there are two data structures to update:
  * 1. The mappedNodes data structure containing the list of top-level items
  * 2. The sliceHash_ which contains the actual slice associated with the given top-level item
  *
  * @param sourceParentIndex [const QModelIndex &]
  * @see loadSlice(), createSlice()
  */
void SliceModel::releaseSlice(const QModelIndex &sourceParentIndex)
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::releaseSlice", "Missing valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    if (sourceParentIndex.isValid()
        && adocTreeModel_ != sourceParentIndex.model())    // Model compatibility check
    {
        qWarning("SliceModel::releaseSlice - sourceParentIndex does not belong to the associated adocTreeModel_");
        return;
    }

    AdocTreeNode *parent = adocTreeModel_->nodeFromIndex(sourceParentIndex);
    Q_ASSERT_X(parent, "SliceModel::releaseSlice", "parent pointer must not be null");
    if (!parent)    // release mode guard
        return;

    if (!sliceHash_.contains(parent))
        return;

    Slice *slice = sliceHash_[parent];
    --slice->refCount_;
    if (slice->refCount_ < 0)
        qWarning("SliceModel::releaseSlice - slice reference count dropped below zero");

    if (slice->refCount_ <= 0)      // Reference count should never drop below zero, but check for it for precaution
    {
        // The slice is no longer needed, remove from the mapped nodes and sliceHash
        int row = mappedNodes_.indexOf(parent);
        Q_ASSERT_X(row != -1, "SliceModel::releaseSlice", "parent pointer must be present within the mapped nodes list");

        // -----------------------------------------------------
        // Call the virtual "signal" for the slice about to be removed
        sliceAboutToBeRemoved(slice);

        beginRemoveRows(QModelIndex(), row, row);
        mappedNodes_.removeAt(row);

        // Remove from the nodeHash
        nodeHash_.remove(slice);

        delete slice;
        slice = 0;
        sliceHash_.remove(parent);
        endRemoveRows();
    }
}

/**
  * If another source model has been configured, disconnect any attached signals, set the new source model,
  * and hook up the appropriate signal handlers.
  *
  * 5 possible scenarios:
  *    Current model     Provided model  Result
  * 1) Null              Null            No change
  * 2) Null              Good            Reset
  * 3) Model1            Model1          No change
  * 4) Model2            Model1          Reset
  * 5) Good              Null            Reset
  *
  * Not certain that the layoutAboutToBeChanged/layoutChanged signals need or should be propagated.
  *
  * The modelAboutToBeReset signal is not mapped because we emti this signal as needed when the source
  * model has finished resetting (see sourceModelReset()).
  *
  * @param adocTreeModel [AdocTreeModel *]
  */
void SliceModel::setTreeModel(AdocTreeModel *adocTreeModel)
{
    if (adocTreeModel_ == adocTreeModel)
        return;

    // If an existing adoc tree model is already configured, disconnect its old model signals
    if (adocTreeModel_)
    {
        QObject::disconnect(adocTreeModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
        QObject::disconnect(adocTreeModel_, SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
        QObject::disconnect(adocTreeModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        QObject::disconnect(adocTreeModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        QObject::disconnect(adocTreeModel_, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

        // Signals
        QObject::disconnect(adocTreeModel_, SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged()));
        QObject::disconnect(adocTreeModel_, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
    }

    beginResetModel();
    clearStructures();

    // Update the source adoc tree model
    adocTreeModel_ = adocTreeModel;

    // If adocTreeModel_ is valid, attach to the relevant signals
    if (adocTreeModel_)
    {
        QObject::connect(adocTreeModel_, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
        QObject::connect(adocTreeModel_, SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
        QObject::connect(adocTreeModel_, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        QObject::connect(adocTreeModel_, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        QObject::connect(adocTreeModel_, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

        // Signals
        QObject::connect(adocTreeModel_, SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged()));
        QObject::connect(adocTreeModel_, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
    }

    endResetModel();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * This is a convenience function that simply calls createSlice with all child indices if the parent
  * contains at least one child. Otherwise, it returns an empty slice.
  *
  * @param parent [AdocTreeNode *]
  * @returns Slice *
  * @see createSlice(AdocTreeNode *, int, int)
  */
Slice *SliceModel::createSlice(AdocTreeNode *parent) const
{
    if (parent->childCount() == 0)
        return new Slice;

    return createSlice(parent, 0, parent->childCount()-1);
}

/**
  * Utility function that returns the parent Slice containing index. Valid slices will only be returned
  * for second-level items (actual slice items). All other cases will return 0. Moreover, zero is also
  * returned if the parent slice does not contain index.
  *
  * @param index [const QModelIndex &]
  * @returns Slice *
  */
Slice *SliceModel::getParentSlice(const QModelIndex &index) const
{
    // Slices may not be associated with the root node
    if (!index.isValid())
        return 0;

    if (index.model() != this)
    {
        qWarning("SliceModel::getParentSlice - wrong index passed to this model");
        return 0;
    }

    // The parent of top-level items is the root node which does not have an associated slice
    QModelIndex parent = index.parent();
    if (!parent.isValid())
        return 0;

    // Only provide data for the second tier items
    Q_ASSERT_X(parent.row() < mappedNodes_.count(), "SliceModel::getParentSlice", "parent row reference valid mappedNodes");
    if (parent.row() >= mappedNodes_.count())    // Release mode guard
        return 0;
    AdocTreeNode *node = mappedNodes_.at(parent.row());

    Q_ASSERT_X(sliceHash_.contains(node), "SliceModel::getParentSlice", "parent tree node must exist within the slice hash");
    if (!sliceHash_.contains(node)) // Release mode guard
        return 0;

    Slice *slice = sliceHash_[node];

    // Check that the index row is a valid entry within slice
    if (index.row() >= slice->rows_.count())
        return 0;

    return slice;
}

/**
  * Because slices are purely associated with a top-level item, a valid QModelIndex will be
  * returned whenever the given slice is present.
  *
  * @param slice [Slice *]
  * @returns QModelIndex
  */
QModelIndex SliceModel::indexFromSlice(Slice *slice) const
{
    if (nodeHash_.contains(slice))
        return indexFromNode(nodeHash_[slice]);

    return QModelIndex();
}

// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Private signals
/**
  * Because AdocTreeModel contains only a single column, only consider dataChanged signals where
  * the topLeft and bottomRight columns are zero. While in most SliceModels, the top level items
  * will not be displayed, emit dataChanged for these elements as well.
  *
  * ASSUME: topLeft.parent() == bottomRight.parent()
  *
  * @param topLeft [const QModelIndex &]
  * @param bottomRight [const QModelIndex &]
  */
void SliceModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::sourceDataChanged", "Received sourceDataChanged signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    if (topLeft.column() != 0)
        return;

    // Check if the top level item has been changed
    for (int i=topLeft.row(), z=bottomRight.row(); i<=z; ++i)
    {
        AdocTreeNode *node = adocTreeModel_->nodeFromIndex(topLeft.sibling(i, 0));
        if (!node)
            continue;

        if (sliceHash_.contains(node))
        {
            QModelIndex index = indexFromNode(node);
            emit dataChanged(index, index);
        }
    }

    // Now handle any slice items
    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(topLeft.parent());
    if (!sliceHash_.contains(parentNode))
        return;

    Slice *slice = sliceHash_[parentNode];
    QModelIndex parentIndex = indexFromNode(parentNode);
    Q_ASSERT_X(parentIndex.isValid(), "SliceModel::sourceDataChanged", "parentIndex must not be invalid");

    int x = topLeft.row();
    int y = bottomRight.row();
    int n = y - x + 1;

    // Find all nodes that have changed in our model
    // OPTIMIZE: Utilize a sorted list!
    for (int i=0, z= slice->rows_.count(); i<z; ++i)
    {
        if (slice->sourceRowIndices_[i] >= x && slice->sourceRowIndices_[i] <= y)
        {
            QModelIndex childIndex = parentIndex.child(i, 0);
            emit dataChanged(childIndex, childIndex);

            --n;
            if (n == 0)
                break;
        }
    }
}

/**
  * Clean up and re-transmit the model reset to this model.
  */
void SliceModel::sourceModelReset()
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::sourceModelReset", "Received sourceModelReset signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    clear();
}

/**
  * When new rows are inserted into the source tree model, it is necessary to find those rows that
  * adhere with the subclass filtering and append these to the appropriate slice. Before this can
  * happen though, the source parent node of the rows being inserted must have been previously loaded.
  * New rows to be inserted into this model are generated in the same fashion as during the loadSlice
  * method. The slice creation process is delegated to the subclass via the virtual createSlice method;
  * however, here, createSlice is called with the start and end arguments of the parent rows to consider.
  * Thus, a slice is created only with the newly added items that match the subclasses filter.
  *
  * The new slice items are simply appended to the original slice for this mapped node - ordering is
  * not taken into consideration. If a specific order is desired, a sort mechanism this should be
  * implemented.
  *
  * Conditions:
  * 1) adocTreeModel_ must be defined
  * 1) sourceParent.column must be zero because AdocTreeModel by definition does not have other columns
  * 2) corresponding tree node must be non-null and previously mapped
  *
  * Ordering does not strictly matter at this level, because this model is not functioning as a sort model.
  * That responsibility would be layered on via a QSortFilterProxyModel derivative or redefine the sort method for this
  * class.
  *
  * All pre-existing mapped slice items must have their parent row indicies updated depending on if these
  * rows are inserted before their parent items in the source tree model. Because of this, there will be
  * instances where a slice need its sourceRowIndices_ updated yet without emitting any signals because no
  * additional rows were mapped.
  *
  * @param sourceParent [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  */
void SliceModel::sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::sourceRowsInserted", "Received sourceRowsInserted signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    // Column must be zero unless the sourceParent is the root node (as indicated by an invalid QModelIndex)
    if (sourceParent.isValid() && sourceParent.column() != 0)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(sourceParent);
    if (!parentNode)
        return;

    if (!sliceHash_.contains(parentNode))
        return;

    Slice *currentSlice = sliceHash_[parentNode];

    // Update the sourceRowIndices_ that occur after start by the number of rows being inserted (end - start + 1)
    for (int i=0, z=currentSlice->rows_.count(); i<z; ++i)
        if (currentSlice->sourceRowIndices_.at(i) >= start)
            currentSlice->sourceRowIndices_[i] += (end - start + 1);

    // Call subclass method to return the slice in this range
    Slice *newSlice = createSlice(parentNode, start, end);

    // Only propagate this response if new rows are to be inserted
    if (newSlice->rows_.count())
    {
        QModelIndex parentIndex = indexFromNode(parentNode);

        // Insert these rows at the end of the slice - ordering does not matter
        int insert_start = currentSlice->rows_.count();
        int insert_end = insert_start + newSlice->rows_.count() - 1;
        beginInsertRows(parentIndex, insert_start, insert_end);

        currentSlice->rows_.append(newSlice->rows_);
        currentSlice->sourceNodes_.append(newSlice->sourceNodes_);
        currentSlice->sourceRowIndices_.append(newSlice->sourceRowIndices_);

        endInsertRows();

        // -----------------------------------------------------
        // Call the virtual "signal" for the newly inserted rows
        sliceRowsInserted(currentSlice, insert_start, insert_end);
    }

    delete newSlice;
    newSlice = 0;
}

/**
  * Because SliceModel performs filtering (via subclasses) and it utilizes a unique two-tier tree structure,
  * there are two possible cases to deal with when the source tree model rows are removed:
  *
  * Case 1: source tree rows also represented in a slice are removed (slice items)
  * Case 2: a mapped tree node is removed (top-level item); less obvious because a mapped node may occur deeply
  *         beneath the unmapped sourceParent node
  *
  * Both cases are relatively easy. Case 1 only occurs when the sourceParent node has a mapped slice. In this
  * instance, any slice row source indices that occur between start and end are tagged for removal. In the
  * sourceRowsRemoved method, rows tagged for removal are removed, the remaining neighboring source row index
  * numbers updated accordingly, and the appropriate signals emitted.
  *
  * There are two ways for handling the second case. The first involves recursing through all nodes of sourceParent,
  * start, and end. If any of the TreeNodes is mapped (sliceHash_.contains(AdocTreeNode *)), then tag it for removal.
  * While this is a workable method, it utilizes recursion and could be costly if the source tree contains many
  * nodes. The alternative approach taken here simply iterates through all mappedNodes (which typically would be very
  * few), and walks up its chain of parents. If any of its ancestors are the parent node and this parent node falls
  * between start and stop, tag it for removal. This latter technique should be quite rapid and efficient and does
  * not require any recursion.
  *
  * The appropriate rows to remove from this SliceModel are recognized and stored in these method and then
  * actually removed in its cognate method, sourceRowsRemoved(). It is necessary to perform these steps *before*
  * the source tree rows are removed in order to traverse the tree structure when searching for mapped nodes.
  *
  * Because of the arbitrary ordering of rows in this model relative to its source tree model, a 1:1 mapping of the
  * sourceRowsAboutToBeRemoved(...) and sourceRowsRemoved(...) signals is not possible. In most cases, the corresponding
  * start..stop tree range will not be contiguous within this model. Therefore, removal of rows in response to this
  * event is divided into 1 or more ranges each of which will trigger its own pair of beginRemoveRows(...) and
  * endRemoveRows(), which are called in the sourceRowsRemoved method.
  *
  * ASSUME: arguments in this method will be called with exact same arguments of sourceRowsRemoved
  *
  * OPTIMIZATION: In the current implementation, each row to be removed is stored separately. This could be optimized
  *               by combining these into contiguous ranges when possible; however, this would require additional
  *               effort at this time which likely would result in a negligible performance benefit.
  *
  * @param sourceParent [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  * @see sourceRowsRemoved()
  */
void SliceModel::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::sourceRowsAboutToBeRemoved", "Received sourceRowsAboutToBeRemoved signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    // Column must be zero unless the sourceParent is the root node (as indicated by an invalid QModelIndex)
    if (sourceParent.isValid() && sourceParent.column() != 0)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(sourceParent);
    if (!parentNode)
        return;

    // The following two containers should be emptied via the sourceRowsRemoved() method. If they
    // are not empty, either sourceRowsRemoved contains an error or the caller did not call endRemoveRows
    // before calling beginRemoveRows again.
    Q_ASSERT_X(sliceRowsToRemove_.isEmpty(), "SliceModel::sourceRowsAboutToBeRemoved", "sliceItemsToRemove is not empty; did you forget to call endRemoveRows()?");
    Q_ASSERT_X(mappedNodesToRemove_.isEmpty(), "SliceModel::sourceRowsAboutToBeRemoved", "mappedNodesToRemove_ is not empty; did you forget to call endRemoveRows()?");

    // Release mode guard
    sliceRowsToRemove_.clear();
    mappedNodesToRemove_.clear();

    // Case 1: is the parentNode mapped?
    if (sliceHash_.contains(parentNode))
    {
        // Yes. Tag for removal any slice rows that fall between start and stop.
        // It is not possible to simultaneously update the parent row numbering at this point
        // because we do not know how many rows between start and end are to be removed (due to
        // potential filtering implemented in subclasses).
        //
        // Also, we iterate through all the slice rows because they may be arbitrarily ordered within
        // this model (to provide for sorting, easier insertion of new items, etc.)
        Slice *slice = sliceHash_[parentNode];
        for (int i=0, z=slice->sourceRowIndices_.count(); i<z; ++i)
        {
            if (slice->sourceRowIndices_.at(i) >= start
                && slice->sourceRowIndices_.at(i) <= end)
            {
                sliceRowsToRemove_.append(qMakePair(i, i));

                // Shortcut optimization - if we have already found stop - end + 1 rows to be removed,
                // there is no reason to continue searching the remaining slice rows. Of course, this
                // condition will only occur if all end - start + 1 rows in the tree model are also
                // present within this SliceModel.
                if (sliceRowsToRemove_.count() == end - start + 1)
                    break;
            }
        }
    }

    // Case 2: Find all top-level rows that need to be removed
    for (int i=0, z=mappedNodes_.count(); i<z; ++i)
    {
        TreeNode *node = mappedNodes_.at(i);
        while (node)
        {
            if (node->parent() == parentNode
                && node->row() >= start
                && node->row() <= end)
            {
                mappedNodesToRemove_.append(i);
                break;
            }

            // Walk up the parent chain to the next parent
            node = node->parent();
        }
    }
}

/**
  * Cognate method to sourceRowsAboutToBeRemoved which performs the following responsibilities:
  * o Remove the actual slice items (case 1) from SliceModel and issue the appropriate signals
  * o Update the parent source row index numbering
  * o Remove top-level mapped nodes
  *
  * @param sourceParent [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  * @see sourceRowsAboutToBeRemoved()
  */
void SliceModel::sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end)
{
    Q_ASSERT_X(adocTreeModel_, "SliceModel::sourceRowsRemoved", "Received sourceRowsRemoved signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    // Column must be zero unless the sourceParent is the root node (as indicated by an invalid QModelIndex)
    if (sourceParent.isValid() && sourceParent.column() != 0)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(sourceParent);
    if (!parentNode)
        return;

    // Case 1: slice rows
    //         In the sourceRowsAboutToBeRemoved method, the slice rows were appended to the sliceRowsToRemove_
    //         in ascending order. Since we remove them one at a time, work backwards through the list of row
    //         numbers (to prevent out of range error)
    if (sliceHash_.contains(parentNode))    // Release mode guard
    {
        Slice *slice = sliceHash_[parentNode];
        if (sliceRowsToRemove_.count())
        {
            QModelIndex parentIndex = indexFromNode(parentNode);
            Q_ASSERT_X(parentIndex.isValid(), "SliceModel::sourceRowsRemoved", "Removal of slice rows must have valid parent index");
            Q_ASSERT_X(sliceHash_.contains(parentNode), "SliceModel::sourceRowsRemoved", "parentNode must be mapped to a valid slice (removing slice rows)");
            for (int i=sliceRowsToRemove_.count() - 1; i>= 0; --i)
            {
                QPair<int, int> sliceRange = sliceRowsToRemove_.at(i);
                beginRemoveRows(parentIndex, sliceRange.first, sliceRange.second);

                // -----------------------------------------------------
                // Call the virtual "signal" for the rows being removed
                sliceRowsAboutToBeRemoved(slice, sliceRange.first, sliceRange.second);

                for (int j=sliceRange.second; j>= sliceRange.first; --j)
                {
                    slice->rows_.removeAt(j);
                    slice->sourceNodes_.removeAt(j);
                    slice->sourceRowIndices_.removeAt(j);
                }

                endRemoveRows();

                // -----------------------------------------------------
                // Call the virtual "signal" for the rows being removed
                sliceRowsRemoved(slice, sliceRange.first, sliceRange.second);
            }
        }

        // Now update the parent row numbering for all remaining rows
        for (int i=0, z=slice->rows_.count(); i<z; ++i)
        {
            if (slice->sourceRowIndices_.at(i) > end)
            {
                slice->sourceRowIndices_[i] -= (end - start + 1);
                continue;
            }

            Q_ASSERT_X(slice->sourceRowIndices_.at(i) < start, "SliceModel::sourceRowsRemoved", "Remaining parent row index falls between start and end");
        }
    }

    // Case 2: top-level items
    //         Same logic as for case 1 when removing top-level rows
    for (int i=mappedNodesToRemove_.count()-1; i>= 0; --i)
    {
        int mappedRowI = mappedNodesToRemove_.at(i);
        beginRemoveRows(QModelIndex(), mappedRowI, mappedRowI);

        // Free the slice
        Slice *slice = sliceHash_[ mappedNodes_.at(mappedRowI) ];

        // -----------------------------------------------------
        // Call the virtual "signal" for the slice being removed
        sliceAboutToBeRemoved(slice);

        delete slice;
        nodeHash_.remove(slice);
        slice = 0;
        sliceHash_.remove(mappedNodes_.at(mappedRowI));
        mappedNodes_.removeAt(mappedRowI);

        endRemoveRows();
    }

    // Clear all stored items to remove
    sliceRowsToRemove_.clear();
    mappedNodesToRemove_.clear();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * This method should only be called during model resets and destruction. All appropriate signals
  * should be emitted externally to calling this method to ensure the model stays in a consistent
  * state and views do not attempt to request invalid data.
  */
void SliceModel::clearStructures()
{
    // Free all slice memory
    qDeleteAll(sliceHash_);
    mappedNodes_.clear();
    sliceHash_.clear();
    nodeHash_.clear();

    // -----------------------------------------------------
    // Call the virtual "signal" that all slices are removed
    slicesReset();
}
