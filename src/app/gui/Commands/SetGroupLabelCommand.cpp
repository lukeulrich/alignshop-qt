/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "SetGroupLabelCommand.h"

#include "../models/AdocTreeModel.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param treeModel [AdocTreeModel *]
  * @param groupNode [AdocTreeNode *]
  * @param value [const QVariant &]
  * @param parentCommand [QUndoCommand *]
  */
SetGroupLabelCommand::SetGroupLabelCommand(AdocTreeModel *treeModel,
                                           AdocTreeNode *groupNode,
                                           const QVariant &value,
                                           QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      treeModel_(treeModel),
      groupNode_(groupNode),
      value_(value)
{
    ASSERT(treeModel != nullptr);
    ASSERT(groupNode->nodeType_ == eGroupNode);

    oldValue_ = treeModel->data(treeModel_->indexFromNode(groupNode));

    setText(QString("Renamed folder: %1 - %2").arg(oldValue_.toString()).arg(value_.toString()));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void SetGroupLabelCommand::redo()
{
    QModelIndex index = treeModel_->indexFromNode(groupNode_);
    treeModel_->setDataPrivate(index, value_);
}

/**
  */
void SetGroupLabelCommand::undo()
{
    QModelIndex index = treeModel_->indexFromNode(groupNode_);
    treeModel_->setDataPrivate(index, oldValue_);
}
