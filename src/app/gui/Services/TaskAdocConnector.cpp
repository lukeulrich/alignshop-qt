/****************************************************************************
**
** Copyright (C) 2011 Agile Genomics, LLC
** All rights reserved.
** Author: Luke Ulrich
**
****************************************************************************/

#include "TaskAdocConnector.h"
#include "TaskManager.h"
#include "Tasks/IEntityBuilderTask.h"
#include "Tasks/ITask.h"
#include "../models/AdocTreeModel.h"
#include "../models/ColumnAdapters/TransientTaskColumnAdapter.h"
#include "../models/TaskModel.h"
#include "../../core/Adoc.h"
#include "../../core/AdocTreeNode.h"
#include "../../core/macros.h"

#include <QtDebug>

class TransientTask;

// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Constructors and destructor
/**
  * @param taskManager [TaskManager *]
  * @param adoc [Adoc *]
  * @param adocTreeModel [AdocTreeModel *]
  * @param transientTaskEntityColumnAdapter [TransientTaskEntityColumnAdapter *]
  * @param parent [QObject *]
  */
TaskAdocConnector::TaskAdocConnector(TaskManager *taskManager,
                                     Adoc *adoc,
                                     AdocTreeModel *adocTreeModel,
                                     TransientTaskColumnAdapter *transientTaskEntityColumnAdapter,
                                     QObject *parent)
    : QObject(parent),
      adocTreeModel_(adocTreeModel),
      transientTaskEntityColumnAdapter_(transientTaskEntityColumnAdapter),
      adoc_(adoc)
{
    ASSERT(adoc_ != nullptr);
    connect(adoc_, SIGNAL(closed()), SLOT(onAdocClosed()));

    if (taskManager != nullptr)
    {
        connect(taskManager, SIGNAL(taskAboutToStart(ITask*)), SLOT(onTaskAboutToStart(ITask*)));

        ASSERT(taskManager->taskModel() != nullptr);
        connect(taskManager->taskModel(), SIGNAL(taskAboutToBeDestroyed(ITask*)), SLOT(onTaskAboutToBeDestroyed(ITask*)));
    }
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Public methods
/**
  * @param entityBuilderTask [IEntityBuilderTask *]
  * @param adocTreeNode [AdocTreeNode *]
  * @returns bool
  */
bool TaskAdocConnector::add(IEntityBuilderTask *entityBuilderTask, AdocTreeNode *adocTreeNode)
{
    if (entityBuilderTask == nullptr || adocTreeNode == nullptr)
        return false;

    if (adocTreeNode->nodeType_ != eTransientTaskNode)
        return false;

    if (adocTreeNode->entity() == nullptr || adocTreeNode->entity()->type() != eTransientTaskEntity)
        return false;

    // It is possible that the task has already been initiated before calling this method; go ahead and watch for its
    // signals.
    if (entityBuilderTask->isActive())
        watch(entityBuilderTask);

    taskToNode_.insert(entityBuilderTask, adocTreeNode);
    nodeToTask_.insert(adocTreeNode, entityBuilderTask);

    return true;
}

/**
  * @param adocTreeNode [AdocTreeNode *]
  * @returns bool
  */
bool TaskAdocConnector::remove(AdocTreeNode *adocTreeNode)
{
    if (!nodeToTask_.contains(adocTreeNode))
        return false;

    ITask *task = nodeToTask_.value(adocTreeNode);
    ASSERT(task != nullptr);
    unwatch(task);

    nodeToTask_.remove(adocTreeNode);
    taskToNode_.remove(task);

    return true;
}

/**
  * @param entityBuilderTask [IEntityBuilderTask *]
  * @returns bool
  */
bool TaskAdocConnector::remove(IEntityBuilderTask *entityBuilderTask)
{
    if (!taskToNode_.contains(entityBuilderTask))
        return false;

    AdocTreeNode *adocTreeNode = taskToNode_.value(entityBuilderTask);
    ASSERT(adocTreeNode != nullptr);
    unwatch(entityBuilderTask);

    nodeToTask_.remove(adocTreeNode);
    taskToNode_.remove(entityBuilderTask);

    return true;
}

/**
  * Since any tasks and adoc tree nodes may only be associated with a given Adoc, clear these out when the adoc is
  * closed.
  */
void TaskAdocConnector::onAdocClosed()
{
    taskToNode_.clear();
    nodeToTask_.clear();
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private slots
/**
  * @param task [ITask *]
  */
void TaskAdocConnector::onTaskAboutToStart(ITask *task)
{
    if (taskToNode_.contains(task))
        watch(task);
}

/**
  * @param task [ITask *]
  */
void TaskAdocConnector::onTaskAboutToBeDestroyed(ITask *task)
{
    if (!taskToNode_.contains(task))
        return;

    AdocTreeNode *adocTreeNode = taskToNode_.value(task);
    ASSERT(adocTreeNode != nullptr);
    ASSERT(adocTreeNode->nodeType_ == eTransientTaskNode);
    ASSERT(adocTreeNode->entity() != nullptr);
    ASSERT(adocTreeNode->entity()->type() == eTransientTaskEntity);

    // Clear the task pointer (ITask *, this is not an entity and thus does not need "unfind"ing)
    boost::shared_static_cast<TransientTask>(adocTreeNode->entity())->setTask(nullptr);

    // Remove from our internal hashes
    IEntityBuilderTask *entityBuilderTask = static_cast<IEntityBuilderTask *>(task);
#ifdef QT_DEBUG
    ASSERT(remove(entityBuilderTask));
#else
    remove(entityBuilderTask);
#endif
}

/**
  * @param task [ITask *]
  */
void TaskAdocConnector::onTaskDone(ITask *task)
{
    if (!taskToNode_.contains(task))
        return;

    AdocTreeNode *adocTreeNode = taskToNode_.value(task);

    ASSERT(nodeToTask_.contains(adocTreeNode));
    ASSERT(nodeToTask_.value(adocTreeNode) == task);

    ASSERT(adocTreeNode != nullptr);
    ASSERT(adocTreeNode->nodeType_ == eTransientTaskNode);
    ASSERT(adocTreeNode->entity() != nullptr);
    ASSERT(adocTreeNode->entity()->type() == eTransientTaskEntity);

    const TransientTaskSPtr &transientTask = boost::shared_static_cast<TransientTask>(adocTreeNode->entity());
    IEntityBuilderTask *entityBuilderTask = static_cast<IEntityBuilderTask *>(task);

    IEntitySPtr newEntity(entityBuilderTask->createFinalEntity(transientTask->name()));
    ASSERT(adoc_ != nullptr);
    IRepository *repository = adoc_->repository(newEntity);
    ASSERT(repository != nullptr);
    repository->addGeneric(newEntity, false);

    // As with SequenceImporter, this method will no longer maintain a reference to newEntity, so it should be
    // "unfind"ed; however, because it is replacing another entity on the tree model, it will have a final reference
    // count of 1 anyways (and AdocTreeModel does not deal directly with reference counting whatsoever - it delegates
    // this duty to the AdocTreeNodeEraserService via signals).
    //
    // And if viewed in the Multi model view, its reference count will increase to 2.

#ifdef QT_DEBUG
    bool retval = adocTreeModel_->replaceTransientEntity(adocTreeModel_->indexFromNode(adocTreeNode), newEntity);
    ASSERT(retval);
#else
    adocTreeModel_->replaceTransientEntity(adocTreeModel_->indexFromNode(adocTreeNode), newEntity);
#endif

    // Remove from our internal hashes
#ifdef QT_DEBUG
    ASSERT(remove(entityBuilderTask));
#else
    remove(entityBuilderTask);
#endif

    // Special case: users starts long task (e.g. BLAST), saves, then task finishes. Technically, the document is
    // modified; however, it won't allow the user to save.
    //
    // ISSUE: If user makes a change afterwards and then undoes that, the same problem resurfaces
    adoc_->setModified();
}

/**
  * @param task [ITask *]
  * @see onTaskStatusChanged()
  */
void TaskAdocConnector::onTaskError(ITask *task)
{
//    if (!taskToNode_.contains(task))
//        return;

    // Do not bother updating the TransientTask because a status changed signal should have been emitted which will
    // perform the relevant update.

#ifdef QT_DEBUG
//    ASSERT(remove(static_cast<IEntityBuilderTask *>(task)));
#else
//    remove(static_cast<IEntityBuilderTask *>(task));
#endif
}

/**
  * @param task [ITask *]
  */
void TaskAdocConnector::onTaskProgressChanged(ITask *task)
{
    if (!taskToNode_.contains(task))
        return;

    if (transientTaskEntityColumnAdapter_ == nullptr)
        return;

    AdocTreeNode *adocTreeNode = taskToNode_.value(task);
    ASSERT(adocTreeNode != nullptr);
    ASSERT(adocTreeNode->nodeType_ == eTransientTaskNode);
    ASSERT(adocTreeNode->entity() != nullptr);
    ASSERT(adocTreeNode->entity()->type() == eTransientTaskEntity);

    // Note the 4th parameter is false, which indicates that this update should not be pushed onto the stack. This is
    // simply a progress update - not actually changing any user data.
    transientTaskEntityColumnAdapter_->setData(adocTreeNode->entity(),
                                               TransientTaskColumnAdapter::eProgressColumn,
                                               task->progress(),
                                               false);
}

/**
  * @param task [ITask *]
  */
void TaskAdocConnector::onTaskStatusChanged(ITask *task)
{
    if (!taskToNode_.contains(task))
        return;

    if (transientTaskEntityColumnAdapter_ == nullptr)
        return;

    AdocTreeNode *adocTreeNode = taskToNode_.value(task);
    ASSERT(adocTreeNode != nullptr);
    ASSERT(adocTreeNode->nodeType_ == eTransientTaskNode);
    ASSERT(adocTreeNode->entity() != nullptr);
    ASSERT(adocTreeNode->entity()->type() == eTransientTaskEntity);

    // Note the 4th parameter is false, which indicates that this update should not be pushed onto the stack. This is
    // simply a progress update - not actually changing any user data.
    transientTaskEntityColumnAdapter_->setData(adocTreeNode->entity(),
                                               TransientTaskColumnAdapter::eStatusColumn,
                                               task->status(),
                                               false);
}


// -------------------------------------------------------------------------------------------------
// -------------------------------------------------------------------------------------------------
// Private methods
/**
  * @param task [ITask *]
  */
void TaskAdocConnector::unwatch(ITask *task)
{
    disconnect(task, SIGNAL(done(ITask*)), this, SLOT(onTaskDone(ITask*)));
    disconnect(task, SIGNAL(error(ITask*)), this, SLOT(onTaskError(ITask*)));
    disconnect(task, SIGNAL(progressChanged(ITask*)), this, SLOT(onTaskProgressChanged(ITask*)));
}

/**
  * @param task [ITask *]
  */
void TaskAdocConnector::watch(ITask *task)
{
    connect(task, SIGNAL(done(ITask*)), SLOT(onTaskDone(ITask*)));
    connect(task, SIGNAL(error(ITask*)), SLOT(onTaskError(ITask*)));
    connect(task, SIGNAL(progressChanged(ITask*)), SLOT(onTaskProgressChanged(ITask*)));
}
