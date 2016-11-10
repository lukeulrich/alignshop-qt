/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocSortFilterProxyModel.h"

#include <QtGui/QHeaderView>

#include "models/AdocTreeModel.h"

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
AdocSortFilterProxyModel::AdocSortFilterProxyModel(QObject *parent) : QSortFilterProxyModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Function stub for tweaking the horizontal header. Subclass and re-implement this function
  * to provide specific header tweaking.
  *
  * @param header [QHeaderView *]
  */
void AdocSortFilterProxyModel::tweakHorizontalHeader(QHeaderView *header) const
{
    Q_ASSERT_X(header, "AdocSortFilterProxyModel::::tweakHorizontalHeader", "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Left align all columns
    for (int i=0, z=header->count(); i<z; ++i)
        header->setDefaultAlignment(Qt::AlignLeft);

    // Disable moving header sections
    header->setMovable(false);
}

bool AdocSortFilterProxyModel::lessThan(const QModelIndex &a, const QModelIndex &b) const
{
    Q_ASSERT_X(a.column() == b.column(), "AdocSortFilterProxyModel::lessThan", "a column must be the same as the b column");
    if (a.column() != 0)
        return QSortFilterProxyModel::lessThan(a, b);

    // Get the node type
    bool aIsFolder = a.data(IsGroupRole).toBool();
    bool bIsFolder = b.data(IsGroupRole).toBool();

    // Cases:
    // 1) a = group, b = group
    // 2) a = group, b = file
    // 3) a = file, b = group
    // 4) a = file, b = file
    //
    // Always display groups first and then items regardless of the SortRole
    if (sortOrder() == Qt::AscendingOrder)
    {
        if ((aIsFolder && bIsFolder) || (!aIsFolder && !bIsFolder))
            return a.data().toString() < b.data().toString();
        else if (!bIsFolder)
            return true;
        else // if (!aIsFolder)
            return false;
    }
    else // Descending order
    {
        if ((aIsFolder && bIsFolder) || (!aIsFolder && !bIsFolder))
            return a.data().toString() < b.data().toString();
        else if (!bIsFolder)
            return false;
        else // if (!aIsFolder)
            return true;
    }
}
