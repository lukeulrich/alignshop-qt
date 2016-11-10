/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include <QtGui/QAbstractButton>
#include <QtGui/QMessageBox>

#include "InsertTaskNodesCommand.h"

#include "../Services/TaskManager.h"
#include "../Services/Tasks/ITask.h"

#include "../../core/Entities/TransientTask.h"

#include "../../core/macros.h"

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param adocTreeModel [AdocTreeModel *]
  * @param adocTreeNode [AdocTreeNode *]
  * @param parentNode [AdocTreeNode *]
  * @param taskManager [TaskManager *]
  * @param widget [QWidget *]
  * @param parentCommand [QUndoCommand *]
  */
InsertTaskNodesCommand::InsertTaskNodesCommand(AdocTreeModel *adocTreeModel,
                                               AdocTreeNode *adocTreeNode,
                                               AdocTreeNode *parentNode,
                                               TaskManager *taskManager,
                                               QWidget *widget,
                                               QUndoCommand *parentCommand)
    : InsertAdocTreeNodesCommand(adocTreeModel, adocTreeNode, parentNode, parentCommand),
      taskManager_(taskManager),
      widget_(widget)
{
    ASSERT(taskManager_ != nullptr);
}

/**
  * @param adocTreeModel [AdocTreeModel *]
  * @param adocTreeNodeVector [const AdocTreeNodeVector &]
  * @param parentNode [AdocTreeNode *]
  * @param taskManager [TaskManager *]
  * @param widget [QWidget *]
  * @param parentCommand [QUndoCommand *]
  */
InsertTaskNodesCommand::InsertTaskNodesCommand(AdocTreeModel *adocTreeModel,
                                               const AdocTreeNodeVector &adocTreeNodeVector,
                                               AdocTreeNode *parentNode,
                                               TaskManager *taskManager,
                                               QWidget *widget,
                                               QUndoCommand *parentCommand)
    : InsertAdocTreeNodesCommand(adocTreeModel, adocTreeNodeVector, parentNode, parentCommand),
      taskManager_(taskManager),
      widget_(widget)
{
    ASSERT(taskManager_ != nullptr);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @returns bool
  */
bool InsertTaskNodesCommand::acceptUndo() const
{
    int nActiveJobs = 0;

    foreach (AdocTreeNode *adocTreeNode, adocTreeNodeVector_)
    {
        // It is quite possible and likely in fact that the node has been "upgraded" and is no longer transient. In this
        // case there is no need to check the task status
        if (adocTreeNode->nodeType_ != eTransientTaskNode)
            continue;

        ASSERT(adocTreeNode->entity() != nullptr);
        ASSERT(adocTreeNode->entity()->type() == eTransientTaskEntity);

        const TransientTaskSPtr &transientTaskEntity = boost::shared_static_cast<TransientTask>(adocTreeNode->entity());
        ITask *task = transientTaskEntity->task();

        // Another possibility is that the task was killed prematurely (undone) and therefore is no longer valid. One
        // sequence of events that could lead up to this:
        // 1) Start a task
        // 2) Undo -> kill job
        //    o this will set the transientTaskEntity's ITask pointer to null (see
        //      TaskAdocConnector::onTaskAboutToBeDestroyed)
        // 3) Redo - this recreates the task node, but does not re-issue the task to the taskmanager
        // 4) Undo
        if (task == nullptr)
            continue;

        if (task->status() == Ag::NotStarted || task->isActive())
            ++nActiveJobs;
    }

    if (nActiveJobs)
    {
        if (widget_ == nullptr)
            return false;

        // Else, ask the user
        QMessageBox msgBox(widget_);
        msgBox.setWindowTitle("Confirm undo");
        msgBox.setText(QString("%1 jobs are either pending or actively running. Please confirm that you want to "
                               "undo this action. All related jobs will be killed. Continue?").arg(nActiveJobs));
        msgBox.setStandardButtons(QMessageBox::Cancel | QMessageBox::Yes);
        msgBox.setDefaultButton(QMessageBox::Cancel);
        msgBox.button(QMessageBox::Yes)->setText("Kill jobs and continue");
        if (msgBox.exec() == QMessageBox::Cancel)
            return false;
    }

    return true;
}

/**
  * Case 1: Task is pending
  * Action -> remove from task manager
  *
  * Case 2: Task is actively running
  * Action -> kill the job
  *
  * Case 3: Task is in the current queue, but not active
  * Action -> remove from the taskmanager
  */
void InsertTaskNodesCommand::undo()
{
    // Remove any outstanding jobs
    foreach (AdocTreeNode *adocTreeNode, adocTreeNodeVector_)
    {
        // It is quite possible and indeed likely that the node has been "upgraded" and is no longer transient. In this
        // case there is no need to check the task status.
        if (adocTreeNode->nodeType_ != eTransientTaskNode)
            continue;

        ASSERT(adocTreeNode->entity() != nullptr);
        ASSERT(adocTreeNode->entity()->type() == eTransientTaskEntity);

        const TransientTaskSPtr &transientTaskEntity = boost::shared_static_cast<TransientTask>(adocTreeNode->entity());
        ITask *task = transientTaskEntity->task();
        if (task != nullptr && (task->status() != Ag::Error && task->status() != Ag::Killed))
            taskManager_->remove(task);
    }

    // Finally, remove the nodes from the Adoc Tree Model
    InsertAdocTreeNodesCommand::undo();
}
