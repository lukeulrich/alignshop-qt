/****************************************************************************
**
** Copyright (C) 2012 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "FilterColumnProxyModel.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
FilterColumnProxyModel::FilterColumnProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
void FilterColumnProxyModel::excludeColumn(int sourceColumn)
{
    ASSERT_X(isValidSourceColumn(sourceColumn), "sourceColumn out of range");

    if (excludedColumns_.contains(sourceColumn))
        return;

    beginRemoveColumns(QModelIndex(), sourceColumn, sourceColumn);
    excludedColumns_ << sourceColumn;
    endRemoveColumns();
}

void FilterColumnProxyModel::includeColumn(int sourceColumn)
{
    ASSERT_X(isValidSourceColumn(sourceColumn), "sourceColumn out of range");

    if (!excludedColumns_.contains(sourceColumn))
        return;

    beginInsertColumns(QModelIndex(), sourceColumn, sourceColumn);
    excludedColumns_ << sourceColumn;
    endRemoveColumns();
}

int FilterColumnProxyModel::mapFromSource(const int sourceColumn) const
{
    if (!isValidSourceColumn(sourceColumn))
        return -1;

    if (excludedColumns_.contains(sourceColumn))
        return -1;

    int proxyColumn = sourceColumn;
    for (int i=0; i<sourceColumn; ++i)
    {
        if (excludedColumns_.contains(i))
            --proxyColumn;
    }

    return proxyColumn;
}

void FilterColumnProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    excludedColumns_.clear();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
bool FilterColumnProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex & /* sourceParentIndex */) const
{
    return !excludedColumns_.contains(sourceColumn);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
bool FilterColumnProxyModel::isValidSourceColumn(const int sourceColumn) const
{
    if (sourceModel() != nullptr)
        return sourceColumn >= 0 && sourceColumn < sourceModel()->columnCount();

    return false;
}
