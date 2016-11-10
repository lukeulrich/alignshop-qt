/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "RemoveAdocTreeNodesCommand.h"

#include "../models/AdocTreeModel.h"
#include "../../core/macros.h"

// #include <QtDebug>

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocTreeModel [AdocTreeModel *]
  * @param index [const QModelIndex &]
  * @param row [int]
  * @param count [int]
  * @param parent [QUndoCommand *]
  */
RemoveAdocTreeNodesCommand::RemoveAdocTreeNodesCommand(AdocTreeModel *adocTreeModel,
                                                       int row,
                                                       int count,
                                                       AdocTreeNode *parentNode,
                                                       QUndoCommand *parent)
    : QUndoCommand(parent), adocTreeModel_(adocTreeModel), row_(row), count_(count), parentNode_(parentNode)
{
    ASSERT(adocTreeModel != nullptr);
    ASSERT(row >= 0);
    ASSERT(parentNode != nullptr);
    ASSERT(row + count - 1 < parentNode->childCount());
    setText(QString("Removing %1 node(s) from beginning at %2").arg(count).arg(row));
}

/**
  * Note: should be empty if the command has been undone. Emit the nodesAboutToBeDestroyed signal so that interested
  * components (e.g. AdocTreeNodeEraserService) may perform any teardown associated with these nodes before they are
  * deallocated. For example, if the node contains an entity pointer that needs dereferencing ("unfound") from the
  * repository.
  */
RemoveAdocTreeNodesCommand::~RemoveAdocTreeNodesCommand()
{
    if (adocTreeNodeVector_.isEmpty())
        return;

    emit adocTreeModel_->nodesAboutToBeDestroyed(adocTreeNodeVector_);
    qDeleteAll(adocTreeNodeVector_);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  */
void RemoveAdocTreeNodesCommand::redo()
{
    adocTreeNodeVector_ = adocTreeModel_->takeRows(row_, count_, adocTreeModel_->indexFromNode(parentNode_));

//    qDebug() << Q_FUNC_INFO << row_ << count_ << adocTreeNodeVector_;
}

/**
  * For this to work properly, the adocTreeModel_ must not perform its own sorting!
  */
void RemoveAdocTreeNodesCommand::undo()
{
    // Restore rows simply appends the items to the parent node, therefore, it is essential to update the row_
    row_ = adocTreeModel_->addRows(adocTreeNodeVector_, parentNode_);
    adocTreeNodeVector_.clear();

    //    qDebug() << Q_FUNC_INFO << row_ << count_;
}
