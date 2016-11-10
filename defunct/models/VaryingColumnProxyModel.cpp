/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "VaryingColumnProxyModel.h"

#include <QtCore/QStringList>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Forward declarations
bool modelIndexRowGreaterThan(const QModelIndex &a, const QModelIndex &b);
bool modelIndexRowLessThan(const QModelIndex &a, const QModelIndex &b);


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * If nColumns is negative, nColumns will be set to zero.
  *
  * @param parent [QObject *]
  * @param nColumns [int]
  */
VaryingColumnProxyModel::VaryingColumnProxyModel(QObject *parent, const int nColumns) :
    QAbstractProxyModel(parent), nColumns_(nColumns)
{
    if (nColumns_ < 0)
        nColumns_ = 0;
}

/**
  * Free up the all memory associated with the source index mapping
  */
VaryingColumnProxyModel::~VaryingColumnProxyModel()
{
    clearSourceIndexMap();
}

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int VaryingColumnProxyModel::columnCount(const QModelIndex & /* parent */) const
{
    return nColumns_;
}

/**
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex VaryingColumnProxyModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!sourceModel())
    {
        qWarning("VaryingColumnProxyModel::index - no source model defined");
        return QModelIndex();
    }

    if (parent.isValid()
        && parent.model() != this)
    {
        qWarning("VaryingColumnProxyModel::index - parent does not belong to the sourceModel");
        return QModelIndex();
    }

    QModelIndex sourceParentIndex = mapToSource(parent);
    ParentMap *parentMap = parentMapFromSourceIndex(sourceParentIndex);
    return createIndex(row, column, parentMap);
}

/**
  * Returns a proxy index that is a replica of sourceIndex except in proxy space. For mapping purposes
  * the parentMap is stored within the internal pointer; however, since the internal pointer corresponds
  * to a heap-allocated instance of the private ParentMap structure, it is not possible to inspect
  * this data externally. It is vital that calling classes do not attempt to delete this data or the
  * program will likely crash and/or lead to undefined behavior.
  *
  * The core magic of this function really takes place in the parentMapFromSourceIndex() function, which
  * updates the child rows which have been mapped.
  *
  * @param sourceIndex [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex VaryingColumnProxyModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceModel())
    {
        qWarning("VaryingColumnProxyModel::mapFromSource - no source model defined");
        return QModelIndex();
    }

    if (!sourceIndex.isValid())
        return QModelIndex();

    if (sourceIndex.model() != sourceModel())
    {
        qWarning("VaryingColumnProxyModel::mapFromSource - sourceIndex does not belong to the sourceModel");
        return QModelIndex();
    }

    // Get the source parent of this node
    QModelIndex sourceParentIndex = sourceIndex.parent();

    // Extract its parent map
    ParentMap *parentMap = parentMapFromSourceIndex(sourceParentIndex);

    // Each proxy index contains a ParentMap pointer containing the sourceParent_ and those rows that
    // this index's parent has mapped. This functionality is handled in the parentMapFromSourceIndex
    // method.
    return createIndex(sourceIndex.row(), sourceIndex.column(), parentMap);
}

/**
  * Each proxy index contains an internal pointer to its parent map which contains its source parent.
  * Because of the 1:1 source:proxy mapping, finding the source index simply entails extracting the
  * source parent and calling the sourceModel with the same row, column and its stored parent index.
  *
  * @param proxyIndex [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex VaryingColumnProxyModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if (!sourceModel())
    {
        qWarning("VaryingColumnProxyModel::mapToSource - no source model defined");
        return QModelIndex();
    }

    if (!proxyIndex.isValid())
        return QModelIndex();

    if (proxyIndex.model() != this)
    {
        qWarning("VaryingColumnProxyModel::mapToSource - proxyIndex does not belong to this model");
        return QModelIndex();
    }

    ParentMap *parentMap = parentMapFromProxyIndex(proxyIndex);
    if (proxyIndex.row() < 0 || proxyIndex.row() >= sourceModel()->rowCount(parentMap->sourceParent_))
        return QModelIndex();

    // 1:1
    return sourceModel()->index(proxyIndex.row(), proxyIndex.column(), parentMap->sourceParent_);
}

/**
  * @param index [const QModelIndex &]
  * @returns QModelIndex
  */
QModelIndex VaryingColumnProxyModel::parent(const QModelIndex &index) const
{
    if (!sourceModel())
    {
        qWarning("VaryingColumnProxyModel::parent - no source model defined");
        return QModelIndex();
    }

    if (!index.isValid())
        return QModelIndex();

    if (index.model() != this)
    {
        qWarning("VaryingColumnProxyModel::parent - index does not belong to this model");
        return QModelIndex();
    }

    return mapFromSource(mapToSource(index).parent());
}

/**
  * @param parent [const QModelIndex &]
  * @returns int
  */
int VaryingColumnProxyModel::rowCount(const QModelIndex &parent) const
{
    if (!sourceModel())
    {
        qWarning("VaryingColumnProxyModel::rowCount - no source model defined");
        return 0;
    }

    if (parent.isValid()
        && parent.model() != this)
    {
        qWarning("VaryingColumnProxyModel::rowCount- parent does not belong to the sourceModel");
        return 0;
    }

    return sourceModel()->rowCount(mapToSource(parent));
}

/**
  * Always resets the model regardless if the column count has changed. Thus, it is the callers responsibility
  * to strategically call this function. The ideal approach would call the appropriate beginInsertColumns/
  * endInsertColumns (or remove equivalent) for each index in the model. However, to avoid having to traverse
  * the entire tree and deal with this logic, it is simpler to simply reset the model. Because of this approach,
  * it is advised to only change the column count during initialization or take pains to minimize unexpected
  * user behavior (losing current hierarchical state or reference invalid nodes).
  *
  * Any model indexes that have been saved at this point will no longer be valid after calling this function.
  * If it is necessary to reference such indexes, it is best to map them to a source model that will remain
  * stable and not invalidate the relevant saved indexes.
  *
  * @param nColumns [int]
  */
void VaryingColumnProxyModel::setColumnCount(int nColumns)
{
    Q_ASSERT_X(nColumns >= 0, QString("VaryingColumnProxyModel::setColumnCount(%1)").arg(nColumns).toAscii(), "Number of columns must be greater than zero");
    if (nColumns < 0)   // Release mode guard
        return;

    /*
    // Future enhancement:
    // - Trigger the appropriate column modification signals for all parent nodes in the tree
    if (nColumns_ < nColumns)
    {
        // Adding columns
    }
    else
    {
        // Removing columns
    }
    */

    nColumns_ = nColumns;

    // Perform the same functions as those executed when the source model resets.
    // The name is a slight misnomer given that we are not actually resetting the source model, but
    // rather calling the slot for when the source model is reset
    sourceModelReset();
}

/**
  * If another source model has been configured, disconnect any attached signals, set the the new source model,
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
  * @param sourceModel [QAbstractItemModel *]
  */
void VaryingColumnProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    if (QAbstractProxyModel::sourceModel() == sourceModel)
        return;

    // If source model already exists, disconnect from old source model signals
    if (QAbstractProxyModel::sourceModel())
    {
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

//        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)), this, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)));
//        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(columnsInserted(QModelIndex,int,int)), this, SIGNAL(columnsInserted(QModelIndex,int,int)));
//        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)), this, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)));
//        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(columnsRemoved(QModelIndex,int,int)), this, SIGNAL(columnsRemoved(QModelIndex,int,int)));

        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged()));
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
        QObject::disconnect(QAbstractProxyModel::sourceModel(), SIGNAL(modelAboutToBeReset()), this, SIGNAL(modelAboutToBeReset()));
    }

    // Call the parent method for this proxy model
    QAbstractProxyModel::setSourceModel(sourceModel);

    // If sourceModel is valid, attach to new source model signals
    if (sourceModel)
    {
        QObject::connect(sourceModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(sourceDataChanged(QModelIndex,QModelIndex)));
        QObject::connect(sourceModel, SIGNAL(modelReset()), this, SLOT(sourceModelReset()));
        QObject::connect(sourceModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)), this, SLOT(sourceRowsAboutToBeInserted(QModelIndex,int,int)));
        QObject::connect(sourceModel, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(sourceRowsInserted(QModelIndex,int,int)));
        QObject::connect(sourceModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)), this, SLOT(sourceRowsAboutToBeRemoved(QModelIndex,int,int)));
        QObject::connect(sourceModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), this, SLOT(sourceRowsRemoved(QModelIndex,int,int)));

//        QObject::connect(sourceModel, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)), this, SIGNAL(columnsAboutToBeInserted(QModelIndex,int,int)));
//        QObject::connect(sourceModel, SIGNAL(columnsInserted(QModelIndex,int,int)), this, SIGNAL(columnsInserted(QModelIndex,int,int)));
//        QObject::connect(sourceModel, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)), this, SIGNAL(columnsAboutToBeRemoved(QModelIndex,int,int)));
//        QObject::connect(sourceModel, SIGNAL(columnsRemoved(QModelIndex,int,int)), this, SIGNAL(columnsRemoved(QModelIndex,int,int)));

        QObject::connect(sourceModel, SIGNAL(headerDataChanged(Qt::Orientation,int,int)), this, SIGNAL(headerDataChanged(Qt::Orientation,int,int)));
        QObject::connect(sourceModel, SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged()));
        QObject::connect(sourceModel, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
        QObject::connect(sourceModel, SIGNAL(modelAboutToBeReset()), this, SIGNAL(modelAboutToBeReset()));
    }

    // Perform the same functions as those executed when the source model resets
    // The name is a slight misnomer given that we are not actually resetting the source model, but
    // rather calling the slot for when the source model is reset
    sourceModelReset();
}

// Drag and drop methods
/**
  * Because this model is a mirror of its source model, simply pass through all requests to the source model after
  * appropriately mapping the parent model index.
  *
  * @param data [const QMimeData *]
  * @param action [Qt::DropAction]
  * @param row [int]
  * @param column [int]
  * @param parent [const QModelIndex &]
  * @returns bool
  */
bool VaryingColumnProxyModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return sourceModel()->dropMimeData(data, action, row, column, mapToSource(parent));
}

/**
  * Because this model is a mirror of its source model, simply pass through all requests to the source model after
  * appropriately mapping the proxy indexes.
  *
  * @param indexes [const QModelIndexList &]
  * @returns QMimeData *
  */
QMimeData *VaryingColumnProxyModel::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList sourceIndexList;
    foreach (QModelIndex proxyIndex, indexes)
        sourceIndexList << mapToSource(proxyIndex);

    return sourceModel()->mimeData(sourceIndexList);
}

/**
  * Because this model is a mirror of its source model, simply call the source model function.
  *
  * @returns QStringList
  */
QStringList VaryingColumnProxyModel::mimeTypes() const
{
    return sourceModel()->mimeTypes();
}

/**
  * Because this model is a mirror of its source model, simply call the source model function.
  *
  * @returns Qt::DropActions
  */
Qt::DropActions VaryingColumnProxyModel::supportedDropActions() const
{
    return sourceModel()->supportedDropActions();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param topLeft [const QModelIndex &]
  * @param bottomRight [const QModelIndex &]
  */
void VaryingColumnProxyModel::sourceDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    emit dataChanged(mapFromSource(topLeft), mapFromSource(bottomRight));
}

/**
  * Clean up the sourceIndexMap and re-emit the model reset method
  */
void VaryingColumnProxyModel::sourceModelReset()
{
    clearSourceIndexMap();

    beginResetModel();
    endResetModel();
}

/**
  * @param sourceParent [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  */
void VaryingColumnProxyModel::sourceRowsAboutToBeInserted(const QModelIndex &sourceParent, int start, int end)
{
    beginInsertRows(mapFromSource(sourceParent), start, end);
}

/**
  * After the source model has inserted items, if the insertion occurred previous to any neighboring mapped rows
  * (which are tracked via their QModelIndexes), these will no longer be properly indexed. Thus, it is essential
  * to update the mappedRows_ as necessary.
  *
  * @param sourceParent [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  */
void VaryingColumnProxyModel::sourceRowsInserted(const QModelIndex &sourceParent, int start, int end)
{
    // If we have not mapped sourceParent, simply return after signaling endInsertRows
    if (!sourceIndexMap_.contains(sourceParent))
    {
        endInsertRows();
        return;
    }

    // Amount that the rows were shifted by this operation
    int delta = end - start + 1;

    // Extract the parentMap
    ParentMap *parentMap = sourceIndexMap_[sourceParent];

    // Loop through all mapped rows and if their location is >= start, then update their mapping in the
    // parentMap.
    //
    // Before beginning this operation, first sort the mapped rows in descending order by their row number.
    // This is to prevent overriding any previously set locations in the sourceIndexMap that might occur by
    // first updating mappedRows with a higher row number than those that have a lower number. For example,
    //
    // Root
    // ---> New node insert here <---
    // |___ Alpha * (old position: 0, 0) --> (1, 0)
    // |___ Beta * (old position: 1, 0) --> (2, 0)
    //
    // * Both alpha and beta are mapped (for demonstration purposes only, in reality they would both have
    //   to have children). If alpha is processed first, its updated location in sourceIndexMap_ would be 1, 0
    //   and overwrite the mapping currently stored at beta before beta has been moved to 2,0.
    qSort(parentMap->mappedRows_.begin(), parentMap->mappedRows_.end(), modelIndexRowGreaterThan);
    for (int i=0, z= parentMap->mappedRows_.count(); i<z; ++i)
    {
        QModelIndex mappedRow = parentMap->mappedRows_.at(i);

        // Ignore all mapped rows that occur prior to the start of the insert - these are unaffected by
        // the insert operation
        if (mappedRow.row() < start)
            continue;

        Q_ASSERT(sourceIndexMap_.contains(mappedRow));

        // Remove the mapping from the sourceIndexMap_
        ParentMap *parentParentMap = sourceIndexMap_.take(mappedRow);

        // Associate the map with a new model index that points to the proper position
        QModelIndex newIndex = sourceModel()->index(mappedRow.row() + delta, mappedRow.column(), sourceParent);
        sourceIndexMap_.insert(newIndex, parentParentMap);

        parentMap->mappedRows_[i] = newIndex;
    }

    endInsertRows();
}

/**
  * Updating the mapped proxy parent maps when removing nodes is a two-step process that is split across this
  * method and its sister method, sourceRowsRemoved(). This method handles removing all mapped source nodes
  * that are part of the subtree being removing (sourceParent, start -> end). The second method updates the
  * mapped parent node indices.
  *
  * @param sourceParent [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  * @see sourceRowsRemoved()
  */
void VaryingColumnProxyModel::sourceRowsAboutToBeRemoved(const QModelIndex &sourceParent, int start, int end)
{
    beginRemoveRows(mapFromSource(sourceParent), start, end);

    // Part 1: Recursively remove all mappings for those parents which are removed
    int nParentColumns = sourceModel()->columnCount(sourceParent);

    // Need to remove all those rows in sourceIndexMap_ that are no longer present!
    // Have to do this now before the source model is modified because we require the sourceModel()->index(...)
    // function, which requires valid indices at these positions.
    for (int i=start; i<=end; ++i)
    {
        for (int j=0; j<nParentColumns; ++j)
        {
            QModelIndex sourceIndex = sourceModel()->index(i, j, sourceParent);
            if (!sourceIndexMap_.contains(sourceIndex))
                continue;

            // This node is a parent that has been previously mapped
            // Remove from the grandparent parentRows structure
            Q_ASSERT(sourceIndexMap_.contains(sourceParent));

            ParentMap *parentMap = sourceIndexMap_[sourceParent];

            // OPTIMIZATION: Utilize a QHash in place of a QVector for immediate lookups
            Q_ASSERT(parentMap->mappedRows_.indexOf(sourceIndex) != -1);
            parentMap->mappedRows_.remove(parentMap->mappedRows_.indexOf(sourceIndex));

            recurseRemoveFromSourceIndexMap(sourceIndex);
        }
    }
}

/**
  * After the source model has removed items, if the removal occurred previous to any neighboring mapped rows
  * (which are tracked via their QModelIndexes), these will no longer be properly indexed. Thus, it is essential
  * to update the mappedRows_ as necessary.
  *
  * @param sourceParent [const QModelIndex &]
  * @param first [int]
  * @param last [int]
  * @see sourceRowsAboutToBeRemoved()
  */
void VaryingColumnProxyModel::sourceRowsRemoved(const QModelIndex &sourceParent, int start, int end)
{
    if (!sourceIndexMap_.contains(sourceParent))
    {
        endRemoveRows();
        return;
    }

    // Amount that the rows were shifted by this operation
    int delta = end - start + 1;

    // Extract the parentMap
    ParentMap *parentMap = sourceIndexMap_[sourceParent];

    // Loop through all mapped rows and if their location is > end, then update their mapping in the
    // parentMap.
    //
    // Before beginning this operation, first sort the mapped rows in ascending order by their row number.
    // This is to prevent overriding any previously set locations in the sourceIndexMap that might occur by
    // first updating mappedRows with a lower row number than those that have a higher row number. For example,
    //
    // Root
    // |___ TBR <--- being removed
    // |___ Alpha * (1, 0) -> (0, 0)
    // |___ Beta * (2, 0) -> (1, 0)
    //
    // * Both alpha and beta are mapped (for demonstration purposes only, in reality they would both have
    //   to have children). If beta is processed first, its updated location in sourceIndexMap_ would be 1, 0
    //   and overwrite the mapping currently stored at alpha before alpha has been moved to 0, 0.
    qSort(parentMap->mappedRows_.begin(), parentMap->mappedRows_.end(), modelIndexRowLessThan);
    for (int i=0, z=parentMap->mappedRows_.count(); i<z; ++i)
    {
        QModelIndex mappedRow = parentMap->mappedRows_.at(i);

        // All mapped rows between start and stop inclusive should have been removed in the rowsAboutToBeRemoved method
        Q_ASSERT(mappedRow.row() < start || mappedRow.row() > end);

        // Ignore all rows that do not occur after the end position
        if (mappedRow.row() <= end)
            continue;

        Q_ASSERT(sourceIndexMap_.contains(mappedRow));

        // Remove the mapping
        ParentMap *parentParentMap = sourceIndexMap_.take(mappedRow);

        // Associate the map with a new model index that points to the proper position
        QModelIndex newIndex = sourceModel()->index(mappedRow.row() - delta, mappedRow.column(), sourceParent);
        sourceIndexMap_.insert(newIndex, parentParentMap);

        parentMap->mappedRows_[i] = newIndex;
    }

    endRemoveRows();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Private methods
/**
  * If a node contains children, it will have a map of those children stored within its internal pointer.
  * This function is a convenience method for retrieving that value.
  *
  * @param proxyIndex [const QModelIndex &]
  * @returns ParentMap *
  */
VaryingColumnProxyModel::ParentMap *VaryingColumnProxyModel::parentMapFromProxyIndex(const QModelIndex &proxyIndex) const
{
    Q_ASSERT_X(proxyIndex.internalPointer(), "VaryingColumnProxyModel::parentMapFromProxyIndex", "Proxy index internal pointer must not be null");

    return reinterpret_cast<ParentMap *>(proxyIndex.internalPointer());
}

/**
  * This function not only returns a ParentMap that is associated with sourceIndex, it also updates the mapped children
  * for the sourceIndex's parent.
  *
  * @param sourceIndex [const QModelIndex &]
  * @returns ParentMap *
  */
VaryingColumnProxyModel::ParentMap *VaryingColumnProxyModel::parentMapFromSourceIndex(const QModelIndex &sourceIndex) const
{
    ParentMap *parentMap = 0;
    if (sourceIndexMap_.contains(sourceIndex))
    {
        parentMap = sourceIndexMap_[sourceIndex];
    }
    else
    {
        parentMap = new ParentMap;
        parentMap->sourceParent_ = sourceIndex;
        sourceIndexMap_[sourceIndex] = parentMap;
    }

    // If this index is not the source root, then ensure that this node is mapped in its parent map.
    if (sourceIndex.isValid())
    {
        QModelIndex parentSourceIndex = sourceIndex.parent();

        // If the parentSourceIndex is not mapped, then recursively call this function until it is mapped
        if (!sourceIndexMap_.contains(parentSourceIndex))
            parentMapFromSourceIndex(parentSourceIndex);

        Q_ASSERT_X(sourceIndexMap_.contains(parentSourceIndex), "VaryingColumnProxyModel::parentMapFromSourceIndex", "Parent source index must be mapped before child index may be mapped");

        ParentMap *parentParentMap = sourceIndexMap_[parentSourceIndex];

        // OPTIMIZATION: Utilize a QHash in place of a QVector for immediate lookups
        if (parentParentMap->mappedRows_.indexOf(sourceIndex) == -1)
            parentParentMap->mappedRows_.append(sourceIndex);
    }

    return parentMap;
}

/**
  * This function traverses the entire subtree beneath sourceIndex and removes all mapped nodes from the
  * sourceIndexMap_. This method is usually called after removing a particular node from the tree that was
  * previously mapped.
  *
  * @param sourceIndex [const QModelIndex &]
  */
void VaryingColumnProxyModel::recurseRemoveFromSourceIndexMap(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid())
        return;

    // Only want to remove sourceParent from the index map if it is not the root element
    if (!sourceIndexMap_.contains(sourceIndex))
        return;

    // Free up the memory and removed from the sourceIndexMap in the same step
    delete sourceIndexMap_.take(sourceIndex);

    // Loop through all children and update as necessary
    for (int i=0, z=sourceModel()->rowCount(sourceIndex); i<z; ++i)
        for (int j=0, y=sourceModel()->columnCount(sourceIndex); j<y; ++j)
            recurseRemoveFromSourceIndexMap(sourceModel()->index(i, j, sourceIndex));
}

/**
  * Because this function de-allocates all pointers to the mapped nodes, it is vital that it only be called
  * when it can be guaranteed that all proxy indices will not attempt to access this data.
  *
  */
void VaryingColumnProxyModel::clearSourceIndexMap()
{
    qDeleteAll(sourceIndexMap_);
    sourceIndexMap_.clear();
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Translation unit private methods
/**
  * @param a [const QModelIndex &]
  * @param b [const QModelIndex &]
  * @returns bool
  */
bool modelIndexRowGreaterThan(const QModelIndex &a, const QModelIndex &b)
{
    return b.row() < a.row();
}

/**
  * @param a [const QModelIndex &]
  * @param b [const QModelIndex &]
  * @returns bool
  */
bool modelIndexRowLessThan(const QModelIndex &a, const QModelIndex &b)
{
    return a.row() < b.row();
}
