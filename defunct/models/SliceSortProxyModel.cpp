/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "SliceSortProxyModel.h"

#include "SliceProxyModel.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
SliceSortProxyModel::SliceSortProxyModel(QObject *parent) : QSortFilterProxyModel(parent), sourceSliceProxyModel_(0)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplmented public methods
/**
  * Overriding this method accomplishes the following:
  * 1) Ensures that the source model is a SliceModel
  * 2) Store the sourceSliceModel pointer for use in the lessThan method which translates into
  *    a small performance boost by avoiding the cost of calling sourceModel() and casting the result
  *    to a SliceModel pointer
  *
  * @param sourceSliceProxyModel [SliceProxyModel *]
  */
void SliceSortProxyModel::setSourceModel(SliceProxyModel *sourceSliceProxyModel)
{
    QSortFilterProxyModel::setSourceModel(sourceSliceProxyModel);
    sourceSliceProxyModel_ = sourceSliceProxyModel;
}

/**
  * Overriding this method purely serves to store the SortOrder locally for optimization purposes and
  * simply calls the parent sort method.
  *
  * @param column [int]
  * @param order [Qt::SortOrder]
  * @see lessThan()
  */
void SliceSortProxyModel::sort(int column, Qt::SortOrder order)
{
    sortOrder_ = order;
    QSortFilterProxyModel::sort(column, order);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplmented protected methods
/**
  * This method is called when sorting in ascending and descending order. The difference is that
  * when items should be ordered in descending order, the calling function swaps the left and right
  * arguments. While functional with respect to sorting a list of items as a whole, the base class
  * implementation does not provide the desired functionality of this class. That is, to always have
  * group items appear before other items and then apply the normal sort mechanics within these subsets.
  *
  * For the above reasons, the SortOrder is examined during each call and proper logic returned:
  * o If left and right are both groups or both non-group items, call the parent lessThan method
  * o Ascending order
  *   If left is group, and right is non-group -> return true
  *   Vice versa                               -> return false
  * o Descending order
  *   If left is group and right is non-group  -> return false
  *   Vice versa                               -> return true
  *
  * @param left [const QModelIndex &]
  * @param right [const QModelIndex &]
  * @returns bool
  */
bool SliceSortProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    // Determine if left and right are groups
    bool leftIsGroup = sourceSliceProxyModel_->isGroupItem(left);
    bool rightIsGroup = sourceSliceProxyModel_->isGroupItem(right);

    // Case 1: Left is group and right is group --> QSortFilterProxyModel::lessThan
    // Case 2: Left is item and right is item   --> QSortFilterProxyModel::lessThan
    if (leftIsGroup == rightIsGroup)
        return QSortFilterProxyModel::lessThan(left, right);

    // Depending on the sort order handle the following cases
    // Case 3: Left is group and right is item
    // Case 4: Left is item and right is group
    if (sortOrder_ == Qt::AscendingOrder)
        return leftIsGroup;
    else
        return rightIsGroup;
}
