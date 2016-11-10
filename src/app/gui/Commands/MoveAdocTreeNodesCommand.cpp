/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "MoveAdocTreeNodesCommand.h"
#include "../models/AdocTreeModel.h"
#include "../../core/AdocTreeNode.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocTreeModel [AdocTreeModel *]
  * @param srcRow [int]
  * @param srcCount [int]
  * @param srcParentNode [AdocTreeNode *]
  * @param dstParentNode [AdocTreeNode *]
  * @param parentCommand [QUndoCommand *]
  */
MoveAdocTreeNodesCommand::MoveAdocTreeNodesCommand(AdocTreeModel *adocTreeModel,
                                                   int srcRow,
                                                   int count,
                                                   AdocTreeNode *srcParentNode,
                                                   AdocTreeNode *dstParentNode,
                                                   QUndoCommand *parentCommand)
    : QUndoCommand(parentCommand),
      adocTreeModel_(adocTreeModel),
      srcRow_(srcRow),
      count_(count),
      srcParentNode_(srcParentNode),
      dstParentNode_(dstParentNode)
{
    ASSERT(srcParentNode != nullptr);
    ASSERT(dstParentNode != nullptr);
    ASSERT(count > 0);
    ASSERT(srcRow >= 0 && srcRow + count - 1 < srcParentNode->childCount());
    setText(QString("Moving %1 node(s)").arg(count));
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void MoveAdocTreeNodesCommand::redo()
{
    srcRow_ = adocTreeModel_->moveRows(srcRow_, count_, srcParentNode_, dstParentNode_);
}

/**
  */
void MoveAdocTreeNodesCommand::undo()
{
    srcRow_ = adocTreeModel_->moveRows(srcRow_, count_, dstParentNode_, srcParentNode_);
}
