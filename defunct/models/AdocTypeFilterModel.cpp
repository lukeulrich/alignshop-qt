/****************************************************************************
**
** Copyright (C) 2010 Agile Genomics, LLC
** All rights reserved.
** Primary author: Luke Ulrich
**
****************************************************************************/

#include "AdocTypeFilterModel.h"

#include "models/AdocTreeModel.h"

#include <QtGui/QHeaderView>

// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Constructors
/**
  * @param parent [QObject *]
  */
AdocTypeFilterModel::AdocTypeFilterModel(QObject *parent) : AdocSortFilterProxyModel(parent)
{
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns QList<AdocTreeNode::NodeType>
  */
QList<AdocTreeNode::NodeType> AdocTypeFilterModel::acceptNodeTypes() const
{
    return acceptNodeTypes_;
}

/**
  * Because filtering will be updated to reflect the newly set acceptNodeTypes, it is essential that we reset
  * the model to update these new acceptable node types.
  *
  * @param acceptNodeTypes [const QList<AdocTreeNode::NodeType> &]
  */
void AdocTypeFilterModel::setAcceptNodeTypes(QList<AdocTreeNode::NodeType> acceptNodeTypes)
{
    acceptNodeTypes_ = acceptNodeTypes;
    reset();
}

/**
  * Function stub for tweaking the horizontal header. Subclass and re-implement this function
  * to provide specific header tweaking.
  *
  * @param header [QHeaderView *]
  */
void AdocTypeFilterModel::tweakHorizontalHeader(QHeaderView *header) const
{
    Q_ASSERT_X(header, "AdocTypeFilterModel::::tweakHorizontalHeader", "header argument must not be null");
    if (!header)    // Release mode guard
        return;

    // Call parent method
    AdocSortFilterProxyModel::tweakHorizontalHeader(header);

    // This command is needed to prevent the last column from acquiring too great a width and forcing a scrollbar to appear
    // on the bottom
    header->setStretchLastSection(true);
}


// ------------------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------------------
// Protected methods
/**
  * Only returns those rows where nodeType is present within acceptNodeTypes_.
  *
  * @param nodeType [AdocTreeNode::NodeType]
  * @returns bool
  */
bool AdocTypeFilterModel::filterAcceptsRow(AdocTreeNode::NodeType nodeType) const
{
    return acceptNodeTypes_.indexOf(nodeType) != -1;
}

/**
  * If the source index is valid, extracts its corresponding NodeType and passes it to the pure virtual
  * filterAcceptsRow(AdocTreeNode::NodeType) function.
  *
  * @param sourceRow [int]
  * @param sourceParent [const QModelIndex &]
  * @returns bool
  * @see filterAcceptsRow(AdocTreeNode::NodeType)
  */
bool AdocTypeFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex source_index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (source_index.isValid())
    {
        AdocTreeNode::NodeType type = sourceModel()->data(source_index, AdocTreeModel::NodeTypeRole).value<AdocTreeNode::NodeType>();
        return filterAcceptsRow(type);
    }

    return false;
}
