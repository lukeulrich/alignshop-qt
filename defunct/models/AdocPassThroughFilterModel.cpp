/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocPassThroughFilterModel.h"

#include <QtGui/QHeaderView>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor
/**
  * @param parent [QObject *]
  */
AdocPassThroughFilterModel::AdocPassThroughFilterModel(QObject *parent) :
    AdocSortFilterProxyModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * Always returns 1 independent of the parent parameter because this is tied to a concrete model
  * implementation.
  *
  * @param parent [const QModelIndex &] - unused
  * @returns int
  */
int AdocPassThroughFilterModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

/**
  * @param header [QHeaderView *]
  */
void AdocPassThroughFilterModel::tweakHorizontalHeader(QHeaderView *header) const
{
    Q_ASSERT_X(header, "AdocPassThroughFilterModel::::tweakHorizontalHeader", "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Call parent method
    AdocSortFilterProxyModel::tweakHorizontalHeader(header);

    header->setStretchLastSection(true);
}

/**
  * @param section [int]
  * @param orientation [Qt::Orientation]
  * @param role [int]
  * @returns QVariant
  */
QVariant AdocPassThroughFilterModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();

    // Column headers
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
        case 0:
            return "Name";
        default:
            return QVariant();
        }
    }
    else // orientation == Qt::Vertical; use this to indicate the row number. Section is 0-based, so we add 1 to make it 1-based.
        return section + 1;
}
