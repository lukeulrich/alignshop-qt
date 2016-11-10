/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "AdocTreeNodeFilterModel.h"
#include "AdocTreeModel.h"
#include "../../core/AdocTreeNode.h"
#include "../../core/global.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param parent [QObject *]
  */
AdocTreeNodeFilterModel::AdocTreeNodeFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent), adocTreeModel_(nullptr)
{
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param acceptableNodeTypes [QSet<AdocNodeType>]
  */
void AdocTreeNodeFilterModel::setAcceptableNodeTypes(QSet<AdocNodeType> acceptableNodeTypes)
{
    beginResetModel();
    acceptableNodeTypes_ = acceptableNodeTypes;
    endResetModel();
}

/**
  * @returns QSet<AdocNodeType>
  */
QSet<AdocNodeType> AdocTreeNodeFilterModel::acceptableNodeTypes() const
{
    return acceptableNodeTypes_;
}

/**
  * @param sourceModel [AdocTreeModel *]
  */
void AdocTreeNodeFilterModel::setSourceModel(AdocTreeModel *sourceModel)
{
    adocTreeModel_ = sourceModel;

    QSortFilterProxyModel::setSourceModel(sourceModel);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Protected methods
/**
  * @param sourceRow [int]
  * @param sourceParent [const QModelIndex &]
  * @returns bool
  */
bool AdocTreeNodeFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    ASSERT(adocTreeModel_ != nullptr);
    ASSERT(sourceParent.isValid() == false || sourceParent.model() == adocTreeModel_);

    AdocTreeNode *sourceParentNode = adocTreeModel_->nodeFromIndex(sourceParent);
    ASSERT(sourceParentNode != nullptr);

    AdocNodeType rowNodeType = sourceParentNode->childAt(sourceRow)->nodeType_;

    return acceptableNodeTypes_.contains(rowNodeType);
}
