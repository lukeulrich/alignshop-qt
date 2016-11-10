/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AllSliceModel.h"

#include <QtGui/QHeaderView>

#include <QtSql/QSqlField>

#include "../AdocTreeNode.h"
#include "AdocTreeModel.h"
#include "global.h"             // For ASSERT

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructor and destructor
/**
  * @param parent [QObject *]
  */
AllSliceModel::AllSliceModel(QObject *parent) : SliceProxyModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Reimplemented public methods
/**
  * Only one column - the name.
  *
  * @param parent [const QModelIndex & ]
  * @returns int
  */
int AllSliceModel::columnCount(const QModelIndex & /* parent */) const
{
    return 1;
}

/**
  * Only return data for the DisplayRole of child slice nodes. All other roles are supplied by the
  * underlying AdocTreeModel. Top-level nodes do not return any data.
  *
  * @param index [const QModelIndex &]
  * @param role [int]
  * @returns QVariant
  */
QVariant AllSliceModel::data(const QModelIndex &index, int role) const
{
    ASSERT(sourceTreeModel());

    if (!isGoodIndex(index))
        return QVariant();

    AdocTreeNode *node = slice_.sourceNodes_.at(index.row());
    return sourceTreeModel()->indexFromNode(node).data(role);
}


/**
  * @param index [const QModelIndex &]
  * @param value [const QVariant &]
  * @param role [int]
  * @returns bool
  */
bool AllSliceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    ASSERT(sourceTreeModel());

    if (!isGoodIndex(index))
        return false;

    AdocTreeNode *node = slice_.sourceNodes_.at(index.row());
    return sourceTreeModel()->setData(sourceTreeModel()->indexFromNode(node), value, role);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param header [QHeaderView *]
  */
void AllSliceModel::tweakHorizontalHeader(QHeaderView *header) const
{
    ASSERT_X(header, "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Call parent method
    header->setStretchLastSection(true);
}
