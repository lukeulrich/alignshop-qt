/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "InsertAdocTreeNodesCommand.h"

#include "../models/AdocTreeModel.h"
#include "../../core/AdocTreeNode.h"
#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocTreeModel [AdocTreeModel *]
  * @param adocTreeNode [AdocTreeNode *]
  * @param parentNode [AdocTreeNode *]
  * @param parentCommand [QUndoCommand *]
  */
InsertAdocTreeNodesCommand::InsertAdocTreeNodesCommand(AdocTreeModel *adocTreeModel, AdocTreeNode *adocTreeNode, AdocTreeNode *parentNode, QUndoCommand *parentCommand)
    : ConditionalUndoCommand(parentCommand),
      adocTreeModel_(adocTreeModel),
      adocTreeNodeVector_(AdocTreeNodeVector() << adocTreeNode),
      parentNode_(parentNode),
      row_(0),
      count_(1),
      done_(false)
{
    ASSERT(adocTreeModel != nullptr);
    ASSERT(adocTreeNode != nullptr);
    ASSERT(parentNode != nullptr);
    setText(QString("Appending 1 node"));
}

/**
  * @param adocTreeModel [AdocTreeModel *]
  * @param adocTreeNodeVector [const AdocTreeNodeVector &]
  * @param parentNode [AdocTreeNode *]
  * @param parentCommand [QUndoCommand *]
  */
InsertAdocTreeNodesCommand::InsertAdocTreeNodesCommand(AdocTreeModel *adocTreeModel,
                                                       const AdocTreeNodeVector &adocTreeNodeVector,
                                                       AdocTreeNode *parentNode,
                                                       QUndoCommand *parentCommand)
    : ConditionalUndoCommand(parentCommand),
      adocTreeModel_(adocTreeModel),
      adocTreeNodeVector_(adocTreeNodeVector),
      parentNode_(parentNode),
      row_(0),
      count_(adocTreeNodeVector_.size()),
      done_(false)
{
    ASSERT(adocTreeModel != nullptr);
    ASSERT(adocTreeNodeVector.size() > 0);
    ASSERT(parentNode != nullptr);
    setText(QString("Appending %1 nodes").arg(adocTreeNodeVector_.size()));
}

/**
  */
InsertAdocTreeNodesCommand::~InsertAdocTreeNodesCommand()
{
//    if (adocTreeNodeVector_.isEmpty())
    if (done_)
        return;

    emit adocTreeModel_->nodesAboutToBeDestroyed(adocTreeNodeVector_);
    qDeleteAll(adocTreeNodeVector_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void InsertAdocTreeNodesCommand::redo()
{
    row_ = adocTreeModel_->addRows(adocTreeNodeVector_, parentNode_);
//    adocTreeNodeVector_.clear();
    done_ = true;
}

/**
  */
void InsertAdocTreeNodesCommand::undo()
{
    adocTreeNodeVector_ = adocTreeModel_->takeRows(row_, count_, adocTreeModel_->indexFromNode(parentNode_));
    done_ = false;
}
