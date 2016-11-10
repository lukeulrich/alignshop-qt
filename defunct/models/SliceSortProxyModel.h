/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#ifndef SLICESORTPROXYMODEL_H
#define SLICESORTPROXYMODEL_H

#include <QtGui/QSortFilterProxyModel>

// ------------------------------------------------------------------------------------------------
// Forward declarations
class SliceProxyModel;

/**
  * SliceSortProxyModel provides specific sorting capabilities for handling groups and items
  * represented within a source SliceModel.
  *
  * Group nodes are always displayed before any individual items of any sort. To achieve this
  * two steps are required. First, only permit SliceModel source models to guarantee that items
  * will be associated with slices. Second, override the virtual lessThan method to control how
  * the items should be sorted.
  */
class SliceSortProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    // ------------------------------------------------------------------------------------------------
    // Constructor and destructor
    explicit SliceSortProxyModel(QObject *parent = 0);                          //!< Standard constructor

    // ------------------------------------------------------------------------------------------------
    // Reimplemented public methods
    void setSourceModel(SliceProxyModel *sourceSliceProxyModel);                //!< Sets the given sourceSliceModel to be processed by this model
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);            //!< Reimplemented from QSortFilterProxyModel::sort()

protected:
    // ------------------------------------------------------------------------------------------------
    // Reimplemented protected methods
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const;     //!< Reimplemented from QSortFilterProxyModel::lessThan()

private:
    SliceProxyModel *sourceSliceProxyModel_;
    Qt::SortOrder sortOrder_;
};

#endif // SLICESORTPROXYMODEL_H
