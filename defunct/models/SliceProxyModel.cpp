/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SliceProxyModel.h"

#include <QtCore/QStringList>

#include "AdocTreeModel.h"
#include "TagGenerator.h"
#include "../AdocTreeNode.h"

#include "global.h"             // For ASSERT, ASSERT_X

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * Initialize pointers to zero.
  *
  * @param parent [QObject *]
  */
SliceProxyModel::SliceProxyModel(QObject *parent) : QAbstractItemModel(parent)
{
    adocTreeModel_ = 0;
    sourceParent_ = 0;
    removeMappedParent_ = false;
    defaultSortColumn_ = 0;
    primaryColumn_ = 0;
}

/**
  */
SliceProxyModel::~SliceProxyModel()
{
    clear();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * Utilizes the pure virtual methods, primaryColumn() and isEditableColumn(), along with index checking
  * to determine the appropriate flags for the given index. If the index is a group node for the
  * primaryColumn, then simply return the flags determined by source AdocTreeModel. Otherwise, if it is
  * not a group item, enable it and if isEditableColumn, add the editable flag.
  *
  * If index is an invalid index and sourceParent_ is not null, then return the flags for the index
  * corresponding to the sourceParent_ index.
  *
  * @param index [const QModelIndex &]
  * @see primaryColumn(), isEditableColumn()
  * @returns Qt::ItemFlags
  */
Qt::ItemFlags SliceProxyModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        if (sourceParent_)
            return adocTreeModel_->indexFromNode(sourceParent_).flags();

        return 0;
    }

    if (index.model() != this
        || index.row() >= slice_.sourceNodes_.count()
        || index.column() >= columnCount())
    {
        return 0;
    }

    AdocTreeNode *node = slice_.sourceNodes_.at(index.row());
    if (index.column() == primaryColumn())
        return adocTreeModel_->indexFromNode(node).flags();

    // This condition occurs when there are artificial indices created for a column (e.g. associated with a group
    // row or msa row) that is not the primary column.
    if (node->nodeType_ == AdocTreeNode::GroupType
        || node->nodeType_ == AdocTreeNode::MsaAminoType
        || node->nodeType_ == AdocTreeNode::MsaDnaType
        || node->nodeType_ == AdocTreeNode::MsaRnaType)
    {
        return 0;
    }

    // By default, all items are enabled
    Qt::ItemFlags flags = Qt::ItemIsEnabled;

    // But not all columns may be edited
    if (isEditableColumn(index.column()))
        flags |= Qt::ItemIsEditable;

    return flags;
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant SliceProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (!adocTreeModel_)
        return QVariant();

    return adocTreeModel_->headerData(section, orientation, role);
}

/**
  * No valid indices will be returned if a valid tree source model has not yet been defined. All items
  * must have row/column values that are in range. Parent is assumed to be invalid and thus is unused.
  *
  * Additionally, even though some slice rows may only have one column of data, due to the nature of
  * the Qt Model/View framework, it is necessary to create indices for all columns. Other model methods
  * are responsible for handling requests for these indices as needed (e.g. flags, data, setData).
  *
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &] - unused
  * @returns QModelIndex
  * @see columnCount()
  */
QModelIndex SliceProxyModel::index(int row, int column, const QModelIndex & /* parent */) const
{
    // Must have a source tree model
    if (!adocTreeModel_)
        return QModelIndex();

    // Check that row and column have valid values
    if (row < 0 || column < 0 || row >= rowCount() || column >= columnCount())
        return QModelIndex();

    return createIndex(row, column);
}

/**
  * Because SliceProxyModel is a flat table of rows, all rows have an invalid parent.
  *
  * @param child [const QModelIndex &]
  * @returns QModelIndex
  * @see index()
  */
QModelIndex SliceProxyModel::parent(const QModelIndex & /* child */) const
{
    return QModelIndex();
}

/**
  * This method provides for removing slice item rows.
  *
  * While functional in its own right, this is primarily to support drag and drop operations from the
  * various QAbstractItemView's that may be attached to this model.
  *
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &] - unused
  * @see releaseSlice()
  */
bool SliceProxyModel::removeRows(int row, int count, const QModelIndex & /* parent */)
{
    // Must have a source tree model
    if (!adocTreeModel_)
        return false;

    ASSERT_X(row >= 0 && row + count <= rowCount(), "row out of range");
    ASSERT_X(count >= 0, "count parameter must be greater than or equal to zero");
    if (row < 0 || row + count > rowCount())    // Release mode guard
        return false;

    if (count == 0)
        return true;

    QModelIndex sourceParentIndex = adocTreeModel_->indexFromNode(sourceParent_);

    // Removal of nodes is carried one by one beginning with the largest source tree row position and
    // ending with the smallest source tree row index. Because the order within sourceRowIndices_ does
    // not necessarily follow the source tree order, it is required to first sort them in ascending order.
    // If nodes that occurred earlier in the tree (smaller position) were removed before those that occur
    // later (larger position value), the tree would be corrupted.
    QList<int> orderedPositions = slice_.sourceRowIndices_.mid(row, count);
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
int SliceProxyModel::rowCount(const QModelIndex & /* parent */) const
{
    return slice_.count();
}


// ---------------------
// ---------------------
// Drag and drop methods
/**
  * A SliceProxyModel may only accept mime data if the column is the primaryColumn(). Otherwise, the source tree
  * model result with the corresponding parameters is returned.
  *
  * The parent argument must either be an invalid model index, or with valid row and whose column value is equivalent
  * to the primary column. Moreover, if valid, it must have originated from this model.
  *
  * @param data [const QMimeData *]
  * @param action [Qt::DropAction]
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  * @see primaryColumn()
  */
bool SliceProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    ASSERT_X(adocTreeModel_, "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return false;

    // If parent is invalid, dropping on sourceParent_; let AdocTreeModel decide
    QModelIndex sourceTreeParent;
    if (!parent.isValid())
    {
        sourceTreeParent = adocTreeModel_->indexFromNode(sourceParent_);
    }
    else
    {
        if (parent.model() != this)
        {
            qWarning("SliceModel::dropMimeData - parent index does not belong to this model");
            return false;
        }

        // Only consider drops onto the primary column
        if (parent.column() != primaryColumn())
            return false;

        // Check that the row is valid
        if (parent.row() >= rowCount())
            return false;

        sourceTreeParent = adocTreeModel_->indexFromNode(slice_.sourceNodes_.at(parent.row()));
    }

    return adocTreeModel_->dropMimeData(data, action, row, column, sourceTreeParent);
}

/**
  * Because this is a public function, it is possible that indexes contains indices that are not "correct" - they
  * may not belong to this model, may reference invalid columns, etc. Returns 0 if an error or invalid state is
  * encountered when mapping any index to its source tree equivalent. In other words, all indexes must be valid
  * to expect a non-zero result.
  *
  * Indices with different parents are allowed.
  *
  * @param indexes [const QModelIndexList &]
  * @returns QMimeData *
  * @see dropMimeData()
  */
QMimeData *SliceProxyModel::mimeData(const QModelIndexList &indexes) const
{
    ASSERT_X(adocTreeModel_, "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return 0;

    // Transform indexes into the source tree model indexes
    QModelIndexList sourceIndices;
    foreach (const QModelIndex &index, indexes)
    {
        if (!index.isValid() || index.model() != this || index.row() >= rowCount())
        {
            qWarning("SliceModel::mimeData - invalid index (row: %d, column %d, data: %s", index.row(), index.column(), index.data().toString().toAscii().constData());
            return 0;
        }

        if (index.column() != primaryColumn())
        {
            qWarning("SliceProxyModel::mimeData - index column (column: %d, data: %s) does not correspond to the primary column; skipping", index.column(), index.data().toString().toAscii().constData());
            return 0;
        }

        sourceIndices << adocTreeModel_->indexFromNode(slice_.sourceNodes_.at(index.row()));
    }

    return adocTreeModel_->mimeData(sourceIndices);
}

/**
  * @returns QStringList
  */
QStringList SliceProxyModel::mimeTypes() const
{
    Q_ASSERT_X(adocTreeModel_, "SliceProxyModel::mimeTypes", "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return QStringList();

    return adocTreeModel_->mimeTypes();
}

/**
  * @returns Qt::DropActions
  */
Qt::DropActions SliceProxyModel::supportedDragActions() const
{
    Q_ASSERT_X(adocTreeModel_, "SliceProxyModel::supportedDragActions", "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return 0;

    return adocTreeModel_->supportedDragActions();
}

/**
  * @returns Qt::DropActions
  */
Qt::DropActions SliceProxyModel::supportedDropActions() const
{
    Q_ASSERT_X(adocTreeModel_, "SliceProxyModel::supportedDropActions", "Missing valid source tree model");
    if (!adocTreeModel_)  // Release mode guard
        return 0;

    return adocTreeModel_->supportedDropActions();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Because setSourceParent takes a QModelIndex which will always refer to a valid AdocTreeNode (if the index
  * is invalid, the root pointer is returned), this method is necessary to set the model into an uninitialized
  * state. This entails setting the sourceParent_ pointer to 0, clearing the slice, and issuing a reset signal
  */
void SliceProxyModel::clear()
{
    beginResetModel();
    clearStructures();
    endResetModel();

    sliceCleared();
}

/**
  * @returns int
  */
int SliceProxyModel::defaultSortColumn() const
{
    return defaultSortColumn_;
}

/**
  * @param column [int]
  * @returns bool
  */
bool SliceProxyModel::isEditableColumn(int /* column */) const
{
    return false;
}

/**
  * @param index [const QModelIndex &]
  * @returns bool
  */
bool SliceProxyModel::isGoodIndex(const QModelIndex &index) const
{
    if (!index.isValid()
        || index.model() != this
        || index.row() >= rowCount()
        || index.column() >= columnCount())
    {
        return false;
    }

    return true;
}

/**
  * @param index [const QModelIndex &]
  * @returns bool
  */
bool SliceProxyModel::isGroupItem(const QModelIndex &index) const
{
    if (!index.isValid())
        return false;

    if (index.model() != this)
    {
        qWarning("SliceProxyModel::isGroupItem - index does not belong to this model");
        return false;
    }

    if (index.row() >= rowCount())
        return false;

    return slice_.sourceNodes_.at(index.row())->nodeType_ == AdocTreeNode::GroupType;
}

/**
  * The mechanism involves retrieving the corresponding AdocTreeNode pointer and then using
  * the source tree model's, indexFromNode() method to fetch the appropriate source tree index.
  *
  * If proxyIndex is an invalid index and sourceParent_ is defined, return the source parent
  * index.
  *
  * @param proxyIndex [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex SliceProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    ASSERT_X(adocTreeModel_, "Missing valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return QModelIndex();

    if (!proxyIndex.isValid())
    {
        if (sourceParent_)
            return adocTreeModel_->indexFromNode(sourceParent_);

        return QModelIndex();
    }

    if (proxyIndex.model() != this)    // Model compatibility check
    {
        qWarning("SliceProxyModel::mapToSource - proxyIndex does not belong to this model");
        return QModelIndex();
    }

    // Safe to assume if the above conditions pass, that proxyIndex is a validly created index
    // because the only mechanism for creating indices for this model is the index function. And
    // the index function constrains this process to only validly referencable indices.
    //
    // What about stale indices that the user has stored?
    // -> Should only have to check the upper row-count boundary for such indices

    ASSERT_X(proxyIndex.row() < rowCount(), "proxyIndex out of range");
    if (proxyIndex.row() >= rowCount())
        return QModelIndex();

    return adocTreeModel_->indexFromNode(slice_.sourceNodes_.at(proxyIndex.row()));
}

/**
  * @returns int
  */
int SliceProxyModel::primaryColumn() const
{
    return primaryColumn_;
}

/**
  * @param column [int]
  */
void SliceProxyModel::setDefaultSortColumn(int column)
{
    ASSERT_X(column >= 0 && (column == 0 || column < columnCount()), "column out of range");
    if (column >= 0 && (column == 0 || column < columnCount()))
        defaultSortColumn_ = column;
}

/**
  * @param column [int]
  */
void SliceProxyModel::setPrimaryColumn(int column)
{
    ASSERT_X(column >= 0 && (column == 0 || column < columnCount()), "column out of range");
    if (column >= 0 && (column == 0 || column < columnCount()))
        primaryColumn_ = column;
}

/**
  * Clears and resets the model to utilize the TreeNode pointer referenced by sourceParentIndex. If the
  * sourceParentIndex refers to the existing sourceParent_, do nothing and return. Only calls
  * taggedSliceCreated if there are any rows in the slice returned by createSlice.
  *
  * @param sourceParent [const QModelIndex &]
  * @see clear(), createSlice()
  */
void SliceProxyModel::setSourceParent(const QModelIndex &sourceParentIndex)
{
    ASSERT_X(adocTreeModel_, "Missing valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    // Do nothing if the source parent has not changed
    AdocTreeNode *newSourceParent = adocTreeModel_->nodeFromIndex(sourceParentIndex);
    if (newSourceParent == sourceParent_)
        return;

    // Remove all existing data and links
    clear();

    // Update the parent node
    sourceParent_ = newSourceParent;

    // Create any new slices
    Slice newSlice = createSlice(sourceParent_);
    if (newSlice.count())
    {
        int tag = TagGenerator::nextValue();
        taggedSlices_.insert(tag, newSlice);

        // Call the virtual function that this slice has been created. Subclasses may use this virutal
        // method to perform additional operations.
        taggedSliceCreated(tag);
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
void SliceProxyModel::setTreeModel(AdocTreeModel *adocTreeModel)
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

    sliceCleared();
}

/**
  * @returns const AdocTreeNode *
  */
const AdocTreeNode *SliceProxyModel::sourceParentNode() const
{
    return sourceParent_;
}

/**
  * @returns AdocTreeModel *
  * @see setTreeModel()
  */
AdocTreeModel *SliceProxyModel::sourceTreeModel() const
{
    return adocTreeModel_;
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public slots
/**
  * This method should be called only when necessary as all model data will be cleared and reloaded,
  * which potentially could be a very expensive operation. It is useful in cases such as in SubseqSliceModel
  * where one of the source Table Models is reset and it thus it is necessary to reload all that data.
  */
void SliceProxyModel::refreshSourceParent()
{
    // Do nothing if a source parent has not yet been defined
    if (!sourceParent_)
        return;

    // Problem: setSourceParent does nothing if sourceParent_ is the same as the one currently configured.
    //          but we want to refresh the data using the same sourceParent_.
    // Workaround: set sourceParent_ to 0 after first determining its corresponding source index
    QModelIndex sourceParentIndex = sourceTreeModel()->indexFromNode(sourceParent_);
    sourceParent_ = 0;

    setSourceParent(sourceParentIndex);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Subclasses should override this method and define whether a given node is accepted into the model.
  *
  * @param node [AdocTreeNode *]
  * @returns bool
  */
bool SliceProxyModel::filterAcceptsNode(AdocTreeNode * /* node */) const
{
    return true;
}

/**
  * @param slice [const Slice &]
  */
void SliceProxyModel::appendSlice(const Slice &slice)
{
    if (slice.isEmpty())
        return;

    // All additions are added at the end. Determine the start and end indices after all
    // tagged slice items have been transferred.
    int insert_start = slice_.count();
    int insert_end = insert_start + slice.count() - 1;
    beginInsertRows(QModelIndex(), insert_start, insert_end);
    slice_ << slice;
    endInsertRows();

    // Emit our virtual signal
    sliceRowsInserted(insert_start, insert_end);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Internal virtual "signals"
/**
  * Default implementation automatically transfers all slice items identified by tag (and thus
  * contained within the taggedSlices_ hash via tag) into slice_.
  *
  * @param tag [int]
  */
void SliceProxyModel::taggedSliceCreated(int tag)
{
    ASSERT(taggedSlices_.contains(tag));
    if (!taggedSlices_.contains(tag))   // Release mode guard
        return;

    // Extract tagged slice and return if it is empty
    appendSlice(taggedSlices_.take(tag));
}


// -------------------------------------------------------------------------------
// -------------------------------------------------------------------------------
// Private signals
/**
  * Because AdocTreeModel contains only a single column, only consider dataChanged signals where
  * the topLeft and bottomRight columns are zero.
  *
  * ASSUME: topLeft.parent() == bottomRight.parent()
  *
  * @param topLeft [const QModelIndex &]
  * @param bottomRight [const QModelIndex &]
  */
void SliceProxyModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    ASSERT_X(adocTreeModel_, "Received sourceDataChanged signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    if (topLeft.column() != 0)
        return;

    // Parent must be the same parent as the one we're modeling
    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(topLeft.parent());
    ASSERT(parentNode);
    if (sourceParent_ != parentNode)
        return;

    int x = topLeft.row();
    int y = bottomRight.row();
    int n = y - x + 1;

    // Find all nodes that have changed in our model
    // OPTIMIZE: Utilize a sorted list and contiguous blocks!
    for (int i=0, z= slice_.sourceRowIndices_.count(); i<z; ++i)
    {
        if (slice_.sourceRowIndices_[i] >= x && slice_.sourceRowIndices_[i] <= y)
        {
            QModelIndex childIndex = index(i, primaryColumn());       // CHECK: should column be primaryColumn?
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
void SliceProxyModel::sourceModelReset()
{
    ASSERT_X(adocTreeModel_, "Received sourceModelReset signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    clear();
}

/**
  * When new rows are inserted into the source tree model, it is necessary to find those rows that
  * adhere with the subclass filtering and append these to the appropriate slice. Before this can
  * happen though, the source parent node of the rows being inserted must be equivalent to that of
  * sourceParent_. New rows to be inserted into this model are generated in the same fashion as the
  * setSourceParent method. The slice creation process is delegated to the subclass via the virtual
  * createSlice method; however, here, createSlice is called with the start and end arguments of the
  * parent rows to consider. Thus, a slice is created only with the newly added items that match the
  * subclasses filter.
  *
  * The new slice items are simply appended to the original slice for this mapped node - ordering is
  * not taken into consideration. If a specific order is desired, a sort mechanism should be
  * implemented. Ordering does not strictly matter at this level, because this model is not functioning
  * as a sort model. That responsibility would be layered on via a QSortFilterProxyModel derivative
  * or redefine the sort method for this class.
  *
  * Conditions:
  * 1) adocTreeModel_ must be defined
  * 2) sourceParentIndex.column must be zero because AdocTreeModel by definition does not have other
  *    columns
  * 3) corresponding tree node must be non-null and previously mapped
  *
  * All pre-existing mapped slice items must have their parent row indices updated depending on if these
  * rows are inserted before their siblings in the source tree model. Because of this, there will be
  * instances where a slice need its sourceRowIndices_ updated yet without emitting any signals because no
  * additional rows were mapped.
  *
  * Update (27 October 2010): Added support to keep taggedSlices_ properly synced.
  *
  * @param sourceParentIndex [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  */
void SliceProxyModel::sourceRowsInserted(const QModelIndex &sourceParentIndex, int start, int end)
{
    ASSERT_X(adocTreeModel_, "Received sourceRowsInserted signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    // Column must be zero unless the sourceParent is the root node (as indicated by an invalid QModelIndex)
    if (sourceParentIndex.isValid() && sourceParentIndex.column() != 0)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(sourceParentIndex);
    if (!parentNode)
        return;

    if (parentNode != sourceParent_)
        return;

    // Update the sourceRowIndices_ that occur after start by the number of rows being inserted (end - start + 1)
    for (int i=0, z=slice_.sourceRowIndices_.count(); i<z; ++i)
        if (slice_.sourceRowIndices_.at(i) >= start)
            slice_.sourceRowIndices_[i] += (end - start + 1);

    // Update the tagged slices similarly
    QHash<int, Slice>::iterator i = taggedSlices_.begin();
    while (i != taggedSlices_.end())
    {
        Slice &slice = i.value();
        for (int j=0, z=slice.count(); j<z; ++j)
            if (slice.sourceRowIndices_.at(j) >= start)
                slice.sourceRowIndices_[j] += (end - start + 1);

        ++i;
    }

    // Call subclass method to return the slice in this range
    Slice newSlice = createSlice(parentNode, start, end);

    // Only propagate this response if new rows are to be inserted
    if (newSlice.count())
    {
        int tag = TagGenerator::nextValue();
        taggedSlices_.insert(tag, newSlice);
        taggedSliceCreated(tag);
    }
}

/**
  * Because SliceProxyModel performs filtering (via subclasses) and it utilizes a unique two-tier tree structure,
  * there are two possible cases to deal with when the source tree model rows are removed:
  *
  * Case 1: source tree rows also represented in a slice are removed (slice items)
  * Case 2: the sourceParent_ node is removed; less obvious because it may be the child of a parent unmapped
  *         sourceParentIndex node
  *
  * Both cases are relatively easy. Case 1 only occurs when the sourceParentIndex node corresponds to sourceParent_.
  * In this instance, any slice row source indices that occur between start and end are tagged for removal. In the
  * sourceRowsRemoved method, rows tagged for removal are removed, the remaining neighboring source row index
  * numbers updated accordingly, and the appropriate signals emitted.
  *
  * There are two ways for handling the second case. The first involves recursing through all nodes of sourceParent,
  * start, and end. If sourceParent_ equals any of the TreeNodes, then tag it for removal. While this is a workable
  * method, it utilizes recursion and could be costly if the source tree contains many nodes. The alternative approach
  * taken here simply iterates through all parents of sourceParent_. If any of its ancestors are the parent node and
  * this parent node falls between start and stop, tag it for removal. This latter technique should be quite rapid and
  * efficient and does not require any recursion.
  *
  * The appropriate rows to remove from this SliceProxyModel are recognized and stored in this method and then
  * actually removed in its cognate method, sourceRowsRemoved(). It is necessary to perform these steps *before*
  * the source tree rows are removed in order to traverse the tree structure when searching for mapped nodes.
  *
  * Because of the arbitrary ordering of rows in this model relative to its source tree model, a 1:1 mapping of the
  * sourceRowsAboutToBeRemoved(...) and sourceRowsRemoved(...) signals is not possible. In most cases, the corresponding
  * start..stop tree range will not be contiguous within this model. Therefore, removal of rows in response to this
  * event is divided into 1 or more ranges each of which will trigger its own pair of beginRemoveRows(...) and
  * endRemoveRows(), which are called in the sourceRowsRemoved method.
  *
  * Update (27 October 2010): Added support to keep taggedSlices_ properly synced.
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
void SliceProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParentIndex, int start, int end)
{
    Q_ASSERT_X(adocTreeModel_, "SliceProxyModel::sourceRowsAboutToBeRemoved", "Received sourceRowsAboutToBeRemoved signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    // Column must be zero unless the sourceParent is the root node (as indicated by an invalid QModelIndex)
    if (sourceParentIndex.isValid() && sourceParentIndex.column() != 0)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(sourceParentIndex);
    if (!parentNode)
        return;

    // The following two containers should be emptied via the sourceRowsRemoved() method. If they
    // are not empty, either sourceRowsRemoved contains an error or the caller did not call endRemoveRows
    // before calling beginRemoveRows again.
    ASSERT_X(sliceRowsToRemove_.isEmpty(), "sliceItemsToRemove is not empty; did you forget to call endRemoveRows()?");
    ASSERT_X(removeMappedParent_ == false, "removeMappedParent_ is not false; did you forget to call endRemoveRows()?");

    // Release mode guard
    sliceRowsToRemove_.clear();
    removeMappedParent_ = false;

    // Case 1: is the parentNode mapped?
    if (parentNode == sourceParent_)
    {
        // Yes. Save for removal any slice rows that fall between start and stop.
        // It is not possible to simultaneously update the parent row numbering at this point
        // because we do not know how many rows between start and end are to be removed (due to
        // potential filtering implemented in subclasses).
        //
        // Also, we iterate through all the slice rows because they may be arbitrarily ordered within
        // this model (to provide for sorting, easier insertion of new items, etc.)
        for (int i=0, z=slice_.sourceRowIndices_.count(); i<z; ++i)
        {
            if (slice_.sourceRowIndices_.at(i) >= start
                && slice_.sourceRowIndices_.at(i) <= end)
            {
                sliceRowsToRemove_.append(qMakePair(i, i));

                // Shortcut optimization - if we have already found stop - end + 1 rows to be removed,
                // there is no reason to continue searching the remaining slice rows. Of course, this
                // condition will only occur if all end - start + 1 rows in the tree model are also
                // present within this SliceProxyModel.
                if (sliceRowsToRemove_.count() == end - start + 1)
                    break;
            }
        }

        // --------------------------------
        // Do the same for all taggedSlices
        QHash<int, Slice>::iterator i = taggedSlices_.begin();
        while (i != taggedSlices_.end())
        {
            Slice &slice = i.value();

            QList<QPair<int, int> > sliceRowsToRemove;
            for (int j=0, z=slice.sourceRowIndices_.count(); j<z; ++j)
            {
                if (slice.sourceRowIndices_.at(j) >= start
                    && slice.sourceRowIndices_.at(j) <= end)
                {
                    sliceRowsToRemove.append(qMakePair(j, j));
                }
            }

            if (sliceRowsToRemove.count())
                taggedSliceRowsToRemove_.insert(i.key(), sliceRowsToRemove);

            ++i;
        }
        // --------------------------------
    }

    // Case 2: Check if sourceParent_ is a deep child to be removed
    TreeNode *node = sourceParent_;
    while (node)
    {
        if (node->parent() == parentNode
            && node->row() >= start
            && node->row() <= end)
        {
            removeMappedParent_ = true;
            break;
        }

        // Walk up the parent chain to the next parent
        node = node->parent();
    }
}

/**
  * Cognate method to sourceRowsAboutToBeRemoved which performs the following responsibilities:
  * o Remove the actual slice items (case 1) from SliceProxyModel and issue the appropriate signals
  * o Update the parent source row index numbering
  * o Unmap sourceParent_ if it has been removed
  *
  * Update (27 October 2010): Added support to keep taggedSlices_ properly synced.
  *
  * @param sourceParent [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  * @see sourceRowsAboutToBeRemoved()
  */
void SliceProxyModel::sourceRowsRemoved(const QModelIndex &sourceParentIndex, int start, int end)
{
    ASSERT_X(adocTreeModel_, "Received sourceRowsRemoved signal without valid source tree model");
    if (!adocTreeModel_)    // Release mode guard
        return;

    // Column must be zero unless the sourceParent is the root node (as indicated by an invalid QModelIndex)
    if (sourceParentIndex.isValid() && sourceParentIndex.column() != 0)
        return;

    AdocTreeNode *parentNode = adocTreeModel_->nodeFromIndex(sourceParentIndex);
    if (!parentNode)
        return;

    // Case 1: slice rows
    //         In the sourceRowsAboutToBeRemoved method, the slice rows were appended to the sliceRowsToRemove_
    //         in ascending order. Since we remove them one at a time, work backwards through the list of row
    //         numbers (to prevent out of range error)
    if (parentNode == sourceParent_)    // Release mode guard
    {
        if (sliceRowsToRemove_.count())
        {
            for (int i=sliceRowsToRemove_.count() - 1; i>= 0; --i)
            {
                QPair<int, int> sliceRange = sliceRowsToRemove_.at(i);
                beginRemoveRows(QModelIndex(), sliceRange.first, sliceRange.second);

                // -----------------------------------------------------
                // Call the virtual "signal" for the rows being removed
                sliceRowsAboutToBeRemoved(sliceRange.first, sliceRange.second);

                for (int j=sliceRange.second; j>= sliceRange.first; --j)
                {
                    slice_.sourceNodes_.removeAt(j);
                    slice_.sourceRowIndices_.removeAt(j);
                }

                endRemoveRows();

                // -----------------------------------------------------
                // Call the virtual "signal" for the rows being removed
                sliceRowsRemoved(sliceRange.first, sliceRange.second);
            }
        }

        // Now update the parent row numbering for all remaining rows
        for (int i=0, z=slice_.sourceRowIndices_.count(); i<z; ++i)
        {
            if (slice_.sourceRowIndices_.at(i) > end)
            {
                slice_.sourceRowIndices_[i] -= (end - start + 1);
                continue;
            }

            ASSERT_X(slice_.sourceRowIndices_.at(i) < start, "Remaining parent row index falls between start and end");
        }

        // -------------------------------------------
        // Do the same for all taggedSliceRowsToRemove
        if (taggedSliceRowsToRemove_.count())
        {
            foreach (int tag, taggedSliceRowsToRemove_.keys())
            {
                QList<QPair<int, int> > taggedSliceRowsToRemove = taggedSliceRowsToRemove_[tag];
                for (int i=taggedSliceRowsToRemove.count() - 1; i>= 0; --i)
                {
                    QPair<int, int> sliceRange = taggedSliceRowsToRemove.at(i);
                    for (int j=sliceRange.second; j>= sliceRange.first; --j)
                    {
                        taggedSlices_[tag].sourceNodes_.removeAt(j);
                        taggedSlices_[tag].sourceRowIndices_.removeAt(j);
                    }
                }
            }
        }

        // Now update the parent row numbering for all remaining rows
        foreach (int tag, taggedSlices_.keys())
        {
            Slice &slice = taggedSlices_[tag];
            for (int i=0, z=slice.sourceRowIndices_.count(); i<z; ++i)
            {
                if (slice.sourceRowIndices_.at(i) > end)
                {
                    slice.sourceRowIndices_[i] -= (end - start + 1);
                    continue;
                }

                ASSERT_X(slice.sourceRowIndices_.at(i) < start, "Remaining parent row index falls between start and end");
            }
        }
        // -----------------------------------------
    }

    // Case 2: mapped sourceParent_
    if (removeMappedParent_)
        clear();

    // Clear all stored items to remove
    sliceRowsToRemove_.clear();
    taggedSliceRowsToRemove_.clear();
    removeMappedParent_ = false;
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * This method should only be called during model resets and destruction. All appropriate signals
  * should be emitted externally to calling this method to ensure the model stays in a consistent
  * state and views do not attempt to request invalid data.
  */
void SliceProxyModel::clearStructures()
{
    sourceParent_ = 0;
    slice_ = Slice();
    taggedSlices_.clear();
}

/**
  * This is a convenience function that simply calls createSlice with all child indices if the parent
  * contains at least one child. Otherwise, it returns an empty slice.
  *
  * @param parent [AdocTreeNode *]
  * @returns Slice
  * @see createSlice(AdocTreeNode *, int, int)
  */
SliceProxyModel::Slice SliceProxyModel::createSlice(AdocTreeNode *parent) const
{
    if (parent->childCount() == 0)
        return Slice();

    return createSlice(parent, 0, parent->childCount()-1);
}

/**
  * Calls the virtual filterAcceptsNode method with each AdocTreeNode child of parent between start and
  * end inclusive and adds those which return true.
  *
  * @param parent [AdocTreeNode *parent]
  * @param start [int]
  * @param end [int]
  * @returns SliceProxyModel::Slice
  * @see filterAcceptsNode()
  */
SliceProxyModel::Slice SliceProxyModel::createSlice(AdocTreeNode *parent, int start, int end) const
{
    ASSERT_X(parent, "Null parent pointer not allowed");
    ASSERT_X(start >= 0, "start must be >= 0");
    ASSERT_X(start <= end, "start must be <= end");
    ASSERT_X(parent->childCount(), "parent must have at least one child");
    ASSERT_X(end < parent->childCount(), "end must be < parent->childCount()");

    Slice newSlice;
    for (int i=start; i<= end; ++i)
    {
        AdocTreeNode *childNode = static_cast<AdocTreeNode *>(parent->childAt(i));
        ASSERT(childNode);

        if (filterAcceptsNode(childNode))
        {
            newSlice.sourceNodes_ << childNode;
            newSlice.sourceRowIndices_ << i;
        }
    }

    return newSlice;
}
